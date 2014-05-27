/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="urnreco.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   URNRECO.CPP  $</workFile>
<revision>$Revision: 1.14 $</revision>
<workFileTimeStamp>$Modtime:   Jan 26 2005 16:13:34  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Urn/URNRECO.CPP-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/21 12:38:51 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>URNRECO.CPP</fileName>
  <creationDate>20/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Enregistrement des voix sur le système 'URN' ou 'REC'</fr>
    <nl>Opnemen van de stemmen in het 'URN'- of 'REC' - systeem</nl></description>
  <history>
   01/06/93: revision history starting date
   03/09/93: Remove_TOTAL_File function added and Find_Out_Startup_Mode
         modified for the RECOUNT program. We now remove the B001 file
         on both MASTER and BACKUP disks before we go further.
   14/01/94: Remove_TOTAL_File function changed.  We now remove the B001 file
         on all disks MASTER, BACKUP1, BACKUP2, ... .
   22/03/94  rvh-ebe Process_Card_in_Bac has been modified
         Clear_Entry_In_Journal has been added (author: EBE)
   13/04/94  rvh sleep() bijgevoegd v¢¢r sommige ...Remove_Card...</history>
  <comment>Recording of votes on the URN-system</comment>
  <comment>
    Magnetic Cards, entered in the Reader on the BAC, are verified
    and recorded  in dynamic memory.
    The verification has the following steps :
    - Identification/Authentification  : Encryption Key URNMAV
    - Physical Integrity : MAC Checksum on Vote Data
    - Logical Integrety : Lookup Functions

    Valid data on the card are encrypted (method URNVOT) and randomly
    inserted in a structure, as to mimic the physical BAC (Secrecy).

    The maximum number of Cards the 'Memory_BAC' can hold is 2000.

    In case of a fault (e.g. power failure, hardware failure), already
    counted votes can be restored in main memory to resume the registration.
    This is achieved by maintaining before and after journals of the Memory_BAC
  </comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <errno.h>
#include <sys\stat.h>
//#include <fstream.h>

#include "..\gen\gentype.h"
#include "..\gen\gencard.h"
#include "..\gen\gendata.h"
#include "..\gen\genflop.h"
#include "..\gen\gencryp.h"

#include "..\urn\urnmes.h"
#include "..\urn\urnerro.h"
#include "..\urn\urnreco.h"
#include "..\urn\status.h"

#include "..\divers\logging.h"
//</includes>

#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#define _TIMERMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#define _TRACEMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
   #include "..\tools\chrono.h"
#endif  // _DEBUGMODE

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static RET_CODE Write_Flag_Integrity( int Journal_File);
static RET_CODE Read_Flag_Integrity( int Journal_File);

static void Switch_To_BACKUP( void);
static void Install_BACKUP( void);
static void Restart_Recount( void);
static void Process_Comp_Error( void);
static void Process_Restart_Counter( void);
static void Process_Restart_BAC( void);

static void Find_Out_Startup_Mode(void);
static void Remove_TOTAL_File (void);
// functione added for demo-version (date: 10/03/94 - author: EBE)
void Remove_All_Files_EveryWhere(void) ;
void Allocate_Memory_BAC( void);
static int Cluster_Position( int Card_Position);
static int Position_In_Cluster( int Card_Position);

static RET_CODE Dump_Memory_BAC( int Journal_File);
static RET_CODE Dump_Memory_Card( FILE_CHAIN Journal_File_Chain, int Index);

//rvh-ebe toegevoegd op 22/03/94
static void Clear_Entry_In_Journal( FILE_CHAIN Journal_File_Chain, int Index);

static int Drop_In_Memory_BAC( void);
static RET_CODE Set_Data_Error( void);
static RET_CODE Restore_Memory_BAC( int Journal_File);

static void Restore_Counter( int Counter_File);
static RET_CODE Dump_Counter( int Counter_File);
static RET_CODE Dump_Abs_Counter( FILE_CHAIN Counter_File_Chain);

static void Corrupt_Integrity_State( void);

static BOOL Card_Is_Blanco( Select_Reader * Reader);
static BOOL Card_Is_Formatted( Select_Reader * Reader);
static void Concat_Data( char Card_Data [], char Concat_Data []);
static void Deconcat_Data( char Concat_Data [], char Card_Data []);
// new functions for DEMO-version only Date: 18/08/94 - author: EBE
static BOOL Test_File_Exists( char* filename);
static void Remove_All(void) ;

static void Initialize_Memory_BAC ( void);
static RET_CODE Create_Journals_BAC ( void);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern int Cluster_Length;
extern unsigned short Clusters_Grouping;
extern MODE Mode;
extern STARTUP_MODE    Startup_Mode;
extern INTEGRITY_STATE Flag_Integrity;
extern ERROR_HANDLING Error_Handling;
extern int  PositionCardTypeElector;// stefG: relative position of elector card

extern char Floppy_MASTER[];
extern char BACKUP_Label[];
extern char Floppy_BACKUP[];
extern char * Floppy_Current;
extern SCREEN_TYPE Glob_Screen_Type;

extern BOOL Background_URN;
extern int CurrentTypeElector;
extern int NbrOfCounters;

extern Select_Reader * Intern_Reader, * Extern_Reader;

extern int NUMBER_TYPE_ELECTOR; // stefG number of elector type
extern struct ElectionMain Root;
extern int CurrentTypeElectorInBac; //STEF: Type of elector for last card in BAC

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
#ifdef _TEST
   const int BUF_SIZE = 2048;
#else
   const int BUF_SIZE = 8192;              // maximum possible is 65534 due to _read
#endif

char * TOTALS_TEMP_FILE     = "c:B001.TMP";
char * TOTALS_FILE_NAME     = "a:B001";

char * JRN_BEFORE_BAC       = "a:B003";
char * JRN_AFTER_BAC        = "a:B013";

char * JRN_BEFORE_FORMATTED = "a:B005";
char * JRN_AFTER_FORMATTED  = "a:B015";

char Vote_Data      [MAX_VOTE_BUF];
char Concat_Vote    [MAX_VOTE_BUF];
char Vote_Encrypted [MAX_VOTE_BUF];

//int Cards_Formatted        = 0; STEF:old variable counter card validate
//int Cards_Really_Formatted = 0; STEF:old variable counter card really formatted
//int Cards_In_BAC = 0;           STEF:
int TAB_C_Formatted [MAX_TYPE_ELECTOR]; // STEF:new array with counter list card formatted
int TAB_CR_Formatted[MAX_TYPE_ELECTOR]; // STEF:new array with counter list card really formatted
int TAB_C_InBAC     [MAX_TYPE_ELECTOR]; // STEF:new array with counter list card in BAC

int Concat_Length, Data_Length = 0, Memory_Card_Length, Journal_Card_Length;


int Cards_Per_Cluster;
int Nb_Tracks, Nb_Clusters;
char ** Clusters_BAC, * Clusters_BAC_Partial, * Cluster_Counter;

int Journal_Before = -1;
FILE_CHAIN Journal_Before_Chain = NULL;
int Journal_After = -1;
FILE_CHAIN Journal_After_Chain = NULL;
int Counter_Before = -1;
FILE_CHAIN Counter_Before_Chain = NULL;
int Counter_After = -1;
FILE_CHAIN Counter_After_Chain = NULL;

int Totals_File;
int Totals_Temp_File;

char File_Buffer [BUF_SIZE];
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Recover_Registration">
<heading>
  <description><en>depending on the program state, the function trys to
    recover from an error by jumping to a restart point</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>the 'what to do in case of error' flag</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP[]</name>
      <description><en>the floppy BACKUP label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>the rollback point type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_xxxxx</name>
      <description><en>the jump position markers</en></description>
    </parameter>
  </parameters>
  <usage>if( Error) Recover_Registration();</usage>
  <comment>
   This function is called only if their is a hardware problem with
   the floppy and if the error handling method is ROLLBACK.
   If the current floppy is the floppy BACKUP, there is nothing else
   to do, we go in corrupted mode. Else if we are in the programs
   main loop we first switch to the BACKUP disk, by installing all
   the files, and then we jump to the restart point to redo the
   operation that failed on the MASTER disk.
   If we are in startup mode (not in the main loop) we only jump
   to the restart point.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Recover_Registration ( void)
// ---------------------------------------------------------------------------
{
   Error_Handling = ABORT;        // there can't be a rollback during rollback

   if( Floppy_Current == Floppy_BACKUP)        // even the BACKUP disk is stuk
   {
      Corrupt_Integrity_State();                    // so go in CORRUPTED mode
      Floppy_Close_All();
      Error_Handling = ABORT;
      return;
   }

   switch( Rollback_Point)              // we are before the program main loop
   {
   case RBP_STARTUP  : longjmp( JMP_STARTUP, 1);
   }

   Install_BACKUP();                // install all the file on the BACKUP disk

   switch( Rollback_Point)              // and then redo the failled operation
   {
   case RBP_BEFORE_FORMATTED : longjmp( JMP_BEFORE_FORMATTED, 1);
   case RBP_AFTER_FORMATTED  : longjmp( JMP_AFTER_FORMATTED, 1);
   case RBP_BEFORE_IN_BAC    : longjmp( JMP_BEFORE_IN_BAC, 1);
   case RBP_AFTER_IN_BAC     : longjmp( JMP_AFTER_IN_BAC, 1);
   case RBP_CLOSE_OFF        : longjmp( JMP_CLOSE_OFF, 1);
   }
}
//</code></function>


/*<function id="Install_BACKUP">
<heading>
  <description><en>
    install all the journal files on the BACKUP disk</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>he 'what to do in case of error' flag</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>the rollback point type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
  </parameters>
  <usage>Install_BACKUP();</usage>
  <checks>
   Flag_Integrity, it makes no sense to switch to BACKUP if we are
   in corrutped mode, normally the function should even not been
   called</checks>
  <comment>
   This function makes the image from the MASTER disk on the BACKUP.
   All the files, in the state they actually have, and thus before
   the current update (if any) are created on the BACKUP, if the
   program was registering a card, this card will only be registered
   after the complete BACKUP installation, with the jump to the
   operation's restart point.
   The rollback method is set to ABORT before entering this function
   and it is reset to ROLLBACK when installation was successfull.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Install_BACKUP ( void)
// ---------------------------------------------------------------------------
{
    Error_Handling = ABORT;

    if( Flag_Integrity == CORRUPTED)         // it makes no sense to go further
        return;

    Switch_To_BACKUP();                             // ask to insert the BACKUP

    #ifndef _RECOUNT
    if( Create_Journals_Formatted() == ABORTED)        // install B005 and B015
    {
        Corrupt_Integrity_State();
            return;
    }
    #endif

    if( Create_Journals_BAC() == ABORTED ||            // install B003 and B013
        Initialize_Direct_Access() == ABORTED) // B003 and B013 absolute access
    {
        Corrupt_Integrity_State();
            return;
    }

    // restore the message that was displayed before switching to the BACKUP

    if( Rollback_Point == RBP_CLOSE_OFF)
    {
        Display_Message( M_URN_END1);
        Display_Message( M_WAIT, 0, 1);
    }
    else
        ScreenAllCounters();

    Error_Handling = ROLLBACK;       // succesfull recovery, now again ROLLBACK
}
//</code></function>


/*<function id="Switch_To_BACKUP">
<heading>
  <description><en>
    ask and wait until the BACKUP disk is inserted in the floppy</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>the 'what to do in case of error' flag</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP[]</name>
      <description><en>the floppy BACKUP label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description>
    </parameter>
  </parameters>
  <usage>Switch_To_Backup();</usage>
  <checks>floppy functions</checks>
  <comment>
   The error handling is set to abort during BACKUP switching to
   avoid error cascading.
   The function ask the user to remove the MASTER disk and to insert
   the BACKUP.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Switch_To_BACKUP ( void)
// ---------------------------------------------------------------------------
{
   Error_Handling = ABORT;                     // no rollback in rollback mode

   Floppy_Close_All();                 // close the handles on the MASTER disk

   // 20061128 avn - flush files added while function causes a disk reset,
   // without disk reset reading the disk label can return a dos error
   Floppy_Flush_Files();

   Display_Message( M_ERR_DISK2, 1, 0);

   Floppy_Current = Floppy_BACKUP;                         // switch to BACKUP

   if( Floppy_Request_Disk() == ABORTED)                 // request the BACKUP
   {
      Corrupt_Integrity_State();
      return;
   }

   Display_Message( M_WAIT);

   Error_Handling = ROLLBACK;
}
//</code></function>


/*<function id="Initialize_Direct_Access">
<heading>
  <description><en>get the journal files floppy mapping to access them with the
    direct floppy routines</en></description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = no error,
      ABORTED   = error while getting file chain</en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_BEFORE_BAC</name>
      <description><en>the file names</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_AFTER_BAC</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_BEFORE_FORMATTED</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_AFTER_FORMATTED</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Journal_Before_Chain</name>
      <description><en>the files cluster chain</en></description>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Journal_After_Chain</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Counter_Before_Chain</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Counter_After_Chain</name><description/>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
  </parameters>
  <usage>if( Initialize_Direct_Access() == ABORTED)</usage>
  <comment>
   If the program is CORRUPTED, the function only performs a free
   of previously allocated chains.
   Only the journal files are accessed in direct mode.
   We can not close the associated file handles because we may need
   them to write the integrity flag, this is because the integrity
   flag can become corrupted before we have the file chains.
   The counter files handles can be closed because they are only
   accessed with absolute floppy routines once we are in the
   program's main loop.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Initialize_Direct_Access ( void)
// ---------------------------------------------------------------------------
{
   if( Journal_Before_Chain)
      free( Journal_Before_Chain);
   if( Journal_After_Chain)
      free( Journal_After_Chain);
   if( Counter_Before_Chain)
      free( Counter_Before_Chain);
   if( Counter_After_Chain)
      free( Counter_After_Chain);

   Journal_Before_Chain = Journal_After_Chain = NULL;
   Counter_Before_Chain = Counter_After_Chain = NULL;

   if( Flag_Integrity == CORRUPTED)
      return SUCCEEDED;

   if( Floppy_Get_File_Chain( JRN_BEFORE_BAC, & Journal_Before_Chain) == ABORTED)
      return ABORTED;
   if( Floppy_Get_File_Chain( JRN_AFTER_BAC, & Journal_After_Chain) == ABORTED)
      return ABORTED;
   if( Floppy_Get_File_Chain( JRN_BEFORE_FORMATTED, & Counter_Before_Chain) == ABORTED)
      return ABORTED;
   if( Floppy_Get_File_Chain( JRN_AFTER_FORMATTED, & Counter_After_Chain) == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Write_Flag_Integrity">
<heading>
  <description><en>
    writes the current flag integrity in a journal file</en></description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = no error,
      ABORTED   = error while writing flag</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Journal_File</name>
      <description>
        <en>the file handle where to write the flag integrity</en>
      </description><defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the memory allocated to store all the file contents in
      memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
  </parameters>
  <usage>if( Write_Flag_Integrity( Journal_After) == ABORTED)</usage>
  <checks>floppy functions</checks>
  <comment>
   The flag integrity uses the first cluster in the journal file,
   this is too much space for it but by this way it is more easier
   to access the cards in the journal file.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Write_Flag_Integrity ( int Journal_File)
// ---------------------------------------------------------------------------
{
   Clusters_BAC [0][0] = Flag_Integrity;

   if( Floppy_Write_At( Journal_File, 0, Clusters_BAC[0] ) == ABORTED ||
       Floppy_Flush_Files() == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Corrupt_Integrity_State">
<heading>
  <description><en>
    sets the flag integrity value to CORRUPTED and writes the flag
    in the two journal files</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>the 'what to do in case of error' flag</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Before</name>
      <description><en>the B003 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_After</name>
      <description><en>the B013 file handle</en></description>
    </parameter>
  </parameters>
  <usage>Corrupt_Integrity_State()</usage>
  <checks>
   if the flag integrity is already corrupted it makes no sense to
   run the function once again. If the B003 or B013 file does not
   exist we can't write the flag integrity.</checks>
  <comment>
   We need the B003 and B013 file handles even if we already know
   their cluster chain, this is because the function can be called
   before we know the chain.
   The function only trys to write the flag integrity, if the floppy
   function gives an error we don't worry about it.
   If the program is the REC then there is no corrupted mode, you
   must recount the votes with a new disk.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Corrupt_Integrity_State ( void)
// ---------------------------------------------------------------------------
{
#ifndef _RECOUNT

   if( Flag_Integrity == CORRUPTED)                         // once, always...
      return;

   Display_Message( M_CORRUPTED, 1, 0);              // goes in corrupted mode
   Flag_Integrity = CORRUPTED;
   Error_Handling = ABORT;

   if( Journal_Before == -1)
      return;

   Write_Flag_Integrity( Journal_Before);

   if( Journal_After == -1)
      return;

   Write_Flag_Integrity( Journal_After);

#else

   Error_Exit( M_MUST_RECOUNT);

#endif  // _RECOUNT
}
//</code></function>


/*<function id="Read_Flag_Integrity">
<heading>
  <description><en>
    reads the flag integrity from a journal file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED  = no error while reading flag,
      ABORTED    = error while reading flag.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Journal_File</name>
      <description><en>the file handle were to write the flag in</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the memory allocated to store all the file contents in
      memory</en></description>
    </parameter>
  </parameters>
  <usage>if( Read_Flag_Integrity( Journal_Before) == ABORTED)</usage>
  <checks>floppy function</checks>
  <comment>Only B003 and B013 have the correct structure. By default the
   flag integrity is set the CORRUPTED, it is only reset if we can
   read the file and if the value in the file corresponds to
   CONSISTENT.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Read_Flag_Integrity ( int Journal_File)
// ---------------------------------------------------------------------------
{
   Flag_Integrity = CORRUPTED;

   if( Floppy_Read_At( Journal_File, 0, Clusters_BAC [0] ) == ABORTED)
      return ABORTED;

   if( ! Clusters_BAC [0][0])
      Flag_Integrity = CONSISTENT;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Restart_Recount">
<heading>
  <description><en>
    reinitialize the journal files to restart the recount process</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Restart_Recount();</usage>
  <comment>
   when an error occurs when restarting an interrupted recount
   process or simply if the user wants to restart the recount,
   this function ask the user to remove all the cards from the BAC,
   reinitialize the memory and the journal files.</comment>
</heading>
<code>*/
#ifdef _RECOUNT
// ---------------------------------------------------------------------------
static void Restart_Recount ()
// ---------------------------------------------------------------------------
{
   Display_Message( M_REMOVE_CARDS, 1, 0);
   Initialize_Memory_BAC();
   Create_Journals_BAC();
   Display_Message( M_WAIT);
}
#endif  // _RECOUNT
//</code></function>


/*<function id="Process_Comp_Error">
<heading>
  <description><en>
    execute the function corresponding to the current program when
    a comparison error occurs</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Process_Comp_Error();</usage>
  <comment>
   This function only calls another function but the function is
   different wether _RECOUNT is defined or not, depending on the
   program. This is done to get more lisibility in the code.
   For the REC program a comparison error means you must restart the
   recounting. For the URN you go in CORRUPTED mode.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Comp_Error ()
// ---------------------------------------------------------------------------
{
#ifndef _RECOUNT
   Corrupt_Integrity_State();
#else
   Restart_Recount();
#endif
}
//</code></function>


/*<function id="Setup_Registration">
<heading>
<description>
  <en>prepares the system for the vote processing</en></description>
<parameters>
  <parameter type="return"><datatype>void</datatype></parameter>
  <parameter type="arg"><datatype>void</datatype></parameter>
  <parameter type="global">
    <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
    <description><en>program CORRUPTED ?</en></description>
  </parameter>
  <parameter type="global">
    <datatype>char</datatype><name>Floppy_MASTER []</name>
    <description><en>the floppy MASTER label</en></description>
  </parameter>
  <parameter type="global">
    <datatype>char</datatype><name>Floppy_BACKUP []</name>
    <description><en>the floppy BACKUP label</en></description>
  </parameter>
  <parameter type="global">
    <datatype>char*</datatype><name>Floppy_Current</name>
    <description><en>the current floppy label</en></description>
  </parameter>
  <parameter type="global">
    <datatype>int</datatype><name>Counter_Before</name>
    <description><en>the B005 file handle</en></description>
  </parameter>
  <parameter type="global">
    <datatype>int</datatype><name>Counter_After</name>
    <description><en>the B015 file handle</en></description>
  </parameter>
  <parameter type="global">
    <datatype>STARTUP_MODE</datatype><name>Startup_Mode</name>
    <description><en>the startup mode</en></description>
  </parameter>
</parameters>
<usage/>
<checks>Flag_Integrity</checks>
<comment>
This is the key function for the URN restart, it prepares the
system for the vote processing by loading the database,
allocating the memory used to store the journal file, creating
or reprocessing the B003 and B013 files.
If there is an error while reading the B021 file, this is a
fatal error, you must restart the system with the spare disk.
There are two 'modes', one for a first time start and one for
a restart after incident.
A first time start only says: the journal files are not on this
floppy, but if the floppy is BACKUP they may exist on the MASTER.
In that case there are cards in the BAC, the MASTER was corrupted
and the BACKUP installation was not completed. There is no other
way to know if there are already cards in the BAC than asking the
user about it. If there are already cards we can't recover these
cards so we go in CORRUPTED mode.
If we are in RESUME mode we know the flag integrity may have been
set in the journal before file, so we read it.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Setup_Registration ( void)
// ---------------------------------------------------------------------------
{
   if( Setup_Database() == ABORTED)                 // load the election datas
   {
      Display_Message( M_B021_CORRUPTED, 1);

      if( Floppy_Current == Floppy_MASTER)
         Error_Exit( E_REBOOT_BACKUP);
      else
         Error_Exit( E_FATAL_DISKETTE);
   }
   //rvh
   Allocate_Memory_BAC();         // allocate the memory for the cards storing

   Find_Out_Startup_Mode();    // determines if it is a restart after incident

   if( Flag_Integrity == CORRUPTED)        // bad luck the system is corrupted
      return;

   switch( Startup_Mode)
   {
   case FIRST_TIME :

    if( Floppy_Current == Floppy_BACKUP)

     if( Ask_Question( M_ALREADY_CARDS) )     // bad luck we can't recover
     {
#ifndef _RECOUNT
        Corrupt_Integrity_State();
        Display_Message( M_WAIT);
        return;
#else
        Restart_Recount();
        break;
#endif
     }
     else
        Display_Message( M_WAIT);

   // 20040302 avn - E_OUT_OF_MEMORY problem
   // when the B021 contains huge amounts of data it can take up enough space on the
   // floppy to prevent the creation of the B003/B013, this was not checked before...
   Initialize_Memory_BAC();
   if ( Create_Journals() == ABORTED)        // install B005, B015, B003 and B013
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "No disk space available for journals!"); dcio();
      #endif // _DEBUGMODE
      Corrupt_Integrity_State();
   }

    break;

   case RESUME:

      Display_Message( M_URN_RESTART);

      Read_Flag_Integrity( Journal_Before);  // read the flag integrity from
                           // the existing files
      if( Flag_Integrity == CORRUPTED)
      {
#ifndef _RECOUNT
         Display_Message( M_CORRUPTED, 1, 0);
         return;
#else
         Restart_Recount();
         break;
#endif
      }
#ifndef _RECOUNT
      Process_Restart_Counter();     // recover the initialized cards counter

      Floppy_Close_File( Counter_Before);    // we don't need these handles
      Floppy_Close_File( Counter_After);     // anymore
#endif
      Process_Restart_BAC();                 // recover the voted cards
   }
}
//</code></function>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _RECOUNT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Process_Restart_Counter">
<heading>
  <description><en>
    recover the informations already in the journal files on floppy</en></description>
  <parameters>
    <parameter type="return"><datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_Before</name>
      <description><en>the B005 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_After</name>
      <description><en>the B015 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>number of bytes per cluster</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Intern_Reader</name>
      <description><en>the intern reader</en></description>
    </parameter>
  </parameters>
  <usage>Process_Restart_Counter();</usage>
  <comment>
    This function uses the B005 and B015 to recover the already
    formatted cards counter. We restore the contents of one file
    to recreate the other.

        - If the files are identical, we can recover from each file.
        If there is a card in the intern reader we eject this card, it
        may be formatted or not, we don't look for it.
        - If the files are not same and if there is a card in the reader
        we restore the B015 file (after file) and we deformat the card
        before we eject it. So we are sure to have a correct counter
        because the card may or may not have been formatted
        - If the comparison aborts we go in CORRUPTED mode because we
        can't recover the counters
        - If there is a CRC error on the B005 file (before file), we
        restore the B015 counter. If there is a card in the reader, we
        deformat it as if the B005 and B015 files were different.
        But if there was no CRC error and the B005 and B015 were equal
        this will deformat an already counted card.
        It can give an error of 1 card in some cases.
        - If there is a CRC error on the B015 file we restore the B005
        file and we only eject the present card.
        But if there was no CRC error and the B005 and B015 were
        different this can count card that was not formatted yet.
        It can give an error of 1 card in some cases.

    Refer to the Process_Card_Formatting function for a complete
    understanding</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Restart_Counter ( void)
// ---------------------------------------------------------------------------
{
    switch( Floppy_Compare_Files( Counter_Before, Counter_After, Cluster_Length) )
    {
        case SAME :
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRC - SAME.");
            #endif // _TRACEMODE

            if( Reader_Card_Position( Intern_Reader) != NO_CARD)
            {
                Reader_Eject_Card( Intern_Reader);
                Display_Message( M_TAKE_CARD);
                Reader_Wait_Card_Removed( Intern_Reader);
                Display_Message( M_URN_RESTART);
            }

            Restore_Counter( Counter_After);
            break;

        case NOT_SAME :
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRC - NOT_SAME.");
            #endif // _TRACEMODE

            if( Reader_Card_Position( Intern_Reader) == HOME)
            {
                Restore_Counter( Counter_After);
                Dump_Counter( Counter_Before);
                Reader_Deformat_Card( Intern_Reader);
            }
            else
            {
                Restore_Counter( Counter_Before);
                Dump_Counter( Counter_After);
            }

            if( Reader_Card_Position( Intern_Reader) == HOME)
            {
                Reader_Eject_Card( Intern_Reader);
                Display_Message( M_CARD_NOT_FORMATTED);
                sleep(5);
                Reader_Wait_Card_Removed( Intern_Reader);
                Display_Message( M_URN_RESTART);
            }
            break;

        case COMP_ABORT :
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRC - COMP_ABORT.");
            #endif // _TRACEMODE

            Process_Comp_Error();
            break;

        case CRC_ERROR_A :
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRC - CRC_ERROR_A.");
            #endif // _TRACEMODE

            Restore_Counter( Counter_After);
            Dump_Counter( Counter_Before);

            if( Reader_Card_Position( Intern_Reader) == HOME)
            {
                Reader_Deformat_Card( Intern_Reader);
                Reader_Eject_Card( Intern_Reader);
                Display_Message( M_CARD_NOT_FORMATTED);
                sleep(5);
                Reader_Wait_Card_Removed( Intern_Reader);
                Display_Message( M_URN_RESTART);
            }
            break;

        case CRC_ERROR_B :
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRC - CRC_ERROR_B.");
            #endif // _TRACEMODE

            Restore_Counter( Counter_Before);
            Dump_Counter( Counter_After);

            if( Reader_Card_Position( Intern_Reader) == HOME)
            {
                Reader_Eject_Card( Intern_Reader);
                Display_Message( M_TAKE_CARD);
                Reader_Wait_Card_Removed( Intern_Reader);
                Display_Message( M_URN_RESTART);
            }
            break;
    }
}
//</code></function>


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Process_Restart_BAC">
<heading>
  <description><en>
    recover the informations already in the journal files on floppy</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Clusters</name>
      <description><en>number of clusters used by a journal file (B003)</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>number of bytes per cluster</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Extern_Reader</name>
      <description><en>the extern reader</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Before</name>
      <description><en>the B003 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_After</name>
      <description><en>the B013 file handle</en></description>
    </parameter>
  </parameters>
  <usage>Process_Restart_BAC();</usage>
  <comment>
    The conversion int * int -&gt; long lose significant bits because
    int * int = int so you must first convert the int to long.

    This function uses the B003 and B013 to recover the cards
    already in BAC. We restore the contents of one file
    to recreate the other.

    - If the files are identical, we can recover from each file.
    If there is a card in the extern reader, it will be processed
    later by the standard program loop.
    - If the files are not same and if there is a card in the extern
    reader, we restore the B013 file because it was not dropped,
    the card will be reprocessed by the standard program loop.
    - If the files are not same and there is no card in the extern
    reader we restore the B003 file because the card was already
    dropped.
    - If the comparison aborts we go in CORRUPTED mode because we
    can't recover the files
    - If there is a CRC error on the B003 file and if there is still
    a card in the external reader we restore the B013. If there is
    no card in the reader we can't conclude, so we go in
    corrupted mode. The card is already dropped and we are not sure
    about the fact that the B003 was equal to the B013 if there
    was no CRC error. So by restoring the B013 we can lose a card
    and this is not permitted.
    The card will be reprocessed by the standard program loop.
    - If there is a CRC error on the B013 file and if there is no
    card in the external reader we restore the B003. If there is
    still a card in the reader we can't conclude, so we go in
    corrupted mode. The card is still in the reader and we don't
    know if it is already in the B003 file. Restoring the B003
    could count a card twice.
    The card will be reprocessed by the standard program loop.

    Refer to the Process_Card_In_BAC function for a complete
    understanding</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Restart_BAC ( void)
// ---------------------------------------------------------------------------
{
      IMAGE_STATE ImageStatus;

   long File_Length = (long)Nb_Clusters * (long)Cluster_Length;          //!!!

   switch( Floppy_Compare_Files( Journal_Before, Journal_After, File_Length) )
   {
   case SAME :
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PRB - SAME.");
      #endif // _TRACEMODE

      Restore_Memory_BAC( Journal_Before);
      // no dump needed, images are identical
      break;

   case NOT_SAME :
      switch( Reader_Card_Position( Extern_Reader) )
      {
      case HOME : // card in reader
         // rollback images, card will be reprocessed by the urn loop
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PRB - NOT_SAME - HOME.");
         #endif // _TRACEMODE
         Restore_Memory_BAC( Journal_After);
         Dump_Memory_BAC( Journal_Before);
         break;

      case NO_CARD : // card not in reader ( at the gate, taken away, dropped)
      default      :

         if ( GetImageStatus( &ImageStatus))
         {
            Process_Comp_Error();
            break;
         }
         switch( ImageStatus)
         {
         case IMAGE_STATE_RESET :      // card was not dropped
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRB - NOT_SAME - card NOT dropped.");
            #endif // _TRACEMODE
            Restore_Memory_BAC( Journal_After);
            Dump_Memory_BAC( Journal_Before);
            break;
         case IMAGE_STATE_CONSISTENT : // card was dropped
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRB - NOT_SAME - card dropped.");
            #endif // _TRACEMODE
            Restore_Memory_BAC( Journal_Before);
            Dump_Memory_BAC( Journal_After);
            break;
         default :                     // transaction broken
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PRB - NOT_SAME - broken...");
            #endif // _TRACEMODE
            // corrupted mode is adopted to force a recount
            Process_Comp_Error();
            break;
         }
         break;
      }
      break;

   case COMP_ABORT :
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PRB - COMP_ABORT.");
      #endif // _TRACEMODE

      Process_Comp_Error();
      break;

   case CRC_ERROR_A :
      switch( Reader_Card_Position( Extern_Reader) )
      {
      case HOME :                  // card in reader : must be processed again
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PRB - CRC_ERROR_A - HOME.");
         #endif // _TRACEMODE

         Restore_Memory_BAC( Journal_After);
         Dump_Memory_BAC( Journal_Before);
         break;

      case NO_CARD :                 // card not in reader ( already inserted)
      default      :
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PRB - CRC_ERROR_A - default.");
         #endif // _TRACEMODE

         Process_Comp_Error();
      }
      break;

   case CRC_ERROR_B :
      switch( Reader_Card_Position( Extern_Reader) )
      {
      case HOME :                  // card in reader : must be processed again
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PRB - CRC_ERROR_B - HOME.");
         #endif // _TRACEMODE

         Process_Comp_Error();
         break;

      case NO_CARD :                 // card not in reader ( already inserted)
      default      :
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PRB - CRC_ERROR_B - default.");
         #endif // _TRACEMODE

         Restore_Memory_BAC( Journal_Before);
         Dump_Memory_BAC( Journal_After);
      }
      break;
   }
}
//</code></function>


/*<function id="Find_Out_Startup_Mode">
<heading>
  <description><en>
    determines if all the journal files are created which indicates
    we are in resume mode</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>MODE</datatype><name>Mode</name>
      <description><en>program phase (STARTUP or NORMAL)</en></description>
    </parameter>
    <parameter type="global">
      <datatype>STARTUP_MODE</datatype><name>Startup_Mode</name>
      <description><en>the startup mode</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Extern_Reader</name>
      <description><en>the extern reader</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Totals_File</name>
      <description><en>the B001 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_Before</name>
      <description><en>the B005 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_After</name>
      <description><en>the B015 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Before</name>
      <description><en>the B003 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_After</name>
      <description><en>the B013 file handle</en></description>
    </parameter>
  </parameters>
  <usage>Find_Out_Startup_Mode();</usage>
  <comment>
   If the totals file B001 exist, it means the vote is terminated.
   For the URN program you are only allowed to perform a X25
   transfer a the B001 file. For the REC program, the B001 file isd
   removed so you are allowed to continue the recount.
   Then we try to open all the journal files B003, B013, B005 and
   B015. If none of the files exist or if one the files does not
   exist, the startup mode will be FIRST_TIME. If some files exist
   we close all the opened files because we will recreate them.
   We assume this is due to an aborted STARTUP and not to an other
   that has erased the file from the FAT.
   If all the files exist we are in RESTART mode. If the program
   is the REC the user can chose to erase all the files or to
   continue the counting.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Find_Out_Startup_Mode ( void)
// ---------------------------------------------------------------------------
{
   Floppy_Open_File( TOTALS_FILE_NAME, & Totals_File);

#ifdef _DEMO
   if( Totals_File != -1)                                        // B001 exist
       if(Ask_Question(M_CLEAR_DEMO_DISKETTES))
        {
        Remove_All_Files_EveryWhere() ;
        Error_Exit(M_REBOOT_FOR_NEW_DEMO) ;
        }
#endif _DEMO

#ifdef _RECOUNT

      Remove_TOTAL_File ();

#else

   if( Totals_File != -1)                                        // B001 exist
   {
      Mode = NORMAL;

/*stefG : remove X25
      if( Floppy_Copy_File( "a:x25ftb.exe", "c:\\x25ftb.exe") != ABORTED)
    if( Ask_Question( M_CLOSE_WITH_X25))
       if( Install_X25() == SUCCEEDED)
          Process_X25();
*/
      Error_Exit( M_URN_END2);

   }

   Floppy_Open_File( JRN_BEFORE_FORMATTED, & Counter_Before);
   Floppy_Open_File( JRN_AFTER_FORMATTED , & Counter_After);

#endif // _RECOUNT

   Floppy_Open_File( JRN_BEFORE_BAC      , & Journal_Before);
   Floppy_Open_File( JRN_AFTER_BAC       , & Journal_After );

#ifndef _RECOUNT

   if( Counter_Before == -1 && Counter_After == -1
    && Journal_Before == -1 && Journal_After == -1)

      Startup_Mode = FIRST_TIME;

   else if( Counter_Before != -1 && Counter_After != -1
     && Journal_Before != -1 && Journal_After != -1)

      Startup_Mode = RESUME;

#else

   if( Journal_Before == -1 && Journal_After == -1)
      Startup_Mode = FIRST_TIME;
   else if( Journal_Before != -1 && Journal_After != -1)
   {
      if( Ask_Question( M_RECOUNT))           // continue recount or restart ?
     Startup_Mode = RESUME;
      else                                    // remove all the cards even the
      {                                       // card in the external reader

     Display_Message( M_REMOVE_CARDS, 1, 0);

     if( Reader_Card_Position( Extern_Reader) != NO_CARD)
     {
        Reader_Eject_Card( Extern_Reader);
        Display_Message( M_TAKE_CARD_2);
        Reader_Wait_Card_Removed( Extern_Reader);
     }

     Display_Message( M_WAIT);                         // remove the files
     Floppy_Close_File( Journal_Before);
     remove( JRN_BEFORE_BAC);
     Floppy_Close_File( Journal_After);
     remove( JRN_AFTER_BAC);
     Startup_Mode = FIRST_TIME;
      }
   }

#endif  // _RECOUNT

   else       // Some journal files exist : so assume previous startup aborted
   {
      Startup_Mode = FIRST_TIME;
      Floppy_Close_All();
   }

   #if _TRACEMODE
      sprintf( gcLogBuf, "FOSM - Startup_Mode[%d].", Startup_Mode);
      Add_To_Logging( EMPTY, gcLogBuf);
   #endif // _TRACEMODE
}
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Remove_TOTAL_File">
<heading>
  <description><en>
    remove the B001 file from MASTER and BACKUP disks</en></description>
  <parameters>
    <parameter type="return"><datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_MASTER []</name>
      <description><en>the floppy MASTER label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP []</name>
      <description><en>the floppy BACKUP label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description>
    </parameter>
  </parameters>
  <usage>Remove_TOTAL_File();</usage>
  <checks>on all floppy functions</checks>
  <comment>
   This function removes the B001 file from the MASTER and BACKUP
   disks, so there is no more trace of it. (consistency)</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Remove_TOTAL_File (void)
// ---------------------------------------------------------------------------
{
   char Floppy_Name[15];
   char *Floppy_Boot ;
   SessionList Tag = Find_Session( 8);
   int Copies = ( (Copies = atoi( Tag -> Text)) < 2 ? 2 : Copies);

   Floppy_Boot = Floppy_Current ;

   if( Floppy_Remove_File( TOTALS_FILE_NAME) == ABORTED)
      Error_Exit( M_ERR_DISK1);

   // Save the name of the BACKUP disk
   strncpy( Floppy_Name, Floppy_BACKUP, sizeof(Floppy_Name));

   // skip the BACKUP1 if we already on it
   int i = 1;
   if( Floppy_Current == Floppy_BACKUP)
      i = 2;

   for( ; i < Copies; i++)
   {
      sprintf( Floppy_BACKUP, "%s%d", BACKUP_Label, i);
      Display_Message( M_TAKE_DISK);
      Wait_Until_Floppy_Removed();

      Floppy_Current = Floppy_BACKUP;             // switch to BACKUP

      for ( ; kbhit(); getch());                  // clear the keyboard buffer
      Display_Message( M_BACKUP_IN1, 1);
      Display_Message( M_WAIT);

      if( Floppy_Request_Disk() == ABORTED)       // request the BACKUP
      continue;

      if( Floppy_Remove_File( TOTALS_FILE_NAME) == ABORTED)
      continue;
   }

   // Restore the name of the BACKUP disk
   strncpy( Floppy_BACKUP, Floppy_Name, sizeof(Floppy_Name));

   Display_Message( M_TAKE_DISK);
   Wait_Until_Floppy_Removed();

   Floppy_Current = Floppy_Boot;                      // switch to MASTER

   Display_Message( M_MASTER_IN1, 1);
   if( Floppy_Request_Disk() == ABORTED)              // request the MASTER
      {
      Floppy_Current = Floppy_BACKUP;                 // switch to BACKUP1

      Display_Message( M_BACKUP_IN1, 1);
      if( Floppy_Request_Disk() == ABORTED)           // request the BACKUP1
        Error_Exit( M_ERR_DISK1);
      }
   Display_Message( M_WAIT);
}
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Remove_All_Files_EveryWhere">
<heading>
  <description><en>
    remove All files from MASTER and BACKUP disks</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_MASTER []</name>
      <description><en>the floppy MASTER label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP []</name>
      <description><en>the floppy BACKUP label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description>
    </parameter>
  </parameters>
  <usage>Remove_All_Files_EveryWhere();</usage>
  <checks>on all floppy functions</checks>
  <comment>
   This function removes all journal/total files from the MASTER
   and BACKUP disks so that there's no more trace left of them.
   (clean up of previous election)
   13/04/94: bug-patch =&gt; in the old version it was impossible
   to do a boot with the BACKUP1-diskette because after
   the removal of the tot &amp; journal-files the urn asked the
   MASTER-diskette (hard coded) and locked up after 3 retries
   with the BACKUP1
   In this version the URN asks for the BOOT-diskette</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Remove_All_Files_EveryWhere (void)
// ---------------------------------------------------------------------------
{
   char Floppy_Name[15];
    char *Floppy_Boot;
   SessionList Tag = Find_Session( 8);
   int Copies = ( (Copies = atoi( Tag -> Text)) < 2 ? 2 : Copies);

    Floppy_Boot = Floppy_Current ;
   Remove_All() ;

   // Save the name of the BACKUP disk
   strncpy( Floppy_Name, Floppy_BACKUP, sizeof(Floppy_Name));

   // skip the BACKUP1 if we already on it
   if( Floppy_Current == Floppy_BACKUP)
      sprintf( Floppy_BACKUP, "%s%d", BACKUP_Label, 2);

   for( int i = atoi( Floppy_BACKUP + strlen( BACKUP_Label)); i < Copies; i++)
   {
      Display_Message( M_TAKE_DISK);
      Wait_Until_Floppy_Removed();

      Floppy_Current = Floppy_BACKUP;                      // switch to BACKUP

      for ( ; kbhit(); getch());                  // clear the keyboard buffer
      Display_Message( M_BACKUP_IN1, 1);
      Display_Message( M_WAIT);

      if( Floppy_Request_Disk() == ABORTED)              // request the BACKUP
      Error_Exit( M_ERR_DISK1);

      Remove_All() ;

      sprintf( Floppy_BACKUP, "%s%d", BACKUP_Label, i+1);
   }

   // Restore the name of the BACKUP disk
   strncpy( Floppy_BACKUP, Floppy_Name, sizeof(Floppy_Name));

   Display_Message( M_TAKE_DISK);
   Wait_Until_Floppy_Removed();

   Floppy_Current = Floppy_Boot ;                      // switch to Boot-diskette

    if(Floppy_Boot == Floppy_MASTER)
        Display_Message( M_MASTER_IN1, 1);
    else if (Floppy_Boot == Floppy_BACKUP)
       Display_Message( M_BACKUP_IN1, 1) ;

   if( Floppy_Request_Disk() == ABORTED )              // request the boot
      {
        if(Floppy_Current == Floppy_MASTER)
            {
          Floppy_Current = Floppy_BACKUP;                 // switch to BACKUP1
          Display_Message( M_BACKUP_IN1, 1);
       if( Floppy_Request_Disk() == ABORTED)           // request the BACKUP1
      Error_Exit( M_ERR_DISK1);
            }
        else
            Error_Exit( M_ERR_DISK1);                                                               // We're already using BACKUP1
      }

   Display_Message( M_WAIT);
}
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Restore_Counter" confidential="yes">
<heading>
  <description><en>
    restore the counter value by reading a counter file</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Counter_File</name>
      <description><en>file handle of the counter file to read</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Cluster_Counter</name>
      <description><en>memory allocated to store the cluster;
      used for the counter</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Formatted</name>
      <description><en>the counter of formatted cards; the card may not be
      formatted yet but the counter is already changed</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Really_Formatted</name>
      <description><en>the counter of really formatted cards</en></description>
    </parameter>
  </parameters>
  <usage>Restore_Counter( Counter_Before);</usage>
  <checks>the Floppy_Read_At function should never return ABORTED because
   the error is trapped before by the ROLLBACK procedure, so it is
   not tested !!!</checks>
  <comment>Because we use absolute disk access, all the file are planed for
   that use, so even if the counter files only holds an integer we
   use a whole cluster for it. Reading the file consist to read
   the cluster used by the file. An integer use to bytes, we have
   it differently, because the counter is maximum 2000, we have
   saved this value as 20 and 00.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Restore_Counter ( int Counter_File)
// ---------------------------------------------------------------------------
{
    Floppy_Read_At( Counter_File, 0, Cluster_Counter);

    // LIMITATION: ONE CLUSTER=512 BYTES, 128 integer can be stored in file

    int icounter;
    int icluster = 0;


//    for ( icounter = 0; icounter < ( NbrOfCounters + 1) ; icounter++ ) // STEF: restore counters
    // restore ALL counters
    for( icounter = 0 ; icounter < MAX_TYPE_ELECTOR; icounter++ )
    {
        TAB_C_Formatted [icounter]= Cluster_Counter [icluster] * 100 + Cluster_Counter [icluster+1];
        TAB_CR_Formatted[icounter]= TAB_C_Formatted[icounter];
        icluster += 2;
    }
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif // _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Dump_Counter" confidential="yes">
<heading>
  <description><en>write the counter of formatted cards in the counter file,
   either with standard write routines either with absolute write routines</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = write ok,
      ABORTED   = error while writing counter.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Counter_File</name>
      <description><en>the counter file file handle</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Cluster_Counter</name>
      <description><en>memory allocated to store the cluster;
      used for the counter</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Formatted</name>
      <description><en>the counter of formatted cards; the card may not be
      formatted yet but the counter is already changed</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Really_Formatted</name>
      <description><en>the counter of really formatted cards</en></description>
    </parameter>
  </parameters>
  <usage>if( Dump_Counter( Counter_Before) == ABORTED)</usage>
  <checks>floppy function</checks>
  <comment>
   Because we use absolute disk access, all the file are planed for
   that use, so even if the counter files only holds an integer we
   use a whole cluster for it. Writing the file consist to write
   the cluster used by the file. An integer use to bytes, we have
   it differently, because the counter is maximum 2000, we have
   saved this value as 20 and 00.
   For the Dump_Counter function we use the Cards_Really_Formatted
   value because if we have the case were we must switch to the
   BACKUP disk, we must install the files with the number of cards
   really formatted otherwize if the system hangs before the card
   is processed we can have an error of 1 card.
   The Dump_Abs_Counter use the Cards_Formatted value because this
   function is only called by the Process_Cards_Formatting and not
   when installing the BACKUP or creating the files.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Dump_Counter ( int Counter_File)
// ---------------------------------------------------------------------------
{
   int icounter;
   int icluster=0;

//   for( icounter = 0 ; icounter < ( NbrOfCounters + 1 ); icounter++ ) // STEF:save counters
   // dump ALL counters
   for( icounter = 0 ; icounter < MAX_TYPE_ELECTOR; icounter++ )
   {
     Cluster_Counter [icluster]   = TAB_CR_Formatted[icounter] / 100;
     Cluster_Counter [icluster+1] = TAB_CR_Formatted[icounter] % 100;
     icluster += 2;
   }

   if( Floppy_Write_At( Counter_File, 0, Cluster_Counter) == ABORTED )
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Dump_Abs_Counter" confidential="yes">
<heading>
  <description></description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>Counter_File_Chain</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Dump_Abs_Counter ( FILE_CHAIN Counter_File_Chain)
// ---------------------------------------------------------------------------
{
   int icounter;
   int icluster = 0;

//   for (icounter = 0; icounter < ( NbrOfCounters + 1 ); icounter++ ) //STEF: save counters
   // dump ALL counters
   for( icounter = 0 ; icounter < MAX_TYPE_ELECTOR; icounter++ )
   {
     Cluster_Counter [icluster]  = TAB_C_Formatted[icounter] / 100;
     Cluster_Counter [icluster+1]= TAB_C_Formatted[icounter] % 100;
     icluster += 2;
   }

   if(Floppy_Write_Cluster( Counter_File_Chain, 0, Cluster_Counter) == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Find_Out_Disk">
<heading>
  <description><en>
    get the disk label and ask confirmation if we try to use the
    BACKUP disk</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_MASTER []</name>
      <description><en>the floppy MASTER label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP []</name>
      <description><en>the floppy BACKUP label</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>the current floppy label</en></description>
    </parameter>
  </parameters>
  <usage>Find_Out_Disk();</usage>
  <comment>
   To avoid a mistake we ask a confirmation if the disk is the
   BACKUP. Of course if we start with the BACKUP by mistake, we
   don't have a spare disk for switching.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Find_Out_Disk ( void)
// ---------------------------------------------------------------------------
{
   char Label[13];

   for(;;)
   {
      if( Floppy_Get_Label( Label) == ABORTED)
      {
     Display_Message( M_ERR_DISK1, 1, 0);
     Display_Message( M_WAIT);
     continue;
      }

      if( ! strcmp( Label, "MASTER") )
      {
     Floppy_Current = Floppy_MASTER;
     break;
      }
      else if( !strcmp( Label, "BACKUP1") )
      {
     if( ! Ask_Question( M_USE_BACKUP) )
     {
        Display_Message( M_MASTER_IN1, 1, 0);
        Display_Message( M_WAIT);
        continue;
     }
     Floppy_Current = Floppy_BACKUP;
     Display_Message( M_WAIT);
     break;
      }                                               // not MASTER nor BACKUP
      else                                            // so request MASTER...
     Display_Message( M_MASTER_IN1, 1, 0);
     Display_Message( M_WAIT);
   }
}
//</code></function>


/*<function id="Create_Journals">
<heading>
  <description><en>
    regroups the journals files creation functions calls</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
<usage>If( Create_Journals() == ABORTED)</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Create_Journals ( void)
// ---------------------------------------------------------------------------
{
#ifndef _RECOUNT
   if( Create_Journals_Formatted() == ABORTED)
      return ABORTED;
#endif
   if( Create_Journals_BAC() == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Create_Journals_Formatted">
<heading>
  <description><en>
    creates the counter files (B005 &amp; B015)</en></description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = files correctly created,
      ABORTED   = error while creating file.</en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_Before</name>
      <description><en>the B005 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Counter_After</name>
      <description><en>the B015 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_BEFORE_FORMATTED</name>
      <description><en>the B005 file name</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_AFTER_FORMATTED</name>
      <description><en>the B015 file name</en></description>
    </parameter>
  </parameters>
  <usage>if( Create_Journals_Formatted() == ABORTED)</usage>
  <checks>RET_CODE on all the functions</checks>
  <comment>
   The files are created in binary mode, the counter are dumped
   with the standard floppy access routines and the files are
   closed because they will only be accessed with absolute disk
   access routines, so we don't need the file handles anymore.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Create_Journals_Formatted ( void)     // coded rollback-able...
// ---------------------------------------------------------------------------
{
   if( Floppy_Create_File( JRN_BEFORE_FORMATTED, & Counter_Before) == ABORTED
    || Dump_Counter( Counter_Before)                               == ABORTED
    || Floppy_Close_File( Counter_Before)                          == ABORTED

    || Floppy_Create_File( JRN_AFTER_FORMATTED, & Counter_After)   == ABORTED
    || Dump_Counter( Counter_After)                                == ABORTED
    || Floppy_Close_File( Counter_After)                           == ABORTED

    )  return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Create_Journals_BAC">
<heading>
  <description><en>creates the memory card files (B003 &amp; B013)</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = files correctly created,
      ABORTED   = error while creating file.</en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Before</name>
      <description><en>the B003 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_After</name>
      <description><en>the B013 file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_BEFORE_BAC</name>
      <description><en>the B003 file name</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>JRN_AFTER_BAC</name>
      <description><en>the B013 file name</en></description>
    </parameter>
  </parameters>
  <usage>if( Create_Journals_BAC() == ABORTED)</usage>
  <checks>RET_CODE on all the functions</checks>
  <comment>
   The files are created in binary mode, the memory card
   (Clusters_BAC) is dumped with the standard floppy access
   routines, the files are closed to write the FAT and to flush the
   file, then the files are reopened because we may need the files
   handles to write the flag integrity. Every other write uses the
   absolute disk access routines.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Create_Journals_BAC ( void)
// ---------------------------------------------------------------------------
{
   if( Floppy_Create_File( JRN_BEFORE_BAC, & Journal_Before) == ABORTED
    || Dump_Memory_BAC( Journal_Before)                      == ABORTED
    || Floppy_Close_File( Journal_Before)                    == ABORTED
    || Floppy_Open_File( JRN_BEFORE_BAC, & Journal_Before)   == ABORTED

    || Floppy_Create_File( JRN_AFTER_BAC, & Journal_After)   == ABORTED
    || Dump_Memory_BAC( Journal_After)                       == ABORTED
    || Floppy_Close_File( Journal_After)                     == ABORTED
    || Floppy_Open_File( JRN_AFTER_BAC, & Journal_After)     == ABORTED

    ) return ABORTED;

    if ( CreateStatusFile() != SUCCEEDED) return ABORTED;

    return SUCCEEDED;
}
//</code></function>


/*<function id="Dump_Memory_BAC" confidential="yes">
<heading>
  <description><en>
    write the whole memory card to journal file</en></description>
  <parameters>
    <parameter type="return"><datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = no error during dump,
      ABORTED   = error during dump.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Journal_File</name>
      <description><en>the journal file file handle</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Tracks</name>
      <description><en>number of tracks used by the B003 file</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the memory used to store the B003 image</en></description>
    </parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>number of clusters per track</en></description>
    </parameter>
  </parameters>
  <usage>if( Dump_Memory_BAC( Journal_Before) == ABORTED)</usage>
  <checks>every floppy routine</checks>
  <comment>
   The memory used to store the memory card (image of B003) is
   organized in clusters blocks, but all the clusters belonging to
   the same track were created at the same time to get a contiguous
   memory block, so it is very simple to write the file track per
   track because we know the number of clusters per track, the
   index in the array will be i * Clusters_Grouping.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Dump_Memory_BAC ( int Journal_File)   // coded rollback-able...
// ---------------------------------------------------------------------------
{
   for( int i=0; i < Nb_Tracks; i++)                         // for each track
      if( Floppy_Write_Track( Journal_File, i,
                  Clusters_BAC [i * Clusters_Grouping]) == ABORTED)

     return ABORTED;

   if( Floppy_Flush_Files() == ABORTED)
      return ABORTED;

   return SUCCEEDED;
}
//</code></function>


//=============================================================================
/*[*/
/*
   Because we use absolute disk access for the journal files, we always write
   by clusters or even by tracks. The memory is organized to comply with that
   structure.
   To store the card we have allocate memory track by track length, so we are
   sure to have only one block of memory for each track. We use the track
   length because it gives better performance while writing to disk, a track
   correponds to a disk revolution. The memory is only used by track to create
   the files or to read the files not to write just a card.
   Within each track in memory we organize clusters simply by getting the
   adress of the corresponding point in the track. There is no gap at the
   end of the track, the track length used is a multiple of the cluster length
   even if on floppy the last cluster may be for a part on one track and for
   another to the next track. This is because the atomic unit on disk is the
   sector and not the track.
   Within each cluster we organize the cards buffers, these buffers contains
   the exact image of the card on floppy. The data vote is compacted to
   spare some place in memory and on file, and to win some time also.
   At the end of each cluster there may be a gap because the length used
   by each card in memory may not be a multiple of the cluster length.

   So when we want to drop a card in memory we must know its cluster number
   and its position in that cluster. Remember cluster 0 is used for the
   flag integrity.

   For the counter files we only use one cluster, not a whole track.

*/
/*]*/

/*<function id="Cluster_Position">
<heading>
  <description><en>
    convert a card position in a cluster position and in a position
    in cluster value</en></description>
  <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Card_Position</name>
      <description><en>the card position (2000 possibilities)</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Per_Cluster</name>
      <description><en>number of cards per cluster</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Card_Length</name>
      <description><en>the length used to store a card in
      memory = usage flag + data + mac</en></description>
    </parameter>
  </parameters>
  <usage>Cluster_Number = Cluster_Position( Card_Number);</usage>
  <comment>
   The cluster in which we found a particular card is simple to
   compute because there is Cards_Per_Cluster card in one cluster.
   The only thing to remember is that the first cluster (index 0) is
   used by the integrity flag, this is why we add 1 to the resulting
   number. This number is used to know which cluster to write when
   we want to save the card in the journal files (due to absolute
   disk access).
   The position in cluster gives us the offset where to write the
   card in memory, knowing the cluster number. The card position in
   memory will look like:
   Clusters_BAC [Cluster_Position(Card)][Position_In_Cluster(Card)]
   The card position can go from 0 to 1999 (2000 cards maximum),
   thus card 0 appears in cluster 1 at offset 0 in that cluster.</comment>
</heading>
<code>
*/
// ---------------------------------------------------------------------------
static int Cluster_Position ( int Card_Position)
// ---------------------------------------------------------------------------
{
   // card 0 is in cluster 0 + 1 (cluster 0 is integrity flag)
   return ((int)( Card_Position / Cards_Per_Cluster) + 1);
}
//</code></function>


/*<function id="Position_In_Cluster">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Card_Position</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int Position_In_Cluster ( int Card_Position)
// ---------------------------------------------------------------------------
{
   return (int)( Card_Position % Cards_Per_Cluster) * Journal_Card_Length;
}
//</code></function>


/*<function id="Initialize_Memory_BAC">
<heading>
  <description><en>
    initialize the memory used to store the b003 file contents</en></description>
  <parameters>
    <parameter type="return"><datatype scope="static">void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the memory used to store the b003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Card_Length</name>
      <description><en>the length used to store a card in
      memory = usage flag + data + mac</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Clusters</name>
      <description><en>the total number of clusters used by the b003 file</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description>
    </parameter>
  </parameters>
  <usage>Initialize_Memory_BAC();</usage>
  <comment>
   the Clusters_BAC initialisation could be more simple, but we do
   it this way to old get some visual control on the journal files.
   The first cluster, used for the integrity flag is totaly set
   with 0. All the other cluster are set to 1, this are the clusters
   used for the card. In each cluster we reset the usage flag of
   each card to 0. So by making an hex dump of the b003 file you
   can clearly see where are all the cards. The usage flags are
   positionned each Journal_Card_Length position in the cluster.
   At the end of each cluster you will normally see a card that
   seems to be longer, but this is due to the gap at the end of each
   cluster.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Initialize_Memory_BAC ( void)
// ---------------------------------------------------------------------------
{
   memset( Clusters_BAC [0], 0, Cluster_Length); // the flag integrity cluster

   for( int i=1; i < Nb_Clusters; i++)           // for all the cards clusters
   {
      memset( Clusters_BAC [i], 1, Cluster_Length);

      for( int j=0; j < Cards_Per_Cluster; j++)             // the usage flags
            Clusters_BAC [i][j * Journal_Card_Length] = 0;
   }
}
//</code></function>


/*<function id="Allocate_Memory_BAC" confidential="yes">
<heading>
  <description><en>
    allocates the memory used to store the counter and card files image</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>struct ElectionMain</datatype><name>Root</name>
      <description><en>the election data structure</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>length of the vote data on buffer</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Concat_Length</name>
      <description><en>length of the vote data in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Memory_Card_Length</name>
      <description><en>length of the vote + mac in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Journal_Card_Length</name>
      <description><en>length of the vote + mac + usage flag</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Per_Cluster</name>
      <description><en>number of cards in one cluster</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Clusters</name>
      <description><en>total number of clusters used by B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Tracks</name>
      <description><en>total number of tracks used by B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>memory to hold the B003 image</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Clusters_BAC_Partial</name>
      <description><en>memory to hold cluster from B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Cluster_Counter</name>
      <description><en>memory to hold the B005 image</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description>
    </parameter>
    <parameter type="global">
      <datatype>unsigned short </datatype><name>Clusters_Grouping</name>
      <description><en>number of clusters per track</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks>memory allocation</checks>
  <comment>
   This is one of the most important function of the program.
   The function first determines the election structure, this gives
   us the normal total length of a vote on the card (only the datas)
   Then we concat the vote length, to get the length use by the
   vote data in memory, aligned on a multiple of AESBLOCKLEN.
   With that length we compute the length with the mac (+AESMACLEN) and
   with the usage flag (+1). This gives us the total length of a
   card in memory and on floppy.
   Now we have to determine our memory structure to catch the most
   efficient floppy performance. The cards will be grouped per
   clusters and the clusters by track.
   The first cluster in the B003 file is used by the flag integrity,
   all the others are used for the cards. Knowing the total number
   of clusters needed by the file we compute the total number of
   tracks and we then correct the number of clusters.
   The memory is allocated track by track to have contiguous memory
   blocks.
   The counter file uses one cluster, not a whole track.
   The Clusters_BAC_Partial cluster is used to put the image of
   a Clusters_BAC cluster in.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Allocate_Memory_BAC ( void)
// ---------------------------------------------------------------------------
{
   int Concat_Length = 0;

   ///////////////////////////////////////
   // determine the vote data size on card

   for( ElectionList Link = Root.FirstElection; Link != NULL; Link = Link->Next)
   {
      switch( Link->Type)
      {
      case 0 :  Data_Length += sizeof( VOTE_TYPE_0);     break;
      case 1 :  Data_Length += sizeof( VOTE_TYPE_1);     break;
      }
   }

   Data_Length ++; // NEW add type of elector

   /////////////////////////////////////////
   // determine the vote data size in memory

   Concat_Length  = (Data_Length + 1) / 2;
   Concat_Length += (( Concat_Length % AESBLOCKLEN) == 0 ? 0 : AESBLOCKLEN - ( Concat_Length % AESBLOCKLEN));

   ///////////////////////////////////////////////////////
   // determine the card data length in memory and on file

   Memory_Card_Length  = Concat_Length + AESMACLEN;          // Add MAC-size
   Journal_Card_Length = 1 + Memory_Card_Length;             // Add Usage flag

   /////////////////////////////////
   // number of cards in one cluster

   Cards_Per_Cluster = (int)( Cluster_Length / Journal_Card_Length);

   ///////////////////////////////////////////////////////
   // total number of clusters and tracks used by the B003

   Nb_Clusters  = 1;                                     // for flag integrity

   Nb_Clusters += (int)(( MAX_CARDS_IN_BAC + Cards_Per_Cluster - 1)
              / Cards_Per_Cluster);
   Nb_Tracks    = (int)( Nb_Clusters + Clusters_Grouping - 1) / Clusters_Grouping;

   Nb_Clusters  = Nb_Tracks * Clusters_Grouping;                 // correction

/*
            //dump logging data
            sprintf( dbgtxt, " Data_Length:[%d]\n", Data_Length); dbg();
            sprintf( dbgtxt, " Concat_Length:[%d]\n", Concat_Length); dbg();
            sprintf( dbgtxt, " Memory_Card_Length:[%d]\n", Memory_Card_Length); dbg();
            sprintf( dbgtxt, " Journal_Card_Length:[%d]\n", Journal_Card_Length); dbg();

            sprintf( dbgtxt, " Cluster_Length:[%d]\n", Cluster_Length); dbg();
            sprintf( dbgtxt, " Cards_Per_Cluster:[%d]\n", Cards_Per_Cluster); dbg();
            sprintf( dbgtxt, " Clusters_Grouping:[%d]\n", Clusters_Grouping); dbg();

            sprintf( dbgtxt, " Nb_Clusters:[%d]\n", Nb_Clusters); dbg();
            sprintf( dbgtxt, " Nb_Tracks:[%d]\n", Nb_Tracks); dbg();
            sprintf( dbgtxt, "close"); dbg(); //close log
*/

   ////////////////////////////////////
   // allocate the memory for the cards

   if( (Clusters_BAC = (char **) malloc( Nb_Clusters * sizeof(char *))) == NULL)
      Error_Exit( E_OUT_OF_MEMORY);

   int i, j;
   for( i=0; i < Nb_Tracks; i++)         // allocate the memory track by track
   {
      if( (Clusters_BAC [i * Clusters_Grouping]
          = (char *) malloc( Clusters_Grouping * Cluster_Length)) == NULL)
     Error_Exit( E_OUT_OF_MEMORY);

      // determine each cluster adress in memory

      for( j=0; j < Clusters_Grouping; j++)
     Clusters_BAC [(i * Clusters_Grouping) + j]
        = & Clusters_BAC [i * Clusters_Grouping][j * Cluster_Length];
   }

   if( (Cluster_Counter = (char *) malloc( Cluster_Length)) == NULL)
      Error_Exit( E_OUT_OF_MEMORY);
   memset( Cluster_Counter, 0, Cluster_Length);

   if( (Clusters_BAC_Partial = (char *) malloc( Cluster_Length)) == NULL)
      Error_Exit( E_OUT_OF_MEMORY);
   memset( Clusters_BAC_Partial, 0, Cluster_Length);

   randomize();                          // initialize random number generator
}
//</code></function>


/*<function id="Dump_Memory_Card">
<heading>
  <description><en>
    writes the cluster corresponding to the card's position in
    the journal file</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>result of Floppy_Write_Cluster</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>Journal_File_Chain</name>
      <description><en>the journal's cluster chain</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Index</name>
      <description><en>the card's position</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Clusters_BAC_Partial</name>
      <description><en>updated copy of the cluster where the card was dropped</en></description>
    </parameter>
  </parameters>
  <usage>if( Dump_Memory_Card( Journal_Before_Chain, Card) == ABORTED)</usage>
  <comment>the function uses absolute disk access, so it writes a whole
   cluster.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Dump_Memory_Card ( FILE_CHAIN Journal_File_Chain, int Index)
// ---------------------------------------------------------------------------
{
   int Position = Cluster_Position( Index);

   return Floppy_Write_Cluster( Journal_File_Chain, Position, Clusters_BAC_Partial);
}
//</code></function>


/*<function id="Set_Data_Error">
<heading>
  <description><en>
    displays a message indicating the data is corrupted and goes in
    corrupted mode</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>always ABORTED because it is called in case of error</en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
  <usage>return Set_Data_Error();</usage>
  <comment>This function replace a code like:  if ( Error) {
                                                Display_Message(..);
                                                Corrupt_Integrity_State();
                                                return ABORTED;
                                             }</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Set_Data_Error ( void)
// ---------------------------------------------------------------------------
{
   Display_Message( M_DATA_CORRUPTED, 1, 0);
   Corrupt_Integrity_State();
   return ABORTED;
}
//</code></function>


/*<function id="Restore_Memory_BAC" confidential="yes">
<heading>
  <description><en>
    reload all the B003 or B013 file in memory</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = no error,
      ABORTED   = error while restoring file.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Journal_File</name>
      <description><en>the journal file handle to restore from</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Clusters</name>
      <description><en>total number of clusters used by B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Nb_Tracks</name>
      <description><en>total number of tracks used by B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>memory to hold the B003 image</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description>
    </parameter>
    <parameter type="global">
      <datatype>unsigned short</datatype><name>Clusters_Grouping</name>
      <description><en>number of clusters per track</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_In_BAC</name>
      <description><en>number of cards in the BAC</en></description>
    </parameter>
  </parameters>
  <usage>if( Restore_Memory_BAC( Journal_Before) == ABORTED)</usage>
  <checks>Each floppy function is checked.
   The file length is also checked, because we know the number of
   tracks we can determine the file length
   Finaly we verify the file integrity by checking the MAC</checks>
  <comment>
   This function reads a journal file track by track. We have
   already allocated the Clusters_BAC memory, so we know the number
   of tracks and each track adress in memory. It is only called
   after an incident.
   If there is an error while reading the file or if the flag
   integrity is set to CORRUPTED in the file we go in CORRUPTED
   mode.
   Once all the file is read and in memory, we go through the
   memory to count and to verify all the card that were already
   stored in the B003 file.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Restore_Memory_BAC ( int Journal_File)
// ---------------------------------------------------------------------------
{
    int     i;
    int     Position;
    int     Offset;
    long    File_Length;

    // ensure file has the good size
    if( Floppy_File_Length( Journal_File, & File_Length) == ABORTED)
        return Set_Data_Error();

    if( File_Length != ((long)Nb_Clusters * (long)Cluster_Length))
        return Set_Data_Error();

    if( Read_Flag_Integrity( Journal_File) == ABORTED)
        return Set_Data_Error();

    for( i = 0; i < Nb_Tracks; i++ )
        if( Floppy_Read_Track( Journal_File, i, Clusters_BAC [i * Clusters_Grouping]) == ABORTED)
            return Set_Data_Error();

    /////////////////////////////////////////
    // count and verifies the cards in memory

    for( i=0; i < MAX_CARDS_IN_BAC; i++)
    {
        Position = Cluster_Position( i);
        Offset   = Position_In_Cluster( i);

        if( Clusters_BAC [Position][Offset] != 0)
        {
            ++ TAB_C_InBAC[Totaux];       //STEF:   // NEW increment counter Totaux

            if( Verify_MAC_On_Vote( & (Clusters_BAC [Position][Offset]) + 1, Memory_Card_Length) == ABORTED)
                return Set_Data_Error();

       //***** NEW apply to each cards in B003
            char Vote_temp [MAX_VOTE_BUF];
            char Vote_dummy[MAX_VOTE_BUF];

            Decrypt_Vote(&(Clusters_BAC [Position][Offset]) + 1,Vote_temp, Memory_Card_Length - 8);
            Deconcat_Data( Vote_temp, Vote_dummy);

            ++ TAB_C_InBAC[ (Vote_dummy[Data_Length-1]-0x30+1) ]; //stefG: increment counter
       //***** END NEW
        }
    }
    return SUCCEEDED;
}
//</code></function>


/*<function id="Drop_In_Memory_BAC" confidential="yes">
<heading>
  <description><en>
    return the random position where the card was dropped in
    memory</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>the position</en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Vote_Data [200]</name>
      <description><en>the vote as it is on BUFFER</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Concat_Vote [200]</name>
      <description><en>the vote concatened</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Vote_Encrypted [200]</name>
      <description><en>the concatened vote encrypted</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Memory_Card_Length</name>
      <description><en>length of the vote + mac in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>memory to hold the B003 image</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Clusters_BAC_Partial</name>
      <description><en>memory to hold cluster from B003</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cluster_Length</name>
      <description><en>the cluster length</en></description>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>is program CORRUPTED ?</en></description>
    </parameter>
  </parameters>
  <usage>Position = Drop_In_Memory_BAC();</usage>
  <comment>
   The function first searches a free position in the memory where
   it can drop the card. The start begins at a random position in
   memory so it is impossible to deduct the origin of the card.
   Once we have found a position we concat the data from the card,
   we encrypt it, we store it in memory and we set the usage flag.
   The card is not dropped in the real memory. We first copy the
   cluster from the real memory in Clusters_BAC_Partial and we
   drop the card in that memory. Clusters_BAC will be updated
   later when the card is in the BAC. This is to prevent
   a side effect of the Switch_To_Backup function. This function
   will use the memory that corresponds exactly to the cards
   already in the BAC not the cards already treated.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int Drop_In_Memory_BAC ( void)
// ---------------------------------------------------------------------------
{
      int Index;
      int Started;

   if ( Flag_Integrity == CORRUPTED)
      return 0;

   ResetPrng();
   Index = rand() % MAX_CARDS_IN_BAC;
   Started = Index;

    int Position = Cluster_Position( Index);
    int Offset   = Position_In_Cluster( Index);

    while( Clusters_BAC [Position][Offset] != 0)
    {
        if( Index + 1 < MAX_CARDS_IN_BAC)
            ++Index;
        else
        {
            Index = 0;
            Position = Cluster_Position( Index);
            Offset   = Position_In_Cluster( Index);

            while( Clusters_BAC [Position][Offset] != 0)
            {
                if( Index < Started)
                    ++Index;

                else
                {
                    Display_Message( M_TOO_MANY_CARDS, 1, 0);
                    Corrupt_Integrity_State();
                    return 0;
                }
                Position = Cluster_Position( Index);
                Offset   = Position_In_Cluster( Index);
            }
            break;
        }
        Position = Cluster_Position( Index);
        Offset   = Position_In_Cluster( Index);
    }

    Concat_Data( Vote_Data, Concat_Vote);
    Encrypt_Vote( Concat_Vote, Vote_Encrypted, (Memory_Card_Length - AESMACLEN));

    memcpy( Clusters_BAC_Partial, Clusters_BAC [Position], Cluster_Length);
    memcpy( &(Clusters_BAC_Partial [Offset]) + 1, Vote_Encrypted, Memory_Card_Length);

    Clusters_BAC_Partial [Offset] = 1;

    return Index;
}
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Card_Is_Blanco" confidential="yes">
<heading>
  <description>
    <en>detects if a card contains the blanco vote that is written on
    the card to initialize it</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = Card is blanco,
      FALSE = Card is not blanco.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>Select_Reader*</datatype><name>Reader</name>
      <description><en>the reader where to find the card</en></description>
      <defaultValue/>
    </parameter>
  </parameters>
  <usage>if( Card_Is_Blanco( Intern_Reader))</usage>
  <comment>
   this function detects if a card correponds to a just initialized
   card, we initialise a card by writting a blanco vote on the card
   and the usage flag from the card is not set.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Card_Is_Blanco ( Select_Reader * Reader)
// ---------------------------------------------------------------------------
{
      char        Card_Buffer[MAX_CARD_BUF+1];
      extern char Blanco_Card[];

   if ( Reader_Read_Card( Reader, Card_Buffer) != HOME)
      return FALSE;

   // 20040309 avn - usage flag moved to first college, thus -1 otherwise the
   // electortype will trigger an already voted message...
   if ( strncmp( Blanco_Card, Card_Buffer, PositionCardTypeElector+8 -1))
      return FALSE;

   return TRUE;
}
//</code></function>


/*<function id="Card_Is_Formatted" confidential="yes">
<heading>
  <description><en>
    determines if the card was formatted by the URN</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = Card is formatted,
      FALSE = Card is not formatted.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>Select_Reader*</datatype><name>Reader</name>
      <description><en>the reader where to find the card</en></description>
      <defaultValue/>
    </parameter>
  </parameters>
  <usage>if( Card_Is_Formatted( Intern_Reader))</usage>
  <comment>
   a card is said formatted if it can be inserted in the BAC, this
   function is used to avoid a card to be reformatted because the
   formatted cards counter will be false. So it is impossible to
   erase a voted card, even a blanco card.
   rvh: No retry here (Reader_Physical_Read_Card) speed is important</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Card_Is_Formatted ( Select_Reader * Reader)
// ---------------------------------------------------------------------------
{
        char Card_Buffer[ MAX_CARD_BUF+1];
        char cDummy[ MAX_VOTE_BUF];
        int iDummy;

    if ( Reader_Read_Card( Reader, Card_Buffer) != HOME)
       return FALSE;

    if ( Split_Card_Buffer( Card_Buffer, & iDummy, cDummy) == ABORTED)
       return FALSE;

    return TRUE;
}
//</code></function>


/*<function id="Card_Is_Initialized">
<heading>
  <description><en>
    avoid any card to be initialized twice</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = Card is initialized,
      FALSE = Card is not initialized.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>Select_Reader*</datatype><name>Reader</name>
      <description><en>the reader where to find the card</en></description>
      <defaultValue/>
    </parameter>
  </parameters>
  <usage>if( Card_Is_Initialized( Intern_Reader))</usage>
  <comment>
   this function detects if a card is formatted to prevent it from
   being reformated.
   If the card is formatted we inform the user and we also indicate
   if the card is voted or only formatted.</comment>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
BOOL Card_Is_Initialized ( Select_Reader * Reader)
// ---------------------------------------------------------------------------
{
    #ifndef _DEMO
    if( Card_Is_Formatted( Reader))
    {
        if( Card_Is_Blanco( Reader))
            Error_Message( M_FORMATTED);
        else
            Error_Message( M_VOTED);

        sleep(2);
        Reader_Eject_Card( Reader);
        Reader_Wait_Card_Removed( Reader);

        return TRUE;
    }
    #endif

    return FALSE;
}
//</code></function>


/*<function id="Process_Card_Formatting">
<heading>
  <description>
    <en>process the card in the intern reader for initialisation with a
    blanco vote</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Blanco_Card[]</name>
      <description><en>the data to initialize a card</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>MsgElect</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>IndexCountElector</datatype><name>ielect</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>MODE_DISP</datatype><name>modedisp</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Intern_Reader</name>
      <description><en>the intern reader</en></description>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Formatted</name>
      <description><en>number of cards formatted</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_Really_Formatted</name>
      <description><en></en></description>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Counter_Before_Chain</name>
      <description><en>the B005 cluster chain</en></description>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Counter_After_Chain</name>
      <description><en>the B015 cluster chain</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>the rollback point type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_BEFORE_FORMATTED</name>
      <description><en>the rollback points</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_AFTER_FORMATTED</name>
      <description><en></en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks>In case of error the function will rollback to the point that
   causes the error after BACKUP installation
   Only not initialized and not voted cards are accepted
   In CORRUPTED mode the card is only initialized</checks>
  <comment>
   Before we begin to format the card with the blanco vote, we
   update the Cards_Formatted but not the Cards_Really_Formatted
   So if there is an error and the switch to backup is activated,
   the written value will not be the updated one.

   The logical is:
   - update Cards_Formatted
   - write the before file
   - initialize the card
   - update Cards_Really_Formatted
   - write the after file
   - eject the card

   The eject is at the end of the routine because ejecting it
   before is not safe enough. Ejecting the card only means that the
   card can be taken. If the card is still at the card gate when
   rebooting it will be taken into the reader by initialize, and
   nothing can guarantee that this card is the one we have ejected.
   So when restarting and if the B005 and B015 files are different
   you will always restore the B015 and that is not correct.

   The eject only ejects the card, it does'nt make a disable. This
   is to permit a new card entry as soon as possible.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Process_Card_Formatting ( char Blanco_Card[],
                                  int ielect )
// ---------------------------------------------------------------------------
{

    if( Card_Is_Initialized( Intern_Reader))
    {
        if( CurrentTypeElector == 1 )
            ScreenAllCounters();
        else
            ScreenOneCounter();

        return ABORTED;
    }


    TAB_CR_Formatted[Totaux] = TAB_C_Formatted[Totaux];
    ++TAB_C_Formatted[Totaux];

    TAB_CR_Formatted[ielect] = TAB_C_Formatted[ielect];
    ++TAB_C_Formatted[ielect];

    Rollback_Point = RBP_BEFORE_FORMATTED;
        setjmp( JMP_BEFORE_FORMATTED);

    if( Flag_Integrity != CORRUPTED)
        Dump_Abs_Counter( Counter_Before_Chain );

    if( Reader_Write_Card( Intern_Reader, Blanco_Card) != HOME )
    {
        --TAB_C_Formatted[Totaux];
        --TAB_C_Formatted[ielect];

        if( Flag_Integrity != CORRUPTED )
            Dump_Abs_Counter( Counter_Before_Chain);

        Reader_Eject_Card( Intern_Reader);
        Error_Message( M_ERR_CARD1);
        sleep(5);
        Reader_Wait_Card_Removed( Intern_Reader);

        if( CurrentTypeElector == 1 )
            ScreenAllCounters();
        else
            ScreenOneCounter();

        return ABORTED;
    }

    if( Flag_Integrity == CORRUPTED )
    {
        Reader_Only_Eject_Card( Intern_Reader);
        delay(2500);
        if( CurrentTypeElector == 1 )
            ScreenAllCounters();

        return SUCCEEDED;
    }

    TAB_CR_Formatted[Totaux]= TAB_C_Formatted[Totaux];
    if( NbrOfCounters != 0 )
    {
        ScreenOneCounter();
        delay(2500);
    }

    ScreenAllCounters();
    TAB_CR_Formatted[ielect]= TAB_C_Formatted[ielect];

    Rollback_Point = RBP_AFTER_FORMATTED;
        setjmp( JMP_AFTER_FORMATTED);
    Dump_Abs_Counter( Counter_After_Chain);

    Reader_Only_Eject_Card( Intern_Reader);

    return SUCCEEDED;
}
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reset_Counters">
<heading>
  <description><en>
    Initialize Counters for Cards Formatted(TAB_C_Formatted),
    Cards Really Formatted(TAB_CR_Formatted), Cards In BAC(TAB_C_InBAC).</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Reset_Counters()</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Reset_Counters ( void)
// ---------------------------------------------------------------------------
{
    int i;
    for( i = 0; i < MAX_TYPE_ELECTOR; i++ )
    {
        TAB_C_Formatted[ i] = 0;
        TAB_CR_Formatted[ i] = 0;
        TAB_C_InBAC[ i] = 0;
    }
}
//</code></function>


/*<function id="Get_BACcounter">
<heading>
  <description>
    <en>Retrieve a specifiec counter from TAB_C_InBAC ( Cards in BAC).</en></description>
  <parameters>
  </parameters>
<usage>Get_BACcounter( x)</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Get_BACcounter ( int CounterType) //JRU: get a counter of cards in BAC
// ---------------------------------------------------------------------------
{
   if ( CounterType == CardsWithNoTicket)
   {
      // :KLUDGE: avn 20030325 - Not needed if usage flag was in the memory BAC...
      return TAB_C_Formatted[CardsWithNoTicket];
   }

   return( TAB_C_InBAC[CounterType]);
}
//</code></function>

/*<function id="Process_Card_In_BAC">
<heading>
  <description>
    <en>Wrapper function for Process_Card_Fast</en>
  </description>
  <parameters/>
  <usage />
  <checks />
  <comment />
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Process_Card_In_BAC ( char Card_Buffer [])
// ---------------------------------------------------------------------------
{
   while( Floppy_Request_Disk() == ABORTED); // floppy available?

   SetRetryModeOff(); // disable retry's in card processing
   Process_Card_Fast( Card_Buffer);
   SetRetryModeOn();
}
//</code></function>


/*<function id="Go_Corrupt">
<heading>
  <description>
    <en>Corrupts system integrity and updates the counters display.</en>
  </description>
  <parameters/>
  <usage />
  <checks />
  <comment />
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Go_Corrupt( void)
// ---------------------------------------------------------------------------
{
   Corrupt_Integrity_State();
   if ( CurrentTypeElector == 1) ScreenAllCounters();
   else ScreenOneCounter();

   return;
}
//</code></function>

/*<function id="Process_Card_Fast" confidential="yes">
<heading>
  <description><en>
    process the vote written on a card, check the validity, store
    the vote in memory and on floppy.</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Card_Buffer []</name>
      <description><en>the complete card data</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Version</name>
      <description><en>DV I or II</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Vote_Data [200]</name>
      <description><en>the decrypted vote</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Cards_In_BAC</name>
      <description><en>number of cards in the BAC</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Extern_Reader</name>
      <description><en>the extern reader</en></description>
    </parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>program CORRUPTED ?</en></description>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Journal_Before_Chain</name>
      <description><en>the B003 cluster chain</en></description>
    </parameter>
    <parameter type="global">
      <datatype>FILE_CHAIN</datatype><name>Journal_After_Chain</name>
      <description><en>the B013 cluster chain</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>the rollback point type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_BEFORE_IN_BAC</name>
      <description><en>the rollback points</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_AFTER_IN_BAC</name>
      <description><en></en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the image of B003 in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>a cluster of B003</name>
      <description><en></en></description>
    </parameter>
    <parameter type="global">
      <datatype>BOOL</datatype><name>Background_URN</name>
      <description><en>Background_URN</en></description>
    </parameter>
  </parameters>
  <usage>Process_Card_Fast( Card_Buffer);</usage>
  <checks>Maximum number of cards limit, if it is reached we go in CORRUPTED mode.</checks>
  <comment>We first decrypt and verify the vote validity on the card, before
   doing anything else.

   Then the function logical is:
   - find a place where to drop the card and drop it in a copy of
   the cluster in memory
   - reset image state
   - write that cluster in the before file
   - set image state broken
   - take the card and increment the counter
   - set image state consistent
   - update the after file
   - copy the cluster in the global cluster memory
   - reactivate the card acceptation</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Process_Card_Fast ( char Card_Buffer [])
// ---------------------------------------------------------------------------
{
      int iUsageFlag;     // 0=Unused, 1=Voted, 9=Canceled ( ticketing only)
      int iBadCard = 0;

   // exceeding maximum number of cards by accepting this card?
   if ( ( TAB_C_InBAC[Totaux] + 1) > MAX_CARDS_IN_BAC)
   {
      if ( Flag_Integrity == CONSISTENT)       // prevents repeating the message
      {
         Display_Message( M_TOO_MANY_CARDS, 1);
         Go_Corrupt();
         return;
      }
   }

   // 20040106 avn - acces to special election of 2004 is not allowed
   // unless the elector votes for a party of the dutch languagegroup at the
   // Conseil Regional Bruxelles-Capitale
   AllowSpecialElection( FALSE);

   // reject cards with inconsistent content
   if ( Split_Card_Buffer( Card_Buffer, & iUsageFlag, Vote_Data) == ABORTED ||
      Count_Vote( CHECK_IT) == ABORTED)       // Integrity Test
   {
      Error_Message( M_INVALID_CARD);
      iBadCard = 1;
   }

   // reject the cards where the elector has canceled his ticket ( flag = 9)
   if ( iUsageFlag > 1)
   {
      // 20040816 avn
      if ( Root.iBureauWithTicketing)
      {
         //Error_Message( M_CANCELED_CARD); // card canceled by elector
         Error_Message( M_INVALID_CARD);
      }
      else
      {
         Error_Message( M_INVALID_CARD);
      }
      iBadCard = 1;
   }

   if ( iBadCard)
   {
      sleep( 5);
      Reader_Eject_Card( Extern_Reader);
      Reader_Wait_Card_Removed( Extern_Reader);
      Reader_Formatted_Card( Extern_Reader);

      if ( CurrentTypeElector == 1) ScreenAllCounters();
      else ScreenOneCounter();

      return;                                   // jump to COM1
   }

   // no more registration, just taking cards in corrupted mode
   if ( Flag_Integrity == CORRUPTED)
   {
      // critical moment:card is dropped in BAC
      if ( CARD_ERROR == Reader_Take_Card( Extern_Reader))
         Error_Procedure( Extern_Reader);

      Reader_Formatted_Card( Extern_Reader);
      return;
   }

#if _TIMERMODE
   ChronoStart();
#endif

   // kick start floppy
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Status reset - start.");
      #endif // _TRACEMODE
   if ( SetImageStatus( IMAGE_STATE_RESET) != SUCCEEDED)
   {
      Go_Corrupt();
      return;
   }
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Status reset - done.");
      #endif // _TRACEMODE

#if _TIMERMODE
   gotoxy( 3, 1);
   printf( "Status [%d] - seconds: [%f].\n", 900, ChronoEllapsed());
   ChronoStart();
#endif

   // find a position and drop in memory
   int BAC_Index = Drop_In_Memory_BAC();
   #if _TRACEMODE
		sprintf( gcLogBuf, "PCIB - Parking found at [%d].", BAC_Index);
		Add_To_Logging( EMPTY, gcLogBuf);
   #endif // _TRACEMODE

   if ( Flag_Integrity == CORRUPTED)
      return;

   // update before images
   Rollback_Point = RBP_BEFORE_IN_BAC;
      setjmp( JMP_BEFORE_IN_BAC);

      // card contents
#if _TIMERMODE
   ChronoStart();
#endif

         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PCIB - Before image - start.");
         #endif // _TRACEMODE
      Dump_Memory_Card( Journal_Before_Chain, BAC_Index);
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PCIB - Before image - done.");
         #endif // _TRACEMODE

      // unused cards counter
      if ( Root.iBureauWithTicketing && 0 == iUsageFlag)
      {
         ++TAB_C_Formatted[CardsWithNoTicket];
         TAB_CR_Formatted[CardsWithNoTicket] = TAB_C_Formatted[CardsWithNoTicket];

            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PCIB - Before counter - start.");
            #endif // _TRACEMODE
         Dump_Abs_Counter( Counter_Before_Chain );
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PCIB - Before counter - done.");
            #endif // _TRACEMODE
      }

#if _TIMERMODE
   gotoxy( 1, 2);
   printf( "Before files - seconds: [%f].\n", ChronoEllapsed());
   ChronoStart();
#endif

   // image status broken
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Before status - start :-).");
      #endif // _TRACEMODE
   if ( SetImageStatus( IMAGE_STATE_BROKEN) != SUCCEEDED)
   {
      Go_Corrupt();
      return;
   }
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Before status - done.");
      #endif // _TRACEMODE

#if _TIMERMODE
   gotoxy( 1, 3);
   printf( "Status [%d] - seconds: [%f].\n", 910, ChronoEllapsed());
   ChronoStart();
#endif

   // critical moment:card is dropped in BAC
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Capture card - start.");
      #endif // _TRACEMODE
   if ( CARD_ERROR == Reader_Take_Card( Extern_Reader))
   {
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Capture aborted, clearing before image.");
      #endif // _TRACEMODE
      Clear_Entry_In_Journal( Journal_Before_Chain, BAC_Index);
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Capture aborted, clearing done.");
      #endif // _TRACEMODE

      // if we can eject the card succesfully there's no need to go into corrupted mode...
      Save_Reader_Reply();
      if ( HOME == Reader_Disable( Extern_Reader))
      {
         // reader has been disabled succesfully
         if ( SLOT == Reader_Eject_Card( Extern_Reader))
         {
            // card has been ejected and is at the gate
            if ( SetImageStatus( IMAGE_STATE_RESET) != SUCCEEDED)
            {
               // status is uncertain, so we must go corrupted
               Go_Corrupt();
            }
            else
            {
               // card has been ejected, status is reset, now we can throw the capture error
               //  safely without going corrupt during startup
               Restore_Reader_Reply();
            }
         }
      }

      // 20050928 avn - unable to capture card, issue an error state
      //  if the status has not been reset the system will go corrupt during startup...
      Error_Procedure( Extern_Reader);
   }
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Capture card - done.");
      #endif // _TRACEMODE

#if _TIMERMODE
   gotoxy( 1, 4);
   printf( "Capture - seconds: [%f].\n", ChronoEllapsed());
   ChronoStart();
#endif

   // image status consistent
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - After status - start.");
      #endif // _TRACEMODE
   if ( SetImageStatus( IMAGE_STATE_CONSISTENT) != SUCCEEDED)
   {
      Go_Corrupt();
      return;
   }
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - After status - done.");
      #endif // _TRACEMODE

#if _TIMERMODE
   gotoxy( 41, 1);
   printf( "Status [%d] - seconds: [%f].\n", 911, ChronoEllapsed());
   ChronoStart();
#endif

   // update after images
   Rollback_Point = RBP_AFTER_IN_BAC;
     setjmp( JMP_AFTER_IN_BAC);

      // card contents
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PCIB - After image - start.");
         #endif // _TRACEMODE
      Dump_Memory_Card( Journal_After_Chain, BAC_Index);
         #if _TRACEMODE
            Add_To_Logging( EMPTY, "PCIB - After image - done.");
         #endif // _TRACEMODE

      // unused cards counter
      if ( Root.iBureauWithTicketing && 0 == iUsageFlag)
      {
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PCIB - After counter - done.");
            #endif // _TRACEMODE
         Dump_Abs_Counter( Counter_After_Chain );
            #if _TRACEMODE
               Add_To_Logging( EMPTY, "PCIB - After counter - done.");
            #endif // _TRACEMODE
      }

#if _TIMERMODE
   gotoxy( 41, 2);
   printf( "After files - seconds: [%f].\n", ChronoEllapsed());
   ChronoStart();
#endif

   // update the global memory
   memcpy( Clusters_BAC [ Cluster_Position( BAC_Index) ],
      Clusters_BAC_Partial, Cluster_Length);

   ++TAB_C_InBAC[Totaux];
   ++TAB_C_InBAC[CurrentTypeElectorInBac];
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Updated counters...");
      #endif // _TRACEMODE

   if ( CurrentTypeElector == 1 )
      ScreenAllCounters();
   else
      ScreenOneCounter();
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Updated screen counters...");
      #endif // _TRACEMODE

#if _TIMERMODE
   gotoxy( 41, 3);
   printf( "Memory update - seconds: [%f].\n", ChronoEllapsed());
#endif

   // monitor votes counters periodically
   if ( CheckVotesCounters() == ABORTED)
   {
      // 20040916 avn - fatal error now, restart URN
      Add_To_Logging( ERR, Get_Text( FE_BAD_MEMORY, Working_Language( ASK)));
      Error_Exit( FE_BAD_MEMORY);
   }

   // open shutter as quick as possible ( enable reader)
   Reader_Formatted_Card( Extern_Reader);
      #if _TRACEMODE
         Add_To_Logging( EMPTY, "PCIB - Reader enabled.");
      #endif // _TRACEMODE

   return;
}
//</code></function>


/*<function id="Count_Votes" confidential="yes">
<heading>
  <description><en>
    counting of all the votes</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = counting valid,
        ABORTED   = counting is not valid</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Vote_Encrypted [200]</name>
      <description><en>the card as she is in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Concat_Vote [200]</name>
      <description><en>the vote data in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Vote_Data [200]</name>
      <description><en>the vote data as it was on buffer</en></description>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Memory_Card_Length</name>
      <description><en>length of the card in memory (+mac)</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char**</datatype><name>Clusters_BAC</name>
      <description><en>the memory allocated to store all the b003 file contents in memory</en></description>
    </parameter>
  </parameters>
  <usage>Count_Votes();</usage>
  <checks>the MAC is not checked because you can't have a corrupted card
   in memory.</checks>
  <comment>All the emitted votes are stored in memory, in fact we have the
   image of the b003 file in memory but under an other form, the
   card datas are compacted for memory storing.
   Before we can process each vote for counting we must restitute
   the form  it has on card. We extract each card from memory one
   by one, we decrypt the datas, we deconcat it and then we process
   the vote on it with the EXECUTE mode from Count_Vote().</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Count_Votes (
   void
   )
// ---------------------------------------------------------------------------
{
    int Position, Offset;

    for( int i=0; i < MAX_CARDS_IN_BAC; ++i )   // for each card position in mem
    {
        Position = Cluster_Position( i );
        Offset   = Position_In_Cluster( i );

        if( Clusters_BAC [ Position ][ Offset ] != 0 ) // for each used card position
        {
            // extract the card data+mac but not the usage flag  (first position)
            memcpy( Vote_Encrypted,
                    & (Clusters_BAC[ Position ][ Offset ] ) + 1,
                    Memory_Card_Length );

            Decrypt_Vote ( Vote_Encrypted,
                           Concat_Vote,
                           Memory_Card_Length - 8 );

            Deconcat_Data( Concat_Vote, Vote_Data);
            AllowSpecialElection( FALSE);
            if ( Count_Vote( EXECUTE) == ABORTED)
            {
               return ABORTED;
            }
        }
    } // eofor

    return SUCCEEDED;
}
//</code></function>


/*[*/
/*
   All the cards are saved in memory, but making this by copying the card
   contents as it is to memory will consume a lot a space. This is because
   we can only code '0' to '9' on the card, 10 values. In memory the
   smallest value is coded on 8 bytes, these are 255 values. So we can
   reduce the space consuming by grouping to card position into one memory
   position. To give an image 1 and 2 on card will become 12 in memory.
   For the mapping simplicity we do not try to reach the maximum compression
   possible, 10 values use 3 and a half bit but we will use 4.
   Depending on the original card length we will win a factor 2, but not
   always as the data once compacted must also have a length that is a AESBLOCKLEN
   multiple.
   IMPORTANT: '0'on card is used as 0 for memory (we always substract '0')
          0 in memory is '0' on card (we always add '0')

   Concat  : xxxxaaaa xxxxbbbb will become aaaabbbb
   Deconcat: aaaabbbb will become xxxxaaaa xxxxbbbb

*/
/*]*/

/*<function id="Concat_Data" confidential="yes">
<heading>
  <description><en>
    concat the card data for storing in memory</en></description>
  <parameters>
    <parameter type="return"><datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Card_Data []</name>
      <description><en>the card data to concat</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Concat_Data []</name>
      <description><en>the same data once concateneted</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>the data length on the magnetic card</en></description>
    </parameter>
  </parameters>
  <usage>Concat_Data( Vote_Data, Concat_Vote);</usage>
  <comment>If the data length is odd then there is no ending xxxxbbbb, the
   resulting value is the aaaa0000.
   First value in array corresponds to index 0, this is a little bit
   confusing of course, as odd values have even index.
   The concat data is aligned on a AESBLOCKLEN multiple with 0.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Concat_Data ( char Card_Data[], char Concat_Data[] )
// ---------------------------------------------------------------------------
{

    int Limit = ( Data_Length + 1 ) / 2;

    for( int i=0; i < Limit; i++)       // xxxxaaaa,xxxxbbbb will become aaaabbbb
    {
        Concat_Data[ i]  = ( Card_Data[ 2 * i] - '0') << 4;        // odd positions

        if( ( ( 2 * i) + 1) < Data_Length )
            Concat_Data[i] += ( Card_Data[ ( 2 * i) + 1] - '0');   // even positions
    }
                          // Align on a multiple of AESBLOCKLEN
    for( i = 0; i < (( Limit % AESBLOCKLEN ) == 0 ? 0 : AESBLOCKLEN - ( Limit % AESBLOCKLEN )); i++ )
        Concat_Data [Limit + i] = 0;
}
//</code></function>


/*<function id="Deconcat_Data" confidential="yes">
<heading>
  <description><en>
    deconcat the card data stored in memory to restore the card
    format so it can be used for the totals</en></description>
  <parameters>
    <parameter type="return"><datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Concat_Data []</name>
      <description><en>the concatenated data</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>Card_Data []</name>
      <description><en>the card data once deconcatenated</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>the data length on the magnetic card</en></description>
    </parameter>
  </parameters>
  <usage>Deconcat_Data( Concat_Vote, Vote_Data);</usage>
  <comment>If the data length is odd then there is no ending xxxxbbbb, the
   resulting value is the xxxxaaaa only.
   First value in array corresponds to index 0, this is a little bit
   confusing of course, as odd values have even index.
   The concat data is aligned on a AESBLOCKLEN multiple with 0.</comment>
</heading>
<code>*/

// ---------------------------------------------------------------------------
static void Deconcat_Data ( char Concat_Data[], char Card_Data[] )
// ---------------------------------------------------------------------------
{
    int Limit = ( Data_Length + 1 ) / 2;

    for( int i = 0; i < Limit; i++ )     // aaaabbbb will become 0000aaaa,0000bbbb
    {
        Card_Data [2*i] = ((Concat_Data [i] >> 4) & 15) + '0';  // aaaa0000 >> 4

        if( ((2*i) + 1) < Data_Length)
            Card_Data [(2*i) + 1] = (Concat_Data [i] & 15) + '0';     // 0000bbbb
    }
}
//</code></function>


/*<function id="Test_File_Exists">
<heading>
  <description>
    <en>verifies if a certain file (filename) exists</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>filename</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
  <usage>  if (Test_File_Exists(filename))
                         &lt;statements&gt;           // file exists
                 else
                         &lt;statements&gt;           // file doesn't exist</usage>
  <comment>
   External functions: Floppy_Open_File()  (from GENFLOP.CPP)
                       Floppy_Close_File() (        "       )</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL Test_File_Exists ( char* filename)
// ---------------------------------------------------------------------------
{
    int iFileHandle ;

    if( Floppy_Open_File( filename, &iFileHandle) == ABORTED ||
        iFileHandle == -1 )
    {
        if(errno == EACCES)
        {
            chmod(filename, S_IREAD | S_IWRITE);
            return Test_File_Exists(filename) ;
        }
        else
            return FALSE ;
    }
    else
    {
        Floppy_Close_File(iFileHandle) ;
        return TRUE ;
    }
}
//</code></function>


/*<function id="Remove_All">
<heading>
  <description><en>
    if journal-files an/or total-file exist on the floppy
    they will be removed</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
<usage> Remove_All()</usage>
<comment> External function   : Floppy_Remove_File(char *)
          Globals  :  TOTALS_FILE_NAME
                      JRN_BEFORE_BAC
                      JRN_AFTER_BAC
                      JRN_BEFORE_FORMATTED
                      JRN_AFTER_FORMATTED</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Remove_All ()
// ---------------------------------------------------------------------------
{
    if( Test_File_Exists( TOTALS_FILE_NAME))
        if( Floppy_Remove_File( TOTALS_FILE_NAME) == ABORTED)
            Error_Exit(M_ERR_DISK1) ;

    if( Test_File_Exists( JRN_BEFORE_BAC))
        if( Floppy_Remove_File( JRN_BEFORE_BAC) == ABORTED)
            Error_Exit( M_ERR_DISK1);

    if( Test_File_Exists( JRN_AFTER_BAC))
        if( Floppy_Remove_File( JRN_AFTER_BAC) == ABORTED)
            Error_Exit( M_ERR_DISK1);

    if( Test_File_Exists( JRN_BEFORE_FORMATTED))
        if( Floppy_Remove_File( JRN_BEFORE_FORMATTED) == ABORTED)
            Error_Exit( M_ERR_DISK1);

    if( Test_File_Exists( JRN_AFTER_FORMATTED))
        if( Floppy_Remove_File( JRN_AFTER_FORMATTED) == ABORTED)
          Error_Exit( M_ERR_DISK1);
}
//</code></function>


/*<function id="Clear_Entry_In_Journal">
<heading>
  <description><en>
    This function resets the valid flag of the record.
    It then writes the cluster corresponding to the card's position
    in the journal file.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>FILE_CHAIN</datatype><name>Journal_File_Chain</name>
      <description><en>the journal's cluster chain</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Index</name>
      <description><en>the card's position</en></description>
      <defaultValue/>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Clusters_BAC_Partial</name>
      <description><en>updated copy of the cluster where the card was dropped</en></description>
    </parameter>
  </parameters>
  <usage>Clear_Entry_In_Journal( Journal_Before_Chain, Card)</usage>
  <comment>
   rvh-ebe 22/03/94 (author: ebe)
   It uses absolute disk access, so it writes a whole cluster.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Clear_Entry_In_Journal ( FILE_CHAIN Journal_File_Chain, int Index)
// ---------------------------------------------------------------------------
{
    int Position = Cluster_Position( Index);
    int Offset = Position_In_Cluster( Index);

    if( Flag_Integrity != CORRUPTED)
    {
        Clusters_BAC_Partial[Offset] = 0;
        Floppy_Write_Cluster( Journal_File_Chain,
                              Position,
                              Clusters_BAC_Partial );
    }
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Clear_Image_Files">
<heading>
  <description>Wipe contents and remove image files</description>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Clear_Image_Files (
	void
	)
// ---------------------------------------------------------------------------
{
	    int i;
		int TAB_Temp [MAX_TYPE_ELECTOR];

	// erase card contents from memory
    Initialize_Memory_BAC();

    // copy empty memory bac to image files
    if ( Create_Journals_BAC() == ABORTED) return ABORTED;

	// remove files when cleared...
	if ( Floppy_Remove_File( JRN_BEFORE_BAC) == ABORTED) return ABORTED;
	if ( Floppy_Remove_File( JRN_AFTER_BAC) == ABORTED) return ABORTED;
	if ( RemoveStatusFile() == ABORTED) return ABORTED;

    // save counters first, then clear counter images
    for( i = 0; i < MAX_TYPE_ELECTOR; i++ )
    {
        TAB_Temp[ i] = TAB_CR_Formatted[ i];
        TAB_CR_Formatted[ i] = 0;
    }
    if ( Create_Journals_Formatted() == ABORTED) return ABORTED;
	if ( Floppy_Remove_File( JRN_BEFORE_FORMATTED) == ABORTED) return ABORTED;
	if ( Floppy_Remove_File( JRN_AFTER_FORMATTED) == ABORTED) return ABORTED;

    // restore value for display
    for( i = 0; i < MAX_TYPE_ELECTOR; i++ )
    {
        TAB_CR_Formatted[ i] = TAB_Temp[ i];
    }

	return SUCCEEDED;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

