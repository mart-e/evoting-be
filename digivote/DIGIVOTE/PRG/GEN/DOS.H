//=============================================================================
/*

   File        : DOS.H
   Creation    : 01/04/93

   Author      : B.S.S.

   Description : DOS.CPP header file

*/
//=============================================================================
/*h*/


#ifndef _DOS_H                        // avoid mutiple inclusions of this file
   #define _DOS_H

//============================ T Y P E D E F S ================================

typedef int Boolean;
const int FALSE=0;
const int TRUE =1;

#if defined(_APPLICATION_MAV) && defined(_PREVIEW)
   // library not used in preview
#else

extern "C" { Boolean OwnAbsRead( int, int, unsigned int, void *);
             Boolean OwnAbsWrite( int, int, unsigned int, void *); }

typedef struct fcb * DB;

struct DirEntryLayout
{
   unsigned char DriveIdentification;  // 0:CurrenDrive,
                                       // 1:DriveA,
                                       // 2:DriveB, ...
   char DatabaseName[8];               // No ending 0
   char DatabaseExtension[3];          // No ending 0
   unsigned char FileAttribute;
   unsigned char Reserved[10];
   unsigned short TimeLastChange;
   unsigned short DateLastChange;
   unsigned short ClusterStartNr;      // Link to FAT
   unsigned long DatabaseLength;
};

typedef struct DirEntryLayout *PtrDirEntryLayout;

struct BootSectorLayout
{
   unsigned char  BootAddress[3];     // NEAR JUMP
   char           OEMCompanyName[8];  // Not 0 terminated
   unsigned short BytesPerSector;
   unsigned char  SectorsPerCluster;
   unsigned short FirstSectorFAT;
   unsigned char  NrFATs;
   unsigned short RootDirectorySize;  // 32 Bytes each
   unsigned short TotalSectors;       // 0 if partition > 32Mb
   unsigned char  MediaDescriptor;
   unsigned short SectorsPerFAT;
   unsigned short SectorsPerTrack;
   unsigned short HeadCount;
   unsigned short HiddenSectors;
   unsigned long  TotalSectorsGT32Mb; // TotalSectors = 0
   unsigned char  PhysicalDriveNr;
   unsigned char  Reserved1;
   unsigned char  Signature;
   unsigned long  VolumeSerialNr;
   char           VolumeName[11];
   unsigned char  Reserved2[8];
   unsigned char  StartBootCode;      // Can continue on following sectors.
};

//=========================== P R O T O T Y P E S =============================

Boolean Dos_FindFirstDB( DB AllocatedDB, char * DatabaseName);
Boolean Dos_FindNextDB( DB FilledDB);
PtrDirEntryLayout Dos_GetDirEntryLayout();
unsigned short * Dos_GetClusterChain( DB FilledDB, unsigned short Cluster_Length);
void PrintDirEntryLayout( PtrDirEntryLayout DirEntryLayout);

#endif //#if defined(_APPLICATION_MAV) && defined(_PREVIEW)

#endif                                                // end of #ifndef _DOS_H

//=== DOS.H ===================================================================
