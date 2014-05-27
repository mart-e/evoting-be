/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="genflop.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENFLOP.CPP  $</workFile>
<revision>$Revision: 1.7 $</revision>
<workFileTimeStamp>$Modtime:   Oct 28 2004 12:15:10  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENFLOP.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/07 12:38:51 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENFLOP.CPP</fileName>
  <creationDate>05/02/93</creationDate>

  <rights/>
  <author>B.S.S.</author>

  <description>
    <fr>Librairie des fonctions lecteur disquette avec gestion d'erreurs</fr>
    <nl>Functielibrary diskettelezer met fouten beheer</nl></description>
  <history>
    Revision History
    ################

   01/06/93: revision history starting date
   03/09/93: Floppy_Remove_File was added
   07/03/94: The routine Floppy_Request_Disk returns ABORTED for the
             URN and REC program.  At some places special treatment was
             necessary.
             --&gt; All normal routines using Floppy_Request_Disk must loop now.
   07/03/94: The floppy may NEVER be changed in ALL routines.</history>
 <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <conio.h> // getch() -> test-purpose only
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>
#include <alloc.h>
#include <errno.h>

#include "..\gen\gentype.h"
#include "..\gen\dos.h"
#include "..\gen\genflop.h"
#include "..\gen\gencryp.h"

#ifdef _APPLICATION_URN
   #include "..\urn\urnmes.h"
   #include "..\urn\urnerro.h"
#endif

#ifdef _APPLICATION_MAV
   #include "..\mav\mavmes.h"
#endif

#define _DEBUGMODE      FALSE      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---
//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
static void Floppy_Instructions( BOOL Append = FALSE);
static RET_CODE Reset_Errors( void);
static RET_CODE Floppy_Handle_Error( void);
static RET_CODE Floppy_Fseek( int File_Handle, OFFSET Offs);
static RET_CODE Floppy_Get_FCB( char * File_Name, struct fcb * Fcb);
static RET_CODE Floppy_Read_BootSector();

void Recover_Registration( void);
void Restore_Messages( void);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern char Floppy_MASTER[];
extern char Floppy_BACKUP[];

extern char * Floppy_Current;

extern INTEGRITY_STATE Flag_Integrity;
extern ERROR_HANDLING Error_Handling;
extern MODE Mode;

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>

//=========================== C O N S T A N T S ===============================

const int MAX_FILES = 10;

const int WRITE_PROTECTED = 0;
const int DRIVE_NOT_READY = 2;
const int DATA_ERROR_CRC = 4;

const int  RETRYS = 3;

//===================== G L O B A L   V A R I A B L E S =======================

static int Counter = RETRYS;

int DOS_Error_Code = NO_ERROR;         // set by Floppy_Error_Handler ()
static int Max_Clusters = 0;
unsigned short Cluster_Length = 0;
unsigned short Sectors_Per_Cluster;
unsigned short Clusters_Grouping;   // rounded number of clusters per track
struct BootSectorLayout *BootSector = NULL;
//</globals>

/*
     General informations:
     =====================

     All the floppy errors are intercepted with an built in error handler.

     The functions use handles to access the files.

     The URN program can run in an alternate mode called 'mode d‚grad‚',
     corrupted mode. For that mode the floppy is no longer accessed
     because he caused the program to go in corrupted mode.
     Normally there should'nt have any Floppy function call in corrupted mode,
     but to prevent a problem due to that, all the functions return
     immediately the correct RET_CODE (depending on the function)

     Once we know the disk we are working with, we check if the disk is still
     the same before any disk access, by first checking if the the disk was
     changed and then by getting the label.

     Warning: there is no garanty if the floppy was changed but the label is
              still the same to have the same files or the same disk at all.
              To get the maximum security, the disk should be entirely checked
              to see if there is no change on it.

     Most of the floppy functions are programmed as infinite loops, the loop
     implements the retrys on error and only ends if the function completely
     succeeds or if it aborts.
*/
/*
     Floppy access basics:
     =====================

     The maximum performance is reached when we read a whole track at one
     time. This is because a track corresponds to a complete disk revolution.
     If a track is too large for the real need, the next good performance
     length is the cluster length. When DOS reads a file he must read a whole
     cluster even if the length he had to read is smaller; this is because the
     disk is divided in clusters (and themselves in sectors) and the file is
     mapped by these clusters.
     Reading a whole track by reading it cluster by cluster will decrease the
     performance because after reading the first cluster the head must wait for
     a complete disk revolution to access the next cluster. Then reading a
     cluster in multiple passes will once again increase the time used to
     reposition the floppy head.
     Even if a file is smaller than the cluster length, the real used size
     on floppy will be the cluster length because DOS can't use the remaining
     space for a new file as they will share the same cluster and thus
     conflict.

     Thus for best performance it is a good practice for this program to
     create files with a multiple of cluster length and even of track length
     if it makes sense. But you should make sure to have following clusters
     when creating the files, no fragmentation. A fragmented file can
     dramaticaly decrease the performance, because what you think to be a
     track can in reality just be a cluster collection with no order.

     Absolute floppy access basics:
     ==============================
     We don't create the files with direct access functions, the files must be
     created with the standard functions. The basic of absolute disk access
     and the most difficult to obtain is the file cluster chain. This chain
     must be extracted from the FAT.
     The function written in ASM to access the FAT and the disk are completely
     compatible with our error handler, they generate an int24 error.

     The functions were tested with 1.44M and 720k floppies, all the variables
     are extracted from the floppy's FAT.

     See HelpTech for complete informations on FAT and disk structure.

*/

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Floppy_Error_Handler">
<heading>
  <description><en>
    establishes a hardware error handler for the current program.
    This error handler is invoked whenever an interrupt 0x24 occurs.
    Int24 is generated when an floppy access error or a device
    error occurs. The function sets the DOS_Error_Code value.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>
        response to the Dos message Ignore, Abort, Retry, Fail
        _HARDERR_ABORT     0  invoke int0x23 (ctrl-break)
        _HARDERR_IGNORE    1
        _HARDERR_RETRY     2
        _HARDERR_FAIL      3</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>errval</name>
      <description><en>
        error code set in the _DI register by DOS</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ax</name>
      <description><en>
        if &gt; 0, anded with 0x00FF gives the failing drive
        if &lt; 0, it is not a floppy error</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>bp</name>
      <description><en>
        bp and si together point to the device driver</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>si</name>
      <description><en>
        header of the failing drive (segment:address)</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>
        used to store the current DOS error code</en></description></parameter>
  </parameters>
  <usage>
    harderr( Floppy_Error_Handler);</usage>
  <checks></checks>
  <comment>
    If the error was not caused by a floppy, it returns IGNORE to
    DOS, otherwise it attempts some RETRY and if they failled
    returns IGNORE to DOS.</comment>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
int Floppy_Error_Handler ( int errval, int ax, int bp, int si)
// ---------------------------------------------------------------------------
{
   if( ax < 0)                                   // this is not a floppy error
      return ( _HARDERR_IGNORE);             // pass '(I)gnore' message to DOS

   DOS_Error_Code = _DI & 0x00FF;                     // get floppy error code

   if( -- Counter)
   {
      DOS_Error_Code = NO_ERROR;                     // in case retry succeeds
      return( _HARDERR_RETRY);                       // pass '(R)etry' message
   }

   Counter = RETRYS;                                  // reset static variable
   return( _HARDERR_IGNORE);                 // pass '(I)gnore' message to DOS
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
     For the MAV there is no floppy change allowed. The code below makes
     no sense for the MAV, so it is not compiled.
*/

/*<function id="Floppy_Message">
<heading>
  <description><en>
    displays the message corresponding to the floppy that is
    currently used</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index_M</name>
      <description><en>
        the message number if the floppy is MASTER</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index_B</name>
      <description><en>
        the message number if the floppy is BACKUP</en></description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Confirm</name>
      <description><en>
        see the Display_Message function for a
        full explanation of these parameters
        their default value is FALSE</en></description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Append</name></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Exception</name></parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype><name>Floppy_MASTER[]</name>
      <description><en>the floppy MASTER label</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype><name>Floppy_BACKUP[]</name>
      <description><en>the floppy BACKUP label</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description></parameter>
  </parameters>
  <usage>
    Floppy_Message( M_MASTER_IN1, M_BACKUP_IN1, TRUE, Append, TRUE);</usage>
  <checks></checks>
  <comment>
    this function only calls Display_Message with the message
    corresponding to the current floppy</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Floppy_Message ( int M_Index_M, int  M_Index_B, BOOL Confirm, BOOL Append,
                                                    BOOL Exception)
// ---------------------------------------------------------------------------
{
   if( Floppy_Current == Floppy_BACKUP)
      Display_Message( M_Index_B, Confirm, Append, Exception);
   else
      Display_Message( M_Index_M, Confirm, Append, Exception);
}
//</code></function>

/*<function id="Floppy_Instructions">
<heading>
  <description><en>
    displays a message asking the user to insert the floppy</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Append</name>
      <description><en>see Display_Message for a full explanation</en></description>
      <defaultValue>FALSE</defaultValue></parameter>
  </parameters>
  <usage>
    Floppy_Instructions( FALSE);</usage>
  <checks></checks>
  <comment>
    this function is used in case of error when accessing the floppy,
    it let the user insert the floppy, waits on a RETURN and then
    redisplays the screen as it was</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Floppy_Instructions ( BOOL Append)
// ---------------------------------------------------------------------------
{
    Floppy_Message( M_MASTER_IN1, M_BACKUP_IN1, TRUE, Append, TRUE);
    Restore_Messages();
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reset_Errors">
<heading>
  <description><en>
    reset the DOS_Error_Code and the retrys counter</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>local function : always SUCCEEDED</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
    <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Counter</name>
      <description><en>number of retrys on floppy error</en></description></parameter>
  </parameters>
  <usage>
    return Reset_Errors();</usage>
  <checks></checks>
  <comment>
    This function must be called for each Floppy function that
    returns SUCCEEDED to be sure that the DOS_Error_Code and
    Counter values are correctly set</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Reset_Errors ( void)
// ---------------------------------------------------------------------------
{
   DOS_Error_Code = NO_ERROR;
   Counter        = RETRYS;

   return SUCCEEDED;
}
//</code></function>

/*<function id="Floppy_Handle_Error">
<heading>
  <description><en>
    for the MAV the function locks the system with an error message
    but the error processing is more efficient for the URN and may
    be corrected, depending on the error type and on when it happens
  </en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        local function
        RETRY      calling function can retry,
        ABORTED    calling function must ABORT.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>is the program CORRUPTED ?</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>what to do with an hard error</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">MODE</datatype><name>Mode</name>
      <description><en>the program phase (STARTUP..)</en></description></parameter>
  </parameters>
  <usage>
    if( Floppy_Handle_Error) == ABORTED)
       return ABORTED;
  </usage>
  <checks>
    If it was called while in corrupted mode or if there was no
    error, it returns ABORTED because it should not have been called
    This can be a programmation error if it happens.
  </checks>
  <comment>
    For the MAV there are no errors admitted, the user is not allowed
    to remove the floppy, so if it happens it is fatal even if it
    can be resolved.
    For the URN the processing is more elaborated:
    - in startup mode, until we are in the program's main loop there
    is no floppy removal allowed, so it is FATAL
    - otherwise, if the floppy was removed, or if the floppy is
    write protected the user is asked to insert the floppy or to
    remove the write protection.
    - a CRC error is only processed for comparison purpose, otherwise
    the processing is left to the default error processing
    - the default error processing depends on the Error_Handling
    value, if it is ROLLBACK it trys to restart the whole process
    from a rollback point (if any) otherwise it returns ABORTED
  </comment>
</heading>
<code>*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// ---------------------------------------------------------------------------
static RET_CODE Floppy_Handle_Error ( void)
// ---------------------------------------------------------------------------
{
   RET_CODE Result = ABORTED;                   // the function's return value

   if( DOS_Error_Code == NO_ERROR || Flag_Integrity == CORRUPTED)
      return ABORTED;

   switch( DOS_Error_Code)
   {

   case WRITE_PROTECTED  :

      Result = RETRY;
      Display_Message( M_WRITE_PROT, 0, 0, 1);
      Wait_Until_Floppy_Removed();
      Floppy_Instructions( FALSE);
      break;

   case DRIVE_NOT_READY  :

      if( Mode == STARTUP)        // no floppy removal allowed in startup mode
      {
         Display_Message( M_NOT_INSERTED);
         Display_Message( M_RESTART, 0, 1);
         for( ;;);
      }
      else
      {
         Result = RETRY;
         Display_Message( M_NOT_INSERTED, 0, 0, 1);
         Floppy_Instructions( TRUE);
         break;
      }

   case DATA_ERROR_CRC :             // this is only used when comparing files

      if( Error_Handling == CRC_DETECT)
      {
          Result = ABORTED;
          break;
      };

   default :

       switch( Error_Handling)
       {
       case ABORT    : Result = ABORTED;  break;
       case IGNORE   : Result = ABORTED;  break;

       case ROLLBACK :

          Display_Message( M_UNRECOVERABLE_ERROR, 1);
          DOS_Error_Code = NO_ERROR;                    // for next invocation

          Recover_Registration();

       default       : Result = ABORTED;
       }
   }

   Reset_Errors();
   return Result;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _APPLICATION_URN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_MAV
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static RET_CODE Floppy_Handle_Error( void)
{
   if( DOS_Error_Code == NO_ERROR || Flag_Integrity == CORRUPTED )
      return ABORTED;

   switch( DOS_Error_Code)
   {
   case DRIVE_NOT_READY : Error_Exit( M_NOT_INSERTED);
   default              : Error_Exit( M_ERROR_ON_DISK);
   }

   // the code below makes no sense, but it is safe to conserve it just in
   // case one of the Error_Exit becomes on non fatal function

   DOS_Error_Code = NO_ERROR;                     // reset for next invocation
   return ABORTED;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _APPLICATION_MAV
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//</code></function>

/*<function id="Floppy_Get_Label">
<heading>
  <description><en>
    reads the label from the a: floppy disk
  </en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Label</name>
      <description><en>
        the read label string</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description>
    </parameter>
  </parameters>
  <usage>if( Floppy_Get_Label( Label) == ABORTED)</usage>
  <checks>every disk access is checked</checks>
  <comment>
    The _dos_findfirst function can return 0 even if there was an
    error, so we must check the DOS_Error_Code to be sure.
    A label name is the same thing as a file name, only the
    attributes are different, so if the label is longer than 8 the
    name will appear with a . separator; the maximum length is 11
    for the name + 1 for the . and 1 for the null terminated string
    MASTER_DISK will give MASTER_D.ISK</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Get_Label ( char * Label)
// ---------------------------------------------------------------------------
{
   struct find_t Control_Block;
   int Status;

   for(;;)
   {
      DOS_Error_Code = NO_ERROR;
      Status = _dos_findfirst( "A:*.*", _A_VOLID, & Control_Block);

      if ( Status == 0)
         if( DOS_Error_Code == NO_ERROR)
         {
            strncpy( Label, Control_Block.name, 13);
            return Reset_Errors();
         }

      // 20041007 avn - _dos_findfirst does not return a blank label as expected when
      // the floppy has no label, it just returns an error state
      if ( 18 == Status && -1 == DOS_Error_Code)
      {
         // floppy has no label in this case
         Label[0] = NULL;
         return Reset_Errors();
      }

      if( Floppy_Handle_Error() == ABORTED)                  // RETRY or ABORT
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Read_BootSector">
<heading>
  <description><en>
    Updates the BootSector.  If you just want to use the
    information presented in the BootSector, checks first if the
    BootSector not already is loaded.  You can do this by following
    check: if( !BootSector) Floppy_Read_BootSector();</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>struct BootSectorLayout*</datatype><name>BootSector</name>
      <description><en>bootsector information</en></description>
    </parameter>
  </parameters>
  <usage>if (!BootSector) Floppy_Read_BootSector();</usage>
  <checks>memory allocation</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Floppy_Read_BootSector ()
// ---------------------------------------------------------------------------
{
   struct fatinfo FatInfo;

   getfat( 1, &FatInfo);

   if( ! BootSector)
       BootSector = (struct BootSectorLayout *)malloc( FatInfo.fi_bysec);

   if( ! BootSector)
       Error_Exit( E_OUT_OF_MEMORY);

   for(;;)
   {
      if( ! OwnAbsRead( 0, 1, 0, BootSector))
      {
         Reset_Errors();
         break;
      }

      if( Floppy_Handle_Error() == ABORTED)
      {
         free( BootSector);
         BootSector = NULL;
         return( ABORTED);
      }
   }
   return( SUCCEEDED);
}
//</code></function>

/*<function id="Floppy_Request_Disk">
<heading>
  <description><en>
    check if the wanted disk is present, otherwise request this disk
    three times maximum and then lock the system</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  the disk is the requested one,
        ABORTED    function has failed.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="extern">char *</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>is the program CORRUPTED?</en></description></parameter>
  </parameters>
  <usage>
    if( Floppy_Request_Disk() == ABORTED)</usage>
  <comment>
    this function is used for two purposes, first to check if the
    floppy was not removed and then check if the floppy is still
    the same, second to request a floppy change</comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Request_Disk ( void)
// ---------------------------------------------------------------------------
{
   if( Flag_Integrity == CORRUPTED)      // simulates floppy in corrupted mode
      return SUCCEEDED;

   static char Previous_Floppy[15] = "";       // to detect a floppy switching
   static long Previous_VolumeSerialNr = 0;

   if( !strcmp(Previous_Floppy,Floppy_Current))          // no switching asked
   {
      if( ! IsFloppyRemoved() )
      {
         return SUCCEEDED;         // floppy was not removed so it is the same
      }
      else
      {
      }
   }

   for( int Tries = 3; Tries > 0; --Tries)         // ask 3 times for the disk
   {
      char Floppy_Name [15] = "A:";

      if( Floppy_Get_Label( Floppy_Name+2) == ABORTED)   // get the disk label
         {
#ifdef _APPLICATION_URN
         if( Tries != 3) Restore_Messages();
#endif
         return ABORTED;
         }

      if( Floppy_Read_BootSector() == ABORTED)
         {
#ifdef _APPLICATION_URN
         if( Tries != 3) Restore_Messages();
#endif
         return ABORTED;
         }

     if( !strcmp( Floppy_Name, Floppy_Current) &&
          (strcmp( Floppy_Current,Previous_Floppy) ||
           BootSector->VolumeSerialNr == Previous_VolumeSerialNr))
            {
            // Asked floppy = GOOD LABEL and
            //                if =PREVIOUS_FLOPPY then SAME SERIAL Nr
            //
            strcpy(Previous_Floppy,Floppy_Current);
            Previous_VolumeSerialNr = BootSector->VolumeSerialNr;
#ifdef _APPLICATION_URN
            if( Tries != 3) Restore_Messages();
#endif
            return SUCCEEDED;
            }

#ifdef _APPLICATION_URN
      else
            {
            Display_Message( M_INVALID_DISK, 0, 0, 1);
            Floppy_Message( M_MASTER_IN1, M_BACKUP_IN1, 1, 1, 1);
            if( Tries != 3) Restore_Messages();
            }
#endif
   }
                                       // we tried 3 times, now we give up ...
#ifdef _APPLICATION_MAV
   Error_Exit( M_INVALID_DISK);
#endif

// 20040430 avn - uncommented
#ifdef _APPLICATION_URN
   Error_Exit( M_RESTART_URN);
#endif

//rvh
   Previous_Floppy[0] = ' ';
   return ABORTED;
}
//</code></function>

/*<function id="IsFloppyRemoved">
<heading>
  <description><en>
    this function detects is a floppy was removed by testing the
    floppy change line status</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  floppy was changed,
        FALSE floppy was not changed.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    if( ! IsFloppyRemoved())</usage>
  <comment>
    perform a call to int13 function 16h to test the change status
    on entry  AH = 16h, DL = drive number
    on return AH = 00 no disk change
                   01 disk changed
              CF = set if disk has been removed or an error occured
    the floppy drive must support this call, it can also be tested
    with the int13 function 15 (found in Helppc)
    The floppy changed is not reset with the floppy insertion, it can
    only be reset with a floppy access.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL IsFloppyRemoved ( void)
// ---------------------------------------------------------------------------
{
   union REGS regs;

   regs.h.ah = 0x16;
   regs.h.dl = 0;
   int86( 0x13, &regs, &regs);
   return regs.x.cflag;
}
//</code></function>

/*<function id="Wait_Until_Floppy_Removed">
<heading>
  <description><en>
    waits until the floppy is removed</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>is the program CORRUPTED?</en></description></parameter>
  </parameters>
  <usage>Wait_Until_Floppy_Removed();</usage>
  <comment>
    because the floppy changed line does not change between two disk
    access, it is not sufficient to know that the floppy was
    removed. If the floppy was changed we must first force a floppy
    access to reset the change line status, if the floppy is really
    absent we can return otherwise we must perform the normal wait
    on IsFloppyRemoved().
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Wait_Until_Floppy_Removed ( void)
// ---------------------------------------------------------------------------
{
   if( Flag_Integrity == CORRUPTED)      // simulates floppy in corrupted mode
      return;

   if( IsFloppyRemoved() )                               // force a disk access
   {                                             // just to get the error code
      struct find_t Control_Block;

      DOS_Error_Code = NO_ERROR;
      _dos_findfirst( "a:*.*", _A_VOLID, & Control_Block);

      if( DOS_Error_Code == DRIVE_NOT_READY)
      {
         DOS_Error_Code = NO_ERROR;           // the floppy was really changed
         return;
      }
   }

   while( ! IsFloppyRemoved());
   DOS_Error_Code = NO_ERROR;
}
//</code></function>

/*<function id="Floppy_Inserted">
<heading>
  <description><en>
    this function should detect if there is a floppy inserted but
    we did'nt find the method to make it properly, without any
    floppy head noise. We hope to find the solution someday ...</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  = Floppy was inserted or is in the floppy reader,
        FALSE = waiting a floppy disk.</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>while( ! Floppy_Inserted());</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL Floppy_Inserted ( void)
// ---------------------------------------------------------------------------
{
   return TRUE;
}
//</code></function>
/*<function id="Floppy_Open_File">
<heading>
  <description><en>
    try to open a file in read/write binary mode</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded but this will
                   not mean file is open, you must test
                   the handle to be sure
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>
        the file to open (path admitted)</en></description></parameter>
    <parameter type="arg">
      <datatype>int*</datatype><name>File_Handle</name>
      <description><en>
        the file's handle once opened
        no default (-1 if file not found)</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>the current DOS error code</en></description></parameter>
  </parameters>
  <usage>
    if( Floppy_Open_File( "a:essai", &amp; File_Handle) == ABORTED ||
      File_Handle == -1)</usage>
  <checks>
    Floppy_Request_Disk()</checks>
  <comment>
    if the file does not exist, it is not created but the handle's
    value is -1, it is the calling routine's responsability to
    test the file handle value
  </comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Open_File ( char * File_Name, int * File_Handle)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( ( * File_Handle = open( File_Name, O_RDWR | O_BINARY)) != -1)
         return Reset_Errors();                  // file exist and is now open

      if( DOS_Error_Code == NO_ERROR)             // only means file not found
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_File_Length">
<heading>
  <description><en>
    get the length of a file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>
        the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>long*</datatype><name>Length</name>
      <description><en>
        the variable address where to put the length</en></description></parameter>
  </parameters>
  <usage>
    if( Floppy_File_Length( File_Handle, &amp; Length) == ABORTED)</usage>
  <checks>
    Floppy_Request_Disk()</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_File_Length ( int File_Handle, long * Length)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( ( * Length = filelength( File_Handle)) != -1)
        return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>
/*<function id="Floppy_Read_Buffer">
<heading>
  <description><en>
    reads a buffer from a file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>unsigned int*</datatype><name>Length</name>
      <description><en>the length to read and really read</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>the buffer where to put what is read</en></description></parameter>
  </parameters>
  <usage>
    unsigned int Length = 512;
    char Buffer[ Length];
    if( Floppy_Read_Buffer( File_Handle, &amp; Length, Buffer) == ABORTED)
  </usage>
  <checks>
    Floppy_Request_Disk();
    The read length must correspond to the length that must be read.
  </checks>
  <comment>
    the read function has strange reactions when a floppy error
    occurs, it can respond with a correct number of characters
    read even if nothing was read at all. So to recover the most
    floppy errors we must save the length to read and the current
    position in the file so they can be reset to their original
    values in case of error.
    Sometimes when the drive is not ready, we get a error 8 (for
    sector not found) from which we can't recover. We haven't
    discover why at this time. This is the mean reason why there
    is no floppy removal admitted during startup, as we can't
    garantee a 100% error free retry.
    The maximum length read can read in one time is 65534 (unsigned)
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Read_Buffer ( int File_Handle, unsigned int * Length,
                             void * Buffer)
// ---------------------------------------------------------------------------
{
   unsigned Bytes_To_Read = * Length;

   long Offset = tell( File_Handle);           // get current position in file

   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      * Length = Bytes_To_Read;           // length read must = length to read

      if( ( * Length = read( File_Handle, Buffer, * Length)) == Bytes_To_Read
          &&  DOS_Error_Code == NO_ERROR)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;

      if( Floppy_Fseek( File_Handle, Offset) == ABORTED)  // set file position
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Close_File">
<heading>
  <description><en>
    closes an open file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle to close</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Close_File( File_Handle) == ABORTED)</usage>
  <checks>Floppy_Request_Disk()</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Close_File ( int File_Handle)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( _close( File_Handle) == 0)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>
/*<function id="Floppy_Close_All">
<heading>
  <description><en>
    closes ALL the open files</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>if( Floppy_Close_All() == ABORTED)</usage>
  <checks>Floppy_Request_Disk()</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Close_All ()
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( fcloseall() != EOF)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Fseek">
<heading>
  <description><en>
    set the current position in the file</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        local function
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>OFFSET</datatype><name>Offset</name>
      <description><en>the new current position in the file from beginning of the file</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Fseek( File_Handle, 0L) == ABORTED) //rewind the file</usage>
  <checks>Floppy_Request_Disk()</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Floppy_Fseek ( int File_Handle, OFFSET Offs)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( lseek( File_Handle, Offs, SEEK_SET) != -1L)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Remove_File">
<heading>
  <description><en>
    removes a file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>the file name to remove</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Remove_File( "a:b001") == ABORTED)</usage>
  <checks>Floppy_Request_Disk</checks>
  <comment>
    This function is only used with the RECOUNT program to remove
    the B001 files from floppy, we need to remove them from both
    MASTER and BACKUP floppy for two reasons. First, the rename is
    not able to rename a file if the dest file already exist.
    Second, we must erase the B001 files on the two floppies
    before we start recounting to avoid the possibility to reuse
    the B001 from BACKUP afterwards.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Remove_File ( char * File_Name)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( remove( File_Name) == 0)
        return Reset_Errors();         // ok, file removed successfully

      if( errno == ENOENT)             // ok, file does not exist
        return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="RAMdrive_Open_File" confidential="yes">
<heading>
  <description><en>
    Open a file on the RAMdrive</en></description>
  <parameters>
    <parameter type="return">
      <datatype>FILE*</datatype>
      <description><en>
        FILE* , if a file could be created,
        NULL  , if it is not possible to create a new file.</en></description></parameter>

    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name></parameter>
  </parameters>
  <usage>if( (RAM_File = RAMdrive_Open_File( File_Name)) == NULL)</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
FILE * RAMdrive_Open_File ( char * File_Name)
// ---------------------------------------------------------------------------
{
   return fopen( File_Name, "rt");
}
//</code></function>

/*<function id="RAMdrive_Create_File" confidential="yes">
<heading>
  <description><en>
    creates a file in text mode for writing and reading, if the
    file already exist it is overwritten</en></description>
  <parameters>
    <parameter type="return">
      <datatype>FILE*</datatype>
      <description><en>
        FILE* , if possible to create file,
        NULL  , if impossible to create file.</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>the file to open (path admitted)</en></description></parameter>
  </parameters>
  <usage>if( RAMdrive_Create_File( "a:\b003") == NULL)</usage>
  <comment>
    in opposition to RAMdrive_Open_File, the function returns NULL
    if the file can't be created.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
FILE * RAMdrive_Create_File ( char * File_Name)
// ---------------------------------------------------------------------------
{
   return fopen( File_Name, "wt");
}
//</code></function>

/*<function id="RAMdrive_Read_Buffer" confidential="yes">
<heading>
  <description><en>
    Read until end-of-line detected in Text-file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  if possible to read from file,
        ABORTED    if inpossible to read from file.</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE*</datatype><name>File_Handle</name></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Buffer</name></parameter>
  </parameters>
  <usage>if( RAMdrive_Read_Buffer( RAM_File, Buffer) == SUCCEEDED)</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE RAMdrive_Read_Buffer ( FILE * File_Handle, int Length,
                               char * Buffer)
// ---------------------------------------------------------------------------
{
   fgets( Buffer, Length, File_Handle);
   if( strlen( Buffer) > 0)
      return SUCCEEDED;
   else
      return ABORTED;
}
//</code></function>

/*<function id="RAMdrive_Write_Buffer" confidential="yes">
<heading>
  <description><en>
    writes a buffer to file with standard floppy access</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE*</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Length</name>
      <description><en>the length to write (number of characters)</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Buffer</name>
      <description><en>a pointer to the buffer to write</en></description></parameter>
  </parameters>
  <usage>if( RAMdrive_Write_Buffer( File_Handle, 2, "aa") == ABORTED)</usage>
  <checks>The written length must correspond to the length that must be written</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE RAMdrive_Write_Buffer ( FILE * File_Handle, char * Buffer)
// ---------------------------------------------------------------------------
{
   strcat( Buffer, "\n");
   if( fputs( Buffer, File_Handle) != EOF)
      return SUCCEEDED;
   else
      return ABORTED;
}
//</code></function>

/*<function id="RAMdrive_Close_File" confidential="yes">
<heading>
  <description><en>
    Close a file on the RAMdrive</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  if possible to close file,
        ABORTED    if inpossible to close file</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE*</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
  </parameters>
  <usage>if( RAMdrive_Close_File( RAM_File) == SUCCEEDED)</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE RAMdrive_Close_File ( FILE * File_Handle)
// ---------------------------------------------------------------------------
{
   if( fclose( File_Handle) == 0)
      return SUCCEEDED;
   else
      return ABORTED;
}
//</code></function>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Floppy_Create_File">
<heading>
  <description><en>
    creates a file in binary mode for writing and reading, if the
    file already exist it is overwritten</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>he file to open (path admitted)</en></description></parameter>
    <parameter type="arg">
      <datatype>int*</datatype><name>File_Handle</name>
      <description><en>the file's handle once opened</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>is the program CORRUPTED?</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Create_File( "a:\b003", &amp; File_Handle) == ABORTED)</usage>
  <checks>
    Floppy_Request_Disk()
    Flag_Integrity</checks>
  <comment>
    in opposition to Floppy_Open_File, the function returns an error
    if the file can't be created.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Create_File ( char * File_Name, int * File_Handle)
// ---------------------------------------------------------------------------
{
   if( Flag_Integrity == CORRUPTED)
      return SUCCEEDED;

   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( ( * File_Handle = open( File_Name, O_RDWR | O_CREAT | O_BINARY,
                                  S_IREAD | S_IWRITE)) != -1)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Write_Buffer">
<heading>
  <description><en>
    writes a buffer to file with standard floppy access</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>unsigned int</datatype><name>Length</name>
      <description><en>the length to write (number of characters)</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>a pointer to the buffer to write</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Write_Buffer( File_Handle, 2, "aa") == ABORTED)</usage>
  <checks>
    Floppy_Request_Disk()
    The written length must correspond to the length that must be
    written
  </checks>
  <comment>
    The maximum length _write can write is 65534 (unsigned int)
    To recover the most floppy errors we must save the current
    position in the file so it can be reset to its original value
    in case of error.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Write_Buffer ( int File_Handle, unsigned int Length,
                              void * Buffer)
// ---------------------------------------------------------------------------
{
   long Offset = tell( File_Handle);           // get current position in file

   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( _write( File_Handle, Buffer, Length) == Length &&
          DOS_Error_Code == NO_ERROR)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;

      if( Floppy_Fseek( File_Handle, Offset) == ABORTED) // reposition in file
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Turn_Long">
<heading>
  <description><en>
    modifies the duration the floppy motor will turn after the
    last access was made</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Floppy_Turn_Long();</usage>
  <comment>
    we set the time to the maximum value it can admit, the motor
    will run for 10 seconds after the last access.
    The address where this time is defined was found in HelpTech.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Floppy_Turn_Long ( void)
// ---------------------------------------------------------------------------
{
   BYTE ** DiskParameters = (BYTE **) MK_FP( 0, 0x78);

   (* DiskParameters) [2] = 0xFF;      // Floppy will turn as long as possible
}
//</code></function>

/*<function id="Floppy_Flush_Files">
<heading>
  <description><en>
    forces all the buffers to be written to disk</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>DOS_Error_Code</name>
      <description><en>used to store the current DOS error code</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Flush_Files() == ABORTED)</usage>
  <checks>Floppy_Request_Disk</checks>
  <comment>
    the function calls the function 0x0D from int 21, this causes
    a disk reset, the consequence is that all the file buffers are
    written to disk (found in HelpPc)
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Flush_Files ( void)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      _AH = 0x0D;
      geninterrupt( 0x21);               // flush the stream's internal buffer

      if( DOS_Error_Code == NO_ERROR)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Copy_File">
<heading>
  <description><en>
    very fast file copy</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Source_File</name>
      <description><en>the source file name</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Dest_File</name>
      <description><en>the destination file name</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>rounded number of clusters per track</en></description></parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>an already allocated buffer</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Copy_File( "a:\\b001", 'c');</usage>
  <checks>
    memory allocation, there must be at least one already allocated buffer
  </checks>
  <comment>
    The source file name may not contain a directory specification
    because a lower level function does'nt support it, the source
    file name can be specified as "a:b001" but not as "a:\\b001"
    There is no such limitation for the destination file.
    This function is intended to be used after Alloc_Memory_BAC(),
    so their is at least already one buffer allocated with a track
    length. We overwrite the existing buffer.
    The source file is accessed with the direct floppy routines and
    the destination file is written with standard floppy routines.
    The function reads the file with a maximum buffer of a track and
    a minimum buffer of cluster length, but writes only back
    what really belongs to the file to have the correct file size.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Copy_File ( char * Source_File, char * Dest_File)
// ---------------------------------------------------------------------------
{
   extern char ** Clusters_BAC;                     // coming from URNRECO.CPP

   int Source_Handle, Dest_Handle;
   FILE_CHAIN Source_Chain = NULL;

   long Buffer_Size = (long)Cluster_Length * (long)Clusters_Grouping;
   long File_Length;
   unsigned int To_Write;                 // the number of bytes to write back
   int i;

   if( Clusters_BAC [0] == NULL) // there must already be one buffer allocated
      return ABORTED;

   if( Floppy_Open_File( Source_File, & Source_Handle) == ABORTED ||
       Source_Handle == -1)
      return ABORTED;

   if( Floppy_Create_File( Dest_File, & Dest_Handle) == ABORTED)
      return ABORTED;

   if( Floppy_File_Length( Source_Handle, & File_Length) == ABORTED)
      return ABORTED;
                                // the number of clusters constituing the file
   int Tot_Clusters = (File_Length + Cluster_Length - 1) / Cluster_Length;

   int Current_Cluster = 0; // the current cluster num. relatively to the file

   if( Floppy_Get_File_Chain( Source_File, & Source_Chain) == ABORTED)
      return ABORTED;

   do
   {
      if( File_Length < Buffer_Size)
         To_Write = (unsigned int) File_Length;
      else
         To_Write = (unsigned int) Buffer_Size;

                // number of clusters correponding to the length to read/write
      int Clusters_To_Read = (To_Write + Cluster_Length - 1) / Cluster_Length;

                                    // optimized read of the contiguous tracks
      if( Floppy_Read_Clusters( Source_Chain, Current_Cluster, Clusters_To_Read,
                                Clusters_BAC [0]) == ABORTED)
      {
         free( Source_Chain);
         Source_Chain = NULL;
         return ABORTED;
      }

      Current_Cluster += Clusters_To_Read;

                                           // writes only the needed size back
      if( Floppy_Write_Buffer( Dest_Handle, To_Write, Clusters_BAC [0])
          == ABORTED)
      {
         free( Source_Chain);
         Source_Chain = NULL;
         return ABORTED;
      }

      File_Length -= To_Write;           // the length that must still be read

   } while( Current_Cluster < Tot_Clusters);

   free( Source_Chain);
   Source_Chain = NULL;

   if( Floppy_Close_File( Source_Handle) == ABORTED ||
       Floppy_Close_File( Dest_Handle) == ABORTED)

      return ABORTED;

   return SUCCEEDED;
}
//</code></function>

/*<function id="Floppy_Compare_Files">
<heading>
  <description><en>
    compares two files and return a comprehensive comparison result</en></description>
  <parameters>
    <parameter type="return">
      <datatype>FILE_COMP</datatype>
      <description><en>
        NOT_SAME    = files are not same
        SAME        = files are identical
        COMP_ABORT  = error during the comparison
        CRC_ERROR_A = error while reading A
        CRC_ERROR_B = error while reading B</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle_A</name>
      <description><en>the first file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle_B</name>
      <description><en>the second file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>long</datatype><name>File_Length</name>
      <description><en>the theorical file length</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char**</datatype><name>Clusters_BAC</name>
      <description><en>an already allocated buffer</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">int</datatype><name>Nb_Tracks</name>
      <description><en>the number of allocated buffers</en></description></parameter>
  </parameters>
  <usage>switch( Floppy_Compare_File( File_A, File_B, File_Length))</usage>
  <checks>memory allocation</checks>
  <comment>
    This function is intended to be used after Alloc_Memory_BAC(),
    so their is at least already one buffer allocated with a track
    length. If their is only one allocated buffer, we allocate a
    second one, otherwise we use the existing ones to prevent a
    memory overflow.
    The files must be identical but also have the correct length,
    this is because we know the length the file must have.
    During file comparison we allow the CRC error handling, a CRC
    error says: the file can't be read, but this will not mean that
    the disk is not good. Recreating the file may resolve that error
    but it can also be a real floppy problem.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
FILE_COMP Floppy_Compare_Files ( int File_Handle_A, int File_Handle_B, long File_Length)
// ---------------------------------------------------------------------------
{
   extern char ** Clusters_BAC;
   extern int Nb_Tracks;

   unsigned char * Buffer_A = NULL, * Buffer_B = NULL;
   long Length_A, Length_B;
                                // the optimal buffer size is the track length
   long Buffer_Size = (long)Cluster_Length * Clusters_Grouping;
   unsigned To_Read;
   FILE_COMP Ret = SAME;

   if( Clusters_BAC [0] == NULL)        // there must already exist one buffer
      return COMP_ABORT;

   if( Floppy_File_Length( File_Handle_A, & Length_A) == ABORTED ||
       Floppy_Fseek( File_Handle_A, 0) == ABORTED ||
       Length_A != File_Length)

      return CRC_ERROR_A;

   if( Floppy_File_Length( File_Handle_B, & Length_B) == ABORTED ||
       Floppy_Fseek( File_Handle_B, 0) == ABORTED ||
       Length_B != File_Length)

      return CRC_ERROR_B;

   Buffer_A = Clusters_BAC [0];

   if( Nb_Tracks > 1)
      Buffer_B = Clusters_BAC [Clusters_Grouping];
   else
      if( (Buffer_B = (unsigned char *) malloc( Buffer_Size)) == NULL)
         Error_Exit( E_OUT_OF_MEMORY);

   Error_Handling = CRC_DETECT;             // allows the CRC error processing
   do                                       // by the error handling function
   {
      if( Length_A < Buffer_Size)
         To_Read = (int) Length_A;
      else
         To_Read = Buffer_Size;

      if( Floppy_Read_Buffer( File_Handle_A, & To_Read, Buffer_A) == ABORTED)
      {
         Ret = CRC_ERROR_A;
         break;
      }

      if( Floppy_Read_Buffer( File_Handle_B, & To_Read, Buffer_B) == ABORTED)
      {
         Ret = CRC_ERROR_B;
         break;
      }

      if( memcmp( Buffer_A, Buffer_B, To_Read))
      {
         Ret = NOT_SAME;
         break;
      }

      Length_A -= Buffer_Size;

   } while( Length_A > 0);

   if( Nb_Tracks < 2)
   {
      free( Buffer_B);
      Buffer_B = NULL;
   }

   Error_Handling = ROLLBACK;
   return Ret;
}
//</code></function>

/*<function id="Floppy_Read_At">
<heading>
  <description><en>
    reads a whole cluster with standard disk access</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>OFFSET</datatype><name>Offs</name>
      <description><en>
        the cluster number to read, related to the
        file, it is not an absolute cluster number
        0 = first cluster of the file</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>a pointer to the buffer where to put the cluster's contents</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Read_At( File_Handle, 0, Buffer) == ABORTED)</usage>
  <comment>
    For a proper use, the file length must be a multiple of the
    Cluster_Length. It is not an error if the file is shorter but
    the buffer's content must then be limited afterwards by the
    calling function.
    The function first position the file's current read position
    and the makes a call to Floppy_Read_Buffer. The Length variable
    is needed because Floppy_Read_Buffer may modify it and we don't
    want Cluster_Length to be changed.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Read_At ( int File_Handle, OFFSET Offs, void * Buffer)
// ---------------------------------------------------------------------------
{
   unsigned int Length = Cluster_Length;

   if( Floppy_Fseek( File_Handle, Offs * (long)Length) == ABORTED)
      return ABORTED;

   return Floppy_Read_Buffer( File_Handle, & Length, Buffer);
}
//</code></function>

/*<function id="Floppy_Read_Track">
<heading>
  <description><en>
    reads a complete track with standard floppy access routines</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>OFFSET</datatype><name>Offs</name>
      <description><en>
        the cluster number to read, related to the
        file, it is not an absolute cluster number
        0 = first cluster of the file</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>a pointer to the buffer where to put the cluster's contents</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>rounded number of clusters per track</en></description></parameter>
  </parameters>
  <usage>
    if( Floppy_Read_Track( File_Handle, 0, Buffer) == ABORTED)
      (this reads the first track used by the file)
  </usage>
  <comment>
    For a proper use, the file length must be a multiple of the
    track length. It is not an error if the file is shorter but
    the buffer's content must then be limited afterwards by the
    calling function.
    The function first position the file's current read position
    and the makes a call to Floppy_Read_Buffer. The Length variable
    is needed because Floppy_Read_Buffer may modify it and we don't
    want Cluster_Length to be changed.
    The file may not begin at a track start point, but this is not
    a real problem because DOS controls the reading, it can only
    decrease a little bit the performance.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Read_Track ( int File_Handle, OFFSET Offs, void * Buffer)
// ---------------------------------------------------------------------------
{
   unsigned int Length = Cluster_Length * Clusters_Grouping;   // track length

   if( Floppy_Fseek( File_Handle, Offs * (long)Length ) == ABORTED)
      return ABORTED;

   return Floppy_Read_Buffer( File_Handle, & Length, Buffer);
}

//</code></function>

/*<function id="Floppy_Write_At">
<heading>
  <description><en>
    write a whole cluster with standard floppy access routines</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>OFFSET</datatype><name>Offs</name>
      <description><en>
        the cluster number to read, related to the
        file, it is not an absolute cluster number</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>the cluster contents to write</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Write_At( File_Handle, 0, Cluster_Buffer) == ABORTED)</usage>
  <comment>
    the function first positions the current position in the file
    at the cluster we want to write. The cluster number is computed
    relatively to the file not to the disk.
    The function should not be used to create a file because when
    creating a file the position in the file is always at the end
    so there is no need to perform a seek, this can avoid some
    errors when trying to position outside the file.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Write_At ( int File_Handle, OFFSET Offs, void * Buffer)
// ---------------------------------------------------------------------------
{
   if( Floppy_Fseek( File_Handle, Offs * Cluster_Length) == ABORTED)
      return ABORTED;

   return Floppy_Write_Buffer( File_Handle, Cluster_Length, Buffer);
}
//</code></function>

/*<function id="Floppy_Write_Track">
<heading>
  <description><en>
    writes a complete track with standard floppy access routines</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>File_Handle</name>
      <description><en>the file handle</en></description></parameter>
    <parameter type="arg">
      <datatype>OFFSET</datatype><name>Offs</name>
      <description><en>
        the cluster number to read, related to the
        file, it is not an absolute cluster number</en></description></parameter>
    <parameter type="arg">
      <datatype>void*</datatype><name>Buffer</name>
      <description><en>a pointer to the track buffer we want to write</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>rounded number of clusters per track</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Write_Track( File_Handle, 0, Track_Buffer) == ABORTED)</usage>
  <comment>
    the function first positions the current position in the file
    at the track we want to write. The track number is computed
    relatively to the file not to the disk.
    The function should not be used to create a file because when
    creating a file the position in the file is always at the end
    so there is no need to perform a seek, this can avoid some
    errors when trying to position outside the file.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Write_Track ( int File_Handle, OFFSET Offs, void * Buffer)
// ---------------------------------------------------------------------------
{
   unsigned short Length = Cluster_Length * Clusters_Grouping;

   if( Floppy_Fseek( File_Handle, Offs * (long) Length) == ABORTED)
      return ABORTED;

   return Floppy_Write_Buffer( File_Handle, Length, Buffer);
}
//</code></function>

/*<function id="Floppy_Rename_File">
<heading>
  <description><en>
    renames a file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Old</name>
      <description><en>the old file name</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>New</name>
      <description><en>the new file name</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Rename_File( "a:b001.tmp", "a:b001") == ABORTED)</usage>
  <checks>Floppy_Request_Disk</checks>
  <comment>
    it is possible to lose the file when you rename it if you remove
    the disk at a critical point, this point is very difficult to
    obtain, but you should remember that this function is not 100%
    safe, lets say 95% is more realist.
    In fact the renamed file exist but her length is 0.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Rename_File ( char * Old, char * New)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( rename( Old, New) == 0)
        return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

//==
//       A B S O L U T E   F L O P P Y   A C C E S S   F U N C T I O N S
//==

/*<function id="Floppy_Read_Cluster">
<heading>
  <description><en>
    read a file cluster with absolute disk access</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>File_Chain</name>
      <description><en>the file's cluster chain</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Cluster</name>
      <description><en>
      the file's cluster number to read
      0 = first cluster used by the file</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Data</name>
      <description><en>the cluster's data to read</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Max_Clusters</name>
      <description><en>the maximum number of clusters on the floppy</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Sectors_Per_Cluster</name>
      <description><en>number of sectors per cluster</en></description></parameter>
  </parameters>
  <usage>if (Floppy_Read_Cluster( File_Chain, 0, Buffer) == ABORTED)</usage>
  <checks>
    the given cluster number is checked to avoid trying to access
    outside of the file, the total number of clusters used by the
    file and the maximum number of clusters on the floppy are the
    limits.</checks>
  <comment>
    first cluster has number 0</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Read_Cluster ( FILE_CHAIN File_Chain, int Cluster, char * Data)
// ---------------------------------------------------------------------------
{
   for( int i=0; File_Chain [i]; i++);              // gets number of clusters

   if( Cluster >= i || i > Max_Clusters)      // try to access outside of file
      return ABORTED;

   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( ! OwnAbsRead( 0, Sectors_Per_Cluster, File_Chain [Cluster], Data))
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Read_Clusters">
<heading>
  <description><en>
    intelligent read function, contiguous clusters are read in one
    time, uses absolute disk access routines</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>File_Chain</name>
      <description><en>the file's cluster chain</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Cluster</name>
      <description><en>the starting cluster number for the read</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Clusters_To_Read</name>
      <description><en>the number of clusters to read</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Data</name>
      <description><en>the pointer to the buffer where to put the read data</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Max_Clusters</name>
      <description><en>the maximum number of clusters on the floppy</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Sectors_Per_Cluster</name>
      <description><en>number of sectors per cluster</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Read_Clusters( File_Chain, 0, 10, Buffer) == ABORTED)</usage>
  <checks>
    Floppy_Request_Disk() ,
    The last cluster number to read is checked to avoid trying to
    access outside of the file, the total number of clusters used
    by the file and the maximum number of clusters on the floppy
    are the limits.</checks>
  <comment>
    The function optimizes the absolute read of multiple clusters
    by grouping the reading of the continuous clusters.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Read_Clusters ( FILE_CHAIN File_Chain, int Cluster,
                               int Clusters_To_Read, char * Data)
// ---------------------------------------------------------------------------
{
   int N_Clusters_Read = 0,             // the number of already read clusters
       Current_Cluster = Cluster,   // the actual cluster position in the file
       N_Clusters_To_Read = 0;   // the number of clusters to read in one read

   for( int i=0; File_Chain [i]; i++);              // gets number of clusters

   if( (Current_Cluster + Clusters_To_Read) > i || i > Max_Clusters)
      return ABORTED;                         // try to access outside of file

   while( N_Clusters_Read < Clusters_To_Read)   // until all clusters are read
   {
      N_Clusters_To_Read = 1;

      for(;;)                  // get the maximum number of contiguous sectors
      {
         if( N_Clusters_Read + N_Clusters_To_Read == Clusters_To_Read)
            break;

         // check if the next cluster number in the file chain is equal to
         // the current cluster number plus the number of sectors per cluster
         // (then the clusters are contiguous)

         if( File_Chain [Current_Cluster + N_Clusters_To_Read - 1] +
             Sectors_Per_Cluster !=
             File_Chain [Current_Cluster + N_Clusters_To_Read])

            break;                              // clusters are not contiguous

         N_Clusters_To_Read++;    // clusters are contiguous, so read one more
      }

      for(;;)                  // read the number of contiguous clusters found
      {
         while( Floppy_Request_Disk() == ABORTED);

         if( ! OwnAbsRead( 0, Sectors_Per_Cluster * N_Clusters_To_Read,
                           File_Chain [Current_Cluster],
                           Data + (N_Clusters_Read * Cluster_Length) ))
         {
            Reset_Errors();
            break;
         }

         if( Floppy_Handle_Error() == ABORTED)
            return ABORTED;
      }

      N_Clusters_Read += N_Clusters_To_Read;
      Current_Cluster += N_Clusters_To_Read;
   }

   return Reset_Errors();
}
//</code></function>

/*<function id="Floppy_Write_Cluster">
<heading>
  <description><en>
    writes a cluster by accessing the floppy absolutely</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>File_Chain</name>
      <description><en>the file's cluster chain</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Cluster</name>
      <description><en>
      the file's cluster number to write
      0 = first cluster used by the file</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Data</name>
      <description><en>the cluster's data to write</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Max_Clusters</name>
      <description><en>the maximum number of clusters on the floppy</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Sectors_Per_Cluster</name>
      <description><en>number of sectors per cluster</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Write_Cluster( File_Chain, 1, Cluster_Data) == ABORTED)</usage>
  <checks>
    the given cluster number is checked to avoid trying to access
    outside of the file, the total number of clusters used by the
    file and the maximum number of clusters on the floppy are the
    limits.</checks>
  <comment>
    first cluster has number 0</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Write_Cluster ( FILE_CHAIN File_Chain, int Cluster, char * Data)
// ---------------------------------------------------------------------------
{
   for( int i=0; File_Chain [i]; i++);         // gets number of clusters used

   if( Cluster >= i || i > Max_Clusters)      // try to access outside of file
      return ABORTED;

   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( ! OwnAbsWrite( 0, Sectors_Per_Cluster, File_Chain [Cluster], Data))
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Get_File_Chain">
<heading>
  <description><en>
    gets all the cluster chain used by the file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>the file's name</en></description></parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN*</datatype><name>File_Chain</name>
      <description><en>the collected cluster chain</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Get_File_Chain( "a:b003", &amp; File_Chain) == ABORTED)</usage>
  <checks>
    if there was a problem during the chain reading the function
    returns ABORTED</checks>
  <comment>
    The source file name may not contain a directory specification
    because a lower level function does'nt support it, the source
    file name can be specified as "a:b001" but not as "a:\\b001"

    see HelpTech for a full explanation</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Get_File_Chain ( char * File_Name, FILE_CHAIN * File_Chain)
// ---------------------------------------------------------------------------
{
   struct fcb Fcb;

   if( Floppy_Get_FCB( File_Name, & Fcb) == ABORTED)
   {
      * File_Chain = NULL;
      return ABORTED;
   }

   for( ;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      * File_Chain = Dos_GetClusterChain( & Fcb, Cluster_Length);

      if( * File_Chain != NULL)
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Get_FCB">
<heading>
  <description><en>
    get the file's FCB</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>File_Name</name>
      <description><en>the file name</en></description></parameter>
    <parameter type="arg">
      <datatype>struct fcb*</datatype><name>Fcb</name>
      <description><en>were to put the read fcb</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Get_FCB( "a:b003", &amp; Struct_Fcb) == ABORTED)</usage>
  <comment>
    This function is only used before collecting the file chain
    The file name may not contain a directory specification
    because a lower level function does'nt support it, the source
    file name can be specified as "a:b001" but not as "a:\\b001"

    see HelpTech for a full explanation</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Floppy_Get_FCB ( char * File_Name, struct fcb * Fcb)
// ---------------------------------------------------------------------------
{
   for(;;)
   {
      while( Floppy_Request_Disk() == ABORTED);

      if( Dos_FindFirstDB( Fcb, File_Name))
         return Reset_Errors();

      if( Floppy_Handle_Error() == ABORTED)
         return ABORTED;
   }
}
//</code></function>

/*<function id="Floppy_Get_FAT_Info">
<heading>
  <description><en>
    reads the floppy FAT to collect some very important structural
    informations</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED  function has succeeded,
        ABORTED    function has failed</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Max_Clusters</name>
      <description><en>the maximum number of clusters on the floppy</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Sectors_Per_Cluster</name>
      <description><en>number of sectors per cluster</en></description></parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>rounded number of clusters per track</en></description></parameter>
  </parameters>
  <usage>if( Floppy_Get_FAT_Info() == ABORTED)</usage>
  <checks>memory allocation</checks>
  <comment>
    This function must be called as soon as possible because it
    sets critical values used to allocate memory and to access floppy
    with direct functions. It must be called before
    Alloc_Memory_BAC().

    See Helptech for a full explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Floppy_Get_FAT_Info ( void)
// ---------------------------------------------------------------------------
{
   if( !BootSector)
     if( Floppy_Read_BootSector() == ABORTED)
       return ABORTED;

   Sectors_Per_Cluster = BootSector->SectorsPerCluster;

   Cluster_Length =   BootSector -> BytesPerSector
                    * BootSector -> SectorsPerCluster;

   Max_Clusters = (int)(  BootSector -> TotalSectors
                        / BootSector -> SectorsPerCluster);

   Clusters_Grouping = (unsigned short)( (  BootSector -> SectorsPerTrack
                                          + BootSector -> SectorsPerCluster
                                          - 1)
                                         / BootSector -> SectorsPerCluster);

   return SUCCEEDED;
}
//</code></function>

/*<function id="copyRAMFileToDisk">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE copyRAMFileToDisk (
	char* pSource,			// input filename
	char* pTarget			// output filename
	)
// ---------------------------------------------------------------------------
{
		FILE* 	input;
		int		output;

		char 	buffer[1024];
		int 	bytes_read;

	input = fopen( pSource, "rb");	// open for binary read
    if ( Floppy_Create_File( pTarget, &output) == ABORTED) return ABORTED;

	while( ( bytes_read = fread( buffer, 1, sizeof( buffer), input)) != 0)
	{
	    if( Floppy_Write_Buffer( output, bytes_read, buffer) == ABORTED) return ABORTED;
	}

	fclose( input);
	if ( Floppy_Close_File( output) == ABORTED) return ABORTED;

	return SUCCEEDED;
}
//</code></function>

/*<function id="secureDeleteFile">
<heading>
  <description>Wipes a file's content before deleting it</description>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE secureDeleteFile(
	char* pTarget				// in: filename to secure delete
	)
// ---------------------------------------------------------------------------
{
		FILE* 			fs;
		int 			i;
		long 			fileSize, j;
		unsigned char	patterns[] = { 0x00, 0x11, 0x55, 0xFF};
		unsigned char	pattern;

	for ( i = 0; i < sizeof( patterns); i++ )
	{
		pattern = patterns[i];
		fs = fopen( pTarget, "r+b");	// open for binary read/write
		if ( fs)
		{
			fileSize = filelength( fileno( fs));

			do {
				fwrite( &pattern, 1, 1, fs);
			} while ( --fileSize != 0);

			fflush( fs); fclose( fs);

			#if _DEBUGMODE
				sprintf( gcDcio, "Wiped file using [%02X].", pattern); dcio();
			#endif // _DEBUGMODE

		}
		else
		{
			return ABORTED; // file not found
		}
	}

	if ( unlink( pTarget)) return ABORTED; // file not found or permission denied

      #if _DEBUGMODE
         sprintf( gcDcio, "File[%s] succesfully deleted!", pTarget); dcio();
      #endif // _DEBUGMODE

	return SUCCEEDED;
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
