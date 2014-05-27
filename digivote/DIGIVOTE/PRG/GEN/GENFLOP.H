/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genflop.h"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.5 $</revision>
  <archiveTimeStamp>$Date: 2008/10/02 15:04:21 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>genflop.h</fileName>
  <creationDate>05/02/1993</creationDate>
  <rights>Steria Benelux</rights>
  <author>B.S.S.</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _GENFLOP                      // avoid mutiple inclusions of this file
   #define _GENFLOP

//--------------------------------------------------------------- INCLUDES ---
//<includes>
   #include "..\gen\gentype.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
typedef unsigned short *  FILE_CHAIN;

typedef enum { NOT_SAME, SAME, COMP_ABORT, CRC_ERROR_A, CRC_ERROR_B }
   FILE_COMP;
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
int Floppy_Error_Handler( int errval, int ax, int bp, int si);
void Floppy_Message( int M_Index_M, int  M_Index_B, BOOL Confirm = FALSE,
                             BOOL Append = FALSE, BOOL Exception = FALSE);

RET_CODE Floppy_Get_Label( char * Label);
RET_CODE Floppy_Request_Disk( void);
BOOL IsFloppyRemoved( void);
void Wait_Until_Floppy_Removed( void);
BOOL Floppy_Inserted( void);

RET_CODE Floppy_Open_File  ( char * File_Name, int * File_Handle);
RET_CODE Floppy_File_Length( int File_Handle, long * Length);
RET_CODE Floppy_Read_Buffer( int File_Handle, unsigned int * Length,
                             void * Buffer);
RET_CODE Floppy_Close_File( int File_Handle);
RET_CODE Floppy_Close_All( void);

FILE * RAMdrive_Open_File     ( char * File_Name);
FILE * RAMdrive_Create_File   ( char * File_Name);
RET_CODE RAMdrive_Read_Buffer ( FILE * File_Handle, int Length,
                                char * Buffer);
RET_CODE RAMdrive_Write_Buffer( FILE * File_Handle, char * Buffer);
RET_CODE RAMdrive_Close_File  ( FILE * File_Handle);

// Normally Floppy_Remove_File is only for _APPLICATION_URN
RET_CODE Floppy_Remove_File( char * File_Name);

// for _APPLICATION_URN only

RET_CODE Floppy_Create_File( char * File_Name, int * File_Handle);
RET_CODE Floppy_Write_Buffer( int File_Handle, unsigned int Length, void * Data);
void Floppy_Turn_Long( void);
RET_CODE Floppy_Flush_Files( void);
RET_CODE Floppy_Copy_File( char * Source_File, char * Dest_File);
FILE_COMP Floppy_Compare_Files( int File_Handle_A, int File_Handle_B, long File_Length);
RET_CODE Floppy_Read_At( int File_Handle, OFFSET Offs, void * Buffer);
RET_CODE Floppy_Read_Track( int File_Handle, OFFSET Offs, void * Buffer);
RET_CODE Floppy_Write_At( int File_Handle, OFFSET Offs, void * Buffer);
RET_CODE Floppy_Write_Track( int File_Handle, OFFSET Offs, void * Buffer);
RET_CODE Floppy_Rename_File( char * Old, char * New);

RET_CODE Floppy_Read_Cluster ( FILE_CHAIN File_Chain, int Cluster, char * Data);
RET_CODE Floppy_Read_Clusters( FILE_CHAIN File_Chain, int Cluster,
                               int Clusters_To_Read, char * Data);
RET_CODE Floppy_Write_Cluster( FILE_CHAIN File_Chain, int Cluster, char * Data);
RET_CODE Floppy_Get_File_Chain( char * File_Name, FILE_CHAIN * File_Chain);
RET_CODE Floppy_Get_FAT_Info( void);

RET_CODE copyRAMFileToDisk (
	char* pSource,			// input filename
	char* pTarget			// output filename
	);
RET_CODE secureDeleteFile(
	char* pTarget				// in: filename to secure delete
	);
//</prototypes>

#endif // end of #ifndef _GENFLOP
//============================================================= EOF HEADER ===
/*</header>
</source>
<!-- eof -->*/