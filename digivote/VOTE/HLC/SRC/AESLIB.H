/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="aeslib.h">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2008/11/04 12:54:16 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>aesLib.h</fileName>
  <creationDate>Sept 2001</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>header file</fr>
    <nl>header file</nl>
  </description>
  <history></history>
  <comment></comment>
</comments>
<header>*/

#ifndef _AESLIB_H                                /* avoids multiple inclusions of this file */
    #define _AESLIB_H

/*<includes>*/
#include "fullspec.h"                            /* interface to AES core functions */
/*</includes>*/

   #define AESKEYLEN          16       /* AES keys are 16 bytes long */
   #define AESBLOCKLEN        16       /* AES encrypts data in blocks of 16 bytes */
   #define AESMACLEN           8       /* size of MAC returned by AES */

   #define DESKEYLEN           8       /* DES keys were 8 bytes long (used for MAVVOT
                                          key, cannot be increased due to the limited
                                          space on the card) */

   #define ENCKEYSMAX         11       /* size of array holding encryption keys */
      #define URNEXE   0  /* to compute the MAC for the urn.exe program */
      #define MAVEXE   1  /* to compute the MAC for the mav.exe program */
      #define GENDAT   2  /* for the encrypted data file */
      #define URNVOT   3  /* for encryption of the BEFORE & AFTER files */
      #define URNTOT   4  /* for encryption of the totals file (same as URNVOT) */
      #define MAVVOT   5  /* for use with the card */
      #define PRPTOT   6  /* for encryption of passwords in PRP/TOT database */
      #define URNCHK   7  /* to compute MAC's on floppy's content */
      #define RECEXE   8  /* to compute the MAC for the rec.exe program (?) */
      #define PRPEXP   9  /* for encryption of import/export files exchanged between PRP's */
      #define RESMAC  10  /* to compute MAC on totals */

   typedef struct {                        /* Structure to hold keys */
     BYTE data[AESKEYLEN+1];                   /* holds the key material */
   } AESKEY;

   typedef struct {                        /* Structure to hold IV */
     BYTE data[MAX_IV_SIZE];                   /* holds the IV data */
   } AESIV;

   typedef struct {                        /* Structure to hold MAC */
     BYTE data[AESMACLEN+1];                   /* holds the MAC data */
   } AESMAC;

/*<prototypes><!-- these functions are public -->*/
   extern int Mac( int Method, BYTE * Text, int TextLen);
   extern int Dec( BYTE * Data, int N_Blocks);
   extern int InitDec( int method);
   extern int Enc( BYTE * Data, int N_Blocks);
   extern int InitEnc( int method);
   extern int MakeEncryptionKeys( void);
   extern int AlignText( char* txt, int* noBlocks);
   extern int InitiateAes( void);
   extern int TerminateAes( void);
   extern char* getTimeStamp( void);
/*</prototypes>*/

   /* defined in alg-fst.c, malloc-ed by initiateAes(), free-d by terminateAes() */
   extern u32* Te0;
   extern u32* Te1;
   extern u32* Te2;
   extern u32* Te3;
   extern u32* Te4;
   extern u32* Td0;
   extern u32* Td1;
   extern u32* Td2;
   extern u32* Td3;
   extern u32* Td4;

   /* defined in aesLib.c */
   extern AESKEY mKey;
   extern AESKEY sKey;

   extern AESKEY encKeys [ENCKEYSMAX];

   extern AESIV iv;

   extern keyInstance encKeyInst;
   extern cipherInstance encCipherInst;

   extern keyInstance decKeyInst;
   extern cipherInstance decCipherInst;

   extern BYTE Result_Key [AESMACLEN+1];

   extern char gcGlobalsFileName[];
   extern char gcMessage[];

#endif  /* #ifndef _aesLib_H */
/*</header>
</source>
<!-- eof -->*/
