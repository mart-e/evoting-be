/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="maketree.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   MAKETREE.CPP  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 12:00:26  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives_dummy/Digivote/Prg/ARC/MAKETREE.CPP-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/02/07 11:32:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

/***********************************************************
   maketree.c -- make Huffman tree
***********************************************************/
#include <mem.h>     // memset()
#include <string.h>     // memset()

#include "..\arc\ar.h"

static short n, heapsize, heap[NC + 1];
static ushort *freq;
static uchar *len;

#ifdef DIGTOOLS
static void clean_statics ( void)
{
   n = heapsize = 0;
   memset( heap, 0, sizeof( heap));
   freq= NULL;
   len = NULL;
}

void clean_maketree ( void)
{
   clean_statics();
   clean_slide();
}
#endif

static void make_len (short i)  /* call with i = root */
{
   static uchar depth = 0;

   if (i < n) len[i] = depth;
   else {
      depth++;
      make_len(left [i]);
      make_len(right[i]);
      depth--;
   }
}

static void downheap (short i)
   /* priority queue; send i-th entry down heap */
{
   short j, k;

   k = heap[i];
   while ((j = 2 * i) <= heapsize) {
      if (j < heapsize && freq[heap[j]] > freq[heap[j + 1]])
         j++;
      if (freq[k] <= freq[heap[j]]) break;
      heap[i] = heap[j];  i = j;
   }
   heap[i] = k;
}

short make_tree (short nparm, ushort freqparm[], uchar lenparm[])
   /* make tree, calculate len[], return root */
{
   short i, j, k, avail;

   n = nparm;  freq = freqparm;  len = lenparm;
   avail = n;  heapsize = 0;  heap[1] = 0;
   for (i = 0; i < n; i++) {
      len[i] = 0;
      if (freq[i]) heap[++heapsize] = i;
   }
   if (heapsize < 2) return heap[1];
   for (i = heapsize / 2; i >= 1; i--)
      downheap(i);  /* make priority queue */
   do {  /* while queue has at least two entries */
      i = heap[1];  /* take out least-freq entry */
      heap[1] = heap[heapsize--];
      downheap(1);
      j = heap[1];  /* next least-freq entry */
      k = avail++;  /* generate new node */
      freq[k] = freq[i] + freq[j];
      heap[1] = k;  downheap(1);  /* put into queue */
      left[k] = i;  right[k] = j;
   } while (heapsize > 1);
   for (i = 0; i < n; i++) len[i] = 0;
   make_len(k);
   return k;  /* return root */
}

//</source>
//<!-- eof -->
