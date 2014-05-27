/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="slide.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   SLIDE.CPP  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 12:01:30  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives_dummy/Digivote/Prg/ARC/SLIDE.CPP-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/02/07 11:32:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

/***********************************************************
   slide.c -- sliding dictionary with percolating update
***********************************************************/
#include "..\arc\ar.h"
#include <stdlib.h>
#include <string.h>  /* memmove() */

#define PERCOLATE  1

#define NIL        0
#define MAX_HASH_VAL (3 * DICSIZ + (DICSIZ / 512 + 1) * UCHAR_MAX)

#ifdef __X_ONLY__
static uchar *text;
#else
typedef short  node;

static uchar *text, *childcount;
static node pos, matchpos, avail,
   *position, *parent, *prev, *next = NULL;
static int remainder, matchlen;

#if MAXMATCH <= (UCHAR_MAX + 1)
   static uchar *level;
#else
   static ushort *level;
#endif

#ifdef DIGTOOLS
static void clean_statics ( void)
{
   pos = matchpos, avail = 0;
   text = childcount = NULL;
   position = parent = prev = next = NULL;
   remainder = matchlen = 0;
   level = NULL;
}

void clean_slide ( void)
{
   clean_statics();
}
#endif

static void init_slide (void)
{
   node i;

   if (next == NULL) {
      level      = ( unsigned char*)malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*level));
      childcount = ( unsigned char*)malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*childcount));
      #if PERCOLATE
        position = ( short*)malloc((DICSIZ + UCHAR_MAX + 1) * sizeof(*position));
      #else
        position = ( short*)malloc(DICSIZ * sizeof(*position));
      #endif
      parent     = ( short*)malloc(DICSIZ * 2 * sizeof(*parent));
      prev       = ( short*)malloc(DICSIZ * 2 * sizeof(*prev));
      next       = ( short*)malloc((MAX_HASH_VAL + 1) * sizeof(*next));
      if (next == NULL) error("Out of memory.");
   }
   for (i = DICSIZ; i <= DICSIZ + UCHAR_MAX; i++) {
      level[i] = 1;
      #if PERCOLATE
         position[i] = NIL;  /* sentinel */
      #endif
   }
   for (i = DICSIZ; i < DICSIZ * 2; i++) parent[i] = NIL;
   avail = 1;
   for (i = 1; i < DICSIZ - 1; i++) next[i] = i + 1;
   next[DICSIZ - 1] = NIL;
   for (i = DICSIZ * 2; i <= MAX_HASH_VAL; i++) next[i] = NIL;
}

#define HASH(p, c) ((p) + ((c) << (DICBIT - 9)) + DICSIZ * 2)

node child (node q, uchar c)
   /* q's child for character c (NIL if not found) */
{
   node r;

   r = next[HASH(q, c)];
   parent[NIL] = q;  /* sentinel */
   while (parent[r] != q) r = next[r];
   return r;
}

void makechild (node q, uchar c, node r)
   /* Let r be q's child for character c. */
{
   node h, t;

   h = HASH(q, c);
   t = next[h];  next[h] = r;  next[r] = t;
   prev[t] = r;  prev[r] = h;
   parent[r] = q;  childcount[q]++;
}

void split (node oldN)
{
   node newN, t;

   newN = avail;  avail = next[newN];  childcount[newN] = 0;
   t = prev[oldN];  prev[newN] = t;  next[t] = newN;
   t = next[oldN];  next[newN] = t;  prev[t] = newN;
   parent[newN] = parent[oldN];
   level[newN] = matchlen;
   position[newN] = pos;
   makechild(newN, text[matchpos + matchlen], oldN);
   makechild(newN, text[pos + matchlen], pos);
}

static void insert_node (void)
{
   node q, r, j, t;
   uchar c, *t1, *t2;

   if (matchlen >= 4) {
      matchlen--;
      r = (matchpos + 1) | DICSIZ;
      while ((q = parent[r]) == NIL) r = next[r];
      while (level[q] >= matchlen) {
         r = q;  q = parent[q];
      }
      #if PERCOLATE
         t = q;
         while (position[t] < 0) {
            position[t] = pos;  t = parent[t];
         }
         if (t < DICSIZ) position[t] = -pos;
      #else
         t = q;
         while (t < DICSIZ) {
            position[t] = pos;  t = parent[t];
         }
      #endif
   } else {
      q = text[pos] + DICSIZ;  c = text[pos + 1];
      if ((r = child(q, c)) == NIL) {
         makechild(q, c, pos);  matchlen = 1;
         return;
      }
      matchlen = 2;
   }
   for ( ; ; ) {
      if (r >= DICSIZ) {
         j = MAXMATCH;  matchpos = r;
      } else {
         j = level[r];
         matchpos = position[r];
         if (matchpos < 0) matchpos = -matchpos;
      }
      if (matchpos >= pos) matchpos -= DICSIZ;
      t1 = &text[pos + matchlen];  t2 = &text[matchpos + matchlen];
      while (matchlen < j) {
         if (*t1 != *t2) {  split(r);  return;  }
         matchlen++;  t1++;  t2++;
      }
      if (matchlen == MAXMATCH) break;
      position[r] = pos;
      q = r;
      if ((r = child(q, *t1)) == NIL) {
         makechild(q, *t1, pos);  return;
      }
      matchlen++;
   }
   t = prev[r];  prev[pos] = t;  next[t] = pos;
   t = next[r];  next[pos] = t;  prev[t] = pos;
   parent[pos] = q;  parent[r] = NIL;
   next[r] = pos;  /* special use of next[] */
}

static void delete_node (void)
{
   #if PERCOLATE
      node q, r, s, t, u;
   #else
      node r, s, t, u;
   #endif

   if (parent[pos] == NIL) return;
   r = prev[pos];  s = next[pos];
   next[r] = s;  prev[s] = r;
   r = parent[pos];  parent[pos] = NIL;
   if (r >= DICSIZ || --childcount[r] > 1) return;
   t = position[r];
   #if PERCOLATE
      if (t < 0) t = -t;
   #endif
   if (t >= pos) t -= DICSIZ;
   #if PERCOLATE
      s = t;  q = parent[r];
      while ((u = position[q]) < 0) {
         u = -u;  if (u >= pos) u -= DICSIZ;
         if (u > s) s = u;
         position[q] = (s | DICSIZ);  q = parent[q];
      }
      if (q < DICSIZ) {
         if (u >= pos) u -= DICSIZ;
         if (u > s) s = u;
         position[q] = -(s | DICSIZ);
      }
   #endif
   s = child(r, text[t + level[r]]);
   t = prev[s];  u = next[s];
   next[t] = u;  prev[u] = t;
   t = prev[r];  next[t] = s;  prev[s] = t;
   t = next[r];  prev[t] = s;  next[s] = t;
   parent[s] = parent[r];  parent[r] = NIL;
   next[r] = avail;  avail = r;
}

void get_next_match (void)
{
   int n;

   remainder--;
   if (++pos == DICSIZ * 2) {
      memmove(&text[0], &text[DICSIZ], DICSIZ + MAXMATCH);
      n = fread_crc(&text[DICSIZ + MAXMATCH], DICSIZ, infile);
      remainder += n;  pos = DICSIZ;
      #ifndef DIGTOOLS
      putc('.', stdout);
      #endif
   }
   delete_node();  insert_node();
}

void encode (void)
{
   int lastmatchlen;
   node lastmatchpos;

   text = ( unsigned char*)malloc(DICSIZ * 2 + MAXMATCH);
   if (text == NULL) error("Out of memory.");
   init_slide();  encode_start();
   remainder = fread_crc(&text[DICSIZ], DICSIZ + MAXMATCH, infile);
   #ifndef DIGTOOLS
   putc('.', stdout);
   #endif
   matchlen = 0;
   pos = DICSIZ;  insert_node();
   if (matchlen > remainder) matchlen = remainder;
   while (remainder > 0 && ! unpackable) {
      lastmatchlen = matchlen;  lastmatchpos = matchpos;
      get_next_match();
      if (matchlen > remainder) matchlen = remainder;
      if (matchlen > lastmatchlen || lastmatchlen < THRESHOLD)
         output(text[pos - 1], 0);
      else {
         output(lastmatchlen + (UCHAR_MAX + 1 - THRESHOLD),
               (pos - lastmatchpos - 2) & (DICSIZ - 1));
         while (--lastmatchlen > 0)
            get_next_match();
         if (matchlen > remainder) matchlen = remainder;
      }
   }
   encode_end();  free(text);
}
#endif // __X_ONLY__

void decode (void)
{
   int i, j, k, r, c;
   unsigned long count;

   text = ( unsigned char*)malloc(DICSIZ);
   if (text == NULL) error("Out of memory.");
   decode_start();
   count = 0;  r = 0;
   while (count < origsize) {
      c = decode_c();
      if (c <= UCHAR_MAX) {
         text[r++] = c;
         if (r == DICSIZ) {
            #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
            if (outfile != stdout) putc('.', stdout);
            #endif
            fwrite_crc(text, DICSIZ, outfile);
            r = 0;
         }
         count++;
      } else {
         j = c - (UCHAR_MAX + 1 - THRESHOLD);  count += j;
         i = (r - decode_p() - 1) & (DICSIZ - 1);
         for (k = 0; k < j; k++) {
            c = text[(i + k) & (DICSIZ - 1)];
            text[r++] = c;
            if (r == DICSIZ) {
               #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
               if (outfile != stdout) putc('.', stdout);
               #endif
               fwrite_crc(text, DICSIZ, outfile);
               r = 0;
            }
         }
      }
   }
   if (r != 0) {
      fwrite_crc(text, r, outfile);
      #if !defined( __X_ONLY__) && !defined( DIGTOOLS)
      if (outfile != stdout) putc('.', stdout);
      #endif
   }
   free(text);
}

//</source>
//<!-- eof -->
