/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="rawrite2.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:05 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>rawrite2.c</fileName>
  <creationDate/>
  <rights></rights>
  <author>JRU</author>
  <description/>
  <history/>
  <comment>

 rawrite.c Write a binary image to a diskette.
Originally by Mark Becker

 Heavily modified by Guy Helmer (4/29/91) to improve performance and
 add features.

 Compiling:
Appeared to have been written for Turbo C, so I've surrounded
compiler-specific code in "#if defined(__TURBOC__)" and added
code in the "#else" clauses for Microsoft C.  Under MSC, code
should be compiled in the Large memory model.

 Usage:
MS-DOS prompt> RAWRITE
and follow the prompts, -or-

MS-DOS prompt> RAWRITE [-f 'file'] [-d 'drive'] [-n(owait)] [-h(elp)]
where:-f 'file' - name of disk image file
-d 'drive' - diskette drive to use, must be A or B
-n - don't prompt for user to insert diskette
-h - print usage information to stdout

History
-------

  1.0-Initial release
  1.1-Beta test (fixing bugs)4/5/91
  Some BIOS's don't like full-track writes.
  1.101-Last beta release.4/8/91
  Fixed BIOS full-track write by only
writing 3 sectors at a time.
  1.2-Final code and documentation clean-ups.4/9/91
  2.0 (ghelmer@dsuvax.dsu.edu)4/30/92
  -Performance improvements
Added command line options
Now compiles under Microsoft C (version 5.1)

Version 2.0 Copyright 1992 Guy Helmer
    Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both the above copyright notice and this permission notice appear
in supporting documentation.  This software is made available "as is",
and
GUY HELMER DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, WITH
REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
AND IN NO EVENT SHALL GUY HELMER BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
(INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  </comment>
</comments>
<header>*/

/*<includes>*/
#include <bios.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hlc.h"
#include "genlib.h"
/*</includes>*/

#define SECTORSIZE 512
#define MAXPATH   _MAX_PATH

#define RESET     _DISK_RESET
#define LAST      _DISK_STATUS
#define READ      _DISK_READ
#define WRITE     _DISK_WRITE
#define VERIFY    _DISK_VERIFY
#define FORMAT    _DISK_FORMAT

static unsigned
biosdisk( unsigned, unsigned, unsigned, unsigned, unsigned, unsigned, void *);

/*</header>
<body>*/

/*<function id="biosdisk">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
static unsigned
biosdisk ( service, drive, head, track, sector, nsectors, buffer)
     unsigned service;
     unsigned drive;
     unsigned head;
     unsigned track;
     unsigned sector;
     unsigned nsectors;
     void    *buffer;
{
  struct diskinfo_t diskinfo;

  diskinfo.drive = drive;
  diskinfo.head = head;
  diskinfo.track = track;
  diskinfo.sector = sector;
  diskinfo.nsectors = nsectors;
  diskinfo.buffer = buffer;

  return ( _bios_disk( service, &diskinfo) >> 8);
}
/*</code></function>*/


static char gcLibName[]  = "RaWrite2";
int done;

/*
 Identify what kind of diskette is installed in the specified drive.
 Return the number of sectors per track assumed as follows:
 9-360 K and 720 K 5.25".
15-1.2 M HD5.25".
18-1.44 M3.5".
*/
/*int
nsects( int ( drive))
{
  static int nsect[] = {18, 15, 9};

  char buffer[SECTORSIZE];
  int i, status;
*/
/*
 Read sector 1, head 0, track 0 to get the BIOS running.
*/
/*  biosdisk( RESET, drive, 0, 0, 0, 0, buffer);
  status = biosdisk( READ, drive, 0, 10, 1, 1, buffer);
  if( status == 0x06)*//* Door signal change?*/
/*    status = biosdisk( READ, drive, 0, 0, 1, 1, buffer);

  for( i = 0; i < sizeof( nsect) / sizeof( int); ++i) {
      biosdisk( RESET, drive, 0, 0, 0, 0, buffer);
      status = biosdisk( READ, drive, 0, 0, nsect[i], 1, buffer);
      if( status == 0x06)
          status = biosdisk( READ, drive, 0, 0, nsect[i], 1, buffer);
      if( status == 0x00)
          break;
  }

  if(i == sizeof( nsect)/sizeof( int)) {
      strcpy( gcMessage, "Can't figure out how many sectors/track for this diskette.");
      giStatus = -8010;
  }
  return( nsect[i]);
}
*/

/*<function id="rawrite">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
int
rawrite ( argc, argv)
     int argc;
     char *argv[];
{
  char fname[MAXPATH], drvtmp[4];
  char buffer[9216];
  int count, fdin, drive, head, track, status, spt, buflength, ns, i;
  unsigned long addrtmp;

  /* Init error handling */
  InitFunction( gcLibName, "rawrite", "fileok", TRUE);
  if( giStatus == -1) return( 0);

  done = 0;

  /* 1st parameter is the name of the image file */
  strncpy( fname, argv[1], sizeof( fname) - 1);
  fname[sizeof( fname) - 1] = '\0';

  /* 2nd parameter is the destination drive letter */
  drvtmp[0] = *argv[2];
  drvtmp[1] = '\0';

  _fmode = O_BINARY;
  if(( fdin = open( fname, O_RDONLY)) < 0) {
      giStatus = 10;
      goto error;
  }
  if( done) {
      giStatus = 20;
      goto error;
  }
  strupr( drvtmp);
  if( strlen(drvtmp) == 0 || drvtmp[0] < 'A' || drvtmp[0] > 'B') {
      sprintf(gcMessage, "Drive was '%s'; must be A or B.", drvtmp);
      giStatus = 30;
      goto error;
  }
  if( done) {
      giStatus = 40;
      goto error;
  }
  drive = drvtmp[0] - 'A';
  if( done) {
      giStatus = 50;
      goto error;
  }
/*
 * Determine number of sectors per track and allocate buffers.
 */
  spt = 18; //nsects(drive);
  if( giStatus != 0) goto error;
  buflength = sizeof( buffer); //spt * SECTORSIZE;
 /* if( ( buffer = malloc( buflength)) == NULL) {
      strcpy(gcMessage, "Couldn't allocate track buffer.");
      giStatus = 60;
      goto error;
  }
*/
/*
 * Start writing data to diskette until there is no more data to write.
 */

  head = track = 0;
  while( ( count = read( fdin, buffer, buflength)) > 0 && !done) {
      status = biosdisk(WRITE, drive, head, track, 1, spt, buffer);
      if( status != 0) {
          switch( status) {
              case 0x00: strcpy( gcMessage, "Operation Successful");                     break;
              case 0x01: strcpy( gcMessage, "Bad command");                              break;
              case 0x02: strcpy( gcMessage, "Address mark not found");                   break;
              case 0x03: strcpy( gcMessage, "Attempt to write on write-protected disk"); break;
              case 0x04: strcpy( gcMessage, "Sector not found");                         break;
              case 0x05: strcpy( gcMessage, "Reset failed (hard disk)");                 break;
              case 0x06: strcpy( gcMessage, "Disk changed since last operation");        break;
              case 0x07: strcpy( gcMessage, "Drive parameter activity failed");          break;
              case 0x08: strcpy( gcMessage, "DMA overrun");                              break;
              case 0x09: strcpy( gcMessage, "Attempt to DMA across 64K boundary");       break;
              case 0x0A: strcpy( gcMessage, "Bad sector detected");                      break;
              case 0x0B: strcpy( gcMessage, "Bad track detected");                       break;
              case 0x0C: strcpy( gcMessage, "Unsupported track");                        break;
              case 0x10: strcpy( gcMessage, "Bad CRC/ECC on disk read");                 break;
              case 0x11: strcpy( gcMessage, "CRC/ECC corrected data error");             break;
              case 0x20: strcpy( gcMessage, "Controller has failed");                    break;
              case 0x40: strcpy( gcMessage, "Seek operation failed");                    break;
              case 0x80: strcpy( gcMessage, "Attachment failed to respond");             break;
              case 0xAA: strcpy( gcMessage, "Drive not ready (hard disk only");          break;
              case 0xBB: strcpy( gcMessage, "Undefined error occurred (hard disk only)");break;
              case 0xCC: strcpy( gcMessage, "Write fault occurred");                     break;
              case 0xE0: strcpy( gcMessage, "Status error");                             break;
              case 0xFF: strcpy( gcMessage, "Sense operation failed");                   break;
          }
          giStatus = -status - 9000;
          goto error;
      }
      if( ( head = ( head + 1) & 1) == 0) ++track;
  }
  if( done) {
      giStatus = 70;
      goto error;
  }
  if( eof( fdin)) {
      biosdisk( READ, drive, 0, 0, 1, 1, buffer); /* Retract head */
  }

error:
//  free( buffer);
  if( fdin) close( fdin);
  if( giStatus) {
      giStatus -= 1000; /* customize error code */
      RaiseError( FALSE);
  }
  else {
      updSharedVar();
  }

  return( 0);
}
/*</code></function>

</body>
</source>
<!-- eof -->*/
