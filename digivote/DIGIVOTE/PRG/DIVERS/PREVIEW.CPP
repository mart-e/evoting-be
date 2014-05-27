/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="preview.cpp"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.10 $</revision>
  <archiveTimeStamp>$Date: 2009/01/21 21:24:27 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
<fileName>PREVIEW.CPP</fileName>
<creationDate>24/03/93</creationDate>
<rights/>
<author>B.S.S.</author>
<description>
  <fr>Programme pour visualiser les listes des candidats</fr>
  <nl>Programma om de kandidaatslijsten te visualiseren</nl></description>
<comment>
    This program allows the PREP to make a preview of what the
    lists should look like as if they were displayed by the MAV.</comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <dos.h>        // getdate(), gettime(), sound(), delay(), nosound()
#include <string.h>     // strcpy(), strcat(), strcmp()
#include <conio.h>      // getch()
#include <stdlib.h>     // exit(), atoi()
#include <graphics.h>   // initgraph(), closegraph()

#include "..\divers\logging.h"

#include "..\gen\gentype.h"
#include "..\gen\genstr.h"             // Alloc_String()
#include "..\gen\genvideo.h"
#include "..\mav\logo.h"               // setLogoPath()

#include "..\mav\display.h"
#include "..\mav\drawings.h"
#include "..\mav\mavvideo.h"


#define _DEBUGMODE	0 // :WARNING: avn 20020201 - must be 0 for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define EOL -1
//</defines>

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static void Usage_Info ();
static void getKey (
   int ForcePrtScr
   );
static RET_CODE Read_Data (
   void
   );
static void Effec_Suppl (
   int   Effectief,
   int   Candidate_Ind,
   char* pSurName,
   char* pName
   );

//</prototypes>

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>

struct ElectionMain     Root;
struct Select_College   Colleges;
struct Select_Candidate Candidates;
struct Select_Party     Parties;

ElectionList            ThisElection;

FILE*                   gFileHandle;
char                    gcFileName[32+1];

int                     Allready_Voted = 0;

//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="preview" type="main">
<heading>
  <description>
    <fr>Programme principal</fr>
    <nl>Hoofdprogramma</nl></description>
  <parameters></parameters>
</heading>
<code>*/
#pragma argsused
#pragma startup  logOnStartup
#pragma exit     logOnExit
// ---------------------------------------------------------------------------
int main (
   int argc,
   char* argv[]
   )
// ---------------------------------------------------------------------------
{
      char cString[80];
      int   lang1,
            lang2,
            iInValidArgumentsError = 0;

   ThisElection = new Election;

   // log command line arguments
	sprintf( cString, "v%4.1f: ", BUILD_NUMBER); // dump build number
   for ( int i = 1; i < argc; i++)
   {
      strcat( cString, argv[i]);
      strcat( cString, " ");
   }
   Add_To_Logging( EMPTY, cString);

   // check arguments
   switch( argc )
   {
      case 4:
         // 3 parameters, first one should be D
         if ( strcmp( argv[1], "D" )) iInValidArgumentsError++;
         break;

      case 6:
         // 5 parameters, first one should be E
         if ( strcmp( argv[1], "E" )) iInValidArgumentsError++;
         ThisElection->MaxEffectives = atoi( argv[2]);
         ThisElection->MaxSuppliants = atoi( argv[3]);
         break;

      default:
         iInValidArgumentsError++;
         break;
   }

   if ( iInValidArgumentsError) Usage_Info();

   Setup_Video();

   /* Define textstyle for header information */
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);

   /* last argument contains path to input file */
   sprintf( gcFileName, "%sPREVIEW.DAT", argv[argc-1]);
   setLogoPath( argv[argc-1]);

   switch( argv[1][0])
   {

      case 'd' : // fall through intended!
      case 'D' :

         // display party list
         if ( Read_Data() == SUCCEEDED)
         {
            lang1 = InitPartyData();
            CalculatGapAndSize();
            FillParty();

            if ( lang1 != NONE)
            {
               if ( lang1 == Root.Language[0])
                  lang2 = Root.Language[1];
               else lang2=Root.Language[0];

               if ( lang2 > 2 )  /* ??? */
               {
                  sound(880);
                  delay(10);
                  nosound();
               }

               Display_Parties( lang1);
               getKey( argv[argc - 2][0] == '1');
               Display_Parties( lang2);
               getKey( argv[argc - 2][0] == '1');
            }
            else
            {
               Display_Parties(lang1);
               getKey( argv[argc - 2][0] == '1');
            }
         }
         else
            Add_To_Logging( ERR, "Error while reading party list");
         break;

      case 'e' : // fall through intended!
      case 'E' :

         // display candidates list
         if ( 0 == atoi( argv[2]) + atoi( argv[3]))
         {
            Add_To_Logging( ERR, "Nothing to do...");
         }
         else
         {
            // display candidates
            if ( Read_Data() == SUCCEEDED)
            {
               Parties.ElectionMode = Calculate_Mode( ThisElection);
               Set_Candidates_Display_Mode(
                  &Root, &Parties, &Candidates, ThisElection->Ind);
               Calculate_Positions( & Root, & Parties, & Candidates, ThisElection->Ind);
               Display_Candidates( &Candidates);
               getKey( argv[argc - 2][0] == '1');
            }
            else
            {
               Add_To_Logging( ERR, "Error while reading candidate list");
            }
         }
         break;

      default  :
         closegraph();
         Usage_Info();
         break;
   }

   closegraph();

   // create flag file to notify Progress we have been running
   sprintf( gcFileName, "%sPREVIEW.RES", argv[argc-1]);
   gFileHandle = fopen( gcFileName, "wb");
   if ( gFileHandle == NULL)
   {
      Add_To_Logging( ERR, "Cannot create RES file");
   }
   else
   {
      fclose( gFileHandle);
   }

   // flush keyboard buffer
   while( kbhit() != 0) getch();

   return 0;
}
//</code></function>

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Usage_Info">
<heading>
  <description>
   <fr>Affichage du mode d' emploi du programme</fr>
   <nl>Tonen van de gebruikswijze van het programma</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Usage_Info ()
// ---------------------------------------------------------------------------
{
   printf( "Usage: preview (D)|(E MaxCand MaxSuppl) (0|1) path\n");
   printf( "-----\n\n");
   printf( " D to make a party list preview\n");
   printf( " E to make a candidate list preview\n\n");
   printf( "   MaxCand  = max # of effectives in the election.\n");
   printf( "   MaxSuppl = max # of suppliants in the election.\n\n");
   printf( "   1 = Dump screen to printer, 0 = No dump.\n\n");
   printf( "   path = location of PREVIEW.DAT and the logo data (if any).\n\n");

   Add_To_Logging( ERR, "Application error - invalid arguments");

   exit( 1);
}
//</code></function>

/*<function id="getKey">
<heading>
  <description/>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void getKey (
   int ForcePrtScr
   )
// ---------------------------------------------------------------------------
{
   if( ForcePrtScr)
     geninterrupt( 0x05);
   else {
     int c = getch();

     if ( c)
       return;

     // the key had an extended code, so read the extended value also
     c = getch();
   }
   return;
}
//</code></function>

/*<function id="Read_Data">
<heading>
  <description>
   <fr>Lecture des données</fr>
    <nl>Inlezen gegevens</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE Read_Data (
   void
   )
// ---------------------------------------------------------------------------
{
		char InLine [256], ch;
		int  i, j = 0, Effectief, Candidate_Ind, lg;

		CandidateNamesType cSurname;
		CandidateNamesType cName;
		char 		cTAB[] 	= "\t";    // holds the field delimiter
		char* 	cToken;

   if ( ( gFileHandle = fopen( gcFileName, "rt")) == NULL) return ABORTED;

   // init candidates
   Colleges.NbrOfColleges = 0;
   Parties.DisplayCol    = FALSE;
   Candidates.Cancel     = FALSE;
   Candidates.Party_Vote = FALSE;
   Candidates.EffectivesSelected = 0;
   Candidates.SuppliantsSelected = 0;
   Candidates.NbrOfEffectives = 0;
   Candidates.NbrOfSuppliants = 0;

	// read records
   while ( fgets( InLine, sizeof( InLine), gFileHandle))
   {
		InLine[ strlen( InLine) - 1] = '\0'; // get rid of the CR

		#if _DEBUGMODE
			sprintf( gcDcio, "Record read[%s]", InLine); dcio();
		#endif // _DEBUGMODE

      switch ( InLine [0])
      {
			case '1' : // line contains language code ( 1=Fr, 2=Nl, 4=De)
			case '2' :
			case '4' : // fall thru intended!
				cToken = strtok( InLine, cTAB);
				if ( cToken) Init_Languages( cToken, cToken);
				break;

      	case 'B' : // line contains election name
				cToken = strtok( InLine, cTAB); // record type

				cToken = strtok( NULL, cTAB); // election id
				if ( cToken) {
					Parties.ElectionInd = atoi( cToken);
					Candidates.ElectionInd = Parties.ElectionInd;
					ThisElection->Ind = Parties.ElectionInd;
				}

				cToken = strtok( NULL, cTAB); // college id

				cToken = strtok( NULL, cTAB); // election long name
				if ( cToken) {
					Colleges.ElectionName = Alloc_String( cToken);
					Parties.ElectionName = Alloc_String( cToken);
				}
				cToken = strtok( NULL, cTAB); // election short name
				if ( cToken) {
					Parties.ElectionSName = Alloc_String( cToken);
					Candidates.ElectionName = Alloc_String( cToken);
				}
				Parties.NbrOfParties = 0;
				Parties.Blanco_Vote  = FALSE;
				Parties.Selected     = FALSE;
				break;

	      case 'C' : // line contains college name
				cToken = strtok( InLine, cTAB); // record type

				cToken = strtok( NULL, cTAB); // election id
				cToken = strtok( NULL, cTAB); // college id
				cToken = strtok( NULL, cTAB); // college long name
				if ( cToken) {
					Colleges.Choice [Colleges.NbrOfColleges].Name = Alloc_String( cToken);
					Colleges.NbrOfColleges++;
					Parties.CollegeName = Alloc_String( cToken);
				}
				break;

      	case 'X' : // line contains flag for the college display
				Parties.DisplayCol = TRUE;
				break;

      	case 'D' :
				cToken = strtok( InLine, cTAB); // record type

				cToken = strtok( NULL, cTAB); // election id
				cToken = strtok( NULL, cTAB); // college id
					//!\ Assuming that all party's in the file have the same college id !
				Parties.CollegeInd = atoi( cToken);
				Candidates.CollegeInd = Parties.CollegeInd;

				cToken = strtok( NULL, cTAB); // partynumber
				Candidates.PartyInd    = atoi( cToken);
				Parties.Choice [j].Ind = atoi( cToken);

				cToken = strtok( NULL, cTAB); // partyname
				Parties.Choice [j].Name = Alloc_String( cToken);
				Candidates.PartyName    = Alloc_String( cToken);

				cToken = strtok( NULL, cTAB); // logo width
				if ( cToken) {
	            Parties.Choice [j].Logo.Width = atoi( cToken);
				}
				else Parties.Choice [j].Logo.Width = 0;

				cToken = strtok( NULL, cTAB); // logo height
				if ( cToken) {
               Parties.Choice [j].Logo.Height = atoi( cToken);
				}
				else Parties.Choice [j].Logo.Height = 0;

				Candidates.PartyLogo.Width = Parties.Choice [j].Logo.Width;
				Candidates.PartyLogo.Height = Parties.Choice [j].Logo.Height;

				lg = EOL;
	         // :WARNING: avn 20030206 - languagegroup must be last field if present!
				cToken = strtok( NULL, cTAB); // language group
				if ( cToken) {
					switch( atoi( cToken))
					{
						case 1:   //french
							lg = 0;
							break;

						case 2:   //dutch
							lg = 1;
							break;

						case 4:   //german
							lg = 2;
							break;

						default:
							lg = EOL;
							break;
					}
				}
				Parties.Choice [j].LanguageGroup =  lg;

				++Parties.NbrOfParties;
				++j;
				break;

      case 'E' :
				cToken = strtok( InLine, cTAB); // record type

				cToken = strtok( NULL, cTAB); // election id
				cToken = strtok( NULL, cTAB); // college id
				cToken = strtok( NULL, cTAB); // party id
				cToken = strtok( NULL, cTAB); // candidate type
				if ( cToken) {
		         Effectief = atoi( cToken); // 0 -> effectief 1 -> suppliant
				}

				cToken = strtok( NULL, cTAB); // candidate number
				if ( cToken) {
					Candidate_Ind = atoi( cToken);
				}
				cToken = strtok( NULL, cTAB); // candidate surname ( ex. Doe)
				if ( cToken) {
					strcpy( cSurname, cToken);
				} else strcpy( cSurname, "");
				cToken = strtok( NULL, cTAB); // candidate name ( ex. John)
				if ( cToken) {
					strcpy( cName, cToken);
				} else strcpy( cName, "");
         		Effec_Suppl( Effectief, Candidate_Ind, cName, cSurname);
				break;

			default:
				// eat the fields we don't need, so we can read next record
				cToken = strtok( InLine, cTAB);
				while ( cToken)
				{
					#if _DEBUGMODE
						sprintf( gcDcio, "Eating token[%s].", cToken); dcio();
					#endif // _DEBUGMODE

					cToken = strtok ( NULL, cTAB);
				}
				break;
      } // eoswitch
   } // eowhile

//rvh2330
   lg = Parties.Choice [0].LanguageGroup;

   if ( lg != EOL)                      // language groups are used
      for ( i = 0; i < MAX_PARTIES; i++)
         if( Parties.Choice [i].LanguageGroup != Root.Language[0])
         {
             Root.Language[1] = Parties.Choice [i].LanguageGroup;
             break;
         }

   // :WARNING: 20040218 avn - bug alert
   // if preview.dat contains language code FRENCH and all parties belong to
   // the FR languagegroup the display is buggy because Root.Language[1] remains
   // at -1, all other combinations work because the initial value of
   // Parties.Choice[i].LanguageGroup happens to be 0 which is the same value as
   // the FR language code... Used following patch:
   if (( EOL != lg) && ( EOL == Root.Language[1]))
   {
      if ( FRENCH == Root.Language[0]) Root.Language[1] = DUTCH;
      if ( DUTCH == Root.Language[0]) Root.Language[1] = FRENCH;
   }

//rvh2330

   fclose( gFileHandle);

   return SUCCEEDED;
}
//</code></function>

/*<function id="Effec_Suppl">
<heading>
  <description>
    <fr>Réservation de mémoire pour un candidat</fr>
    <nl>Reservatie van geheugen voor één kandidaat</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Effec_Suppl (
   int   Effectief,
   int   Candidate_Ind,
   char* pSurName,
   char* pName
   )
// ---------------------------------------------------------------------------
{
   static char j2=0, j1=0;

   CandidateNamesType Names;

   // glue name and surname together using a delimiter
   strcpy( Names, pName);
      strcat( Names, "@");
      strcat( Names, pSurName);

   // reserve memory space for each candidate
   if( Effectief)
   {
      Candidates.Suppliant[j2].Flag  = FALSE;
      Candidates.Suppliant[j2].Ind   = Candidate_Ind;

      // allocating name+surname eats up 48 bytes, so we have a 25% better memory usage
      Candidates.Suppliant[j2].Name1 = Alloc_String( Names);

      Candidates.NbrOfSuppliants++;
      ++j2;
   }
   else
   {
      Candidates.Effective[j1].Flag  = FALSE;
      Candidates.Effective[j1].Ind   = Candidate_Ind;

      // allocating name+surname eats up 48 bytes, so we have a 25% better memory usage
      Candidates.Effective[j1].Name1 = Alloc_String( Names);

      Candidates.NbrOfEffectives++;
      ++j1;
   }

	return;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

