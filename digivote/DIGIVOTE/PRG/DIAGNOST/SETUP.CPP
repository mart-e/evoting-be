/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="setup.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   SETUP.CPP  $</workFile>
  <revision>$Revision: 1.8 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 19 2004 11:47:22  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/Diagnost/SETUP.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2006/10/26 12:48:17 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

//======================== Include files ==================================
#include <io.h>
#include <dos.h>
#include <dir.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream.h>
#include <graphics.h>

#ifndef DIGTOOLS

#ifdef YELLOW_DISK
#include <dir.h>
#include <graphics.h>
#endif //YELLOW_DISK

#include "..\DIAGNOST\setup.h"
#include "..\DIAGNOST\disk.h"
#include "..\DIAGNOST\config.h"
#include "..\DIAGNOST\IDE_Info.h"

#ifndef _LIGHTPEN_
#include "..\DIAGNOST\config.h"
#endif

#ifdef DIAGNOST
#include "..\DIAGNOST\Msg.h"
#include "..\DIAGNOST\DosError.h"
#include "..\DIAGNOST\TimeScal.h"
#endif //DIAGNOST

#include "..\DIAGNOST\responsa.h"
#include "..\gen\genvideo.h"

#else  // DIGTOOLS

#define FALSE 0
#define TRUE  1

#include "..\DIAGNOST\setup.h"

#endif // DIGTOOLS

//=============================================================================

#ifdef DIGTOOLS
#define NBR_CHK  9
#else // !DIGTOOLS
#define NBR_CHK 14
#endif // DIGTOOLS
struct {
   char         sQuery[40];
   BOOLEAN      Found;
   BIOS_VERSION currVersion;
   int          refUpgradeTo;
   BOOLEAN      detail;
} Check[]
= {
/* 0*/{ "AMI",                            FALSE,CORRECT_VERSION,         -1,FALSE },
       // Initial Digivote System - 80386 - Digivote I
/* 1*/{ "Award Modular BIOS v4.51PG",     FALSE,CORRECT_VERSION,         -1,FALSE },
       // correct Elite - Digivote II
/* 2*/{ "Award Modular BIOS v6.00PG",     FALSE,CORRECT_VERSION,         -1,FALSE },
       // correct Finmek/Olivetti - Digivote III
/* 3*/{ "P5SJ-B Ver 2.0c",                FALSE,AWARD_v451PG_Ver_20c,     5,TRUE  },
       // must upgrade !! - Digivote II
/* 4*/{ "P5SJ-B Ver 2.0d",                FALSE,AWARD_v451PG_Ver_20d,     5,TRUE  },
       // beter to upgrade - Digivote II
/* 5*/{ "P5SJ-B Ver 2.0d Bull/E-vote II", FALSE,CORRECT_VERSION,         -1,TRUE  },
       // correct Elite - Digivote II
/* 6*/{ "W6178MS V2.0 ",                  FALSE,AWARD_v600PG_V20,         7,TRUE  },
       // must upgrade !! - Digivote III
       // 6) Be aware that the character after V2.0 is indeed a space.
/* 7*/{ "W6178MS V2.0ÿBull/E-vote III",   FALSE,CORRECT_VERSION,         -1,TRUE  },
       // correct Finmek/Olivetti - Digivote III
       // 7) Be aware that the character between V2.0 and Bull/E-vote III
       // is NOT a space, but 0xFF -> Ctrl+Alt+255 !!
       // And this for making a clear difference beween a 'standard'
       // bios and an 'upgraded' one.
/* 8*/{ "V693 Baby AT Ver. 1.03",         FALSE,AMI_V693_Baby_AT,        -1,TRUE  },
       // replacement for Digivote I motherboard
       // THIS BIOS TYPE (AMI) IS NOT SUPPORTED IN THE DIAGNOSTIC
       // thus, Machine ID and other BIOS functionality will NOT work !
#ifndef DIGTOOLS
       // check for FUJITSU-SIEMENS if NOT DIGTOOLS !!!
       // =============================================

       // Following 3 items are mandatory to recognize SIEMENS PC !
/* 9*/{ "FUJITSU SIEMENS",                FALSE,SIEMENS_CONSTRUCTOR,     -1,TRUE  },

       // SCENIC P300 : D1561
/*10*/{ "Phoenix Technologies Ltd.",      FALSE,SIEMENS_BIOS_TYPE_D1561, -1,FALSE },
/*11*/{ "4.06  Rev. 1.06.1561",           FALSE,SIEMENS_BIOS_REV_D1561,  -1,TRUE  },

       // SCENICO P : D1711
/*12*/{ "American Megatrends Inc.",       FALSE,SIEMENS_BIOS_TYPE_D1711, -1,FALSE },
/*13*/{ "08.00.09 Rev.1.06-Steria",       FALSE,SIEMENS_BIOS_REV_D1711,  -1,TRUE  }

#endif
};
int          _index  = -1;
BIOS_TEST    bios    = UNKNOWN_BIOS;
BIOS_VERSION version = UNKNOWN_VERSION;

#ifndef DIGTOOLS

#define ACTIONS 4

dataSerialNo SerialNo_D1561[ACTIONS] =
{
/*0*/ { "Acting on: PcIdentnumber data,   Action: display",   "NONE" },
/*1*/ { "Acting on: ProductName data,   Action: display",     "NONE" },
/*2*/ { "Acting on: CustomSerialNo data,   Action: display",  "NONE" },
/*3*/ { "Acting on: ChassisAssetTag data,   Action: display", "NONE" }
};

dataSerialNo SerialNo_D1711[ACTIONS] =
{
/*0*/ { "PcId: '", "NONE" },
/*1*/ { "PN  : '", "NONE" },
/*2*/ { "CSN : '", "NONE" },
/*3*/ { "ChAT: '", "NONE" },
};

BOOLEAN snTested = FALSE;
BOOLEAN snFailed = FALSE;
int     snReturn = 0;

FILE    *SerialNoCmd;

char snProgFile1[]  =  "?:\\SerialNo.exe",
     snParamFile1[] = "@?:\\SerialNo.cmd",
     snProgFile2[]  =  "?:\\CustomNo.exe",
     snOvlFile2[]   =  "?:\\CustomNo.ovl",
     snLogFile[]    =  "?:\\FS.log";

// it's easy to manipulate bits using this union ...
// simply put a value in sByte.Value ...
// change a bit (e.g. sByte.Bit.p5 = 1) ..
// and sByte.Value has been changed ...
union BitsAndBytes {
   struct {
      char p0 : 1;
      char p1 : 1;
      char p2 : 1;
      char p3 : 1;
      char p4 : 1;
      char p5 : 1;
      char p6 : 1;
      char p7 : 1;
   } Bit;
   char Value;
} sByte;

//=============================================================================
// AMI Bios
//=============================================================================
//            M??????? / U??????? / P??????? / T???????
//            MAV      / URN      / PRP      / TOT
//=============================================================================
union AMI_machine_id   { unsigned long value_l;
                   unsigned char value_c[5]; };

#define _AMI_   5
unsigned char AMI_addr[_AMI_] = {0x38,0x39,0x3A,0x3B,0x3C};

//=============================================================================
// AWARD Bios -> as well for P5SJ-B (Elite) as for W6178MS (Finmek/Olivetti)
//=============================================================================
//            YWW?????
//               00001 -> 69999 : MAV
//               70001 -> 89999 : URN
//               90001 -> 99999 : PRP/TOT
//=============================================================================
//        P5SJ-B W6178MS
// Flag 1 (0x4B) (0x37) : Modulo 97 of Machine ID
// Flag 2 (0x4C) (0x3A) : M for MAV
//                        U for URN
//                        T for PRP/TOT (see rules above)
//
//== obsolete == obsolote == obsolete == obsolete == obsolete == obsolete ==
// Flag 3 (0x4E) <0x??) : Status DIAGNOSTIC (only for P5SJ-B)
//                        F = old situation
//                        A =        Award 2.0d Bull/E-vote II
//                        1 = Test + Award 2.0d
//                        B = Test + Award 2.0d Bull/E-vote II
//== obsolete == obsolote == obsolete == obsolete == obsolete == obsolete ==
//
//=============================================================================
union AWARD_machine_id { unsigned long value_l;
                   unsigned char value_c[4]; };
unsigned int  _AWARD_         = 0;
unsigned char *AWARD_addr     = NULL;
unsigned char *AWARD_flag     = NULL;
unsigned char *AWARD_checksum = NULL;
#define HiByte   0
#define LoByte   1
#define FromByte 2
#define ToByte   3

#define _AWARD_v451PG_ 4
unsigned char AWARD_v451PG_addr[_AWARD_v451PG_] = { 0x54, 0x55, 0x57, 0x5B};
unsigned char AWARD_v451PG_flag[2]              = { 0x4B, 0x4C};
unsigned char AWARD_v451PG_checksum[4]          = { 0x7C, 0x7D, 0x40, 0x7B};

#define _AWARD_v600PG_ 4
unsigned char AWARD_v600PG_addr[_AWARD_v600PG_] = { 0x33, 0x34, 0x35, 0x36};
unsigned char AWARD_v600PG_flag[2]              = { 0x37, 0x3A};
unsigned char AWARD_v600PG_checksum[4]          = { 0x00, 0x00, 0x00, 0x00};

unsigned char Flag[2];

#ifdef CMOS
BOOLEAN BiosInfo = FALSE;
#endif //CMOS

const char *MachineIDfile = "\\MACHINE.ID";
const char *RTCfile       = "\\SYSTEM.RTC";

//=============================================================================
//= D E B U G = D E B U G = D E B U G = D E B U G = D E B U G = D E B U G =====
//=============================================================================
#ifdef DEBUG
ofstream dFile;
char     dMsg[255];
char     dName[10] = "DEBUG.LOG";
BOOLEAN  ToFile    = TRUE;
#endif // DEBUG
//=============================================================================
//= D E B U G = D E B U G = D E B U G = D E B U G = D E B U G = D E B U G =====
//=============================================================================

#endif // !DIGTOOLS

unsigned char GetByteFromCMOS( unsigned char Address)
{
   unsigned char TheByte;
   int           i;
   #ifndef DIGTOOLS
   BOOLEAN       Bank2 = FALSE;
   #endif // !DIGTOOLS

   // ===================================================================
   // DON'T USE Bios_Test() !!!!
   // If you do so, you'll get an infinite loop when Bios is of type AMI,
   // because it will be tested as AMI_1, AMI_2, AMI_3 or BAD_AMI, using
   // this functions more than once !!
   // ===================================================================
   switch( bios)
   {
      // DIGTOOLS limitation !!!
      // This function is only called on Digivote I systems !
      #ifndef DIGTOOLS
      case AMI_REPLACEMENT:
         Bank2 = TRUE;

      case AWARD_W6178MS:
         for( i = 0 ; i < _AWARD_ ; i++)
            if( Address == AWARD_addr[i])
               Bank2 = TRUE;

         if( Address == AWARD_flag[0] ||
            Address == AWARD_flag[1]   )
            Bank2 = TRUE;
      case AWARD_P5SJ_B :
         if( Address >= 0x80)
            Bank2 = TRUE;

         if( Bank2)
         {
            asm {
               MOV AL,Address
               OUT 72H,AL
               IN  AL,73H
            }
            break;
         }

      #endif // !DIGTOOLS
      default:
         asm {
            MOV AL,Address
            OUT 70H,AL
            MOV AL,BYTE PTR [BP+10] // Spend some time
            IN  AL,71H
         }
         break;
   }
   TheByte = _AL;

   #ifdef DEBUG
   if( ToFile)
   {
      sprintf( dMsg, "[<0x%.2X>] => [<0x%.2X><%.3u>]%s\n",
                  Address, TheByte, TheByte,
                  Bank2 ? " -> (Bank2)"
                      : " -> (Bank1)");
      dFile << dMsg;
      dFile.flush();
   }
   #endif

   return (TheByte);
}

#ifndef DIGTOOLS

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ATTENTION  ATTENTION ATTENTION  ATTENTION  ATTENTION  ATTENTION  ATTENTION
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//=============================================================================
//        **                                            ****
// SetByteToCMOS should only be called using the SetByteFromCMOS-function
//        **                                            ****
//=============================================================================

void SetByteToCMOS( unsigned char Address, unsigned char Data)
{
   int     i;
   BOOLEAN Bank2 = FALSE;

   switch( Bios_Test())
   {
	  case FS_SCENIC_D1561:
	  case FS_SCENICO_D1711:
	     /* DO NOT ATTEMPT TO WRITE TO CMOS */
	     /* !!!Checksum Error CAN occur !!! */
	     break;

      case AWARD_W6178MS:
         for( i = 0 ; i < _AWARD_ ; i++)
            if( Address == AWARD_addr[i])
               Bank2 = TRUE;

         if( Address == AWARD_flag[0] ||
            Address == AWARD_flag[1]   )
            Bank2 = TRUE;

      case AWARD_P5SJ_B :
         if( Address >= 0x80)
            Bank2 = TRUE;

         if( Bank2)
         {
            asm {
               MOV AL,Address
               OUT 72H,AL
               MOV AL,Data
               OUT 73H,AL
            }
            break;
         }

      default:
         asm {
            MOV AL,Address
            OUT 70H,AL
            MOV AL,BYTE PTR Data     // Spend some time
            OUT 71H,AL
         }
         break;
   }

   #ifdef DEBUG
   sprintf( dMsg, "[Addr<0x%.2X><0x%.2X>] -> (Bank%d)\n",
               Address, Data, Bank2 ? 2 : 1);
   dFile << dMsg;
   dFile.flush();
   #endif //DEBUG
   return;
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// ATTENTION  ATTENTION ATTENTION  ATTENTION  ATTENTION  ATTENTION  ATTENTION
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void SetByteFromCMOS( unsigned char Address, unsigned char Data)
{
   unsigned int Checksum;
   unsigned char oldData, addr;

   #ifdef DEBUG
   dFile << ">>SetByteFromCMOS\n";
   dFile.flush();
   #endif
   // Preserve original data from CMOS
   oldData = GetByteFromCMOS( Address);

   SetByteToCMOS( Address, Data);
   #ifdef DEBUG
   sprintf( dMsg, "[<0x%.2X>] <= [<0x%.2X><%.3u>] <= [<0x%.2X><%.3u>]\n",
               Address, Data, Data, oldData, oldData);
   dFile << dMsg;
   dFile.flush();
   #endif

   // Adjust/Modify checksum, if needed !
   switch( Bios_Test())
   {
      case AMI_1   :
      case AMI_2   :
      case AMI_3   :
      case BAD_AMI :
         if( Address >= 0x10 && Address <= 0x2D)
         {
            #ifdef DEBUG
            Checksum =  GetByteFromCMOS( 0x2E);
            Checksum =  Checksum<<8;
            Checksum += GetByteFromCMOS( 0x2F);
            sprintf( dMsg, "Checksum at 0x2E/0x2F changed from %.4X",
                        Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG
            for( Checksum = 0, addr = 0x10 ; addr <= 0x2D ; addr++)
               Checksum += GetByteFromCMOS( addr);

            // write the checksum to the CMOS (2 bytes)
            SetByteToCMOS( 0x2E, Checksum>>8 & 0xFF);
            SetByteToCMOS( 0x2F, Checksum    & 0xFF);
            #ifdef DEBUG
            sprintf( dMsg, " to %.4X\n", Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG
         }
         else if(                    ( Address >= 0x34 &&
                                Address <= 0x3D   )  ||
                ( bios == AMI_2 && ( Address >= 0x40 &&
                                Address <= 0x5F   )) ||
                ( bios == AMI_3 && ( Address >= 0x40 &&
                                Address <= 0x6F   ))   )
         {   // compute the extended CMOS checksum
            #ifdef DEBUG
            Checksum =  GetByteFromCMOS( 0x3E);
            Checksum =  Checksum<<8;
            Checksum += GetByteFromCMOS( 0x3F);
            sprintf( dMsg, "ExtChecksum at 0x3E/0x3F changed from %.4X",
                        Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG
            for( Checksum = 0, addr = 0x34 ; addr <= 0x3D ; addr++)
               Checksum += GetByteFromCMOS( addr);

            if( bios == AMI_2)
               for( addr = 0x40 ; addr <= 0x5F ; addr++)
                  Checksum += GetByteFromCMOS( addr);
            if( bios == AMI_3)
               for( addr = 0x40 ; addr <= 0x6F ; addr++)
                  Checksum += GetByteFromCMOS( addr);

            // write the checksum to the CMOS (2 bytes)
            SetByteToCMOS( 0x3E, Checksum>>8 & 0xFF);
            SetByteToCMOS( 0x3F, Checksum    & 0xFF);
            #ifdef DEBUG
            sprintf( dMsg, " to %.4X\n", Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG
         }
         break;

      case AWARD_P5SJ_B :
      case AWARD_W6178MS:
         if( ( AWARD_checksum[HiByte] != 0x00      &&
              AWARD_checksum[LoByte] != 0x00        ) &&
            ( Address >= AWARD_checksum[FromByte] &&
              Address <= AWARD_checksum[ToByte]     )   )
         {
            Checksum  = GetByteFromCMOS( AWARD_checksum[HiByte])<<8;
            Checksum += GetByteFromCMOS( AWARD_checksum[LoByte]);
            #ifdef DEBUG
            sprintf( dMsg, "Checksum at 0x%.2X/0x%.2X changed from %.4X",
                        AWARD_checksum[HiByte],
                        AWARD_checksum[LoByte],
                        Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG

            Checksum = Checksum - oldData + Data;

            SetByteToCMOS( AWARD_checksum[HiByte], Checksum>>8 & 0xFF);
            SetByteToCMOS( AWARD_checksum[LoByte], Checksum    & 0xFF);
            #ifdef DEBUG
            sprintf( dMsg, " to %.4X\n", Checksum);
            dFile << dMsg;
            dFile.flush();
            #endif //DEBUG
         }
         break;
   }
   return;
}

//=============================================================================

#endif // !DIGTOOLS

//=============================================================================
//
//   Digivote I : AMI BIOS
//
//=============================================================================

static BOOLEAN ExtChecksumType1()
{
   unsigned int checksum=0;
   int addr;

   // compute the extended CMOS checksum
   for( addr=0x34; addr<=0x3d; addr++)
      checksum += GetByteFromCMOS( addr);

   if( (checksum>>8 & 0xFF) == GetByteFromCMOS( 0x3E) &&
      (checksum & 0xFF) == GetByteFromCMOS( 0x3F))
      return( TRUE);

   return( FALSE);
}

//=============================================================================

static BOOLEAN ExtChecksumType2()
{
   unsigned int checksum=0;
   int addr;

   // compute the extended CMOS checksum
   for( addr=0x34; addr<=0x3d; addr++)
      checksum += GetByteFromCMOS( addr);
   for( addr=0x40; addr<=0x5f; addr++)
      checksum += GetByteFromCMOS( addr);

   if( (checksum>>8 & 0xFF) == GetByteFromCMOS( 0x3E) &&
      (checksum & 0xFF) == GetByteFromCMOS( 0x3F))
      return( TRUE);

   return( FALSE);
}

//=============================================================================

static BOOLEAN ExtChecksumType3()
{
   unsigned int checksum=0;
   int addr;

   // compute the extended CMOS checksum
   for( addr=0x34; addr<=0x3d; addr++)
      checksum += GetByteFromCMOS( addr);
   for( addr=0x40; addr<=0x6f; addr++)
      checksum += GetByteFromCMOS( addr);

   if( (checksum>>8 & 0xFF) == GetByteFromCMOS( 0x3E) &&
      (checksum & 0xFF) == GetByteFromCMOS( 0x3F))
      return( TRUE);

   return( FALSE);
}

//=============================================================================

BIOS_TEST Bios_Test()
{
   // =====================================================
   // There is no need to scan the far data everytime
   // we need the exact bios version.
   // Return the initial found bios version <> UNKNOWN_BIOS
   // =====================================================
   #ifdef _DEBUG
   if( bios != UNKNOWN_BIOS && ToFile)
   {
      dFile << "Bios already recognized !\n";
      dFile.flush();
   }
   #endif //DEBUG

   if( bios != UNKNOWN_BIOS)
      return( bios);

   unsigned char far * address = (unsigned char far *) MK_FP( 0xF000, 0x0000);
   unsigned int i, j;

   for( i = 0 ; i < 0xFFFF ; i++, address++)
      for( j = 0 ; j < NBR_CHK ; j++)
         if( *address == Check[j].sQuery[0] &&
            ! _fmemcmp( Check[j].sQuery, address, strlen( Check[j].sQuery)))
         {
            Check[j].Found = TRUE;
            version        = Check[j].currVersion;
            _index         = j;
         }

   // Corrections => prevent 'duplicate' solutions
   if( Check[5].Found)
      Check[4].Found = FALSE;

   #ifdef CMOS
   if( BiosInfo)
   {
      printf( "=========== B I O S ===========\n");
      for( j = 0 ; j < NBR_CHK ; j++)
         if( Check[j].Found)
         {
            printf( "%d) %s\n", j, Check[j].sQuery);
            if( Check[j].refUpgradeTo != -1)
               printf( "Please Upgrade BIOS to\n%s\n",
                     Check[Check[j].refUpgradeTo].sQuery);
         }
      printf( "===============================\n");
   }
   #endif //CMOS

   #ifdef DEBUG
   dFile << "========== Bios_Test (" << _index << ")=======\n";
   for( j = 0 ; j < NBR_CHK ; j++)
      if( Check[j].Found)
      {
         dFile << j << ") " << Check[j].sQuery << "\n";
         if( Check[j].refUpgradeTo != -1)
            dFile << "   Please Upgrade BIOS to\n"
                 << Check[j].refUpgradeTo << ") "
                 << Check[Check[j].refUpgradeTo].sQuery << "\n";
      }
   dFile << "===============================\n";
   dFile.flush();
   #endif //DEBUG

   if( Check[0].Found && !Check[9].Found)
   {
      if     ( Check[8].Found)
         bios = AMI_REPLACEMENT;

      else if( ExtChecksumType1())
         bios = AMI_1;

      else if( ExtChecksumType2())
         bios = AMI_2;

      else if( ExtChecksumType3())
         bios = AMI_3;

      else
         bios = BAD_AMI;
   }

   // Digivote II : Elite
   else if( Check[1].Found &&
            Check[5].Found   )
      {
         #ifndef DIGTOOLS
         _AWARD_        = _AWARD_v451PG_;
         AWARD_addr     = &AWARD_v451PG_addr[0];
         AWARD_flag     = &AWARD_v451PG_flag[0];
         AWARD_checksum = &AWARD_v451PG_checksum[0];
         #endif // !DIGTOOLS
         bios           = AWARD_P5SJ_B;
      }

   // Digivote II : Elite, but needs to be upgraded to latest version
   else if( Check[1].Found &&
          ( Check[3].Found ||
            Check[4].Found   ))
          bios = AWARD_UPGRADE;

   // Digivote III : Finmek/Olivetti but not upgraded yet !!
   else if( Check[2].Found &&
          Check[6].Found   )
          bios = AWARD_UPGRADE;

   // Digivote III : Finmek/Olivetti
   else if( Check[2].Found &&
          Check[7].Found   )
       {
          #ifndef DIGTOOLS
          _AWARD_        = _AWARD_v600PG_;
          AWARD_addr     = &AWARD_v600PG_addr[0];
          AWARD_flag     = &AWARD_v600PG_flag[0];
          AWARD_checksum = &AWARD_v600PG_checksum[0];
          #endif // !DIGTOOLS
          bios           = AWARD_W6178MS;
       }

   // Fujitsu-Siemens SCENIC P300 (D1561)
   else if( Check[ 9].Found &&
            Check[10].Found &&
            Check[11].Found   )
          bios = FS_SCENIC_D1561;

   // Fujitsu-Siemens SCENICO P (D1711)
   else if( Check[ 9].Found &&
            Check[12].Found &&
            Check[13].Found   )
          bios = FS_SCENICO_D1711;

   else
   {
      _index  = -1;
      version = UNKNOWN_VERSION;
      bios    = UNSUPPORTED_BIOS;
   }

   #ifdef DEBUG
   sprintf( dMsg, "[bios=<%d>] [version=<%d>] [_index=<%d>]\n",
               bios, version, _index);
   dFile << dMsg;
   dFile.flush();
   #endif //DEBUG

   return( bios);
}

//=============================================================================

#ifndef DIGTOOLS

#ifndef _LIGHTPEN_
#ifndef TEST
#ifndef CMOS
#ifndef RTC_LOAD
char *RetrieveBiosVersion( BOOLEAN DetailInfo)
{
   char *AmiType;
   int i;

   Bios_Test();
   if( _index != -1)
      for( i = 0 ; i < NBR_CHK ; i++)
         if( Check[i].Found  == TRUE       &&
            Check[i].detail == DetailInfo   )
            return( Check[i].sQuery);
   if( DetailInfo)
   {
      AmiType = ( char *) malloc( 10);
      switch( bios)
      {
         case AMI_1  :
         case AMI_2  :
         case AMI_3  :
            sprintf( AmiType, "Type %d", 1 + bios - AMI_1);
            return( AmiType);

         default     :
            sprintf( AmiType, "N/A");
            return( AmiType);
      }
   }
   else
      return( Get_Msg( UnsupportedBios));
}
#endif //RTC_LOAD
#endif //CMOS
#endif //TEST
#endif // _LIGHTPEN_

//=============================================================================

unsigned char Calculate_Mod97( unsigned long ID)
{
   switch( Bios_Test())
   {
      case AWARD_W6178MS:
      case AWARD_P5SJ_B :
         return( ID % 97);

      default :
         return( 0x00);
   }
}

//=============================================================================

unsigned char SetMod97FromCMOS( unsigned long ID)
{
   unsigned char Mod97;

   switch( Bios_Test())
   {
      case AWARD_W6178MS:
      case AWARD_P5SJ_B :
         Mod97 = Calculate_Mod97( ID);
         SetByteFromCMOS( AWARD_flag[0], Mod97);
         break;

      default :
         Mod97 = 0x00;
         break;
   }
   return( Mod97);
}

//=============================================================================

unsigned char Determine_System( unsigned long ID)
{
   switch( Bios_Test())
   {
      case FS_SCENIC_D1561 :
      case FS_SCENICO_D1711:
      case AWARD_W6178MS   :
      case AWARD_P5SJ_B    :
         unsigned long SerialNbr = ID % 100000;

         if     ( SerialNbr >     0 && SerialNbr <  70000)
            return( 'M'); // MAV
         else if( SerialNbr > 70000 && SerialNbr <  90000)
            return( 'U'); // URN
         else if( SerialNbr > 90000 && SerialNbr < 100000)
            return( 'T'); // PRP/TOT
         else
            return( 'X'); // Nothing

      default :
         return( 0x00);
   }
}

//=============================================================================

unsigned char SetSystemFromCMOS( unsigned long ID)
{
   unsigned char Machine;

   switch( Bios_Test())
   {
      case AWARD_W6178MS:
      case AWARD_P5SJ_B :
         Machine = Determine_System( ID);
         SetByteFromCMOS( AWARD_flag[1], Machine);
         break;

      default :
         Machine = 0x00;
         break;
   }
   return( Machine);
}

//=============================================================================

// Only necessary for Digivote I systems !!
// DI      : M9999999 in case of URN/PRP/TOT
// DII/III : original YWWXXXXX is restored in GetMachineName in case of
//           a forced MAV machine name
///////////////////////////////////////////////////////////////////////
BOOLEAN forcedMachineID( char machine_name[9])
{
   BOOLEAN forcedID = FALSE;

   switch( Bios_Test())
   {
      case AMI_1  :
      case AMI_2  :
      case AMI_3  :
      case BAD_AMI:
         if( !strcmp( machine_name, "M9999999"))
            forcedID = TRUE;
         break;
   }
   return( forcedID);
}

//=============================================================================

void setDrvLetter( void)
{
   int currDrive = getdisk();

   if( snProgFile1[0] == '?')
   {
      // SCENIC D1561
      snProgFile1[0]  = Get_RAMdrive( TRUE);
      snParamFile1[1] = snProgFile1[0];
      // SCENICO D1711
      snProgFile2[0]  = snProgFile1[0];
      snOvlFile2[0]   = snProgFile1[0];
      // LOGGING ... (all systems)
      snLogFile[0]    = snProgFile1[0];
      if( Get_RAMdrive( FALSE) != '#')
      {
         switch( bios)
         {
            case FS_SCENIC_D1561:
               if( access( snProgFile1, 0x00))
               {
                  setdisk( snProgFile1[0] - 'A');
                  LoadProgram( "A:\\TOOLSCAB.EXE", "-gD1@gn0st1c -y SerialNo.*", "NUL");
                  setdisk( currDrive);
               }
               break;

            case FS_SCENICO_D1711:
               if( access( snProgFile2, 0x00) ||
                   access( snOvlFile2,  0x00)   )
               {
                  setdisk( snProgFile1[0] - 'A');
                  LoadProgram( "A:\\TOOLSCAB.EXE", "-gD1@gn0st1c -y CustomNo.*", "NUL");
                  setdisk( currDrive);
               }
               break;
         }
      }
   }
}

//=============================================================================

#ifdef CMOS
BOOLEAN GetMachineName( char machine_name[9],
                  BOOLEAN CMOSvalue)
#else
void    GetMachineName( char machine_name[9])
#endif
{
   BOOLEAN forcedMAV = FALSE;
   int i, lenCSN, index;

   #ifdef DEBUG
   dFile << ">>GetMachineName\n";
   dFile.flush();
   #endif //DEBUG
   switch( Bios_Test())
   {
      case AMI_1  :
      case AMI_2  :
      case AMI_3  :
      case BAD_AMI:
         AMI_machine_id id1;

         char c = GetByteFromCMOS( AMI_addr[0]);
         if( c != 'M' &&
            c != 'U' &&
            c != 'P' &&
            c != 'T')
            machine_name[0] = 0;
         else
         {
            for( i=1 ;  i < _AMI_ ; i++)
               id1.value_c[i-1] = GetByteFromCMOS( AMI_addr[i]);
            sprintf( machine_name, "%c%07lu", c, id1.value_l);
            machine_name[8] = 0;
         }
         // Fix problem for following elections ...
         //////////////////////////////////////////
         if( !strcmp( machine_name, "M9999999"))
         {
            machine_name[0] = 0;
            WriteMachineName( machine_name);
         }
         break;

      case AWARD_W6178MS:
      case AWARD_P5SJ_B :
         AWARD_machine_id id2;
         unsigned long original_id;
         unsigned int Y_built, FW_built;

         // Get Machine ID
         for( i=0 ;  i < _AWARD_ ; i++)
            id2.value_c[i] = GetByteFromCMOS( AWARD_addr[i]);
         #ifdef DEBUG
         sprintf( dMsg, "%.2X%.2X%.2X%.2X", id2.value_c[0],
                                    id2.value_c[1],
                                    id2.value_c[2],
                                    id2.value_c[3]);
         dFile << "From CMOS : " << dMsg
              << " / " << id2.value_l << "\n";
         dFile.flush();
         #endif //DEBUG

         // Get Flag information
         for( i=0 ;  i < 2 ; i++)
            Flag[i] = GetByteFromCMOS( AWARD_flag[i]);

         // Save ID from BIOS to compare afterwards to see
         // whether the ID has been forced or not so that we
         // can re-write the ID if it was forced to 0
         original_id = id2.value_l;

         // If ID = 44444444, we must reset ID (bug in v3.2b -> Bureautica)
         // further controls to be sure the ID is still intact !
         if( id2.value_l == 44444444)
            id2.value_l = 0;

         Y_built  = ( int)( id2.value_l / 10000000);
         FW_built = ( int)( id2.value_l / 100000) - ( Y_built * 100);
         if( FW_built  <  1 ||
            FW_built  > 53   )
            // If FW equals 99, it concerns a forced MAV machine name
            // In this case Flag[0] (Mod97) contains the real FW
            // The serial number must be raised with 70000 to become
            // the original serial number
            /////////////////////////////////////////////////////////
            if( FW_built == 99)     // work around LIGHTPEN.EXE
            {                       // work around LIGHTPEN.EXE
               forcedMAV = TRUE;    // work around LIGHTPEN.EXE
               FW_built  = Flag[0]; // work around LIGHTPEN.EXE
            }                       // work around LIGHTPEN.EXE
            else                    // work around LIGHTPEN.EXE
               id2.value_l = 0;     // normal situation

         #ifdef DEBUG
         sprintf( dMsg, "Flag[0] = 0x%.2X<%.3u> ?=? Mod97 = 0x%.2X<%.3u>\n",
                     Flag[0], Flag[0],
                     Calculate_Mod97( id2.value_l),
                     Calculate_Mod97( id2.value_l));
         dFile << dMsg;
         dFile.flush();
         #endif //DEBUG
         // Check Mod97 if it doesn't concern a forced MAV machine name
         //////////////////////////////////////////////////////////////
         if( !forcedMAV && Flag[0] != Calculate_Mod97( id2.value_l))
         {
            id2.value_l = 0;
            Flag[0]     = SetMod97FromCMOS ( id2.value_l);
            Flag[1]     = SetSystemFromCMOS( id2.value_l);
         }
         else
         {
            if( id2.value_l != 0   &&
               Flag[1]     == 'X'   )
               id2.value_l = 0;

            if( Flag[1] != Determine_System( id2.value_l))
               Flag[1] = SetSystemFromCMOS( id2.value_l);
         }
         sprintf( machine_name, "%08lu", id2.value_l);
         machine_name[8] = 0;
         if( original_id != id2.value_l)
            WriteMachineName( machine_name);

         // If it concerns a forced MAV machine name, reproduce the
         // original machine name (see above)
         //////////////////////////////////////////////////////////
         #ifdef CMOS
         else if( forcedMAV && !CMOSvalue)
         #else  // !CMOS
         else if( forcedMAV)
         #endif // CMOS
         {
            sprintf( machine_name, "%c%02d%05lu",
                              machine_name[0],
                              FW_built,
                              atol( &machine_name[3]) + 70000);
            machine_name[8] = 0;
            // Fix problem for following elections ...
            //////////////////////////////////////////
            WriteMachineName( machine_name);
         }
         break;

      case FS_SCENIC_D1561 :
         // No check when fetching Machine ID
         // Normally ID should be correct if proper tools have been used
         // to register Machine ID !
         machine_name[0] = 0;
         if( !snTested)
         {
            char Buffer[100];

            setDrvLetter();
            SerialNoCmd = fopen( &snParamFile1[1], "wt" );
           	fprintf( SerialNoCmd, "PcIdent.d\n");
           	fprintf( SerialNoCmd, "PN.d\n");
            fprintf( SerialNoCmd, "CSN.d\n");
            fprintf( SerialNoCmd, "ChAT.d\n");
            fclose( SerialNoCmd);
            LoadProgram( snProgFile1, snParamFile1, snLogFile);

            ifstream RESULTS( snLogFile);
            index = -1;
            while( !RESULTS.eof() && RESULTS)
            {
               RESULTS.getline( Buffer, sizeof( Buffer));
               if( RESULTS)
                  if( strlen( Buffer) > 0)
                  {
                     if( index == -1)
                     {
                        for( i = 0 ; i < ACTIONS && index == -1 ; i++)
                           if( !strncmp( Buffer,
                                         SerialNo_D1561[i].Descr,
                                         strlen( SerialNo_D1561[i].Descr)))
                           {
                              index = i;
                              break;
                           }
                     }
                     else
                     {
                        if( strncmp( Buffer, "NONE", 4))
                        {
                           strcpy( SerialNo_D1561[index].Value, &Buffer[1]);
                           SerialNo_D1561[index].Value[strlen( Buffer) - 2] = '\0';
                        }
                        index = -1;
                     }
                  }
            }
            RESULTS.close();
            remove( &snParamFile1[1]);
            remove( snLogFile);

            if( strncmp( SerialNo_D1561[CSN].Value, "NONE", 4))
            {
               // (Dis-)Assemble MachineID ...
               if( !strncmp( SerialNo_D1561[CSN].Value, "SN[", 3) &&
                  SerialNo_D1561[CSN].Value[strlen( SerialNo_D1561[CSN].Value) - 1] == ']')
               {
                  lenCSN = strlen( SerialNo_D1561[CSN].Value) - 4;
                  strncpy( SerialNo_D1561[CSN].Value,
                           &SerialNo_D1561[CSN].Value[3],
                           lenCSN);
                  SerialNo_D1561[CSN].Value[lenCSN] = 0;
               }
               else // Force value to "NONE" if CSN doesn't meet
                   // standards ! => "SN[YWWXXXXX]"
               {
                  memset( SerialNo_D1561[CSN].Value, 0,
                        sizeof( SerialNo_D1561[CSN].Value));
                  strcpy( SerialNo_D1561[CSN].Value, "NONE");
               }
            }
            snTested = TRUE;
         }
         if( strncmp( SerialNo_D1561[CSN].Value, "NONE", 4))
            strcpy( machine_name, SerialNo_D1561[CSN].Value);

         break;

      case FS_SCENICO_D1711 :
         // No check when fetching Machine ID
         // Normally ID should be correct if proper tools have been used
         // to register Machine ID !
         machine_name[0] = 0;
         if( !snTested)
         {
            char Buffer[100];

            setDrvLetter();
            LoadProgram( snProgFile2, "-List", snLogFile);
            ifstream RESULTS( snLogFile);
            index = -1;
            while( !RESULTS.eof() && RESULTS)
            {
               RESULTS.getline( Buffer, sizeof( Buffer));
               if( RESULTS)
                  if( strlen( Buffer) > 0)
                  {
                     for( i = 0 ; i < ACTIONS ; i++)
                        if( !strncmp( Buffer,
                                      SerialNo_D1711[i].Descr,
                                      strlen( SerialNo_D1711[i].Descr)))
                        {
                           index = i;
                           strcpy( SerialNo_D1711[index].Value,
                                   &Buffer[strlen( SerialNo_D1711[i].Descr)]);
                           SerialNo_D1711[index].Value[strlen( SerialNo_D1711[index].Value) - 1] = '\0';
                           break;
                        }
                  }
            }
            RESULTS.close();
            remove( &snParamFile1[1]);
            remove( snLogFile);

            if( strncmp( SerialNo_D1711[CSN].Value, "NONE", 4))
            {
               // (Dis-)Assemble MachineID ...
               if( !strncmp( SerialNo_D1711[CSN].Value, "SN[", 3) &&
                  SerialNo_D1711[CSN].Value[strlen( SerialNo_D1711[CSN].Value) - 1] == ']')
               {
                  lenCSN = strlen( SerialNo_D1711[CSN].Value) - 4;
                  strncpy( SerialNo_D1711[CSN].Value,
                           &SerialNo_D1711[CSN].Value[3],
                           lenCSN);
                  SerialNo_D1711[CSN].Value[lenCSN] = 0;
               }
               else // Force value to "NONE" if CSN doesn't meet
                   // standards ! => "SN[YWWXXXXX]"
               {
                  memset( SerialNo_D1711[CSN].Value, 0,
                        sizeof( SerialNo_D1711[CSN].Value));
                  strcpy( SerialNo_D1711[CSN].Value, "NONE");
               }
            }
            snTested = TRUE;
         }
         if( strncmp( SerialNo_D1711[CSN].Value, "NONE", 4))
            strcpy( machine_name, SerialNo_D1711[CSN].Value);

         break;

      default:
         machine_name[0] = 0;
         break;

   }
   #ifdef DEBUG
   dFile << "<<GetMachineName\n";
   dFile.flush();
   #endif //DEBUG

   #ifdef CMOS
   return( forcedMAV);
   #else  // !CMOS
   return;
   #endif // CMOS
}

//=============================================================================

#pragma argsused
void WriteMachineName( char machine_name[9])
{
#ifndef _LIGHTPEN_
   BOOLEAN forcedMAV = FALSE;
   int addr, i, j;

   #ifdef DEBUG
   dFile << ">>WriteMachineName\n";
   dFile.flush();
   #endif //DEBUG
   switch( Bios_Test())
   {
      case AMI_1   :
      case AMI_2   :
      case AMI_3   :
      case BAD_AMI :
         AMI_machine_id id1;
         char MachineType;

         MachineType = machine_name[0];

            // Fix problem for following elections ...
            //////////////////////////////////////////
         // Force MAV if URN/PRP/TOT => LIGHTPEN.EXE problem/bug !!!
         ///////////////////////////////////////////////////////////
         // if( machine_name[0] != 'M')                // work around LIGHTPEN.EXE
         // {                                          // work around LIGHTPEN.EXE
         //    MachineType = 'M';                     // work around LIGHTPEN.EXE
         //    id1.value_l = 9999999;                 // work around LIGHTPEN.EXE
         // }                                          // work around LIGHTPEN.EXE
         // else                                       // work around LIGHTPEN.EXE
            id1.value_l = atol(machine_name + 1);  // normal situation

         // use the encrypted CMOS password position to store
         // our machine_name
         SetByteFromCMOS( AMI_addr[0], MachineType);
         for( i=1 ; i < _AMI_ ; i++)
            SetByteFromCMOS( AMI_addr[i], id1.value_c[i-1]);

         // get the current stored value
         unsigned char p = GetByteFromCMOS( 0x34);
         // reset the password check flags
         SetByteFromCMOS( 0x34, p & 0x3f);
         break;

      case FS_SCENIC_D1561 : // same rules, different way of writing ...
      case FS_SCENICO_D1711: // same rules, different way of writing ...
      case AWARD_W6178MS   :
      case AWARD_P5SJ_B    :
         AWARD_machine_id id2;
         unsigned int Y_built, FW_built;

         id2.value_l = atol( machine_name);
         #ifdef DEBUG
         sprintf( dMsg, "%.2X%.2X%.2X%.2X", id2.value_c[0],
                                    id2.value_c[1],
                                    id2.value_c[2],
                                    id2.value_c[3]);
         dFile << "Machine ID = " << machine_name
              << " / " << id2.value_l
              << " / " << dMsg << "\n"
              << "Determine_System = "
              << Determine_System( id2.value_l) << "\n";
         dFile.flush();
         #endif //DEBUG
         if( Determine_System( id2.value_l) == 'X')
         {
            id2.value_l = 0;
            machine_name[0] = NULL;
         }
         else
         {
            Y_built  = ( int)( id2.value_l / 10000000);
            FW_built = ( int)( id2.value_l / 100000) - ( Y_built * 100);
            #ifdef DEBUG
            dFile << "Y_built  = " << Y_built  << "\n"
                 << "FW_built = " << FW_built << "\n";
            dFile.flush();
            #endif //DEBUG
            if( FW_built <  1 ||
               FW_built > 53   )
            {
               id2.value_l = 0;
               machine_name[0] = NULL;
            }
            // Fix problem for following elections ...
            //////////////////////////////////////////
            // else if( Determine_System( atol( machine_name)) != 'M')
            // {
            //    char forced_machine_name[9];
            //    sprintf( forced_machine_name,
            //           "%c99%05lu", machine_name[0],
            //                     atol( &machine_name[3]) - 70000);
            //    id2.value_l = atol( forced_machine_name);
            //    forcedMAV   = TRUE;
            // }
         }
         if(      bios == FS_SCENIC_D1561 )
         {
            if( snTested)
               strcpy( SerialNo_D1561[CSN].Value, machine_name);

            setDrvLetter();
            SerialNoCmd = fopen( &snParamFile1[1], "wt" );
            fprintf( SerialNoCmd, "CSN.w=\"SN[%s]\"\n", SerialNo_D1561[CSN].Value);
            fprintf( SerialNoCmd, "ChAT.w=\"Steria/eVote\"\n");
            fclose( SerialNoCmd);

            LoadProgram( snProgFile1, snParamFile1, snLogFile);

            remove( &snParamFile1[1]);
            remove( snLogFile);
         }
         else if( bios == FS_SCENICO_D1711)
         {
            char Buffer[100];
            char returning[] = "Returning ";
            char theParams[64];

            if( snTested)
               strcpy( SerialNo_D1711[CSN].Value, machine_name);

            setDrvLetter();
            sprintf( theParams, "-CSN=\"SN[%s]\" -ChAT=\"Steria/eVote\"",
                                SerialNo_D1711[CSN].Value);
            LoadProgram( snProgFile2, theParams, snLogFile);
            ifstream RESULTS( snLogFile);
            while( !RESULTS.eof() && RESULTS)
            {
               RESULTS.getline( Buffer, sizeof( Buffer));
               if( RESULTS)
                  if( strlen( Buffer) > 0)
                     if( !strncmp( Buffer, returning, strlen( returning))) {
                        snReturn = Buffer[strlen( returning)] - '0';
                        break;
                     }
            }
            RESULTS.close();
            remove( snLogFile);

            snFailed = snReturn == 0 ? FALSE : TRUE;
		 }
         else
         {
            for( i = 0 ; i < _AWARD_ ; i++)
               SetByteFromCMOS( AWARD_addr[i], id2.value_c[i]);

            SetByteFromCMOS( AWARD_flag[0], forcedMAV
                                    ? FW_built
                                    : Calculate_Mod97( id2.value_l));
            SetByteFromCMOS( AWARD_flag[1], Determine_System( id2.value_l));
         }
         break;
   }
   #ifdef DEBUG
   dFile << "<<WriteMachineName\n";
   dFile.flush();
   #endif //DEBUG
   return;
#endif // _LIGHTPEN_
}

//=============================================================================

#ifndef _LIGHTPEN_
BOOLEAN IsShadowOn( void)
{
   unsigned int value;

   switch( Bios_Test())
   {
      case AMI_1  :
      case AMI_2  :
      case AMI_3  :
      case BAD_AMI:
         if( GetByteFromCMOS( 0x34) & 0x3F ||  // ??11 1111
            GetByteFromCMOS( 0x35) & 0xF0   ) // 1111 ????
            return( TRUE);
         else
            return( FALSE);

      case AWARD_P5SJ_B:
         if( ( GetByteFromCMOS( 0x3E) >> 7 & 0x01) == 1 &&  // 1??? ????
            ( GetByteFromCMOS( 0x53) >> 5 & 0x01) == 1 &&  // ??1? ????
            ( GetByteFromCMOS( 0x53) >> 6 & 0x01) == 1   ) // ?1?? ????
            return( FALSE);
         else
            return( TRUE);

      case AWARD_W6178MS:
         // Because we don't know for the moment how and where
         // to update the checksum, we force as if it's all OK
         if( ( GetByteFromCMOS( 0x40)      & 0x01) == 1 &&  // ???? ???1
            ( GetByteFromCMOS( 0x40) >> 7 & 0x01) == 1   ) // 1??? ????
            // return( TRUE);
            return( FALSE);
         else
            return( FALSE);
   }
   return( FALSE);
}

//=============================================================================

void SetShadowOff()
{
   // set the Video ROM Shadow ON and the System ROM Shadow ON, all others
   // are not activated
   unsigned char value, checksum;

   switch( Bios_Test())
   {
      case AMI_1  :
      case AMI_2  :
      case AMI_3  :
      case BAD_AMI:
         value = GetByteFromCMOS( 0x34) & 0xC0;
         SetByteFromCMOS( 0x34, value);
         value = (GetByteFromCMOS( 0x35) & 0x01) | 0x0E;
         SetByteFromCMOS( 0x35, value);
         break;

      case AWARD_P5SJ_B:
         // Set 'Video BIOS Shadow' to Enable
         sByte.Value  = GetByteFromCMOS( 0x3E);
         if( sByte.Bit.p7 != 1)
         {
            sByte.Bit.p7 = 1;
            SetByteFromCMOS( 0x3E, sByte.Value);
         }
         // Set 'Video BIOS Cachable' to Enable  -> Bit.p5
         // Set 'System BIOS Cachable' to Enable -> Bit.p6
         sByte.Value  = GetByteFromCMOS( 0x53);
         if( sByte.Bit.p5 != 1 ||
            sByte.Bit.p6 != 1   )
         {
            sByte.Bit.p5 = 1;
            sByte.Bit.p6 = 1;
            SetByteFromCMOS( 0x53, sByte.Value);
         }
         break;

      case AWARD_W6178MS:
         // Set 'Video BIOS Cachable' to Enable  -> Bit.p7
         // Set 'System BIOS Cachable' to Enable -> Bit.p0
         sByte.Value  = GetByteFromCMOS( 0x40);
         if( sByte.Bit.p0 != 1 ||
            sByte.Bit.p7 != 1   )
         {
            sByte.Bit.p0 = 1;
            sByte.Bit.p7 = 1;
            SetByteFromCMOS( 0x40, sByte.Value);
         }
         break;
   }
}

//=============================================================================

void RebootMachine()
{
   asm {
      mov ax, 0xFFFF;
      push ax;
      xor ax, ax;
      push ax;
      retf;
   }
}

//=============================================================================

void SetKeyboardCheck(BOOLEAN On)
{
   unsigned int ByteFromCMOS;

   switch( Bios_Test())
   {

      case AMI_1   :
      case AMI_2   :
      case AMI_3   :
      case BAD_AMI :
         SetByteFromCMOS( 0x14,On ? (GetByteFromCMOS(0x14) | 0x04)
                            : (GetByteFromCMOS(0x14) & 0xfb));
         break;

      case AWARD_P5SJ_B :
      case AWARD_W6178MS:
         // Bin = ?????000 All Errors
         // Bin = ?????001 No Errors
         // Bin = ?????010 All, But Keyboard
         // Bin = ?????100 All, But Diskette
         // Bin = ?????110 All,But Disk/Key
         sByte.Value = GetByteFromCMOS( 0x3E);
         if( ( sByte.Value & 0x07) != ( On ? 0x00 : 0x02))
         {
            sByte.Bit.p0 = 0;
            sByte.Bit.p1 = On ? 0 : 1;
            sByte.Bit.p2 = 0;
            SetByteFromCMOS( 0x3E, sByte.Value);
         }
         break;

      default:
         break;
   }
}

//=============================================================================

unsigned GetBaseMemorySize()
{
   unsigned BaseMemory;

   BaseMemory  = GetByteFromCMOS( 0x16)<<8;
   BaseMemory += GetByteFromCMOS( 0x15);

   #ifdef DEBUG
   sprintf( dMsg, "Base Memory : %uKB\n", BaseMemory);
   dFile << dMsg;
   dFile.flush();
   #endif //DEBUG

   return( BaseMemory);
}

//=============================================================================

unsigned GetExtMemorySize()
{
   unsigned ExtMemory;

   ExtMemory  = GetByteFromCMOS( 0x18)<<8;
   ExtMemory += GetByteFromCMOS( 0x17);

   #ifdef DEBUG
   sprintf( dMsg, "Ext. Memory : %uKB\n", ExtMemory);
   dFile << dMsg;
   dFile.flush();
   #endif //DEBUG

   return( ExtMemory);
}

//=============================================================================

void SetConfig( BOOLEAN On)
{
   unsigned int IDE_CMOS, Resource_CMOS, IRQ10_CMOS;

   SetKeyboardCheck( On);
   switch( Bios_Test())
   {
      case AWARD_P5SJ_B :
         sByte.Value   = GetByteFromCMOS( 0x56); // Internal PCI/IDE
         if( sByte.Bit.p2 != ( On ? 1 : 0) ||
            sByte.Bit.p3 != ( On ? 1 : 0)   )
         {
            sByte.Bit.p2  = On ? 1 : 0;
            sByte.Bit.p3  = On ? 1 : 0;
            SetByteFromCMOS( 0x56, sByte.Value);
         }
         sByte.Value   = GetByteFromCMOS( 0x58); // Resource Controlled By
         if( sByte.Bit.p0 != 1)
         {
            sByte.Bit.p0  = 1;
            SetByteFromCMOS( 0x58, sByte.Value);
         }
         sByte.Value   = GetByteFromCMOS( 0x59); // IRQ-10
         if( sByte.Bit.p5 != 1)
         {
            sByte.Bit.p5  = 1;
            SetByteFromCMOS( 0x59, sByte.Value);
         }
         break;

      case AWARD_W6178MS:
         sByte.Value   = GetByteFromCMOS( 0x45); // On-Chip Primary PCI IDE
         if( sByte.Bit.p7  != ( On ? 1 : 0))
         {
            sByte.Bit.p7  = On ? 1 : 0;
            SetByteFromCMOS( 0x45, sByte.Value);
         }
         sByte.Value   = GetByteFromCMOS( 0x46); // On-Chip Secondary PCI IDE
         if( sByte.Bit.p3  != ( On ? 1 : 0))
         {
            sByte.Bit.p3  = On ? 1 : 0;
            SetByteFromCMOS( 0x46, sByte.Value);
         }
         break;
   }
}

//=============================================================================

BOOLEAN Check_Configuration()
{
   switch( Bios_Test())
   {
      case AMI_1       :
      case AMI_2       :
      case AMI_3       :
      case BAD_AMI     :
         // Not used as BUREAUTIQUE
         return( TRUE);

      case AWARD_P5SJ_B :
         if(((GetByteFromCMOS( 0x56)>>2&0x03) == 0) &&  // Internal PCI/IDE : Disabled
            ((GetByteFromCMOS( 0x58)>>0&0x01) == 1) &&  // Resources Controlled By : Manual
            ((GetByteFromCMOS( 0x59)>>5&0x01) == 1)   ) // IRQ-10 : Legacy ISA
            return( TRUE);
         else
            return( FALSE);

      case AWARD_W6178MS:
         if(((GetByteFromCMOS( 0x45)>>7&0x01) == 0) &&  // On-Chip Primary PCI IDE
            ((GetByteFromCMOS( 0x46)>>3&0x01) == 0)   ) // On-Chip Secondary PCI IDE
            return( TRUE);
         else
            return( FALSE);
   }
   return FALSE;
}

//=============================================================================

BOOLEAN Bureautica_Present( void)
{
   switch( Bios_Test())
   {
      case AWARD_W6178MS:
      case AWARD_P5SJ_B :
         // Only allowed BIOS versions
         break;

      default           :
         // Assume no HDD/CD/... present if BIOS/CMOS layout unknown
         return( FALSE);
   }

   char MachineIDName[8+1];
   long t, c, h, s;
   int  i;
   BOOLEAN HDD_present;
   #define NbrAWARD 4
   struct Info_IDE_AWARD
   {
      unsigned char addrCyl;
      unsigned char addrHd;
      unsigned char addrSect;
      unsigned char addrType;
   } IDE_AWARD[NbrAWARD]
   = { { 0x26, 0x28, 0x2D, 0x19},
      { 0x1E, 0x20, 0x25, 0x1A},
      { 0x71, 0x73, 0x78, 0x67},
      { 0x68, 0x6A, 0x6F, 0x70} };

   #ifdef CMOS
   GetMachineName( MachineIDName, FALSE);
   #else // !CMOS
   GetMachineName( MachineIDName);
   #endif // CMOS
   if( Determine_System( atol( MachineIDName)) == 'T')
      return( TRUE);

   HDD_present = FALSE;
   for( i=0 ; i < NbrAWARD && !HDD_present; i++)
   {
      t= GetByteFromCMOS(IDE_AWARD[i].addrType);
      c=(GetByteFromCMOS(IDE_AWARD[i].addrCyl+1)<<8) +
         GetByteFromCMOS(IDE_AWARD[i].addrCyl);
      h= GetByteFromCMOS(IDE_AWARD[i].addrHd);
      s= GetByteFromCMOS(IDE_AWARD[i].addrSect);
      if( t!=0 && c!=0 && h!=0 && s!=0)
         HDD_present = TRUE;
   }
   switch( bios)
   {
      case AWARD_P5SJ_B :
         if( ( GetByteFromCMOS( 0x56)>>2&0x03) != 0) // Internal PCI/IDE
            HDD_present = TRUE;
         break;

      case AWARD_W6178MS:
         if(((GetByteFromCMOS( 0x45)>>7&0x01) != 0) ||  // On-Chip Primary PCI IDE
            ((GetByteFromCMOS( 0x46)>>3&0x01) != 0)   ) // On-Chip Secondary PCI IDE
            HDD_present = TRUE;
         break;
   }
   return( HDD_present);
}

//=============================================================================

BOOLEAN Save_ID_To_File( void)
{
   char MachineIDName[8+1];

   if( !access( MachineIDfile, 0x00))
      remove( MachineIDfile);
   ofstream ID( MachineIDfile);
   if( !ID)
      return( FALSE);
   #ifdef CMOS
   GetMachineName( MachineIDName, FALSE);
   #else // !CMOS
   GetMachineName( MachineIDName);
   #endif // CMOS
   if( !strcmp( MachineIDName, ""))
      strcpy( MachineIDName, "00000000");
   ID << MachineIDName;
   if( !ID)
   {
      ID.close();
      return( FALSE);
   }
   ID.close();
   return( TRUE);
}

//=============================================================================

BOOLEAN Restore_ID_From_File( void)
{
   char Saved_MachineIDName[8+1],
       MachineIDName[8+1];

   if( access( MachineIDfile, 0x00))
      return( FALSE);
   #ifdef CMOS
   GetMachineName( MachineIDName, FALSE);
   #else // !CMOS
   GetMachineName( MachineIDName);
   #endif // CMOS
   ifstream ID( MachineIDfile);
   if( !ID)
      return( FALSE);
   ID >> Saved_MachineIDName;
   if( !ID)
   {
      ID.close();
      return( FALSE);
   }
   ID.close();
   remove( MachineIDfile);
   if( strcmp( Saved_MachineIDName, MachineIDName))
      WriteMachineName( Saved_MachineIDName);
   return( TRUE);
}

//=============================================================================

BOOLEAN Save_RTC_To_File( char *Filename)
{
   unsigned int  addr, MaxAddr;
   unsigned char Byte;
   FILE *RTC_file;

   switch( Bios_Test())
   {
      case AWARD_W6178MS:
         MaxAddr = 0xFF;
         break;

      case AWARD_P5SJ_B :
         MaxAddr = 0x7F;
         break;

      default           :
         return( FALSE);
   }
   if( Filename == NULL)
      strcpy( Filename, RTCfile);

   if( ( RTC_file = fopen( Filename, "wb")) == NULL)
      return( FALSE);

   for( addr = 0x00 ; addr <= MaxAddr ; addr++)
   {
      Byte = GetByteFromCMOS( addr);
      if( fputc( Byte, RTC_file) != Byte)
      {
         fclose( RTC_file);
         return( FALSE);
      }
   }
   fclose( RTC_file);
   return( TRUE);
}

//=============================================================================

#ifndef TEST
BOOLEAN Restore_RTC_From_File( char *Filename)
{
   unsigned int  addr, MaxAddr;
   unsigned char Byte;
   FILE *RTC_file;

   switch( Bios_Test())
   {
      case AWARD_W6178MS:
         MaxAddr = 0xFF;
         break;

      case AWARD_P5SJ_B :
         MaxAddr = 0x7F;
         break;

      default           :
         return( FALSE);
   }
   if( Filename == NULL)
      strcpy( Filename, RTCfile);

   if( access( Filename, 0x00))
      return( FALSE);

   if( ( RTC_file = fopen( Filename, "rb")) == NULL)
      return( FALSE);

   if( filelength( fileno( RTC_file)) != ( MaxAddr + 1))
      return( FALSE);

   Get_Date();
   Get_Time();
   for( addr = 0x00 ; addr <= MaxAddr ; addr++)
      SetByteFromCMOS( addr, fgetc( RTC_file));
   Set_Time();
   Set_Date();

   fclose( RTC_file);
   return( TRUE);
}
#endif // TEST

//=============================================================================

#ifndef RTC_LOAD
#ifndef CMOS
#ifndef TEST
void VerifyIfDiagnosticDiskette( BOOLEAN Diagnostic)
{
   int ErrorMsg = 0;

   while( ErrorMsg != -1)
   {
      DosErrorNr = NO_ERROR;
      ErrorMsg   = -1;
      if( !IsDiskInserted( "A") )
         ErrorMsg = NoDiskPresent;
      else
         if( IsDiagnost() != Diagnostic)
            ErrorMsg = Diagnostic ? AskForDiagnosticDisk
                             : AskForOtherDisk;
         else
            if( IsWriteProtected() )
               ErrorMsg = WriteProtectedDisk;

      if( ErrorMsg != -1)
      {
         ClearTimeScale();
         SelectItem( Get_Msg( ErrorMsg),
                  ProblemSolved,
                  MaxPixelY - 30);
         EraseInstruction( Get_Msg( ErrorMsg));
      }
   }
}

//=============================================================================

#ifdef YELLOW_DISK
BOOLEAN Backup_BIOS_Configuration( char *MachineID)
{
   BOOLEAN Result = TRUE;
   char SystemRTC[MAXPATH],
       *MsgLine;
   int  i = 0,
       MiddleX,
       downY,
       currY = 100;

   EraseScreen();
   DrawTitle( Get_Msg( SaveBiosConfig));
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 3);
   settextjustify( CENTER_TEXT, BOTTOM_TEXT);
   setcolor( WHITE);
   MiddleX = getmaxx() / 2;
   downY   = textheight( "Font Height") + 5;
   while( strcmp( ( MsgLine = Get_Msg( BackupInfo + i++)), "NULL"))
   {
      outtextxy( MiddleX, currY, MsgLine);
      currY += downY;
   }
   setcolor( LIGHTGRAY);
   outtextxy( MiddleX, 410, Get_Msg( AskForBackup));
   switch( SelectItem( NULL,
                  AcceptConfiguration ,
                  getmaxy() - 40))
   {
      case 0: // YES
         settextstyle( TRIPLEX_FONT, HORIZ_DIR, 3);
         settextjustify( CENTER_TEXT, BOTTOM_TEXT);
         setcolor( BLACK);
         outtextxy( MiddleX, 410, Get_Msg( AskForBackup));
         VerifyIfDiagnosticDiskette( FALSE);
         sprintf( SystemRTC, "A:\\%s.RTC", MachineID);
         Result = Save_RTC_To_File( SystemRTC);
         settextstyle( TRIPLEX_FONT, HORIZ_DIR, 3);
         settextjustify( CENTER_TEXT, BOTTOM_TEXT);
         setcolor( BLACK);
         i     =   0;
         currY = 100;
         while( strcmp( ( MsgLine = Get_Msg( BackupInfo + i++)), "NULL"))
         {
            outtextxy( MiddleX, currY, MsgLine);
            currY += downY;
         }
         settextstyle( TRIPLEX_FONT, HORIZ_DIR, 5);
         setcolor( WHITE);
         outtextxy( MiddleX, getmaxy() / 8 * 5,
                  Result ? Get_Msg( RestoreSucceeded)
                       : Get_Msg( RestoreFailed));
         break;

      case 1: // NO
         settextstyle( TRIPLEX_FONT, HORIZ_DIR, 3);
         settextjustify( CENTER_TEXT, BOTTOM_TEXT);
         setcolor( BLACK);
         outtextxy( MiddleX, 410, Get_Msg( AskForBackup));
         break;
   }
   VerifyIfDiagnosticDiskette( TRUE);
   if( Result)
      Result = Save_ID_To_File();
   if( Result)
   {
      switch( Bios_Test())
      {
         case AWARD_P5SJ_B :
            Result = Restore_RTC_From_File( "A:\\P5SJ-B.RTC");
            break;

         case AWARD_W6178MS:
            Result = Restore_RTC_From_File( "A:\\W6178MS.RTC");
            break;
      }
   }
   if( Result)
      Result = Restore_ID_From_File();

   return( Result);
}

//=============================================================================

BOOLEAN Check_Yellow_Configuration( void)
{
   char *AskSystem,
       System = 'X',
       SystemType[15],
       MachineIDName[8+1];
   int  MiddleX, PartOfY;
   BOOLEAN retCode;
   static BOOLEAN AlreadyTested = FALSE;

   if( AlreadyTested)
      // To avoid indefinite loop of asking for the Machine ID
      // we force result of Check_Yellow_Configuration to FALSE !
      ///////////////////////////////////////////////////////////
      retCode = FALSE;
   else
   {
      AlreadyTested = TRUE;
      switch( Bios_Test())
      {
         case AWARD_W6178MS:
         case AWARD_P5SJ_B :
            // *****
            // Go on checking yellow configuration
            // *****
            break;

         default               :
            // *****
            // Abort checking yellow configuration
            // *****
            return( FALSE);
      }
      if( Bureautica_Present())
      {
         GetMachineName( MachineIDName);
         System = Determine_System( atol( MachineIDName));
         switch( System)
         {
            case 'M':
               strcpy( SystemType, "MAV");
               break;

            case 'U':
               strcpy( SystemType, "URN");
               break;

            case 'T':
               strcpy( SystemType, "TOT");
               break;

            case 'P':
               strcpy( SystemType, "PRP");
               break;

            default :
               System        = 'X';
               AlreadyTested = FALSE;
               break;
         }
         if( System != 'X')
         {
            EraseScreen();
            DrawTitle( Get_Msg( VerifySysInfo));
            MiddleX = getmaxx() / 2;
            PartOfY = getmaxy() / 8;
            settextstyle( TRIPLEX_FONT, HORIZ_DIR, 3);
            settextjustify( CENTER_TEXT, BOTTOM_TEXT);
            setcolor( LIGHTGRAY);
            outtextxy( MiddleX, PartOfY * 2, Get_Msg( NotForThisSystem));
            AskSystem = new char[strlen( Get_Msg( CorrectSystem)) +
                            strlen( SystemType) + 1];
            sprintf( AskSystem, Get_Msg( CorrectSystem), SystemType);
            outtextxy( MiddleX, 410, AskSystem);
            settextjustify( LEFT_TEXT, BOTTOM_TEXT);
            outtextxy( 30, PartOfY * 4, Get_Msg( DetectedSystem));
            outtextxy( 30, PartOfY * 5, Get_Msg( CurrentMachineID));
            settextstyle( TRIPLEX_FONT, HORIZ_DIR, 5);
            settextjustify( RIGHT_TEXT, BOTTOM_TEXT);
            setcolor( WHITE);
            outtextxy( getmaxx() - 30, PartOfY * 4, SystemType);
            outtextxy( getmaxx() - 30, PartOfY * 5, MachineIDName);
            switch( SelectItem( NULL,
                           AcceptConfiguration,
                           getmaxy() - 40))
            {
               case 0: // YES
                  if( System == 'M' ||
                     System == 'U'   )
                     if( Backup_BIOS_Configuration( MachineIDName))
                        RebootMachine();
                  break;

               case 1: // NO
                  AlreadyTested = FALSE;
                  WriteMachineName( "00000000");
                  break;
            }
         }
      }
      retCode = AlreadyTested;
   }
   return( retCode);
}
#endif // YELLOW_DISK
#endif // TEST
#endif // CMOS
#endif // RTC_LOAD
#endif // _LIGHTPEN_
//=============================================================================

#endif // !DIGTOOLS
//</source>
//<!-- eof -->
