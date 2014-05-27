/*H*/
//=============================================================================
/*


   File        : DOS.CPP
   Creation    : 01/04/93

   Author      : B.S.S.

   Description : functions to acces the disk characteristics via the low
                 level DOS functions

*/
//=============================================================================
/*h*/
/*R*/
//=============================================================================
/*

   Revision History
   ################

   01/06/93: revision history starting date

*/
//=============================================================================
/*r*/
/*#*/

#if defined(_APPLICATION_MAV) && defined(_PREVIEW)
   // library not used in preview
#else

//======================== I N C L U D E   F I L E S ==========================
#include <alloc.h>
#include <mem.h>
#include <dos.h>

#include "..\gen\dos.h"

//============================= D E F I N E S =================================

#define Year( DateLastChange)   ( DateLastChange >> 9)
#define Month( DateLastChange) (( DateLastChange >> 5) & 0x0F)
#define Day( DateLastChange)    ( DateLastChange & 0x1F)

#define Hours( TimeLastChange)  ( TimeLastChange >> 11)
#define Mins( TimeLastChange)  (( TimeLastChange >> 5) & 0x3F)
#define Secs( TimeLastChange)   ( TimeLastChange & 0x1F)

//=================== E X T E R N A L   V A R I A B L E S =====================

extern struct BootSectorLayout *BootSector;

//===================== G L O B A L   V A R I A B L E S =======================

void * DTA_Address = NULL;
long DTA_Length = 0;

//================================= B O D Y ===================================

/*{*/
//=============================================================================
//
// Function : Dos_GetNextCluster
//
// Purpose  : returns the next cluster found in the FAT
//
// Return   : static void  , local function
//
// Arg List : unsigned char * FAT               , the complete FAT contents
//            unsigned char TypeFAT             , type of the FAT 12 or 16 bits
//            unsigned short * CurrentCluster   , the new current cluster
//
// Globals  : none
//
// Usage    : Dos_GetNextCluster( FAT, TypeFAT, & CurrentCluster);
//
// Checks   :
//
// Comment  : At the end of a chain CurrentCluster will have the value 0xFFFF.
//            Refer to FAT description in Helppc for a complete explanation on
//            how to calculate the next cluster position.
//
//-----------------------------------------------------------------------------
/*}*/

static void Dos_GetNextCluster( unsigned char * FAT,
                                unsigned char TypeFAT,
                                unsigned short * CurrentCluster)
{
   unsigned short Offset;

   switch (TypeFAT)
   {
      case 12: Offset = * CurrentCluster * 3 / 2;

               if( * CurrentCluster & 1)                       // High 12 Bits
                   * CurrentCluster = * (( unsigned short *)( FAT + Offset)) >> 4;

               else                                             // Low 12 Bits
                   * CurrentCluster = * (( unsigned short *)( FAT + Offset)) & 0xFFF;

               if( * CurrentCluster > 0x0FEF)
                   * CurrentCluster = 0xFFFF;

               break;

      case 16: Offset = * CurrentCluster * 2;
               * CurrentCluster = * (( unsigned short *)( FAT + Offset));

               if( * CurrentCluster > 0xFFEF)
                   * CurrentCluster = 0xFFFF;

               break;

      default: * CurrentCluster = 0xFFFF;
               break;
   }
}

/*{*/
//=============================================================================
//
// Function : Dos_FindFirstDB
//
// Purpose  : Searches for occurence of file, using FCB
//
// Return   : BOOLEAN   , TRUE  = file was found
//                        FALSE = file was not found
//
// Arg List : DB AllocatedDB    , where to put the result
//            char * FileName   , the file name to search
//
// Globals  : none
//
// Usage    : if( Dos_FindFirstDB( Fcb, File_Name))
//
// Checks   :
//
// Comment  : See Dos function 0x11 of interrupt 0x21 in Helppc.
//            Only Databases in the current directory can be found.
//
//-----------------------------------------------------------------------------
/*}*/

Boolean Dos_FindFirstDB( DB AllocatedDB, char * FileName)
{
   // parse filename and buil the Filme Control Block (FCB)
   if( parsfnm( FileName, AllocatedDB, 1) == NULL)
       return( FALSE);

   asm {
         push ds
         mov  ah,0x11
         lds  dx,AllocatedDB
         int  0x21
         pop  ds
       }

   if( _AL == 255)
       return(FALSE);

   return(TRUE);
}

/*{*/
//=============================================================================
//
// Function : Dos_FindNextDB
//
// Purpose  : find the next occurence of the filename
//
// Return   : BOOLEAN   , TRUE  = file was found
//                        FALSE = file was not found
//
// Arg List : DB FilledDB  ,
//
// Globals  : none
//
// Usage    :
//
// Checks   :
//
// Comment  : See Dos function 0x12 of interrupt 0x21 in Helppc.
//            Only Databases in the current directory can be found.
//
//-----------------------------------------------------------------------------
/*}*/

/*
Boolean Dos_FindNextDB( DB FilledDB)
{
   asm {
         push ds
         mov  ah,0x12
         lds  dx,FilledDB
         int  0x21
         pop  ds
       }

   if( _AL == 255)
       return( FALSE);

   return( TRUE);
}
*/

////////////////////////////////////////////////////////////////////////////
// Get DirectoryEntryLayout of a found DB.           (See DOS FUNCTION 0x0F)
//

PtrDirEntryLayout Dos_GetDirEntryLayout()
{
   if( ! DTA_Address)
   {
      DTA_Address = (char *) getdta();
      DTA_Length = 0x20;
   }

   return( PtrDirEntryLayout( DTA_Address));
}

/*{*/
//=============================================================================
//
// Function : Dos_GetClusterChain
//
// Purpose  : get the cluster chain used by the file
//
// Return   : unsigned short *   , pointer to the cluaster chain
//
// Arg List : DB FilledDB,                   , the DB describing the file
//            unsigned short Cluster_Length  , the cluster lenght
//
// Globals  : struct BootSectorLayout * BootSector  , structure describing the
//                                                    boot sector
//
// Usage    : * File_Chain = Dos_GetClusterChain( & Fcb, Cluster_Length);
//
// Checks   :
//
// Comment  : FilledDB has to be filled with Dos_FindFirstDB, BootSectorLayout
//            must also be filled first.
//            The array returned by this routine is allocated in memory, when
//            its contents is no longer needed, please free this array.
//
//-----------------------------------------------------------------------------
/*}*/

unsigned short * Dos_GetClusterChain( DB FilledDB,
                                      unsigned short Cluster_Length)
{
   unsigned char * FAT;
   unsigned short * Result;
   unsigned char TypeFAT;           // 12:FAT12 or 16:FAT16
   unsigned short FirstDataSector;
   unsigned short CurrentCluster;
   unsigned short NrClusters = 0;

   FirstDataSector = ( BootSector -> RootDirectorySize * 32)
                                   / BootSector -> BytesPerSector +
                     ( BootSector -> SectorsPerFAT * BootSector -> NrFATs) +
                       BootSector -> FirstSectorFAT;

   // According to DOS 3.2
   TypeFAT = BootSector -> TotalSectors > 20740 ? 16 : 12;

   // Read Complete FAT
   FAT = ( unsigned char *) malloc( BootSector -> SectorsPerFAT *
                                    BootSector -> BytesPerSector);
   if( ! FAT)
       return( NULL);

   if( OwnAbsRead( FilledDB -> fcb_drive - 1, BootSector -> SectorsPerFAT,
       BootSector -> FirstSectorFAT, FAT) != 0)
   {
      free( FAT);
      FAT = NULL;
      return( NULL);
   }

   // Get the number of clusters used by the file
   PtrDirEntryLayout PtrDirEntryLayoutOfFile = Dos_GetDirEntryLayout();
   int TotalClusters = (int)(( PtrDirEntryLayoutOfFile -> DatabaseLength +
                              (long)( Cluster_Length - 1)) / Cluster_Length);

   // Save the found AbsClusters
   CurrentCluster = PtrDirEntryLayoutOfFile -> ClusterStartNr;

   // Allocate Result
   Result = (unsigned short *)malloc( (TotalClusters + 1) * sizeof( unsigned short));

   if( ! Result)
   {
      free( FAT);
      FAT = NULL;
      return( NULL);
   }

   while( CurrentCluster != 0xFFFF)
   {
      Result [NrClusters++] = ( CurrentCluster - 2) * BootSector -> SectorsPerCluster
                              + FirstDataSector;
      Dos_GetNextCluster( FAT, TypeFAT, & CurrentCluster);
   }

   Result [NrClusters++] = NULL;
   free( FAT);
   FAT = NULL;
   return( Result);
}

/*{*/
//=============================================================================
//
// Function : PrintDirEntryLayout
//
// Purpose  : Prints the Directory Entry Layout.
//
// Return   : void
//
// Arg List : PtrDirEntryLayout DirEntryLayout  , the directory layout
//
// Globals  : none
//
// Usage    :
//
// Checks   :
//
// Comment  :
//
//-----------------------------------------------------------------------------
/*}*/

/*
void PrintDirEntryLayout( PtrDirEntryLayout DirEntryLayout)
{
   unsigned short i;

   printf( "DriveIdentification:%d\n", DirEntryLayout -> DriveIdentification);
   printf( "DatabaseName:%8.8s.%3.3s\n", DirEntryLayout -> DatabaseName,
                                         DirEntryLayout -> DatabaseExtension);
   printf( "FileAttribute:%x\n", DirEntryLayout -> FileAttribute);
   printf( "Reserved:");

   for( i=0; i <= 8; i++)
       printf( "%x",DirEntryLayout -> Reserved[i]);

   printf( "\n");
   printf( "TimeLastchange:%d:%d:%d\n", Hours( DirEntryLayout -> TimeLastChange),
                                        Mins( DirEntryLayout -> TimeLastChange),
                                        Secs( DirEntryLayout -> TimeLastChange));
   printf( "DateLastChange:%d/%d/%d\n", Year( DirEntryLayout -> DateLastChange),
                                        Month( DirEntryLayout -> DateLastChange),
                                        Day( DirEntryLayout -> DateLastChange));
   printf( "ClusterStartNr:%d\n", DirEntryLayout -> ClusterStartNr);
   printf( "DatabaseLength:%d\n", DirEntryLayout -> DatabaseLength);
}
*/

//=== DOS.CPP =================================================================
#endif //#if defined(_APPLICATION_MAV) && defined(_PREVIEW)
