/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ar.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   AR.CPP  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 11:56:42  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives_dummy/Digivote/Prg/ARC/AR.CPP-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/02/07 11:32:03 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

/***********************************************************
   ar.c -- archiver in ANSI C
   Apr 22 1990 H.Okumura
***********************************************************/

#if !defined( __X_ONLY__) && !defined( DIGTOOLS)
static char *usage =
   "ar -- archiver\n"
   "Usage: ar command archive [file ...]\n"
   "Commands:\n"
   "   ?: Show Usage information\n)"
   "   a: Add files to archive (replace if present)\n"
   "   x: Extract files from archive\n"
   "   r: Replace files in archive\n"
   "   d: Delete files from archive\n"
   "   p: Print files on standard output\n"
   "   l: List contents of archive\n"
   "If no files are named, all files in archive are processed,\n"
   "   except for commands 'a' and 'd'.\n"
   "You may copy and distribute this program freely.\n";
#endif // __X_ONLY__

/***********************************************************
Structure of archive block (low order byte first):
 2 basic header size (from 'method' thru 'filename' below)
      = 18 + strlen(filename) (= 0 if end of archive)
 2 method (0 = stored, 1 = compressed)
 1 file type (0: binary, 1: text(not supported yet))
 1 sec + (timeinfo << 6), where timeinfo = 0 (local),
      1 (local, not DST), 2 (local, DST),
      3 (UTC, not supported yet).
 2 (day << 11) + (hour << 6) + min
 2 ((year - 1900) << 4) + month
 4 compressed size
 4 original size
 2 original file's CRC
 ? filename (not null-terminated)
 2 basic header CRC
 2 1st extended header size (0 if none)
 ? 1st extended header
 2 1st extended header's CRC
 ...
 ? compressed file
***********************************************************/

#include "..\arc\ar.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <dos.h>

#include <conio.h>   // getch();

#define BYTESIZE  8
#define FNAME_MAX  1024
#define HEADERSIZE_MAX  (FNAME_MAX + 18)

#if defined( __X_ONLY__) || defined( DIGTOOLS)
#define BUFFERSIZE   128
#else // !__X_ONLY__
#define BUFFERSIZE  4096
#endif // __X_ONLY__

int unpackable;  /* global, set in io.c */
ulong compsize, origsize;  /* global */
uchar buffer[BUFFERSIZE];

static uchar header[HEADERSIZE_MAX + 1];
static ushort headersize, method;
static uchar file_type, flag_sec;
static ushort day_hour_min, year_mon;
#define filename ((char *)&header[18])
static ushort file_crc, header_crc;

static ushort namelen;

#ifdef DIGTOOLS
static void clean_statics ( void)
{
   memset( header, 0, sizeof( header));
   headersize   = method     = 0;
   file_type    = flag_sec   = ' ';
   day_hour_min = year_mon   = 0;
   file_crc     = header_crc = 0;
   namelen      = 0;
}

void cleanup ( void)
{
   unpackable   = compsize   = origsize = 0;
   memset( buffer, 0, sizeof( buffer));

   clean_statics();
   clean_huf();
}
#endif

#ifndef __X_ONLY__
static char *temp_name;

static uint ratio (ulong a, ulong b)  /* [(1000a + [b/2]) / b] */
{
   int i;

   for (i = 0; i < 3; i++)
      if (a <= ULONG_MAX / 10) a *= 10;  else b /= 10;
   if ((ulong)(a + (b >> 1)) < a) {  a >>= 1;  b >>= 1;  }
   if (b == 0) return 0;
   return (uint)((a + (b >> 1)) / b);
}

static void putword (ushort x, FILE *f)
{
   fputc(x & 0xFF, f);
   fputc(x >> BYTESIZE, f);
}
#endif // __X_ONLY__

ushort getword (FILE *f)
{
   ushort x;

   x = fgetc(f);
   return x + ((uint)fgetc(f) << BYTESIZE);
}

#ifndef __X_ONLY__
static void put_to_header (int i, ushort x)
{
   header[i]     = (uchar)(x & 0xFF);
   header[i + 1] = (uchar)(x >> BYTESIZE);
}
#endif // __X_ONLY__

ushort headerword (int i)
{
   return header[i] + ((ushort)header[i + 1] << BYTESIZE);
}

static int read_header (void)
{
   ushort extheadersize;

   headersize = getword(arcfile);
   if (headersize == 0) return 0;  /* end of archive */
   if (headersize > HEADERSIZE_MAX) error("Bad header");
   crc = 0xFFFFU;
   fread_crc(header, headersize, arcfile);
   header_crc = getword(arcfile);
   if ((crc ^ 0xFFFFU) != header_crc) error("Header CRC error");
   method = headerword(0);
   file_type = header[2];
   flag_sec = header[3];
   day_hour_min = headerword(4);
   year_mon = headerword(6);
   compsize = headerword(8)  + ((ulong)headerword(10) << (2 * BYTESIZE));
   origsize = headerword(12) + ((ulong)headerword(14) << (2 * BYTESIZE));
   file_crc = headerword(16);
   namelen = headersize - 18;
   filename[namelen] = '\0';
   while ((extheadersize = getword(arcfile)) != 0)
      fseek(arcfile, extheadersize + 2, SEEK_CUR);
   return 1;  /* success */
}

#ifndef __X_ONLY__
static void set_header (void)
{
   time_t t;
   struct tm *tp;

   t = time(NULL);  tp = localtime(&t);
   flag_sec = tp->tm_sec;
   if      (tp->tm_isdst == 0) flag_sec +=  64;
   else if (tp->tm_isdst >  0) flag_sec += 128;
   /* tp = gmtime(&t); --> flag_sec +=  64 + 128; */
   day_hour_min =
      (tp->tm_mday << 11) + (tp->tm_hour << 6) + tp->tm_min;
   year_mon = (tp->tm_year << 4) + tp->tm_mon;
   file_type = 0;  /* binary */

   put_to_header( 0, method);
   header[2] = file_type;  header[3] = flag_sec;
   put_to_header( 4, day_hour_min);
   put_to_header( 6, year_mon);
   put_to_header( 8, (ushort)compsize & 0xFFFFU);
   put_to_header(10, (ushort)(compsize >> 16));
   put_to_header(12, (ushort)origsize & 0xFFFFU);
   put_to_header(14, (ushort)(origsize >> 16));
   put_to_header(16, file_crc);
}

static void write_header (void)
{
   putword(headersize, outfile);
   crc = 0xFFFFU;
   fwrite_crc(header, headersize, outfile);
   putword(crc ^ 0xFFFFU, outfile);
   putword(0, outfile);  /* no ext header */
}
#endif // __X_ONLY__

static void skip (void)
{
   fseek(arcfile, compsize, SEEK_CUR);
}

#ifndef __X_ONLY__
static void copy (void)
{
   uint n;

   write_header();
   while (compsize != 0) {
      n = (uint)((compsize > BUFFERSIZE) ? BUFFERSIZE : compsize);
      if (fread ((char *)buffer, 1, n, arcfile) != n)
         error("Can't read");
      if (fwrite((char *)buffer, 1, n, outfile) != n)
         error("Can't write");
      compsize -= n;
   }
}

static void store (void)
{
   uint n;

   origsize = 0;
   crc = 0xFFFFU;
   while ((n = fread((char *)buffer, 1, BUFFERSIZE, infile)) != 0) {
      fwrite_crc(buffer, n, outfile);  origsize += n;
   }
   compsize = origsize;
}

static int add (int replace_flag)
{
   long headerpos, arcpos;
   uint r;
   struct find_t ffblk;
   int done, files_added = 0;
   char drive[_MAX_DRIVE];
   char dir  [_MAX_DIR];

   _splitpath( filename, drive, dir, NULL, NULL);
   done = _dos_findfirst( filename, _A_NORMAL, &ffblk);
   if( done) return 0; /* nothing to do, so failure */
   while (!done) {
      sprintf(filename, "%s%s%s", drive, dir, ffblk.name);
      if ((infile = fopen(filename, "rb")) == NULL) {
         #ifndef DIGTOOLS
         printf("Can't open %s\n", filename);
         #endif
         return 0;  /* failure */
      }
      if (replace_flag) {
         #ifndef DIGTOOLS
         printf("Replacing %s ", filename);
         #endif
         skip();
      } else {
         #ifndef DIGTOOLS
         printf("Adding %s ", filename);
         #endif
      }
      headerpos = ftell(outfile);
      namelen = strlen(filename);
      headersize = 18 + namelen;
      method = 1;
      write_header();  /* temporarily */
      arcpos = ftell(outfile);
      origsize = compsize = 0;  unpackable = 0;
      crc = 0xFFFFU;  encode();
      if (unpackable) {
         method = 0;  /* store */
         rewind(infile);
         fseek(outfile, arcpos, SEEK_SET);
         store();
      }
      file_crc = crc ^ 0xFFFFU;
      fclose(infile);
      set_header();
      fseek(outfile, headerpos, SEEK_SET);
      write_header();
      fseek(outfile, 0L, SEEK_END);
      #ifndef DIGTOOLS
      r = ratio(compsize, origsize);
      printf(" %d.%d%%\n", r / 10, r % 10);
      #endif
      files_added++;
      // find next file (only when wildcards are used !)
      done = _dos_findnext(&ffblk);
   }
   return files_added;  /* success */
}
#endif // __X_ONLY__

static void unstore (void)
{
   uint n;

   crc = 0xFFFFU;
   while (compsize != 0) {
      n = (uint)((compsize > BUFFERSIZE) ? BUFFERSIZE : compsize);
      if (fread((char *)buffer, 1, n, arcfile) != n)
         error("Can't read");
      fwrite_crc(buffer, n, outfile);
      #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
      if (outfile != stdout) putc('.', stdout);
      #endif
      compsize -= n;
   }
}

int get_line (char *s, int n)
{
   int i, c;

   i = 0;
   while ((c = getchar()) != EOF && c != '\n')
      if (i < n) s[i++] = (char)c;
   s[i] = '\0';
   return i;
}

#pragma argsused
static void extract (int to_file)
{
   #if defined( __X_ONLY__) || defined( DIGTOOLS)
   if( ( outfile = fopen( filename, "wb")) == NULL)
      return;
   #else
   if (to_file) {
      while( ( outfile = fopen( filename, "wb")) == NULL)
      {
         printf("Can't open %s\nnew filename: ", filename);
         if (get_line(filename, FNAME_MAX) == 0) {
            printf("Not extracted\n");
            skip();  return;
         }
         namelen = strlen(filename);
      }
      printf("Extracting %s ", filename);
   } else {
      outfile = stdout;
      printf("===== %s =====\n", filename);
   }
   #endif
   crc = 0xFFFFU;
   if      (method == 1) decode();
   else if (method == 0) unstore();
   else {
      #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
      printf("Unknown method: %u\n", method);
      #endif
      skip();
   }
   if (to_file) fclose(outfile);  else outfile = NULL;
   #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
   printf("\n");
   #endif
   if ((crc ^ 0xFFFFU) != file_crc)
   {
      #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
      printf("CRC error\n");
      #endif
   }
}

#ifndef __X_ONLY__
static void list_start (void)
{
   printf("Filename     Mode  Original Compressed Ratio"
         "  Archived date/time  CRC Method\n");
}

static void list (void)
{
   uint r;
   char dst[4] = { ' ', 'N', 'D', 'U' };
      /* Local(no DST info), Local(not DST), Local(DST), UTC */
   char mode[2] = { 'B', 'T' };  /* Binary, Text */

   printf("%-14s", filename);
   if (namelen > 14) printf("\n              ");
   r = ratio(compsize, origsize);
   printf(" %c %10lu %10lu %u.%3u %4u-%02u-%02u %02u:%02u:%02u%c %04X %5u\n",
      mode[file_type & 1], origsize, compsize, r / 1000, r % 1000,
      (year_mon >> 4) + 1900, (year_mon & 15) + 1,
      day_hour_min >> 11, (day_hour_min >> 6) & 31, day_hour_min & 31,
      flag_sec & 63, dst[flag_sec >> 6],
      file_crc, method);
}

static int match (char *s1, char *s2)
{
   for ( ; ; ) {
      while (*s2 == '*' || *s2 == '?') {
         if (*s2++ == '*')
            while (*s1 && *s1 != *s2) s1++;
         else if (*s1 == 0)
            return 0;
         else s1++;
      }
      if (*s1 != *s2) return 0;
      if (*s1 == 0  ) return 1;
      s1++;  s2++;
   }
}

static int search (int argc, char *argv[])
{
   int i;

   if (argc == 3) return 1;
   for (i = 3; i < argc; i++)
      if (match(filename, argv[i])) return 1;
   return 0;
}

static void exitfunc (void)
{
   fclose(outfile);  remove(temp_name);
}
#endif // __X_ONLY__

#ifdef __X_ONLY__
int DecompressFile ( char *cpFileName)
{
   // 20040326 avn - decompress B021 automatically
   int cmd = 'X', done = 0;

   init_table();
   strcpy( filename, cpFileName);
   arcfile = fopen( filename, "rb");
   make_crctable();
   while (! done && read_header())
      extract(cmd == 'X');
   fclose( arcfile);
   free_table();
   return EXIT_SUCCESS;
}
#else
#ifdef DIGTOOLS
int ar ( int cmd, char *archive_file, char *file2archive)
{
   int nfiles;

   cleanup();
#else
int main (int argc, char *argv[])
{
   char *archive_file, *file2archive;
   int cmd, nfiles;

   cmd = toupper(argv[1][0]);
   if (argv[1][1] || strchr("AXRDPL?", cmd) == NULL)
      error("Invalid command: %s", argv[1]);
   if (argc < 3 + (strchr("AD", cmd) != NULL)) error(usage);
   nfiles = argc - 3;
   archive_file = argv[2];
#endif
   int i, j, count, added, found, done;
   char *temp_filename;
   char drive[_MAX_DRIVE];
   char dir  [_MAX_DIR];

   count = 0;
   init_table();
   arcfile = fopen(archive_file, "rb");
   if (arcfile == NULL && cmd != 'A')
      error("Can't open %s", archive_file);
   temp_name = NULL;
   if (strchr("ARD", cmd)) {
      temp_name = ( char *)malloc( _MAX_PATH + 1);
      temp_filename = tmpnam(NULL);
      _splitpath( archive_file, drive, dir, NULL, NULL);
      sprintf( temp_name, "%s%s%s", drive, dir, temp_filename);
      outfile = fopen(temp_name, "wb");
      if (outfile == NULL)
         error("Can't open temporary file");
      atexit(exitfunc);
   }
   make_crctable();  count = done = 0;
   if (cmd == 'A') {
      #ifdef DIGTOOLS
      strcpy( filename, file2archive);
      count += add(0);
      #else  // !DIGTOOLS
      for (i = 3; i < argc; i++) {
         for (j = 3; j < i; j++)
            if (strcmp(argv[j], argv[i]) == 0) break;
         if (j == i) {
            strcpy(filename, argv[i]);
            added = add(0);
            if( added) count += added;  else argv[i][0] = 0;
         } else nfiles--;
      }
      #endif // DIGTOOLS
      if (count == 0 || arcfile == NULL) done = 1;
   }
   while (! done && read_header()) {
      #ifdef DIGTOOLS
      // If file2archive is provided, find it,
      // otherwise process ALL files !
      found = file2archive
            ? match( filename, file2archive)
            : 1;
      #else
      found = search( argc, argv);
      #endif // !DIGTOOLS
      switch (cmd) {
      case 'R':
         if (found) {
            added = add(1);
            if( added) count += added;  else copy();
         } else copy();
         break;
      case 'A':  case 'D':
         if (found) {
            count += (cmd == 'D');  skip();
         } else copy();
         break;
      case 'X':  case 'P':
         if (found) {
            extract(cmd == 'X');
            if (++count == nfiles) done = 1;
         } else skip();
         break;
      case 'L':
         if (found) {
            if (count == 0) list_start();
            list();
            if (++count == nfiles) done = 1;
         }
         skip();  break;
      }
   }
   fclose( arcfile);
   if (temp_name != NULL && count != 0) {
      putword(0, outfile);  /* end of archive */
      if (ferror(outfile) || fclose(outfile) == EOF)
         error("Can't write");
      remove( archive_file);
      rename( temp_name, archive_file);
   }
   #ifndef DIGTOOLS
   printf("  %d files\n", count);
   #endif
   free_table();
   return EXIT_SUCCESS;
}
#endif // __X_ONLY__

//</source>
//<!-- eof -->
