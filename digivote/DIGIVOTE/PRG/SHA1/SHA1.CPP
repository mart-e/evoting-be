/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="sha1.cpp">
<pvcs><!-- these tags are filled out by version control during check in -->
<workFile>$Workfile:   sha1.cpp  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp/>
<archive />
<archiveTimeStamp />
<pvcsUser/>
</pvcs>
<comments>
  <fileName>sha1.cpp</fileName>
  <creationDate>09/2008</creationDate>
  <rights/>
  <author/>
  <description>
    <fr>Librairie de fonctions SHA1</fr>
    <nl>SHA1 functielibrary</nl></description>
  <history/>
  <comment>
code for the SHA-1 hashing algorithm, implemented for 8-bit byte data
by W. Gray,  walterg@ukonline.co.uk, started 7 September 2003

code derived by editing text from NIST document FIPS180-1
of 17 April 1995

NB, assumes compiler provides unsigned longs of 4 bytes
  </comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#include "..\sha1\sha1.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#ifndef NDIG
#error need NDIG defined for sha1.cpp
#endif

#if NDIG != 20
#error NDIG must be 20 for the SHA-1
#endif

#define SHA1_BUFFLEN 64


/* note, the name ROTL is taken from FIPS180-2 */
#define ROTL(n,x)  ( ((x) << (n)) | ((x) >> (32-n)) )  /* end-around shift */

/* note, the functions are taken from FIPS180-2 */
#define CH(x,y,z)  (((x) & (y)) ^ ((~x) & (z)))               /* ( 0 <= t <= 19) */
#define PAR(x,y,z)  ((x) ^ (y) ^ (z))                          /* (20 <= t <= 39) */
#define MAJ(x,y,z)  (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))  /* (40 <= t <= 59) */
/* uses PAR for (60 <= t <= 79) */

#define K1  0x5A827999                  /* ( 0 <= t <= 19) */
#define K2  0x6ED9EBA1                  /* (20 <= t <= 39) */
#define K3  0x8F1BBCDC                  /* (40 <= t <= 59) */
#define K4  0xCA62C1D6                  /* (60 <= t <= 79) */
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
void initSHA1(unsigned long []);
void makeSHA1(unsigned char [],unsigned long []);
void doSHA1block(unsigned char [],unsigned long []);

int bufpad(unsigned char [],int,unsigned long,unsigned long);
void clearbuf(unsigned char [], int, int);
void long2byte(unsigned long,unsigned char []);
void byte2long(unsigned long *,unsigned char []);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//-------------------------------------------------------- PRIVATE GLOBALS ---
//<globals>
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

void initSHA1(unsigned long H[])/*-------------------------------------
load 5 magic numbers into SHA1 intermediates
*/
{
  H[0] = 0x67452301;
  H[1] = 0xEFCDAB89;
  H[2] = 0x98BADCFE;
  H[3] = 0x10325476;
  H[4] = 0xC3D2E1F0;
}/* end initSHA1 */

void doSHA1block(unsigned char block[],unsigned long H[5])/*-----------------
do SHA-1 process on 64-byte message block, as per FIPS180-1
*/
{
unsigned long A,B,C,D,E,W[80],TEMP;
int k,t;

/* load 64 bytes into 16 longwords */
for (t=0,k=0; t<16; t++,k+=4)  byte2long(&W[t],&block[k]);

/* fill rest of buffer W */
for ( ; t<80; t++)  W[t] = ROTL(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);

A = H[0]; B = H[1]; C = H[2]; D = H[3]; E = H[4];

/* process buffer W */
for (t=0; t<80; t++)
  {
  if (t>=60)      TEMP = ROTL(5,A) + PAR(B,C,D) + E + W[t] + K4;
  else if (t>=40) TEMP = ROTL(5,A) + MAJ(B,C,D) + E + W[t] + K3;
  else if (t>=20) TEMP = ROTL(5,A) + PAR(B,C,D) + E + W[t] + K2;
  else            TEMP = ROTL(5,A) + CH(B,C,D) + E + W[t] + K1;
  E = D; D = C; C = ROTL(30,B); B = A; A = TEMP;
  }

/* accumulate intermediates */
H[0] += A; H[1] += B; H[2] += C; H[3] += D; H[4] += E;

}/* end doSHA1block */



void makeSHA1(unsigned char digest[],unsigned long h[5])/*-------------------
convert SHA-1 intermediates to final digest format
*/
{
int i,t;

/* load 5 longwords into 20 bytes */
for (i=0,t=0; i<5; i++,t+=4)  long2byte(h[i],&digest[t]);

}/* end makeSHA1 */



int bufpad(unsigned char buffer[],int buflen,
						unsigned long locount, unsigned long hicount)
/*------------------------------------------------------------------
pads a byte-buffer with 0x80, zeros and 64-bit big-endian data bit-count

written for SHA-1 but usable for other hashes

return = 1  means routine must be called again to complete
the padding, return = 0 means padding is complete
*/
{
int k;
static int flag=0;

if (flag)
  {                  /* do 2nd and 3rd parts of padding */
  clearbuf(buffer,0,buflen-8);
  long2byte(hicount,&buffer[buflen-8]);
  long2byte(locount,&buffer[buflen-4]);
  flag=0; return(flag);
  }

k=(locount>>3) % buflen;           /* index after last data byte */
/* if k=0 the block is empty or has already been processed */

if (k<=buflen-9)
  {                           /* do full 3-part padding */
  buffer[k++]=0x80;
  clearbuf(buffer,k,buflen-8);
  long2byte(hicount,&buffer[buflen-8]);
  long2byte(locount,&buffer[buflen-4]);
  flag=0; return(flag);
  }

if (k<=buflen-1)
  {                        /* do 1st and 2nd parts of padding */
  buffer[k++]=0x80;
  clearbuf(buffer,k,buflen);
  flag=1; return(flag);   /* finish padding on next call */
  }

fprintf(stderr,"this can't happen\n");
exit(1);
/* following code has no meaning, but it avoids compiler warnings */
return(0);
}/* end bufpad */



void clearbuf(unsigned char buf[], int from, int upto)/*-----------------
*/
{
int i;

if (from>=upto) return;
for (i=from; i<upto; i++) buf[i]=0;
}/* end clearbuf */



void long2byte(unsigned long w,unsigned char c[])/*--------------
convert longword to byte array with MSbyte of longword
going to first byte of array
*/
{
unsigned long n=w;

c[3]=n & 0xff;
n>>=8;  c[2]=n & 0xff;
n>>=8;  c[1]=n & 0xff;
n>>=8;  c[0]=n;
}/* end long2byte */


void byte2long(unsigned long *w,unsigned char c[])/*-----------------
convert byte array to longword with first byte of array
going to MSbyte of longword
*/
{
*w=c[0];
*w<<=8;  *w |= c[1];
*w<<=8;  *w |= c[2];
*w<<=8;  *w |= c[3];
}/* end byte2long */



//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="SHA1_stream">
<heading>
  <description><en>Digests a file</en></description>
  <parameters />
  <usage />
  <checks />
  <comment>calc digest of stream (file), assumed opened "rb" or MS-DOS binary mode</comment>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SHA1_stream(
	FILE* file,
	unsigned char digest[NDIG]
	)
// ---------------------------------------------------------------------------
{
unsigned char c,buf[SHA1_BUFFLEN];
unsigned long loco=0,hico=0,H[5];
int i,k=1;

initSHA1(H);

while (k)
  {
  for (i=0; i<SHA1_BUFFLEN; i++)
    {
    c=getc(file);
    if (feof(file)) {k=bufpad(buf,SHA1_BUFFLEN,loco,hico); break;}
    else
      {buf[i]=c;
      loco+=8;
      if (loco==0)
        {++hico; if (hico==0) {fprintf(stderr,"too much data!"); exit(1);}}
      }
    }
  doSHA1block(buf,H);
  }

makeSHA1(digest,H);
clearbuf(buf,0,SHA1_BUFFLEN);
}/* end SHA1_stream */
//</code></function>


/*<function id="SHA1_multi">
<heading>
  <description><en>Accumulates digest of bytes</en></description>
  <parameters />
  <usage />
  <checks />
  <comment>
accumulate digest of bytes from string buffer, up to 32000 bytes each
initialises on first call, terminates if called with len=0

 accumulate,               call as; SHA1_multi(string,len,digest);
 terminate, with padding,  call as; SHA1_multi("",0,digest);
  </comment>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SHA1_multi(
	unsigned char *str,
	int len,
	unsigned char digest[NDIG]
	)
// ---------------------------------------------------------------------------
{
static unsigned char buf[SHA1_BUFFLEN];
static int init=1,j;
static unsigned long loco,hico,H[5];
int k;

if (init)
  {
  initSHA1(H);
  loco=0; hico=0;
  j=0;
  init=0;
  }

if (len==0)           /* finish up */
  {
  k=1;
  while (k)
    {k=bufpad(buf,SHA1_BUFFLEN,loco,hico);
    doSHA1block(buf,H);
    }
  makeSHA1(digest,H);
  clearbuf(buf,0,SHA1_BUFFLEN);
  init=1;
  return;
  }

k=-1;
while (1)
  {
  for ( ; j<SHA1_BUFFLEN; j++)     /* j carried over from previous call */
    {
    ++k;
    if (k>=len)  return;
    buf[j]=str[k];
    loco+=8;
    if (loco==0)
      {++hico; if (hico==0) {fprintf(stderr,"too much data!"); exit(1);}}
    }
  doSHA1block(buf,H);
  j=0;
  }/* end while */

}/* end SHA1_multi */
//</code></function>


/*<function id="SHA1_string">
<heading>
  <description><en>Digests a string</en></description>
  <parameters />
  <usage />
  <checks />
  <comment>
calculate the digest of len bytes from string buffer, up to 32000 bytes
ignores hico because of short strings
  </comment>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SHA1_string(
	unsigned char *str,
	int len,
	unsigned char digest[NDIG]
	)
// ---------------------------------------------------------------------------
{
unsigned char buf[SHA1_BUFFLEN];
unsigned long count=0,loco=0,H[5];
int j,k=1;

initSHA1(H);

while (k)
  {
  for (j=0; j<SHA1_BUFFLEN; j++)
    {
    if (count>=len) {k=bufpad(buf,SHA1_BUFFLEN,loco,0); break;}
    else {buf[j]=str[count++];  loco+=8;}
    }
  doSHA1block(buf,H);
  }

makeSHA1(digest,H);
clearbuf(buf,0,SHA1_BUFFLEN);
}/* end SHA1_string */
//</code></function>

