/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="aeslib.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.9 $</revision>
  <archiveTimeStamp>$Date: 2009/01/30 13:56:01 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>AESLIB.C</fileName>
  <creationDate>01/12/2001</creationDate>
  <rights></rights>
  <author>AVN</author>
  <description>
    <fr>Librarie d'encryptage/décryptage et contrôle d'intégrité</fr>
    <nl>Library voor encryptie/decryptie en integriteitskontrole</nl></description>
  <history>
    25/03/2003: JRU - (DEBUG-)Logging to C:\VOTE\MENU.LOG
    12/12/2001: AVN - Implementation of AES encryption algorithm
  </history>
  <comment>Functions for the data enc/decryption and integrity check under Progress runtime.</comment>
</comments>
<header>*/

/*<includes>*/
#include <dos.h>
#include <stdio.h>

#include "aeslib.h"
#include "genlib.h"
/*</includes>*/

AESKEY mKey;                            /* holds master key */
AESKEY sKey;                            /* holds session key */

// :WARNING: avn 20041021 - labels must be regenerated after every election
char* EntropyPool[ENCKEYSMAX + 1] =
{
   "ufgle(artystee1eci1i@m$dualhUrlg",    // URNEXE
   "dfdtyabjecmpinthondgddfGefoLlydi",    // MAVEXE
   "sdgyr&fdg1lK0erfudherm^ùcohenvan",    // GENDAT
   "g§(hirsdfcvrfGgùpµpmheloulkbarne",    // URNVOT
   "yemendewargimlkKlkkmfbfgaletfdgd",    // URNTOT
   "hiltseafghfoaxskIkKoladenpiperab",    // MAVVOT

   "motfrbarKsnowymolauèitran3ed!rep",    // PRPTOT
   "chrtHtrd[rtvuyaidcaGeylfdghdtran",    // URNCHK
   "duckahmyifdgdfRtedgaRmasonqtsoli",    // RECEXE

   "knfdgfyç!fghgj45ncraSch0ifghhihu",    // PRPEXP

   "Qp645t0ke3shapekathYa)eticfe+'ra",    // RESMAC

   "kllsdflAnzombaHurtrideb(fghftyfg"     // NIVMAT
};
#define NIVMAT ENCKEYSMAX
// :WARNING: avn 20041021 - same labels must be used both in cpp and Progress

AESKEY encKeys [ENCKEYSMAX];            /* holds all the encryption keys derived from */
               /*  master and session key */

AESIV iv;                               /* holds hashed IV */

keyInstance encKeyInst;                 /* state of the encryption engine */
cipherInstance encCipherInst;           /* specifies the encryption mode */

keyInstance decKeyInst;                 /* state of the decryption engine */
cipherInstance decCipherInst;           /* specifies the decryption mode */

BYTE Result_Key [AESMACLEN+1];          /* holds temporary result keys cfr. Mac() */

char gcGlobalsFileName[] = "GLOBALS.BIN";

/* Error handling */
static int iRet = 0;                /* Holds errorcode to write back to Progress */
                /*  errorcodes are always negative,
               first digits identify function,
               last two identify error */

typedef enum { EMPTY, OK, NOK, ERR } RESULTS;

typedef enum {
   UNDEFINED   = 0,           // 0 = state at startup,
   INITIATED   = 1,           // 1 = initiated,
   OPERATIONAL = 2            // 2 = operational (enc keys generated) */
   } AES_STATE;

static int iAesLibStatus = 0;           // holds state of AesLib

static char gcLibName[] = "AesLib";

/*</header>
<body>*/

/*<function id="Mac">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int Mac ( int Method, BYTE * Text, int TextLen)
/*==========================================================================*/
{
   if ( MAVVOT == Method)
   {
      sprintf( gcMessage, "Mac : encryption method [%i] not supported.", Method);
      LogError();
      return -1010;
   }
   iRet = computeMac( Result_Key, AESMACLEN, Text, TextLen, encKeys[Method].data);
   if ( iRet < 0)
   {
      sprintf( gcMessage, "Mac : computeMac returns [%i].", iRet);
      LogError();
      return -1020;
   }
   return 0;
}                                      /* eo Mac () */
/*</code></function>

<function id="Dec">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int Dec ( BYTE * Data, int N_Blocks)
/*==========================================================================*/
{
   iRet = decryptBlock( Data, &decCipherInst, &decKeyInst, Data, (N_Blocks * AESBLOCKLEN));
   if ( iRet < 0)
   {
      sprintf( gcMessage, "Dec : decryptBlock returns [%i].", iRet);
      LogError();
      return -2010;
   }
   return 0;
}                                      /* eo Dec () */
/*</code></function>

<function id="InitDec">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int InitDec ( int iMethod)
/*==========================================================================*/
{
   /* select an encryption key based upon iMethod and make it binary */
   if ( iMethod < ENCKEYSMAX)
   {
      /* selected encryption key is part of array of encryption keys */
      iRet = makeKeyBinary( &decKeyInst, DIR_DECRYPT, encKeys[iMethod].data);
   }
   else
   {
      /* :KLUDGE: avn 20020107 -> master key used directly!
    We use master key directly as the encryption key because we don't have
    the session key yet (actually we are preparing to decrypt it here! cfr. B006).
    It would be safer to hash some info with the mK to make a 'real' encKey...*/
      iRet = makeKeyBinary( &decKeyInst, DIR_DECRYPT, mKey.data);
   }
   if ( iRet < 0)
   {
      sprintf( gcMessage, "InitDec : makeKeyBinary returns [%i].", iRet);
      LogError();
      return -3010;
   }
   /* make IV binary ( iv.data is assumed to contain original IV)*/
   cipherInitBinary( &decCipherInst, iv.data);
   if ( iRet < 0)
   {
      sprintf( gcMessage, "InitDec : cipherInitBinary returns [%i].", iRet);
      LogError();
      return -3030;
   }
   return 0;
}                                      /* eo InitDec () */
/*</code></function>

<function id="Enc">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int Enc ( BYTE * Data, int N_Blocks)
/*==========================================================================*/
{
    iRet = encryptBlock( Data, &encCipherInst, &encKeyInst, Data, (N_Blocks * AESBLOCKLEN));
    if ( iRet < 0)
    {
       sprintf( gcMessage, "Enc : encryptBlock returns [%i].", iRet);
       LogError();
       return -4010;
    }
    return 0;
}                                      /* eo Enc () */
/*</code></function>

<function id="InitEnc">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int InitEnc ( int iMethod)
/*==========================================================================*/
{
      char  ivMat[ ( AESKEYLEN * 4) + 1];  // holds IV material before hashing
      int n_blocks, ndx;

   /* select an encryption key based upon iMethod and make it binary */
   if ( iMethod < ENCKEYSMAX)
   {
      /* use the encryption key according to the given iMethod */
      iRet = makeKeyBinary( &encKeyInst, DIR_ENCRYPT, encKeys[iMethod].data);
   }
   else
   {
      /* :KLUDGE: avn 20020107 -> master key used directly!
      We use master key directly as the encryption key because we don't have
      the session key yet (actually we are preparing to decrypt it here! cfr. B006).
      It would be safer to hash some info with the mK to make a 'real' encKey...*/
      iRet = makeKeyBinary( &encKeyInst, DIR_ENCRYPT, mKey.data);
   }
   if ( iRet < 0)
   {
      sprintf( gcMessage, "InitEnc : makeKeyBinary returns [%i].", iRet);
      LogError();
      return -5010;
   }

   /* set ivMat depending on used encryption key */
   memset( ivMat, 0, sizeof( ivMat));
   switch ( iMethod)
   {
      case URNVOT:
         // use a constant here to avoid storing this IV every time ( saving memory)
         sprintf( ivMat, "%-32.32s", EntropyPool[NIVMAT]);
         break;
      case PRPTOT:
         // use a constant here to avoid storing this IV every time ( saving memory)
         sprintf( ivMat, "%-32.32s", EntropyPool[PRPTOT]);
         break;
      default:
         sprintf( ivMat, "%-14.14s%-18.18s", GetTimeStamp(), EntropyPool[NIVMAT]);
         break;
   }
   ndx = AESKEYLEN * 2;
   memcpy( ivMat + ndx, mKey.data, AESKEYLEN);
   ndx += AESKEYLEN;
   memcpy( ivMat + ndx, sKey.data, AESKEYLEN);

   /* hash ivMat in order to set iv */
   n_blocks = 4;

   iRet = computeHash( iv.data, MAX_IV_SIZE, ivMat, (n_blocks * AESBLOCKLEN));
   if ( iRet < 0)
   {
      sprintf( gcMessage, "InitEnc : computeHash returns [%i].", iRet);
      LogError();
      return -5020;
   }
   iRet = cipherInitBinary( &encCipherInst, iv.data);
   if ( iRet < 0)
   {
      sprintf( gcMessage, "InitEnc : cipherInitBinary returns [%i].", iRet);
      LogError();
      return -5030;
   }
   return 0;
}                                      /* eo InitEnc () */
/*</code></function>

<function id="MakeEncryptionKeys">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int MakeEncryptionKeys ( void)
/*==========================================================================*/
{
      char KeyMaterial[ ( AESKEYLEN * 2) + 1];
      BYTE buf[ AESKEYLEN * 4];                 // mK + sK + KeyMaterial
      int i;

#if _DEBUGMODE
   LogFunctionId( gcLibName, "MakeEncryptionKeys");
#endif

   /* is AesLib initialized? */
   if ( iAesLibStatus < INITIATED)
   {
      strcpy( gcMessage, "logic error - call InitiateAes() first.");
      LogError();
      return -6010;
   }

   /* make a buf containing mKey+sKey */
   memcpy( buf, mKey.data, AESKEYLEN);
   memcpy( buf+AESKEYLEN, sKey.data, AESKEYLEN);

   /* generate all encryption keys */
   for ( i = 0; i < ENCKEYSMAX; i++)
   {
      if ( i == MAVVOT)
      {
         /* this encryption key is not used under Progress runtime */
         memset( encKeys[MAVVOT].data, 0xFF, AESKEYLEN);
      }
      else
      {
         //!\ aligned on AESKEYLEN multiple
         sprintf( KeyMaterial, "%-32.32s", EntropyPool[i]);
         memcpy( buf+(AESKEYLEN*2), KeyMaterial, AESKEYLEN*2);

         /* hash this to obtain the specific encryption key */
         iRet = computeHash( encKeys[i].data, AESKEYLEN, buf, sizeof(buf));
         if ( iRet < 0)
         {
            sprintf( gcMessage, "computeHash returns [%i].", iRet);
            LogError();
            return -6020;
         }
      }
   } /* eo for */

#if _DEBUGMODE

   /* dump new encryptionkeys */
   sprintf( gcMessage, "mK[%s] sK[%s]",
      mKey.data, sKey.data); LogDebug();

   sprintf( gcMessage, "mKey.data : "); LogHexa( mKey.data, AESKEYLEN); LogDebug();
   sprintf( gcMessage, "sKey.data : "); LogHexa( sKey.data, AESKEYLEN); LogDebug();

   for (i = 0; i < ENCKEYSMAX; i++)
   {
      sprintf( gcMessage, "encKeys[%i].data : ", i);
      LogHexa( encKeys[i].data, AESKEYLEN); LogDebug();
   }
#endif

   iAesLibStatus = OPERATIONAL;        /* remember enc keys are ready */

   return 0;
}                                      /* eo MakeEncryptionKeys () */
/*</code></function>

<function id="AlignText">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int AlignText ( char *txt, int *noBlocks)
/*==========================================================================*/
{
   int i;
   int max = (AESBLOCKLEN * AESBLOCKLEN);
   int len = strlen( txt);

   *noBlocks = ( len - 1 + AESBLOCKLEN) / AESBLOCKLEN; /* no blocks when aligned */

   if ( (*noBlocks * AESBLOCKLEN) > max)                /* check overflow */
   {
      sprintf( gcMessage, "AlignText : overflowed!");
      LogError();
      return -7010;
   }

   i = (len % AESBLOCKLEN); /* length of input text a multiple of AES block length? */
   if (i) { /* if not */
      for ( i = len; i < (*noBlocks * AESBLOCKLEN); i++) {
     txt[i] = '\0'; /* padding with nulls */
      }
   }
   return 0;
}                                      /* eo AlignText () */
/*</code></function>

<function id="InitiateAes">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int InitiateAes ( void)
/*==========================================================================*/
{
   /* working storage */
   FILE *stream;
   int bytes = 256 * sizeof( u32); /* compute size of globals once */

#if _DEBUGMODE
   LogFunctionId( gcLibName, "InitiateAes");
#endif

   /* have we been here before? */
   if ( iAesLibStatus > UNDEFINED)
      return 0;

   /* Store globals on the heap instead of on the stack */
   /* :WARNING: avn 20011214 -> what comes next happens only once! */
   if ( (stream = fopen( gcGlobalsFileName, "rb")) == NULL )
   {
      /* printf( "Cannot open globals datafile."); */
      return -8010;
   }

   Te0 = malloc( bytes);
   if ( Te0 == NULL)
      return -8020;
   else
      fread( Te0, bytes, 1, stream);

   Te1 = malloc( bytes);
   if ( Te1 == NULL)
      return -8021;
   else
      fread( Te1, bytes, 1, stream);

   Te2 = malloc( bytes);
   if ( Te2 == NULL)
      return -8022;
   else
      fread( Te2, bytes, 1, stream);

   Te3 = malloc( bytes);
   if ( Te3 == NULL)
      return -8023;
   else
      fread( Te3, bytes, 1, stream);

   Te4 = malloc( bytes);
   if ( Te4 == NULL)
      return -8024;
   else
      fread( Te4, bytes, 1, stream);

   Td0 = malloc( bytes);
   if ( Td0 == NULL)
      return -8025;
   else
      fread( Td0, bytes, 1, stream);

   Td1 = malloc( bytes);
   if ( Td1 == NULL)
      return -8026;
   else
      fread( Td1, bytes, 1, stream);

   Td2 = malloc( bytes);
   if ( Td2 == NULL)
      return -8027;
   else
      fread( Td2, bytes, 1, stream);

   Td3 = malloc( bytes);
   if ( Td3 == NULL)
      return -8028;
   else
      fread( Td3, bytes, 1, stream);

   Td4 = malloc( bytes);
   if ( Td4 == NULL)
      return -8029;
   else
      fread( Td4, bytes, 1, stream);

   fclose( stream);
   iAesLibStatus = INITIATED;                  /* remember initiate */

#if _DEBUGMODE
   sprintf( gcMessage, "stackavail[%u] iAesLibStatus[%d]",
      stackavail(), iAesLibStatus); LogDebug();
#endif

   return 0;
}                                      /* eo InitiateAes () */
/*</code></function>

<function id="TerminateAes">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
int TerminateAes ( void)
/*==========================================================================*/
{
   /* free heap if allocated before */
   if ( iAesLibStatus > UNDEFINED)
   {
      free ( Te0); free ( Te1); free ( Te2); free ( Te3); free ( Te4);
      free ( Td0); free ( Td1); free ( Td2); free ( Td3); free ( Td4);
   }

   // must initiate again after this
   iAesLibStatus = UNDEFINED;

#if _DEBUGMODE
   LogFunctionId( gcLibName, "TerminateAes");
   sprintf( gcMessage, "stackavail[%u] iAesLibStatus[%d]",
      stackavail(), iAesLibStatus); LogDebug();
#endif

   return 0;
}                                      /* eo TerminateAes () */
/*</code></function>

<function id="getTimeStamp">
<heading>
  <description><en></en></description>
  <parameters>
  </parameters>
  <comment>
  </comment>
</heading>
<code>*/
/*==========================================================================*/
char* getTimeStamp ( void)
/*==========================================================================*/
{
   struct tm *time_now;
   time_t secs_now;
   static char timeStamp[15]="\0";            /* timeStamp format YYYYMMDDHHMMSS + \0 */

   tzset();
   time( &secs_now);
   time_now = localtime( &secs_now);
   strftime( timeStamp, sizeof(timeStamp), "%Y%m%d%H%M%S", time_now);

#if _DEBUGMODE
   /* dump return value */
   sprintf( gcMessage, "getTimeStamp : returns [%s]. \n", timeStamp);
   LogDebug();
#endif

   return ( timeStamp);
}                                      /* eo getTimeStamp () */
/*</code></function>

</body>
</source>
<!-- eof -->*/
