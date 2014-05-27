/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="disk.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   boilerplate.cpp  $</workFile>
  <revision>$Revision: 1.4 $</revision>
  <workFileTimeStamp>$Modtime:   Dec 17 2003 12:32:10  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/boilerplate.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2006/10/26 12:48:17 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>*/

#include <dos.h>
#include <dir.h>
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <graphics.h>

#include "..\Diagnost\Disk.h"
#include "..\Diagnost\DosError.h"

#ifndef DIGTOOLS

#if !defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)

#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <fstream.h>

#include "Diagnost.h"
#include "TimeScal.h"
#include "Msg.h"
#include "Setup.h"
#include "Config.h"
#include "IDE_Info.h"

#endif //!defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)

#else // DIGTOOLS

#define FALSE 0
#define TRUE  1

#endif // DIGTOOLS

//=============================================================================

#ifndef DIGTOOLS
char specDrive[3];
char typeDrive[45];
#endif // !DIGTOOLS

int loopCount = 0;

//=============================================================================

#pragma warn +par
char Get_RAMdrive( BOOLEAN Force_Drive)
{
   char Drive;
   struct find_t Control_Block;
   char PathName[6];
   BOOLEAN Found=FALSE;

   Found = FALSE;
   InstallDosErrorHandler();
   for( Drive='Z' ; Drive>='C' ; Drive--)
   {
      sprintf( PathName, "%c:*.*", Drive);
      if( _dos_findfirst( PathName, _A_VOLID, &Control_Block)==0)
         if( strcmp( Control_Block.name, "MS-RAMDR.IVE")==0)
         {
            Found = TRUE;
            break;
         }
   }
   if( !Found)
      if( Force_Drive)
         Drive = 'A' + getdisk();
      else
         Drive = '#';
   #ifndef DIGTOOLS
   #if !defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)
   DosErrorNr = 0;
   #endif //!defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)
   #endif
   return( Drive);
}

//=============================================================================

#ifndef DIGTOOLS

#if !defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)

static TerminatorType WriteReadFile( void *Ptr,               // Input : FileName
                                                // Output: TestResult
                            unsigned long NrCalls,
                            unsigned long MaxNrCalls) // Please >=23
{
   // Due to the fact that this function is called form ObjTimeOut,
   // we must be sure that the contents of Buffer1 and Buffer2
   // remains intact, otherwise we will eventually have a verify error
   static FILE *fp;
   static char *FileName;
   static char Buffer1[sizeTestFile];
   static char Buffer2[sizeof( Buffer1)];
   char RAMdrive;
   int x, y, prevPixel;

   if( NrCalls == 0)
   {
     // Position text-cursor in case DOS takes over ...
     gotoxy( 1, 4);

     memset( Buffer1, 'a', sizeof( Buffer1));
     FileName = *(char **)Ptr;

     DosErrorNr = 0;
     remove( FileName);
     fp = fopen( FileName, "a+b");
     if( DosErrorNr) return( CANCELLED);
     if( !fp)
     {
       *(char **)Ptr = Get_Msg( DiskCanNotOpenFile);
       return( CANCELLED);
     }

     if( FileName[0] == 'B')
     {
       prevPixel = getpixel( 0, TitleBottom + 1);
       for( x = 0 ; x < getmaxx(); x++)
       {
         for( y = TitleBottom + 1 ; y < ConfigurationTop ; y++)
            if( getpixel( x, y) != prevPixel)
            {
              x = -1;
              y = -1;
              break;
            }
         if( x == -1 && y == -1)
            break;
       }
       if( x == -1 && y == -1)
       {
         // Erase DOS message
         setfillstyle( SOLID_FILL, BLACK);
         bar( 0, TitleBottom + 1, getmaxx(), ConfigurationTop -1);
         fclose( fp);
         sleep( 1);
         // Force default to drive A (again)
         FileName[0] = 'A';
         gotoxy( 1, 4);
         fp = fopen( FileName, "a+b");
         fclose( fp);
         // Erase DOS message
         setfillstyle( SOLID_FILL, BLACK);
         bar( 0, TitleBottom + 1, getmaxx(), ConfigurationTop -1);
         // Troubles detected when trying to open file on drive B:
         IgnoreRetryTestOnce = TRUE;
         *(char **)Ptr = Get_Msg( DosErrorMsg + 1);
         return( CANCELLED);
       }
     }

     *(char **)Ptr = Get_Msg( OK);
   }

   else if( NrCalls == MaxNrCalls)
   {
     fclose( fp);
     unlink( FileName);

     if( DosErrorNr)
       return( CANCELLED);
   }

   else if( NrCalls == 1)
   {

     fseek( fp, SEEK_SET, 10L);
     if( DosErrorNr)
       return( CANCELLED);
   }

   else if (NrCalls <= 1+10)
   {
     fwrite( Buffer1, sizeof( Buffer1), 1, fp);   //Write buffer1 to file
     if( DosErrorNr)
       return( CANCELLED);
   }

   else if( NrCalls == 12)
   {
     fclose( fp);
     fp = fopen( FileName, "a+b");
     if( DosErrorNr)
       return( CANCELLED);
     if( !fp)
     {
       *(char **)Ptr = Get_Msg( DiskCanNotOpenFile);
       return( CANCELLED);
     }

     fseek(fp,SEEK_SET,10L);

     if( DosErrorNr)
       return( CANCELLED);
   }

   else if( NrCalls <= 12+10)
   {
     fread( Buffer2, sizeof( Buffer2), 1, fp);    //Read buffer2 from file

     if( DosErrorNr)
       return( CANCELLED);

     if( memcmp((char *)Buffer1, (char *)Buffer2, sizeof(Buffer1)))
     {
       fclose( fp);
       unlink( FileName);
       *(char **)Ptr = Get_Msg( DiskVerifyError);
       return( CANCELLED);
     }
   }

   return( CONTINUE);
}

//=============================================================================

static BOOLEAN DrawWriteReadFile( int Heading, unsigned int Y, char FileName[])
{
   char Drive[45], *Instruction;
   const int loopAction = 10;

   if( CurrExe == _TSTMAVD1_ && ContinuousLoop && ( ++loopCount % loopAction))
   {
     sprintf( Drive, "%s [%d/%d]", Get_Msg( Cancelled), loopCount % loopAction, loopAction);
     return( PrintResult( Heading, Y, noIndex, Drive));
   }

   switch( toupper( FileName[0]))
   {
     case 'A':
       if( ( GetByteFromCMOS( 0x10)>>4)  == 0x0)
       {
         IgnoreRetryTestOnce = TRUE;
         return( PrintTestResult( Heading, Y, DiskCanNotOpenFile));
       }
       break;

     case 'B':
       if( ( GetByteFromCMOS( 0x10)&0xF) == 0x0)
       {
         IgnoreRetryTestOnce = TRUE;
         return( PrintTestResult( Heading, Y, DiskCanNotOpenFile));
       }
       break;

     case '?':
       IgnoreRetryTestOnce = TRUE;
       if( StringPartOf( typeDrive, "SyQuest"))
         return( PrintTestResult( Heading, Y, SyQuestNotPresentAtBootTime));
       else
         return( PrintTestResult( Heading, Y, DiskCanNotOpenFile));
   }
   if( toupper( FileName[0]) > 'A')
   {
     if( toupper( FileName[0]) == 'B')
     {
       strncpy( Drive, FileName, 2);
       Drive[2]='\0';
     }
     else
       strcpy( Drive, typeDrive);

     if( !StringPartOf( Drive, "SyQuest"))
     {
       Instruction = ( char *) malloc( sizeof( Get_Msg( AskDiskInDrive))
                              + sizeof( Drive));
       sprintf( Instruction, Get_Msg( AskDiskInDrive), Drive);
       ClearTimeScale();
       SelectItem( Instruction,
                EnterToContinue,
                getmaxy() - 30);
       free( Instruction);
     }
   }

   remove( FileName);
   #ifndef FAST
   ObjTimeScale( 23, WriteReadFile, &FileName);
   #else
   ObjTimeScale(  1, WriteReadFile, &FileName);
   #endif

   if( DosErrorNr)
     return( PrintDosError( Heading, Y));

   return( PrintResult( Heading, Y, noIndex, FileName));
}

//=============================================================================

char *Drive_RemovableDisk( void)
{
   return( ( char *) specDrive);
}

//=============================================================================

char *Type_RemovableDisk( void)
{
   return( ( char *) typeDrive);
}

//=============================================================================

static char DrvLetter_RemovableDisk( int Disk)
{
   char DrvLetter = '?';
   int i, j;

   for( i = 0, j = 0 ; i < NbrDrive ; i++)
     if( StringPartOf( Get_DrvType( i), "ZIP"    ) ||
        StringPartOf( Get_DrvType( i), "SyQuest")   )
     {
       j++;
       if( j == Disk)
       {
          DrvLetter = Get_DrvLetter( i);
          sprintf( specDrive, "%c:", DrvLetter);
          strcpy( typeDrive, Get_DrvType( i));
          // Suppress leading spaces in typeDrive ...
          for( i = strlen( typeDrive) - 1 ; i > 0 ; i--)
            if( typeDrive[i] == ' ')
               typeDrive[i] = '\0';
            else
               break;

          break;
       }
     }
   return( DrvLetter);
}

//=============================================================================

             // MAIN ROUTINES //
             ///////////////////

//=============================================================================

BOOLEAN DrawRamDriveTest( int Heading, unsigned int Y)
{
      char *fnCONFIG_SYS   = { "A:\\CONFIG.SYS" };
      char *fnRAMDRIVE_SYS = { "RAMDRIVE.SYS"   };
      struct diskfree_t dinfo;
      long config_size = -1,
          total_size  = -1;
      char infoRAMdrive[80];
      char Buffer[100];
      int i, j;

   if( Get_RAMdrive( FALSE) == '#')
   {
      IgnoreRetryTestOnce = TRUE;
      PrintTestResult( Heading, Y, NotPresent);
      return( FALSE);
   }
   else
   {
      if( !_dos_getdiskfree(Get_RAMdrive( TRUE) - 'A' + 1, &dinfo))
         // calculate real ramdrive size
         total_size = (long) dinfo.total_clusters
                  * (long) dinfo.bytes_per_sector
                  * (long) dinfo.sectors_per_cluster
                  / 1024;

      // search for configured ramdrive size (CONFIG.SYS)
      ifstream CONFIG( fnCONFIG_SYS);
      while( !CONFIG.eof() && CONFIG)
      {
         CONFIG.getline( Buffer, sizeof( Buffer));
         if( CONFIG)
         {
            if( strlen( Buffer) > 0)
            {
               for( i = 0 ; i < strlen( Buffer) ; i++)
                  Buffer[i] = toupper( Buffer[i]);
               if( StringPartOf( Buffer, fnRAMDRIVE_SYS))
               {
                  while( strlen( Buffer) > 0)
                  {
                     if( !strncmp( Buffer,
                                fnRAMDRIVE_SYS,
                                strlen( fnRAMDRIVE_SYS)))
                     {
                        strcpy( Buffer, &Buffer[strlen( fnRAMDRIVE_SYS)]);
                        while( Buffer[0] < '0' ||
                              Buffer[0] > '9'   )
                           strcpy( Buffer, &Buffer[1]);
                        for( i = 0 ; i < strlen( Buffer) ; i++)
                           if( Buffer[i] < '0' ||
                              Buffer[i] > '9'   )
                           {
                              Buffer[i] = '\0';
                              break;
                           }
                        config_size = atol( Buffer);
                        break;
                     }
                     strcpy( Buffer, &Buffer[1]);
                  }
                  break;
               }
            }
         }
      }
      CONFIG.close();

      if(      config_size == -1 &&
             total_size  == -1   )
         sprintf( infoRAMdrive, "%s", Get_Msg( OK));
      else if( config_size == -1   )
         sprintf( infoRAMdrive, "%s [R:%ldKb]", Get_Msg( OK),
                                       total_size);
      else if( total_size == -1   )
         sprintf( infoRAMdrive, "%s [C:%ldKb]", Get_Msg( OK),
                                       config_size);
      else
         sprintf( infoRAMdrive, "%s [R:%ldKb / C:%ldKb]", Get_Msg( OK),
                                              total_size,
                                              config_size);
      PrintResult( Heading, Y, noIndex, infoRAMdrive);
      return( TRUE);
   }
}

//=============================================================================

BOOLEAN DrawFloppyATest( int Heading, unsigned int Y)
{
   return( DrawWriteReadFile( Heading, Y, "A:\\$_TEST_$"));
}

//=============================================================================

BOOLEAN DrawFloppyBTest( int Heading, unsigned int Y)
{
   return( DrawWriteReadFile( Heading, Y, "B:\\$_TEST_$"));
}

//=============================================================================

BOOLEAN DrawRemovableDisk1Test( int Heading, unsigned int Y)
{
   char TestFile[] = "?:\\$_TEST_$";

   TestFile[0] = DrvLetter_RemovableDisk( 1);
   return( DrawWriteReadFile( Heading, Y, TestFile));
}

//=============================================================================

BOOLEAN DrawRemovableDisk2Test( int Heading, unsigned int Y)
{
   char TestFile[] = "?:\\$_TEST_$";

   TestFile[0] = DrvLetter_RemovableDisk( 2);
   return( DrawWriteReadFile( Heading, Y, TestFile));
}

//=============================================================================

#endif //!defined(_LIGHTPEN_) && !defined(CMOS) && !defined(TEST)

#endif // !DIGTOOLS

//=== DISK.CPP ================================================================
//</source>
//<!-- eof -->
