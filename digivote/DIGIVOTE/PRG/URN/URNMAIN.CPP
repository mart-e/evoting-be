/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="urnmain.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   URNMAIN.CPP  $</workFile>
<revision>$Revision: 1.12 $</revision>
<workFileTimeStamp>$Modtime:   Nov 10 2004 12:00:54  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Urn/URNMAIN.CPP-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/04 12:26:30 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>URNMAIN.CPP</fileName>
  <creationDate>17/01/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Programme principal urne </fr>
    <nl>Hoofdprogramma urne</nl></description>
  <history>
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
   13/04/94: RVH sleep() bijgevoegd v¢¢r sommige ....Remove_Card..</history>
  <comment>Main application logic for URN system</comment>
<feedback>

   Any comment and/or improvement suggestion can be send directly to the
   development team of Steria Benelux at the following email adress :
   dv2004@steria.be

</feedback>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
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
#include "..\urn\urnmes.h"
#include "..\urn\urnerro.h"
#include "..\urn\urnreco.h"

#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

// #include "..\diagnost\setup.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
typedef enum { CONTINUE, CLOSE } MENU_CHOICE;
typedef enum { X25     , DISK  } CLOSING_TYPE;
//</defines>


//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static void Format_Password_Card   ( void);
static void Setup_Readers( TYPE_READER Intern_Type, TYPE_READER Extern_Type);
static void Dialog_Start_MAV       ( void);
static MENU_CHOICE Dialog_Menu     ( void);
static BOOL Accept_Card_Formatting ( void);
static BOOL Accept_Card_In_BAC     ( void);
static RET_CODE Supplemental_Copies( void);
static void Close_Election         ( void);
static MENU_CHOICE Process_Card_Format( );

void interrupt( * Old_Keyboard_Handler)(...);

extern void Remove_All_Files_EveryWhere(void) ;
void Format_Blanco_Votes ( void);
BOOL Accept_Keyboard_ESC ( void);
//</prototypes>


//-------------------------------------------------------------- EXTERNALS ---
extern char Blanco_Card[];
extern int  Vote_Size_On_Card;      //  STEF:
extern int  PositionCardTypeElector;// stefG: relative position of elector card
extern int  PositionMemTypeElector;// stefG: relative position of elector bac
extern int  CurrentTypeElectorInBac;//  STEF:// NEW elector type

extern char * TOTALS_FILE_NAME;
//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
const unsigned int LIMIT = 30000;

INTEGRITY_STATE Flag_Integrity  = CONSISTENT;
ERROR_HANDLING  Error_Handling  = ABORT;
ROLLBACK_POINT  Rollback_Point  = RBP_STARTUP;
STARTUP_MODE    Startup_Mode    = FIRST_TIME;
BOOL            Background_URN  = FALSE;

Select_Reader * Intern_Reader, * Extern_Reader;

void interrupt (* oldfunc)( ...);
unsigned char far * keyb_ptr;
int Version;                           // 1=SANKYO, 2=KDE

int NUMBER_TYPE_ELECTOR = 17; //stefG number of elector type
int M_SPECIAL_ELECT     =  0;
int PermissionsUsed[MAX_TYPE_ELECTOR - 1][MAX_ELECTIONS -1];

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


int NbrOfCounters; /* number of counters the totals counter not included
                          default: only a counter with the totals */

/*
 The names of the disks must always begin with the drive letter + ':'
 followed by MASTER or BACKUPx, where x is the BACKUP order number.
 The names MASTER and BACKUP have a length of 6 characters, the names may
 change but the length must be 6.
 The maximum length for BACKUPx is 8 because this is the space that was
 reserved in the message array (no test is made !!)
 These lengths are VERY important !!!
*/

char   Floppy_MASTER[15] = "A:MASTER";
char   BACKUP_Label[]    = "A:BACKUP";
char   Floppy_BACKUP[15] = "A:BACKUP1";
char * Floppy_Current    = Floppy_MASTER;

//rvh
char Reader_Buffer [MAX_CARD_BUF+1];
char Blanco_Votes  [MAX_VOTE_BUF];
//rvh

ElectionMain Root;

MODE Mode = STARTUP;

jmp_buf JMP_STARTUP;
jmp_buf JMP_BEFORE_FORMATTED;
jmp_buf JMP_AFTER_FORMATTED;
jmp_buf JMP_BEFORE_IN_BAC;
jmp_buf JMP_AFTER_IN_BAC;
jmp_buf JMP_CLOSE_OFF;

CLOSING_TYPE Closing_Type;

#ifndef _RECOUNT
BOOL Card_In_Slot = FALSE; // boolean card in internal slot
#endif

time_t time_ejected;                   //  STEF:// NEW timeout internal cards reader
MODE_DISP modedisp;                    //  STEF:// NEW display mode
int MsgElector;                        //  STEF:// NEW message to display

int CurrentTypeElector;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

/*<function id="Error_Exit_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description>
    <en>locks the program and disables the readers when a fatal error
    has occured.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the mnemonic of the message to display</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Extern_Reader</name>
      <description><en>the extern reader description</en></description>
    </parameter>
  </parameters>
  <usage>Error_Exit_URN( E_ERROR);</usage>
  <comment>
   There is no real exist of the program, in fact the machine is
   locked and the readers are disabled.
   We used the low level functions without error checking to
   disable the readers, because this could cause a loop otherwise.
   When the code protected by the TRACE directive is active and if
   the exit is caused by a card reader error; the last commands
   send to the reader are send to the output (either printer or
   screen)</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Error_Exit ( int M_Index)
// ---------------------------------------------------------------------------
{
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
//</code></function>


/*<function id="Setup_Readers">
<heading>
  <description><en>
    initialize the card readers</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>TYPE_READER</datatype><name>Intern_Type</name>
      <description><en>intern reader type number</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>TYPE_READER</datatype><name>Extern_Type</name>
      <description><en>extern reader type number</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Extern_Reader</name>
      <description><en>the extern reader description</en></description>
    </parameter>
  </parameters>
  <usage>Setup_Readers( Intern_Type, Extern_Type);</usage>
  <checks>on reader initialisation errors</checks>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
static void Setup_Readers ( TYPE_READER Intern_Type, TYPE_READER Extern_Type)
// ---------------------------------------------------------------------------
{
#ifndef _RECOUNT
    if(( Intern_Reader = Open_Command( Intern_Type, COM2)) == NULL)
        Error_Exit( M_ERR_READ1);
#endif

    if(( Extern_Reader = Open_Command( Extern_Type, COM1)) == NULL)
        Error_Exit( M_ERR_READ2);
}
//</code></function>


/*<function id="Dialog_Start_MAV"><!-- _MAV avoids duplicate id's -->
<heading>
  <description><en>dialog to process the remove of the floppy to start
  the MAV</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>STARTUP_MODE</datatype><name>Startup_Mode</name>
      <description><en>first time startup ??</en></description>
    </parameter>
  </parameters>
  <usage>Dialod_Start_MAV();</usage>
  <comment>
      If the URN is in RESUME mode, it may mean that only the URN had
      a problem, the MAVs may still function, so we ask the president
      if he wants to restart the MAVs in that case.
      If the MAVs must be started, we format a card with the password
      and the language code used by the president. The MAV has no
      keyboard, so it is the only way to transmit the password to the
      MAV. Once this is done we wait until the password card and the
      floppy disk are removed. After that the president is supposed to
      start the MAVs and the URN waits for the floppy to come back.
      When the floppy is back, we ask the password once again to
      prevent that someone else than the president starts the voting.
  </comment>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static void Dialog_Start_MAV ( void)
// ---------------------------------------------------------------------------
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

   // 20040718 avn - 5 sec iso 60 speeds up boot process
   sleep( 5);


    Display_Message( M_URN_START);              // Wait until the floppy is back
    Floppy_Message( M_MASTER_IN2, M_BACKUP_IN2, 1, 1);
    while( Floppy_Request_Disk() == ABORTED);

    // ask the password once again
    if( Ask_Password( M_OK_URN1, 345, FALSE) == FALSE)
        Error_Exit( M_PSWD_END);
}
#endif
//</code></function>


/*<function id="Dialog_Menu">
<heading>
  <description><en>
    process the menu allowing the president to perform either a
    closing or a MAV restart</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">MENU_CHOICE</datatype>
      <description><en>
        CLOSE    = make the totals
        CONTINUE = continue voting</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>BOOL</datatype><name>Background_URN</name>
      <description><en>indicates if the background is displayed</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>CLOSING_TYPE</datatype><name>Closing_Type</name>
      <description><en>close the URN with X25 or without</en></description>
    </parameter>
  </parameters>
  <usage>if( Dialog_Menu() == CLOSE)</usage>
  <comment>
   Once in the menu, the president can escape if he wants, the URN
   then continues the normal process.
   For the RECOUNT program there is only one possible choice: to
   close the election by making the totals.
   For the normal URN there is the possbility to make a new
   password card to start the MAV and one to allow to remove
   the floppy. Once this is done we return to the normal program.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static MENU_CHOICE Dialog_Menu ( void)
// ---------------------------------------------------------------------------
{
    int key;
    int stop = FALSE;

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

    while( stop == FALSE && ( key = Get_Key() ) != ESCAPE )
    {
        switch( toupper( key ) )
        {
            case 'C':
                int menu_bef = FALSE;
                while( menu_bef == FALSE )
                {
                    Display_Message( M_CONFIRM_CLOSE );
                    switch( key = toupper( Get_Key() ) )
                    {
                        case 'O':
							if ( Working_Language( ASK) == FRENCH )
							{
								Closing_Type = DISK;
								return CLOSE;
							}
							break;
                        case 'J':
							if ( Working_Language( ASK) == DUTCH || Working_Language( ASK) == GERMAN)
							{
								Closing_Type = DISK;
								return CLOSE;
							}
							break;
                        case 'N' :
                            #ifndef _RECOUNT
                            #ifndef _DEMO
                            	Display_Message( M_MENU_DISPLAY);
                            #else
                            	Display_Message( M_DEMO_MENU_DISPLAY);
                            #endif
                            #else
                            	Display_Message( M_MENU_RECOUNT);
                            #endif
                            menu_bef = TRUE;
                    }
                }

                break;

            #ifndef _RECOUNT

            case 'M': // make password card
                Format_Password_Card();
                Display_Message( M_PSWD_CARD2);
                Reader_Wait_Card_Removed( Intern_Reader);
                stop = TRUE;
                break;

            case 'D': // remove disk
                Display_Message( M_REMOVE_DISK);
                Wait_Until_Floppy_Removed();
                Display_Message( M_MAV_RESTART, 1, 0);
                while( Floppy_Request_Disk() == ABORTED)
                    ;

                stop = TRUE;
                break;

            #ifdef _DEMO
            case 'N' :
                Remove_All_Files_EveryWhere() ;
                Error_Exit( M_REBOOT_FOR_NEW_DEMO);
            #endif _DEMO

            #endif
        }
    }

    ScreenAllCounters();

    return CONTINUE;
}
//</code></function>


/* ------------------------------------------------------- */
/* Situation 4 elections : Europees, Kamer, Senaat en Raad */

/*{*/
//=============================================================================
//
// Function : Display_background_special_elector
//
// Purpose  : process the menu allowing the president to format special
//            elector card
//
// Return   : static MENU_CHOICE , local function returning
//                                 CLOSE    = NO PROCESS
//                                 CONTINUE = continue voting
//
// Arg List : void
//
// Comment  : Once in the menu, the president can escape if he wants, the URN
//            then continues the normal process.
//            Otherwise, the president can format card for special elector
//
//-----------------------------------------------------------------------------
/*}*/

/*<function id="Process_Special_Elector">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">MENU_CHOICE</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <author>STEF</author>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static MENU_CHOICE Process_Special_Elector ( void)
// ---------------------------------------------------------------------------
{
    char Choice;

    ScreenOneCounter();

    for (;;)
    {
        if( Process_Card_Format() == CLOSE )
            return CLOSE;

        if( Accept_Card_In_BAC() == TRUE )
            Process_Card_In_BAC( Reader_Buffer);

        if ( kbhit() )
        {
            Choice = Get_Key();

            switch( Choice )
            {
                case ESCAPE :
                    return CONTINUE;
            }
        }
    }
}
#endif
//</code></function>


/*<function id="DisplaySpecialElector">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <author>STEF</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplaySpecialElector (void)
// ---------------------------------------------------------------------------
{
    Display_Message( M_SPECIAL_ELECT_HEADER );
    for( int i = 2; i < ( NbrOfCounters + 1 ); i++ )
    {
        Display_Message( ElectorTypesUsed[i].SpecialElectMessageID,
                         FALSE, TRUE, FALSE );
    }
    Display_Message( M_SPECIAL_ELECT_FOOTER, FALSE, TRUE, FALSE );
}
//</code></function>


/*<function id="ScreenMenuSpecialElector">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">MENU_CHOICE</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static MENU_CHOICE ScreenMenuSpecialElector (void)
// ---------------------------------------------------------------------------
{
    char Choice;
    MENU_CHOICE codeback;
    BOOL locked = FALSE;

    DisplaySpecialElector();

    for (;;)
    {
        if ( locked == FALSE)
        {
            Reader_Disable( Intern_Reader);
            Reader_Disable( Extern_Reader);

            if( Reader_Card_Position( Intern_Reader) == NO_CARD &&
                Reader_Card_Position( Extern_Reader) == NO_CARD)
            {
                Reader_Eject_Card( Intern_Reader);
                Reader_Eject_Card( Extern_Reader);
            }

            locked = TRUE;
        }

        if ( kbhit() )
        {
            BOOL isEXT;
            Choice = Get_ExtKey( &isEXT ); // for extended key
            codeback = CONTINUE;

            if ( isEXT == FALSE )
            {
                for( int i = 1; i <= ( NbrOfCounters + 1 ); i++ )
                {
                    if( ElectorTypesUsed[i].key == toupper( Choice ) )
                        break;
                }

                if( i > ( NbrOfCounters + 1 ) )
                {
                    if( Choice == ESCAPE )
                        return CONTINUE;
                }
                else
                {

                    CurrentTypeElector = i;
                    codeback = Process_Special_Elector();
                    locked = FALSE;

                    if ( codeback == CLOSE )
                        return CONTINUE;

                    else
                        DisplaySpecialElector();
                }
            }
        }
        else
        {
            switch( Choice )
            {
                case 59:  // sample f1 key detect
                    break;

                case 61:  // sample f3 key detect
                    break;

                case ESCAPE :
                    return CONTINUE;
            }
        }
    }
}
#endif
//</code></function>

/* ------------------------------------------------------- */

/*<function id="Format_Password_Card" confidential="yes">
<heading>
  <description><en>
    write the language code used by the president and the password
    on card.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description></parameter>
    <parameter type="global">
      <datatype>AESKEY *</datatype><name>mKey</name>
      <description><en>the master password</en></description></parameter>
  </parameters>
  <usage>Format_Password_Card();</usage>
  <checks>
   no formatted card is admitted, this is to prevent the lost of
   a vote or an error in the counters. The card reader functions are checked.
  </checks>
  <comment>
   The password must be completely passed to the MAV, we may not
   trunc its value. But each character is coded with 8 bits and
   we can only use 3 bits on each card position. The password
   must be converted, with each byte converted to 3 groups of
   4 bits that can be coded on the card.
   The pasword is completed with the language code used by the
   president so he can continue with the same language on the MAV.
  </comment>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static void Format_Password_Card ( void)
// ---------------------------------------------------------------------------
{
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
//</code></function>


/*<function id="Accept_Card_Formatting">
<heading>
  <description>
    <en>activate the intern reader to accept a card to format</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">BOOL</datatype>
      <description><en>TRUE  = a card was inserted for formatting
        FALSE = no card to format</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
  </parameters>
  <usage>if( Accept_Card_Formatting() == TRUE)</usage>
  <checks>on the card reader function</checks>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static BOOL Accept_Card_Formatting ( void)
// ---------------------------------------------------------------------------
{
    switch( Reader_Unformatted_Card( Intern_Reader) )
    {
        case HOME  :

            return TRUE;

        case CARD_ERROR :

            Reader_Eject_Card( Intern_Reader);
            Display_Message( M_ERR_CARD1);
            sleep(5);
            Reader_Wait_Card_Removed( Intern_Reader);
            Reader_Unformatted_Card( Intern_Reader);

        default :
            return FALSE;
    }
}
#endif
//</code></function>


/*<function id="Accept_Card_In_BAC">
<heading>
  <description><en>
    activate the extern reader to accept a voted card</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">BOOL</datatype>
      <description><en>
        TRUE  = a card is inserted
        FALSE = no inserted card</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Extern_Reader</name>
      <description><en>the extern reader description</en></description>
    </parameter>
  </parameters>
  <usage>if( Accept_Card_In_BAC() == TRUE)</usage>
  <checks>on the card reader functions</checks>
  <comment>
   If there is a card in the reader, we read that card, in case
   of error, we come into the CARD_ERROR case.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Accept_Card_In_BAC (void)
// ---------------------------------------------------------------------------
{
    int CARD_MSG = -1;

    switch( Reader_Formatted_Card( Extern_Reader) )
    {
        case HOME  :

            if( Reader_Physical_Read_Card( Extern_Reader, Reader_Buffer) == HOME)
                return TRUE;

            else
                CARD_MSG = M_ERR_CARD3;

            // Verwijdere de kaart uit de externe lezer en
            // probeer opnieuw.

        case CARD_ERROR :

            if( CARD_MSG == -1 )
                CARD_MSG = M_ERR_CARD1;

            // Deze kaart is verkeerd in de lezer geplaatst of is
            // onbruikbaar

            Display_Message( CARD_MSG);
            sleep( 5 );
            Reader_Eject_Card( Extern_Reader);
            Reader_Wait_Card_Removed( Extern_Reader);
            Reader_Formatted_Card( Extern_Reader);

            ScreenAllCounters();

        default :

            return FALSE;
    }
}
//</code></function>


/*<function id="Supplemental_Copies">
<heading>
  <description><en>
    make supplemental copies of the totals and the journal files on
    the BACKUP disks</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = the last write was successfull
        ABORTED   = we will have to ask a disk where the
                    write was successfull before for X25</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>BACKUP_Label[] </name>
      <description><en>the common label for all BACKUP disks</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP[15]</name>
      <description><en>the current BACKUP disk label</en></description>
    </parameter>
  </parameters>
  <usage>if( Supplemental_Copies == ABORTED)</usage>
  <checks>
   if the last diskette write was aborted, we must return ABORT so
   the calling function knows it has to ask a previous disk.
   The minimum value for Copies is 2 because we always have at
   least one MASTER and one BACKUP disk. If the value is smaller
   we set it to 2.</checks>
  <comment>
   This function first determine the current floppy BACKUP order
   number, then it writes the totals and the journal files on all
   the remaining disks.
   Each time a disk is written, we change the Floppy_BACKUP name.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Supplemental_Copies ( void)
// ---------------------------------------------------------------------------
{
		char aDigestString[40 +1];

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
        if (setjmp( JMP_CLOSE_OFF))
            continue;

        Rollback_Point = RBP_CLOSE_OFF;
        Display_Message( M_TAKE_DISK);
        Wait_Until_Floppy_Removed();
        Display_Message( M_BACKUP_IN1, 1, 0);
        Floppy_Current = Floppy_BACKUP;
        if( Floppy_Request_Disk() == ABORTED)
            continue;

        Display_Message( M_URN_END1);
        Display_Message( M_WAIT, 0, 1);

        // copy journal files to BACKUP and write totals, no images
        if( Copy_Totals_File() == ABORTED)
        {
            Display_Message( M_TOTALS_ERROR_B, 1);

            if( (i+1) == Copies)
                return ABORTED;
        }
    }

    return SUCCEEDED;
}
//</code></function>


/*<function id="Close_Election">
<heading>
  <description><en>
    close the election by making the totals and writing the results
    on disk</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>are the votes corrupted ?</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>what to do in case of error</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>where to jump when rollback</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_MASTER[]</name>
      <description><en>name of the MASTER disk</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Floppy_BACKUP[] </name>
      <description><en>name of the BACKUP disk</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char*</datatype><name>Floppy_Current</name>
      <description><en>name of the current disk</en></description>
    </parameter>
    <parameter type="global">
      <datatype>CLOSING_TYPE</datatype><name>Closing_Type</name>
      <description><en>close with X25 ?</en></description></parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_CLOSE_OFF</name>
      <description><en>jump point description</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks>
   if the Flag_Integrity is corrupted, there is nothing to do, we
   return immediately.
   Each floppy function is checked and can be rolled back.</checks>
  <comment>
   The totals are written to the current disk, if that disk is the
   MASTER disk, we also write the totals to the BACKUP. Before
   writing the totals to the BACKUP, we must also create the journal
   files on it so we have a complete duplicate of the MASTER disk.
   If the closing type is with X25 transfer and there was a problem
   when writing the BACKUP disk, we ask the MASTER disk back to
   perform the transfer.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Close_Election ()
// ---------------------------------------------------------------------------
{
		char aDigestString[40 +1];

    if( Flag_Integrity == CORRUPTED)
    {
        Display_Message( M_NO_TOTALS, 1);
        return;
    }

    Display_Message( M_URN_END1);
    Display_Message( M_WAIT, 0, 1);

    // initialize votes counters first
    ResetVotesCounters();

    // count the votes in memory
    if( Count_Votes() == ABORTED)
    {
        Display_Message( M_TOTALS_ERROR, 1);
        return;
    }

    Error_Handling = ROLLBACK;
    Rollback_Point = RBP_CLOSE_OFF;
    setjmp( JMP_CLOSE_OFF);

    if( Write_Totals_File() == ABORTED) // write the totals on the current disk
    {
		// cannot write totals to disk
        Display_Message( M_TOTALS_ERROR, 1);
        return;
    }

	// resume closing procedure
    Display_Message( M_URN_END1);
    Display_Message( M_WAIT, 0, 1);

    // clear images on current disk
    Clear_Image_Files();

	// attempt to copy results to backup disks
    Supplemental_Copies();

	// remove totals file from memory using secure delete
	Remove_Totals_File();

	Display_Message( M_TAKE_DISK);
	Wait_Until_Floppy_Removed();

	return;
}
//</code></function>


/*<function id="Process_Card_Format">
<heading>
  <description><en>
    Internal Card process (format card) in/out</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <comment>parameter:
   global Card_In_Slot  : flag position card step
   global time_ejected  : timeout
   MsgElect : label of urn counter (card validate)
   ielect   : index of type elector to process
   modedisp : display mode of counter label CARDS READY
  </comment>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
static MENU_CHOICE Process_Card_Format ()
// ---------------------------------------------------------------------------
{
    RET_CODE back;

    if( Card_In_Slot)
    {
        switch( Reader_Card_Position( Intern_Reader))
        {
            case HOME   :
            case NO_CARD:

                Card_In_Slot = FALSE;
                break;

            case CARD_ERROR:

                Reader_Eject_Card( Intern_Reader);
                time_ejected = time( NULL);
                break;

            default:

                if( time( NULL) > time_ejected + 10 )
                {
                    Reader_Eject_Card( Intern_Reader);
                    time_ejected = time( NULL);
                }
        }
    }

    if( ! Card_In_Slot )
    {
        if( Accept_Card_Formatting() == TRUE )
        {
            Format_Blanco_Card();
            if( CurrentTypeElector == 1 )
                ScreenOneCounter();

            back = Process_Card_Formatting( Blanco_Card,
                                            CurrentTypeElector );

            if( Reader_Card_Position( Intern_Reader) == NO_CARD )
            {
                Reader_Unformatted_Card( Intern_Reader);
                Card_In_Slot = FALSE;

                if ( back == SUCCEEDED )
                    return CLOSE;

            }
            else
            {
                Card_In_Slot = TRUE;
                time_ejected = time( NULL);

                if ( back == SUCCEEDED )
                    return CLOSE;

            }
        }
    }
    return CONTINUE;
}
#endif
//</code></function>


/*<function id="Urnmain" type="main">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Arg_Count</name>
      <description><en>number of arguments in command line</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Arg_Value []</name>
      <description><en>arguments of the command line</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Intern_Reader</name>
      <description><en>the intern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>Select_Reader *</datatype><name>Extern_Reader</name>
      <description><en>the extern reader description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ElectionMain</datatype><name>Root</name>
      <description><en>structure with election info</en></description>
    </parameter>
    <parameter type="global">
      <datatype>MODE</datatype><name>Mode</name>
      <description><en>describes program phase for error handling</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>ERROR_HANDLING</datatype><name>Error_Handling</name>
      <description><en>what to do with an error</en></description>
    </parameter>
    <parameter type="global">
      <datatype>ROLLBACK_POINT</datatype><name>Rollback_Point</name>
      <description><en>where to jump in case of error</en></description>
    </parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_STARTUP</name>
      <description><en>jump point description</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Blanco_Votes []</name>
      <description><en>image of a blanco vote card</en></description>
    </parameter>
    <parameter type="global">
      <datatype>unsigned char far*</datatype><name>keyb_ptr</name>
      <description><en>the pointer to the keyboard flags</en></description>
    </parameter>
  </parameters>
  <comment>
   When the president calls the menu, we must disable the readers
   before we can activate it. Otherwise, there may be a card in
   the reader when he choose to close the votes.
   When a card is formatted, we check immedialtely after function's
   return if the card is still on the reader's gate, so we can
   enable card accept as quick as possible. If the card is still
   in reader's gate after a certain time, we reeject the card.

   The arguments in the command line are :

           1) the language code  1=FR, 2=NL, 3=FR+NL, 4=GER, 5=FR+GER
           2) the intern reader type number  (not for the REC program)
           3) the extern reader type number
   stefG   4) a code to indicate the number of elector and the "faciliteit"
           0 normal mode (3 electors, no "faciliteit")
           1 faciliteit mode (3 electors, "faciliteit")
           2 "voeren" mode (6 electors)
  </comment>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
void main ( int Arg_Count, char * Arg_Value [] )
// ---------------------------------------------------------------------------
{

   BOOL Escape_Active = FALSE;
   #ifndef _RECOUNT
      BOOL Space_Active = FALSE;
   #endif

   TYPE_READER Intern_Type, Extern_Type;

   #ifndef _TEST
      ctrlbrk( Control_Break);         // Install control break handler
      Old_Keyboard_Handler = getvect( 0x09);
      setvect( 0x09, Keyboard_Handler);
   #endif

   Mode = STARTUP;
   Reset_Counters();                 // NEW RESET ALL DETAILED VOTE COUNTER (BY TYPE)

   Num_Lock_On();                   // Set Numlock on
   harderr( Floppy_Error_Handler);  // Redirect device errors to application
   Floppy_Turn_Long();              // Floppy will turn as long as possible
   setverify( 1);                   // Set verification on for floppy I-O

   textmode( C4350);				// allows gotoxy() to reach bottom of screen
   Setup_Video();                   // Set graphic mode

   Display_Title_URN();             // Display the background
   Init_Languages( Arg_Value [1]);  // We must set a language before we can
                                    // use the messages
   #ifndef _RECOUNT
      if( Arg_Count != 5)
   #else
      if( Arg_Count != 4)
   #endif
         Error_Exit( M_BAD_ARGUMENTS);

   if( Root.Language [1] != NONE)
      Working_Language( Ask_User_For_Language());

   #ifndef _RECOUNT
      InitApplVariables( atoi( Arg_Value[ 4 ] ));  // parameter %ELECT_MODE%
   #else
      InitApplVariables( atoi( Arg_Value[ 3 ] ));  // parameter %ELECT_MODE%
   #endif

   #ifdef _DEMO
      // display demo_msg1 & demo_msg2
      Display_Message( M_DEMO_MSG1, TRUE, FALSE, FALSE);
      Display_Message( M_DEMO_MSG2, TRUE, FALSE, FALSE);
      // end display demo-messages
   #endif

   Display_Message( M_WAIT);

   #ifndef _RECOUNT
      Intern_Type = (TYPE_READER) atoi( Arg_Value [2]);
      Extern_Type = (TYPE_READER) atoi( Arg_Value [3]);
   #else
      Intern_Type = Extern_Type = (TYPE_READER) atoi( Arg_Value [2]);
   #endif

   M_SPECIAL_ELECT     = 0;     // Geen speciale types verkiezingen
   /* ------------------------------------------------------- */

#if _DEBUGMODE
   sprintf( gcDcio, "Setup and test reader type ahead!"); dcio();
#endif // _DEBUGMODE

   // FYI: generation 1 is obsolete as off october 2006
   Setup_Readers( Intern_Type, Extern_Type);

   // reader generation 1 or 2?
   Version = 1;                  // SANKYO ( gen 1)
   Check_Reader_Type( Extern_Type);
   // we're still alive, so we have a gen 2!
   Version = 2;                  // KDE ( gen 2)

   Error_Handling = ROLLBACK;
   Rollback_Point = RBP_STARTUP;

   if( setjmp( JMP_STARTUP) )              // No switching disks during startup
      if( Floppy_Current == Floppy_MASTER)
         Error_Exit( E_REBOOT_BACKUP);
      else
         Error_Exit( E_FATAL_DISKETTE);

   Find_Out_Disk();                        // determine if MASTER or BACKUP

   if( Floppy_Get_FAT_Info() == ABORTED)   // initialisation for direct access
      longjmp( JMP_STARTUP, 1);

   if( Ask_Password( M_PSWD_INP, 230,  FALSE ) == FALSE)
      Error_Exit( M_PSWD_END);

   Setup_Registration();          // prepares the system for the vote processing
   Initialize_Direct_Access();    // initialize direct access on that disk

   // initialize counters here in case we use monitoring in debug mode later
   ResetVotesCounters();

   // tmt
   Get_Max_Candidates_Per_Election();

   Format_Blanco_Card();
   Mode = NORMAL;
   /*
   if( NbrOfCounters == 0 )
     CurrentTypeElector = 0;
   else
   */
      CurrentTypeElector = 1;

   #ifndef _RECOUNT
      Dialog_Start_MAV();
   #endif
   Display_Message( M_WAIT);

   ScreenAllCounters();

   // Forces to read cards left from previous runs.
   if( Reader_Card_Position( Extern_Reader) == HOME )
      Reader_Physical_Read_Card( Extern_Reader, NULL );

   Background_URN = TRUE;

#if _DEBUGMODE
   sprintf( gcDcio, "Test reader type before loop ahead!"); dcio();
#endif // _DEBUGMODE

   // check type before we start polling the readers
   Check_Reader_Type( Extern_Type);

   for(;;) // URN loop
   {
      // Use the code below to check memory stability
      // printf( "%lu\n", coreleft());

      #ifndef _RECOUNT
         // monitors internal reader and handles case where president inserts
         //  a magnetic card to initialize it for voting
         Process_Card_Format();
      #endif

      // monitors external reader and handles case where elector inserts
      //  his voting card
      if ( Accept_Card_In_BAC() == TRUE)
      {
         Process_Card_In_BAC( Reader_Buffer);
      }

      // monitor keyboard hit and find query type
      if ( kbhit() )
      {
         int keyhit = Get_Key();
         switch ( keyhit )
         {
             case ESCAPE:

                 Escape_Active = TRUE;
                 break;

             #ifndef _RECOUNT
             case SPACE:

                 if( NbrOfCounters != 0 )
                     Space_Active = TRUE;

                 break;
             #endif
         }
      }

      if( Escape_Active == TRUE)
      {
         // make sure it is safe to process the menu
         #ifndef _RECOUNT
            if( Reader_Card_Position( Intern_Reader) == NO_CARD &&
                Reader_Card_Position( Extern_Reader) == NO_CARD)
            {
               Reader_Disable( Intern_Reader);
         #else
            if( Reader_Card_Position( Extern_Reader) == NO_CARD)
            {
         #endif
               Reader_Disable( Extern_Reader);
               #ifndef _RECOUNT
                  Reader_Eject_Card( Intern_Reader);
               #endif
               Reader_Eject_Card( Extern_Reader);
               if( Ask_Password( M_MENU_CALL, 270 , TRUE) != FALSE )
               {
                  if ( Dialog_Menu() == CLOSE)
                  {
                     // we're closing so we don't care about the reader anymore
                     break;
                  }
               }
               else
               {
                  ScreenAllCounters();
               }
               // if reader got swapped is it still a gen 2?
               Check_Reader_Type( Extern_Type);

               #ifndef _RECOUNT
                  Reader_Unformatted_Card( Intern_Reader);
               #endif
               Reader_Formatted_Card( Extern_Reader);
               Escape_Active = FALSE;
            } // if( Reader_Card_Position...
      } // if( Escape_Active == TRUE)


      #ifndef _RECOUNT
         if( Space_Active == TRUE)
         {
            if( NbrOfCounters < 3 )
            {
                CurrentTypeElector = 2;
                Process_Special_Elector();
            }
            else
                ScreenMenuSpecialElector();

            CurrentTypeElector = 1;

            ScreenAllCounters();
            Space_Active = FALSE;
         }
      #endif
   } // eo for(;;) // URN loop

   #ifndef _RECOUNT
      Reader_Disable( Intern_Reader);
   #endif
   Reader_Disable( Extern_Reader);

   Close_Election();

   // polling station is now closed, choose enter or power off now
   Display_Message( M_URN_END3);
   while( Test_Key_Pressed( RETURN) == FALSE);

   // the end
   End_Of_Program = TRUE;
   DisplayCountersForEver();

} // eo main()
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
