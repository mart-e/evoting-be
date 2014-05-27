/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavmain.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVMAIN.CPP  $</workFile>
<revision>$Revision: 1.5 $</revision>
<workFileTimeStamp>$Modtime:   Nov 19 2004 15:03:10  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVMAIN.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/07 12:38:52 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
<fileName>MAVMAIN.CPP</fileName>
<creationDate>01/01/93</creationDate>
  <rights/>
<author>B.S.S.</author>
<description>
  <fr>Programme principal du système MAV.</fr>
  <nl>Hoofdprogramma van het MAV systeem.</nl></description>
<history>
  01/06/93: revision history starting date
  07/09/93: more precise check of the language code on the password card
  21/09/93: modifications made to support the new data structure (college ...)
  27/10/93: Message CARD_NOT_VALID1 was changed to CARD_NOT_VALID3 when there
        is an error reading the card.
  08/03/94: When an error is detected on the pasword card (Process_Password),
        the error message will now always be displayed in the languages
        specified as arguments.
  22/03/94  RVH:
        Do while loop added to check the confirmation for the language
        selection with new function "User_Confirms_Language".
        External "User_Confirms_Language" function added in MAVVIDEO.CPP
        Usage of Reader_Read_Card replaced by Reader_Physical_Read_Card in
        function "Process_Password" and "Main"
</history>
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
#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <alloc.h>

#include "..\gen\gentype.h"
#include "..\gen\genlang.h"
#include "..\gen\gencard.h"
#include "..\gen\gendata.h"
#include "..\gen\gencryp.h"
#include "..\gen\genvideo.h"
#include "..\gen\genflop.h"
#include "..\gen\card.h"

#include "..\mav\mavvideo.h"
#include "..\mav\mavmes.h"
#include "..\mav\mavasm.h"
#include "..\mav\mavlight.h"
#include "..\mav\display.h"
#include "..\mav\select.h"
#include "..\mav\drawings.h"
// #include "..\diagnost\setup.h"

#include "..\mav\ticket.h"        // ticketing lib

#define _DEBUGMODE      FALSE      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>

// external functions
//RVH: 22/03/1994
extern int  User_Confirms_Language( void );

// private functions
static void Rebuild_Password_From_Card( BYTE *Rebuild_MASTER, BYTE *Data_MASTER);
static void Process_Party( ElectionList Election, int CollegeInd, int PartyInd);
static int  Process_College( ElectionList Election);
static void Election_Loop( void);
static void Determine_Disk( void);
static void Process_Password( void);
static void Remove_Card( void);
static void Process_Fault( int M_Index, int M_Index_P);

static void ProceedWhenUserIsReady( void);

// public functions
void Process_Election( ElectionList Election);

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern char Card[];
extern char Blanco_Card[];
extern int Vote_Size_On_Card;       // NEW ADD
extern int PositionCardTypeElector; //stefG

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
int  Show_Time_Candidates = 10;
int  Show_Time_General = 3;
char * Floppy_Current;
char Floppy_MASTER[15] = "A:MASTER";
char BACKUP_Name []    = "BACKUP";
char Floppy_BACKUP[15] = "A:BACKUP1";
extern int  Allready_Voted;

char card_data[MAX_CARD_BUF+1]; // the physical card data as read by the card reader

extern char Card [MAX_VOTE_BUF];              // the write card buffer
extern VOTE_TYPE_0  Vote_0;
extern VOTE_TYPE_1  Vote_1;
extern int Offset;

struct ElectionMain Root;
struct Select_College Colleges;
struct Select_Party Parties;
struct Select_Candidate Candidates;
extern int Vote_0_Type;

jmp_buf JMP_Restart_Loop;
JUMP_POINT Jump = NO_JUMP;

int Fault_Counter = 0;
char Blanco_Votes [MAX_VOTE_BUF];

Select_Reader * Intern_Reader;                 // the intern reader definition

int PenCursor;

// initialisation of some values used elsewhere
// --------------------------------------------
int CurrentElector=0;

ERROR_HANDLING Error_Handling  = ABORT;       // how to handle retrys on floppy
INTEGRITY_STATE Flag_Integrity = CONSISTENT;

int CurrentTypeElector   = Belgian; // NEW Demo default type

int NUMBER_TYPE_ELECTOR  = 0;       // stefG
int PermissionsUsed[MAX_TYPE_ELECTOR - 1][MAX_ELECTIONS -1];


//</globals>

//============================================================= EOF HEADER ===
//</header>
//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---
/*<function id="ProceedWhenUserIsReady">
<heading>
  <description>
    <en>Adds a 'proceed' button on the 'review' screens, so that the user can
    indicate that he has checked his vote with the ticket.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
  <usage>
  </usage>
  <checks>
  </checks>
  <comment>
  </comment>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void ProceedWhenUserIsReady ( void)
// ---------------------------------------------------------------------------
{
        int iPenX, iPenY;

    // display button at the bottom of the screen
    DisplayButtonNextElection( Candidates.PartyMode);

    // wait until user pressed it with the lightpen
    do
    {
        Get_Pen( &iPenX, &iPenY);
    }
    while ( !IsNextElectionSelected( Candidates.PartyMode, iPenX, iPenY));

    return;
}
//</code></function>


/*<function id="Remove_Card">
<heading>
  <description>
    <en>Ask the user to remove the card from the intern reader and wait
until it is done.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype>
      <name>Intern_Reader</name>
      <description><en>the intern reader structure</en></description></parameter></parameters>
  <usage>Remove_Card();</usage>
  <checks></checks>
  <comment>We first test the intern reader to see if there is a card into it.
It makes no sense to display the message in that case, it will cause
a flash effect because it will be dirrectly erased.</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Remove_Card ()
// ---------------------------------------------------------------------------
{
   if( Reader_Card_Position( Intern_Reader) != NO_CARD)
   {
      Display_Message( M_TAKE_CARD);
      Reader_Wait_Card_Removed( Intern_Reader);
   }
}
//</code></function>


/*<function id="Rebuild_Password_From_Card" confidential="yes">
  <heading>
  <description>
    <en>Rebuilds the AESKEYLEN characters password from the AESKEYLEN*3 bytes on card.</en></description>
  <parameters>
    <parameter>
      <datatype>BYTE*</datatype>
      <name>Rebuild_MASTER</name>
      <description>
        <en>Where the password will be stored, no default value.</en></description></parameter>
      <parameter>
        <datatype>BYTE*</datatype>
        <name>Data_MASTER</name>
        <description>
          <en>The data that is stored on card, no default value.</en></description></parameter>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>Local function.</en></description></parameter></parameters>
      <usage>Rebuild_Password_From_Card( rebuild_key, card_data);</usage>
      <checks></checks>
      <comment>Each ascii value from the password was stored in 3 positions on
the card (Refer to GENCARD.CPP for a complete explanation on
card characteristics).</comment>
</heading><code>*/
// ---------------------------------------------------------------------------
static void Rebuild_Password_From_Card ( BYTE *Rebuild_MASTER, BYTE *Data_MASTER)
// ---------------------------------------------------------------------------
{
   for( int i=0; i < AESKEYLEN; i++)
   {
      Rebuild_MASTER[i] = (Data_MASTER[i*3  ] - '0' << 5) +
              (Data_MASTER[i*3+1] - '0' << 2) +
              (Data_MASTER[i*3+2] - '0'     );
   }
}
//</code></function>


/*<function id="Process_Party"><heading>
  <description>
    <en>Processes the candidates selection from a party.</en></description>
  <parameters>
    <parameter>
      <datatype>ElectionList</datatype>
      <name>Election</name>
      <description>
        <en>Structure containing the election data, no default value.</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>CollegeInd</name>
      <description>
        <en></en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>PartyInd</name>
      <description>
        <en>The index of the selected party, no default value.</en></description></parameter>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>Local function.</en></description></parameter></parameters>
  <usage>Process_Party( Election, Parties.Selected);</usage>
  <checks></checks>
  <comment>If the user makes an invalid choice, he gets an error message.
When the message is removed, the Input_Candidates function
returns with FALSE. We then loop, call Display_Candidates and we
reprocess the user candidate selection until the selection is
valid. The reason to do so, is that it was too difficult to
implement it in an other manner.</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Party ( ElectionList Election,
                           int CollegeInd,
                           int PartyInd)
// ---------------------------------------------------------------------------
{
   Init_Candidates( Election -> Ind, CollegeInd, PartyInd);

   Set_Candidates_Display_Mode( & Root, & Parties, & Candidates, Election->Ind);
   Calculate_Positions( & Root, & Parties, & Candidates, Election->Ind);

   Display_Candidates( & Candidates);

    if ( Allready_Voted)
    {
        if ( 2 == Allready_Voted)
        {
            // review mode for ticketing
            ProceedWhenUserIsReady();
        }
        else
        {
            // normal review mode
            SleepUnlessLightpenHit( Show_Time_Candidates);
        }
    }
    else
    {
        // Cancel flag set if newparty selected
        if( Input_Candidates( &Candidates))
        {
            Candidates.Cancel = FALSE;
        }
        else
        {
            Candidates.Cancel = TRUE;
        }
    }

    return;
}
//</code></function>


/*<function id="Process_College"><heading>
  <description>
    <en>Processes the party selection from a college.</en></description>
  <parameters>
    <parameter>
      <datatype>ElectionList</datatype><name>Election</name>
      <description>
        <en>Structure containing the election data, no default value.</en></description></parameter>
    <parameter>
      <datatype>int</datatype><name>CollegeInd</name>
      <description>
        <en>The index of the selected college, no default value.</en></description></parameter>
    <parameter type="return">
      <datatype>SELECT</datatype>
      <description></description></parameter></parameters>
  <usage>Process_College( Election, CollegeInd);</usage>
  <checks>rvh</checks>
  <comment></comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static SELECT Process_College (
   ElectionList   Election,      // election data structure
   int            CollegeInd     // index of selected college
   )
// ---------------------------------------------------------------------------
{
      int pass;

   SELECT Done = OTHER_COL;

   Init_Parties( Election -> Ind, CollegeInd );  // initialize the party

   if ( Allready_Voted)
   {
      // 20040105 avn - used M_LANG_F instead of M_LANG_GROUP_F
      int lang[3] = { M_LANG_F, M_LANG_N, M_LANG_D };

      if ( Vote_0_Type ) Two_Bytes_Int( &Parties.Selected, Vote_0.Party );
      else Two_Bytes_Int( &Parties.Selected, Vote_1.Party );

      int Group = Parties.Choice[Parties.Selected].LanguageGroup;

      switch( Colleges.NbrOfColleges )
      {
         case 0 :
            outtextxy( getmaxx() / 2,
                     getmaxy() / 2,
                     Get_Text( lang[ Working_Language( ASK ) ],
                     Group ) );
            break;

         case 1 :
            if ( CONSEIL_BXL == Election->Ind)
            {
               // 20040206 avn - remember College contains GroupInd since there are no
               // colleges for this election...
               outtextxy( getmaxx() / 2, getmaxy() / 2,
                  Get_Text( lang[Working_Language( ASK)], Vote_1.College - '0'));
            }
            break;

         default :
            for ( int i = 0; i < Colleges.NbrOfColleges;  i++)
            {
               // 20040115 avn - order of colleges not fixed anymore
               if ( Colleges.Choice[i].Ind == (Vote_1.College - '0'))
               {
                  outtextxy( getmaxx()/2, getmaxy()/2, Colleges.Choice[i].Name );
               }
            }
            break;
      }

      // check special election when in review mode
      if ( CONSEIL_BXL == Election->Ind)
      {
         // 20040114 avn - remember College contains GroupInd since there are no
         // colleges for this election...
         if ( DUTCH == ( Vote_1.College - '0')) AllowSpecialElection( TRUE);
      }

      if ( Parties.Selected == 0 )
      {
         outtextxy( getmaxx() /2,
                    getmaxy() / 3 * 2,
                    Get_Text( M_BLANCO_VOTE,Working_Language( ASK) ) );

         if ( 2 == Allready_Voted)
         {
             // review mode for ticketing
             ProceedWhenUserIsReady();
         }
         else
         {
             // normal review mode for blank votes
             SleepUnlessLightpenHit( Show_Time_General);
         }

         return DONE;
      }

      // normal review mode
      SleepUnlessLightpenHit( Show_Time_General);
   }

    for(;;) // until a choice is made
    {
        pass = NONE;

        Set_Party_Display_Mode();

      if ( !Allready_Voted)
      {
         pass = SelectLanguageGroup(); // select language group if there is

         // :WARNING: 20040114 avn - pass contains a language code (0/1/2)
         // and NOT a language group ( 1/2/4) as one might think...
         Parties.GroupInd = pass;
      }

        // return the lingual group selected or NULL if no group selected.
        for(;;)
        {
            if( !Allready_Voted )
            {
                Display_Parties( pass );            // display the parties
                Done = Input_Parties( pass );       // wait for a selection

                if( Done == OTHER_PARTY)   // case : cancel of white vote
                    continue;
                if( Done == OTHER_LANG)    // case : an other languag group asked
                    break;
                if( Done == OTHER_COL)     // case : an other college is selected
                    return(Done);
                // here with Done = DONE
                if( Parties.Blanco_Vote == TRUE)  // case : white vote was selected
                    return(Done);
            }


            // case : a party was selected, so process it : display the candidates
            Process_Party( Election, CollegeInd, Parties.Selected);

            if( !Allready_Voted )
            {
                if( Candidates.Cancel == TRUE )     // the user wanted to choose
                    continue;                        // another party

            }

            return( Done);
        }
    }
}
//</code></function>



/*<function id="Process_Election"><heading>
  <description>
    <en>Processes the college selection for the election.</en></description>
  <parameters>
    <parameter>
      <datatype>ElectionList</datatype>
      <name>Election</name>
      <description>
        <en>structure containing the election data.</en></description>
      <defaultValue>none</defaultValue></parameter>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>ElectionMain</datatype>
      <name>Root</name>
      <description><en>structure containing the used languages</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Party</datatype>
      <name>Parties</name>
      <description><en>structure containing the party list informations</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Candidate</datatype>
      <name>Candidates</name>
      <description><en>structure containing the candidate list for a party</en></description></parameter></parameters>
  <usage>Process_Election( LOOP);</usage>
  <checks>rvh</checks>
<comment>The function displays the election in the user language, waits
a few and then process the college selection. Once the college
is selected it calls the party selection process.</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
void Process_Election ( ElectionList Election)
// ---------------------------------------------------------------------------
{
    int CollegeInd;
    SELECT Done = OTHER_COL;

    if( Skip_Election( Election -> Ind, CurrentTypeElector ) == TRUE )
        return; //stefG

    Display_Election( Election -> Long_Name [ Working_Language( ASK)],
      Election->NbrOfColleges, Election->HasLanguageGroups);

    if( Build_Election( Election -> Ind)   == ABORTED)
        Error_Exit( M_HALTED); //JRU: Build Election In Memory (Candidates)

    do                 // until the user has made a choice
    {
        Init_Colleges( Election -> Ind);
        if( Allready_Voted )
        {
            if ( Vote_0_Type )
            {
                CollegeInd = Vote_0.College - '0';
            }
            else
            {
               if ( CONSEIL_BXL == Election->Ind)
               {
                  // 20040114 avn - remember CollegeInd contains GroupInd since there are no
                  // colleges for this election...
                  CollegeInd = 0;
               }
               else
               {
                  CollegeInd = Vote_1.College - '0';
               }
            }
        }
        else
            CollegeInd = Select_College();

        Done = Process_College( Election, CollegeInd);

        if( Allready_Voted )
        {
            Done = DONE;
        }

    } while( Done == OTHER_COL);

    return;
}
//</code></function>

/*<function id="Election_Loop"><heading>
  <description>
    <en>Processes all the elections and write the emitted vote on card.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>ElectionMain</datatype>
      <name>Root</name>
      <description><en>structure containing the used languages and the election informations</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype>
      <name>Intern_Reader</name>
      <description><en>the intern reader structure</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Card[]</name>
      <description><en>the card buffer</en></description></parameter></parameters>
  <usage>Election_Loop();</usage>
<checks>
when writing the vote to the card, if the operation fails the
alarm box is activated with the green light.</checks>
<comment>
If the language is not defined yet, asks the user to choose his
working language, this will always be the case for bilingual elections.
When all the elections are processed, the vote is written to the card
RVH:22/03/1994
Do while loop added to check the confirmation for the language selection.
"User_Confirms_Language" function added in MAVVIDEO.CPP</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Election_Loop (
   void)
// ---------------------------------------------------------------------------
{
      int   iFlag;
      BYTE  PhysicalCardData[MAX_CARD_BUF+1];

   memset( PhysicalCardData, NULL, sizeof( PhysicalCardData));

      #if _DEBUGMODE
         sprintf( gcDcio, "election_lp pcd[%d]",
            sizeof( PhysicalCardData)); dcio();
      #endif // _DEBUGMODE

   if ( !Allready_Voted)
   {
      // setup printer before we start
      if ( setupTicketing())
      {
         // returned an error
         Red_Light_On( M_NO_PRINTER, NOT_FATAL);
      }
   }

   // initialize buffer for new vote
   Init_Card_Buffer();

   if ( Allready_Voted)
   {
      Split_Card_Buffer( ( BYTE*) card_data, & iFlag, ( BYTE*) Card );
   }

   if ( Root.Language [1] != NONE)
      do
      {
         Working_Language( Ask_User_For_Language());
      } while( ! User_Confirms_Language() );


   if ( Allready_Voted)
   {
      // ask elector if he wants to review his vote
      if ( !Decide_To_Review_Vote())
      {
#ifdef _DEMO
         // ask if he wants to initialize his card ( demo only!)
         if ( isInitializeDemoCardOk())
         {
            // reformat the card so that it can be reused again
            Format_Blanco_Card();
            if ( Reader_Write_Card( Intern_Reader, Blanco_Card) != HOME)
            {
               // unable to write vote
               Green_Light_On( M_WRITE_ERROR_ON_CARD);
            }
            return;
         }
         else
         {
            // no init so we quit, card will be ejected
            return;
         }
#else
         // no review so we quit, card will be ejected
         return;
#endif
      } // end review
   }

   // process all the elections
   for ( ElectionList LOOP = Root.FirstElection; LOOP != NULL;
         LOOP = LOOP -> Next)
   {
      if( Allready_Voted )
      {
         switch( LOOP->Type )
         {
             case 0:                      // single vote mode
                 memcpy( &Vote_0,
                         &Card[Offset],
                         sizeof( Vote_0 ));
                 Offset += sizeof( Vote_0 );
                 Vote_0_Type = 1;
                 break;
             case 1:                      // multiple vote mode
                 memcpy( &Vote_1,
                         &Card[Offset],
                         sizeof( Vote_1 ));
                 Offset  += sizeof( Vote_1 );
                 Vote_0_Type = 0;
                 break;
         }
      }

      Process_Election( LOOP); //processes the college selection for the election.

      if( !Allready_Voted )
      {
         Buffer_Vote( LOOP);
         printTicket( SPOOL_VOTES, PhysicalCardData);   // spool electors votes
      }

      if( Destroy_Election( LOOP -> Ind) == ABORTED)
         Error_Exit( M_HALTED); //JRU: Destroy Election From Memory (Candidates)

   } // eo for all elections

   if ( !Allready_Voted )
   {
      // assume elector accepts
      initCard( 1, PhysicalCardData);

      // print electors votes and ask for cancel/confirm
      if ( printTicket( PRINT_VOTES, PhysicalCardData))
      {
         iFlag = VOTED;
      }
      else
      {
         iFlag = CANCELED;
      }

      // write the card
      if ( Write_Votes( iFlag, PhysicalCardData) == HOME)
      {
         if ( iFlag == VOTED)
         {
             // ticket has been confirmed, card is valid for URN
             Display_Message( M_GOODBYE);
         }
         else
         {
             // ticket was canceled, card is NOT valid for URN
             Display_Message( M_VOTE_CANCELED);
         }

         // drop the damned thing now the card is written
         printTicket( DROP_VOTES, PhysicalCardData);
      }
      else
      {
         // unable to write vote
         Green_Light_On( M_WRITE_ERROR_ON_CARD);
      }

   }
   else
   {
      if ( Root.iBureauWithTicketing)
      {
         if ( 9 == iFlag)
         {
            // ticket was canceled, card is NOT valid for URN
            Display_Message( I_REVIEW_NOK);
         }
         else
         {
            // ticket was aknowledged, card is valid for URN
            Display_Message( I_REVIEW_OK);
         }
         // wait a bit
         sleep( Show_Time_Candidates);
      }
   }

   return;
}
//</code></function>

/*<function id="Determine_Disk"><heading>
  <description>
    <en>Determine the label of the current floppy disk.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Floppy_MASTER[]</name>
      <description><en>complete name of the MASTER diskette</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Floppy_BACKUP[]</name>
      <description><en>complete name of the BACKUP diskette</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Floppy_Current</name>
      <description><en>name of the disk currently in floppy</en></description></parameter></parameters>
  <usage>Determine_Disk();</usage>
  <checks>on result of Floppy function</checks>
  <comment>
Determine the name of the current disk, so we know which disk
we must refer to in messages.</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Determine_Disk ( void)
// ---------------------------------------------------------------------------
{
    char Label[13];

    if( Floppy_Get_Label( Label) == ABORTED)
        Error_Exit( M_BAD_DISK);

    if( ! strcmp( Label, "MASTER") )
        Floppy_Current = Floppy_MASTER;

    else
        if( ! strcmp( Label, "BACKUP1") )
            Floppy_Current = Floppy_BACKUP;

        else
            Error_Exit( M_BAD_DISK);
}
//</code></function>

/*<function id="Process_Password" confidential="yes">
<heading>
  <description>
    <en>Ask the password card and process the password, it also sets the language code that is transported with the card.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype>
      <name>Intern_Reader</name>
      <description><en>the intern reader structure</en></description></parameter></parameters>
  <usage>Process_Password();</usage>
  <checks>on every Card function</checks>
  <comment>
The password is transmitted from the URN to the MAV by a card
called the (guess what) 'password card' ; this card also
transmits the language used by the president on the URN machine
so he continues working in the same language.
The password is rebuilded from card and is checked.
The language code on the card must be compatible with the current
possible languages, otherwise we are certain the card is bad.</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Password ( void)
// ---------------------------------------------------------------------------
{
    char    card_data[MAX_CARD_BUF+1];
    int     lang;
    AESKEY  rebuild_key;

    Jump = NO_JUMP;

    Display_Message( M_INPUT_PASSWORD_CARD);

    if( Reader_Request_Formatted( Intern_Reader) != HOME)  // wait for the card
    {
        Reader_Eject_Card( COM1);
        Error_Exit( M_TAKEOUT);
    }

    if( Reader_Physical_Read_Card( Intern_Reader, card_data) != HOME)   // read the card
    {
        Reader_Eject_Card( Intern_Reader);
        Error_Exit( M_BAD_CARD);
    }

    Rebuild_Password_From_Card( rebuild_key.data, card_data);

    // check if the given password is plausible.
    //
    if( Verify_Password( rebuild_key.data) == ABORTED)  // password on card ?
    {
        Reader_Eject_Card( Intern_Reader);
        Error_Exit( M_BAD_PASSWORD_CARD);
    }

    // set the working language for the president and verify that the language
    // code on the card is plausible
    if( (lang = Working_Language( ASK)) == NONE)
    {
        lang = card_data[AESKEYLEN*3] - '0';

        if( lang == Root.Language [0] || lang == Root.Language [1])
            Working_Language( lang);
        else
            lang = NONE;
    }
    else
        if( Working_Language( ASK) != (card_data[AESKEYLEN*3] - '0'))
            lang = NONE;

    if( lang == NONE)  // language on card ?
    {
        Reader_Eject_Card( Intern_Reader);
        Error_Exit( M_BAD_PASSWORD_CARD);
    }
}
//</code></function>

/*<function id="Process_Fault"><heading>
  <description>
    <en>Warns the user 2 times, and set the alarm on the third time.</en></description>
  <parameters>
    <parameter>
      <datatype>int</datatype>
      <name>M_Index</name>
      <description>
        <en>Mnemonic of the warning message, no default value.</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>M_Index_P</name>
      <description>
        <en>Mnemonic of the message to display when the alarm box is turned on, no default value.</en></description></parameter>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Fault_Counter</name>
      <description><en>number of consecutives errors made by the user</en></description></parameter></parameters>
  <usage>Process_Fault( M_CARD_NOT_VALID1, M_CARD_NOT_VALID1P);</usage>
  <checks></checks>
  <comment>
when the user makes 3 consecutives mistakes, he must refer to
the president</comment></heading>
<code>*/
// ---------------------------------------------------------------------------
static void Process_Fault ( int M_Index, int M_Index_P)
// ---------------------------------------------------------------------------
{
    Fault_Counter++;

    if( Fault_Counter >= 3)
        Green_Light_On( M_Index_P);
    else
        Display_Message( M_Index);
}
//</code></function>


/*<function id="Mavmain" type="main"><heading>
  <description>
    <en>MAV program processing.</en></description>
  <parameters>
    <parameter>
      <datatype>int</datatype>
      <name>argc</name>
      <description>
        <en>Number of arguments in command line.</en></description></parameter>
    <parameter>
      <datatype>char*</datatype>
      <name>argv</name>
      <description>
        <en>Array with all the command line arguments.</en></description></parameter>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en></en></description></parameter></parameters>
  <usage></usage>
  <checks></checks>
  <comment>
The MAV program runs in an infinite loop, the only manner to
stop the program is to turn the machine off.
If the election is bilingual, the language is resetted before
each new user.
We must switch to graphic mode as soon as possible because the
Display_Message function works graphicaly.
We must assign a working language as soon as possible so we
know what language to use to display the messages. If no
argument is specified in the command line or if the language
code is false, the message is displayed in English.
If a card is present in the reader while starting, it is ejected.
In case of consecutive errors, pressing the Reset button on the
alarm box brings our back to the jump point. Only the president
may push on the reset button. When the reset button is pressed,
the card in the reader is ejected and the user must vote again,
all his choices are lost.

The arguments in the command line are :

1) the language code  1=FR, 2=NL, 3=FR+NL, 4=GER, 5=FR+GER
2) the intern reader type number
3) PenCursor not mandatory!!
stefG   4) a code to indicate the number of elector and the "faciliteit"
0 normal mode (3 electors, no "faciliteit")
1 faciliteit mode (3 electors, "faciliteit")
2 "voeren" mode (6 electors)
5)</comment></heading>
<code>*/
//============================================================================
//                                 M A I N
//============================================================================
#pragma argsused
// ---------------------------------------------------------------------------
void main ( int argc, char * argv[])
// ---------------------------------------------------------------------------
{
    int JMP_Ret = 0;
    TYPE_READER Intern_Type;

    Setup_Video();                     // switch to graphic mode
    ConnectAlarmBox();                 // Ensures alarm box is connected
    Reset_Lights();                    // Clears the alarm box lights
    harderr( Floppy_Error_Handler);    // Redirect device errors to application

    // process command line arguments
    Init_Languages( argv[1], argv[5] );          // initialize the languages

    // use lang_admin until we read the presidents card
    switch( atoi( argv[5]))
    {
        case 1: // French - fall thru intended
        case 2: // Dutch  - fall thru intended
        case 4: // German
        {
            Working_Language( ( atoi( argv[5])/2));
            break;
        }
    }

#ifdef _DEMO
    if( atoi( argv[2]) != 500)
#else
    if( atoi( argv[2]) == 500)
#endif
        Error_Exit( M_BAD_ARGUMENTS);

    switch( argc)
    {
        case 6 :
            PenCursor     = atoi( argv [3]);
            break;
        default:

            Error_Exit( M_BAD_ARGUMENTS);
            break;
    }

    InitApplVariables( atoi( argv[4] ));

    // assign and test the Intern card reader
#ifdef _DEMO
    Intern_Type = MCT145_1A;
#else
    Intern_Type = (TYPE_READER) atoi( argv[2]);
#endif

    if(( Intern_Reader = Open_Command( Intern_Type, COM2)) == NULL)
        Error_Exit( M_ERR_READ1);

    Display_Message( M_WAIT, NO, 0, NO, 9, 10, 1, 1);
    Test_Lightpen();
    Determine_Disk();

#ifdef _DEMO
    // display demo_msg1 & demo_msg2
    Display_Message( M_DEMO_MSG1, YES, 0, NO, 9, 10, 1, 1);
    Display_Message( M_DEMO_MSG2, YES, 0, NO, 9, 10, 1, 1);
    // end display demo-messages
#endif _DEMO

    // eject the remaining cards
    if( Reader_Card_Position( Intern_Reader) != NO_CARD)
    {
        Error_Message( M_TAKEOUT);
        Reader_Eject_Card( Intern_Reader);
        Reader_Wait_Card_Removed( Intern_Reader);
    }

    Process_Password();

    // Load the datas
    if( Setup_Database() == ABORTED)
    {
        Reader_Eject_Card( Intern_Reader);
        Error_Exit( M_BAD_DATA_DISK);
    }

    Get_Max_Candidates_Per_Election();
    Format_Blanco_Card();                   // build the 'not voted' card image

    // check printer and upload logo
    if ( initializeTicketing())
    {
       // something went wrong...
       Error_Exit( M_NO_PRINTER);
    }

    // Welcome message, ask president to push the button with the lightpen
    Display_Message( M_TEST_LIGHTPEN , YES, 0, NO);

    // Remove bootdisk procedure before we dive into the infinite loop...
    Reader_Eject_Card( Intern_Reader);
    Display_Message( M_REMOVE_CARD_AND_DISK, NO, 0, NO);
    Reader_Wait_Card_Removed( Intern_Reader);
    while( ! IsFloppyRemoved());
    sleep( 5);

    // install the restart point for the alarm box reset button
    JMP_Ret = setjmp( JMP_Restart_Loop);

    if( JMP_Ret == 0)                      // first time declaration
    {
        Jump = TO_RESTART_LOOP;
        Reset_Lights();
    }
    else                                   // jump has occured
    {
        Fault_Counter = 0;
        Send_Command( Intern_Reader, INITIALIZE, NULL);
        Reader_Eject_Card( Intern_Reader);
        Test_Lightpen();
        Remove_Card();
    }

    //////////////////////////////////////////////////
    // INFINITE LOOP -> begin of the real MAV program

    for(;;)
    {
        // Reset the working language for a bilingual election

        // Use the code below to check memory stability
//      printf( "%lu\n", coreleft());
//      sleep(5);

        if( Root.Language [1] != NONE)
            Working_Language( NONE);

        Display_Message( M_INPUT_CARD_FOR_VOTE);

        if( Reader_Request_Unformatted( Intern_Reader) != HOME)
            Process_Fault( M_CARD_NOT_VALID1, M_CARD_NOT_VALID1P);
        else if( Reader_Physical_Read_Card( Intern_Reader, card_data) != HOME)
            Process_Fault( M_CARD_NOT_VALID3, M_CARD_NOT_VALID3P);
        else
        {   // the inserted card must be not voted

            // WARNING NEED MODIFY (more than one type of blanco card)
            CurrentElector = card_data[8 + PositionCardTypeElector - 1]; // stefG
            CurrentTypeElector = CurrentElector - 0x30 + 1;  //stefG

            #if _DEBUGMODE
               sprintf( gcDcio,
                  "CurrentElector[%d] CurrentTypeElector[%d] PositionCardTypeElector[%d]",
                  CurrentElector, CurrentTypeElector, PositionCardTypeElector); dcio();
            #endif // _DEBUGMODE

            Format_Blanco_Card(); // build the 'not voted' card image with new flag

            // NEED TO TEST BYTE OF BLANCO CARD BETWEEN
            // [0 ..MAX_CARD_BUF-1+8+1] and [MAX_CARD_BUF-1+8+1..END]

            // if( ! strncmp( Blanco_Card, card_data, MAX_CARD_BUF)) // == 0 when equal

            // 20040106 avn - acces to special election of 2004 is not allowed
            // unless the elector votes for a party of the dutch languagegroup at the
            // Conseil Regional Bruxelles-Capitale
            AllowSpecialElection( FALSE);

            if ( ( Split_Card_Buffer( ( unsigned char *) card_data, & Allready_Voted,
                                      ( unsigned char *) Card ) == SUCCEEDED &&
                  Count_Vote( CHECK_IT ) == SUCCEEDED   &&
                  // canceled cards must be visualized also
                  ( Allready_Voted)
                 ) ||
                 !strncmp( Blanco_Card, card_data, Vote_Size_On_Card + 8 )
               )
            {
                    Fault_Counter = 0;

               // reset again ( could be modified by Count_Vote...)
               AllowSpecialElection( FALSE);
                Election_Loop();

                if( Allready_Voted )
                    Display_Message( M_GOODBYE_AGAIN );                 // Card is a not voted one
            }
            else                                   // Card is already voted
            {
               #if _DEBUGMODE
                  sprintf( gcDcio, "Bad card[%s]", card_data); dcio();
               #endif // _DEBUGMODE

                Process_Fault( M_CARD_NOT_VALID2, M_CARD_NOT_VALID2P);
            }
        }

        // erase votes from memory before we eject the card
        ClearVoteBuffers();
        memset( card_data, 0, sizeof( card_data));

        Reader_Eject_Card( Intern_Reader);          // eject card
        Reader_Wait_Card_Removed( Intern_Reader);   // wait until card is removed
        sleep( 2);

    } // eoforever
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
