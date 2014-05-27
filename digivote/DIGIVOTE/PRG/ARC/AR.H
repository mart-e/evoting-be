/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ar.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   AR.H  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 19 2004 11:36:42  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives_dummy/Digivote/Prg/ARC/AR.H-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/02/07 11:32:03 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

/***********************************************************
   ar.h
***********************************************************/
#include <stdio.h>
#include <limits.h>
typedef unsigned char  uchar;   /*  8 bits or more */
typedef unsigned int   uint;    /* 16 - 32 bits or more */
typedef unsigned short ushort;  /* 16 bits or more */
typedef unsigned long  ulong;   /* 32 bits or more */

/* ar.cpp */

extern int unpackable;
extern ulong origsize, compsize;

/* io.cpp */

extern FILE *arcfile, *infile, *outfile;
extern ushort crc, bitbuf;

void error(char *fmt, ...);
void make_crctable(void);
void fillbuf(uchar n);
ushort getbits(uchar n);
/* void putbit(uchar bit); */
void putbits(uchar n, ushort x);
int fread_crc(uchar *p, int n, FILE *f);
void fwrite_crc(uchar *p, int n, FILE *f);
void init_getbits(void);
void init_putbits(void);

/* slide.cpp */

#define DICBIT    12    /* 12 or 13 */
#define DICSIZ (1U << DICBIT)
#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256    /* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3    /* choose optimal value */

void encode(void);
void decode(void);

/* huf.cpp */

#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
   /* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define USHRT_BIT (CHAR_BIT * sizeof(ushort))

extern short left[], right[];

void encode_start(void);
void decode_start(void);
ushort decode_c(void);
ushort decode_p(void);
void output(ushort c, ushort p);
void encode_end(void);

void init_table( void);
void free_table( void);

/* maketbl.cpp */

void make_table(short nchar, uchar bitlen[],
            short tablebits, ushort table[]);

/* maketree.cpp */

short make_tree(short nparm, ushort freqparm[], uchar lenparm[]);

#ifdef DIGTOOLS
void clean_huf( void);
void clean_io( void);
void clean_maketbl( void);
void clean_maketree( void);
void clean_slide( void);
#endif

//</source>
//<!-- eof -->
