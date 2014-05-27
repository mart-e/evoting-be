/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gencryp.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENCRYP.H  $</workFile>
<revision>$Revision: 1.6 $</revision>
<workFileTimeStamp>$Modtime:   Oct 27 2004 12:48:42  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENCRYP.H-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/04 14:23:10 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>gencryp.h</fileName>
  <creationDate>Sept 2001</creationDate>

  <rights/>
  <author>B.S.S.</author>

  <description>
    <fr>Fichier d'entête gencryp.cpp</fr>
    <nl>Header file gencryp.cpp</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header confidential="yes">
#ifndef _GENCRYP_H                      // avoids multiple inclusions of this file
   #define _GENCRYP_H

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\gen\gentype.h"
#include "..\aes\AES.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define KEY_SESSION_FILE  "a:b006"  // the Key_SESSION file

#define MAVEXE_FILE       "a:mav.exe"  // MAV executable
#define MAVEXE_MAC_FILE   "a:b002"     // MAC of the MAV.EXE
#define URNEXE_FILE       "a:urn.exe"  // URN executable
#define URNEXE_MAC_FILE   "a:b012"     // MAC of the URN.EXE
#define RECEXE_FILE       "a:rec.exe"  // REC executable
#define RECEXE_MAC_FILE   "a:b022"     // MAC of the REC.EXE

#define AESKEYLEN      16  // AES keys are 16 bytes long
#define AESBLOCKLEN       16  // AES encrypts data in blocks of 16 bytes
#define AESMACLEN          8  // size of MAC returned by AES

#define DESKEYLEN          8  // DES keys were 8 bytes long (size of MAVVOT key cannot be
                        //  increased due to the limited spaces on the card)

#define N_METHODS          7  // total number of encryption keys
//</defines>

   typedef struct {                 // Structure to hold keys
     BYTE data[AESKEYLEN+1];              // holds the key material
   } AESKEY;

   typedef struct {                 // Structure to hold IV
     BYTE data[MAX_IV_SIZE];              // holds the IV data
   } AESIV;

   typedef struct {                 // Structure to hold MAC
     BYTE data[AESMACLEN+1];              // holds the MAC data
   } AESMAC;

   typedef int AESEXCEPTION;


   extern AESKEY mKey;
   extern AESKEY sKey;

   extern AESKEY encKeys [N_METHODS];

   extern AESIV iv;

   extern keyInstance encKeyInst;
   extern cipherInstance encCipherInst;

   extern keyInstance decKeyInst;
   extern cipherInstance decCipherInst;

   extern BYTE mavSessionKey [DESKEYLEN+1];
   extern BYTE Result_Key [AESMACLEN+1];

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
   void Calculate_Internal_Keys( void);   //modified DES functions
   void Encrypt( BYTE * Data, int N_Blocks);
   void Decrypt( BYTE * Data, int N_Blocks);
   void MAC    ( int Method, BYTE * Text, int TextLen);
   void Generate_Mav_Session( void);
   RET_CODE Verify_Password( char * Password);
   RET_CODE Read_Key_On_Floppy( char * File_Name, BYTE * Key);
   RET_CODE Test_Executable( int Method, char * Exe_File, char * Mac_File);

   void AlignText( char * txt, int * noBlocks); //new AES functions
   void InitEnc( int method);
   void InitDec( int method);

RET_CODE EncryptBinaryFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   );
RET_CODE DecryptBinaryFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   );
RET_CODE CreateMacFile (
   int   pMethod,          // encryption key id
   char* pInputFileName,   // name source file
   char* pOutputFileName   // name destination file
   );
RET_CODE TestMacFile (
   int   pMethod,          // encryption key id
   char* pDataFileName,    // name data file
   char* pMacFileName      // name mac file
   );
RET_CODE signTotals(
	char* pFileName,   	   // file to sign
	char* pOutputFileName   // file that will hold the signature
	);

//</prototypes>

   // Symbolic representation of the encryption methods
   const int URNEXE = 0;   // to compute the MAC for the urn.exe program
   const int MAVEXE = 1;   // to compute the MAC for the mav.exe program
   const int GENDAT = 2;   // for the encrypted data file
   const int URNVOT = 3;   // for the encryption of the BEFORE & AFTER files
   const int URNTOT = 4;   // for the encryption of the totals file (same as URNVOT)
   const int MAVVOT = 5;   // for use with the card
   const int RESMAC = 6;   // mac on totals file

#endif   // <!-- eof gencryp.h -->
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->