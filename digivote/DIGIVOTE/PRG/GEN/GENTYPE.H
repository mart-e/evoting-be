/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gentype.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENTYPE.H  $</workFile>
<revision>$Revision: 1.18 $</revision>
<workFileTimeStamp>$Modtime:   Jan 10 2005 11:30:56  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENTYPE.H-arc  $</archive>
<archiveTimeStamp>$Date: 2008/12/01 15:38:26 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENTYPE.H</fileName>
  <creationDate>05/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Definitions générales pour les applications MAV et URN.</fr>
    <nl>Algemene definities voor de MAV en URN applicaties.</nl></description>
  <history>
   01/06/93: revision history starting date
   17/09/93: modification of the data structure to add the 'college' support
             and addaptations made for the language processing
   07/12/98: Add new constant for Key (tab, space)
             Add type enum IndexCount, use to index counters array
  </history>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
/*
   In order to compile the applications properly you must define either
   _APLICATION_URN either _APPLICATION_MAV
*/
#ifndef _APPLICATION_MAV
#  ifndef _APPLICATION_URN
#     error Must define APPLICATION (URN or MAV)
#  endif
#endif

#ifndef _GENTYPE                      // avoid mutiple inclusions of this file
   #define _GENTYPE

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <alloc.h>   // coreleft()

#include "..\gen\dos.h"
#include "..\gen\genlang.h"            // we must know N_LANG
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define BUILD_NUMBER	17.8       // build number used thru out all applications

#define EXT          0x100   // keyboard scan codes
#define RETURN       0x0D
#define ESCAPE       0x1B
#define BACKSPACE    0x08
#define SPACE        0x20
#define TAB          0x09
//</defines>

//=========================== C O N S T A N T S ===============================

const int YES = 1;
const int NO  = 0;

const int NO_ERROR = -1;

const int MAX_ELECTIONS  =  14;

// the maximum number of colleges for one election is dicted by the display
// capabilities. If the MAX_COLLEGES value is changed, you will have to change
// the Select_College function

#ifdef _PREVIEW
   // 20050421 avn - it is assumed there's only one college in the preview.dat
   const int MAX_COLLEGES   =   1;
#else
   const int MAX_COLLEGES   =   6;
#endif

const int MAX_PARTIES    =  40;  // imposed constant


typedef struct ELECTOR_TYPE
{
    char  key;
    int   AllCountersTitleID;
    int   ValidationMessageID;
    int   UrnMessageID;
    int   SpecialElectMessageID;
};

enum ElectionsList {
   CONSEIL_BXL = 1,     //  1 = Conseil de la Region de Bruxelles-Cap.
                        //  2 = Conseil Communal
                        //  3 = Chambre des Representants
   SENAT = 4,           //  4 = Senat
                        //  5 = Conseil de la Communaute Germanophone
                        //  6 = Conseil Provincial
                        //  7 = Parlement Europeen
                        //  8 = Conseil Flamand
                        //  9 = Conseil Regional Wallon
                        // 10 = OCMW
                        // 11 = Districtraad
   MEMBRES_BXL = 12     // 12 = Membres Bruxellois du Conseil Flamand ( 2004)
   };

#ifdef _PREVIEW
	// permissions not used in preview
#else
const int PERMISSIONS[][MAX_ELECTIONS - 1] =
{                                              // 1 = Allowed, 0 = Not allowed.
//    1  2  3  4  5  6  7  8  9 10 11 12 13
    { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }, // Belg
    { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Belg in het buitenland
    { 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0 }, // European
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // Niet Belg
    { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 }, // VOE1 Vlaams Parlement
    { 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 }, // VOE2 Vlaams Parlement + Kamers
    { 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }  // VOE3 Vlaams Parlement + Europa
};
#endif

// the maximum of candidates is dicted by the screen possibilities, we can
// display a maximum of 92 cells. THe limit is than 90 because their is the
// possibility of using 2 cells for 'Effective' and 'Suppliant'.

const int MAX_CANDIDATES = 90;

const int MAX_LANG = 2;    // the maximum number of languages supported
                           // simultanously

const int MAX_TYPE_ELECTOR = 17; //stefG max of type for elector

//============================ T Y P E D E F S ================================

typedef int BOOL;
typedef unsigned char BYTE;

// party logo definition
typedef struct {
   BYTE Width;                      // in pixels ( max. 255)
   BYTE Height;                     // in pixels ( max. 255)
   } logoType;
// screen position definition
typedef struct {
   int Row;                         // Y coordinate
   int Col;                         // X coordinate
   } positionType;
// rectangular screen area definition
typedef struct {
   positionType Position;           // topleft position
   int Width;
   int Height;
   } areaType;

// 20040302 avn - E_OUT_OF_MEMORY problem
typedef char CandidateNamesType [64];// buffer holding candidate name and surname


typedef enum { LEFT, CENTER, RIGHT} ALIGN;

typedef enum { Totaux = 0,
               Belgian = 1,
               /* European=2, Belgian_outside=3, Voeren1=4, Voeren2=5, Voeren3=6 */
               CardsWithNoTicket = MAX_TYPE_ELECTOR - 1  // goes in last counter
             } IndexCountElector;
// IndexCountElector: index counters array using elector type

// success code
typedef enum { SUCCEEDED, ABORTED, RETRY } RET_CODE;

// what to do in case of error for the floppy function
typedef enum { ABORT, ROLLBACK, IGNORE , CRC_DETECT } ERROR_HANDLING;

// program mode STARTUP = critical phase -> every error is fatal
//              NORMAL  = errors may be recovered
typedef enum { STARTUP, NORMAL} MODE;

typedef enum { NO_COUNTER, ALL_COUNTERS, ONE_COUNTER } SCREEN_TYPE;

// URN startup RESUME = restart after incident
typedef enum { FIRST_TIME, RESUME } STARTUP_MODE;

// Count_Vote operation mode CHECK_IT = only check vote validity
//                           EXECUTE  = check and count the vote in memory
typedef enum { CHECK_IT, EXECUTE } OPCODE_COUNT;

// Indicates if the program is consistent
// CORRUPTED = no more floppy access because we have lost informations
//             or the floppy is defect
typedef enum { CONSISTENT, CORRUPTED } INTEGRITY_STATE;

// MAV indicates type of point where to jump to
typedef enum { NO_JUMP, TO_RESTART_LOOP} JUMP_POINT;

// for performing a seek in the files
typedef long int OFFSET;

// the candidate type
enum CandidateType { Effective, Suppliant };

enum LanguagesList {
   FRENCH   = 0,     // 20040108 avn - values added explicitly
   DUTCH    = 1,
   GERMAN   = 2
   };
enum LingualGroupsList {
   FR_LG    = 1,     // 20040224 avn - values added explicitly
   NL_LG    = 2
   };

// bitflip proof vote counters
typedef struct {
   unsigned int Ctr1;                     // first counter
   unsigned int Ctr2;                     // if different from first we go in corrupted mode
   } VotesCounterType;

// for displaying of candidates
typedef enum {
   MODE14 = 14,      // 20050421 avn - values added explicitly
   MODE22 = 22,
   MODE23 = 23,
   UNDEFINED_ELECT_MODE = 99
   } DISP_ELECT_MODE;

typedef enum { MODE14_1, MODE14_2, MODE14_3,
               MODE22_1, MODE22_2, MODE22_3, MODE22_4,
               MODE23_1, MODE23_2, MODE23_3, MODE23_4,
               UNDEFINED_PARTY_MODE = 99 } DISP_PARTY_MODE;
typedef struct { short X, Y; } POSITION;

// type of display counter
typedef enum { DISPLAY_TOTAUX, DISPLAY_TYPE } MODE_DISP;
typedef enum { ONE, MULTI } MODE_IT;

//-------------- V O T E   S T R U C T U R E   O N   C A R D ------------------

/*[*/
/*
   There are two possible vote types:
   - Type 0 : you may only choose one effective and/or one suppliant
   - Type 1 : you may choose as much effective and suppliant as you want

   For each vote type we have 1 byte used for the college number, 2 bytes
   for the party number (due to card format) and 1 bytes for the type of vote.
   The type of vote can be : 0 party vote
                             1 effective only vote
                             2 suppliant only vote
                             3 effective and suppliant vote

   If the party number is 0, it is a blanco vote.

   For the Type 0 we store the number of the chosen effective and the number
   of the chosen suppliant. (2 bytes for each)
   For the Type 1 there may be more candidates chosen, so we use a binary
   mask in which each bit represents a candidate number. But we can only
   code 10 value on each card position, this is 3 1/2 bits, thus in the 8
   bits of a char we can only use 3 bits, the others are not usable.
   There may be maximum 90 candidates (due to display limit), and we have
   3 bits per char to code them, we dus need 30 chars to be able to code
   all the candidates.
   The suppliants bits follow the effecitves bits, the distinction is possible
   because we know the party they belong to, and then we know the number of
   effectives and suppliants for that party.

   The size on card for Type 0 is 8
                        Type 1 is 34
*/
/*]*/

typedef struct
{
   char College;
   char Party [2];
   char TypeVote;
   char NrCandidate [2];
   char NrSuppleant [2];

} VOTE_TYPE_0;

typedef struct
{
   char College;
   char Party [2];
   char TypeVote;

   struct
   {
      char bit1  : 1;
      char bit2  : 1;
      char bit3  : 1;
      char dummy : 5;

   } Candidate [(MAX_CANDIDATES+2) / 3];          // we can code 90 candidates

} VOTE_TYPE_1;


//------ D Y N A M I C   E L E C T I O N   D A T A   S T R U C T U R E S ------

typedef struct Session    * SessionList;
typedef struct Election   * ElectionList;
typedef struct College    * CollegeList;
typedef struct Party      * PartyList;
typedef struct Candidate  * CandidateList;
typedef struct Referendum * ReferendumList;

//-----------------------------------------------------------------------------

struct ElectionMain            // This is the root entry point
{
   int NbrOfElections;         // total number of election

   int Language [MAX_LANG];    // the languages that can be used for the
                               // messages, NONE to set the limit of supported
                               // languages less than MAX_LANG;
   int iBureauWithTicketing;   // ticketing required in this voting bureau?

#ifdef _APPLICATION_MAV
   char* cpBureauName;         // voting bureau id
   char* cpElectionDate;       // holds the election date ex: "18/05/2003"
#endif

#ifdef _APPLICATION_URN
   SessionList FirstSession;   // the session informations (type A in B021)
#endif

   ElectionList FirstElection; // the pointer to the first election in the list
};

//-----------------------------------------------------------------------------

struct Session            // the list of general informations (type A in B021)
{
   int Tag;                         // the information type
   char* Text;                      // the information text
   SessionList Next;                // pointer to the next session information
};

//-----------------------------------------------------------------------------

struct Election                // the structure allocated for every election
{
   int   Ind;                  // election index number
   int   Type;                 // election mode 0=single vote, 1=multiple votes allowed
   int   NbrOfColleges;        // total number of colleges for that election
   BOOL  HasLanguageGroups;    // TRUE = election with language groups

   int   MaxSuppliants;        // max. # suppliants in this election
   int   MaxEffectives;        // max. # Effectives in this election

   ElectionList Next;          // pointer to the next Election
   CollegeList  FirstCollege;  // pointer to the list of colleges

   char* Long_Name [N_LANG];   // the complete name of the election in all
                               //  the supported languages
   char* Short_Name [N_LANG];  // the short name of the election

#ifdef _APPLICATION_URN
   VotesCounterType  BlankVotes;    // number of blank votes in the election
   VotesCounterType  TotalVotes;    // number of blank votes or party votes
                                    //  in the election
#endif

};

//-----------------------------------------------------------------------------

struct College                  // the structure allocated for each college
{
   int Ind;                     // the college index number
   int NbrOfParties;            // total number of parties for that election

   CollegeList Next;
   PartyList FirstParty;

   char * Long_Name [N_LANG];   // the complete name of the college in all the
                                // supported languages
};

//-----------------------------------------------------------------------------

struct Party      // the structure allocated for each party, for each election
{
   int           Ind;            // the party index number
   PartyList     Next;           // pointer to the next party in the election
   CandidateList FirstEffective; // pointer to the effective candidate list
   CandidateList FirstSuppliant; // pointer to the suppliant candidate list

#ifdef _APPLICATION_MAV
   char * Name;                  // the party name
   int  LanguageGroup;           // the language group of the party

   logoType Logo;                // party logo properties
#endif

   int NbrOfEffectives;          // the number of effective in that party
   int NbrOfSuppliants;          // the number of suppliants in that party

#ifdef _APPLICATION_URN          // number of votes for
   VotesCounterType HeadVotes;              // headvote only
   VotesCounterType EffectiveVotes;         // Effective(s) only
   VotesCounterType SuppliantVotes;         // Suppliant(s) only
   VotesCounterType MixedVotes;             // Effective(s) and Suppliant(s)
#endif
};

//-----------------------------------------------------------------------------

struct Candidate       // the structure allocated for each effective/suppliant
{
   int   Ind;            // the candidate index number
   CandidateList Next;   // the pointer to the next candidate in the same list

#ifdef _APPLICATION_MAV
   char* Name1;          // the candidates name and surname
#endif

#ifdef _APPLICATION_URN
   VotesCounterType NameVotes;     // total number of votes emitted on that candidate
#endif
};

//-----------------------------------------------------------------------------

struct Referendum      // the structure allocated for each question
{
   int   Ind;            // the question index number
   ReferendumList Next;  // the pointer to the next candidate in the same list

#ifdef _APPLICATION_MAV
   char * Question1;     // The Dutch  Question
   char * Question2;     // The French Question
   char * Question3;     // The German Question
#endif

#ifdef _APPLICATION_URN
   VotesCounterType Votes_YES;      // total number of votes emmitted for the question
   VotesCounterType Votes_NO;       // total number of votes emmitted against the question
   VotesCounterType Votes_BLANCO;   // total number of votes without any opinion
#endif
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*
   The next 2 structures are used by the MAV for display purpose only, they
   are used to store the party list or the candidate list to display.
*/

//----------------- F O R   D I S P L A Y   P U R P O S E ---------------------

struct Select_College
{
   char * ElectionName;     // the election long name in the used language
   int    NbrOfColleges;    // the total number of colleges for that election

   struct
   {
      int Ind;      // college index
      char * Name;  // college name

   } Choice [MAX_COLLEGES];  // list of all the colleges
};

//-----------------------------------------------------------------------------

struct Select_Party
{
   char* ElectionName;          // the election long name in the used language
   char* ElectionSName;         // the election short name in the used language
   char* CollegeName;           // the college long name in the used language
   BOOL  DisplayCol;            // indicates if the college must be displayed
   int   NbrOfParties;          // the total number of parties in that college
   DISP_ELECT_MODE ElectionMode;// Display mode for this election
   BOOL  SuppliantsPresent;     // TRUE if a suppliant is present in any party

   int ElectionInd;             // holds the selected election index number
   int CollegeInd;              // holds the selected college index number
   int GroupInd;                // holds the selected languagegroup if any
   int Blanco_Vote;             // indicates if we chose the blanco vote
   int Selected;                // holds the selected party index number

   struct
   {
      int      Ind;             // party index number
      char*    Name;            // party name
      int      LanguageGroup;   // the language group of the party
      POSITION DisplayBox;      // party's coordinates of the displaybox
      logoType Logo;            // party logo properties

   } Choice [MAX_PARTIES];      // list of all the parties

//dcv 060595   POSITION  WhiteDisplayBox[2]; // White Vote's coordinates of displaybox
   POSITION WhiteDisplayBox[3]; // White Vote's coordinates of displaybox
   int      Width;              // Width of the displaybox for the parties
   int      Height;             // Height of the displaybox for the parties

   areaType BackButton;         // screen area reserved for the back button
};

//-----------------------------------------------------------------------------

struct Select_Candidate
{
   char * ElectionName;     // the election short name in the used language
   int    ElectionType;     // election type 0 = single vote, 2 = multiple
   BOOL   DisplayCol;       // indicates if the college must be displayed
   int    CollegeInd;       // the chosen college index number
   int    PartyInd;         // the chosen party index number
   char * PartyName;        // the chosen party name
   DISP_PARTY_MODE PartyMode;// Display mode for all candidates of this party
   POSITION TitleSuppliant; // Coordinates of the suppliant titlebox; if present

   int Cancel;              // return to party selection selected
   int Party_Vote;          // party vote selected
   int EffectivesSelected;  // Number of selected effective candidates
   int SuppliantsSelected;  //                    suppliant

   int NbrOfEffectives;     // the total number of effectives in that party

   struct
   {
      int      Flag;       // candidate is selected
      int      Ind;        // candidate index number
      char*    Name1;      // candidate's name and surname
      POSITION DisplayBox; // Candidate's coordinates of the displaybox
   } Effective[MAX_CANDIDATES];  // list of all the effectives in that party

   int NbrOfSuppliants;     // the total number of suppliants in that party

   struct
   {
      int      Flag;       // candidate is selected
      int      Ind;        // candidate index number
      char*    Name1;      // candidate's name and surname
      POSITION DisplayBox; // Candidate's coordinates of the displaybox
   } Suppliant[MAX_CANDIDATES]; // list of all the suppliants in that party

   int ElectionInd;          // holds the selected election index number
   logoType PartyLogo;       // party logo properties

};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif // _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif                                              // end of #ifndef _GENTYPE
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->
