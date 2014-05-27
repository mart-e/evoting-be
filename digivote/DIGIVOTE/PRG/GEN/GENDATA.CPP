/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gendata.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENDATA.CPP  $</workFile>
<revision>$Revision: 1.11 $</revision>
<workFileTimeStamp>$Modtime:   Jan 26 2005 16:13:14  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENDATA.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/04 14:23:10 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENDATA.CPP</fileName>
  <creationDate>20/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Gestion des données et routines d'accès/traitement</fr>
    <nl>Beheer vd gegevens en toegang/verwerking routines </nl></description>
  <history>
   01/06/93: revision history starting date
   20/09/93: all the functions that deal with the new data structure are
             modified to support the new language processing and the colleges.
   11/10/93: function Find_Session was added, to provide a way to retrieve the
             new tag entry in the session list (= the total number of
             diskettes, tag 8)
   22/11/93: '\t0' added as party number for the blanco vote line in the b001
             file.
   07/03/94: The floppy may not change in the routine Read_In_Line.

   01/09/2001: AVN - Implementation of new AES encryption algorithm
</history>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <direct.h>  // _chdrive()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <alloc.h>   // coreleft()

#include "..\gen\gentype.h"
#include "..\gen\genlang.h"
#include "..\gen\gencard.h"
#include "..\gen\gendata.h"
#include "..\gen\gencryp.h"
#include "..\gen\genflop.h"
#include "..\gen\genstr.h"

#ifdef _APPLICATION_URN
   #include "..\urn\urnmes.h"
   #include "..\urn\urnreco.h"

   #include "..\sha1\hashapi.h"
#endif

#ifdef _APPLICATION_MAV
   #include "..\mav\mavmes.h"
   #include "..\mav\display.h"
#endif

#define _DEBUGMODE      FALSE      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
#endif  // _DEBUGMODE
   #include "..\tools\debug.h"
//;TODO: reset between debugs!!!

//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---
typedef union {
   BYTE Id;
   struct {
      unsigned int Bits       : 2;
      unsigned int UsageFlag  : 1;
      unsigned int            : 5;   // placeholder
   } bf;
} FirstCollegeType ;

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
// these functions are private!
static void Data_Error_Exit( int M_Index);
static void Int_Two_Bytes( int Int, char TB [2]);
static void Init_Vote_0( VOTE_TYPE_0* Vote_0);
static void Init_Vote_1( VOTE_TYPE_1* Vote_1);
static void Align_Vote_On_8_Bytes( BYTE* Text, int* Total_Length);

static void Convert_Vote_Buffer_To_Card_Buffer (
   int   Flag,               // the card's usage flag
   BYTE* Vote_Buffer,        //
   int*  Vote_S,             //
   BYTE* Card_Buffer,        //
   int*  Card_S              //
   );
static void Convert_Card_Buffer_To_Vote_Buffer (
   int*  Flag,               // the card's usage flag
   BYTE* Card_Buffer,
   int* Card_S,
   BYTE* Vote_Buffer,
   int*  Vote_S
   );
static void Build_Card_Buffer (
   BYTE* Data,               // the votes per election
   int   Flag,               // the card's usage flag
   BYTE* Card_Buffer         // physical data on the card
   );

static void Trunc_Key( char* Inkey, char* Outkey);

#ifdef _APPLICATION_MAV
   static void Append_To_Card(char * Vote, int Length);
   static void Format_Vote_0();
   static void Format_Vote_1();
   static void Set_Candidate_Bit( int Ind);
#endif

static ElectionList Find_Election( int ElectionInd);
static CollegeList Find_College( int ElectionInd, int CollegeInd);
static PartyList Find_Party( int ElectionInd, int CollegeInd, int PartyInd);

#ifdef _APPLICATION_MAV
   static CandidateList Find_Candidate(
      int ElectionInd,
      int CollegeInd,
      int PartyInd,
      CandidateType Type,
      int CandidateInd);
#endif
static void Init_Parser( void);
static BOOL Get_Token();

static void Parse_Session( void);
static void Parse_Election( void);
static void Parse_College( void);
static void Parse_Party( void);
static void Parse_Candidate( void);

#ifdef _APPLICATION_MAV
   static void Parse_Candidate_Name( int ElectId);
   static void Get_Max_Candidates(
      int ElectionInd, int CollegeInd, int PartyInd);
#endif

#ifdef _APPLICATION_URN
   static RET_CODE Store_Totals( void);
   static RET_CODE Emit_Session( SessionList Link);
   static RET_CODE Emit_Election( ElectionList Link);
   static RET_CODE Emit_Party(
      ElectionList HeadE, CollegeList HeadCol,
      PartyList Link);
   static RET_CODE Emit_Candidate(
      ElectionList HeadE, CollegeList HeadCol,
      PartyList HeadP, CandidateType Type,
      CandidateList Link);
#endif

#ifdef _APPLICATION_URN
   static int InitVotesCounter (
      VotesCounterType* VotesCounter
      );
   static int IncrementVotesCounter (
      VotesCounterType* VotesCounter
      );
   static int IsVotesCounterCorrupt (
      VotesCounterType* VotesCounter
      );
#endif

static RET_CODE Count_Vote_Candidate(
   OPCODE_COUNT Operation, PartyList Head,
   enum CandidateType Type, int CandidateInd);

static RET_CODE TallyMultiVote (
   OPCODE_COUNT Operation,             // check only or check and count
   ElectionList Head                   // election data
   );
static RET_CODE TallySingleVote (
   OPCODE_COUNT Operation,             // check only or check and count
   ElectionList Head                   // election data
   );

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern INTEGRITY_STATE Flag_Integrity;

extern char Floppy_MASTER[];
extern char* Floppy_Current;

//rvh
extern Select_Reader* Intern_Reader;

extern struct ElectionMain Root;
extern struct Select_College Colleges;
extern struct Select_Party Parties;
extern struct Select_Candidate Candidates;

extern char Vote_Data [];
extern char Blanco_Votes [];

extern int NUMBER_TYPE_ELECTOR; // stefG number of elector type
extern int CurrentTypeElector;  // NEW shared current type of elector to format
extern int NbrOfCounters;
extern ELECTOR_TYPE ElectorTypesUsed[];
extern ELECTOR_TYPE ElectorTypes[];
extern int PermissionsUsed[MAX_TYPE_ELECTOR - 1][MAX_ELECTIONS -1];

#ifdef _APPLICATION_URN
   extern char * TOTALS_FILE_NAME;
   extern char * TOTALS_TEMP_FILE;
   extern int Totals_Temp_File;
#endif

// tmt
extern ScrExit(char*);

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
#ifdef _TEST
   const long int MAX_CARDS_IN_BAC = 100;  // only for testing
#else
   const long int MAX_CARDS_IN_BAC = 2000; // development version
#endif

const int MAX_RECORD_LENGTH = 512;

char* TOTALS_TEMP_COPY = "a:B001.TMP";

char* DATA_FILE_NAME = "C:\\B021";  // template for data files
char* DATA_MAC_NAME  = "C:\\B321";  // template for mac on data files

char FILE_NAME[15];
int Data_File;

//rvh
static int Vote_Size = 0;              // length of the vote,max 200
int Vote_Size_On_Card = 0;             // CHANGE !!! Vote length on the card
                                       // (no holes), max 104
int PositionCardTypeElector = 0;       // stefG relative position regarding
                                       // of the begin of the vote on card
int PositionMemTypeElector = 0;        // stefG relative position regarding
                                       // of the begin of the vote in mem

char Card [MAX_VOTE_BUF]; // the logical card data converted from physical one
int Offset;               // offset into card data

char Blanco_Card[MAX_CARD_BUF+1];       // the physical blanco card buffer
   //!\ 20040224 avn - MAX_CARD_BUF+1 added because the buffer is passed to
   //!\ the cardreader as a string...

// tmt
struct cardinfo Card_Info_Per_Election[MAX_ELECTIONS];

//rvh
char InLine [MAX_RECORD_LENGTH];
char Buffer [MAX_RECORD_LENGTH];
char Token [MAX_RECORD_LENGTH];
int  i, j;

VOTE_TYPE_0 Vote_0;  // temporary buffer used for single vote mode
VOTE_TYPE_1 Vote_1;  // temporary buffer used for multiple vote mode

//AVN
BOOL storeTotalsIV;

int Allready_Voted = 0;
int Vote_0_Type;

int CurrentTypeElectorInBac;   // NEW current type of elector in BAC

BOOL gIsSpecialElectionAllowed;

//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="TallyMultiVote" confidential="yes">
<heading>
  <description>
    <en>Count or check the votes in one election</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>SUCCEEDED = vote is valid,
        ABORTED   = vote is not valid.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>OPCODE_COUNT</datatype><name>Operation</name>
      <description><en>CHECK_IT or EXECUTE, no default value.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>Head</name>
      <description><en>the election linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype><name>Vote_0</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype><name>Vote_1</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
  </parameters>
  <usage/><checks/>
  <comment>Because the party and candidate numbers are coded differently on
card we must first convert them before any use. Refer to the card description
and the different structure definitions.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE TallyMultiVote (
   OPCODE_COUNT Operation,             // check only or check and count
   ElectionList Head                   // election data
   )
// ---------------------------------------------------------------------------
{
      int   CollegeInd;
      int   PartyInd;
      int   Vote_Type;                 // single vote or multiple vote mode
      int   INDEX;                     // index in the vote data
      int   i;                         // index on candidates
      int   Ind;
      BOOL  EffectivesVoted;           // effectives voted
      BOOL  SuppliantsVoted;           // suppliants voted
      PartyList LinkP;
      int   Status;                    // error code


   Status = 0;

   // convert voted college to an int
   if ( CONSEIL_BXL == Head->Ind)
   {
      // 20040114 avn - remember CollegeInd contains GroupInd since there are no
      // colleges for this election...
      CollegeInd = 0;
   }
   else
   {
      CollegeInd = Vote_1.College - '0';
   }

#ifdef _APPLICATION_URN
   if ( Operation == EXECUTE)
   {
      // number of votes in this election
      if ( IncrementVotesCounter( & Head->TotalVotes) != 0)
      {
         Status = 10;
         goto error;
      }
   }
#endif //#ifdef _APPLICATION_URN

   // convert voted party bytes to an int
   Two_Bytes_Int( & PartyInd, Vote_1.Party);

   // blanc vote
   if ( PartyInd == 0)
   {
      // all candidates must be zero in this case
      for ( INDEX = 0; INDEX < ((MAX_CANDIDATES+2) / 3); ++INDEX)
      {
         if ( Vote_1.Candidate[INDEX].bit1 || Vote_1.Candidate[INDEX].bit2 ||
              Vote_1.Candidate[INDEX].bit3)
         {
            Status = 15;
            goto error;
         }
      } // eo for

#ifdef _APPLICATION_URN
      if ( Operation == EXECUTE)
      {
         if ( IncrementVotesCounter( & Head -> BlankVotes) != 0)
         {
            Status = 20;
            goto error;
         }
      }
#endif //#ifdef _APPLICATION_URN

      return SUCCEEDED;
   }

   // retrieve voted party ( if not found, vote not valid)
   LinkP = Find_Party( Head -> Ind, CollegeInd, PartyInd);
   if ( LinkP == NULL)
   {
      #if 0 //_DEBUGMODE
         sprintf( gcDcio, "Party?:[%d][%d][%d]",
            Head->Ind, CollegeInd, PartyInd); dcio();
      #endif // _DEBUGMODE
      Status = 40;
      goto error;
   }

   // any effectives voted?
   EffectivesVoted = FALSE;
   for ( i = 0; ( i < LinkP -> NbrOfEffectives && !EffectivesVoted); ++i)
   {
      INDEX = i/3;

      switch ( i%3)
      {
         case 0:
            if ( Vote_1.Candidate[INDEX].bit1) EffectivesVoted = TRUE;
            break;

         case 1:
            if ( Vote_1.Candidate[INDEX].bit2) EffectivesVoted = TRUE;
            break;

        case 2:
            if ( Vote_1.Candidate[INDEX].bit3) EffectivesVoted = TRUE;
            break;
      }
   } // eo for

   // any suppliants voted?
   SuppliantsVoted = FALSE;
   for ( i = 0; ( i < LinkP -> NbrOfSuppliants && !SuppliantsVoted); ++i)
   {
      Ind = i + LinkP -> NbrOfEffectives;
      INDEX = Ind/3;

      switch ( Ind%3)
      {
         case 0:
            if ( Vote_1.Candidate[INDEX].bit1) SuppliantsVoted = TRUE;
            break;

         case 1:
            if ( Vote_1.Candidate[INDEX].bit2) SuppliantsVoted = TRUE;
            break;

         case 2:
            if ( Vote_1.Candidate[INDEX].bit3) SuppliantsVoted = TRUE;
            break;
      }
   } // eo for

   // validate vote type
   Vote_Type = Vote_1.TypeVote - '0';
   if ( Vote_Type > 3 ) Vote_Type -= 4;

   if ( Vote_Type < 0 || Vote_Type > 3)
   {
      Status = 50;
      goto error;
   }

   // head vote only
   if ( 0 == Vote_Type)
      if ( !( !EffectivesVoted && !SuppliantsVoted))
      {
         Status = 60;
         goto error;
      }

   if ( 1 == Vote_Type || 3 == Vote_Type)
   {
      // only effective candidates voted
      if ( 1 == Vote_Type)
         if ( !( EffectivesVoted && !SuppliantsVoted))
         {
            Status = 70;
            goto error;
         }

      // effective and suppliant candidates voted
      if ( 3 == Vote_Type)
         if ( !( EffectivesVoted && SuppliantsVoted))
         {
            Status = 80;
            goto error;
         }

      // individual votes
      for ( i = 0; i < LinkP -> NbrOfEffectives; ++i)
      {
         INDEX = i/3;

         switch ( i%3)
         {
            case 0:
               if ( Vote_1.Candidate [INDEX].bit1)
                  if ( Count_Vote_Candidate( Operation, LinkP, Effective, i+1) ==
                     ABORTED)
                  {
                     Status = 90;
                     goto error;
                  }
               break;

            case 1:
               if ( Vote_1.Candidate [INDEX].bit2)
                  if ( Count_Vote_Candidate( Operation, LinkP, Effective, i+1) ==
                     ABORTED)
                  {
                     Status = 100;
                     goto error;
                  }
               break;

           case 2:
               if ( Vote_1.Candidate [INDEX].bit3)
                  if ( Count_Vote_Candidate( Operation, LinkP, Effective, i+1) ==
                     ABORTED)
                  {
                     Status = 110;
                     goto error;
                  }
               break;
         }
      } // eo for
   }

   if ( 2 == Vote_Type || 3 == Vote_Type)
   {
      // only suppliant candidates voted
      if ( 2 == Vote_Type)
         if ( !( SuppliantsVoted && !EffectivesVoted))
         {
            Status = 120;
            goto error;
         }

      // individual votes
      for ( i = 0; i < LinkP -> NbrOfSuppliants; ++i)
      {
         Ind = i + LinkP -> NbrOfEffectives;
         INDEX = Ind/3;

         switch ( Ind%3)
         {
            case 0:
               if ( Vote_1.Candidate [INDEX].bit1)
                  if ( Count_Vote_Candidate( Operation, LinkP, Suppliant, i+1) ==
                     ABORTED)
                  {
                     Status = 132;
                     goto error;
                  }
               break;

            case 1:
               if ( Vote_1.Candidate [INDEX].bit2)
                  if ( Count_Vote_Candidate( Operation, LinkP, Suppliant, i+1) ==
                     ABORTED)
                  {
                     Status = 134;
                     goto error;
                  }
               break;

            case 2:
               if ( Vote_1.Candidate[INDEX].bit3)
                  if ( Count_Vote_Candidate( Operation, LinkP, Suppliant, i+1) ==
                     ABORTED)
                  {
                     Status = 136;
                     goto error;
                  }
               break;
         }
      } // eo for
   }

#ifdef _APPLICATION_URN
   // update stat counters
   if ( Operation == EXECUTE)
   {
      switch ( Vote_Type)
      {
         case 0 : // party vote
         {
            if ( IncrementVotesCounter( & LinkP -> HeadVotes) != 0)
            {
               {
                  Status = 162;
                  goto error;
               }
            }
            break;
         }
         case 1 : // effective candidate only vote
         {
            if ( IncrementVotesCounter( & LinkP -> EffectiveVotes) != 0)
            {
               {
                  Status = 164;
                  goto error;
               }
            }
            break;
         }
         case 2 : // suppliant candidate only vote
         {
            if ( IncrementVotesCounter( & LinkP -> SuppliantVotes) != 0)
            {
               {
                  Status = 166;
                  goto error;
               }
            }
            break;
         }
         case 3 : // effective and suppliant vote
         {
            if ( IncrementVotesCounter( & LinkP -> MixedVotes) != 0)
            {
               {
                  Status = 168;
                  goto error;
               }
            }
            break;
         }
      } // eo switch
   }
#endif //#ifdef _APPLICATION_URN

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "TallyMultiVote - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
//</code></function>


/*<function id="TallySingleVote" confidential="yes">
<heading>
  <description>
    <en>Count or check the votes in one election</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>SUCCEEDED = vote is valid,
        ABORTED   = vote is not valid.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>OPCODE_COUNT</datatype><name>Operation</name>
      <description><en>CHECK_IT or EXECUTE, no default value.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>Head</name>
      <description><en>the election linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype><name>Vote_0</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype><name>Vote_1</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
  </parameters>
  <usage/><checks/>
  <comment>Because the party and candidate numbers are coded differently on
  card we must first convert them before any use.
  Refer to the card description and the different structure definitions.

  /!\ until now there are no elections using the 'single' election type mode /!\
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE TallySingleVote (
   OPCODE_COUNT Operation,             // check only or check and count
   ElectionList Head                   // election data
   )
// ---------------------------------------------------------------------------
{
      PartyList   LinkP;
      int         CollegeInd, PartyInd, CandidateInd;
      int         EffectivesVoted;           // effectives voted
      int         SuppliantsVoted;           // suppliants voted
      int         Vote_Type;
      int         Status;                    // error code


   Status = 0;

   CollegeInd = Vote_0.College - '0';

   Two_Bytes_Int( & PartyInd, Vote_0.Party);
   Two_Bytes_Int( & EffectivesVoted, Vote_0.NrCandidate);
   Two_Bytes_Int( & SuppliantsVoted, Vote_0.NrSuppleant);

   if ( PartyInd == 0)  // blank votes
   {
      if ( ! ( !EffectivesVoted && !SuppliantsVoted))
      {
         Status = 10;
         goto error;
      }

#ifdef _APPLICATION_URN
      if ( Operation == EXECUTE)
      {
         if ( IncrementVotesCounter( & Head -> BlankVotes) != 0)
         {
            Status = 12;
            goto error;
         }
      }
#endif //#ifdef _APPLICATION_URN

   }

   else  // non blank votes
   {
      // party not found, vote not valid
      LinkP = Find_Party( Head -> Ind, CollegeInd, PartyInd);
      if ( LinkP == NULL)
      {
         Status = 20;
         goto error;
      }

      Vote_Type = Vote_0.TypeVote - '0';
      if ( Vote_Type > 3 ) Vote_Type -= 4;

      switch( Vote_Type) // Attention : '0' = ASCII 0 !
      {
         case 0 : // party vote
            if ( !( !EffectivesVoted && !SuppliantsVoted))
            {
               Status = 30;
               goto error;
            }

#ifdef _APPLICATION_URN
            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> HeadVotes) != 0)
               {
                  Status = 40;
                  goto error;
               }
            }
#endif //#ifdef _APPLICATION_URN

            break;

         case 1 : // effective candidate only vote
            if ( !( EffectivesVoted && !SuppliantsVoted))
            {
               Status = 50;
               goto error;
            }

#ifdef _APPLICATION_URN
            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> EffectiveVotes) != 0)
               {
                  Status = 60;
                  goto error;
               }
            }
#endif //#ifdef _APPLICATION_URN

            Two_Bytes_Int( & CandidateInd, Vote_0.NrCandidate);
            if ( Count_Vote_Candidate( Operation, LinkP, Effective,
                  CandidateInd) == ABORTED)
            {
               Status = 62;
               goto error;
            }
            break;

         case 2 : // suppliant candidate only vote
            if ( !( !EffectivesVoted && SuppliantsVoted))
            {
               Status = 70;
               goto error;
            }

#ifdef _APPLICATION_URN
            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> SuppliantVotes) != 0)
               {
                  Status = 80;
                  goto error;
               }
            }
#endif //#ifdef _APPLICATION_URN

            Two_Bytes_Int( & CandidateInd, Vote_0.NrSuppleant);
            if ( Count_Vote_Candidate( Operation, LinkP, Suppliant,
                  CandidateInd) == ABORTED)
            {
               Status = 82;
               goto error;
            }
            break;

         case 3 : // effective and suppliant vote
            if ( !( EffectivesVoted && SuppliantsVoted))
            {
               Status = 90;
               goto error;
            }

#ifdef _APPLICATION_URN
            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> MixedVotes) != 0)
               {
                  Status = 100;
                  goto error;
               }
            }
#endif //#ifdef _APPLICATION_URN

            Two_Bytes_Int( & CandidateInd, Vote_0.NrCandidate);
            if( Count_Vote_Candidate( Operation, LinkP,
                                      Effective, CandidateInd) == ABORTED)
            {
               Status = 110;
               goto error;
            }

            Two_Bytes_Int( & CandidateInd, Vote_0.NrSuppleant);
            if ( Count_Vote_Candidate( Operation, LinkP, Suppliant,
                  CandidateInd) == ABORTED)
            {
               Status = 112;
               goto error;
            }
            break;

         default : // bad vote type
            Status = 120;
            goto error;
      } // eo switch
   } // eo else non blank votes

#ifdef _APPLICATION_URN
   if ( Operation == EXECUTE)
   {
      // maximum number of votes in this election
      if ( IncrementVotesCounter( & Head->TotalVotes) != 0)
      {
         {
            Status = 180;
            goto error;
         }
      }
   }
#endif //#ifdef _APPLICATION_URN

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "TallySingleVote - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
//</code></function>



/*<function id="Data_Error_Exit">
<heading>
  <description>
    <en>Displays an error message and locks the system</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>M_Index</name>
      <description><en>a message number</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype>
      <name>Floppy_MASTER[]</name>
      <description><en>the floppy MASTER label name</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char*</datatype>
      <name>Floppy_Current</name>
      <description><en>the current floppy label name</en></description>
    </parameter>
  </parameters>
  <usage>if ( Error)
            Data_Error_Exit( M_ERROR);</usage>
  <checks/>
  <comment>this function is used when an error occurs when reading the
data, due to a floppy error or corrupted datas.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Data_Error_Exit (
   int M_Index)
// ---------------------------------------------------------------------------
{
#ifdef _APPLICATION_ETC
    ScrExit("ERROR DATABASE FILE (PROBABLY WRONG PASSWORD)");
#endif

#ifdef _APPLICATION_URN
   Display_Message( M_Index);

   if( Floppy_Current == Floppy_MASTER)
      Display_Message( E_REBOOT_BACKUP, 0, 1);
   else
      Display_Message( E_FATAL_DISKETTE, 0, 1);

   for(;;);
#endif

#ifdef _APPLICATION_MAV
    Error_Exit( M_Index);
#endif
}
//</code></function>

/*<function id="Int_Two_Bytes">
<heading>
  <description>
    <en>splits an integer value smaller than 100 in tenth and unit.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>Int</name>
      <description><en>the integer to split.</en></description></parameter>
    <parameter>
      <datatype>char</datatype>
      <name>Table [2]</name>
      <description><en>the array with the splitted values</en></description>
    </parameter>
  </parameters>
  <usage>Int_Two_Bytes( 25, Array);    -&gt; Array[0] = 2, Array[1] = 5</usage>
  <checks>there is no check, but you should make sure not to use integer
values greater than 99, because the function is not intended to
work with values greater than 99.</checks>
  <comment>Because we can't put a value greater than 9 on the card, we must
convert it to be able to put it on card and to rebuild it from
card. The 10 values we can code on the card are from '0' to '9',
so we must add/retrieve '0' to the decimal values on card.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Int_Two_Bytes (
   int Int, char Table [2])
// ---------------------------------------------------------------------------
{
   Table [0] = Int / 10 + '0';
   Table [1] = Int % 10 + '0';

   return;
}
//</code></function>

/*<function id="Two_Bytes_Int">
<heading>
  <description>
    <en>rebuilds an integer value smaller than 100 in tenth and unit.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter>
      <datatype>int*</datatype>
      <name>Int</name>
      <description><en>the pointer to the integer where to put the rebuilded value.</en></description></parameter>
    <parameter>
      <datatype>char</datatype>
      <name>Table [2]</name>
      <description><en>the array with the splitted values</en></description></parameter></parameters>
  <usage>Two_Bytes_Int( &amp;Int, Array); -&gt; Int = 25</usage>
  <checks>there is no check, but you should make sure not to use integer
values greater than 99, because the function is not intended to
work with values greater than 99.</checks>
  <comment>Because we can't put a value greater than 9 on the card, we must
convert it to be able to put it on card and to rebuild it from
card. The 10 values we can code on the card are from '0' to '9',
so we must add/retrieve '0' to the decimal values on card.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Two_Bytes_Int (
   int * Int, char Table [2])
// ---------------------------------------------------------------------------
{
   * Int = ( Table [0] - '0') * 10  +  ( Table [1] - '0');

   return;
}
//</code></function>


/*<function id="Format_Blanco_Card" confidential="yes">
<heading>
  <description>
    <en>builds the blanco vote card data</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter>
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>AESKEY</datatype>
      <name>sKey</name>
      <description><en>the session key</en></description></parameter>
    <parameter type="global">
      <datatype>BYTE</datatype>
      <name>mavSessionKey [DESKEYLEN+1]</name>
      <description><en>the random MAV session key</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Blanco_Votes []</name>
      <description><en>the array with the blanco vote</en></description>
    </parameter>
  </parameters>
  <usage>Format_Blanco_Card();</usage>
  <checks/>
  <comment>This function builds the blanco vote data structure, this vote
will be used to initialize the voting cards.
The structure of this vote must be identical to a normal vote,
only the datas are for a white vote.
The key used to compute the MAC on the card is normally a
random generated key, but for the blanco vote we use the key
SESSION. We must truncate the key because this will be the form
under which it is saved on card. It is used in this form to
compute the MAC on the blanco vote.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Format_Blanco_Card ( void)
// ---------------------------------------------------------------------------
{
      BYTE Card_Buffer [MAX_CARD_BUF+1];

   memset( Card_Buffer, NULL, sizeof( Card_Buffer));

   Trunc_Key( sKey.data, mavSessionKey);    // trunc the SESSION key to the MAVSESSION key
                                                // note: limited to first 8 bytes of SESSION key
                                                // since size of MAVSESSION key remains 8 bytes
                                                // due to limited capacity of the magnetic card
   Format_Blanco_Votes();                   // Build data in Blanco_Votes
                                             // Array of vote type 0 and 1 (all vote = blanco vote)
                                             // vote size = length of this buffer
   Build_Card_Buffer( Blanco_Votes, 0, Card_Buffer);

   memcpy( Blanco_Card, Card_Buffer, sizeof( Blanco_Card));

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "formatBC2 - bc[%s] s[%d]  s[%d]",
         Blanco_Card, sizeof( Card_Buffer), sizeof( Blanco_Card)); dcio();
   #endif // _DEBUGMODE

   return;
}
//</code></function>


/*<function id="Format_Blanco_Votes" confidential="yes">
<heading>
  <description>
    <en>builds a buffer corresponding to the current election structure
where each election is voted blanco</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter>
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>ElectionMain</datatype>
      <name>Root</name>
      <description><en>the data linked list</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Vote_Size</name>
      <description><en>the length of a Blanco_Vote, in fact the Vote length</en></description></parameter></parameters>
  <usage>Format_Blanco_Votes();</usage>
  <checks>we check to see if there is not too many elections for our
card length, the card length is used to put the Key_MAV_SESSION,
a usage flag, the vote and the MAC computed on the vote.
So we already need 8+0+8 card positions for the integrity
information, the remaining space can be used for the votes.</checks>
  <comment>goes through the Root linked list and builds a buffer for each
found election with a blanco vote for that election type.
The resulting vote is then aligned on a multiple of AESBLOCKLEN, because
the encryption routines need that kind of format.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Format_Blanco_Votes ( void)
// ---------------------------------------------------------------------------
{
    VOTE_TYPE_0 Blanco_Vote_0;    // buffer type 0 vote
    VOTE_TYPE_1 Blanco_Vote_1;    // buffer type 1 vote
    int Length, Blanco_Offset = 0;
    char * Buffer;

    Init_Vote_0( & Blanco_Vote_0); // initialize each vote type Simple or Multi
    Init_Vote_1( & Blanco_Vote_1); // initialize each vote type Simple or Multi

   // for each election ...
   for( ElectionList Link = Root.FirstElection; Link !=NULL; Link = Link->Next)
   {
       switch( Link->Type)
       {
       case 0 : Buffer = (char *) & Blanco_Vote_0;  // index by char data
                Length = sizeof( VOTE_TYPE_0);
                break;
       case 1 : Buffer = (char *) & Blanco_Vote_1;  // index by char data
                Length = sizeof( VOTE_TYPE_1);
                break;
       }

//rvh
       if( (Blanco_Offset + Length) > MAX_VOTE_BUF) // test for limit
              Error_Exit( E_TOO_MANY_ELECTIONS);

       for( int i=0; i < Length; ++i) // appends to the Blanco_Votes buffer
       {
         Blanco_Votes [Blanco_Offset++] = Buffer [i];
       }
//rvh
   }
//rvh   Align_Vote_On_8_Bytes ( Blanco_Votes, & Blanco_Offset);     // format the vote
   Vote_Size = Blanco_Offset;    // this is the length of the vote in the buffer
//rvh
   PositionMemTypeElector = Blanco_Offset+1; //stefG : position of the elector
                                             //type in memory BAC

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "FormatBV end - vs[%d] pmte[%d] offset[%d]",
         Vote_Size, PositionMemTypeElector, Blanco_Offset); dcio();
   #endif // _DEBUGMODE

   return;
}
//</code></function>


/*<function id="Init_Vote_0" confidential="yes">
<heading>
  <description>
    <en>initialize each vote type to a blanco vote</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter>
      <datatype>VOTE_TYPE_0*</datatype>
      <name>Vote_0</name>
      <description><en>the vote structure to initialize</en></description>
    </parameter>
  </parameters>
  <usage>Init_Vote_0( &amp;Blanco_Vote_0);</usage>
  <checks/>
  <comment>There are two types of vote, one for a single candidate choice
and one for a multiple candidate choice.
Refer to the structure description for a complete explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Init_Vote_0 (
   VOTE_TYPE_0* Vote_0)
// ---------------------------------------------------------------------------
{
    Vote_0 -> College         = '0';
    Vote_0 -> Party [0]       = '0';
    Vote_0 -> Party [1]       = '0';
    Vote_0 -> TypeVote        = '0';
    Vote_0 -> NrCandidate [0] = '0';
    Vote_0 -> NrCandidate [1] = '0';
    Vote_0 -> NrSuppleant [0] = '0';
    Vote_0 -> NrSuppleant [1] = '0';
}
//</code></function>


/*<function id="Init_Vote_1" confidential="yes">
<heading>
  <description>
    <en>initialize each vote type to a blanco vote</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter>
      <datatype>VOTE_TYPE_1*</datatype>
      <name>Vote_1</name>
      <description><en>the vote structure to initialize</en></description>
    </parameter>
  </parameters>
  <usage>Init_Vote_1( &amp;Blanco_Vote_1);</usage>
  <checks/>
  <comment>There are two types of vote, one for a single candidate choice
and one for a multiple candidate choice.
Refer to the structure description for a complete explanation.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Init_Vote_1 (
   VOTE_TYPE_1* Vote_1)
// ---------------------------------------------------------------------------
{
    Vote_1 -> College   = '0';
    Vote_1 -> Party [0] = '0';
    Vote_1 -> Party [1] = '0';
    Vote_1 -> TypeVote  = '0';

    for( int i = 0; i < ( MAX_CANDIDATES + 2 ) / 3; ++i)
    {
        Vote_1 -> Candidate [i].dummy = 0x06;       // so Candidate [i] will go
        Vote_1 -> Candidate [i].bit1  = 0;          // from '0' to '7' so it is
        Vote_1 -> Candidate [i].bit2  = 0;          // acceptable for the card
        Vote_1 -> Candidate [i].bit3  = 0;          // 0x06 = 110 (+ 000 = '0')
    }
}
//</code></function>


/*<function id="Align_Vote_On_8_Bytes" confidential="yes">
<heading>
  <description>
    <en>complete a given array with '0' to reach the upper multiple of 8
boundary size. The aligned vote is returned in place.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description>
    </parameter>
    <parameter>
      <datatype>BYTE*</datatype>
      <name>Vote</name>
      <description><en>the array with the vote to align</en></description>
    </parameter>
    <parameter>
      <datatype>int*</datatype>
      <name>Total_Length</name>
      <description><en>the data length before and after the alignment</en></description>
    </parameter>
  </parameters>
  <usage>Align_Vote_On_8_Bytes( Vote, &amp;Vote_Length);</usage>
  <checks>check to see if the aligned data will not overflow the length
   available to store the data on the card along with the
   security data (8+0+8)</checks>
  <comment>this function must be called to format the data for encryption
   purpose.
   !! The fill character is '0' because we must be able to write
   it on card</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Align_Vote_On_8_Bytes (
   BYTE * Vote, int * Total_Length)
// ---------------------------------------------------------------------------
{
    int N_More = * Total_Length % 8;

    if( N_More)                                 // is the alignment necessary ?
    {
        N_More = 8 - N_More;

        for( int i = 0; i < N_More; i++)        // fill with '0'
            Vote [* Total_Length + i] = '0';

        * Total_Length += N_More;               // the new data length
    }

    if( * Total_Length > MAX_CARD_BUF - (8+0+8) ) // MAC - FLAG - SESSION
        Error_Exit( E_TOO_MANY_ELECTIONS);

    return;
}
//</code></function>


/*<function id="Convert_Vote_Buffer_To_Card_Buffer" confidential="yes">
<heading>
  <description><en>
    Converts the Vote_Buffer of 200 bytes with fixed vote
    structures to a Card_Buffer of 104 bytes with a
    variable vote length.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/>
    </parameter>
    <parameter>
      <datatype>BYTE*</datatype>
      <name>Vote_Buffer</name>
      <description><en>a pointer to the Vote_Buffer array</en></description>
    </parameter>
    <parameter>
      <datatype>int*</datatype>
      <name>Vote_S</name>
      <description/>
    </parameter>
    <parameter>
      <datatype>BYTE*</datatype>
      <name>Card_Buffer</name>
      <description><en>a pointer to the Card_Buffer array</en></description>
    </parameter>
    <parameter>
      <datatype>int*</datatype>
      <name>Card_S</name>
      <description/>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Vote_Size</name></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Vote_Size_On_Card</name>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Convert_Vote_Buffer_To_Card_Buffer (
   int   Flag,               // the card's usage flag
   BYTE* Vote_Buffer,
   int*  Vote_S,
   BYTE* Card_Buffer,
   int*  Card_S
   )
// ---------------------------------------------------------------------------
{
      int               i;
      int               length;
      int               Pointer_Vote_Buffer;
      int               Pointer_Card_Buffer;
      FirstCollegeType  FirstCollege;

   Pointer_Vote_Buffer = 0, Pointer_Card_Buffer = 0;

   for ( i = 0; i < MAX_ELECTIONS; i++ )
   {
      if (( 0 == i) && Flag)
      {
         // :TRICKY: put's the usage flag in the college number of the first election
         // assumes that flag is 0/1 and the collegenumber is first byte of VOTE_TYPE_1
         FirstCollege.Id = Vote_Buffer[Pointer_Vote_Buffer];

         // stuff the usage flag in the college byte and put it back
         FirstCollege.bf.UsageFlag = 1;
         Vote_Buffer[Pointer_Vote_Buffer] = FirstCollege.Id;
      }

      switch( Card_Info_Per_Election[i].Type_Election)
      {
         case 0:
            length = sizeof( VOTE_TYPE_0);                  // take size of typedef
            if ( Pointer_Card_Buffer + length > MAX_CARD_BUF) // test Overflow
               break;

            memcpy( &Card_Buffer[Pointer_Card_Buffer],      // merge data
               &Vote_Buffer[Pointer_Vote_Buffer], length);
            Pointer_Card_Buffer += length;                  // update write pointer
            Pointer_Vote_Buffer += length;                  // update read pointer
            continue;

         case 1:
            // test overflow
            length = sizeof( VOTE_TYPE_1) -
               ( ( ( MAX_CANDIDATES + 2) / 3) -
               ( ( Card_Info_Per_Election[i].Max_Candidates + 2) / 3) );
            if ( Pointer_Card_Buffer + length > MAX_CARD_BUF)
               break;

            // merge data
            memcpy( &Card_Buffer[Pointer_Card_Buffer],
               &Vote_Buffer[Pointer_Vote_Buffer], length);

            // update pointer
            Pointer_Card_Buffer += length;
            Pointer_Vote_Buffer += sizeof( VOTE_TYPE_1);
            continue;

         default:
            continue;

      } // eoswitch

      break;
   } // eofor

   *Vote_S = Pointer_Vote_Buffer;
   *Card_S = Pointer_Card_Buffer;

   return;
}
//</code></function>


/*<function id="Convert_Card_Buffer_To_Vote_Buffer" confidential="yes">
<heading>
  <description><en>Converts the Card_Buffer of 104 bytes with variable vote
    length to a Vote_Buffer of 200 bytes with a fixed vote
    structure.</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype><description/>
    </parameter>
    <parameter>
      <datatype>BYTE*</datatype><name>Card_Buffer</name>
      <description><en>a pointer to the Card_Buffer array</en></description>
    </parameter>
    <parameter><datatype>int*</datatype><name>Card_S</name><description/>
    </parameter>
    <parameter><datatype>BYTE*</datatype><name>Vote_Buffer</name>
      <description><en>a pointer to the Vote_Buffer array</en></description>
    </parameter>
    <parameter><datatype>int*</datatype><name>Vote_S</name><description/>
    </parameter>
    <parameter type="global"><datatype>int</datatype><name>Vote_Size</name>
    </parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Vote_Size_On_Card</name>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Convert_Card_Buffer_To_Vote_Buffer (
   int*  Flag,                         // the card's usage flag
   BYTE* Card_Buffer,
   int*  Card_S,
   BYTE* Vote_Buffer,
   int*  Vote_S
   )
// ---------------------------------------------------------------------------
{
      FirstCollegeType  FirstCollege;
      int               i;
      int               length;
      int               Pointer_Vote_Buffer;
      int               Pointer_Card_Buffer;
      VOTE_TYPE_1       Blanco_Vote_1;          // local vote buffer

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "Convert Card_Buffer:"); dcio();
      strncat( gcDcio, Card_Buffer, 79); dcio();
   #endif // _DEBUGMODE

   Pointer_Vote_Buffer = 0, Pointer_Card_Buffer = 0;
   Init_Vote_1( & Blanco_Vote_1);

   for ( i = 0; i < MAX_ELECTIONS; i++)
   {

      if ( 0 == i)
      {
         // :TRICKY: extract's the usage flag from the college number of the first election
         // assumes that flag is 0/1 and the collegenumber is first byte of VOTE_TYPE_1
         FirstCollege.Id = Card_Buffer[Pointer_Card_Buffer];

         if ( FirstCollege.bf.UsageFlag)
         {
            // mask usage flag from here and put college byte back
            FirstCollege.bf.UsageFlag = 0;
            Card_Buffer[Pointer_Card_Buffer] = FirstCollege.Id;
            *Flag = 1;
         }
         else
         {
            *Flag = 0;
         }
      }

      switch( Card_Info_Per_Election[i].Type_Election)
      {
         case 0:
            length = sizeof( VOTE_TYPE_0);
            memcpy( &Vote_Buffer[Pointer_Vote_Buffer],
                    &Card_Buffer[Pointer_Card_Buffer], length);
            Pointer_Vote_Buffer += length;
            Pointer_Card_Buffer += length;
            continue;

         case 1:
            length = sizeof( VOTE_TYPE_1);

            // clear with blanco
            memcpy( &Vote_Buffer[Pointer_Vote_Buffer],
               (char*) &Blanco_Vote_1, length);

            length = sizeof( VOTE_TYPE_1) -
                     ( ((MAX_CANDIDATES+2) / 3) -
                       ((Card_Info_Per_Election[i].Max_Candidates+2) / 3) );
            memcpy( &Vote_Buffer[Pointer_Vote_Buffer],
               &Card_Buffer[Pointer_Card_Buffer], length);
            Pointer_Card_Buffer += length;
            Pointer_Vote_Buffer += sizeof( VOTE_TYPE_1);
            continue;

       default:
         continue;
      } // eoswitch
   } // eofor

   *Card_S = Pointer_Card_Buffer;
   Vote_Buffer[Pointer_Vote_Buffer] = CurrentTypeElectorInBac - 1 + 0x30; //stefG: adding type elector
   *Vote_S = ++Pointer_Vote_Buffer; // stefG: add 1 to length vote buffer

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "Vote_Buffer:"); dcio();
      strncat( gcDcio, Vote_Buffer, Pointer_Vote_Buffer); dcio();
   #endif // _DEBUGMODE

   return;
}
//</code></function>


/*<function id="Build_Card_Buffer" confidential="yes">
<heading>
  <description><en>
    constructs the card buffer by computing the MAC on the data and
    adding the security informations to the vote.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en> local function</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype>
      <name>Data</name>
      <description><en>the data to secure</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>Flag</name>
      <description><en> the card's usage flag</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype>
      <name>Card_Buffer</name>
      <description><en> the card buffer where to put the formatted data in</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">BYTE</datatype>
      <name>Result_Key []</name>
      <description><en> the array that will contain the result from the MAC function</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype>
      <name>Vote_Size</name>
      <description><en> the length of a Blanco_Vote, in fact the Vote length in buffer</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype>
      <name>Vote_Size_On_Card</name>
      <description><en> the length of a Blanco_Vote, in fact the Vote length on card</en>
      </description>
    </parameter>
  </parameters>
  <usage/>
  <checks>
    this is not checked, but the vote must be aligned on 8 multiple</checks>
  <comment>
    this function is used to secure data on the magnetic card, we add
    the MAVSESSION key, the usage flag and the computed MAC to the
    data on the card.
    We are forced to write the MAVSESSION key because it is a random
    key. The MAVSESSION key is a truncated key, we compute the MAC
    with it, then the MAC is truncated himself. This is to spare some
    space on the card, so their is more space available for the data.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Build_Card_Buffer (
   BYTE* Data,                         // the votes per election
   int   Flag,                         // the card's usage flag
   BYTE* Card_Buffer                   // physical data on the card
   )
// ---------------------------------------------------------------------------
{
   BYTE  Mac[AESMACLEN+1];
   int   Pos;
   int   dummy;
   BYTE  Card_Data[MAX_CARD_BUF+1];

   memset( Card_Data, NULL, sizeof( Card_Data));

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "buildCB - vsize[%d] max[%d] sizeof[%d]",
         Vote_Size_On_Card, MAX_CARD_BUF, sizeof( Card_Data)); dcio();
   #endif // _DEBUGMODE

// Vote_Size_On_Card is update each time
   Convert_Vote_Buffer_To_Card_Buffer(
      Flag, Data, &dummy, Card_Data, & Vote_Size_On_Card );

   PositionCardTypeElector = ++Vote_Size_On_Card;   // stefG Update size with TypeElector
   Card_Data[PositionCardTypeElector-1]= CurrentTypeElector - 1 + 0x30; //stefG insert elector

   Align_Vote_On_8_Bytes(Card_Data, &Vote_Size_On_Card);

   // compute MAC
   MAC( MAVVOT, Card_Data, Vote_Size_On_Card);
   memcpy( Mac, Result_Key, AESMACLEN);
   // trunc the computed MAC
   Trunc_Key( Mac, Mac);

   // build the physical card buffer
   Pos = 0;

   // store the MAVSESSION key
   for( int i=0; i < DESKEYLEN; i++)
   {
      Card_Buffer [Pos++] = mavSessionKey [i];
   }

   // add the votes data
   for( i=0; i < Vote_Size_On_Card; i++)
   {
      Card_Buffer [Pos++] = Card_Data [i];
   }

   // add the truncated MAC
   for( i=0; i < AESMACLEN; i++)
   {
      Card_Buffer [Pos++] = Mac [i];
   }

   // fill the rest of the buffer with 0
   for( i = Pos; i < MAX_CARD_BUF+1; i++)
      Card_Buffer [i] = 0;

   return;
}
//</code></function>


/*<function id="Split_Card_Buffer" confidential="yes">
<heading>
  <description><en>
    splits the card buffer to extract the usage flag and the datas.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED the MAC is correct,
        ABORTED the MAC does not correspond</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE</datatype>
      <name>Card_Buffer []</name>
      <description><en>the card buffer to split</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>Flag</name>
      <description><en>the extracted card's usage flag</en></description></parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype>
      <name>Data</name>
      <description><en>the extracted vote data</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">BYTE</datatype>
      <name>Result_Key []</name>
      <description><en> the array that will contain the result from the MAC function</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">BYTE</datatype>
      <name>mavSessionKey [DESKEYLEN+1]</name>
      <description><en>the random MAV session key extracted from the buffer</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    if( Split_Card_Buffer( Buffer, &amp;Flag, Vote_Data) == ABORTED)</usage>
  <checks>
    checks the MAC of the data on the card</checks>
  <comment>
    splits the data in the same way as Build_Card_Buffer had
    construct it. It first extracts the MAVSESSION key that was
    used to compute the MAC on the Data, it extracts the card's
    usage flag, the data and the mac. It then computes the MAC on
    the extracted data, truncs it and compares it with that on the
    card.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Split_Card_Buffer (
   BYTE  Card_Buffer [],               // physical data on the card
   int*  Flag,                         // the card's usage flag
   BYTE* Data                          // the votes per election
   )
// ---------------------------------------------------------------------------
{
   BYTE  Mac[AESMACLEN+1] = "" , MAC_Computed [AESMACLEN+1] = "";
   int   Data_Blocks, i, Card_Offset = 0;
   int   dummy;
   BYTE  Card_Data[MAX_CARD_BUF+1];
   int   Status;

   Status = 0;

   // extract the MAVSESSION key
   for( i = 0; i < DESKEYLEN; i++)
   {
      mavSessionKey [i] = Card_Buffer [Card_Offset++];
   }

   // extract the vote data
   for( i = 0; i < Vote_Size_On_Card; i++)
   {
      Card_Data [i] = Card_Buffer [Card_Offset++];
   }
   Card_Data [i] = NULL;

    // stefG:find type of elector
   CurrentTypeElectorInBac=Card_Data[PositionCardTypeElector-1]-0x30+1;

   // extract the MAC
   for( i = 0; i < AESMACLEN; i++)
   {
      Mac [i] = Card_Buffer [Card_Offset++];
   }

   // compute the MAC
   MAC( MAVVOT, Card_Data, Vote_Size_On_Card);
   memcpy( MAC_Computed, Result_Key, AESMACLEN);

   // trunc the computed MAC
   Trunc_Key( MAC_Computed, MAC_Computed);

   // compare the 2 MACs
   if ( memcmp( Mac, MAC_Computed, AESMACLEN))
   {
      Status = 10;
      goto error;
   }

   Convert_Card_Buffer_To_Vote_Buffer(
      Flag, Card_Data, &dummy, Data, &dummy );

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Split_Card_Buffer - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
//</code></function>


/*<function id="Trunc_Key" confidential="yes">
<heading>
  <description><en>
    trunc an 8 integer key to a key where each integer has values
    between '0' and '9'</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype>
      <name>Inkey</name>
      <description><en>the key to trunc</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype>
      <name>Outkey</name>
      <description><en>the truncated key</en></description></parameter>
  </parameters>
  <usage>
    Trunc_Key( sKey.data, mavSessionKey);</usage>
  <checks/>
  <comment>
    We can only code 10 values on the card, so if we want to save
    values coded up to 255 we must use 3 card positions. In this case
    we want'nt to save the whole key, because we will then need 24
    card positions, so we trunc each value so they are between '0'
    and '9'.
    Because it is easyer, we trunc the values with a binary mask
    00000111, the resulting values are then from '0' to '7'. '8' and
    '9' are not used because the conversion routines will be more
    complicated.
    This function is used by the MAV to store the MAV_SESSION_Key
    and the MAC computed with it on card.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Trunc_Key (
   char *Inkey, char *Outkey)
// ---------------------------------------------------------------------------
{
   for( int i = 0; i < AESMACLEN; i++)
      Outkey [i] = (Inkey [i] & 7) + '0';                 // and with 00000111
}
//</code></function>


/*<function id="Encrypt_Vote" confidential="yes">
<heading>
  <description><en>
    encrypts the vote and adds the MAC to the encrypted vote.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Vote_Data</name>
      <description><en>the non encrypted vote array</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Vote_Encrypted</name>
      <description><en>the encrypted vote array</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>the data length to de/en crypt</en></description>
    </parameter>
    <parameter type="global">
      <datatype>BYTE</datatype><name>Result_Key []</name>
      <description>
        <en>the array that will contain the result from the MAC function</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    Encrypt_Vote( Clear_Array, Encrypted_Array, 16);</usage>
  <checks>
    The Data_Length is not tested but it should be a 8 multiple.</checks>
  <comment>
    The URNVOT method is used to crypt the datas to write in the
    B003 and B013.
    We must use memcpy before we call the encryption routines
    because the processed datas are returned in place with these
    functions.
    For the encryption we add the MAC at the end of the encrypted
    vote, when decrypting the MAC comes with the encrypted datas
    and is will be tested by the Decrypt function.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Encrypt_Vote (
   BYTE* Vote_Data, BYTE* Vote_Encrypted, int Data_Length)
// ---------------------------------------------------------------------------
{
    int noBlocks = Data_Length / AESBLOCKLEN;

    memcpy( Vote_Encrypted, Vote_Data, Data_Length);

    InitEnc( URNVOT); //setup encryption engine
    Encrypt( Vote_Encrypted, noBlocks);

    MAC( URNVOT, Vote_Encrypted, (noBlocks * AESBLOCKLEN));
    for ( int i=0; i < AESMACLEN; i++)  // add the MAC
        Vote_Encrypted [Data_Length + i] = Result_Key [i];
}
//</code></function>


/*<function id="Decrypt_Vote" confidential="yes">
<heading>
  <description><en>
    decrypts the vote to clear informations</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en>local function</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Vote_Encrypted</name>
      <description><en>the encrypted vote array</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Vote_Data</name>
      <description><en>the non encrypted vote array</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>the data length to de/en crypt</en></description>
    </parameter>
    <parameter type="global">
      <datatype>BYTE</datatype><name>Result_Key []</name>
      <description>
        <en>the array that will contain the result from the MAC function</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    Decrypt_Vote( Encrypted_Array, Clear_Array, 16);</usage>
  <checks>
    The Data_Length is not tested but it should be a 8 multiple.</checks>
  <comment>
    The URNVOT method is used to crypt the datas to write in the
    B003 and B013.
    We must use memcpy before we call the encryption routines
    because the processed datas are returned in place with these
    functions.
    For the encryption we add the MAC at the end of the encrypted
    vote, when decrypting the MAC comes with the encrypted datas
    and is will be tested by the Decrypt function.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Decrypt_Vote (
   BYTE* Vote_Encrypted, BYTE* Vote_Data, int Data_Length)
// ---------------------------------------------------------------------------
{
    int noBlocks = Data_Length / AESBLOCKLEN;

    memcpy( Vote_Data, Vote_Encrypted, Data_Length);

    InitEnc( URNVOT); //in order to get the fixed IV
    InitDec( URNVOT); //setup decryption engine
    Decrypt( Vote_Data, noBlocks);

}
//</code></function>


/*<function id="Verify_MAC_On_Vote" confidential="yes">
<heading>
  <description><en>
    verifies the MAC from a vote saved on floppy (B013, B003)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED the MAC is correct,
        ABORTED the MAC does not correspond</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>Vote_Encrypted_With_MAC</name>
      <description><en>the array which contains the vote along with the MAC as
        it is in memory or in the B003</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Data_Length</name>
      <description><en>the length of the vote data in memory</en></description>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">BYTE</datatype><name>Result_Key []</name>
      <description>
        <en>the array that will contain the result from the MAC function</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    if( Verify_MAC_On_Vote(Vote, Memory_Vote_Length) == ABORTED)</usage>
  <checks/>
  <comment>
    Warning: the Data_Length is not the same as that on card because
    in memory there is no restrictions as '0' to '9' values, we
    compress the data to use less memory, so the data card length in
    memory is shorter than that on card.
    The MAC is positionned after the data and use the 8 last
    positions.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Verify_MAC_On_Vote (
   BYTE * Vote_Encrypted_With_MAC, int Data_Length)
// ---------------------------------------------------------------------------
{
//   int Data_Blocks = (Data_Length - AESMACLEN) / AESBLOCKLEN;

    MAC( URNVOT, Vote_Encrypted_With_MAC, (Data_Length - AESMACLEN));

    if ( memcmp( Result_Key, Vote_Encrypted_With_MAC + (Data_Length - AESMACLEN), AESMACLEN))
        return ABORTED;

    return SUCCEEDED;
}
//</code></function>


/*<function id="initCard" confidential="yes">
<heading>
  <description>
    <en>Format the emitted vote in memory as it will be physically written on the card.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iUsage</name>
      <description><en>Card's usage flag</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>PhysicalCardData</name>
      <description><en>card's physical data</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Card[MAX_CARD_BUF]</name>
      <description><en>the card buffer</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype>
      <name>Offset</name>
      <description><en>Offset into Card_Buffer</en></description>
    </parameter>
  </parameters>
  <comment>
    This function is called when the user has completed all the
    elections. The vote is formatted with the security protection.
    We first align the Vote data on a 8 multiple, we generate a
    random MAVSESSION key, and we build the card buffer with the
    card's usage flag.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void initCard (
   int iUsage,
   BYTE* PhysicalCardData)
// ---------------------------------------------------------------------------
{

   Generate_Mav_Session();
   Build_Card_Buffer( Card, iUsage, PhysicalCardData);

   return;
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Write_Votes" confidential="yes">
<heading>
  <description><en>Write the card's physical data.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iUsage</name>
      <description><en>Card's usage flag</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE*</datatype><name>PhysicalCardData</name>
      <description><en>card's physical data</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Card[MAX_CARD_BUF]</name>
      <description><en>the card buffer</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype>
      <name>Offset</name>
      <description><en>Offset into Card_Buffer</en></description>
    </parameter>
  </parameters>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
CARD_POSITION Write_Votes (
   int   iUsage,                       // cards usage flag
   BYTE* PhysicalCardData              // cards physical data
   )
// ---------------------------------------------------------------------------
{
   CARD_POSITION  Status;
   BYTE           ReadCardData[MAX_CARD_BUF+1];

   memset( ReadCardData, NULL, sizeof( ReadCardData));

   if ( 9 == iUsage)
   {
      // elector canceled so rebuild card data first ( using the same session key)
      Build_Card_Buffer( Card, iUsage, PhysicalCardData);
   }


   return Reader_Write_Card( Intern_Reader, PhysicalCardData);
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Init_Card_Buffer">
<heading>
  <description>
    <en>inits the current position in the card buffer</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype>
      <name>Offset</name>
      <description><en>Offset into Card_Buffer</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    Init_Card_Buffer();</usage>
  <checks/>
  <comment>
    The Offset indicates the position where to append
    the vote datas after voting for each election.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Init_Card_Buffer (
   void)
// ---------------------------------------------------------------------------
{
   memset( Card, 0, sizeof( Card));
   Offset = 0;

   return;
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Append_To_Card">
<heading>
  <description><en>
    stores the vote in the card buffer</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Vote</name>
      <description><en>the vote to append</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Length</name>
      <description><en>the vote length</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Card[MAX_CARD_BUF]</name>
      <description><en>the card buffer</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Offset</name>
      <description><en>Offset into Card_Buffer</en></description></parameter>
    </parameters>
  <usage>
    Append_To_Card( Vote_0, Length_Vote_0);</usage>
  <checks/>
  <comment>
    adds the votes in the Card buffer from the Offset position and
    increments the Offset position.
    This function is called after the user has done voting for the
    current election and before loading the informations for the
    next election.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void Append_To_Card (
   char* Vote, int Length)
// ---------------------------------------------------------------------------
{
    for( int i = 0; i < Length; ++i, ++Offset)
        Card [Offset] = Vote [i];
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Buffer_Vote">
<heading>
  <description><en>
    process the emitted vote for memory storing</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype>
      <name>Election</name>
      <description><en>
        the full structure for the election where the user choices are marked</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype>
      <name>Vote_0</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype>
      <name>Vote_1</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
  </parameters>
  <usage>
    Buffer_Vote( Election);</usage>
  <checks/>
  <comment>
    this functions calls the appropriate functions to convert an
    emitted vote into the card format</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Buffer_Vote (
   ElectionList Election)
// ---------------------------------------------------------------------------
{
    switch( Election->Type)
    {
        case 0 :

            Init_Vote_0( & Vote_0);  // initialize the type 0 vote structure
            Format_Vote_0();         // fill the type 0 vote structure with the vote
            Append_To_Card( (char *) & Vote_0, sizeof( Vote_0) );   // write to card
            break;

        case 1 :

            Init_Vote_1( & Vote_1);
            Format_Vote_1();
            Append_To_Card( (char *) & Vote_1, sizeof( Vote_1) );
            break;
   }
}
#endif // _APPLICATION_MAV
//</code></function>

/*
<function id="Format_Vote_0" confidential="yes">
<heading>
  <description><en>
    fills the vote structure corresponding to the vote type with the
    emitted vote by setting 'flags' for the selected parties and
    candidates.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype>
      <name>Vote_0</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype>
      <name>Vote_1</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>struct Select_Party</datatype>
      <name>Parties</name>
      <description><en>the party list for the election</en></description>
    </parameter>
    <parameter type="global">
      <datatype>struct Select_Candidates</datatype>
      <name>Candidates</name>
      <description><en>the Candidate list for the party</en></description>
    </parameter>
  </parameters>
  <usage>
    Format_Vote_0();</usage>
  <checks/>
  <comment>
    If a blanco vote was emitted, the initialized vote structure
    is used as it is, there is only the usage flag that change
    between a initialized card (blanco card) and a card that is voted
    blanco; simply to mark the card's passage in the MAV.
    For a type 1 vote the differnece is that there may be more than
    one selected candidate.
    See GENTYPE.H for a full explanation of the different structures.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void Format_Vote_0 ( void)
// ---------------------------------------------------------------------------
{
    Vote_0.College = Parties.CollegeInd + '0';

    if( Parties.Blanco_Vote == TRUE)           // blanco vote for this election
        return;

    Int_Two_Bytes( Parties.Selected, Vote_0.Party); // convert the party number


    if( Candidates.EffectivesSelected > 0)          // an effective is selected
    {
        Vote_0.TypeVote = '1';                                // only effectives

        for( int i = 0; i < Candidates.NbrOfEffectives; ++i)        // search it
            if( Candidates.Effective [i].Flag == TRUE)
                break;

                         // convert the selected effective number in two bytes
        Int_Two_Bytes( Candidates.Effective [i].Ind, Vote_0.NrCandidate);
    }

    if( Candidates.SuppliantsSelected > 0)           // a suppliant is selected
    {
        for( int i = 0; i < Candidates.NbrOfSuppliants; ++i)
            if( Candidates.Suppliant [i].Flag == TRUE)
                break;

        Int_Two_Bytes( Candidates.Suppliant [i].Ind, Vote_0.NrSuppleant);

        if( Candidates.EffectivesSelected == 0)
            Vote_0.TypeVote = '2';                             // only suppliants
        else
            Vote_0.TypeVote = '3';                    // effective and suppliant
    }

    if( Candidates.Party_Vote )
        Vote_0.TypeVote += 4;
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Format_Vote_1" confidential="yes">
<heading>
  <description>
    <en>fills the vote structure corresponding to the vote type with the
    emitted vote by setting 'flags' for the selected parties and
    candidates.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype>
      <name>Vote_0</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype>
      <name>Vote_1</name>
      <description><en>temporary buffers for each election type</en></description>
    </parameter>
    <parameter type="global">
      <datatype>struct Select_Party</datatype>
      <name>Parties</name>
      <description><en>the party list for the election</en></description>
    </parameter>
    <parameter type="global">
      <datatype>struct Select_Candidates</datatype>
      <name>Candidates</name>
      <description><en>the Candidate list for the party</en></description>
    </parameter>
  </parameters>
  <usage>Format_Vote_1();</usage>
  <checks/>
  <comment>
    If a blanco vote was emitted, the initialized vote structure
    is used as it is, there is only the usage flag that change
    between a initialized card (blanco card) and a card that is voted
    blanco; simply to mark the card's passage in the MAV.
    For a type 1 vote the differnece is that there may be more than
    one selected candidate.
    See GENTYPE.H for a full explanation of the different structures.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void Format_Vote_1 ( void)
// ---------------------------------------------------------------------------
{

   if ( CONSEIL_BXL == Parties.ElectionInd)
   {
      // 20040114 avn - remember College contains GroupInd since there are no
      // colleges for this election...
      if ( DUTCH == Parties.GroupInd)
      {
         AllowSpecialElection( TRUE);
      }

      // 20040114 avn - put GroupInd on the card instead of CollegeInd, needed
      // on urn for totaling
      Vote_1.College = Parties.GroupInd + '0';
   }
   else
   {
      Vote_1.College = Parties.CollegeInd + '0';
   }


    if( Parties.Blanco_Vote == TRUE)
        return;

    Int_Two_Bytes( Parties.Selected, Vote_1.Party);


    if( Candidates.EffectivesSelected > 0)
    {
        Vote_1.TypeVote = '1';                                // only effectives

        for( int i = 0; i < Candidates.NbrOfEffectives; ++i)
        {
            if( Candidates.Effective [i].Flag == TRUE)
            {
                int Ind = Candidates.Effective[i].Ind;
                               // TMT Ind counting from 0 and not from 1 !!!
                Set_Candidate_Bit( Ind - 1 );
            }
        }
    }

    if( Candidates.SuppliantsSelected > 0)
    {
        for( int i = 0; i < Candidates.NbrOfSuppliants; ++i)
        {
            if( Candidates.Suppliant [i].Flag == TRUE)
            {
                int Ind   = Candidates.NbrOfEffectives +
                            Candidates.Suppliant[i].Ind;
                               // TMT Ind counting from 0 and not from 1 !!!
                Set_Candidate_Bit( Ind - 1 );
            }
        }

        if( Candidates.EffectivesSelected == 0)
            Vote_1.TypeVote = '2';                             // only suppliants
        else
            Vote_1.TypeVote = '3';                    // effective and suppliant
    }

    if( Candidates.Party_Vote )
        Vote_1.TypeVote += 4;

}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Set_Candidate_Bit" confidential="yes">
<heading>
  <description><en>
    sets the bit corresponding to the choosen candidate for an
    election of type 1.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>Ind</name>
      <description><en>the candidate number</en></description></parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype>
      <name>Vote_1</name>
      <description><en>temporary buffer for a type 1 election</en></description>
    </parameter>
  </parameters>
  <usage>
    Set_Candidate_Bit( 5);</usage>
  <checks>
    no checks is done but you should make sure that INDEX is smaller
    than (MAX_CANDIDATES+2) / 3 what is the array limit.</checks>
  <comment>
    this function is only used for an election of type 1 where you
    can select more than only one candidate.
    Refer to the structure description for a complete explanation.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void Set_Candidate_Bit (
   int Ind)
// ---------------------------------------------------------------------------
{
   int INDEX = Ind / 3;                              // maximum value is 29 !!

   switch( Ind % 3)
   {
   case 0 : Vote_1.Candidate [INDEX].bit1 = 1;  break;
   case 1 : Vote_1.Candidate [INDEX].bit2 = 1;  break;
   case 2 : Vote_1.Candidate [INDEX].bit3 = 1;  break;
   }
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Find_Session">
<heading>
  <description><en>
    returns the session corresponding to the given tag number.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>SessionList</datatype>
      <description><en>the session tag</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>SessionInd</name>
      <description><en>the session tag number</en></description>
    </parameter>
    <parameter type="global">
      <datatype>struct ElectionMain</datatype><name>Root</name>
      <description><en>the linked data list</en></description>
    </parameter>
  </parameters>
  <usage>
    SessionList SessionTag = Find_Session( SessionInd);</usage>
  <checks/>
  <comment>
    The function ends if the searched index is found or if we reach
    the last session in the list. The search begins always at the
    beginning of the list with the first session tag.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
SessionList Find_Session (
   int SessionInd)
// ---------------------------------------------------------------------------
{
   SessionList Link = Root.FirstSession;

   for(;;)
   {
       if( Link == NULL || Link -> Tag == SessionInd)
           return Link;
       else
           Link = Link -> Next;
   }
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Find_Election">
<heading>
  <description><en>
    returns the election corresponding to the index in the linked
    data list.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">ElectionList</datatype>
      <description><en>
        the election linked list corresponding to
        the given election index</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ElectionInd</name>
      <description><en>the election number we want to find</en></description>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the linked data list</en></description>
    </parameter>
  </parameters>
  <usage>
    ElectionList Head = Find_Election( ElectionInd);</usage>
  <checks/>
  <comment>
    The function ends if the searched index is found or if we reach
    the last election in the list. The search begins always at the
    beginning of the list with the first election.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static ElectionList Find_Election (
   int ElectionInd)
// ---------------------------------------------------------------------------
{
   ElectionList Link = Root.FirstElection;

   for(;;)
   {
       if( Link == NULL || Link -> Ind == ElectionInd)
           return Link;
       else
           Link = Link -> Next;
   }
}
//</code></function>


/*<function id="Find_College">
<heading>
  <description><en>
    returns the college corresponding to the index in the linked
    data list of the ElectionList corresponding to the given
    election index.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">CollegeList</datatype>
      <description><en>
        the college linked list in the election
        linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>CollegeInd</name>
      <description/></parameter>
  </parameters>
  <usage>
    CollegeList Head = Find_College( ElectionInd, CollegeInd);</usage>
  <checks/>
  <comment>
    same principle as Find_Election.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static CollegeList Find_College (
   int ElectionInd, int CollegeInd)
// ---------------------------------------------------------------------------
{
   ElectionList Head = Find_Election( ElectionInd);
   CollegeList Link = Head -> FirstCollege;

   for(;;)
   {
      if( Link == NULL || Link -> Ind == CollegeInd)
          return Link;
      else
          Link = Link -> Next;
   }
}
//</code></function>


/*<function id="Find_Party">
<heading>
  <description><en>
    returns the party corresponding to the index in the linked
    data list of the ElectionList corresponding to the given
    election index.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">PartyList</datatype>
      <description><en>
        the party linked list in the election
        linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ElectionInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>CollegeInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>PartyInd</name>
      <description/></parameter>
  </parameters>
  <usage>
    PartyList  Head = Find_Party( ElectionInd, PartyInd);</usage>
  <checks/>
  <comment>
    same principle as Find_Election.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static PartyList Find_Party (
   int ElectionInd, int CollegeInd, int PartyInd)
// ---------------------------------------------------------------------------
{
   CollegeList Head = Find_College( ElectionInd, CollegeInd);
   PartyList Link = Head -> FirstParty;

   for(;;)
   {
      if( Link == NULL || Link -> Ind == PartyInd)
          return Link;
      else
          Link = Link -> Next;
   }
}
//</code></function>

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #ifdef _APPLICATION_MAV
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Find_Candidate">
<heading>
  <description><en>
    returns the candidate corresponding to the index in the linked
    data list of the ElectionList corresponding to the given
    election index.</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">CandidateList</datatype>
      <description><en>
        the candidate linked list in the election
        linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>CollegeInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>PartyInd</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>CandidateType</datatype>
      <name>Type</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>CandidateInd</name>
      <description/></parameter>
  </parameters>
  <usage>
    CandidateList  Head = Find_Candidate( ElectionInd, PartyInd,
           CandidateType, CandidateInd);</usage>
  <checks/>
  <comment>
    same principle as Find_Election.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static CandidateList Find_Candidate (
   int ElectionInd,
   int CollegeInd,
   int PartyInd,
   CandidateType Type,
   int CandidateInd)
{
   PartyList Head = Find_Party( ElectionInd, CollegeInd, PartyInd);
   CandidateList Link;

   switch( Type)
   {
      case Effective:
         Link = Head -> FirstEffective;
         for(;;)
         {
            if( Link == NULL || Link -> Ind == CandidateInd)
               return Link;
            else
               Link = Link -> Next;
         }

      case Suppliant:
         Link = Head -> FirstSuppliant;
         for(;;)
         {
            if( Link == NULL || Link -> Ind == CandidateInd)
               return Link;
            else
               Link = Link -> Next;
         }
   }

   return NULL;
}
#endif   // #ifdef _APPLICATION_MAV
//</code></function>


/*<function id="Build_Election" confidential="yes">
<heading>
  <description><en>reads the B021 file and builds a specific election
    linked list from it (and destroys it from memory).</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED = database successfully loaded,
        ABORTED   = error while loading database</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name><description/></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en> the linked data list</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Data_File</name>
      <description><en> the B021 file handle</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the buffer to read the line</en></description>
    </parameter>
  </parameters>
  <usage>if( Build_Election()   == ABORTED)</usage>
  <checks>
    Check is minimized because it is fully tested in the beginning</checks>
  <comment>
    To save memory we only load one election at the time
    (Only College/Party/Candidate information).</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
RET_CODE Build_Election (
   int ElectId)
// ---------------------------------------------------------------------------
{
      FILE* RAM_File;
      char MacFileName[15+1];

      int   Status = 0;                   // error status

   // prepare filename's
   sprintf( FILE_NAME, "%s.%.2d", DATA_FILE_NAME, ElectId);
   sprintf( MacFileName, "%s.%.2d", DATA_MAC_NAME, ElectId);

   #if _DEBUGMODE
      sprintf( gcDcio, "Build_Election - Eid[%d] Coreleft[%lu]",
         ElectId, coreleft()); dcio();
   #endif // _DEBUGMODE
   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "BE - TE[%s][%s]", FILE_NAME, MacFileName); dcio();
   #endif // _DEBUGMODE

   // check MAC
   if( TestMacFile( GENDAT, FILE_NAME, MacFileName) == ABORTED)
   {
      Status = 10;
      goto error;
   }

   // open data file
   if ( ( RAM_File = RAMdrive_Open_File( FILE_NAME)) == NULL)
   {
      Status = 15;
      goto error;
   }

   while ( fgets( InLine, sizeof( InLine), RAM_File))
   {
      // fgets returns the newline, replace with null terminator
      if ( strlen( InLine) > 0) InLine[ strlen( InLine)-1] = NULL;

      #if 0
         sprintf( gcDcio, "3)[%s][%s]", FILE_NAME, InLine); dcio();
      #endif // _DEBUGMODE

      switch( InLine[0])   // the first character defines the information type
      {
         case 'E' : Parse_Candidate_Name( ElectId); break;  // Candidate

         case '/' :         // Comment Line
         case '\0': break;  // or Empty Line

         default  : Error_Exit( E_INVALID_DATA);
      }
   }

   if ( RAMdrive_Close_File( RAM_File) == ABORTED)
   {
      Status = 20;
      goto error;
   }

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Build_E - abend [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
#endif   // #ifdef _APPLICATION_MAV
//</code></function>


/*<function id="Destroy_Election" confidential="yes">
<heading>
  <description><en>
    destroys election from memory.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>
        SUCCEEDED = database successfully destroyed,
        ABORTED   = error while destroying database</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name><description/></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en> the linked data list</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Data_File</name>
      <description><en> the B021 file handle</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the buffer to read the line</en></description>
    </parameter>
  </parameters>
  <usage>
    if( Destroy_Election() == ABORTED)</usage>
  <checks>
    Check is minimized because it is fully tested in the beginning</checks>
  <comment>
    To save memory we only load one election at the time
    (Only College/Party/Candidate information).</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
RET_CODE Destroy_Election (
   int ElectId)
// ---------------------------------------------------------------------------
{
   ElectionList  LinkE;
   CollegeList   LinkC;
   PartyList     LinkP;
   CandidateList Link;

   #if _DEBUGMODE
      sprintf( gcDcio, "Destroy_Election - Eid[%d] Coreleft[%lu]",
         ElectId, coreleft()); dcio();
   #endif // _DEBUGMODE

   LinkE = Find_Election( ElectId);
   for( LinkC=LinkE -> FirstCollege ; LinkC != NULL ; LinkC=LinkC -> Next)
      for( LinkP=LinkC -> FirstParty ; LinkP != NULL ; LinkP= LinkP -> Next)
      {
         for( Link=LinkP -> FirstEffective ; Link != NULL ; Link=Link -> Next)
         {
            delete Link -> Name1;
            Link -> Name1 = NULL;
         }
         for( Link=LinkP -> FirstSuppliant ; Link != NULL ; Link=Link -> Next)
         {
            delete Link -> Name1;
            Link -> Name1 = NULL;
         }
      }
   return SUCCEEDED;
}
#endif   // #ifdef _APPLICATION_MAV
//</code></function>


/*<function id="ClearVoteBuffers" confidential="yes">
<heading>
  <description>
    <en>Clear global vote buffers before we eject the card.</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void ClearVoteBuffers ()
// ---------------------------------------------------------------------------
{
   // 20040401 avn - cfr. review by Paul at www.afront.be
   Init_Card_Buffer();
   Init_Vote_0( & Vote_0);
   Init_Vote_1( & Vote_1);

   return;
}
#endif   // #ifdef _APPLICATION_MAV
//</code></function>


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #endif   // #ifdef _APPLICATION_MAV
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Setup_Database" confidential="yes">
<heading>
  <description><en>reads the B021 file and builds the election linked list from
  it</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = database successfully loaded,
        ABORTED   = error while loading database</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en> the linked data list</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Data_File</name>
      <description><en> the B021 file handle</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the buffer to read the line</en></description>
    </parameter>
  </parameters>
  <usage>
    if( Setup_Database() == ABORTED)</usage>
  <checks/>
  <comment>
  For 2004 the storage of this file has changed fundamentally in order to
  preserve floppy disk space under extreme circomstances, see analysis for
  details.
  // datafile still contains different files :
  // -> B021.00 contains Type A, B, C and D
  // -> B021.xx contains Type E for Election xx (01 -> 09)
  // -> PL*.DAT contains logo data.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Setup_Database ( void)
{
      const char*    DATA_FILE = "B021";

      FILE*          RAM_File;
      char           Source[15+1];
      char           Destination[15+1];
      ElectionList   HeadE;
      char           MacFileName[15+1];
      char           CommandLine[32+1];
      int            Status;                   // error status

   Status = 0;

   #if _DEBUGMODE
      sprintf( gcDcio, "Enter Setup_Database..."); dcio();
   #endif // _DEBUGMODE

   // prepare filenames
   sprintf( Source, "A:%s", DATA_FILE);
   // extention required for arc decompression
   sprintf( Destination, "C:%s.ARC", DATA_FILE);

   #if _DEBUGMODE
      sprintf( gcDcio, "Decrypting %s to %s...", Source, Destination); dcio();
   #endif // _DEBUGMODE

   // decrypt datafile ( mac was checked while receiving the mK)
   if ( DecryptBinaryFile( GENDAT, Source, Destination) != SUCCEEDED)
   {
      Status = 10;
      goto error;
   }

   #if _DEBUGMODE
      sprintf( gcDcio, "Changing to drive C..."); dcio();
   #endif // _DEBUGMODE

   // change current drive to C
   if ( _chdrive( 3) != SUCCEEDED)
   {
      Status = 20;
      goto error;
   }
   #if _DEBUGMODE
      if ( access( Destination, 0))
      {
         sprintf( gcDcio, "Destination %s not found...", Destination); dcio();
      }
      else
      {
         sprintf( gcDcio, "Decompressing %s...", Destination); dcio();
      }
   #endif // _DEBUGMODE

   #ifndef DIGTOOLS
      // DIGTOOLS -> decompression is done with ar() !
      // --------
      // decompress datafile using ar.cpp
      DecompressFile( Destination);
   #endif

   #if _DEBUGMODE
      sprintf( gcDcio, "Changing to drive A..."); dcio();
   #endif // _DEBUGMODE

   // restore current drive to A:
   if ( _chdrive( 1) != SUCCEEDED)
   {
      Status = 40;
      goto error;
   }

   #if _DEBUGMODE
      sprintf( gcDcio, "Opening file..."); dcio();
   #endif // _DEBUGMODE

   // read general data
   sprintf( FILE_NAME, "%s.00", DATA_FILE_NAME);
   if ( ( RAM_File = RAMdrive_Open_File( FILE_NAME)) == NULL)
   {
      Status = 50;
      goto error;
   }

   Root.NbrOfElections = 0;                      // initialize the linked list
   Root.FirstElection  = NULL;

   #if _DEBUGMODE
      sprintf( gcDcio, "Start parsing..."); dcio();
   #endif // _DEBUGMODE

   while ( fgets( InLine, sizeof( InLine), RAM_File))
   {
      // fgets returns the newline, remplace with null terminator
      if ( strlen( InLine) > 0) InLine[ strlen( InLine)-1] = NULL;

      #if _DEBUGMODE
         sprintf( gcDcio, "1)[%s][%s]", FILE_NAME, InLine); dcio();
      #endif // _DEBUGMODE

      switch( InLine[0])   // the first character defines the information type
      {
         case 'A' : Parse_Session();      break;
         case 'B' : Parse_Election();     break;
         case 'C' : Parse_College();      break;
         case 'D' : Parse_Party();        break;

         case '/' :                               // Comment Line
         case '\0':                       break;  // or Empty Line

         default  : Error_Exit( E_INVALID_DATA);
      }  // eoswitch

   }  // eowhile

   #if _DEBUGMODE
      sprintf( gcDcio, "Parsing done..."); dcio();
   #endif // _DEBUGMODE

   if ( Root.NbrOfElections == 0 ) // there was no election defined in the file
      Error_Exit( E_INVALID_DATA);

   if ( RAMdrive_Close_File( RAM_File) == ABORTED)
   {
      Status = 60;
      goto error;
   }

   // read Candidate Information per Election from specific data file
   for ( HeadE = Root.FirstElection; HeadE != NULL; HeadE = HeadE -> Next)
   {
      sprintf( FILE_NAME, "%s.%.2d", DATA_FILE_NAME, HeadE -> Ind);
      if ( ( RAM_File = RAMdrive_Open_File( FILE_NAME)) == NULL)
      {
         Status = 110;
         goto error;
      }

      while ( fgets( InLine, sizeof( InLine), RAM_File))
      {
         // fgets returns the newline, remplace with null terminator
         if ( strlen( InLine) > 0) InLine[ strlen( InLine)-1] = NULL;

         #if _DEBUGMODE
            sprintf( gcDcio, "2)[%s][%s]", FILE_NAME, InLine); dcio();
         #endif // _DEBUGMODE

         switch( InLine[0])   // the first character defines the information type
         {
            case 'E' : Parse_Candidate();
                       break;

            case '/' :         // Comment Line
            case '\0': break;  // or Empty Line

            default  : Error_Exit( E_INVALID_DATA);
         }  // eoswitch

      }  // eowhile

      if ( RAMdrive_Close_File( RAM_File) == ABORTED)
      {
         Status = 120;
         goto error;
      }

#ifdef _APPLICATION_MAV
      // data should be ok, create a mac to insure data integrity when
      // rebuilding elections later on
      sprintf( MacFileName, "%s.%.2d", DATA_MAC_NAME, HeadE -> Ind);
      if( CreateMacFile( GENDAT, FILE_NAME, MacFileName) == ABORTED)
      {
         Status = 130;
         goto error;
      }
#endif // _APPLICATION_MAV

   }  // eofor

   #if _DEBUGMODE
      sprintf( gcDcio, "Leaving Setup_Database..."); dcio();
   #endif // _DEBUGMODE

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Setup_db - abend [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
//</code></function>


/*<function id="Init_Parser">
<heading>
  <description><en>
    sets i to the end position of the first token</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
  </parameters>
  <usage>
    Init_Parser();</usage>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Init_Parser ( void)
// ---------------------------------------------------------------------------
{
   for( i=0; InLine [i] != '\t' && InLine [i] != '\0'; ++i);
}
//</code></function>


/*<function id="Get_Token">
<heading>
  <description><en>
    gets the next token from InLine</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">BOOL</datatype>
      <description><en>
        TRUE  when a new token is found,
        FALSE when no new token is found (Token="")</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>
    Get_Token();</usage>
  <checks/>
  <comment>
    The tokens are separated by \t or ends with the end of string
    character \0.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Get_Token ( void)
// ---------------------------------------------------------------------------
{
   if( InLine [i] == '\0' || InLine [i] == '\n')
      Token [0] = '\0';                          // prevents reading after end
   else
   {
      for( ++i, j=0;
           InLine [i] != '\t' && InLine [i] != '\0' && InLine [i] != '\n';
           ++i)
          Token [j++] = InLine [i];

      Token [j] = '\0';
   }

   return (Token[0]);
}
//</code></function>


/*<function id="Parse_Session">
<heading>
  <description><en>
    stores the session information in the linked data list</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>
    Parse_Session();</usage>
  <checks>
    memory allocation</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to session informations.
    We allocate memory for a new session entry, we fill the session
    list with the informations and then put this list in the
    global linked list.
    Refer to the data structure for a better understanding.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Parse_Session ( void)
// ---------------------------------------------------------------------------
{

   SessionList Link, Temp = new Session;

   if( Temp == NULL)
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "!!! Can't create new Session !!! cleft[%lu]", coreleft()); dcio();
      #endif // _DEBUGMODE
      Error_Exit( E_OUT_OF_MEMORY);
   }

   Temp -> Next = NULL;                         // initialize the session list

   Init_Parser();

   if ( !Get_Token())
   {
      Error_Exit( E_INVALID_DATA);
   }
   Temp -> Tag = atoi( Token);                    // put the entry type number

   if( InLine [i] == '\0')
      Token [0] = '\0';                          // prevents reading after end
   else
   {
      for( ++i, j=0;  InLine [i] != '\0'; ++i)
          Token [j++] = InLine [i];
      Token [j] = '\0';
   }

   Temp -> Text = Alloc_String( Token);                     // store the text

   // indicates wether ticketing is required in this voting bureau
   if ( 10 == Temp -> Tag)
   {
      Root.iBureauWithTicketing = atoi( Temp -> Text);
   }

#ifdef _APPLICATION_MAV
   if ( 9 == Temp -> Tag)
   {
      // memorise election date for ticketing
      Root.cpElectionDate = Temp -> Text;
   }

   if ( 1 == Temp -> Tag)
   {
      // memorise voting bureau id for ticketing
      Root.cpBureauName = Temp -> Text;
   }
#endif  // _APPLICATION_MAV

#ifdef _APPLICATION_URN
   // search a place in the linked list to place the session entry

   if( Root.FirstSession == NULL)
      Root.FirstSession = Temp;
   else
   {
      for( Link=Root.FirstSession; Link -> Next != NULL; Link = Link -> Next);
      Link -> Next = Temp;
   }
#endif  // _APPLICATION_URN

}
//</code></function>


/*<function id="Parse_Election">
<heading>
  <description>
    <en>stores the election information in the linked data list</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>Parse_Election();</usage>
  <checks>memory allocation</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to election informations.
    We allocate memory for a new election entry, we fill the election
    list with the informations and then put this list in the
    global linked list.
    Refer to the data structure for a better understanding.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Parse_Election (
   void
   )
// ---------------------------------------------------------------------------
{
    ElectionList Link, Temp = new Election;

    if ( Temp == NULL)
    {
         #if _DEBUGMODE
            sprintf( gcDcio, "!!! Can't create new Election !!! cleft[%lu]", coreleft()); dcio();
         #endif // _DEBUGMODE
         Error_Exit( E_OUT_OF_MEMORY);
    }

    Temp -> Next          = NULL;
    Temp -> FirstCollege  = NULL;
    Temp -> NbrOfColleges = 0;
    Temp -> HasLanguageGroups = FALSE;

    Init_Parser();

    if( ! Get_Token())
        Error_Exit( E_INVALID_DATA);
    Temp -> Ind = atoi( Token);

#ifdef _APPLICATION_URN

   InitVotesCounter( & Temp -> BlankVotes);
   InitVotesCounter( & Temp -> TotalVotes);

#endif

    if( ! Get_Token())
        Error_Exit( E_INVALID_DATA);
    Temp -> Type = atoi( Token);

    if( Root.Language [1] == NONE)
    {
        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Long_Name [Root.Language [0]] = Alloc_String( Token);

        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Short_Name [Root.Language [0]] = Alloc_String( Token);
    }
    else
    {
        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Long_Name [0] = Alloc_String( Token);
        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Short_Name [0] = Alloc_String( Token);

        if( Root.Language [0] != 0 )
        {
            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Long_Name [Root.Language [0]] = Alloc_String( Token);

            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Short_Name [Root.Language [0]] = Alloc_String( Token);
        }
        else
        {
            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Long_Name [Root.Language [1]] = Alloc_String( Token);
            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Short_Name [Root.Language [1]] = Alloc_String( Token);
        }
    }

    if( ++Root.NbrOfElections == 1)
        Root.FirstElection = Temp;

    else
    {
        for( Link = Root.FirstElection; Link -> Next != NULL;
             Link = Link -> Next);

        Link -> Next = Temp;
   }

}
//</code></function>


/*<function id="Parse_College">
<heading>
  <description><en>
    stores the college information in the linked data list</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>
    Parse_College();</usage>
  <checks>
    we check the MAX_COLLEGE limit.</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to college informations.
    We allocate memory for a new college entry, we fill the college
    list with the informations and then put this list in the
    global linked list.
    Refer to the data structure for a better understanding.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Parse_College ( void)
// ---------------------------------------------------------------------------
{
    ElectionList Head;
    CollegeList Link, Temp = new College;

    if( Temp == NULL)
    {
      #if _DEBUGMODE
         sprintf( gcDcio, "!!! Can't create new College !!! cleft[%lu]", coreleft()); dcio();
      #endif // _DEBUGMODE
        Error_Exit( E_OUT_OF_MEMORY);
    }

    Temp -> Next         = NULL;
    Temp -> FirstParty   = NULL;
    Temp -> NbrOfParties = 0;

    Init_Parser();

    if( ! Get_Token()) Error_Exit( E_INVALID_DATA);

    Head = Find_Election( atoi( Token));
    if( Head == NULL)                                     // election not found
        Data_Error_Exit( M_INVALID_DATA);

    if( ! Get_Token())
        Error_Exit( E_INVALID_DATA);

    Temp -> Ind = atoi( Token);

#ifdef _APPLICATION_MAV
    if( Root.Language [1] == NONE)
    {
        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Long_Name [Root.Language [0]] = Alloc_String( Token);
    }
    else
    {
        if( ! Get_Token())
            Error_Exit( E_INVALID_DATA);

        Temp -> Long_Name [0] = Alloc_String( Token);

        if( Root.Language [0] != 0 )
        {
            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Long_Name [Root.Language [0]] = Alloc_String( Token);
        }
        else
        {
            if( ! Get_Token())
                Error_Exit( E_INVALID_DATA);

            Temp -> Long_Name [Root.Language [1]] = Alloc_String( Token);
        }
    }

#endif // _APPLICATION_MAV

    if( ++( Head -> NbrOfColleges) > MAX_COLLEGES)  // check the constraint
        Error_Exit( E_TOO_MANY_COLLEGES);

    if( Head -> NbrOfColleges == 1)
        Head -> FirstCollege = Temp;

    else
    {
        for( Link=Head -> FirstCollege; Link -> Next != NULL;
             Link = Link -> Next);

        Link -> Next = Temp;
    }
}
//</code></function>


/*<function id="Parse_Party">
<heading>
  <description>
    <en>stores the party information in the linked data list</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>Parse_Party();</usage>
  <checks>we check the MAX_PARTIES limit.</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to party informations.
    We allocate memory for a new party entry, we fill the party
    list with the informations and then put this list in the
    global linked list.
    Refer to the data structure for a better understanding.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Parse_Party (
   void
   )
// ---------------------------------------------------------------------------
{
   ElectionList ThisElection;
   CollegeList Head;
   PartyList Link, Temp = new Party;

   if ( Temp == NULL)
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "!!! Can't create new Party !!! cleft[%lu]", coreleft()); dcio();
      #endif // _DEBUGMODE
      Error_Exit( E_OUT_OF_MEMORY);
   }

   Temp -> Next            = NULL;
   Temp -> FirstEffective  = NULL;
   Temp -> FirstSuppliant  = NULL;
   Temp -> NbrOfEffectives = 0;
   Temp -> NbrOfSuppliants = 0;

#ifdef _APPLICATION_URN
    InitVotesCounter( & Temp -> HeadVotes);
    InitVotesCounter( & Temp -> EffectiveVotes);
    InitVotesCounter( & Temp -> SuppliantVotes);
    InitVotesCounter( & Temp -> MixedVotes);
#endif

   Init_Parser();

   if ( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int ElectionInd = atoi( Token);

   ThisElection = Find_Election( ElectionInd);
   if ( ThisElection == NULL)
   {
      // election not found
      Data_Error_Exit( M_INVALID_DATA);
   }

   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int CollegeInd = atoi( Token);

   Head = Find_College( ElectionInd, CollegeInd);
   if ( Head == NULL)
   {  // election not found
      Data_Error_Exit( M_INVALID_DATA);
   }

   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp -> Ind = atoi( Token);

#ifdef _APPLICATION_MAV
   // get party name
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp -> Name = Alloc_String( Token);

   // logo properties ( it is assumed that width and height are less than 255
   //  and that they are both 0 when there's no logo available
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp->Logo.Width = atoi( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp->Logo.Height = atoi( Token);

   // get language group of party
   if ( ! Get_Token())
   {
      Temp->LanguageGroup =  NONE; // no language group
   }
   else if ( Token[1])
   {
      Error_Exit( E_INVALID_DATA);  // only 1 char permitted
   }
   else
   {
      switch( Token[0])
      {
         case '1' :   break;   //french
         case '2' :   break;   //dutch
         case '4' :   break;   //german
         default  :   Error_Exit( E_INVALID_DATA);
                      break;
      }
      Temp -> LanguageGroup =  atoi( Token);

      if ( ! ThisElection->HasLanguageGroups)
      {
         // remember groups found
         ThisElection->HasLanguageGroups = TRUE;
      }
   }

   // Check to see if group information is consistent for each election
   // Maximum 9 elections can be done at one day
   // ElectionInd-1 is used as an index into the table
   // Init value of the table is -2, no language groups is -1( NONE)
   //nog te verbeteren
   static int ElectionGroupFlag[14] = {-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2};

   switch( ElectionGroupFlag[ ElectionInd-1 ])
   {
     case -2   :   // first time setting for this election
                   ElectionGroupFlag[ ElectionInd-1 ] = Temp -> LanguageGroup;
                   break;
     case NONE :   // no linguistic groups for this election
                   if( ElectionGroupFlag[ ElectionInd-1 ] == Temp -> LanguageGroup)
                      break;
                   else
                       Error_Exit( E_INVALID_DATA);
                       break;
     default   :   // groups are active for this election : groups 1,2 or 4
                   if( Temp -> LanguageGroup < 1) Error_Exit( E_INVALID_DATA);
                   break;
   }
#endif   // #ifdef _APPLICATION_MAV


   if( ++Head -> NbrOfParties > MAX_PARTIES)
       Error_Exit( E_TOO_MANY_PARTIES);

   if( Head -> NbrOfParties == 1)
      Head -> FirstParty = Temp;
   else
   {
      for( Link = Head -> FirstParty; Link -> Next != NULL;
           Link = Link -> Next);

      Link -> Next = Temp;
   }

   return;
}
//</code></function>


/*<function id="Parse_Candidate">
<heading>
  <description><en>
    stores the candidate information in the linked data list</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>
    Parse_Candidate();</usage>
  <checks>
    we check the MAX_CANDIDATES limit.</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to candidate informations.
    We allocate memory for a new candidate entry, we fill the candidate
    list with the informations and then put this list in the
    global linked list.
    Refer to the data structure for a better understanding.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Parse_Candidate ( void)
// ---------------------------------------------------------------------------
{
   ElectionList  HeadE;
   PartyList     Head;
   CandidateList Link, Temp = new Candidate;

   if ( Temp == NULL)
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "!!! Can't create new Candidate !!! cleft[%lu]", coreleft()); dcio();
      #endif // _DEBUGMODE
      Error_Exit( E_OUT_OF_MEMORY);
   }

   Init_Parser();
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int ElectionInd = atoi( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int CollegeInd = atoi ( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int PartyInd   = atoi( Token);

   Head = Find_Party( ElectionInd, CollegeInd, PartyInd);

   if( Head == NULL)                            // party or election not found
      Data_Error_Exit( M_INVALID_DATA);

   Temp -> Next = NULL;

#ifdef _APPLICATION_URN
   InitVotesCounter( & Temp -> NameVotes);
#endif

   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   CandidateType Type = (CandidateType) atoi( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp -> Ind = atoi( Token);

#ifdef _APPLICATION_MAV
   // Check whether there is a Name, but don't store it to save memory
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   Temp -> Name1 = NULL;

#endif  // _APPLICATION_MAV

   switch( Type )
   {
   case  Effective :
      Head -> NbrOfEffectives++;

      if( Head -> NbrOfEffectives + Head -> NbrOfSuppliants > MAX_CANDIDATES)
          Error_Exit( E_TOO_MANY_CANDIDATES);

      if( Head -> FirstEffective == NULL)
      {
         Head -> FirstEffective = Temp;
         return;
      }
      else
         Link = Head -> FirstEffective;

      break;

   case Suppliant :

      Head -> NbrOfSuppliants++;

      if( Head -> NbrOfEffectives + Head -> NbrOfSuppliants > MAX_CANDIDATES)
          Error_Exit( E_TOO_MANY_CANDIDATES);

      if( Head -> FirstSuppliant == NULL)
      {
         Head -> FirstSuppliant = Temp;
         return;
      }
      else
         Link = Head -> FirstSuppliant;
   }

   while( Link -> Next != NULL)
       Link = Link -> Next;

   Link->Next = Temp;
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #ifdef _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Parse_Candidate_Name">
<heading>
  <description><en>
    stores the candidate names in the linked data list</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectId</name>
      <description/></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>InLine [MAX_RECORD_LENGTH]</name>
      <description><en> the read line</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Token [MAX_RECORD_LENGTH]</name>
      <description><en> the found token</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>i</name>
      <description><en> position in InLine</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>j</name>
      <description><en> position in Token</en></description></parameter>
  </parameters>
  <usage>
    Parse_Candidate_Name( ElectId);</usage>
  <checks>
    memory allocation</checks>
  <comment>
    this function may only be called if the current InLine
    corresponds to candidate informations.
    We allocate memory for the candidate(s) name(s)
    but only for the current election to process/vote for</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void Parse_Candidate_Name (
   int ElectId)
// ---------------------------------------------------------------------------
{
   CandidateList Head;

   CandidateNamesType Names;

   Init_Parser();
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int ElectionInd   = atoi( Token);

   // Only process records equal to 'ElectId' (= current Election to Vote for)
   if( ElectionInd != ElectId)
      return;

   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int CollegeInd    = atoi ( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int PartyInd      = atoi( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   CandidateType Type = (CandidateType) atoi( Token);
   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);
   int CandidateInd  = atoi( Token);

   Head = Find_Candidate( ElectionInd, CollegeInd, PartyInd,
                          Type, CandidateInd);

   if( Head == NULL)                            // party or election not found
      Data_Error_Exit( M_INVALID_DATA);

   if( ! Get_Token()) Error_Exit( E_INVALID_DATA);

   // 20040302 avn - E_OUT_OF_MEMORY problem solved
   // memory gets allocated in chunks of 16 bytes, 2 allocations thus eat up 64 bytes
   // allocating name+surname eats up 48 bytes, so we have a 25% better memory usage

   strcpy( Names, Token);
   if ( Get_Token())
   {
      strcat( Names, "@");
      strcat( Names, Token);
   }

   Head -> Name1 = Alloc_String( Names);

   #if 0
      sprintf( cDbugBuf, "M[%lu] E[%d] C[%d] P[%d] T[%d] C[%d] N1[%s] l[%d,%d]",
         coreleft(), ElectionInd, CollegeInd, PartyInd, Type, CandidateInd,
         Head->Name1, strlen( Head->Name1), strlen( Names)); Dbug();
   #endif // _DEBUGMODE

   return;
}
#endif  // _APPLICATION_MAV
//</code></function>


/*<function id="Init_Colleges">
<heading>
  <description>
    <en>fills a list with the colleges corresponding to the election</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name>
      <description><en>the election number</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_College</datatype>
      <name>Colleges</name>
      <description><en>the college list</en></description></parameter>
  </parameters>
  <usage>
    Init_Colleges( 1);           // first election</usage>
  <checks/>
  <comment>The election name is the only language dependent value.
    For 2004 we display the college that matches the user language first.
    When two languages are available (FR/NL or FR/DE) it is important to know
    that Root.Language[0] always contains the value corresponding to FR...</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Init_Colleges (
   int ElectionInd
   )
// ---------------------------------------------------------------------------
{

   ElectionList ThisElection = Find_Election( ElectionInd);

   Colleges.ElectionName  = ThisElection->Long_Name[Working_Language( ASK)];
   Colleges.NbrOfColleges = ThisElection->NbrOfColleges;

   CollegeList ThisCollege = ThisElection->FirstCollege;

   // :WARNING: 20040205 avn - it is assumed that different colleges are available only
   //    in combination with user languages dutch and french
   if ( 2 == Colleges.NbrOfColleges && Working_Language( ASK) == FRENCH)
   {
      // 20040105 avn - reverse B021 order so that first college matches user language
      for ( int i = 1; ThisCollege != NULL;  --i, ThisCollege = ThisCollege->Next)
      {
         Colleges.Choice[i].Ind  = ThisCollege->Ind;
         Colleges.Choice[i].Name = ThisCollege->Long_Name[Working_Language( ASK)];
      }
   }
   else
   {
      // use natural B021 order ( that is DUTCH followed by FRENCH)
      for ( int i = 0; ThisCollege != NULL;  ++i, ThisCollege = ThisCollege->Next)
      {
         Colleges.Choice[i].Ind  = ThisCollege->Ind;
         Colleges.Choice[i].Name = ThisCollege->Long_Name[Working_Language( ASK)];
      }
   }

   #if 0
      sprintf( gcDcio, "noC[%d] wL[%d] rL0[%d] rL[%d]",
         Colleges.NbrOfColleges,
         Working_Language( ASK), Root.Language[0], Root.Language[1]); dcio();
   #endif // _DEBUGMODE

   return;
}
#endif  // _APPLICATION_MAV
//</code></function>

/*<function id="Init_Parties">
<heading>
  <description>
    <en>fills a list with the parties corresponding to a college</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ElectionInd</name>
      <description><en>the election number</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>CollegeInd</name>
      <description><en>the college number</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_College</datatype>
      <name>Colleges</name>
      <description><en>the college list</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_Party</datatype>
      <name>Parties</name>
      <description><en>the party list</en></description></parameter>
  </parameters>
  <usage>
    Init_Parties( 1, 1);         // first election and first college</usage>
  <checks/>
  <comment>
    The election name is the only language dependent value.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Init_Parties (
   int ElectionInd,
   int CollegeInd
   )
// ---------------------------------------------------------------------------
{

   int MaxCandidates = 0;

   Parties.ElectionInd  = ElectionInd;
   Parties.CollegeInd   = CollegeInd;
   Parties.Blanco_Vote  = FALSE;
   Parties.Selected     = FALSE;
   Parties.SuppliantsPresent = FALSE;

   // 20040114 avn - remembers selected language group
   Parties.GroupInd = NONE;

   ElectionList ThisElection = Find_Election( ElectionInd);
   Parties.ElectionName  = ThisElection->Long_Name [ Working_Language( ASK)];
   Parties.ElectionSName = ThisElection->Short_Name [ Working_Language( ASK)];

   Parties.DisplayCol = ( ThisElection->NbrOfColleges > 1) ? TRUE : FALSE;

   CollegeList ThisCollege = Find_College( ElectionInd, CollegeInd);
   Parties.CollegeName  = ThisCollege->Long_Name [ Working_Language( ASK)];
   Parties.NbrOfParties = ThisCollege->NbrOfParties;

   PartyList ThisParty = ThisCollege->FirstParty;

   for ( int i = 0; ThisParty != NULL;  ++i, ThisParty = ThisParty->Next)
   {
      Parties.Choice[i].Ind  = ThisParty->Ind;
      Parties.Choice[i].Name = ThisParty->Name;
      Parties.Choice[i].Logo = ThisParty->Logo;

      // Load the linguistic group of each party
      int lg = ThisParty->LanguageGroup;

      switch ( lg)
      {
         case -1 :              // no language group
            break;
         case  1 :
            lg = 0;      // french
            break;
         case  2 :
            lg = 1;      // dutch
            break;
         case  4 :
            lg = 2;      // german
            break;
      }
      Parties.Choice [i].LanguageGroup = lg;

      // get the maximum number of candidates per party for this election
      Get_Max_Candidates( ElectionInd, CollegeInd, ThisParty->Ind);

      int Total = Candidates.NbrOfEffectives + Candidates.NbrOfSuppliants;
      if ( MaxCandidates < Total) MaxCandidates = Total;

      if ( Candidates.NbrOfSuppliants != 0) Parties.SuppliantsPresent = TRUE;
//
   } // eo for

   Parties.ElectionMode = Calculate_Mode( ThisElection);

   return;
}
#endif  // _APPLICATION_MAV
//</code></function>


/*<function id="Get_Max_Candidates">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name>
      <description><en>the election number</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>CollegeInd</name>
      <description><en>the college number</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>PartyInd</name>
      <description><en>the party number</en></description></parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
  <author></author>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Get_Max_Candidates (
   int ElectionInd, int CollegeInd, int PartyInd)
// ---------------------------------------------------------------------------
{
   PartyList HeadP;
   CandidateList Link;
   int i;

   HeadP  = Find_Party( ElectionInd, CollegeInd, PartyInd);

   Link = HeadP->FirstEffective;
   for( i=0; Link != NULL; ++i, Link = Link -> Next);
   Candidates.NbrOfEffectives = i;

   Link = HeadP->FirstSuppliant;
   for( i=0; Link != NULL; ++i, Link = Link -> Next);
   Candidates.NbrOfSuppliants = i;
}
#endif  // _APPLICATION_MAV
//</code></function>


/*<function id="Init_Candidates">
<heading>
  <description><en>
    fills a list with the candidates corresponding to a party</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>ElectionInd</name>
      <description><en>the election number</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>CollegeInd</name>
      <description><en>the college number</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype>
      <name>PartyInd</name>
      <description><en>the party number</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_College</datatype>
      <name>Colleges</name>
      <description><en>the college list</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_Party</datatype>
      <name>Parties</name>
      <description><en>the party list</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_Candidate</datatype>
      <name>Candidates</name>
      <description><en>the candidate list</en></description></parameter>
  </parameters>
  <usage>
    Init_Candidates( 1, 1, 1);   // party 1 from the first college
                                 // of the first election</usage>
  <checks/>
  <comment>
    The election name is the only language dependent value.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void Init_Candidates (
   int ElectionInd, int CollegeInd, int PartyInd)
// ---------------------------------------------------------------------------
{
    int  Temp_Candidate_Nr;
    unsigned char Temp_Trio;
    int i,j;

    Candidates.ElectionInd = ElectionInd;
    Candidates.CollegeInd = CollegeInd;
    Candidates.Cancel     = FALSE;

    if( Allready_Voted )
        if( Vote_0_Type )
        {
            if( ( Vote_0.TypeVote - '0' ) > 3 )
                Candidates.Party_Vote = TRUE;

            else
                Candidates.Party_Vote = FALSE;

        }
        else
        {
            if( ( Vote_1.TypeVote - '0' ) > 3 )
                Candidates.Party_Vote = TRUE;

            else
                Candidates.Party_Vote = FALSE;
        }
    else
        Candidates.Party_Vote = FALSE;

    Candidates.EffectivesSelected = 0;
    Candidates.SuppliantsSelected = 0;
    Candidates.NbrOfEffectives = 0;
    Candidates.NbrOfSuppliants = 0;

    ElectionList HeadE = Find_Election( ElectionInd);
    Candidates.ElectionName = HeadE -> Short_Name [ Working_Language( ASK)];
    Candidates.ElectionType = HeadE -> Type;

    PartyList HeadP  = Find_Party( ElectionInd, CollegeInd, PartyInd);

    Candidates.PartyName  = HeadP -> Name;
    Candidates.PartyInd   = HeadP -> Ind;
    Candidates.PartyLogo  = HeadP -> Logo;

    CandidateList Link = HeadP -> FirstEffective;

    for( i = 0; Link != NULL; ++i, Link = Link -> Next)
    {
        if( Allready_Voted )
        {
            if( Vote_0_Type )
            {
                Two_Bytes_Int( &Temp_Candidate_Nr, Vote_0.NrCandidate );
                // 20041103 avn - one off error ( suppliant number 1 is in table element 0)
                --Temp_Candidate_Nr;
                if( i == Temp_Candidate_Nr )
                {
                    Candidates.Effective [i].Flag  = TRUE;
                    Candidates.EffectivesSelected = 1;
                }
                else
                    Candidates.Effective [i].Flag  = FALSE;
            }
            else
            {
                memcpy( &Temp_Trio, &Vote_1.Candidate[ i / 3 ], sizeof( char ) );

                Candidates.Effective [i].Flag  = ( Temp_Trio  >> ( i % 3 ) ) & 1;
                if( Candidates.Effective [i].Flag )
                {
                    Candidates.EffectivesSelected++ ;
                }
            }
        }
        else
            Candidates.Effective [i].Flag  = FALSE;

        Candidates.Effective [i].Ind   = Link -> Ind;

         // 20040302 avn - E_OUT_OF_MEMORY problem solved
        Candidates.Effective [i].Name1 = Link -> Name1;

        Candidates.NbrOfEffectives++;
    }


    Link = HeadP->FirstSuppliant;

    j = i;

    for( i = 0; Link != NULL; ++i, Link = Link -> Next)
    {
        if( Allready_Voted )
        {
            if( Vote_0_Type )
            {
                Two_Bytes_Int( &Temp_Candidate_Nr, Vote_0.NrSuppleant );
                // 20041103 avn - one off error ( suppliant number 1 is in table element 0)
                --Temp_Candidate_Nr;
                if( i == Temp_Candidate_Nr )
                {
                    Candidates.Suppliant [i].Flag  = TRUE;
                    Candidates.SuppliantsSelected = 1;
                }
                else
                    Candidates.Suppliant [i].Flag  = FALSE;
            }
            else
            {
                memcpy( &Temp_Trio, &Vote_1.Candidate[ ( i +j ) / 3 ], sizeof( char ) );
                Candidates.Suppliant [i].Flag  = ( Temp_Trio >> ( ( i + j ) % 3 ) ) & 1;
                if( Candidates.Suppliant [i].Flag )
                    Candidates.SuppliantsSelected++ ;
            }
        }
        else
            Candidates.Suppliant [i].Flag  = FALSE;

        Candidates.Suppliant [i].Ind   = Link -> Ind;

         // 20040302 avn - E_OUT_OF_MEMORY problem solved
        Candidates.Suppliant [i].Name1 = Link -> Name1;

        Candidates.NbrOfSuppliants++;
    }
}
#endif  // _APPLICATION_MAV
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #endif  // _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Write_Totals_File" confidential="yes">
<heading>
  <description><en>
    writes the totals to a file in RAM</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        SUCCEEDED = no error,
        ABORTED   = error while writing totals</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>int</datatype>
      <name>Totals_Temp_File</name>
      <description><en>the temporary file handle</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the data linked list</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char*</datatype>
      <name>TOTALS_FILE_NAME</name>
      <description><en>the totals file name</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char*</datatype>
      <name>TOTALS_TEMP_FILE</name>
      <description><en>the totals .TMP file name</en></description></parameter>
  </parameters>
  <usage/>
  <checks>
    on floppy functions</checks>
  <comment>the file in RAM will be copied to the floppy disk and signed</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Write_Totals_File ( void)
// ---------------------------------------------------------------------------
{
      SessionList   LinkS;
      ElectionList  LinkE;
      CollegeList   LinkCol;
      PartyList     LinkP;
      CandidateList LinkCan;

   if ( Floppy_Create_File( TOTALS_TEMP_FILE, & Totals_Temp_File) == ABORTED)
      return ABORTED;

   storeTotalsIV = TRUE;

   if ( Get_BACcounter( 0) > MAX_CARDS_IN_BAC) return ABORTED;

   // Display Total cards in BAC or XXXX if counter is corrupt !
   // Info needed for visualisation in PROGRESS -> NB_VOTERS
   strcpy( Buffer, "A\t0" );

   if ( Flag_Integrity == CORRUPTED)
      for ( int i = 0; i < MAX_TYPE_ELECTOR; i++) strcat( Buffer, "\tXXXX");
   else
      for ( int i = 0; i < MAX_TYPE_ELECTOR; i++)
      {
         char temp[8];
         if ( i < NUMBER_TYPE_ELECTOR || i == CardsWithNoTicket)
         {
            sprintf( temp, "\t%d", Get_BACcounter(i));
         }
         else
         {
            // not used
            sprintf( temp, "\t%d", NONE);
         }
         strcat( Buffer, temp );
      } // eo for

   if( Store_Totals() == ABORTED)
       return ABORTED;

   // write all the session informations
   for( LinkS = Root.FirstSession; LinkS != NULL ; LinkS = LinkS->Next)
       if( Emit_Session( LinkS) == ABORTED)
           return ABORTED;

   // for each election
   for( LinkE = Root.FirstElection; LinkE != NULL ; LinkE = LinkE->Next)
   {
      // write the election description
      if( Emit_Election( LinkE) == ABORTED)
         return ABORTED;

      // for each college
      for( LinkCol = LinkE->FirstCollege; LinkCol != NULL; LinkCol = LinkCol->Next)
      {
         // for each party
         for( LinkP = LinkCol->FirstParty; LinkP != NULL ; LinkP = LinkP->Next)
         {
             if( Emit_Party( LinkE, LinkCol, LinkP) == ABORTED)
                 return ABORTED;

             // for each effective candidate
             for( LinkCan = LinkP->FirstEffective; LinkCan != NULL; LinkCan = LinkCan->Next)
                 if( Emit_Candidate( LinkE, LinkCol, LinkP, Effective,
                                     LinkCan) == ABORTED)
                     return ABORTED;

             // for each suppliant candidate
             for( LinkCan = LinkP->FirstSuppliant; LinkCan != NULL ; LinkCan = LinkCan->Next)
                 if( Emit_Candidate( LinkE, LinkCol, LinkP, Suppliant,
                                     LinkCan) == ABORTED)
                     return ABORTED;
         }
      }
   }

   if ( Floppy_Close_File( Totals_Temp_File) == ABORTED) return ABORTED;

   // copy totals to floppy from ram and sign it
   if ( Copy_Totals_File() == ABORTED) return ABORTED;

   return SUCCEEDED;
}
//</code></function>


/*<function id="Copy_Totals_File">
<heading>
  <description>copy totals to floppy from ram and compare hash with previous</description>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Copy_Totals_File ()
// ---------------------------------------------------------------------------
{

#ifndef _RECOUNT
   // urn is tallying the votes, we remove the recount app to make space on disk
   if ( Floppy_Remove_File( RECEXE_FILE) == ABORTED ||
      Floppy_Remove_File( RECEXE_MAC_FILE) == ABORTED)
      return ABORTED;
#else
   // recount is tallying the votes, we remove the urn app to make space on disk
   if ( Floppy_Remove_File( URNEXE_FILE) == ABORTED ||
      Floppy_Remove_File( URNEXE_MAC_FILE) == ABORTED)
      return ABORTED;
#endif

   // copy totals to floppy from ram and compare hash with previous
   if ( copyRAMFileToDisk( TOTALS_TEMP_FILE, TOTALS_TEMP_COPY) == ABORTED) return ABORTED;

   if ( signTotals( TOTALS_TEMP_COPY, "a:B009") == ABORTED) return ABORTED;
   if ( Test_Executable( RESMAC, TOTALS_TEMP_FILE, "a:B009") == ABORTED) return ABORTED;


   // rename if copy was identical
   if ( Floppy_Rename_File( TOTALS_TEMP_COPY, TOTALS_FILE_NAME) == ABORTED) return ABORTED;

   return SUCCEEDED;
}
//</code></function>

/*<function id="Remove_Totals_File">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Remove_Totals_File ()
// ---------------------------------------------------------------------------
{
   if ( secureDeleteFile( TOTALS_TEMP_FILE) == ABORTED) return ABORTED;

   return SUCCEEDED;
}
//</code></function>



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif // _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*<function id="Store_Totals" confidential="yes">
<heading>
  <description><en>encrypt and write a record in the B001.tmp file</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>SUCCEEDED = storing OK,
        ABORTED   = error while storing data on file</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Totals_Temp_File</name>
      <description><en>the temporary file handle</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Buffer [MAX_RECORD_LENGTH]</name>
      <description><en>the datas to store</en></description>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">BYTE</datatype><name>Result_Key []</name>
      <description>
        <en>the array that will contain the result from the MAC function</en>
      </description>
    </parameter>
    <parameter type="global">
      <datatype>BOOL</datatype><name>storeTotalsIV</name>
      <description><en>remembers if IV has been stored yet</en></description>
    </parameter>
  </parameters>
  <usage>
    if( Store_Totals() == ABORTED)</usage>
  <checks>
    on floppy functions</checks>
  <comment>
    this function stores each line with the same encryption and
    protection technique as for the B021. The data is first aligned
    on AESBLOCKLEN bytes, then we write an integer that is the length of the
    encrypted data plus the MAC length (AESMACLEN), we encrypt the data and
    write the encrypted data, and finally we compute and store the
    MAC from the encrypted data.
    THE MAC IS COMPUTED ON THE ENCRYPTED DATA !!</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE Store_Totals ( void)
// ---------------------------------------------------------------------------
{
    int noBlocks, Record_Length;
    char Text [MAX_RECORD_LENGTH];
    AESMAC Mac;

    //IV must be saved once in an extra record for decryption afterwards
    if ( storeTotalsIV) {
        //store total record length (= IV + MAC)
        Record_Length = MAX_IV_SIZE + AESMACLEN;
        if ( Floppy_Write_Buffer( Totals_Temp_File, sizeof( int), &Record_Length) == ABORTED)
            return ABORTED;

        //setup AES encryption and store IV
        InitEnc( URNTOT);
        if ( Floppy_Write_Buffer( Totals_Temp_File, MAX_IV_SIZE, iv.data) == ABORTED)
          return ABORTED;

        //compute and store MAC on IV
        MAC( URNTOT, iv.data, MAX_IV_SIZE);
        memcpy( Mac.data, Result_Key, AESMACLEN);
        if ( Floppy_Write_Buffer( Totals_Temp_File, AESMACLEN, Mac.data) == ABORTED)
            return ABORTED;

        storeTotalsIV = FALSE;
    }

    //prepare text for encryption
    strcpy( Text, Buffer);
    AlignText( Text, &noBlocks);

    //store total record length (= data + MAC)
    Record_Length = (noBlocks * AESBLOCKLEN) + AESMACLEN;
    if ( Floppy_Write_Buffer( Totals_Temp_File, sizeof( int), &Record_Length) == ABORTED)
        return ABORTED;

    //store encrypted text
    Encrypt( Text, noBlocks);
    if( Floppy_Write_Buffer( Totals_Temp_File, (noBlocks * AESBLOCKLEN), Text) == ABORTED)
      return ABORTED;

    //store computed MAC
    MAC( URNTOT, Text, (noBlocks * AESBLOCKLEN));
    memcpy( Mac.data, Result_Key, AESMACLEN);
    if ( Floppy_Write_Buffer( Totals_Temp_File, AESMACLEN, Mac.data) == ABORTED)
        return ABORTED;

    return SUCCEEDED;
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Emit_Session">
<heading>
  <description><en>format each total data type before it is written to file,
    general informations</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        result of the Store_Totals function</en></description></parameter>
    <parameter type="arg">
      <datatype>SessionList</datatype><name>Link</name>
      <description><en>the session information linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Buffer [MAX_RECORD_LENGTH]</name>
      <description><en>the buffer to store the data</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
    Refer to the B021 data representation for a complete explanation
    of the first line character symbol.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE Emit_Session ( SessionList Link)  // general informations
// ---------------------------------------------------------------------------
{
   sprintf( Buffer, "A\t%d\t%s"
                  , Link -> Tag
                  , Link -> Text);

   return Store_Totals();
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Emit_Election">
<heading>
  <description><en>
    format each total data type before it is written to file,
    election name and number of white votes</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        result of the Store_Totals function</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype>
      <name>Link</name>
      <description><en>the elections linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype>
      <name>Buffer [MAX_RECORD_LENGTH]</name>
      <description><en>the buffer to store the data</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
    Refer to the B021 data representation for a complete explanation
    of the first line character symbol.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE Emit_Election (
   ElectionList   Link                 // points to this election
   )
// ---------------------------------------------------------------------------
{
      CollegeList    LinkCol;
      PartyList      LinkP;
      unsigned int   AllPartyVotes;
      int            Status;

   Status = 0;

   // test maximum votes in election
   if ( Link->TotalVotes.Ctr1 > MAX_CARDS_IN_BAC)
   {
      Status = 10;
      goto error;
   }

   // test votes in election
   AllPartyVotes = Link->BlankVotes.Ctr1;
   for ( LinkCol = Link->FirstCollege; LinkCol != NULL; LinkCol = LinkCol->Next)
   {
      // for each party
      for ( LinkP = LinkCol->FirstParty; LinkP != NULL ; LinkP = LinkP->Next)
      {
         AllPartyVotes += ( LinkP->HeadVotes.Ctr1 + LinkP->EffectiveVotes.Ctr1 +
            LinkP->SuppliantVotes.Ctr1 + LinkP->MixedVotes.Ctr1);

         if ( IsVotesCounterCorrupt( & LinkP->HeadVotes) ||
              IsVotesCounterCorrupt( & LinkP->EffectiveVotes) ||
              IsVotesCounterCorrupt( & LinkP->SuppliantVotes) ||
              IsVotesCounterCorrupt( & LinkP->MixedVotes))
         {
            Status = 20;
            goto error;
         }
      } // eo for each party
   } // eo for each college
   if ( AllPartyVotes != Link->TotalVotes.Ctr1)
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "AllParty[%d] Total[%d]+Blank[%d]",
            AllPartyVotes,
            Link->TotalVotes.Ctr1,
            Link->BlankVotes.Ctr1); dcio();
      #endif // _DEBUGMODE
      Status = 22;
      goto error;
   }

   // generate B-record
   if ( Root.Language[1] != NONE)
   {
      if ( !Root.Language[0])
      {
         sprintf( Buffer, "B\t%d\t%d\t%d\t%s\t%s\t%s\t%s",
            Link->Ind, Link->Type, Link->TotalVotes.Ctr1,
            Link->Long_Name[ Root.Language[0]],
            Link->Short_Name[ Root.Language[0]],
            Link->Long_Name[ Root.Language[1]],
            Link->Short_Name[ Root.Language[1]]);
      }
      else
      {
         sprintf( Buffer, "B\t%d\t%d\t%d\t%s\t%s\t%s\t%s",
            Link->Ind, Link->Type, Link->TotalVotes.Ctr1,
            Link->Long_Name[ Root.Language[1]],
            Link->Short_Name[ Root.Language[1]],
            Link->Long_Name[ Root.Language[0]],
            Link->Short_Name[ Root.Language[0]]);
      }
   }
   else
   {
      sprintf( Buffer, "B\t%d\t%d\t%d\t%s\t%s",
         Link->Ind, Link->Type, Link->TotalVotes.Ctr1,
         Link->Long_Name[ Root.Language [0]],
         Link->Short_Name[ Root.Language [0]]);
   }
   if ( Store_Totals() == ABORTED)
   {
      Status = 30;
      goto error;
   }

   // Print blanco votes as party 0 !!!!
   {
      sprintf( Buffer, "D\t%d\t0\t0\t%u\t0\t0\t0",
         Link->Ind, Link->BlankVotes.Ctr1);
   }

   if ( IsVotesCounterCorrupt( & Link->BlankVotes) ||
        IsVotesCounterCorrupt( & Link->TotalVotes))
   {
      Status = 50;
      goto error;
   }

   if ( Store_Totals() == ABORTED)
   {
      Status = 60;
      goto error;
   }

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Emit_Election aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Emit_Party">
<heading>
  <description><en>format each total data type before it is written to file,
    party votes</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        result of the Store_Totals function</en></description></parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>HeadE</name>
      <description><en>an election linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>CollegeList</datatype><name>HeadCol</name>
      <description><en>a college linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>PartyList</datatype><name>Link</name>
      <description><en>the partys linked list</en></description></parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Buffer [MAX_RECORD_LENGTH]</name>
      <description><en>the buffer to store the data</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
    Refer to the B021 data representation for a complete explanation
    of the first line character symbol.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE Emit_Party (
   ElectionList HeadE,
   CollegeList HeadCol,
   PartyList Link)
// ---------------------------------------------------------------------------
{
   CandidateList  LinkCan;
   unsigned long  AllPersonalVotes;
   int            Status = 0;

   // sum name votes of all effective candidates
   AllPersonalVotes = 0;
   for ( LinkCan = Link->FirstEffective; LinkCan != NULL; LinkCan = LinkCan->Next)
   {
      AllPersonalVotes += LinkCan->NameVotes.Ctr1;
      if ( IsVotesCounterCorrupt( & LinkCan->NameVotes))
      {
         Status = 10;
         goto error;
      }
   }
   // result plausible?
   if ( AllPersonalVotes >=
      (unsigned long) ( Link->EffectiveVotes.Ctr1 + Link->MixedVotes.Ctr1))
   {
      // ok, check all used counters
      if ( IsVotesCounterCorrupt( & Link->EffectiveVotes) ||
           IsVotesCounterCorrupt( & Link->MixedVotes))
      {
         Status = 20;
         goto error;
      }
   }
   else
   {
      Status = 30;
      goto error;
   }

   // sum name votes of all suppliant candidates
   for ( LinkCan = Link->FirstSuppliant, AllPersonalVotes = 0;
         LinkCan != NULL ; LinkCan = LinkCan->Next)
   {
      AllPersonalVotes += LinkCan->NameVotes.Ctr1;
      if ( IsVotesCounterCorrupt( & LinkCan->NameVotes))
      {
         Status = 40;
         goto error;
      }
   }
   // result plausible?
   if ( AllPersonalVotes >=
      (unsigned long) ( Link->SuppliantVotes.Ctr1 + Link->MixedVotes.Ctr1))
   {
      // ok, check all used counters
      if ( IsVotesCounterCorrupt( & Link->SuppliantVotes) ||
           IsVotesCounterCorrupt( & Link->MixedVotes))
      {
         Status = 50;
         goto error;
      }
   }
   else
   {
      Status = 60;
      goto error;
   }

   sprintf( Buffer, "D\t%d\t%d\t%d\t%u\t%u\t%u\t%u"
                  , HeadE -> Ind
                  , HeadCol -> Ind
                  , Link -> Ind
                  , Link -> HeadVotes.Ctr1
                  , Link -> EffectiveVotes.Ctr1
                  , Link -> SuppliantVotes.Ctr1
                  , Link -> MixedVotes.Ctr1);

   if ( IsVotesCounterCorrupt( & Link -> HeadVotes) ||
        IsVotesCounterCorrupt( & Link -> EffectiveVotes) ||
        IsVotesCounterCorrupt( & Link -> SuppliantVotes) ||
        IsVotesCounterCorrupt( & Link -> MixedVotes))
   {
      Status = 70;
      goto error;
   }

   return Store_Totals();

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Emit_Party aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Emit_Candidate">
<heading>
  <description><en>
    format each total data type before it is written to file,
    candidate votes (effectives and suppliants)</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>
        result of the Store_Totals function</en></description></parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>HeadE</name>
      <description><en>an election linked list</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>CollegeList</datatype><name>HeadCol</name>
      <description><en>a college linked list</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>PartyList</datatype><name>HeadP</name>
      <description><en>a party linked list</en></description></parameter>
    <parameter type="arg">
      <datatype>CandidateType</datatype><name>Type</name>
      <description><en>the candidate type</en></description></parameter>
    <parameter type="arg">
      <datatype>CandidateList</datatype><name>Link</name>
      <description><en>the candidates linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>char</datatype><name>Buffer [MAX_RECORD_LENGTH]</name>
      <description><en>the buffer to store the data</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
    Refer to the B021 data representation for a complete explanation
    of the first line character symbol.</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE Emit_Candidate (
   ElectionList   HeadE,
   CollegeList    HeadCol,
   PartyList      HeadP,
   CandidateType  Type,
   CandidateList  Link)
// ---------------------------------------------------------------------------
{
   sprintf( Buffer, "E\t%d\t%d\t%d\t%d\t%d\t%u"
                  , HeadE -> Ind
                  , HeadCol -> Ind
                  , HeadP -> Ind
                  , Type
                  , Link -> Ind
                  , Link -> NameVotes.Ctr1);

   // check counter physically
   if ( IsVotesCounterCorrupt( & Link -> NameVotes))
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "!!! NameVotes bad !!!"); dcio();
      #endif // _DEBUGMODE
      return ABORTED;
   }

   // check counter logically
   switch( Type)
   {
      case Effective:
         if ( Link -> NameVotes.Ctr1 <= ( HeadP -> EffectiveVotes.Ctr1 + HeadP -> MixedVotes.Ctr1))
         {
            // ok, check all used counters
            if ( IsVotesCounterCorrupt( & Link -> NameVotes) ||
                 IsVotesCounterCorrupt( & HeadP -> EffectiveVotes) ||
                 IsVotesCounterCorrupt( & HeadP -> MixedVotes))
            {
               #if _DEBUGMODE
                  sprintf( gcDcio, "!!! effectives NameVotes bad !!!"); dcio();
               #endif // _DEBUGMODE
               return ABORTED;
            }
         }
         else
         {
            // something wrong in the kitchen!
            #if _DEBUGMODE
               sprintf( gcDcio, "!!! schaerbeek syndrom effectives !!!"); dcio();
            #endif // _DEBUGMODE
            return ABORTED;
         }
         break;

      case Suppliant:
         if ( Link -> NameVotes.Ctr1 <= ( HeadP -> SuppliantVotes.Ctr1 + HeadP -> MixedVotes.Ctr1))
         {
            // ok, check all used counters
            if ( IsVotesCounterCorrupt( & Link -> NameVotes) ||
                 IsVotesCounterCorrupt( & HeadP -> SuppliantVotes) ||
                 IsVotesCounterCorrupt( & HeadP -> MixedVotes))
            {
               #if _DEBUGMODE
                  sprintf( gcDcio, "!!! suppliant NameVotes bad !!!"); dcio();
               #endif // _DEBUGMODE
               return ABORTED;
            }
         }
         else
         {
            // something wrong in the kitchen!
            #if _DEBUGMODE
               sprintf( gcDcio, "!!! schaerbeek syndrom suppliants !!!"); dcio();
            #endif // _DEBUGMODE
            return ABORTED;
         }
         break;
   }

   return Store_Totals();
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="Count_Vote" confidential="yes">
<heading>
  <description>
    <en>check for a vote validity or counts the emitted votes</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>RET_CODE</datatype>
      <description><en>SUCCEEDED = vote is valid,
        ABORTED   = vote is not valid</en></description></parameter>
    <parameter type="arg">
      <datatype>OPCODE_COUNT</datatype>
      <name>Operation</name>
      <description><en>
        CHECK_IT only finds out if the data on the card is valid,
        EXECUTE  counts votes in memory</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the data linked list</en></description></parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype><name>Vote_0</name>
      <description><en>Temporary buffers</en></description></parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype><name>Vote_1</name>
      <description><en>Temporary buffers</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype><name>Vote_Data[]</name>
      <description><en>buffer with the decripted vote of each card</en>
      </description>
    </parameter>
  </parameters>
  <usage>Count_Vote( EXECUTE);</usage>
  <checks/>
  <comment>
    The return value is only tested for the CHECK_IT mode because
    it is not possible to have a non valid vote in memory.
    In CHECK_IT mode we only evaluates if the vote is possible but
    in execute mode the function is called from a loop and then we
    count the votes for each party and candidate, and also the
    number of white votes.
    The order in which the elections appears is always the same,
    so we go down through the election list, we determine the length
    of the vote, according to the vote type, we extract the
    corresponding data from the whole vote data and we count or
    check that vote.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE Count_Vote (
   OPCODE_COUNT Operation
   )
// ---------------------------------------------------------------------------
{
   ElectionList   Head;
   int            Length, Card_Offset = 0, TypeElector = 0;
   char*          Vote;
   int            i;
   int            Status;

   Status = 0;
#ifdef _APPLICATION_MAV

   if ( Operation != CHECK_IT)
   {
      Status = 10;
      goto error;
   }

#endif //#ifdef _APPLICATION_MAV

   // for each election
   for ( Head = Root.FirstElection; Head != NULL; Head = Head -> Next)
   {
      switch ( Head -> Type)
      {
         case 0 :
            Vote  = (char *) & Vote_0;
            Length = sizeof( VOTE_TYPE_0);
            break;

         case 1 :
            Vote  = (char *) & Vote_1;
            Length = sizeof( VOTE_TYPE_1);
            break;

         default :
            {
               Status = 20;
               goto error;
            }
      }


#ifdef _APPLICATION_MAV

      if ( ( Card_Offset + Length) > MAX_VOTE_BUF)
      {
         Status = 30;
         goto error;
      }

      for ( i = 0; i < Length; ++i, ++Card_Offset)
      {
         Vote [i] = Card [Card_Offset];
      }

      //stefG : find type elector and verify permissions
      TypeElector = Card [PositionMemTypeElector-1] +1 -0x30;

#endif //#ifdef _APPLICATION_MAV
#ifdef _APPLICATION_URN

      if ( ( Card_Offset + Length) > MAX_VOTE_BUF)
         Error_Exit( E_TOO_MANY_ELECTIONS);

      for ( int i=0; i < Length; ++i, ++Card_Offset)
      {
         Vote [i] = Vote_Data [Card_Offset];
      }

      //stefG : find type elector and verify permissions
      TypeElector = Vote_Data[PositionMemTypeElector-1] +1 -0x30;

#endif //#ifdef _APPLICATION_URN

      if ( Skip_Election( Head -> Ind , TypeElector ) == TRUE )
         continue;

      switch ( Head->Type)
      {
         case 0  :
            if ( TallySingleVote( Operation, Head) == ABORTED)
            {
               Status = 40;
               goto error;
            }
            break;

         case 1  :
            if ( TallyMultiVote( Operation, Head) == ABORTED)
            {
               Status = 50;
               goto error;
            }
            break;

         default :
            Status = 60;
            goto error;

      } // eo switch
    } // eo for each election
    return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "Count_Vote - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
//</code></function>


/*<function id="Count_Vote_Candidate" confidential="yes">
<heading>
  <description><en>search for the voted candidate existence and increments his
    vote counter</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>SUCCEEDED = vote is valid,
        ABORTED   = vote is not valid</en></description></parameter>
    <parameter type="arg">
      <datatype>OPCODE_COUNT</datatype><name>Operation</name>
      <description><en>CHECK_IT or EXECUTE</en></description></parameter>
    <parameter type="arg">
      <datatype>PartyList</datatype><name>Head</name>
      <description><en>the party list</en></description></parameter>
    <parameter type="arg">
      <datatype>CandidateType</datatype><name>Type</name>
      <description><en>the candidate type</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>CandidateInd</name>
      <description><en>the candidate number</en></description></parameter>
  </parameters>
  <usage>
    return Count_Vote_Candidate( Operation, LinkP,
        Effective, CandidateInd);</usage>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Count_Vote_Candidate (
   OPCODE_COUNT Operation, PartyList Head,
   enum CandidateType Type, int CandidateInd)
// ---------------------------------------------------------------------------
{
    CandidateList Link;

    switch( Type)
    {
        case Effective : Link = Head -> FirstEffective;   break;
        case Suppliant : Link = Head -> FirstSuppliant;   break;
    }

    for( ; ; )
    {
        if( Link == NULL)                                 // candidate not found
            return ABORTED;

        if( Link -> Ind != CandidateInd)
            Link = Link -> Next;         // this is not the candidate we look for
        else
        {
            if ( Operation == EXECUTE)   // increments the counter if EXECUTE mode
            {
#ifdef _APPLICATION_URN
               if ( IncrementVotesCounter( & Link -> NameVotes) != 0)
               {
                  #if _DEBUGMODE
                     sprintf( gcDcio, "!!! incr NameVotes failed !!!"); dcio();
                  #endif // _DEBUGMODE
                  return ABORTED;
               }
#endif // _APPLICATION_URN
            }

            return SUCCEEDED;                          // the candidate was found
        }
    }
}
//</code></function>


/*<function id="urnCount_Vote_0" confidential="yes">
<heading>
  <description>
    <en>Count or check the vote for this election type ( single candidate/suppliant)</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">RET_CODE</datatype>
      <description><en>SUCCEEDED = vote is valid,
        ABORTED   = vote is not valid.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>OPCODE_COUNT</datatype><name>Operation</name>
      <description><en>CHECK_IT or EXECUTE, no default value.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>Head</name>
      <description><en>the election linked list</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_0</datatype><name>Vote_0</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
    <parameter type="global">
      <datatype>VOTE_TYPE_1</datatype><name>Vote_1</name>
      <description><en>Temporary buffers</en></description>
    </parameter>
  </parameters>
  <usage>if( urnCount_Vote_0( Operation, Head) == ABORTED)</usage>
  <checks/>
  <comment>Because the party and candidate numbers are coded differently on
   card we must first convert them before any use.
   Refer to the card description and the different structure definitions.

   /!\ until now there are no elections using the 'single' election type mode /!\
  </comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static RET_CODE urnCount_Vote_0 (
   OPCODE_COUNT Operation, ElectionList Head)
// ---------------------------------------------------------------------------
{
    PartyList LinkP;
    int CollegeInd, PartyInd, CandidateInd;
    int Vote_Type;

    CollegeInd = Vote_0.College - '0';

    Two_Bytes_Int( & PartyInd, Vote_0.Party);

    if( PartyInd == 0)                                           // blanco vote
    {
        if ( Operation == EXECUTE)
        {
           if ( IncrementVotesCounter( & Head -> BlankVotes) != 0)
           {
               #if _DEBUGMODE
                  sprintf( gcDcio, "!!! incr0 BlankVotes failed !!!"); dcio();
               #endif // _DEBUGMODE
              return ABORTED;
           }
        }

        return SUCCEEDED;
    }

    LinkP = Find_Party( Head -> Ind, CollegeInd, PartyInd);

    if( LinkP == NULL)                       // party not found, vote not valid
        return ABORTED;

    Vote_Type = Vote_0.TypeVote - '0';

    if( Vote_Type > 3 )
        Vote_Type -= 4;

    switch( Vote_Type )               // Attention : '0' = ASCII 0 !
    {
        case 0 :                                                      // party vote

            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> HeadVotes) != 0)
               {
                  #if _DEBUGMODE
                     sprintf( gcDcio, "!!! incr0 HeadVotes bad !!!"); dcio();
                  #endif // _DEBUGMODE
                  return ABORTED;
               }
            }

            return SUCCEEDED;

        case 1 :                                   // effective candidate only vote

            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> EffectiveVotes) != 0)
               {
                  #if _DEBUGMODE
                     sprintf( gcDcio, "!!! incr0 EffectiveVotes bad !!!"); dcio();
                  #endif // _DEBUGMODE
                  return ABORTED;
               }
            }

            Two_Bytes_Int( & CandidateInd, Vote_0.NrCandidate);

            return Count_Vote_Candidate( Operation, LinkP,
                                         Effective, CandidateInd);

        case 2 :                                   // suppliant candidate only vote

            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> SuppliantVotes) != 0)
               {
                  #if _DEBUGMODE
                     sprintf( gcDcio, "!!! incr0 SuppliantVotes bad !!!"); dcio();
                  #endif // _DEBUGMODE
                  return ABORTED;
               }
            }

            Two_Bytes_Int( & CandidateInd, Vote_0.NrSuppleant);

            return Count_Vote_Candidate( Operation, LinkP, Suppliant, CandidateInd);

        case 3 :                                    // effective and suppliant vote

            if ( Operation == EXECUTE)
            {
               if ( IncrementVotesCounter( & LinkP -> MixedVotes) != 0)
               {
                  #if _DEBUGMODE
                     sprintf( gcDcio, "!!! incr0 MixedVotes bad !!!"); dcio();
                  #endif // _DEBUGMODE
                  return ABORTED;
               }
            }

            Two_Bytes_Int( & CandidateInd, Vote_0.NrCandidate);

            if( Count_Vote_Candidate( Operation, LinkP,
                                      Effective, CandidateInd) == ABORTED)
                return ABORTED;

            Two_Bytes_Int( & CandidateInd, Vote_0.NrSuppleant);

            return Count_Vote_Candidate( Operation, LinkP,
                                         Suppliant, CandidateInd);

        default :

            return ABORTED;
    }
}
#endif // _APPLICATION_URN
//</code></function>


/*<function id="InitVotesCounter" confidential="yes">
<heading>
  <description>
    <en>Initialises the two copies of a votes counter</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description>
        <en>0 indicates succes, negatif return values for failure.</en>
      </description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>Always returns succes, it is unlikely that the assignments can fail!</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static int InitVotesCounter (
   VotesCounterType* VotesCounter
   )
// ---------------------------------------------------------------------------
{
   VotesCounter->Ctr1 = 0;
   VotesCounter->Ctr2 = 0;
   return 0;
}
#endif // #ifdef _APPLICATION_URN
//</code></function>


/*<function id="IncrementVotesCounter" confidential="yes">
<heading>
  <description>
    <en>Increments both copies of a votes counter</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description>
        <en>0 indicates succes, negatif return values for failure.</en>
      </description>
    </parameter>
  </parameters>
  <usage/>
  <checks>Both copies must always be identical</checks>
  <comment>When counters differ we asume a bitflip (SEU) occurred</comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static int IncrementVotesCounter (
   VotesCounterType* VotesCounter
   )
// ---------------------------------------------------------------------------
{
   VotesCounter->Ctr1++;
   VotesCounter->Ctr2++;
   return ( IsVotesCounterCorrupt( VotesCounter));
}
#endif // #ifdef _APPLICATION_URN
//</code></function>

/*<function id="IsVotesCounterCorrupt" confidential="yes">
<heading>
  <description>
    <en>Checks both copies of a votes counter for bitflips</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 indicates succes, negatif values for failure.</en>
      </description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
  </comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
static int IsVotesCounterCorrupt (
   VotesCounterType* VotesCounter
   )
// ---------------------------------------------------------------------------
{
   if ( VotesCounter->Ctr1 != VotesCounter->Ctr2)
   {

      #if _DEBUGMODE
         sprintf( gcDcio, "VotesCounter NOK: [%u] [%u]",
            VotesCounter->Ctr1, VotesCounter->Ctr2); dcio();
      #endif // _DEBUGMODE

      return -1;
   }
   else
   {
      return 0;
   }
}
#endif // #ifdef _APPLICATION_URN
//</code></function>


/*<function id="CheckVotesCounters">
<heading>
  <description>
    <en>Checks all used votes counters for consistency.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 indicates succes, negatif values for failure.</en>
      </description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>
  </comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
RET_CODE CheckVotesCounters (
   void
   )
// ---------------------------------------------------------------------------
{

      ElectionList   LinkE;
      CollegeList    LinkCol;
      PartyList      LinkP;
      CandidateList  LinkCanE;
      CandidateList  LinkCanS;
      int            Status = 0;

   for ( LinkE = Root.FirstElection;
         LinkE != NULL; LinkE = LinkE->Next)
   {
      if ( IsVotesCounterCorrupt( & LinkE->BlankVotes))
      {
         Status = 10;
         goto error;
      }

      for ( LinkCol = LinkE->FirstCollege;
            LinkCol != NULL; LinkCol = LinkCol->Next)
      {
         for ( LinkP = LinkCol->FirstParty;
               LinkP != NULL ; LinkP = LinkP->Next)
         {
            if ( IsVotesCounterCorrupt( & LinkP->HeadVotes))
            {
               Status = 20;
               goto error;
            }
            if ( IsVotesCounterCorrupt( & LinkP->EffectiveVotes))
            {
               Status = 30;
               goto error;
            }
            if ( IsVotesCounterCorrupt( & LinkP->SuppliantVotes))
            {
               Status = 40;
               goto error;
            }
            if ( IsVotesCounterCorrupt( & LinkP->MixedVotes))
            {
               Status = 50;
               goto error;
            }

            for ( LinkCanE = LinkP->FirstEffective;
                  LinkCanE != NULL; LinkCanE = LinkCanE->Next)
            {
               if ( IsVotesCounterCorrupt( & LinkCanE->NameVotes))
               {
                  Status = 60;
                  goto error;
               }
            }
            for ( LinkCanS = LinkP->FirstSuppliant;
                  LinkCanS != NULL ; LinkCanS = LinkCanS->Next)
            {
               if( IsVotesCounterCorrupt( & LinkCanS->NameVotes))
               {
                  Status = 70;
                  goto error;
               }
            }
         } // eofor party
      }  // eofor college
   } // eofor election

   return SUCCEEDED;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "CheckVotesCounters - aborted [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return ABORTED;
}
#endif // #ifdef _APPLICATION_URN
//</code></function>


/*<function id="ResetVotesCounters">
<heading>
  <description>
    <en>Initialises all used votes counters at once.</en>
  </description>
  <parameters/>
  <usage/>
  <checks>No checks, counters are monitored periodically.</checks>
  <comment>
  </comment>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
void ResetVotesCounters (
   void
   )
// ---------------------------------------------------------------------------
{

      ElectionList   LinkE;
      CollegeList    LinkCol;
      PartyList      LinkP;
      CandidateList  LinkCanE;
      CandidateList  LinkCanS;

   for ( LinkE = Root.FirstElection;
         LinkE != NULL; LinkE = LinkE->Next)
   {
      InitVotesCounter( & LinkE->BlankVotes);

      for ( LinkCol = LinkE->FirstCollege;
            LinkCol != NULL; LinkCol = LinkCol->Next)
      {
         for ( LinkP = LinkCol->FirstParty;
               LinkP != NULL ; LinkP = LinkP->Next)
         {
            InitVotesCounter( & LinkP->HeadVotes);
            InitVotesCounter( & LinkP->EffectiveVotes);
            InitVotesCounter( & LinkP->SuppliantVotes);
            InitVotesCounter( & LinkP->MixedVotes);

            for ( LinkCanE = LinkP->FirstEffective;
                  LinkCanE != NULL; LinkCanE = LinkCanE->Next)
            {
               InitVotesCounter( & LinkCanE->NameVotes);
            }
            for ( LinkCanS = LinkP->FirstSuppliant;
                  LinkCanS != NULL ; LinkCanS = LinkCanS->Next)
            {
               InitVotesCounter( & LinkCanS->NameVotes);
            }
         } // eofor party
      }  // eofor college
   } // eofor election

   return;
}
#endif // #ifdef _APPLICATION_URN
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #endif // _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #ifdef _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Count_Vote_0" confidential="yes">
<heading>
  <description>
    <en>Count the vote for the election type</en>
  </description>
  <parameters>
  </parameters>
  <usage/>
  <checks/>
  <comment>
  </comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static RET_CODE Count_Vote_0 (
   ElectionList Head)
// ---------------------------------------------------------------------------
{
    PartyList LinkP;
    int CollegeInd, PartyInd;
    int Effective, Suppleant;
    int Vote_Type;

    CollegeInd = Vote_0.College - '0';

    Two_Bytes_Int( & PartyInd, Vote_0.Party);
    Two_Bytes_Int( & Effective, Vote_0.NrCandidate);
    Two_Bytes_Int( & Suppleant, Vote_0.NrSuppleant);

    if( PartyInd == 0)                                           // blanco vote
    {
        if( !Effective && !Suppleant)
            return SUCCEEDED;

        return ABORTED;
    }

    LinkP = Find_Party( Head -> Ind, CollegeInd, PartyInd);

    if( LinkP == NULL)                       // party not found, vote not valid
        return ABORTED;

    Vote_Type = Vote_0.TypeVote - '0';

    if( Vote_Type > 3 )
        Vote_Type -= 4;

    switch( Vote_Type )               // Attention : '0' = ASCII 0 !
    {
        case 0 :
            if( !Effective && !Suppleant)
                return SUCCEEDED;

            return ABORTED;

        case 1 :                                   // effective candidate only vote
            if( Effective && !Suppleant )
                return SUCCEEDED;

            return ABORTED;

        case 2 :                                   // suppliant candidate only vote
            if( !Effective && Suppleant )
                return SUCCEEDED;

            return ABORTED;

        case 3 :                                    // effective and suppliant vote

            if( !Effective && Suppleant )
                return SUCCEEDED;

            return ABORTED;

        default :

            return ABORTED;
    }
}
#endif // #ifdef _APPLICATION_MAV
//</code></function>


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// #endif // #ifdef _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*<function id="Get_Max_Candidates_Per_Election" confidential="yes">
<heading>
  <description><en>
    Calculate the maximum number of candidates,
    over all partys, colleges for each election.
    Store in: int Card_Info_Per_Election[MAX_ELECTIONS].Max_Candidates
    char Card_Info_Per_Election[MAX_ELECTIONS].Type_Election</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the data linked list</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>rvh 29/12/94</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Get_Max_Candidates_Per_Election ( void)
// ---------------------------------------------------------------------------
{
      ElectionList  LinkE;
      CollegeList   LinkCol;
      PartyList     LinkP;
      CandidateList LinkCan;
      int i,j;
      int NbrOfEffectives, NbrOfSuppliants;
      int TotalCandidates;
      int iMaxSuppliants;                 // max. # suppliants in an election
      int iMaxEffectives;                 // max. # effectives in an election

   for ( i = 0; i < MAX_ELECTIONS; i++)
   {
      Card_Info_Per_Election[i].Max_Candidates = -1;
      Card_Info_Per_Election[i].Type_Election = -1;
   }

   // for each election
   for( LinkE = Root.FirstElection,j=0; LinkE != NULL ;
        LinkE = LinkE->Next, j++)
   {
      if (LinkE->Ind > (MAX_ELECTIONS-1)) Error_Exit( E_TOO_MANY_ELECTIONS);

      iMaxSuppliants = 0;
      iMaxEffectives = 0;

      // for each college
      for( LinkCol = LinkE->FirstCollege; LinkCol != NULL; LinkCol = LinkCol->Next)
      {
          // for each party
          for( LinkP = LinkCol->FirstParty; LinkP != NULL ; LinkP = LinkP->Next)
          {
            // for each effective candidate
            LinkCan = LinkP->FirstEffective;
            for ( i = 0; LinkCan != NULL; ++i, LinkCan = LinkCan->Next);
            NbrOfEffectives = i;
            if ( iMaxEffectives < NbrOfEffectives)
            {
               iMaxEffectives = NbrOfEffectives;
            }

            // for each suppliant candidate
            LinkCan = LinkP->FirstSuppliant;
            for ( i = 0 ; LinkCan != NULL ; ++i, LinkCan = LinkCan->Next);
            NbrOfSuppliants = i;
            if ( iMaxSuppliants < NbrOfSuppliants)
            {
               iMaxSuppliants = NbrOfSuppliants;
            }

          }
      } //end for each college


      // save the maxima per election
      Card_Info_Per_Election[j].Max_Candidates = ( iMaxEffectives + iMaxSuppliants);
      Card_Info_Per_Election[j].Type_Election = LinkE->Type;

      LinkE->MaxSuppliants = iMaxSuppliants;
      LinkE->MaxEffectives = iMaxEffectives;

   } // end for each election

   return;
}
//</code></function>


/*<function id="Skip_Election">
<heading>
  <description>
    <en>Verify if an elector can access an election</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype><description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>IndElection</name>
      <description/></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>TypeElector</name>
      <description/></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the data linked list</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">const</datatype>
      <name>PERMISSIONS</name>
      <description/></parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment>sge stefG 22/02/99</comment>
</heading>
<code>*/
//#pragma argsused
// ---------------------------------------------------------------------------
BOOL Skip_Election (
   int IndElection, int TypeElector
   )
// ---------------------------------------------------------------------------
{
      BOOL  Skip = FALSE;

   if ( IndElection >= MAX_ELECTIONS) Error_Exit( M_INTERN_1);
   if ( TypeElector >= NUMBER_TYPE_ELECTOR ) Error_Exit( M_INTERN_2);

#ifdef _APPLICATION_URN
   if ( CONSEIL_BXL == IndElection)
   {
      // 20040114 avn - remember CollegeInd contains GroupInd since there are no
      // colleges for this election...
      if ( DUTCH == ( Vote_1.College - '0'))
      {
         AllowSpecialElection( TRUE);
      }
   }
#endif // _APPLICATION_URN

   // read the permission table first
   if ( PermissionsUsed[ TypeElector - 1 ][ IndElection - 1 ] == 0)
   {
      // elector is not allowed to vote for this election, the election will be skipped
      Skip = TRUE;
   }
   else
   {
      // 20040106 avn - if table allows we check the special election for 2004
      if ( MEMBRES_BXL == IndElection )
      {
         if ( !gIsSpecialElectionAllowed)
         {
            Skip = TRUE;
         }
      }
   }

   return Skip;

}
//</code></function>


/*<function id="InitApplVariables">
<heading>
  <description>
    <en>Verify if an elector can access an election</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description/></parameter>
    <parameter type="arg">
      <datatype>unsigned int</datatype><name>combination_code</name>
      <description>
        <en>command line parameter %ELECT_MODE% from AUTOEXEC.BAT</en>
      </description>
    </parameter>
  </parameters>
</heading>
<code>*/
#ifdef _APPLICATION_URN
// ---------------------------------------------------------------------------
void InitApplVariables (
   unsigned int combination_code )
// ---------------------------------------------------------------------------
{
    unsigned int copy_comb_code = combination_code;
    int i;
    NbrOfCounters = 0;

    ElectorTypesUsed[0].key = ElectorTypes[0].key;
    ElectorTypesUsed[0].AllCountersTitleID = ElectorTypes[0].AllCountersTitleID;
    ElectorTypesUsed[0].ValidationMessageID = ElectorTypes[0].ValidationMessageID;
    ElectorTypesUsed[0].UrnMessageID = ElectorTypes[0].UrnMessageID;
    ElectorTypesUsed[0].SpecialElectMessageID = ElectorTypes[0].SpecialElectMessageID;

    int j = 0;

    for( i = 1; i < MAX_TYPE_ELECTOR; i++)
    {
        if( combination_code & 1 )
        {
            ElectorTypesUsed[++j].key = ElectorTypes[i].key;
            ElectorTypesUsed[j].AllCountersTitleID = ElectorTypes[i].AllCountersTitleID;
            ElectorTypesUsed[j].ValidationMessageID = ElectorTypes[i].ValidationMessageID;
            ElectorTypesUsed[j].UrnMessageID = ElectorTypes[i].UrnMessageID;
            ElectorTypesUsed[j].SpecialElectMessageID = ElectorTypes[i].SpecialElectMessageID;
        }
        combination_code >>= 1;
    }

    NbrOfCounters = j;
    if( NbrOfCounters == 1)
        NbrOfCounters = 0;

    j = 0;
    int k = 0;

    for( i = 0; i < MAX_TYPE_ELECTOR; i++)
    {
        if( copy_comb_code & 1 )
        {
            for( j = 0; j < MAX_ELECTIONS; j++)
            {
                PermissionsUsed[k][j] = PERMISSIONS[i][j];
            }
            k++;
        }
        copy_comb_code >>= 1;
    }

    NUMBER_TYPE_ELECTOR = k + 1;
}
#endif // #ifdef _APPLICATION_URN

#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void InitApplVariables (
   unsigned int combination_code )
// ---------------------------------------------------------------------------
{

    int j = 0;
    int k = 0;

    for( i = 0; i < MAX_TYPE_ELECTOR; i++)
    {
        if( combination_code & 1 )
        {
            for( j = 0; j < MAX_ELECTIONS; j++)
            {
                PermissionsUsed[k][j] = PERMISSIONS[i][j];
            }
            k++;
        }
        combination_code >>= 1;
    }

    NUMBER_TYPE_ELECTOR = k + 1;
}
#endif // #ifdef _APPLICATION_MAV
//</code></function>


/*<function id="AllowSpecialElection" confidential="no">
<heading>
  <description>
    <en>Sets a global flag indicating the elector can vote for the special
    election 'Membres Bruxellois du Conseil Flamand' of 2004.</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype>
      <name>Permission</name><description><en>TRUE/FALSE</en></description></parameter>
  </parameters>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void AllowSpecialElection (
   BOOL Permission
   )
// ---------------------------------------------------------------------------
{

   gIsSpecialElectionAllowed = Permission;

   return;
}
//</code></function>

/*<function id="ErrorArc" confidential="no">
<heading>
  <description>
    <en>Redirects errors from arc library to URN/MAV.</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
#pragma argsused
void ErrorArc (
   char* fmt
   )
// ---------------------------------------------------------------------------
{

   #if _DEBUGMODE
      sprintf( gcDcio, "Arc error [%s]", fmt); dcio();
   #endif // _DEBUGMODE

   // decompression library generated an error
   Error_Exit( E_INVALID_DATA);

   return;
}
//</code></function>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="ResetPrng" confidential="no">
<heading>
  <description>
    <en>Reinitializes the pseudo random number generator.</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void ResetPrng (
   void
   )
// ---------------------------------------------------------------------------
{
   unsigned long MavOffset;
   unsigned long Seconds;
   unsigned int  Seed;

   MavOffset   = (unsigned long) atol( mavSessionKey);
   Seconds     = (unsigned long) time( NULL);
   Seed        = (unsigned ) ( ( Seconds + MavOffset) % 65535);

   #if 0 //_DEBUGMODE
      sprintf( gcDcio, "Seconds[%lu] Offset[%lu] Total[%lu], Seed[%u]\n",
         Seconds,
         MavOffset,
         Seconds + MavOffset,
         Seed
         );dcio();
   #endif // _DEBUGMODE

   srand( Seed);

   return;
}
//</code></function>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif // #ifdef _APPLICATION_URN
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
