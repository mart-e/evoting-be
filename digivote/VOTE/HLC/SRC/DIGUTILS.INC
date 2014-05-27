/* digutils.c 
utils for handling data digests, not specific to any particular digest

by W. Gray, started 27 Aug 2001

NOTES;

need to #define NDIG 16 (or whatever) above here
note, NDIG=16 for MD5,  =20 for SHA1


need STRINGDIGEST #defined as the name of the routine for
digesting a single null-terminated string,
eg for all_md5.c, #define STRINGDIGEST  MD5_string

called as; STRINGDIGEST(char *string,unsigned int len,char dig[NDIG])


to use the random-byte routines, #define RBYTESBUF above here
eg, #define RBYTESBUF 512       (must be multiple of 4*NDIG)



*/

#ifndef NDIG
#error need NDIG defined above digutils.c
#endif


/* these routines only need NDIG defined above here */
void printDigest(char *,unsigned char [NDIG]);
int foundDigest(char *,unsigned char [NDIG]);
void addDigest(char *,unsigned char [NDIG]);
void sendDigest(FILE *,unsigned char [NDIG]);
void getLastDigest(char *,unsigned char [NDIG]);
int getNextDigest(FILE *,unsigned char [NDIG]);
int matchDigest(unsigned char [NDIG], unsigned char [NDIG]);
void copyDigest(unsigned char [NDIG], unsigned char [NDIG]);
void makeFileName(char [],unsigned char [NDIG]);

void crash(char *);


#ifdef STRINGDIGEST
/* these routines need STRINGDIGEST #defined above here */
void rollKey(char *,unsigned char [NDIG]);
void streamCryp(FILE *,FILE *,char *,unsigned char [NDIG]);
int isKeyPair(unsigned char [NDIG], unsigned char [NDIG]);
void makeRandomBytes(long,FILE *,char *);
#endif


/* these routines need STRINGDIGEST and RBYTESBUF #defined above here */
#ifdef RBYTESBUF         /* should be multiple of 4*16 */

#ifndef STRINGDIGEST
#error need STRINGDIGEST defined above digutils.c
#endif

/* globals and decls for random number routines */
unsigned char RANDBYTE[RBYTESBUF];
int BUFINDEX;

unsigned char randomByte1(char *);
unsigned int randomByte2(char *);
unsigned long randomByte4(char *);
unsigned char rbyte1(unsigned char, char *);
unsigned int rbyte2(unsigned int, char *);
unsigned long rbyte4(unsigned long, char *);
void fillRandBuf(char *,unsigned char [NDIG]);
#endif




int foundDigest(char *file,unsigned char digest[NDIG])
/*------------------------------------------------------------------
search file for a match to digest[], assumes file exists ok
returns 1 if found
returns 0 if not found
returns 0 if file not opened
assumes all digests are same length NDIG
*/
{
unsigned char dig2[NDIG];
FILE *pfile;
int i;

pfile=fopen(file,"rb");
if (pfile==NULL) return(0);

while (1)
  {for (i=0; i<NDIG; i++)
    {dig2[i]=getc(pfile);
    if (i==0 && feof(pfile)) {fclose(pfile); return(0);}
    if (feof(pfile)) crash("foundDigest: format error in key file");
    }

  if ( matchDigest(dig2,digest) ) {fclose(pfile); return(1);}
  }

}/* end foundDigest */



void printDigest(char *mess,unsigned char digest[NDIG])/*-----------------
print digest in hex to stdout, message is optional
*/
{
int i;

if (mess[0]) printf("%s",mess);
for (i=0; i<NDIG; i++) printf("%02x",digest[i]);
printf("\n");
}/* end printDigest */




void addDigest(char *pwdfile,unsigned char digest[NDIG])/*---------------
append a digest to a file
*/
{
FILE *pfile;
int i;

pfile=fopen(pwdfile,"ab");
if (pfile==NULL) crash("addDigest: key file not opened");

for (i=0; i<NDIG; i++) fprintf(pfile,"%c",digest[i]);
fclose(pfile);

}/* end addDigest */


void sendDigest(FILE *pfile,unsigned char digest[NDIG])/*---------------
send a digest on stream, assumed opened "wb" or "ab"
*/
{
int i;

for (i=0; i<NDIG; i++) fprintf(pfile,"%c",digest[i]);
}/* end sendDigest */


void getLastDigest(char *pwdfile,unsigned char digest[NDIG])/*-------------
retrieve the last digest in a file, assumes all digests
are NDIG in length
*/
{
FILE *pfile;
unsigned char c;
int i,j=0;

pfile=fopen(pwdfile,"rb");
if (pfile==NULL) crash("getLastDigest: key file not opened");

while (1)
  {for (i=0; i<NDIG; i++)
    {c=getc(pfile);
    if (feof(pfile))
      {if (i==0 && j==0) crash("getLastDigest: no keys in key file");
      if (i) crash("getLastDigest: format error in key file");
      fclose(pfile);
      return;                        /* found it */
      }
    digest[i]=c;
    }
  ++j;
  }

}/* end getLastDigest */



int getNextDigest(FILE *pfile,unsigned char arr[NDIG])
/*------------------------------------------------------------------
Gets next NDIG bytes from file or stream, assumed opened "rb"
returns number of bytes recovered, test if =NDIG
*/
{
int i;

for (i=0; i<NDIG; i++)
  {arr[i]=getc(pfile);
  if (feof(pfile)) return(i);
  }
return(i);

}/* end getNextDigest */



int matchDigest(unsigned char arr1[NDIG], unsigned char arr2[NDIG])
/*------------------------------------------------------------------
returns 1 if arr1 = arr2, else 0
*/
{
int i;

for (i=0; i<NDIG; i++)  if (arr1[i] != arr2[i]) return(0);

return(1);
}/* end matchDigest */


void copyDigest(unsigned char arr1[NDIG], unsigned char arr2[NDIG])
/*------------------------------------------------------------------
copy arr1 to arr2
*/
{
int i;

for (i=0; i<NDIG; i++)  arr2[i] = arr1[i];

}/* end copyDigest */


void makeFileName(char file[],unsigned char digest[NDIG])
/*----------------------------------------------------------------------
build a MS-DOS 8+3 filename from a digest
allows 52^11 = 7.5e18 different filenames for old-style 8+3 format

file must be declared at 13 bytes, digest assumed to be
longer than this
string legal83 includes all 52 legal chars in a MS-DOS 8+3 filename
*/
{
char *legal83="ABCDEFGHIJKLMNOPQRSTUVWXYZ_-$0123456789^~!#%&{}@`'()";
int i,j,l=52,name=8,type=3;

if ((name+type+1)>NDIG) crash("makeFileName: filename too long for digest");

for (i=0; i<name; i++)
  {j=(int)(digest[i]%l);
  file[i]=legal83[j];
  }
file[i]='.'; ++i;
for ( ; i<=name+type; i++)
  {j=(int)(digest[i]%l);
  file[i]=legal83[j];
  }
file[i]=0;
}/* end makeFileName */


void crash(char *message)/*---------------------------------------------
crashes with message
*/
{
fprintf(stderr,"\nhalted: %s\n",message);
fflush(stdout);
fflush(stderr);
exit(1);
}/* end crash */




#ifdef STRINGDIGEST

void rollKey(char *key,unsigned char digest[NDIG])/*-----------------
generate next digest from previous digest + keyPhrase
NB, can't use strcat because of embedded zeros in digest
*/
{
int i,j;
unsigned char buf[128];

j=NDIG+strlen(key);
if (j>=128) crash("rollKey: keyPhrase too long");
if (j==NDIG) crash("rollKey: no keyPhrase");

for (i=0; i<NDIG; i++) buf[i]=digest[i];
for ( ; i<j; i++) buf[i]=*(key+i-NDIG);

STRINGDIGEST(buf,j,digest);

}/* end rollKey */



void streamCryp(FILE *infile,FILE *outfile,
                              char *key,unsigned char digest[NDIG])
/*---------------------------------------------------------------
encrypt a binary-mode file or stream, send on outfile (binary-mode)
digest is initialised by one call of STRINGDIGEST() applied to keyphrase
*/
{
unsigned char C;
int i=0;

rollKey(key,digest);
C=(unsigned char) getc(infile);

while (! feof(infile))             /* process to end-of-file */
  {C^=digest[i];
  putc(C,outfile);
  ++i;
  if (i==NDIG)                  /* create new key */
    {rollKey(key,digest);
    i=0;
    }
  C=(unsigned char) getc(infile);
  }

rollKey(key,digest);
}/* end streamCryp */


int isKeyPair(unsigned char dig1[NDIG], unsigned char dig2[NDIG])
/*------------------------------------------------------------
returns 1 if digests are related as  dig2=STRINGDIGEST(dig1)  else 0
*/
{
unsigned char d[NDIG];

STRINGDIGEST(dig1,NDIG,d);

return(matchDigest(d,dig2));
}/* end isKeyPair */

#endif


void makeRandomBytes(long loops,FILE *outfile,char *seedString)
/*------------------------------------------------------------------
use rollover process to recursively generate random bytes
using a message digest, send on binary stream
note; total bytes = NDIG * loops
*/
{
long i;
unsigned char digest[NDIG];

/* initialise digest */
STRINGDIGEST(seedString,strlen(seedString),digest);

for (i=0; i<loops; i++)
  {rollKey(seedString,digest);
  sendDigest(outfile,digest);
  }

}/* end makeRandomBytes */




#ifdef RBYTESBUF

#if RBYTESBUF%(4*NDIG) != 0
#error RBYTESBUF should be multiple of 4*NDIG
#endif


unsigned char randomByte1(char *seedString)
/*-------------------------------------------------------------------
use rollover process to return random bytes <=255
*/
{
static unsigned char dig[NDIG];
unsigned char r;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("randomByte1: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

/* make more bytes */
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
r=RANDBYTE[BUFINDEX++];

return(r);
}/* end randomByte1 */


unsigned int randomByte2(char *seedString)
/*-------------------------------------------------------------------
use rollover process to return random ints <=65535
*/
{
static unsigned char dig[NDIG];
unsigned int r;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("randomByte2: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

/* make more bytes, don't need to check after each buffer access */
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);

r=RANDBYTE[BUFINDEX++];
r=(r<<8) | RANDBYTE[BUFINDEX++];

return(r);
}/* end randomByte2 */

unsigned long randomByte4(char *seedString)
/*-------------------------------------------------------------------
use rollover process to return random longs
*/
{
static unsigned char dig[NDIG];
unsigned long r;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("randomByte4: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

/* make more bytes, don't need to check after each buffer access */
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);

r=RANDBYTE[BUFINDEX++];
r=(r<<8) | RANDBYTE[BUFINDEX++];
r=(r<<8) | RANDBYTE[BUFINDEX++];
r=(r<<8) | RANDBYTE[BUFINDEX++];

return(r);
}/* end randomByte4 */


unsigned char rbyte1(unsigned char nmax,char *seedString)
/*------------------------------------------------------------------
return random value < nmax, where nmax<=255
note, use randomByte1 to get numbers <=255
*/
{
static unsigned char dig[NDIG];
unsigned char j,n;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("rbyte1: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

if (nmax<2) {fprintf(stderr,"rbyte1: illegal nmax\n"); exit(1);}
n=(0xff/nmax)*nmax;

j=n;                        /* run until found within range */
while (j>=n)
  {if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
  j=RANDBYTE[BUFINDEX++];
  }

return(j%nmax);                /* reduce to demanded range */
}/* end rbyte1 */


unsigned int rbyte2(unsigned int nmax,char *seedString)
/*------------------------------------------------------------------
return random value < nmax, where nmax<=65535
note, use randomByte2 to get numbers <=65535
*/
{
static unsigned char dig[NDIG];
unsigned int j,n;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("rbyte2: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

if (nmax<2) {fprintf(stderr,"rbyte2: illegal nmax\n"); exit(1);}
n=(0xffff/nmax)*nmax;

/* build 2-byte word */
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=RANDBYTE[BUFINDEX++];
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=(j<<8) | RANDBYTE[BUFINDEX++];

while (j>=n)                /* run until found within range */
  {if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
  j=(j<<8) | RANDBYTE[BUFINDEX++];
  }

return(j%nmax);                /* reduce to demanded range */
}/* end rbyte2 */


unsigned long rbyte4(unsigned long nmax,char *seedString)
/*------------------------------------------------------------------
return random value < nmax, where nmax<=4294967295
note, use randomByte4 to get numbers <=4294967295
*/
{
static unsigned char dig[NDIG];
unsigned long j,n;
static int init=1;

if (init)
  {if (strlen(seedString)<10) crash("rbyte4: want longer seed string");
  STRINGDIGEST(seedString,strlen(seedString),dig);
  fillRandBuf(seedString,dig);
  init=0;
  }

if (nmax<2) {fprintf(stderr,"rbyte4: illegal nmax\n"); exit(1);}
n=(0xffffffff/nmax)*nmax;

/* build 4-byte word */
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=RANDBYTE[BUFINDEX++];
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=(j<<8) | RANDBYTE[BUFINDEX++];
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=(j<<8) | RANDBYTE[BUFINDEX++];
if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
j=(j<<8) | RANDBYTE[BUFINDEX++];

while (j>=n)                /* run until found within range */
  {if (BUFINDEX==RBYTESBUF) fillRandBuf(seedString,dig);
  j=(j<<8) | RANDBYTE[BUFINDEX++];
  }

return(j%nmax);                /* reduce to demanded range */
}/* end rbyte4 */


void fillRandBuf(char *seedString,unsigned char dig[NDIG])
/*-------------------------------------------------------------
fills global random bytes buffer from digest
*/
{
int i,j;

for (j=0; j<RBYTESBUF; j+=NDIG)
  {for (i=0; i<NDIG; i++)  RANDBYTE[i+j]=dig[i];
  rollKey(seedString,dig); 
  }

BUFINDEX=0;
}/* end fillRandBuf */

#endif
