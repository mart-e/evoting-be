/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="cryplib.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.16 $</revision>
  <archiveTimeStamp>$Date: 2009/01/30 13:54:08 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>cryplib.c</fileName>
  <creationDate>01/01/2002</creationDate>

  <rights></rights>
  <author>AVN</author>

  <description>
    <fr>Librarie d'encryptage/décryptage et contrôle d'intégrité</fr>
    <nl>Library voor encryptie/decryptie en integriteitskontrole</nl></description>
  <history></history>
  <comment>
These functions always return 0, exceptions are returned to
the Progress shared variable 'crypStat'.
Exceptions are always negative, first digits identify function,
last two identify error within the function.</comment>
</comments>

<header confidential="yes">*/

#define BUFFSIZE   512

/*<includes>*/
#include <stdio.h>                               /* sprintf(), fopen(), fwrite(), fclose() */
#include <dos.h>                                 /* date/time */
#include <io.h>                                  /* filelength() */
#include <errno.h>                               /* filelength() */

#include "hlc.h"
#include "aeslib.h"
#include "genlib.h"
/*</includes>*/

static char gcLibName[] = "CrypLib";

/*</header>
<body>*/

/*<function id="CRYPAUT" confidential="yes">
<heading>
  <description><en>
    Calculates MAC on data file globals.bin (cfr. InitiateAes()) using
    argv[1] encryption key and compares it with an hard coded MAC.</en></description>
<parameters></parameters><comment>
   Input    : source filename,
              encryption key.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypaut( argc, argv)
   int  argc;
   char *argv[];
{
      /* working storage */
      int i, iLength;
      char cBuffer[BUFFSIZE];

      FILE* stream = NULL;
      BYTE Macr[4]="0000", Maci[4], Resr[4]="0000", Resi[4];

//      BYTE zMac[8] = { 233+3, 104-5, 10+4, 236-6, 107+5, 61-7, 193+6, 238-8};
      /* :WARNING: avn 20020409 -> zMac[] is an hardcoded MAC
    It is offset a bit for confidentiality and will be revealed before
    comparison. It was obtained by MACing the globals.bin datafile
    with the internal mK and sK that can be found in INIT_PWD.P.
    Changing these keys requires updating zMac[]! */
      BYTE zMac[8] = {
    0x45 + 3,
    0x70 - 5,
    0xDA + 4,
    0x07 - 6,
    0x45 + 5,
    0x03 - 7,
    0x20 + 6,
    0x58 - 8
    };

/* Init error handling */
   InitFunction( gcLibName, "crypaut", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* Open globals datafile in binary mode */
   if ( ( stream = fopen( gcGlobalsFileName, "rb")) == NULL) {
      /* datafile cannot be opened */
      giStatus = -10;
      goto error;
   }

/* Read input file */
   while ( iLength = fread( cBuffer, 1, BUFFSIZE, stream)) {
      /* read the file buffer after buffer, compute the MAC for each buffer,
     and make the needed combinations between each step */
      Mac( URNCHK, cBuffer, iLength);

      /* this way we can keep rest of the old code unchanged */
      memcpy( Maci, Result_Key, 4);
      memcpy( Resi, Result_Key+4, 4);

      for ( i = 0; i < 4; i++) {
    Macr[i] ^= Maci[i];
    Resr[i] ^= Resi[i];
      }

   } /* eo while */

/* Reveal known MAC */
      for ( i = 0; i < 4; i++) {
    zMac[ (i*2)] -= i+3;
    zMac[ (i*2)+1] += i+5;
      }

#if _DEBUGMODE
   sprintf( gcMessage, "zMac: ");
      LogHexa( zMac, sizeof( zMac)); LogDebug();
   sprintf( gcMessage, "computed MAC:");
      LogHexa( Macr, sizeof( Macr));
      LogHexa( Resr, sizeof( Resr)); LogDebug();
   /* zMAC should be Macr+Resr */
#endif

/* Compare computed MAC with known one */
   if ( memcmp( zMac, Macr, 4) || memcmp( zMac+4, Resr, 4)) {
      // MAC's are different!
      giStatus = -20;
      goto error;
   }

error:
   // close opened files
   if ( stream) fclose( stream);

   if ( giStatus) {
      giStatus -= 9100;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypaut() */
/*</code></function>

<function id="C_KEYGEN" confidential="yes">
<heading>
  <description><en>Generates master en session keys according to the AES usage
recommendations.</en></description>
<parameters></parameters><comment>
   Input    : key counter,
              key material string

   Output   : generated key string,
              shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
c_keygen( argc, argv)
   int argc;
   char *argv[];
{
      /* inputs */
      int iKeyCtr;
      char* cpKeyMat;

      /* working storage */
      char cData[ 128];

#define KEYCHARMAX 34                  /* 34 allowed chars */
      char cAllowed[ KEYCHARMAX] =      /* allowed char in master keys */
      { '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
   'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
   'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
   'W', 'X', 'Y', 'Z'
      };
      int iRet;
      int iModeIsMaster = 0;
      int iModeIsSession = 0;
      int i;

      /* output parameters */
      BYTE cResult[AESKEYLEN+1];

/* init error handling */
   InitFunction( gcLibName, "c_keygen", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   iKeyCtr = atoi( argv[1]);
   cpKeyMat = argv[2];

/* validate function arguments */
   if ( 0 == iKeyCtr) {
      // Argv[1] cannot be converted to int
      giStatus = -10;
      goto error;
   }

   iModeIsMaster = !strnicmp( cpKeyMat, "MASTER", strlen( "MASTER"));
   iModeIsSession = !strnicmp( cpKeyMat, "SESSION", strlen( "SESSION"));
   if ( !(iModeIsMaster || iModeIsSession) ) {
      // Argv[2] invalid key material
      giStatus = -20;
      goto error;
   }

#if _DEBUGMODE
   sprintf( gcMessage, "args [%s] [%s].", argv[1], argv[2]);
   LogDebug();
#endif

/* set up an 80 bytes buffer (must be multiple of AESKEYLEN!)
     content has been determined scientifically cfr. recommendations ;-) */
   sprintf( cData, "%s%s%s%s", getTimeStamp(), argv[1], cpKeyMat, "0" );
   do {
     strcat( cData, "2");
   } while ( strlen( cData) < 78);
   strcat( cData, "80");

#if _DEBUGMODE
   sprintf( gcMessage, "cData [%50s] len[%d].", cData, strlen( cData));
   LogDebug();
#endif

/* hash this material to obtain a key */
   if ( iModeIsMaster) {
      /* master hash */
      iRet = computeKeyGeneratorHash( cResult, AESKEYLEN, cData, strlen( cData));
      if ( iRet < 0) {

#if _DEBUGMODE
    sprintf( gcMessage, "computeKeyGeneratorHash returns error [%d].", iRet);
    LogDebug();
#endif
    giStatus = -30;
    goto error;
      }

      /* convert binary result to text (master keys must be entered by user/keyboard) */
      for ( i = 0; i < AESKEYLEN; i++) {
     cResult[ i] = cAllowed[ ( (int) cResult[i] % KEYCHARMAX)];
      }

   }
   else {
      /* session hash */
      iRet = computeHash( cResult, AESKEYLEN, cData, strlen( cData));
      if ( iRet < 0) {

#if _DEBUGMODE
    sprintf( gcMessage, "computeHash returns error [%d].", iRet);
    LogDebug();
#endif
    giStatus = -40;
    goto error;
      }

      /* :WARNING: avn 20020129 -> null bytes must be patched!
    If not truncated results will be send back to Progress... */
      for ( i = 0; i < AESKEYLEN; i++) {
     if ( 0x00 == cResult[ i]) {
        cResult[ i] = 0xFF;
     }
      }
      /* session keys are binary (they must never be provided by user/keyboard) */
   }

/* process results */
cResult[AESKEYLEN] = 0x00;

#if _DEBUGMODE
   sprintf( gcMessage, "cResult[%s].", cResult);
   LogDebug();
#endif

/* write result back to Progress variables */
   if ( prowtc( "rndKey", 0, cResult, 0) != 0) {

#if _DEBUGMODE
      sprintf( gcMessage, "update rndKey failed!");
      LogDebug();
#endif
      giStatus = -99;
      goto error;
   }

error:
   if ( giStatus) {
      giStatus -= 1000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof c_keygen() */
/*</code></function>

<function id="CRYPSK" confidential="yes">
<heading>
  <description><en>
    Encrypts wether decrypts a session key file.</en></description>
<parameters></parameters><comment>
   Input    : source filename,
              destination filename.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypsk( argc, argv)
   int argc;
   char *argv[];
{

      /* input parameters */
      char* cpMode;
      char* cpFileName;

      /* temporary results */
      FILE* dataStream = NULL;
      BYTE Buffer[ AESKEYLEN + MAX_IV_SIZE + 1];

      /* output parameters */
      BYTE cResult[AESKEYLEN+1];

/* init error handling */
   InitFunction( gcLibName, "crypsk", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   cpMode = argv[1];
   cpFileName = argv[2];

/* validate function arguments */
   if ((cpMode[0] != 'E') && (cpMode[0] != 'D')) {
      // Argv[1] undefined encryption mode
      giStatus = -10;
      goto error;
   }


#if _DEBUGMODE
   sprintf( gcMessage, "args [%s] [%s] [%s].", cpMode, cpFileName, cpFileName);
   LogDebug();
#endif

   if ( cpMode[0] == 'E') {              /* mode = Encryption */
      /* open destination file */
      if ( ( dataStream = fopen( cpFileName, "wb")) == NULL) {
    // Cannot open destination file
    giStatus = -40;
    goto error;
      }

      /* put current sK in buffer */
      memcpy( Buffer, sKey.data, AESKEYLEN);

      /* setup encryption with master key and encrypt session key */
      InitEnc( ENCKEYSMAX);
      Enc( Buffer, 1);

      /* write IV to binary file */
      if ( fwrite( iv.data, 1, MAX_IV_SIZE, dataStream) != MAX_IV_SIZE) {
    // Error writing IV to destination
    giStatus = -44;
    goto error;
      }
      /* write encrypted session key to binary file */
      if ( fwrite( Buffer, 1, AESKEYLEN, dataStream) != AESKEYLEN) {
    // Error writing sK to destination
    giStatus = -46;
    goto error;
      }
   }
   else {                                /* mode = Decryption */
      /* open source file */
      if ( ( dataStream = fopen( cpFileName, "rb")) == NULL) {
    // Cannot open source file
    giStatus = -50;
    goto error;
      }

      if ( filelength( fileno( dataStream)) < ( MAX_IV_SIZE + AESKEYLEN)) {
    // Source file invalid length
    giStatus = -70;
    goto error;
      }

      /* read encrypted session key from binary file */
      if ( fread( Buffer, 1, (MAX_IV_SIZE + AESKEYLEN), dataStream) != (MAX_IV_SIZE + AESKEYLEN)) {
    // Error reading binary file
    giStatus = -72;
    goto error;
      }

      /* setup IV */
      memcpy( iv.data, Buffer, MAX_IV_SIZE);

      /* setup decryption with master key and decrypt session key */
      InitDec( ENCKEYSMAX);
      Dec( &Buffer[MAX_IV_SIZE], (AESKEYLEN / AESBLOCKLEN));

      /* process results */
      memcpy( cResult, Buffer+MAX_IV_SIZE, AESKEYLEN);
      cResult[AESKEYLEN] = 0x00;

      /* write result back to Progress variables */
      if ( prowtc( "rndKey", 0, cResult, 0) != 0) {
    // pdate rndKey failed
    giStatus = -99;
    goto error;
      }
   }

error:
   // close opened files
   if ( dataStream) fclose( dataStream);

   if ( giStatus) {
      giStatus -= 2000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypsk() */
/*</code></function>

<function id="CRYPDEC" confidential="yes">
<heading>
  <description><en>Decrypts Progress database fields.</en></description>
<parameters></parameters><comment>
   Input    : encryption key,
              data to decrypt.

   Output   : shared variable result4 contains decrypted data
              shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypdec( argc, argv)
   int argc;
   char *argv[];
{
      /* inputs */
      int iEncMethod;
      char* cpData;
      int iNumeric;

      /* working storage */
      BYTE cBlock[(AESKEYLEN*2)+1];              /* holds a copy of input data block */
      int iRet, i;
      BYTE left_part, right_part;

      /* output parameters */
      BYTE cResult[AESKEYLEN+1];

/* init error handling */
   InitFunction( gcLibName, "crypdec", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   iEncMethod = atoi( argv[1]);
   cpData = argv[2];
   iNumeric = ( argc == 4 ? atoi( argv[3]) : 0);

#if _DEBUGMODE
   sprintf( gcMessage, "args [%d] [%s].", iEncMethod, cpData );
   LogDebug();
#endif

/* validate function arguments */
   if ( (iEncMethod < 0) || (iEncMethod > (ENCKEYSMAX - 1))) {
      // Argv[1] undefined encryption method
      giStatus = -10;
      goto error;
   }

   if ( (AESKEYLEN*2) != strlen( cpData)) {
      // Argv[2] invalid length
      giStatus = -20;
      goto error;
   }

/* decrypt data */
   memcpy( cBlock, cpData, AESKEYLEN*2);        /* copy input first to keep input unchanged */

   /* glue splitted bytes back together before decrypting, this technique was
      also used with DES. Probably due to avoid problems with binary data in char field. */
   for ( i = 0 ; i < AESKEYLEN; i++) {
       left_part = cBlock[i] & 0x0f;
       left_part = left_part << 4;
       right_part = cBlock[i+AESKEYLEN] & 0x0f;
       cResult[i] = left_part | right_part;
   }

   InitEnc( iEncMethod); /* in order to get the fixed IV back */
   InitDec( iEncMethod); /* setup decryption engine */
   Dec( cResult, 1);

/* prepare result */
   cResult[AESKEYLEN] = 0x00;

/* check numeric contents if required */
   if( iNumeric) {
      for ( i = 0 ; i < AESKEYLEN; i++) {
    /* don't use atoi(), because errors will result in 0 (zero) */
    /* - "0000000000000000" = 0 = numeric !     = OK            */
    /* - "thisisnotnumeric" = 0 = NOT numeric ! = NOT OK        */
    if( cResult[i] < '0' || cResult[i] > '9') {
       // cResult NOT numeric
       giStatus = -90;
       goto error;
    }
      }
   }

/* write result back to Progress variables */
   if ( prowtc( "result4", 0, cResult, 0) != 0) {
      // update result4 failed
      giStatus = -99;
      goto error;
   }

error:
   if ( giStatus) {
      giStatus -= 3000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                         /* eof crypdec() */
/*</code></function>

<function id="CRYPENC" confidential="yes">
<heading>
  <description><en>Encrypts Progress database fields.</en></description>
<parameters></parameters><comment>
   Input    : encryption key,
              data to encrypt.

   Output   : shared variable result3 contains encrypted data
              shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypenc( argc, argv)
   int argc;
   char *argv[];
{
      /* inputs */
      int iEncMethod;
      char* cpData;

      /* working storage */
      BYTE cBlock[AESKEYLEN];                      /* holds a copy of input data block */
      int iRet, i;
      BYTE left_part, right_part;

      /* output parameters */
      BYTE cResult[(AESKEYLEN*2)+1];               /* will be written back to Progress */

/* init error handling */
   InitFunction( gcLibName, "crypenc", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   iEncMethod = atoi( argv[1]);
   cpData = argv[2];

/* validate function arguments */
   if ((iEncMethod < 0) || (iEncMethod > (ENCKEYSMAX - 1))) {
      // Argv[1] undefined encryption method
      giStatus = -10;
      goto error;
   }

   if (AESKEYLEN != strlen( cpData)) {
      // Argv[2] invalid length
      giStatus = -20;
      goto error;
   }

#if _DEBUGMODE
   sprintf( gcMessage, "args [%d] [%s].", iEncMethod, cpData );
   LogDebug();
#endif

/* encrypt data */
   memcpy( cBlock, cpData, AESKEYLEN);          /* copy input first to keep input unchanged */
   InitEnc( iEncMethod);                        /* setup encryption */
    /* :WARNING: avn 20011217 -> fixed block length used here
       This is acceptable since this routine is used for encryption of passwords and counters
       only. It is assumed that this data ALWAYS fits a single block, hence the magical number 1! */
   Enc( cBlock, 1);

/* prepare result */
   /* split encrypted bytes before writeback, this technique was
      also used with DES. Probably due to avoid problems with binary data in char field. */
   for ( i = 0 ; i < AESKEYLEN ; i++) {
       right_part = 0x0f & cBlock[i];
       left_part  = 0xf0 & cBlock[i];
       left_part  = left_part >> 4;
       cResult[i]   = left_part | 0x40;
       cResult[i+AESKEYLEN] = right_part | 0x40;
   }
   cResult[AESKEYLEN*2] = 0x00;

/* write result back to Progress variables */
   if ( prowtc( "result3", 0, cResult, 0) != 0) {
      // update result3 failed
      giStatus = -99;
      goto error;
   }

error:
   if ( giStatus) {
      giStatus -= 4000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof Crypenc() */
/*</code></function>

<function id="CRYPFILE" confidential="yes">
<heading>
  <description><en>Encrypts/decrypts data files.</en></description>
<parameters></parameters><comment>
   Input    : encryption mode,
              encryption key,
              source filename,
              destination filename.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypfile( argc, argv)
   int argc;
   char *argv[];
{

      /* input parameters */
      char* cpMode;
      int iEncMethod;
      int iTerminate;
      char* cpInputFileName;
      char* cpOutputFileName;

      /* temporary results */
      int iFirst;
      int iLen;
      int iBytes;
      int iBlocks;
      int iRecordLength;

      FILE* streamIn = NULL;
      FILE* streamOut = NULL;
      BYTE Buffer[ 256], Text[ 256];
      AESMAC aMac;

      long lSize;
      /* output parameters */

/* init error handling */
   InitFunction( gcLibName, "crypfile", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   cpMode = argv[1];
   iEncMethod = atoi( argv[2]);
   cpInputFileName = argv[3];
   cpOutputFileName = argv[4];
   iTerminate = ( argc == 6 ? atoi( argv[5]) : 1);

/* validate function arguments */
   if ((cpMode[0] != 'E') && (cpMode[0] != 'D')) {
      // Argv[1] undefined encryption mode
      giStatus = -10;
      goto error;
   }

   if ((iEncMethod < 0) || (iEncMethod > (ENCKEYSMAX - 1))) {
      // Argv[2] undefined encryption method
      giStatus = -20;
      goto error;
   }


#if _DEBUGMODE
   sprintf( gcMessage, "args [%s] [%i] [%s] [%s].",
      cpMode, iEncMethod, cpInputFileName, cpOutputFileName );
   LogDebug();
#endif

   if ( cpMode[0] == 'E') {              /* mode = Encryption */
      /* open source file */
      if ( ( streamIn = fopen( cpInputFileName, "rt")) == NULL) {
    // Cannot open source file
    giStatus = -30;
    goto error;
      }

      /* open destination file */
      if ( ( streamOut = fopen( cpOutputFileName, "wb")) == NULL) {
    // Cannot open destination file
    giStatus = -40;
    goto error;
      }

      /* read plain text file and produce encrypted binary file */
      iFirst = TRUE;
      while ( !feof( streamIn)) {
    /* read line from input file (delimited by CR) */
    if ( fgets( Buffer, sizeof( Buffer), streamIn) == NULL) {
       break;
    }

    /* strip off CR */
    if( iTerminate) {
       iLen = strlen( Buffer);
       if ( Buffer[ iLen - 1] == '\n')
       {
          Buffer[ iLen - 1] = 0;
       }
    }
    if ( iFirst) {                   /* IV must be saved for decryption afterwards */
       /* write plain record length (= IV + MAC) */
       iBytes = MAX_IV_SIZE + AESMACLEN;
       fwrite( &iBytes, sizeof( int), 1, streamOut);

       /* write IV made during encryption initialisation */
       InitEnc( iEncMethod);
       fwrite( iv.data, MAX_IV_SIZE, 1, streamOut);

       /* write MAC */
       Mac( iEncMethod, iv.data, MAX_IV_SIZE);
       memcpy( aMac.data, Result_Key, AESMACLEN);
       fwrite( aMac.data, AESMACLEN, 1, streamOut);

       iFirst = FALSE;
    }

    strcpy( Text, Buffer);
    AlignText( Text, &iBlocks);

    /* write record length (= datablocks + MAC) */
    iBytes = ( iBlocks * AESBLOCKLEN) + AESMACLEN;
    fwrite( &iBytes, sizeof( int), 1, streamOut);

    /* encrypt line and write encrypted data */
    Enc( Text, iBlocks);
    fwrite( Text, (iBlocks * AESBLOCKLEN), 1, streamOut);

    /* write MAC */
    Mac( iEncMethod, Text, (iBlocks * AESBLOCKLEN));
    memcpy( aMac.data, Result_Key, AESMACLEN);
    fwrite( aMac.data, AESMACLEN, 1, streamOut);

      /* eowhile */
      }
   }

   else {                               /* mode = Decryption */
      /* open source file */
      if ( ( streamIn = fopen( cpInputFileName, "rb")) == NULL) {
    // Cannot open source file
    giStatus = -50;
    goto error;
      }

      /* open destination file */
      if ( ( streamOut = fopen( cpOutputFileName, "wt")) == NULL) {
    // Cannot open destination file
    giStatus = -60;
    goto error;
      }

      lSize = filelength( fileno( streamIn));
      if ( lSize < ( AESKEYLEN)) {
    // Source file invalid length
    giStatus = -70;
    goto error;
      }

      iFirst = TRUE;
      for ( fread( &iRecordLength, 1, 2, streamIn);
       !feof( streamIn);
       fread( &iRecordLength, 1, 2, streamIn)) {
    /* read datablocks + MAC */
    fread( Buffer, 1, iRecordLength, streamIn);

    /* verify MAC on data */
    Mac( iEncMethod, Buffer, (iRecordLength - AESMACLEN));
    if ( memcmp( Result_Key, Buffer + (iRecordLength - AESMACLEN), AESMACLEN)) {
       // MAC is not correct
       giStatus = -80;
       goto error;
    }

    if ( iFirst) {
       /* retrieve IV and initialise decryption */
       memcpy( iv.data, Buffer, MAX_IV_SIZE);
       InitDec( iEncMethod);

       /* been there, done that */
       iFirst = FALSE;

       /* resume for-loop to get to the encrypted data */
       continue;
    }

    /* decrypt datablocks */
    Dec( Buffer, ((iRecordLength - AESMACLEN) / AESBLOCKLEN));

    /* drop the MAC */
    Buffer[ iRecordLength - AESMACLEN] = 0;

    /* write plain text to destination file */
    if ( (iLen = strlen( Buffer)) > 0) {
       if ( fwrite( Buffer, 1, iLen, streamOut) != iLen) {
          // Error writing text to destination
          giStatus = -90;
          goto error;
       }
    }
    /* terminate line with CR */
    if( iTerminate) {
        if ( fwrite( "\n", 1, 1, streamOut) != 1)
        {
      // Error writing CR to destination
      giStatus = -92;
      goto error;
        }
        }

      } /* eofor */

   }

error:
   // close opened files
   if ( streamIn)  fclose( streamIn);
   if ( streamOut) fclose( streamOut);

   if ( giStatus) {
      giStatus -= 5000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypfile() */
/*</code></function>

<function id="CRYPMAC" confidential="yes">
<heading>
  <description><en>Calculates MAC on argv[2], store in argv[3] using
argv[1] encryption key</en></description>
<parameters></parameters><comment>
   Input    : source filename,
              destination filename,
              encryption key.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypmac( argc, argv)
   int  argc;
   char *argv[];
{
      /* inputs */
      char* cpInName;
      char* cpOutName;
      int iEncMethod;

      /* working storage */
      FILE* stream = NULL;
      FILE* outstream = NULL;
      int i, iLength;
      BYTE Macr[4]="0000", Maci[4], Resr[4]="0000", Resi[4];
      char cBuffer[BUFFSIZE];
#if _DEBUGMODE
      int ii = 0;
#endif

      /* output parameters */

/* init error handling */
   InitFunction( gcLibName, "crypmac", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   iEncMethod = atoi( argv[1]);
   cpInName  = argv[2];
   cpOutName = argv[3];

#if _DEBUGMODE
   sprintf( gcMessage,"crypmac from %s to %s using %d", cpInName, cpOutName, iEncMethod);LogDebug();
#endif

/* validate function arguments */
   if ((iEncMethod < 0) || (iEncMethod > (ENCKEYSMAX - 1))) {
      // Argv[1] undefined encryption method
      giStatus = -10;
      goto error;
   }

/* Open file in binary mode     */
   if ( ( stream = fopen( cpInName, "rb")) == NULL) {
      // unable to open input file
      giStatus = -20;
      goto error;
   }

/* Read input file */
   while ( iLength = fread( cBuffer, 1, BUFFSIZE, stream)) {
      /* read the file buffer after buffer, compute the MAC for each buffer,
     and make the needed combinations between each step */
      Mac( iEncMethod, cBuffer, iLength);

      /* this way we can keep rest of the old code unchanged */
      memcpy( Maci, Result_Key, 4);
      memcpy( Resi, Result_Key+4, 4);

      for ( i = 0; i < 4; i++) {
         Macr[i] ^= Maci[i];
         Resr[i] ^= Resi[i];
      }
#if _DEBUGMODE
      ii++;
      sprintf( gcMessage, "TEXmac'in[%d] : ", ii);
      LogHexa( Macr, 4);
      LogHexa( Resr, 4);
      LogDebug();
#endif

   } /* eo while */

   if ( strcmp( cpOutName, "NULL")) {
      /* Store compute MAC in file */
      if ( ( outstream = fopen( cpOutName, "wb")) == NULL) {
         // unable to open output file
         giStatus = -30;
         goto error;
      }

      /* printf ("%d %d %d %d %d %d %d %d\n",               */
      /*         macr[0],macr[1],macr[2],macr[3],           */
      /*         resr[0],resr[1],resr[2],resr[3]);          */

      fwrite( Macr, 1, 4, outstream);
      fwrite( Resr, 1, 4, outstream);
   }
   else {
      /* Store compute MAC in shared variable 'hexMAC' */
      sprintf( cBuffer, "%02X %02X %02X %02X %02X %02X %02X %02X\0",
                        Macr[0],Macr[1],Macr[2],Macr[3],
                        Resr[0],Resr[1],Resr[2],Resr[3]);
      if ( prowtc( "hexMac", 0, cBuffer, 0) != 0) {
         // unable to write MAC to shared variable
         giStatus = -40;
         goto error;
      }
      /* Store compute MAC in shared variable 'formatMAC' */
      sprintf( cBuffer, "%.3d%.3d%.3d%.3d%.3d%.3d%.3d%.3d\0",
                        Macr[0],Macr[1],Macr[2],Macr[3],
                        Resr[0],Resr[1],Resr[2],Resr[3]);
      if ( prowtc( "formatMac", 0, cBuffer, 0) != 0) {
         // unable to write MAC to shared variable
         giStatus = -50;
         goto error;
      }
   }

error:
   // close opened files
   if ( stream)      fclose( stream);
   if ( outstream)   fclose( outstream);

   if ( giStatus) {
      giStatus -= 6000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypmac() */
/*</code></function>

<function id="READMAC" confidential="yes">
<heading>
  <description><en>Read MAC from argv[1]</en></description>
<parameters></parameters><comment>
   Input    : source filename,

   Output   : shared variable crypStat contains return code
              shared variable binaryMAC contains the read MAC from file

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
readmac( argc, argv)
   int  argc;
   char *argv[];
{
      /* inputs */
      char* cpInName;

      /* working storage */
      FILE* stream = NULL;
      int iLength;
      BYTE cBuffer[BUFFSIZE];
      char hexOutput[BUFFSIZE];

      /* output parameters */

/* init error handling */
   InitFunction( gcLibName, "readmac", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   cpInName  = argv[1];

/* Open file in binary mode     */
   if ( ( stream = fopen( cpInName, "rb")) == NULL) {
      // unable to open input file
      giStatus = -10;
      goto error;
   }

/* Read input file */
   iLength = fread( cBuffer, 1, BUFFSIZE, stream);
   if( iLength == 8) {
      sprintf( hexOutput, "%02X %02X %02X %02X %02X %02X %02X %02X\0",
                           cBuffer[0],cBuffer[1],cBuffer[2],cBuffer[3],
                           cBuffer[4],cBuffer[5],cBuffer[6],cBuffer[7]);
   } else {
      sprintf( hexOutput, "XX XX XX XX XX XX XX XX\0");
   }

/* Store compute MAC in shared variable 'hexMAC' */
   if ( prowtc( "hexMac", 0, hexOutput, 0) != 0) {
      // unable to write MAC to shared variable
      giStatus = -20;
      goto error;
   }

error:
   // close opened files
   if ( stream) fclose( stream);

   if ( giStatus) {
      giStatus -= 6500;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypmac() */
/*</code></function>

<function id="CRYPKEYS" confidential="yes">
<heading>
  <description><en>
Computes the different encryption keys used by all other
encryption functions. This function MUST
be called as soon as the master and session keys are known
and BEFORE any encryption/decryption action.</en></description>
<parameters></parameters><comment>
   Input    : masterkey,
              sessionkey.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypkeys( argc, argv)
   int argc;
   char *argv[];
{
      /* inputs */
      char* cpMasterKey;
      char* cpSessionKey;

      /* working storage */
      int iRet;

      /* output parameters */


/* init error handling */
   InitFunction( gcLibName, "crypkeys", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   cpMasterKey = argv[1];
   cpSessionKey = argv[2];

/* validate function arguments */
   if (AESKEYLEN != strlen( cpMasterKey)) {
      // Argv[1] invalid length
      giStatus = -10;
      goto error;
   }

   if (AESKEYLEN != strlen( cpSessionKey)) {
      // Argv[2] invalid length
      giStatus = -20;
      goto error;
   }

/* set master and session key */
   memcpy( mKey.data, cpMasterKey, AESKEYLEN);
   memcpy( sKey.data, cpSessionKey, AESKEYLEN);

#if _DEBUGMODE
   sprintf( gcMessage, "args [%s] [%s].", mKey.data, sKey.data );
   LogDebug();
#endif

/* make internal encryption keys */
   iRet = MakeEncryptionKeys();
   if ( iRet < 0) {

#if _DEBUGMODE
      sprintf( gcMessage, "MakeEncryptionKeys() returns error [%d].", iRet);
      LogDebug();
#endif
      giStatus = -30;
      goto error;
   }


error:
   if ( giStatus) {
      giStatus -= 7000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof crypkeys() */
/*</code></function>

<function id="CRYPINIT" confidential="yes">
<heading>
  <description><en>
This function must be called ONCE before ANY encryption/decryption
actions in order to allocate global data used by the AES-engine.
This function is specific to the Progress port of AesLib.c .</en></description>
<parameters></parameters><comment>
   Input    : none.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypinit( argc, argv)
   int argc;
   char *argv[];
{

      /* working storage */
      int iRet;

/* init error handling */
   InitFunction( gcLibName, "crypinit", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* start AES engine */
   iRet = InitiateAes();
   if ( iRet < 0) {
      giStatus = -10; // something wrong in the kitchen!
      goto error;
   }

error:
   if ( giStatus) {
      giStatus -= 8000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof Crypinit() */
/*</code></function>

<function id="CRYPSTOP" confidential="yes">
<heading>
  <description><en>
Deallocates global data allocated by crypinit.c
This function is specific to the Progress port of AesLib.c .</en></description>
<parameters></parameters><comment>
   Input    : none.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
int
crypstop( argc, argv)
   int argc;
   char *argv[];
{

      /* working storage */
      int iRet;

      /* output parameters */

/* init error handling */
   InitFunction( gcLibName, "crypstop", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* stop AES engine */
   iRet = TerminateAes();
   if ( iRet < 0) {

#if _DEBUGMODE
      sprintf( gcMessage, "TerminateAes() returns error [%i].", iRet);
      LogDebug();
#endif
      giStatus = -10;
      goto error;
   }

error:
   if ( giStatus) {
      giStatus -= 9000;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}                                      /* eof Crypstop() */
/*</code></function>

<function id="CRYPBINF" confidential="yes">
<heading>
  <description><en>Encrypts binary files</en></description>
  <parameters></parameters>
  <comment>
   Input    : encryption mode,
              encryption key,
              source filename,
              destination filename.

   Output   : shared variable crypStat contains return code

   Remarks  : function always returns 0, actual return code is in crypStat.
</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
int
crypbinf( argc, argv)
   int argc;
   char *argv[];
// ---------------------------------------------------------------------------
{
   long     FileLength;             // file length in bytes
   unsigned ReadLength;             // # bytes to read
   FILE*    InputFile = NULL;              // file handle's
   FILE*    OutputFile = NULL;
   int      First;                  // flag's first read
   int      Blocks;                 // # blocks to encrypt

   BYTE     Buffer[BUFFSIZE];       // read buffer

   int      pMethod;                // encryption key id
   char*    pInputFileName;         // name source file
   char*    pOutputFileName;        // name destination file

/* init error handling */
   InitFunction( gcLibName, "crypbinf", "crypStat", TRUE);
   if( giStatus == -1) return( 0);

/* process function arguments */
   pMethod         = atoi( argv[1]);
   pInputFileName  = argv[2];
   pOutputFileName = argv[3];

   // open input file
   InputFile = fopen( pInputFileName, "rb");
   if( InputFile == NULL) {
    giStatus = -10;
    goto error;
   }
   else {
       FileLength = filelength( fileno( InputFile));
   }

   //check for existence of output file
   if ( !access( pOutputFileName, 0x00)) {
      if ( remove( pOutputFileName)) {       //remove output file
    giStatus = -12;
    goto error;
      }
   }

   //create binary output file
   if( ( OutputFile = fopen( pOutputFileName, "wb")) == NULL) {
    giStatus = -20;
    goto error;
   }

   First = 1;
   do {
      // clear buffer
      memset( Buffer, 0, sizeof( Buffer));

      // avoid reading past eof
      if ( FileLength < BUFFSIZE) {
    ReadLength = FileLength;
      }
      else {
    ReadLength = BUFFSIZE;
      }

      // read the file buffer after buffer
      fread( Buffer, 1, ReadLength, InputFile);

      if ( First) {
    //init AES encryption
    InitEnc( pMethod);

    // IV must be saved for decryption afterwards
    fwrite( iv.data, MAX_IV_SIZE, 1, OutputFile);
    // filelength must be saved for decryption afterwards
    fwrite( & FileLength, sizeof( FileLength), 1, OutputFile);

    First = 0;
      }

      // how many blocks to encrypt?
      Blocks = (int) (( ReadLength + AESBLOCKLEN - 1) / AESBLOCKLEN);

      // encrypt
      Enc( Buffer, Blocks);
      // write encrypted buffer
      fwrite( Buffer, ( Blocks * AESBLOCKLEN), 1, OutputFile);

      // update bytes remaining
      FileLength -= BUFFSIZE;

   } while( FileLength > 0);


error:
   // close opened files
   if ( InputFile)  fclose( InputFile);
   if ( OutputFile) fclose( OutputFile);

   if ( giStatus) {
      giStatus -= 5500;                 /* customize error code */
      RaiseError( FALSE);
   }

   updSharedVar();
   return 0;
}
//</code></function>


//=============================================================== EOF BODY ===
/*</body>
</source>
<!-- eof -->*/
