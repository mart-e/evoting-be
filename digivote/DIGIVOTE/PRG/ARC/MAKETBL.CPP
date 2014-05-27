/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="maketbl.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   MAKETBL.CPP  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 11:59:56  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives_dummy/Digivote/Prg/ARC/MAKETBL.CPP-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/02/07 11:32:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

/***********************************************************
   maketbl.c
***********************************************************/
#include "..\arc\ar.h"

static short c, _n, tblsiz, _len, depth, maxdepth, _avail;
static ushort codeword, bit;
static ushort *tbl;
static uchar *blen;

#ifdef DIGTOOLS
static void clean_statics ( void)
{
   c = _n = tblsiz = _len = depth = maxdepth = _avail = 0;
   codeword = bit = 0;
   tbl = NULL;
   blen = NULL;
}

void clean_maketbl ( void)
{
   clean_statics();
   clean_maketree();
}
#endif

static short mktbl (void)
{
   short i;

   if (_len == depth) {
      while (++c < _n)
         if (blen[c] == _len) {
            i = codeword;  codeword += bit;
            if (codeword > tblsiz) error("Bad table (1)");
            while (i < codeword) tbl[i++] = c;
            return c;
         }
      c = -1;  _len++;  bit >>= 1;
   }
   depth++;
   if (depth < maxdepth) {
      (void) mktbl();  (void) mktbl();
   } else if (depth > USHRT_BIT) {
      error("Bad table (2)");
   } else {
      if ((i = _avail++) >= 2 * _n - 1) error("Bad table (3)");
      left[i] = mktbl();  right[i] = mktbl();
      if (codeword >= tblsiz) error("Bad table (4)");
      if (depth == maxdepth) tbl[codeword++] = i;
   }
   depth--;
   return i;
}

void make_table (short nchar, uchar bitlen[],
            short tablebits, ushort table[])
{
   _n = _avail = nchar;  blen = bitlen;  tbl = table;
   tblsiz = 1U << tablebits;  bit = tblsiz / 2;
   maxdepth = tablebits + 1;
   depth = _len = 1;  c = -1;  codeword = 0;
   (void) mktbl();  /* left subtree */
   (void) mktbl();  /* right subtree */
   if (codeword != tblsiz) error("Bad table (5)");
}

//</source>
//<!-- eof -->
