/*H*/
//=============================================================================
/*

   File        : ETCMAIN.CPP
   Creation    : 17/01/93

   Project     : Bull DIGIVOTE
   Author      : B.S.S.

   Description : Election Test Card application logic for URN system

*/
//=============================================================================
/*h*/
/*R*/
//=============================================================================
/*

   Revision History
   ################

   01/06/93: revision history starting date
   05/08/93: When a card is already positioned in the card reader, a read on
             the external card reader is forced before the main loop starts,
             making the routine Reader_Read_Card() called in
             Process_Card_IN_BAC() work.
   11/10/93: the Supplemental_Copies function was added, and the Close_Election
             function was adapted to support multiple BACKUP disks.
   19/10/93: the 'Pause' key is now disabled, see function Int1C()
   25/10/93: the function Int1C is not used any more because we introduced
             a keyboard filter. See the urnint.cpp file.
             (Remark: the Int1C function was used to reset the pause flag
             regularly, but this was not efficient and did'nt even work with
             the ctrl-s case)
   18/11/93: there was a problem when the URN runs with the BACKUP, the
             Supplemental_Copies function did not work correctly (it asked
             the BACKUP1 disk once again), this is fixed now.
   07/03/94: For the transmission of X25 after the supplemental copies, not
             only the MASTER disk but also the BACKUP1 disk is useable.
   24/03/94: RVH function "Accept_Card_In_BAC( void)".
             Reader_Read_Card replaced by Reader_Physical_Read_Card
   13/04/94: RVH sleep() bijgevoegd v¢¢r sommige ....Remove_Card..
*/
//=============================================================================
/*r*/
/*#*/

//======================== I N C L U D E   F I L E S ==========================

#include <dos.h>
#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <graphics.h>
#include <setjmp.h>
#include <string.h>
#include <alloc.h>


#include "..\gen\gentype.h"
#include "..\gen\gencard.h"
#include "..\gen\genlang.h"
#include "..\gen\gendata.h"
#include "..\gen\genflop.h"
#include "..\gen\genvideo.h"
#include "..\gen\gencryp.h"

#include "..\urn\urnint.h"
//#include "..\urn\urnx25.h"
#include "..\urn\urnmes.h"
//#include "..\urn\urnmsg.h"
#include "..\urn\urnerro.h"
#include "..\urn\urnreco.h"

#define ETC_TITLE   "CARD TEST"
#define _APPLICATION_ETC

//============================ T Y P E D E F S ================================

typedef enum { CONTINUE, CLOSE } MENU_CHOICE;
typedef enum { X25, DISK } CLOSING_TYPE;

//=========================== C O N S T A N T S ===============================

const unsigned int LIMIT = 30000;

//============================= G L O B A L S =================================
int NbrOfCounters; /* number of counters the totals counter not included
                          default: only a counter with the totals */
SCREEN_TYPE Glob_Screen_Type;
BOOL End_Of_Program;
ELECTOR_TYPE ElectorTypes[] =
{
    {
        '\0',
        0,
        M_CARDS_READY_TOTALS,
        M_CARDS_IN_BAC_TOTALS,
        0
    },

    {
        '\0',
        M_ELECTOR_BELGIAN,
        M_CARDS_READY_BELGIAN,
        M_CARDS_IN_BAC,
        0
    },

    {
        'B',
        M_ELECTOR_BELGIAN_OUTSIDE,
        M_CARDS_READY_BELGIAN_OUTSIDE,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_BELGIAN_OUTSIDE
    },

    {
        'E',
        M_ELECTOR_EUROPEAN,
        M_CARDS_READY_EUROPEAN,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_EUROPEAN
    },

    {
        'N',
        M_ELECTOR_NON_BELGIAN,
        M_CARDS_READY_NON_BELGIAN,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_NON_BELGIAN
    },

    {
        'V',
        M_ELECTOR_VOEREN1,
        M_CARDS_READY_VOEREN1,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_VOEREN1
    },

    {
        'K',
        M_ELECTOR_VOEREN2,
        M_CARDS_READY_VOEREN2,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_VOEREN2
    },

    {
        'T',
        M_ELECTOR_VOEREN3,
        M_CARDS_READY_VOEREN3,
        M_CARDS_IN_BAC,
        M_SPECIAL_ELECT_VOEREN3
    }
};


ELECTOR_TYPE ElectorTypesUsed[17];
int Version;
int PermissionsUsed[MAX_TYPE_ELECTOR - 1][MAX_ELECTIONS -1];

int CodeSituation       = 0;
int NUMBER_TYPE_ELECTOR = 0;
int CurrentTypeElector  = Belgian; //Belgian=0

INTEGRITY_STATE Flag_Integrity  = CONSISTENT;
ERROR_HANDLING  Error_Handling  = ABORT;
ROLLBACK_POINT  Rollback_Point  = RBP_STARTUP;
STARTUP_MODE    Startup_Mode    = FIRST_TIME;
BOOL            Background_URN  = FALSE;

Select_Reader * Intern_Reader, * Extern_Reader;

void interrupt (* oldfunc)( ...);
unsigned char far * keyb_ptr;


/*[*/
/*
 The names of the disks must always begin with the drive letter + ':'
 followed by MASTER or BACKUPx, where x is the BACKUP order number.
 The names MASTER and BACKUP have a length of 6 characters, the names may
 change but the length must be 6.
 The maximum length for BACKUPx is 8 because this is the space that was
 reserved in the message array (no test is made !!)
 These lengths are VERY important !!!
*/
/*]*/

char   Floppy_MASTER[15] = "A:MASTER";
char   BACKUP_Label[]    = "A:BACKUP";
char   Floppy_BACKUP[15] = "A:BACKUP1";
char * Floppy_Current    = Floppy_MASTER;

//rvh
char Reader_Buffer [MAX_CARD_BUF];
char Blanco_Votes  [MAX_VOTE_BUF];
//rvh

#if 0
   // defined in gencryp!
BYTE Key_MASTER  [9] = "";
BYTE Key_SESSION [9] = "";
BYTE Key_SESSION_MAV[9];
#endif

ElectionMain Root;

MODE Mode = STARTUP;

jmp_buf JMP_STARTUP;
jmp_buf JMP_BEFORE_FORMATTED;
jmp_buf JMP_AFTER_FORMATTED;
jmp_buf JMP_BEFORE_IN_BAC;
jmp_buf JMP_AFTER_IN_BAC;
jmp_buf JMP_CLOSE_OFF;

CLOSING_TYPE Closing_Type;

void interrupt( * Old_Keyboard_Handler)(...);

// ================== EXTERNAL FUNCTIONS ==========================
extern void Remove_All_Files_EveryWhere(void) ;

// tmt
extern void ETDiskA( void);
extern void Draw_Button( int x_min, int y_min, int x_max, int y_max);
extern void Allocate_Memory_BAC( void);
extern void ETCard( void);
extern RET_CODE ETPassWord( void);
extern void ScrExit( char* );
extern int ETInitSET( char*, long );
extern void ETInitEnglish( void );
extern RET_CODE ETPrepare( void);

//====================== D U M M Y   V A R I A B L E S ========================

int MsgElector          = 0;
MODE_DISP modedisp;

//=================== E X T E R N A L   V A R I A B L E S =====================

extern int Cards_Formatted;
extern int Cards_In_BAC;
extern char Blanco_Card[];
// tmt
extern char* TOTALS_FILE_NAME;
extern int Totals_File;

//====================== L O C A L   F U N C T I O N S ========================

static void Format_Password_Card( void);
static void Setup_Readers( TYPE_READER Intern_Type, TYPE_READER Extern_Type);
static void Dialog_Start_MAV( void);
static MENU_CHOICE Dialog_Menu( void);
static BOOL Accept_Card_Formatting( void);
static BOOL Accept_Card_In_BAC( void);
static RET_CODE Supplemental_Copies( void);
static void Close_Election( void);

//=================== E X T E R N A L   F U N C T I O N S =====================

void Format_Password_Card();
void Count_Votes();
void Format_Blanco_Votes();
BOOL Accept_Keyboard_ESC();


//================================= B O D Y ===================================

/*{*/
//=============================================================================
//
// Function : Error_Exit
//
// Purpose  : locks the program and disables the readers when a fatal error
//            has occured.
//
// Return   : void
//
// Arg List : int M_Index     , the mnemonic of the message to display
//                              no default value
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            Select_Reader * Extern_Reader , the extern reader description
//
// Usage    : Error_Exit( E_ERROR);
//
// Checks   :
//
// Comment  : There is no real exist of the program, in fact the machine is
//            locked and the readers are disabled.
//            We used the low level functions without error checking to
//            disable the readers, because this could cause a loop otherwise.
//            When the code protected by the TRACE directive is active and if
//            the exit is caused by a card reader error; the last commands
//            send to the reader are send to the output (either printer or
//            screen)
//
//-----------------------------------------------------------------------------
/*}*/

void Error_Exit( int M_Index)
{
#ifdef _APPLICATION_ETC
   ScrExit("ERROR DATABASE FILE (PROBABLY WRONG PASSWORD)");
#endif

#ifndef _RECOUNT
   Send_Command( Intern_Reader, DISABLE, NULL);
   Close_Command( & Intern_Reader);
#endif

   Send_Command( Extern_Reader, DISABLE, NULL);
   Close_Command( & Extern_Reader);

   Error_Message( M_Index);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef TRACE
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

   sleep(5);

   if( M_Index == M_BAD_READER1     || M_Index == M_BAD_READER2     ||
       M_Index == M_INTERNAL_ERROR1 || M_Index == M_INTERNAL_ERROR2 ||
       M_Index == M_ERR_READ1       || M_Index == M_ERR_READ2)

      Print_Trace_Buffer( stdout);

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

   setvect( 0x09, Old_Keyboard_Handler); // restore the default keyboard
                                         // interrupt handler

   for(;;);    // infinite loop, simulates machine lock
}

//==============================================================================
//======================= S E T U P - R E A D E R S ============================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : Setup_Readers
//
// Purpose  : initialize the card readers
//
// Return   : static void  , local function
//
// Arg List : TYPE_READER Intern_Type  , intern reader type number
//                                       no default value
//            TYPE_READER Extern_Type  , extern reader type number
//                                       no default value
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            Select_Reader * Extern_Reader , the extern reader description
//
// Usage    : Setup_Readers( Intern_Type, Extern_Type);
//
// Checks   : on reader initialisation errors
//
// Comment  :
//
//-----------------------------------------------------------------------------
/*}*/

#pragma argsused
static void Setup_Readers( TYPE_READER Intern_Type, TYPE_READER Extern_Type)
{
#ifndef _RECOUNT
   if(( Intern_Reader = Open_Command( Intern_Type, COM2)) == NULL)
      Error_Exit( M_ERR_READ1);
#endif

   if(( Extern_Reader = Open_Command( Extern_Type, COM1)) == NULL)
      Error_Exit( M_ERR_READ2);
}

//==============================================================================
//=========================== D I A L O G -  x x x =============================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : Dialog_Start_MAV
//
// Purpose  : dialog to process the remove of the floppy to start the MAV
//
// Return   : static void  , local function
//
// Arg List : void
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            STARTUP_MODE    Startup_Mode  , first time startup ??
//
// Usage    : Dialod_Start_MAV();
//
// Checks   :
//
// Comment  : If the URN is in RESUME mode, it may mean that only the URN had
//            a problem, the MAVs may still function, so we ask the president
//            if he wants to restart the MAVs in that case.
//            If the MAVs must be started, we format a card with the password
//            and the language code used by the president. The MAV has no
//            keyboard, so it is the only way to transmit the password to the
//            MAV. Once this is done we wait until the password card and the
//            floppy disk are removed. After that the president is supposed to
//            start the MAVs and the URN waits for the floppy to come back.
//            When the floppy is back, we ask the password once again to
//            prevent that someone else than the president starts the voting.
//
//-----------------------------------------------------------------------------
/*}*/

#ifndef _RECOUNT
static void Dialog_Start_MAV( void)
{
   if( Startup_Mode == RESUME && ! Ask_Question( M_RESTART_MAV) )
   {
      Display_Message( M_WAIT);                 // Don't want to restart MAV
      return;
   }

   Format_Password_Card();   // make the card with the password and language

   Display_Message( M_MAV_START);
   Reader_Wait_Card_Removed( Intern_Reader);   // Wait until card is removed
   Wait_Until_Floppy_Removed();                // Wait until disk is removed

   sleep(60);                                  //EBE + RVH

   Display_Message( M_URN_START);           // Wait until the floppy is back
   Floppy_Message( M_MASTER_IN2, M_BACKUP_IN2, 1, 1);
   while( Floppy_Request_Disk() == ABORTED);

   // ask the password once again
   if( Ask_Password( M_OK_URN1, 345, FALSE) == FALSE)
      Error_Exit( M_PSWD_END);
}
#endif

/*{*/
//=============================================================================
//
// Function : Dialog_Menu
//
// Purpose  : process the menu allowing the president to perform either a
//            closing or a MAV restart
//
// Return   : static MENU_CHOICE , local function returning
//                                 CLOSE    = make the totals
//                                 CONTINUE = continue voting
//
// Arg List : void
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            BOOL Background_URN , indicates if the background is displayed
//            CLOSING_TYPE Closing_Type , close the URN with X25 or without
//
// Usage    : if( Dialog_Menu() == CLOSE)
//
// Checks   :
//
// Comment  : Once in the menu, the president can escape if he wants, the URN
//            then continues the normal process.
//            For the RECOUNT program there is only one possible choice: to
//            close the election by making the totals.
//            For the normal URN there is the possbility to make a new
//            password card to start the MAV and one to allow to remove
//            the floppy. Once this is done we return to the normal program.
//
//-----------------------------------------------------------------------------
/*}*/

static MENU_CHOICE Dialog_Menu( void)
{
   int key;
   int stop=FALSE;

#ifndef _RECOUNT
#ifndef _DEMO
   Display_Message( M_MENU_DISPLAY);
#else
   Display_Message( M_DEMO_MENU_DISPLAY);
#endif
#else
   Display_Message( M_MENU_RECOUNT);
#endif

   Background_URN = FALSE;

   while( stop == FALSE && ( key = Get_Key() ) != ESCAPE)
   {
      switch( toupper(key))
      {
      case 'C':
         Display_Message( M_CONFIRM_CLOSE);
         switch( key = toupper(Get_Key()) )
            {
            case 'O':
            case 'J':
               Closing_Type = DISK;
               return CLOSE;
            default:
#ifndef _RECOUNT
#ifndef _DEMO
   Display_Message( M_MENU_DISPLAY);
#else
   Display_Message( M_DEMO_MENU_DISPLAY);
#endif
#else
               Display_Message( M_MENU_RECOUNT);
#endif
            }
         break;

#ifndef _RECOUNT
      case 'X':
         Display_Message( M_CONFIRM_CLOSE);
         switch( key = toupper(Get_Key()) )
            {
            case 'O':
            case 'J':
               Closing_Type = X25;
               return CLOSE;
            default:
#ifndef _RECOUNT
#ifndef _DEMO
   Display_Message( M_MENU_DISPLAY);
#else
   Display_Message( M_DEMO_MENU_DISPLAY);
#endif
#else
               Display_Message( M_MENU_RECOUNT);
#endif
            }
         break;

      case 'M': // make password card

         Format_Password_Card();
         Display_Message( M_PSWD_CARD2);
         Reader_Wait_Card_Removed( Intern_Reader);

         stop = TRUE;   break;

      case 'D': // remove disk

         Display_Message( M_REMOVE_DISK);
         Wait_Until_Floppy_Removed();
         Display_Message( M_MAV_RESTART, 1, 0);
         while( Floppy_Request_Disk() == ABORTED);

         stop = TRUE;     break;
#ifdef _DEMO
      case 'N' :
           Remove_All_Files_EveryWhere() ;
           Error_Exit( M_REBOOT_FOR_NEW_DEMO);
#endif _DEMO

#endif
      }
   }
   //Display_Background_URN();  // redisplay the counters

   return CONTINUE;
}

//==============================================================================
//==================== M A K E   P A S S W O R D   C A R D =====================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : Format_Password_Card
//
// Purpose  : write the language code used by the president and the password
//            on card.
//
// Return   : static void  , local function
//
// Arg List : void
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            AESKEY mKey           , the master password
//
// Usage    : Format_Password_Card();
//
// Checks   : no formatted card is admitted, this is to prevent the lost of
//            a vote or an error in the counters.
//            The card reader functions are checked.
//
// Comment  : The password must be completely passed to the MAV, we may not
//            trunc its value. But each character is coded with 8 bytes and
//            we can only code 3 bytes on each card position. The password
//            must be converted, with each 8 bytes converted to 3 groups of
//            3 bytes that can be coded on the card.
//            The pasword is completed with the language code used by the
//            president so he can continue with the same language on the MAV.
//
//-----------------------------------------------------------------------------
/*}*/

#ifndef _RECOUNT
static void Format_Password_Card( void)
{
#if 0
   // DES code
   char Card_Buffer [30];

   // split the password
   for( int i = 0; i < AESKEYLEN; i++ )
   {
      Card_Buffer [i * 3    ] = (   mKey [i] >> 5      ) + '0';
      Card_Buffer [i * 3 + 1] = ( ( mKey [i] & 28) >> 2) + '0';
      Card_Buffer [i * 3 + 2] = ( ( mKey [i] &  3)     ) + '0';
   }

   // put the language code
   Card_Buffer [24] = Working_Language( ASK) + '0';
   Card_Buffer [25] = 0;
#endif

    char Card_Buffer [(AESKEYLEN*3)+2]; // every key-byte takes 3 bytes on the card
                                        // + 1 for language + 1 for null

    // split the password
    for( int i = 0; i < AESKEYLEN; i++ )
    {
        Card_Buffer [i * 3    ] = (   mKey.data [i] >> 5      ) + '0';
        Card_Buffer [i * 3 + 1] = ( ( mKey.data [i] & 28) >> 2) + '0';
        Card_Buffer [i * 3 + 2] = ( ( mKey.data [i] &  3)     ) + '0';
    }

    // put the language code
    Card_Buffer [(AESKEYLEN*3)] = Working_Language( ASK) + '0';
    Card_Buffer [(AESKEYLEN*3)+1] = 0;

   for(;;)  // write the password card
   {
      Display_Message( M_PSWD_CARD1);
      if( Reader_Request_Unformatted( Intern_Reader) != HOME)
      {
         Reader_Eject_Card( Intern_Reader);
         Display_Message( M_ERR_CARD1);
         sleep(5);        //rvh
         Reader_Wait_Card_Removed( Intern_Reader);
         continue;
      }

      Display_Message( M_WAIT);

      if( Card_Is_Initialized( Intern_Reader))  // card may not be used
         continue;

      if( Reader_Write_Card( Intern_Reader, Card_Buffer) != HOME)
      {
         Reader_Eject_Card( Intern_Reader);
         Display_Message( M_ERR_CARD2);
         sleep(5);        //rvh
         Reader_Wait_Card_Removed( Intern_Reader);
         continue;
      }
      break;
   }
   Reader_Eject_Card( Intern_Reader);
}
#endif

//==============================================================================
//======================= A C C E P T - C A R D - x x x ========================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : Accept_Card_Formatting
//
// Purpose  : activate the intern reader to accept a card to format
//
// Return   : static BOOL  , local function
//                           TRUE  = a card was inserted for formatting
//                           FALSE = no card to format
//
// Arg List : void
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//
// Usage    : if( Accept_Card_Formatting() == TRUE)
//
// Checks   : on the card reader function
//
// Comment  :
//
//-----------------------------------------------------------------------------
/*}*/

#ifndef _RECOUNT
static BOOL Accept_Card_Formatting( void)
{
   switch( Reader_Unformatted_Card( Intern_Reader) )
   {
   case HOME  :

      return TRUE;

   case CARD_ERROR :

      Reader_Eject_Card( Intern_Reader);
      Display_Message( M_ERR_CARD1);
      sleep(5);        //rvh
      Reader_Wait_Card_Removed( Intern_Reader);
      Reader_Unformatted_Card( Intern_Reader);
      //Display_Background_URN();

   default :
      return FALSE;
   }
}
#endif

/*{*/
//=============================================================================
//
// Function : Accept_Card_In_BAC
//
// Purpose  : activate the extern reader to accept a voted card
//
// Return   : static BOOL  , local function
//                           TRUE  = a card is inserted
//                           FALSE = no inserted card
//
// Arg List : void
//
// Globals  : Select_Reader * Extern_Reader , the extern reader description
//
// Usage    : if( Accept_Card_In_BAC() == TRUE)
//
// Checks   : on the card reader functions
//
// Comment  : If there is a card in the reader, we read that card, in case
//            of error, we come into the CARD_ERROR case.
//
//-----------------------------------------------------------------------------
/*}*/

static BOOL Accept_Card_In_BAC( void)
{
   switch( Reader_Formatted_Card( Extern_Reader) )
   {
   case HOME  :

      if( Reader_Physical_Read_Card( Extern_Reader, Reader_Buffer) == HOME)
         return TRUE;

   case CARD_ERROR :

      Display_Message( M_ERR_CARD1);
      sleep(5);        //rvh
      Reader_Eject_Card( Extern_Reader);
      Reader_Wait_Card_Removed( Extern_Reader);
      Reader_Formatted_Card( Extern_Reader);
      //Display_Background_URN();

   default :
      return FALSE;
   }
}

//==============================================================================
//====================== C L O S E - E L E C T I O N ===========================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : Supplemental_Copies
//
// Purpose  : make supplemental copies of the totals and the journal files on
//            the BACKUP disks
//
// Return   : RET_CODE  , SUCCEEDED = the last write was successfull
//                        ABORTED   = we will have to ask a disk where the
//                                    write was successfull before for X25
//
// Arg List : void
//
// Globals  : char BACKUP_Label[]    , the common label for all BACKUP disks
//            char Floppy_BACKUP[15] , the current BACKUP disk label
//
// Usage    : if( Supplemental_Copies == ABORTED)
//
// Checks   : if the last diskette write was aborted, we must return ABORT so
//            the calling function knows it has to ask a previous disk.
//            The minimum value for Copies is 2 because we always have at
//            least one MASTER and one BACKUP disk. If the value is smaller
//            we set it to 2.
//
// Comment  : This function first determine the current floppy BACKUP order
//            number, then it writes the totals and the journal files on all
//            the remaining disks.
//            Each time a disk is written, we change the Floppy_BACKUP name.
//
//-----------------------------------------------------------------------------
/*}*/

static RET_CODE Supplemental_Copies( void)
{
   char Floppy_Name [15];
   SessionList Tag = Find_Session( 8);
   int Copies = ( (Copies = atoi( Tag -> Text)) < 2 ? 2 : Copies);

   // skip the BACKUP1 if we are already on it
   int i = 1;
   if( Floppy_Current == Floppy_BACKUP)
      i = 2;

   for( ; i < Copies; i++)
   {
      sprintf( Floppy_BACKUP, "%s%d", BACKUP_Label, i);
      if (setjmp( JMP_CLOSE_OFF)) continue;
      Rollback_Point = RBP_CLOSE_OFF;
      Display_Message( M_TAKE_DISK);
      Wait_Until_Floppy_Removed();
      Display_Message( M_BACKUP_IN1, 1, 0);
      Floppy_Current = Floppy_BACKUP;
      if( Floppy_Request_Disk() == ABORTED) continue;
      Display_Message( M_URN_END1);
      Display_Message( M_WAIT, 0, 1);

      // copy journal files to BACKUP and write totals
      if( Create_Journals() == ABORTED || Write_Totals_File() == ABORTED)
      {
         Display_Message( M_TOTALS_ERROR_B, 1);

         if( (i+1) == Copies)
            return ABORTED;
      }

   }

   return SUCCEEDED;
}

/*{*/
//=============================================================================
//
// Function : Close_Election
//
// Purpose  : close the election by making the totals and writing the results
//            on disk
//
// Return   : static void  , local function
//
// Arg List : void
//
// Globals  : INTEGRITY_STATE Flag_Integrity , are the votes corrupted ?
//            ERROR_HANDLING  Error_Handling , what to do in case of error
//            ROLLBACK_POINT  Rollback_Point , where to jump when rollback
//            char   Floppy_MASTER[]         , name of the MASTER disk
//            char   Floppy_BACKUP[]         , name of the BACKUP disk
//            char * Floppy_Current          , name of the current disk
//            jmp_buf JMP_CLOSE_OFF          , jump point description
//            CLOSING_TYPE Closing_Type      , close with X25 ?
//
// Usage    : Close_Election();
//
// Checks   : if the Flag_Integrity is corrupted, there is nothing to do, we
//            return immediately.
//            Each floppy function is checked and can be rolled back.
//
// Comment  : The totals are written to the current disk, if that disk is the
//            MASTER disk, we also write the totals to the BACKUP. Before
//            writing the totals to the BACKUP, we must also create the journal
//            files on it so we have a complete duplicate of the MASTER disk.
//            If the closing type is with X25 transfer and there was a problem
//            when writing the BACKUP disk, we ask the MASTER disk back to
//            perform the transfer.
//
//-----------------------------------------------------------------------------
/*}*/

static void Close_Election( void)
{
   if( Flag_Integrity == CORRUPTED)
   {
      Display_Message( M_NO_TOTALS, 1);
      return;
   }

   Display_Message( M_URN_END1);
   Display_Message( M_WAIT, 0, 1);

   Count_Votes();                                 // count the votes in memory

   Error_Handling = ROLLBACK;
   Rollback_Point = RBP_CLOSE_OFF;   setjmp( JMP_CLOSE_OFF);

   if( Write_Totals_File() == ABORTED) // write the totals on the current disk
   {
      Display_Message( M_TOTALS_ERROR, 1);
//      if( Closing_Type == X25)
//         Display_Message( M_NO_TRANSFER, 1);
      return;
   }

   if( Supplemental_Copies() == ABORTED && Closing_Type == X25)                  // reinstall MASTER for X25
   {
      Display_Message( M_TAKE_DISK);
      Wait_Until_Floppy_Removed();
      Display_Message( M_MASTER_IN1, 1, 0);
      Floppy_Current = Floppy_MASTER;
      if( Floppy_Request_Disk() == ABORTED)
        {
        strcpy( Floppy_BACKUP, "a:BACKUP1");
        Floppy_Current = Floppy_BACKUP;
        }
      while( Floppy_Request_Disk() == ABORTED);
   }

//#ifndef _RECOUNT
//   if( Closing_Type == X25)                        // perform the X25 transfer
//      if( Flag_Integrity == CORRUPTED)
//         Display_Message( M_NO_TRANSFER, 1);
//      else
//      {
//         if( Install_X25() == ABORTED)
//            return;
//         Process_X25();
//      }
//#endif
}

//==============================================================================
//================================ M A I N =====================================
//==============================================================================

/*{*/
//=============================================================================
//
// Function : main
//
// Purpose  :
//
// Return   : void
//
// Arg List : int Arg_Count       , number of arguments in command line
//            char * Arg_Value [] , arguments of the command line
//
// Globals  : Select_Reader * Intern_Reader , the intern reader description
//            Select_Reader * Extern_Reader , the extern reader description
//            ElectionMain Root             , structure with election info
//            MODE Mode                     , describes program phase for
//                                            error handling
//            ERROR_HANDLING Error_Handling , what to do with an error
//            ROLLBACK_POINT Rollback_Point , where to jump in case of error
//            jmp_buf JMP_STARTUP           , jump point description
//            char Blanco_Votes []          , image of a blanco vote card
//            unsigned char far * keyb_ptr  , the pointer to the keyboard flags
//
// Usage    :
//
// Checks   :
//
// Comment  : When the president calls the menu, we must disable the readers
//            before we can activate it. Otherwise, there may be a card in
//            the reader when he choose to close the votes.
//            When a card is formatted, we check immedialtely after function's
//            return if the card is still on the reader's gate, so we can
//            enable card accept as quick as possible. If the card is still
//            in reader's gate after a certain time, we reeject the card.
//
//            The arguments in the command line are :
//
//             1) the language code  1=FR, 2=NL, 3=FR+NL, 4=GER, 5=FR+GER
//             2) the intern reader type number  (not for the REC program)
//             3) the extern reader type number
//
//-----------------------------------------------------------------------------
/*}*/

#pragma argsused
void main( int Arg_Count, char * Arg_Value [] )
{
   time_t time_ejected;
   TYPE_READER Intern_Type,Extern_Type;
   int rc;
   char Language, Situation;
#ifndef _TEST
   ctrlbrk( Control_Break);         // Install control break handler
   Old_Keyboard_Handler = getvect( 0x09);
   setvect( 0x09, Keyboard_Handler);
#endif

   Num_Lock_On();                   // Set Numlock on
   harderr( Floppy_Error_Handler);  // Redirect device errors to application
   Floppy_Turn_Long();              // Floppy will turn as long as possible
   setverify(1);                    // Set verification on for floppy I-O
   textmode(BW80);
   textcolor(WHITE);
   textbackground(BLACK);
   window(1,1,80,25);

   if( Arg_Count != 3)
      ScrExit("INVALID DISK");

   Intern_Type = (TYPE_READER) atoi( Arg_Value [1]);
   if( (Intern_Reader = Open_Command(Intern_Type, COM2)) == NULL )
      ScrExit("INTERN CARD READER NOT CONNECTED");
#ifndef _TMTDEBUG
   Extern_Type = (TYPE_READER) atoi( Arg_Value [2]);
   if( (Extern_Reader = Open_Command(Extern_Type, COM1)) == NULL )
      ScrExit("EXTERN CARD READER NOT CONNECTED");
#endif

   ETDiskA();                    // insert master or backup1 disk, wait...

// :TODO: 20030516 avn - should be rewritten checking absolute positions now!!

   rc = ETInitSET( &Language, 50); // get language parameter from autoexec.bat
   if( rc < 0 )                    // necessary for Setup_Database()
      ScrExit("INVALID DISK");
   Init_Languages(&Language);

   // :TODO: 20030516 avn - remove these later
   // printf( "Language [%d]\n", Language);

   rc = ETInitSET( &Situation, 102); // get elect_mode parameter from autoexec.bat
   if( rc < 0 )                      // necessary for Setup_Database()
      ScrExit("INVALID DISK");
   CodeSituation = Situation - '0';

   // :TODO: 20030516 avn - remove these later
   // printf( "Elect mode [%d]\n", CodeSituation);getch();

   switch( CodeSituation)
   {
      case 0:
      case 1:
         NUMBER_TYPE_ELECTOR = 4;
         break;
      case 2:
         NUMBER_TYPE_ELECTOR = 7;
         break;
      default:
         ScrExit("WRONG PARAMETER-VALUE");
   }

   // init permissions
   InitApplVariables( CodeSituation);

   if( Floppy_Get_FAT_Info() == ABORTED)   // initialisation for direct access
      ScrExit("FATAL ERROR");

   if( ETPassWord() == ABORTED )
      ScrExit("WRONG PASSWORD");

   Root.Language[0] = 1;               // force 1 language by default: English
   Root.Language[1] = NONE;

   if( ETPrepare() == ABORTED)         // prepare the election datas
      ScrExit("ERROR DATABASE FILE");
   if( Setup_Database() == ABORTED)    // load the election datas
      ScrExit("ERROR DATABASE FILE (PROBABLY WRONG PASSWORD)");

   Get_Max_Candidates_Per_Election();
   Format_Blanco_Card();
   ETInitEnglish();

   //rvh
   Allocate_Memory_BAC();         // allocate the memory for the cards storing

   Floppy_Open_File( TOTALS_FILE_NAME, & Totals_File);
   if( Totals_File != -1)                                    // B001 exists
      ETCard();                      // start election test card module
   else
      ScrExit("VOTE MACHINE NOT CLOSED");
}

//=== ETCMAIN.CPP =============================================================

