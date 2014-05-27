/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gencryp.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENCRYP.CPP  $</workFile>
<revision>$Revision: 1.10 $</revision>
<workFileTimeStamp>$Modtime:   Jan 26 2005 15:45:04  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENCRYP.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/04 14:23:09 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENCRYP.CPP</fileName>
  <creationDate>Sept 2001</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Librairie de fonctions d'encryptage/décryptage et contrôles d'intégrité</fr>
    <nl>Library functies voor encryptie/decryptie en integriteitscontrole</nl></description>
  <history>
    01/06/93:   revision history starting date
    07/09/93:   function Test_Free_Mem was removed because configurations are
                not identical between machines and thus free memory can vary.
                (reminder: coreleft was used to test free memeory)
    16/12/93:   the Method_MAVVOT was changed because the combination could lead
                to accept a card from another system, only 2 characters from
                the MASTER key were used, which only gives 26 * 26 combinations
    01/09/2001: AVN - Implementation of new AES encryption algorithm</history>
 <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <dos.h>
#include <io.h>                        // filelength(), access()

#include "..\gen\gencryp.h"
#include "..\gen\gentype.h"
#include "..\gen\genflop.h"


#ifdef _APPLICATION_URN
   #include "..\urn\urnmes.h"
#endif

#ifdef _APPLICATION_MAV
   #include "..\mav\mavmes.h"
#endif

#define _DEBUGMODE	0 // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//;TODO: reset between debugs!!!

//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
// these functions are private!
   static char* GetTimeStamp( void);
   static void Calculate_Internal_MAVVOT( void);
   static RET_CODE Read_MAC_On_Floppy( char * File_Name, BYTE * MAC);
//</prototypes>

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
   AESKEY mKey;                            // holds MASTER key
   AESKEY sKey;                            // holds SESSION key

   AESKEY encKeys [N_METHODS];             // holds all the encryption keys derived from
                                           // MASTER and SESSION key

   AESIV iv;                               // holds hashed IV

   keyInstance encKeyInst;                 // state of the encryption engine
   cipherInstance encCipherInst;           // specifies the encryption mode

   keyInstance decKeyInst;                 // state of the decryption engine
   cipherInstance decCipherInst;           // specifies the decryption mode

   BYTE mavSessionKey [DESKEYLEN+1];       // holds the temporary MAV SESSION key
   BYTE Result_Key [AESMACLEN+1];          // holds temporary result keys

   // :WARNING: avn 20041021 - data must be regenerated after every election
   char* EntropyPool[ N_METHODS + 1] =
   {
      "ufgle(artystee1eci1i@m$dualhUrlg",    // URNEXE
      "dfdtyabjecmpinthondgddfGefoLlydi",    // MAVEXE
      "sdgyr&fdg1lK0erfudherm^ùcohenvan",    // GENDAT
      "g§(hirsdfcvrfGgùpµpmheloulkbarne",    // URNVOT
      "yemendewargimlkKlkkmfbfgaletfdgd",    // URNTOT
      "hiltseafghfoaxskIkKoladenpiperab",    // MAVVOT
      "Qp645t0ke3shapekathYa)eticfe+'ra",    // RESMAC

//    "motfrbarKsnowymolauèitran3ed!rep",    // PRPTOT
//    "chrtHtrd[rtvuyaidcaGeylfdghdtran",    // URNCHK
//    "duckahmyifdgdfRtedgaRmasonqtsoli",    // RECEXE
//    "knfdgfyç!fghgj45ncraSch0ifghhihu",    // PRPEXP

      "kllsdflAnzombaHurtrideb(fghftyfg"     // NIVMAT
   };
   #define NIVMAT N_METHODS         // index to iv material in entropy pool
   // :WARNING: avn 20041021 - same data must be used both in cpp and Progress

//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="AlignText" confidential="yes">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype>
      <name>txt</name><description></description></parameter>
    <parameter type="arg">
      <datatype>int*</datatype>
      <name>noBlocks</name><description></description></parameter>
  </parameters>
  <usage></usage>
  <checks></checks>
  <comment></comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void AlignText ( char *txt, int *noBlocks)
// ---------------------------------------------------------------------------
{
    int i;
    int max = (AESBLOCKLEN * AESBLOCKLEN);
    int len = strlen( txt);

    *noBlocks = ( len - 1 + AESBLOCKLEN) / AESBLOCKLEN;     // no blocks when aligned

    if ( (*noBlocks * AESBLOCKLEN)  > max)              // check overflow
        Error_Exit( E_OUT_OF_MEMORY);

    i = (len % AESBLOCKLEN);                // length of input text a multiple of AES block length?
    if (i) {                                                // if not
        for ( i = len; i < (*noBlocks * AESBLOCKLEN); i++) {
            txt[i] = '\0';                                      // padding with nulls
        }
    }

    return;
}
//</code></function>


/*<function id="GetTimeStamp" confidential="yes">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>char*</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype><name></name><description></description></parameter>
  </parameters>
  <usage></usage>
  <checks></checks>
  <comment></comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
char* GetTimeStamp ( void)
// ---------------------------------------------------------------------------
{
       struct tm *time_now;
       time_t secs_now;
       static char timeStamp[15]="\0";              // timeStamp format YYYYMMDDHHMMSS + \0

    tzset();
    time( &secs_now);
    time_now = localtime( &secs_now);
    strftime( timeStamp, sizeof(timeStamp), "%Y%m%d%H%M%S", time_now);
        #if 0 //#if _DEBUGMODE
            sprintf( cDbugBuf, "GetTimeStamp: %s\n", timeStamp); Dbug();
        #endif

    return ( timeStamp);
}
//</code></function>


/*<function id="Calculate_Internal_Keys" confidential="yes">
<heading>
  <description><en>
    Generate the different encryption key values by hashing MASTER and SESSION key</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype><name></name><description></description></parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype>
      <name>encKeys[N_METHODS]</name>
      <description><en>the values to initialize</en></description></parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype>
      <name>mKey</name>
      <description><en>current master key</en></description></parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype>
      <name>sKey</name>
      <description><en>current session key</en></description></parameter>
  </parameters><usage/><checks/>
  <comment>
    This function must be called immediately after the MASTER and
    SESSION keys are known in order to use properly all the other
    encryption functions that need an encryption key.
    The MAVVOT encryption key is not generated because it must be different
    for every magnetic card that is written.
    to the encryption documentation for a full explanation.

    The EntropyPool table is generated as explained at www.diceware.com.
    For each encryption key we generate a 7 word ( aka paranoid) passphrase,
    insert a special character, capitalize one letter and remove all spaces.
    </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Calculate_Internal_Keys ( void)
// ---------------------------------------------------------------------------
{
      char KeyMaterial[ ( AESKEYLEN * 2) + 1];
      BYTE buf[ AESKEYLEN * 4];                 // mK + sK + KeyMaterial
      int i;

   #if _DEBUGMODE
      int ret;
      sprintf( cDbugBuf, "Calculate_Internal_Keys:\n"); Dbug( );
      sprintf( cDbugBuf, "mKey.data: "); DbugHex( mKey.data, AESKEYLEN);
      sprintf( cDbugBuf, "sKey.data: "); DbugHex( sKey.data, AESKEYLEN);
   #endif

   // make a buf containing mKey+sKey
   memcpy( buf, mKey.data, AESKEYLEN);
   memcpy( buf+AESKEYLEN, sKey.data, AESKEYLEN);

   for ( i = 0; i < N_METHODS; i++)
   {
      if ( i == MAVVOT )
      {
         // MAVVOT encryption key is regenerated for every magnetic card that is written
         // see Generate_Mav_Session() function for details
         Generate_Mav_Session();
         Calculate_Internal_MAVVOT();
      }
      else
      {
         //!\ aligned on AESKEYLEN multiple
         sprintf( KeyMaterial, "%-32.32s", EntropyPool[i]);
         memcpy( buf+(AESKEYLEN*2), KeyMaterial, AESKEYLEN*2);

         #if _DEBUGMODE
            sprintf( cDbugBuf, "KeyMaterial:[%s] len[%d]\n",
               KeyMaterial, strlen( KeyMaterial)); Dbug( );

             ret =
         #endif

         // hash this to obtain the specific encryption key
         computeHash( encKeys[i].data, AESKEYLEN, buf, sizeof( buf));

         #if 0 //#if _DEBUGMODE
             if (ret < 0) {
                 sprintf( cDbugBuf, "computeHash aborts [%i].\n", ret); Dbug();
             }
         #endif
      }

      #if _DEBUGMODE
          // dump encryption key data
          sprintf( cDbugBuf, "encKeys[%d].data:", i);
          DbugHex( encKeys[i].data, AESKEYLEN);
      #endif
   } // eofor

   #if _DEBUGMODE
      sprintf( cDbugBuf, "close"); Dbug( );
   #endif

   return;
}
//</code></function>


/*<function id="Calculate_Internal_MAVVOT" confidential="yes">
<heading>
  <description><en>
    Generate the actual encryption key value for the magnetic card data</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype><name></name><description></description></parameter>
    <parameter type="global">
      <datatype>BYTE</datatype>
      <name>mavSessionKey[DESKEYLEN+1]</name>
      <description><en></en></description></parameter>
  </parameters><usage/><checks/>
  <comment>
    This encryption key is just used to MAC the card data. It is not
    possible to encrypt the card data due to the card capacity.</comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Calculate_Internal_MAVVOT ( void)
// ---------------------------------------------------------------------------
{
      char KeyMaterial[ ( AESKEYLEN * 2) + 1];
      BYTE buf[ AESKEYLEN * 4];
      int i;

   // prepare keymaterial aligned on AESKEYLEN multiple
   strcpy( KeyMaterial, mavSessionKey);
   sprintf( KeyMaterial+DESKEYLEN, "%-24.24s", EntropyPool[MAVVOT]);

   #if _DEBUGMODE
      sprintf( cDbugBuf, "Calculate_Internal_MAVVOT:\n"); Dbug();
      sprintf( cDbugBuf, "KeyMaterial:[%s] len[%d]\n",
         KeyMaterial, strlen( KeyMaterial)); Dbug( );
   #endif

   // prepare the hash buffer containing mKey + sKey + mavSessionKey + entropy
   memcpy( buf,                  mKey.data,     AESKEYLEN);
   memcpy( buf+AESKEYLEN,        sKey.data,     AESKEYLEN);
   memcpy( buf+( AESKEYLEN * 2), KeyMaterial,   AESKEYLEN * 2);

   #if 0 //#if _DEBUGMODE
      int ret;
      ret =
   #endif

   //hash this to obtain the specific encryption key
   computeHash( encKeys[MAVVOT].data, AESKEYLEN, buf, sizeof( buf));

   #if 0 //#if _DEBUGMODE
       if (ret < 0) {
           sprintf( cDbugBuf, "computeHash aborts [%i].\n", ret); Dbug();
       }
   #endif

   return;
}
//</code></function>


/*<function id="InitEnc" confidential="yes">
<heading>
  <description><en>Initialise AES for encryption : make encryption key binary,
    generate an IV and make this IV binary.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>method</name>
      <description><en>encryption key to make binary</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESIV</datatype><name>iv</name>
      <description><en>holds hashed IV</en></description></parameter>
  </parameters>
  <usage>
    InitEnc( GENDAT);               //init AES encryption</usage>
  <checks></checks>
  <comment>
    Refer to the encryption documentation for a full explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void InitEnc ( int iMethod)
// ---------------------------------------------------------------------------
{
      char  ivMat[ ( AESKEYLEN * 4) + 1];  // holds IV material before hashing
      int   n_blocks, ndx;

    #if _DEBUGMODE
           int ret;
        sprintf( cDbugBuf, "InitEnc:\n"); Dbug();
    #endif

    if ( iMethod < N_METHODS)
    {
        // use the encryption key according to the given method
        #if 0 //#if _DEBUGMODE
           ret =
        #endif
        makeKeyBinary( &encKeyInst, DIR_ENCRYPT, encKeys[iMethod].data);
    }
    else
    {
        // else use MASTER key as encryption key (cfr. B006)
        #if 0 //#if _DEBUGMODE
           ret =
        #endif
        makeKeyBinary( &encKeyInst, DIR_ENCRYPT, mKey.data);
    }

        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, " makeKeyBinary returns error code [%i]", ret); Dbug();
            }
        #endif

    //init iv material
    memset( ivMat, NULL, sizeof( ivMat));
    if ( iMethod == URNVOT)
    {
       // use a constant here to avoid storing this IV every time ( saving memory)
       sprintf( ivMat, "%-32.32s", EntropyPool[NIVMAT]);
    }
    else
    {
        sprintf( ivMat, "%-14.14s%-18.18s", GetTimeStamp(), EntropyPool[NIVMAT]);
    }
    ndx = AESKEYLEN * 2;
    memcpy( ivMat + ndx, mKey.data, AESKEYLEN);
    ndx += AESKEYLEN;
    memcpy( ivMat + ndx, sKey.data, AESKEYLEN);

    #if 0 //#if _DEBUGMODE
       sprintf( cDbugBuf, "InitEnc - ivMat:[%s] len[%d]\n",
          ivMat, strlen( ivMat)); Dbug( );
    #endif

    n_blocks = 4;

        #if 0 //#if _DEBUGMODE
            ret =
        #endif

    computeHash( iv.data, MAX_IV_SIZE, ivMat, (n_blocks * AESBLOCKLEN));

        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, " computeHash returns error code [%i].\n", ret); Dbug();
            }
            //dump iv data
            sprintf( cDbugBuf, " iv.data:"); Dbug();
            DbugHex( iv.data, AESKEYLEN);

            ret =
        #endif

    cipherInitBinary( &encCipherInst, iv.data);

        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, " cipherInitBinary returns error code [%i]", ret); Dbug();
            }
        #endif

    return;
}
//</code></function>


/*<function id="Encrypt" confidential="yes">
<heading>
  <description><en>Encrypt the given data, the encrypted data is returned in place.
    The encryption method has been set by the InitEnc() function!</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Data</name>
      <description><en>
        the data to encrypt (AESBLOCKLEN boundary)</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>N_Blocks</name>
      <description><en>
        the number of blocks of AESBLOCKLEN constituing the data</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description><en>holds all the encryption keys derived from MASTER and
      SESSION key</en></description>
    </parameter>
  </parameters>
  <usage>
    Encrypt( "0123456789ABCDEF", 1);</usage>
  <checks></checks>
  <comment>
    If the method is MAVVOT (always another MAV_SESSION key) we
    must first compute the internal keys, otherwise these keys are
    already known.
    The data must be completed with 0 to reach a multiple of AESBLOCKLEN length.
    Refer to the encryption documentation for a full explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Encrypt ( BYTE * Data, int N_Blocks)
// ---------------------------------------------------------------------------
{
   #if 0 //#if _DEBUGMODE
      int ret;

      sprintf( cDbugBuf, " Encrypt:"); Dbug();
      DbugHex( Data, N_Blocks * AESBLOCKLEN);

      ret =
   #endif

         encryptBlock( Data, &encCipherInst, &encKeyInst, Data, (N_Blocks * AESBLOCKLEN));

   #if 0 //#if _DEBUGMODE
      if (ret < 0) {
          sprintf(cDbugBuf, " returns error code [%i]", ret); Dbug();
      }
      //dump encrypted data
      //sprintf( cDbugBuf, " Data:"); Dbug();
      //DbugHex( Data, N_Blocks * AESBLOCKLEN);
   #endif

   return;
}
//</code></function>


/*<function id="InitDec" confidential="yes">
<heading>
  <description><en>
    Initialises AES for decryption, making encryption key and
    IV binary.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>method</name>
      <description><en>determines the encryption key that must be made
      binary</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESIV</datatype><name>iv</name>
      <description><en>holds hashed IV</en></description></parameter>
  </parameters>
  <usage>
    memcpy( iv.data, Buffer, MAX_IV_SIZE);
    InitDec( SD.mnemo);             //init AES decryption</usage>
  <checks></checks>
  <comment>
    The encryption key is passed as an argument because it can be one
    of the internal encryption keys (depending on the data being decrypted)
    or the MASTER key (when decrypting the SESSION key).
    Refer to the encryption documentation for a full explanation.</comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void InitDec ( int method)
// ---------------------------------------------------------------------------
{

    #if _DEBUGMODE
           int ret;

        sprintf( cDbugBuf, "InitDec:\n"); Dbug();
    #endif

    if ( method < N_METHODS)
    {
        // use the encryption key according to the given method
        #if 0 //#if _DEBUGMODE
            ret =
        #endif
        makeKeyBinary( &decKeyInst, DIR_DECRYPT, encKeys[method].data);
    }
    else
    {
        // else use MASTER key as encryption key (cfr. B006)
        #if 0 //#if _DEBUGMODE
            ret =
        #endif
        makeKeyBinary( &decKeyInst, DIR_DECRYPT, mKey.data);
    }

        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, "makeKeyBinary returns error code [%i]", ret); Dbug();
            }

            ret =
        #endif

    cipherInitBinary( &decCipherInst, iv.data); //iv.data is assumed to contain original IV

        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, " cipherInitBinary returns error code [%i]", ret); Dbug();
            }
            //dump iv data
            sprintf( cDbugBuf, " iv.data:"); Dbug();
            DbugHex( iv.data, AESKEYLEN);
        #endif

    return;
}
//</code></function>


/*<function id="Decrypt" confidential="yes">
<heading>
  <description><en>decrypt the given data with the given method, the
    decrypted data is returned in place</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Data</name>
      <description><en>the data to decrypt (AESBLOCKLEN boundary)</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>N_Blocks</name>
      <description><en>the number of blocks of AESBLOCLEN bytes constituing
      the data</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description><en>holds all the encryption keys derived from MASTER
      and SESSION key</en></description>
    </parameter>
  </parameters>
  <usage>
    Decrypt( "0123456789ABCDEF", 1);</usage>
  <checks></checks>
  <comment>
    If the method is MAVVOT (always another MAV_SESSION key) we
    must first compute the internal keys, otherwise these keys are
    already known.
    The data is normaly a multiple of AESBLOCKLEN because it is the result of
    a previous encryption.
    Refer to the encryption documentation for a full explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Decrypt ( BYTE * Data, int N_Blocks)
// ---------------------------------------------------------------------------
{

   #if 0 //#if _DEBUGMODE
          int ret;

      ret =
   #endif

   decryptBlock( Data, &decCipherInst, &decKeyInst, Data, (N_Blocks * AESBLOCKLEN));

   #if 0 //#if _DEBUGMODE
      if (ret < 0) {
          sprintf(cDbugBuf, " returns error code [%i]", ret); Dbug();
      }
      sprintf( cDbugBuf, " Decrypted:"); Dbug();
      DbugHex( Data, N_Blocks * AESBLOCKLEN);
   #endif

   return;
}
//</code></function>


/*<function id="MAC" confidential="yes">
<heading>
  <description><en>
    compute a MAC on the data with the given method, the MAC is stored
    in the Result_Key array</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Method</name>
      <description><en>the encryption key to use</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Text</name>
      <description><en>the text on which to compute the MAC</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>TextLen</name>
      <description><en>
        size of the text in bytes</en></description></parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description><en>holds all the encryption keys derived from master and
      session key</en></description>
    </parameter>
    <parameter type="global">
      <datatype>BYTE</datatype><name>Result_Key[AESMACLEN+1]</name>
      <description><en>holds the computed MAC</en></description></parameter>
  </parameters>
  <usage>
    MAC( SD.mnemo, iv.data, 1);
    memcpy( Mac.data, Result_Key, AESMACLEN));</usage>
  <checks></checks>
  <comment>
    If the method is MAVVOT (always another MAV_SESSION key) we
    must first compute the internal keys, otherwise these keys are
    already known.
    The data must be completed with 0 to reach a multiple of AESBLOCKLEN length.
    Refer to the encryption documentation for a full explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void MAC ( int Method, BYTE * Text, int TextLen)
// ---------------------------------------------------------------------------
{
    if (Method == MAVVOT) {
        // the encryption key is not known yet
        Calculate_Internal_MAVVOT();
    }

    int ret;
        #if _DEBUGMODE
            sprintf( cDbugBuf, "MAC Method:[%d], Text:[], TextLen:[%d]\n", Method, TextLen); Dbug();
        #endif

    computeMac( Result_Key, AESMACLEN, Text, TextLen, encKeys[Method].data);
        #if 0 //#if _DEBUGMODE
            if (ret < 0) {
                sprintf( cDbugBuf, " computeMac returns error code [%i].\n", ret); Dbug();
            }
            //dump MAC data
            sprintf( cDbugBuf, " Result_Key:"); Dbug();
            DbugHex( Result_Key, AESMACLEN);
        #endif

    return;
}
//</code></function>


/*<function id="Generate_Mav_Session" confidential="yes">
<heading>
  <description><en>
    generates randomly a new MAV session key; this key will be used
    to compute the MAC for the vote on the card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="global">
      <datatype>BYTE</datatype><name>mavSessionKey[DESKEYLEN+1]</name>
      <description></description></parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
    To prevent the random of giving always the same sequence we
    reinitialize the random genarator each time we call the function.
    Because the MAVSESSION key must be written on the magnetic card,
    and because we will'nt use more than 8 card positions to store
    the key on card, we can only use a combination of card writable
    values. These values are 10, from ascii '0' to '9'.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Generate_Mav_Session ()
// ---------------------------------------------------------------------------
{
   randomize();                         // initializes random number generator

   for( int i=0; i < DESKEYLEN; i++)
      mavSessionKey [i] = random( 10) + '0';       // '0' to '9' is possible

   mavSessionKey [i] = 0;
}
//</code></function>


/*<function id="Read_Key_On_Floppy" confidential="yes">
<heading>
  <description><en>
    reads IV + key (n unsigned chars) from a file on floppy</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = key was correctly read,
      ABORTED = key was not read.</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>
        the file that holds the key</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Key</name>
      <description>
        <en>an array of unsigned char to store the read key</en></description>
    </parameter>
  </parameters>
  <usage>
    if( Read_Key_On_Floppy( FILE_NAME, Key) == ABORTED)</usage>
  <checks>
    every Floppy function return code is tested</checks>
  <comment>
    because Floppy_Read_Buffer may modify the lenght of the data to
    read we can't pass the value directly, so we use a
    local variable for that purpose</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Read_Key_On_Floppy ( char * File_Name, BYTE * Key)
// ---------------------------------------------------------------------------
{
   int File;                                                // the file handle
   unsigned Data_Length = MAX_IV_SIZE+AESKEYLEN;            // length to read (iv + key)

   if( Floppy_Open_File( File_Name, & File) == ABORTED || File == -1)
      return ABORTED;

   if( Floppy_Read_Buffer( File, & Data_Length, Key) == ABORTED)
      return ABORTED;

   if( Floppy_Close_File( File) == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Read_MAC_On_Floppy" confidential="yes">
<heading>
  <description>
    <en>reads MAC (AESMACLEN unsigned chars) from a file on floppy</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = key was correctly read,
      ABORTED = key was not read.</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description>
        <en>the file that holds the key</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>MAC</name>
      <description>
        <en>an array of unsigned char to store the read MAC</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    if( Read_Key_On_Floppy( FILE_NAME, MAC) == ABORTED)</usage>
  <checks>
    every Floppy function return code is tested</checks>
  <comment>
    because Floppy_Read_Buffer may modify the lenght of the data to
    read we can't pass the value directly, so we use a
    local variable for that purpose</comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Read_MAC_On_Floppy ( char * File_Name, BYTE * MAC)
// ---------------------------------------------------------------------------
{
   int File;                                                // the file handle
   unsigned Data_Length = AESMACLEN;                        // length to read (MAC)

   if( Floppy_Open_File( File_Name, & File) == ABORTED || File == -1)
      return ABORTED;

   if( Floppy_Read_Buffer( File, & Data_Length, MAC) == ABORTED)
      return ABORTED;

   if( Floppy_Close_File( File) == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Verify_Password" confidential="yes">
<heading>
  <description><en>verifies if a given password is correct and sets the
    mKey.data value accordingly</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = Password is correct,
      ABORTED = Password is not correct.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Password</name>
      <description><en>a string of AESKEYLEN unsigned chars (MASTER_Key)</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>mKey</name>
      <description><en>holds MASTER key</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>sKey</name>
      <description><en>holds SESSION key</en></description>
    </parameter>
  </parameters>
  <usage>if( Verify_Password( "azertyui") == ABORTED)</usage>
  <checks></checks>
  <comment>
    If the mKey.data is already know, the function only perform a
    comparison between the given Password and the mKey.data.
    Otherwise, the function checks the password by testing the
    executable file's mac with what is supposed to be the mKey.data.
    The password is supposed to be the mKey.data, so we compute all
    the encKeys because there is only a little difference between
    computing all the encKeys and only one.
    If the password is not correct we only reset the mKey.data.
    If _TEST was defined at compile time, the executables are not
    checked, so there is no password check. But if the password was
    not good there will be an error while reading the datas.
    Some part of the code is dedicated to a particular application,
    (MAV, URN or REC) this is done by #ifdef .... #endif</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Verify_Password ( char * Password)
// ---------------------------------------------------------------------------
{
        BYTE record[(MAX_IV_SIZE+AESKEYLEN)];   // holds iv + crypted session key

   if( strlen( mKey.data))           // the correct password is already known
   {
      if( ! strncmp( Password, mKey.data, AESKEYLEN) )
         return SUCCEEDED;
      else
         return ABORTED;
   }
                                           // verification will take some time
#ifdef _APPLICATION_MAV
   Display_Message( M_WAIT, NO, 0, NO, 9, 10, 1, 1);
#else
   Display_Message( M_WAIT);
#endif

   memcpy( mKey.data, Password, AESKEYLEN+1);     // we suppose the password is correct
                                         // so we already set the mKey.data

   // we need the Key_SESSION from the floppy

   if( Read_Key_On_Floppy( KEY_SESSION_FILE, record) == ABORTED)
   {
      // the disk is not correct, so we must reset the mKey.data
      memset( mKey.data, 0, AESKEYLEN+1);
      return ABORTED;
   }

    memcpy( iv.data, record, MAX_IV_SIZE);                      //set IV
    InitDec( N_METHODS);                                        //init AES decryption engine

    // 20040322 avn - contribution from review by Paul at www.afront.be ;-)
    memcpy( sKey.data, &record[MAX_IV_SIZE], AESKEYLEN);        //set encrypted key

    Decrypt( sKey.data, (AESKEYLEN / AESBLOCKLEN));             //decrypt SESSION key

   Calculate_Internal_Keys();   // we still suppose that the given Password is
                                // correct, so we already compute the
                                // Internal_Keys with it rather than first
                                // computing a partial Internal_Keys struct

#ifndef _TEST                  // the code will disappear if no test is wanted

 #ifdef _APPLICATION_URN
  #ifndef _RECOUNT

   if( Test_Executable( URNEXE, URNEXE_FILE, URNEXE_MAC_FILE) == ABORTED)

  #else

   if( Test_Executable( URNEXE, RECEXE_FILE, RECEXE_MAC_FILE) == ABORTED)

  #endif  // _RECOUNT
 #endif   // _APPLICATION_URN

 #ifdef _APPLICATION_MAV

   // authenticate datafile and executable
   if ( Test_Executable( GENDAT, "B021", "B121") == ABORTED ||
      Test_Executable( MAVEXE, MAVEXE_FILE, MAVEXE_MAC_FILE) == ABORTED)

 #endif   // _APPLICATION_MAV

   {
      // if the password is not correct, first reinitialize it
      memset( mKey.data, 0, AESKEYLEN+1);
      return ABORTED;
   }

#endif    // _TEST

   return SUCCEEDED;
}
//</code></function>


/*<function id="Test_Executable" confidential="yes">
<heading>
  <description><en>
    the function computes the MAC of the executable file with the
    given method and compares the result with the MAC stored in
    the MAC file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = the MAC computed on the executable file
      corresponds to that in the MAC file, ABORTED = the MAC does not
      correspond.</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Method</name>
      <description><en>
        the method to use to combine the MASTER and SESSION keys</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Exe_File</name>
      <description><en>
        the name of the executable file</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Mac_File</name>
      <description><en>
        the name of the file containing the correct MAC</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description><en>holds all the encryption keys derived from MASTER and
      SESSION key</en></description></parameter>
  </parameters>
  <usage>
    if( Test_Executable( URNEXE, "urn.exe", "b012") == ABORTED)</usage>
  <checks>
    all the Floppy routines are checked for error, if one occurs then
    the disk is not usable</checks>
  <comment>
    Refer to the encryption documentation for a full explanation
    of how to compute the MAC of an executable file.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Test_Executable ( int Method, char * Exe_File, char * Mac_File)
// ---------------------------------------------------------------------------
{
   const int BUFFSIZE = 512;                              // the buffer length
   unsigned Length;                                         // the read length
   long File_Length;                                        // the file length
   int File;                                                // the file handle
   BYTE Buffer[BUFFSIZE];                             // used to read the file
   BYTE Mac[AESMACLEN+1];                                      // the MAC read from file
   BYTE Macr[4]="0000", Maci[4], Resr[4]="0000", Resi[4];  // the computed MAC
   int   Status = 0;
   int   i;

   if ( Floppy_Open_File( Exe_File, & File) == ABORTED || File == -1 ||
      Floppy_File_Length( File, & File_Length) == ABORTED)
   {
      // file does'nt exist or is corrupted
      Status = 10;
      goto error;
   }

   do
   {
      // read the file buffer after buffer, compute the MAC for each buffer,
      // and make the needed combinations between each step

      if ( File_Length < BUFFSIZE)
         Length = File_Length;
      else
         Length = BUFFSIZE;

      if ( Floppy_Read_Buffer( File, & Length, Buffer) == ABORTED)
      {
         Status = 20;
         goto error;
      }

      File_Length -= BUFFSIZE;

        MAC( Method, Buffer, Length);
        memcpy( Maci, Result_Key, 4);           // this way we can keep rest of the old code unchanged
        memcpy( Resi, Result_Key+4, 4);

      for( int i = 0; i < 4; i++)
      {
         Macr[i] ^= Maci[i];
         Resr[i] ^= Resi[i];
      }

   } while( File_Length > 0);

   // now read the MAC from the file
   if ( Floppy_Close_File( File) == ABORTED ||
       Read_MAC_On_Floppy( Mac_File, Mac) == ABORTED)
   {
      Status = 30;
      goto error;
   }

   // compare the two MACs
   for( i = 0; i < 4; i++)
   {
      if( Macr[i] != Mac[i]  ) return ABORTED;
      if( Resr[i] != Mac[i+4]) return ABORTED;
   }


   return SUCCEEDED;

error:
   if ( Status)
   {
      Status = - Status;
      #if _DEBUGMODE
         sprintf( gcDcio, " TEX Aborted - status[%d]", Status); dcio();
      #endif // _DEBUGMODE
   }
   Error_Exit( M_BAD_DISK);

   return ABORTED;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="EncryptBinaryFile">
<heading>
  <description>
    <en>Encrypts a binary file.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description>
        <en>SUCCEEDED = encryption succeeded, ABORTED = failed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>pMethod</name>
      <description><en>encryption key id</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pInputFileName</name>
      <description><en>name source file</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pOutputFileName</name>
      <description><en>name destination file</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description>
        <en>holds all the encryption keys derived from MASTER and SESSION
        key</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE EncryptBinaryFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   )
// ---------------------------------------------------------------------------
{
      long     FileLength;             // file length in bytes
      int      ReadLength;             // # bytes to read
      FILE*    InputFile;              // file handle's
      FILE*    OutputFile;
      BOOL     First;                  // flag's first read
      int      Blocks;                 // # blocks to encrypt

      const int BUFFSIZE = 512;        // the buffer length
      BYTE Buffer[BUFFSIZE];           // read buffer
      int Status = 0;                  // error code


   // open input file
   InputFile = fopen( pInputFileName, "rb");
   if( InputFile == NULL)
   {
         #if 0 //#if _DEBUGMODE
            sprintf( gcDcio, "EBF - %s cannot open!", pInputFileName); dcio();
         #endif // _DEBUGMODE
         Status = -10;
         goto error;
   }
   else
   {
       FileLength = filelength( fileno( InputFile));
   }


   #if 0 //#if _DEBUGMODE
      sprintf( cDbugBuf, "EBF - k[%d] src[%s] dst[%s] len[%d]\n",
         pMethod, pInputFileName, pOutputFileName, FileLength); Dbug();
   #endif

   //check for existence of output file
   if ( !access( pOutputFileName, 0x00))
   {
      if ( remove( pOutputFileName))       //remove output file
      {
         #if 0 //#if _DEBUGMODE
            sprintf( gcDcio, "EBF - Error removing %s!", pOutputFileName); dcio();
         #endif // _DEBUGMODE
         fclose( InputFile);
         Status = -12;
         goto error;
      }
   }

   //create binary output file
   if( ( OutputFile = fopen( pOutputFileName, "wb")) == NULL)
   {
         #if 0 //#if _DEBUGMODE
            sprintf( gcDcio, "EBF - %s cannot open!", pOutputFileName); dcio();
         #endif // _DEBUGMODE
         fclose( InputFile);
         Status = -20;
         goto error;
   }

   First = TRUE;
   do
   {
      // clear buffer
      memset( Buffer, NULL, sizeof( Buffer));

      // avoid reading past eof
      if ( FileLength < BUFFSIZE)
      {
         ReadLength = FileLength;
      }
      else
      {
         ReadLength = BUFFSIZE;
      }

      // read the file buffer after buffer
      fread( Buffer, ReadLength, 1, InputFile);

      if ( First)
      {
         //init AES encryption
         InitEnc( pMethod);

         // IV must be saved for decryption afterwards
         fwrite( iv.data, MAX_IV_SIZE, 1, OutputFile);
         #if 0 //#if _DEBUGMODE
            //dump iv data
            sprintf( cDbugBuf, " EBF - iv.data:"); Dbug();
            DbugHex( iv.data, AESKEYLEN);
         #endif

         // filelength must be saved for decryption afterwards
         fwrite( & FileLength, sizeof( FileLength), 1, OutputFile);
         #if 0 //#if _DEBUGMODE
            //dump filelength
            sprintf( cDbugBuf, " EBF - filelength:"); Dbug();
            DbugHex( (BYTE*) & FileLength, sizeof( FileLength));
         #endif

         First = FALSE;
      }

      // how many blocks to encrypt?
      Blocks = (int) (( ReadLength + AESBLOCKLEN - 1) / AESBLOCKLEN);
      #if 0 //#if _DEBUGMODE
         sprintf( cDbugBuf, " EBF blocks:[%d] readlen[%d]", Blocks, ReadLength); Dbug();
      #endif

      // encrypt
      Encrypt( Buffer, Blocks);
      // write encrypted buffer
      fwrite( Buffer, ( Blocks * AESBLOCKLEN), 1, OutputFile);

      // update bytes remaining
      FileLength -= BUFFSIZE;

   } while( FileLength > 0);

   // close files
   fclose( OutputFile);
   fclose( InputFile);

error:
   if ( Status)
   {
      #if 0 //#if _DEBUGMODE
         sprintf( gcDcio, " EBF Aborted status[%d]", Blocks, Status); dcio();
         sprintf( cDbugBuf, " EBF Aborted status[%d]", Blocks, Status); Dbug();
      #endif // _DEBUGMODE

      return ABORTED;
   }
   #if 0 //#if _DEBUGMODE
      sprintf( cDbugBuf, " EBF Succeeded"); Dbug();
   #endif

   return SUCCEEDED;
}
//</code></function>


/*<function id="DecryptBinaryFile">
<heading>
  <description>
    <en>Decrypts a binary file.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description>
        <en>SUCCEEDED = decryption succeeded, ABORTED = failed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>pMethod</name>
      <description><en>encryption key id</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pInputFileName</name>
      <description><en>name source file</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pOutputFileName</name>
      <description><en>name destination file</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description>
        <en>holds all the encryption keys derived from MASTER and SESSION
        key</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE DecryptBinaryFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   )
// ---------------------------------------------------------------------------
{
      long     FileLength;             // original file length in bytes
      int      ReadLength;             // # bytes to read
      FILE*    InputFile;              // file handle's
      FILE*    OutputFile;
      BOOL     First;                  // flag's first read
      int      Blocks;                 // # blocks to encrypt

      const int BUFFSIZE = 512;        // the buffer length
      BYTE Buffer[BUFFSIZE];           // read buffer
      int Status = 0;                  // error code


   #if 0 //#if _DEBUGMODE
      sprintf( cDbugBuf, "DBF - k[%d] src[%s] dst[%s]\n",
         pMethod, pInputFileName, pOutputFileName); Dbug();
   #endif

   // open input file
   InputFile = fopen( pInputFileName, "rb");
   if ( InputFile == NULL)
   {
      #if 0 //#if _DEBUGMODE
         sprintf( gcDcio, "DBF - %s cannot open!", pInputFileName); dcio();
      #endif // _DEBUGMODE
      Status = -10;
      goto error;
   }

   //check for existence of output file
   if ( !access( pOutputFileName, 0x00))
   {
      if ( remove( pOutputFileName))       //remove output file
      {
         #if 0 //#if _DEBUGMODE
            sprintf( gcDcio, "DBF - Error removing %s!", pOutputFileName); dcio();
         #endif // _DEBUGMODE
         fclose( InputFile);
         Status = -12;
         goto error;
      }
   }

   //create binary output file
   if( ( OutputFile = fopen( pOutputFileName, "wb")) == NULL)
   {
         #if 0 //#if _DEBUGMODE
            sprintf( gcDcio, "DBF - %s cannot open!", pOutputFileName); dcio();
         #endif // _DEBUGMODE
         fclose( InputFile);
         Status = -20;
         goto error;
   }

   First = TRUE;
   do
   {
      // clear buffer
      memset( Buffer, NULL, sizeof( Buffer));

      if ( First)
      {
         // read IV back and initialize decryption engine
         fread( iv.data, MAX_IV_SIZE, 1, InputFile);
         #if 0 //#if _DEBUGMODE
            //dump iv data
            sprintf( cDbugBuf, " DBF - iv.data:"); Dbug();
            DbugHex( iv.data, AESKEYLEN);
         #endif

         InitDec( pMethod);

         // read the original filelength back
         fread( & FileLength, sizeof( FileLength), 1, InputFile);

         First = FALSE;
      }

      // avoid reading past eof
      if ( FileLength < BUFFSIZE)
      {
         ReadLength = FileLength;
      }
      else
      {
         ReadLength = BUFFSIZE;
      }

      // how many blocks to read/decrypt?
      Blocks = (int) (( ReadLength + AESBLOCKLEN - 1) / AESBLOCKLEN);

      // read the file buffer after buffer
      fread( Buffer, Blocks * AESBLOCKLEN, 1, InputFile);

      #if 0 //#if _DEBUGMODE
         sprintf( cDbugBuf, " DBF blocks:[%d] rl[%d] al[%d]",
            Blocks, ReadLength, Blocks *  AESBLOCKLEN); Dbug();
      #endif

      // decrypt
      Decrypt( Buffer, Blocks);
      // write decrypted buffer, truncate at orginale file size at the end
      fwrite( Buffer, ReadLength, 1, OutputFile);

      // update bytes remaining
      FileLength -= BUFFSIZE;

   } while( FileLength > 0);

   // close files
   fclose( OutputFile);
   fclose( InputFile);

error:
   if ( Status)
   {
      #if 0 //#if _DEBUGMODE
         sprintf( gcDcio, " DBF Aborted status[%d]", Blocks, Status); dcio();
         sprintf( cDbugBuf, " DBF Aborted status[%d]", Blocks, Status); Dbug();
      #endif // _DEBUGMODE

      return ABORTED;
   }
   #if 0 //#if _DEBUGMODE
      sprintf( cDbugBuf, " DBF Succeeded", Blocks); Dbug();
   #endif

   return SUCCEEDED;
}
//</code></function>


/*<function id="CreateMacFile">
<heading>
  <description>
    <en>Compute and store a MAC in a file.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description>
        <en>SUCCEEDED = creation succeeded, ABORTED = failed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>pMethod</name>
      <description><en>encryption key id</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pInputFileName</name>
      <description><en>name source file</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pOutputFileName</name>
      <description><en>name destination file</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description>
        <en>holds all the encryption keys derived from MASTER and SESSION
        key</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE CreateMacFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   )
// ---------------------------------------------------------------------------
{
      int      BytesRead;
      FILE*    File;
      BYTE     buffer[512],
               mac   [AESMACLEN+1],
               macr  [4] = "0000",
               maci  [4],
               resr  [4] = "0000",
               resi  [4];
      int      Status = 0;

      #if _DEBUGMODE
         sprintf( gcDcio, " CMF( pMethod[%d], pInFile[%s], pOutFile[%s])",
         	pMethod, pInputFileName, pOutputFileName); dcio();
      #endif // _DEBUGMODE

    // try to open the file
    if( ( File = fopen( pInputFileName, "rb")) == NULL)
    {
         Status = 10;
         goto error;
    }

    // read the file and compute the MAC of it
    while( ( BytesRead = fread( buffer, sizeof( buffer), 1, File)) != 0)
    {
        MAC( pMethod, buffer, BytesRead);
        memcpy( maci, Result_Key, 4);
        memcpy( resi, Result_Key+4, 4);

        for( int i=0; i < 4; i++)
        {
            macr[i] ^= maci[i];
            resr[i] ^= resi[i];
        }
    }

    // close input
    if( ( fclose( File)) != 0)
    {
         Status = 20;
         goto error;
    }

    if( ( File = fopen( pOutputFileName, "wb")) == NULL)
    {
         Status = 30;
         goto error;
    }

    fwrite( macr, 4, 1, File);
    fwrite( resr, 4, 1, File);

    if( fclose( File) == EOF)
    {
         Status = 40;
         goto error;
    }

    return SUCCEEDED;

error:
   if ( Status)
   {
      Status = - Status;
      #if _DEBUGMODE
         sprintf( gcDcio, " CMF Aborted - status[%d]", Status); dcio();
      #endif // _DEBUGMODE
   }

   return ABORTED;
}
//</code></function>


/*<function id="TestMacFile">
<heading>
  <description>
    <en>Test file integrity by computing new mac and comparing it with a
    previously recorded one.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description>
        <en>SUCCEEDED = creation succeeded, ABORTED = failed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>pMethod</name>
      <description><en>encryption key id</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pInputFileName</name>
      <description><en>name source file</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pOutputFileName</name>
      <description><en>name destination file</en></description>
    </parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype><name>encKeys[N_METHODS]</name>
      <description>
        <en>holds all the encryption keys derived from MASTER and SESSION
        key</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>Can't use existing functions because there MUST be a
  floppy in the drive!!</comment>
  <author/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE TestMacFile (
   int   pMethod,          // encryption key id
   char* pDataFileName,   // name source file
   char* pMacFileName   // name destination file
   )
// ---------------------------------------------------------------------------
{
      int      BytesRead;
      FILE*    File;
      BYTE     buffer[512],
               mac   [AESMACLEN+1],
               macr  [4] = "0000",
               maci  [4],
               resr  [4] = "0000",
               resi  [4];
      int      Status = 0;
      int      i;

      #if _DEBUGMODE
         sprintf( gcDcio, " TMF( pMethod[%d], pDataFile[%s], pMacFile[%s])",
         	pMethod, pDataFileName, pMacFileName); dcio();
      #endif // _DEBUGMODE

   // try to open the file
   if ( ( File = fopen( pDataFileName, "rb")) == NULL)
   {
      Status = 10;
      goto error;
   }

   // read the file and compute the MAC of it
   while ( ( BytesRead = fread( buffer, sizeof( buffer), 1, File)) != 0)
   {
      MAC( pMethod, buffer, BytesRead);
      memcpy( maci, Result_Key, 4);
      memcpy( resi, Result_Key+4, 4);

      for ( i = 0; i < 4; i++)
      {
         macr[i] ^= maci[i];
         resr[i] ^= resi[i];
      }
   }

   // close input
   if ( ( fclose( File)) != 0)
   {
      Status = 20;
      goto error;
   }

   // now read the MAC from the file
   if ( ( File = fopen( pMacFileName, "rb")) == NULL)
   {
      Status = 30;
      goto error;
   }

   // compare the two MACs
   fread( mac, AESMACLEN, 1, File);
   for ( i = 0; i < 4; i++)
   {
      if( macr[i] != mac[i]  ) return ABORTED;
      if( resr[i] != mac[i+4]) return ABORTED;
   }

   if ( fclose( File) == EOF)
   {
      Status = 40;
      goto error;
   }

    return SUCCEEDED;

error:
   if ( Status)
   {
      Status = - Status;
      #if _DEBUGMODE
         sprintf( gcDcio, " CMF Aborted - status[%d]", Status); dcio();
      #endif // _DEBUGMODE
   }

   return ABORTED;
}
//</code></function>

/*<function id="signTotals">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE signTotals(
	   char* pFileName,   	   // file to sign
	   char* pOutputFileName   // file that will hold the signature
)
// ---------------------------------------------------------------------------
{
   char Location_B_File  [32], Location_EXE_File[32];
   int  numread;
   FILE *file;
    BYTE buffer[512],
         mac   [AESMACLEN+1],
         macr  [4] = "0000",
         maci  [4],
         resr  [4] = "0000",
         resi  [4];

      int      Status = 0;

   sprintf( Location_EXE_File, "%s", pFileName);
   sprintf( Location_B_File,   "%s", pOutputFileName);

    // try to open the file
    if( ( file = fopen( Location_EXE_File, "rb")) == NULL)
    {
        Status = 10; goto error;
    }
    // read the file and compute the MAC of it
    while( (numread = fread( buffer, 1, sizeof( buffer), file)) != 0)
    {
        MAC( RESMAC, buffer, numread);
        memcpy( maci, Result_Key, 4);       // this way we can keep rest of the old code unchanged
        memcpy( resi, Result_Key+4, 4);

        for( int i=0; i < 4; i++)
        {
            macr[i] ^= maci[i];
            resr[i] ^= resi[i];
        }
    }

    // close the file
    if( ( fclose( file)) != 0)
    {
        Status = 20; goto error;
    }

    if( ( file = fopen( Location_B_File, "wb")) == NULL)
    {
        Status = 30; goto error;
    }

    fwrite( macr, 4, 1, file);
    fwrite( resr, 4, 1, file);

    if( fclose( file) == EOF)
    {
        Status = 40; goto error;
    }

      #if _DEBUGMODE
         sprintf( gcDcio, " signTotals OK - in[%s] out[%s]", pFileName, pOutputFileName); dcio();
      #endif // _DEBUGMODE

    return( SUCCEEDED);

error:
   if ( Status)
   {
      Status = - Status;
      #if _DEBUGMODE
         sprintf( gcDcio, " XMAC Aborted - status[%d]", Status); dcio();
      #endif // _DEBUGMODE
   }

   return ABORTED;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

