/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ticket.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <revision>$Revision: 1.2 $</revision>
  <archiveTimeStamp>$Date: 2005/07/01 15:13:18 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>ticket.cpp</fileName>
  <creationDate>08/2002</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>Librairie de fonctions impression ticket</fr>
    <nl>Functielibrary afdruk ticket</nl></description>
  <history/>
  <comment>
  This library provides functionality for the ticketing project, printer
  specific functionality is confined into a separate library.

  Remember that election data is discarded from memory after each election
  processed. Therefore we have to spool the printout to the ramdrive after
  each election. When all elections are processed we send those files to the
  printer.</comment>
</comments>*/
//<header>
//============================================================= BOF HEADER ===

//<includes>
//--------------------------------------------------------------- INCLUDES ---
#include <conio.h>              // getch(), gotoxy(), clreol()
#include <dos.h>                // sleep()
#include <string.h>             // memset()
#include <stdlib.h>             // atoi(), atol()

#include "..\gen\gentype.h"     // defines BOOL, BYTE
#include "..\gen\gencard.h"     // defines BOOL, BYTE
#include "..\gen\gendata.h"     // Build_Election(), Destroy_Election(), ...
#include "..\mav\mavmes.h"      // Error_Exit(), ...
#include "..\mav\mavvideo.h"    // Screen_White(), ...
#include "..\mav\mavlight.h"

#include "..\mav\sprinter.h"
#include "..\mav\ticket.h"

/* :WARNING: avn 20020201 -> turn OFF for production code! */
#define _DEBUGMODE      0       // debug mode OFF = 0, ON = 1
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                         declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---

// line feeds to move the ticket in the viewing window
#define WHITE_SPACE            42
#define QUIET_SPACE           33
#define BYTES_PER_BC          30
#define LINES_FOR_BC          5

// Error codes returned by this library
#define NO_ERRORS             0        // so far so good :-)

#define ERROR_LIST_OFFSET     -2000    // should be different for every lib
#define BMP_NOT_FOUND         -2001    // BMP file not found
#define UPLOAD_ERROR          -2002    // comm. errors during upload
#define PRINT_ERROR           -2003    // error during printing

#define CREATE_SPOOL_ERROR    -2010    // cannot create spool file
#define OPEN_SPOOL_ERROR      -2020    // cannot open spool file
#define PRINTER_OFFLINE       -2030    // printer not available
#define UNDEFINED_COMMAND     -2040    // undefined printer command
#define UNDEFINED_FUNCTION    -2050    // undefined ticket function

//--------------------------------------------------------------- TYPEDEFS ---
typedef enum { SPOOL, PRINT, OFFLINE } PMODE;

//<prototypes>
//------------------------------------------------------------- PROTOTYPES ---
extern void Process_Election( ElectionList Election);

static int sendPrinterCommand( int Cmd);
static int spoolVotes( void);
static int printVotes( BYTE* PhysicalCardData);
static int dropVotes( void);

static void dropTicket( void);

static int reviewTicket( void);
static void reviewVotes( void);

static int printLine( char text[]);
static int printChar( char chr);
static int uploadLogo( void);

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern struct ElectionMain Root;            // printVotes()
extern struct Select_Party Parties;         // spoolVotes(), printVotes()
extern struct Select_Candidate Candidates;  // spoolVotes(), printVotes()

// defined in gendata.cpp
extern int Allready_Voted;
extern char Card [MAX_VOTE_BUF];              // the write card buffer
extern int Offset;
extern VOTE_TYPE_0  Vote_0;
extern VOTE_TYPE_1  Vote_1;
extern int Vote_0_Type;

//----------------------------------------------------------------------------
//                                              definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---

enum PRINTERCOMMANDS
   {
   RESET, BEEP, TESTFORM, CUT, UPSIDEDOWN, REVERSE, COMPRESSED, CODEPAGE,
   MARGINS, BOLD, CENTERED, PRLOGO, BC_HEADER, BC_FOOTER, BIG
   };

struct
{
   int   iInd;             // election counter - names the spoolfiles in sequence
   char  cRamDrive;        // ramdrive letter ( C default)
   char  cFileName[13];    // holds dynamic spoolfile name
   FILE* SpoolFile;        // handle to the spoolfile
   PMODE iMode;            // are we spooling, direct printing or in error ?
} gs;

static BOOL gInitReport = TRUE;
static BOOL gIsTicketPending = TRUE;
                           // used to cut off pending tickets after a crash


//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===
//------------------------------------------------------ PRIVATE FUNCTIONS ---

//<function id="spoolVotes">
/*<heading>
    <description><en>Process the electors choices. To save memory all data
    concerning an election is discarded from memory after each election, so we
    must spool the data that has to be printed until all elections have been
    processed. When the elector has finished voting for every election we will
    read these spoolfiles back in and send them to the printer.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int spoolVotes ( void)
// ---------------------------------------------------------------------------
{
#if 0
      // remove these later...
      int iLang[3] = { M_LANG_GROUP_F, M_LANG_GROUP_N, M_LANG_GROUP_D };
#else
      // 20040105 avn - used M_LANG_l instead of M_LANG_GROUP_l
      int iLang[3] = { M_LANG_F, M_LANG_N, M_LANG_D };
#endif
      int i;
      char cLine[64]; // paper holds maximum 56 characters / line

   // compose dynamic filename
   gs.iInd++;
   sprintf( gs.cFileName, "%c:\\SPOOL%d.BIN", gs.cRamDrive, gs.iInd);

   // open file for writing ( binary to avoid added CRLF)
   if ( (gs.SpoolFile = fopen( gs.cFileName, "wb")) == NULL)
   {

#if _DEBUGMODE
      sprintf( gcDcio, "genDetail cannot open spoolfile %s.", gs.cFileName); dcio();
#endif  // _DEBUGMODE

      return CREATE_SPOOL_ERROR;
   };

   // we cannot print until elector has finished voting...
   gs.iMode = SPOOL;

   // must spool lines in reverse order from here because we will be
   // printing upside down!!
   if ( !Parties.Blanco_Vote)
   {
      sendPrinterCommand( COMPRESSED);
      // scan suppliants if any ( reverse order for upsidedown printing!)
      if ( Candidates.NbrOfSuppliants)
      {
          BOOL iFound = FALSE;

          for ( i = Candidates.NbrOfSuppliants - 1; i >= 0; i--)
          {
              // suppliant selected?
              if ( Candidates.Suppliant[i].Flag)
              {
                    iFound = TRUE;
                    {
                       sprintf( cLine, " %s ",
         // :TODO: 20040302 avn - E_OUT_OF_MEMORY problem
                          Candidates.Suppliant[i].Name1);
                    }
                    printLine( cLine);
              }
          }

          if ( iFound)
          {
              // mark difference with effectives
              sendPrinterCommand( CENTERED);
                 sprintf( cLine, " %s ",
                     Get_Text( SUPPLIANT, Working_Language( ASK)));
                 printLine( cLine);
                 printLine( " ");
              sendPrinterCommand( CENTERED);
          }
      }
      // scan candidates ( reverse order for upsidedown printing!)
      for ( i = Candidates.NbrOfEffectives - 1; i >= 0; i--)
      {
          // candidate selected?
          if ( Candidates.Effective[i].Flag)
          {
             {
                sprintf( cLine, "%s ",
         // :TODO: 20040302 avn - E_OUT_OF_MEMORY problem
                   Candidates.Effective[i].Name1);
             }
              printLine( cLine);
          }
      }
      sendPrinterCommand( COMPRESSED);
      sendPrinterCommand( BOLD);
      // print party name
      if ( Candidates.Party_Vote)
      {
          // head vote
          sprintf( cLine, "%d - %s - %s",
              Candidates.PartyInd, Candidates.PartyName,
              Get_Text( T_HEADVOTE, Working_Language( ASK)));
      }
      else
      {
          // no head vote
          sprintf( cLine, "%d - %s",
              Candidates.PartyInd, Candidates.PartyName);
      }
      printLine( cLine);

      // print language group
      if ( !Parties.DisplayCol)
      {
          // find language group (if any)
          for ( i=0; i <= MAX_PARTIES; i++)
          {
              // found LanguageGroup of party
              if ( Parties.Choice[i].Ind == Candidates.PartyInd)
              {
                  // valid language?
                  if ( Parties.Choice[i].LanguageGroup >= 0 &&
                      Parties.Choice[i].LanguageGroup <= 2)
                  {
                      sendPrinterCommand( CENTERED);
                      sendPrinterCommand( COMPRESSED);
                          sprintf( cLine, " %s",
                              Get_Text( iLang[ Working_Language( ASK)],
                              Parties.Choice[i].LanguageGroup));
                          printLine( cLine);
                      sendPrinterCommand( COMPRESSED);
                      sendPrinterCommand( CENTERED);
                  }
              }
          }
      }
   }
   else
   // if none print 'blanco'
   {
      sendPrinterCommand( BOLD);
      sprintf( cLine, "%s", Get_Text( M_BLANCO_VOTE, Working_Language( ASK)));
      printLine( cLine);
   }

   sendPrinterCommand( CENTERED);
   printLine( " ");
   // print college name (if any)
   if ( Parties.DisplayCol)
   {
      sendPrinterCommand( COMPRESSED);
          sprintf( cLine, " %s ", Parties.CollegeName);
          printLine( cLine);
      sendPrinterCommand( COMPRESSED);
   }

   // print short election name
   sendPrinterCommand( BIG);
      sprintf( cLine, " %s ", Parties.ElectionSName);
      printLine( cLine);
   sendPrinterCommand( BIG);
   sendPrinterCommand( CENTERED);
   sendPrinterCommand( BOLD);

   // close spool file
   fclose( gs.SpoolFile);

   return NO_ERRORS;
}
//</code></function>


//<function id="printVotes">
/*<heading>
    <description><en>Terminate report. We read the spoolfiles back in and send
    them to the printer, then we ask the elector to cancel OR confirm and
    print out his choice. Wether he cancels or confirms will be recorded on
    the magnetic card, canceled votes will not be accepted at the URN.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg">
      <datatype>BYTE</datatype><name>PhysicalCardData</name>
      <description><en>Card's physical data ( barcode)</en></description>
    </parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int printVotes ( BYTE* PhysicalCardData)
// ---------------------------------------------------------------------------
{
      char  cLine[64]; // paper holds maximum 56 characters / line
      BYTE  c;
      int   i;
      int   iRet = 0;
      int   iWhiteSpace;

      int iBureauNbr;
      long lBureauNbr;

   // get last 3 digits of cpBureauName
   if ( strlen( Root.cpBureauName) <= 3)
   {
      iBureauNbr = 0;
   }
   else
   {
      // the bureau id is a quoted string from which we need the 3 last digits
      strncpy( cLine, Root.cpBureauName + ( strlen( Root.cpBureauName) - 4), 3);
      cLine[ 3] = 0x00;
      iBureauNbr = atoi( cLine);
   }

   // get first 8 digits of PhysicalCardData
   strncpy( cLine, PhysicalCardData, 8); cLine[ 8] = 0x00;
   lBureauNbr = atol( cLine);

   // begin ticket 'transaction'
   gIsTicketPending = TRUE;

   // use direct printing
   gs.iMode = PRINT;

   // init printer
   sendPrinterCommand( RESET);

   // setup printer
   sendPrinterCommand( CODEPAGE);
   sendPrinterCommand( MARGINS);

   // print some blanks to start moving the paper, this should avoid that the first line
   //  gets printed weird ( kind of enlarged :-)
   printLine( ""); printLine( ""); printLine( "");

   // start printing
   sendPrinterCommand( UPSIDEDOWN);

   // footer
   sendPrinterCommand( CENTERED);
   sendPrinterCommand( BOLD);
   sprintf( cLine, "--- EINDE TICKET ---");
   printLine( cLine);
   printLine( "");
   sendPrinterCommand( BOLD);
   sendPrinterCommand( CENTERED);

   // send spoolfiles to printer (last election first when printing upsidedown!)
   for ( i = gs.iInd; i > 0; i--)
   {

      // open spoolfile ( binary to avoid CRLF)
      sprintf( gs.cFileName, "%c:\\SPOOL%d.BIN", gs.cRamDrive, i);
      if ( (gs.SpoolFile = fopen( gs.cFileName, "rb")) == NULL)
      {

#if _DEBUGMODE
         sprintf( gcDcio, "printVotes cannot open spoolfile %s.", gs.cFileName); dcio();
#endif  // _DEBUGMODE

         return OPEN_SPOOL_ERROR;
      };

      // send spoolfile contents to device
      for (;;)
      {
         c = fgetc( gs.SpoolFile);
         if ( feof( gs.SpoolFile))
         {
            printLine( ""); // leave a blank between elections
            break;
         }
         else
         {
            printChar( c);
         }
      }

      // close spoolfile
      fclose( gs.SpoolFile);
   } // eo for

   // header
   sendPrinterCommand( CENTERED);

   // election date
   sendPrinterCommand( BOLD);
   // :TODO: avn 20030331 - should be a mnemonic eventually...
   sprintf( cLine, "%s", Root.cpElectionDate);
   printLine( cLine);
   printLine( "Verkiezingen van");
   sendPrinterCommand( BOLD);

   // compute number of lines to center barcodes
   iWhiteSpace = ( WHITE_SPACE -
      ( ( ( strlen( PhysicalCardData) / BYTES_PER_BC) + 1) * LINES_FOR_BC)) / 2;

   // feed paper so that the elector can see his votes thru the window
   for ( i = 0; i < iWhiteSpace; i++) printLine( "");

   sendPrinterCommand( BC_HEADER);
   // dump card content in barcodes
   for ( i = 0; PhysicalCardData[i]; i++ )
   {
      printChar( PhysicalCardData[i]);
      if ( !( ( i+1) % BYTES_PER_BC))
      {
         sendPrinterCommand( BC_FOOTER);
         printLine( "");
         sendPrinterCommand( BC_HEADER);
      }
   }
   // complete last barcode with zeros ( reader has trouble with variable length bc)
   while ( i % BYTES_PER_BC)
   {
     printChar( '0');
     i++;
   }
   sendPrinterCommand( BC_FOOTER);
   printLine( "");

   // barcode with bureau id ( we add mav session key to make different barcode patterns)
   ltoa( lBureauNbr + iBureauNbr, cLine, 10);
   sendPrinterCommand( BC_HEADER);
   // print leading zeros first ( reader has trouble with variable length bc)
   i = 1;
   while ( i % ( BYTES_PER_BC - strlen( cLine) + 1))
   {
      printChar( '0');
      i++;
   }
   printLine( cLine);
   sendPrinterCommand( BC_FOOTER);

   // feed other half of whitespace
   for ( i = 0; i < iWhiteSpace; i++) printLine( "");

   sendPrinterCommand( BOLD);

   // gimme da logo
   sendPrinterCommand( PRLOGO);

   for ( i = 0; i < 2; i++) printLine( "");

   // test printer status
   if ( OFFLINE == gs.iMode)
   {
      return PRINTER_OFFLINE;
   }

   iRet = reviewTicket();
   sendPrinterCommand( BIG);
   if ( iRet)
   {
      sprintf( cLine, " === %s === ",
         Get_Text( T_VOUCHER_CONFIRMED, Working_Language( ASK)));
      printLine( cLine);
   }
   else
   {
      sprintf( cLine, " === %s === ",
         Get_Text( T_VOUCHER_CANCELED, Working_Language( ASK)));
      printLine( cLine);
   }
   sendPrinterCommand( BIG);

   // finally test printer status to see if the printer survived
   if ( OFFLINE == gs.iMode)
   {
      return PRINTER_OFFLINE;
   }

   return iRet;    // returns cancel/confirm
}
//</code></function>


//<function id="dropVotes">
/*<heading>
    <description><en>Terminates report by cutting off the ticket. As this operation takes
    several seconds ( needed for the physical transport to the urn), it has been postponed
    until the card is written. By doing so we reduce the time in which the paper en the
    electronic data are inconsistent...</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int dropVotes ()
// ---------------------------------------------------------------------------
{
      char  cLine[64]; // paper holds maximum 56 characters / line
      int   iRet = 0;

   printLine( "");
   sprintf( cLine, "--- BEGIN TICKET ---");
   printLine( cLine);

   sendPrinterCommand( BOLD);
   sendPrinterCommand( CENTERED);

   sendPrinterCommand( UPSIDEDOWN);

   dropTicket();

   // finally test printer status to see if the printer survived
   if ( OFFLINE == gs.iMode)
   {
      return PRINTER_OFFLINE;
   }
   else
   {
      // end ticket 'transaction'
      gIsTicketPending = FALSE;
   }

   return iRet;    // returns cancel/confirm
}
//</code></function>


//<function id="dropTicket">
/*<heading>
    <description><en>Drops the ticket in the container.</en></description>
   <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static void dropTicket ( void)
// ---------------------------------------------------------------------------
{
      int i;

   // the report ended (abnormally), it must be reset
   gInitReport = TRUE;

   // feed ticket past the cutter
   for ( i = 0; i < 5; i++) printLine( "");
   sendPrinterCommand( CUT);
   sendPrinterCommand( BEEP);

   // feed paper until it reaches the 'conveyor belt' to stop the noise of the engine...
   sleep( 3); // allows the current ticket to disappear!
   for ( i = 0; i < QUIET_SPACE; i++) printLine( "");

   sendPrinterCommand( RESET);

   return;
}
//</code></function>


//<function id="reviewTicket">
/*<heading>
   <description><en>Controls reviewing process.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description><en>TRUE indicates confirmation, FALSE indicates cancel.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  <comment></comment>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static BOOL reviewTicket ( void)
// ---------------------------------------------------------------------------
{
        BOOL iRet;

    // display info screen
    Display_Message( I_REVIEW_ELECTOR, YES, 0, NO, 9, 10, 1, 1);

    // display electors votes
    reviewVotes();

    // ask confirmation from elector
    if ( DisplayTwoButtonDialog( "", M_VOUCHER, B_NO, B_YES))
    {
        // elector confirmed
        iRet = TRUE;
    }
    else
    {
        // elector canceled, trigger alarm to call mr. president
        WaitForPresident( M_PRESIDENT);

        // display info screen
        Display_Message( I_REVIEW_PRESIDENT, YES, 0, NO, 9, 10, 1, 1);

        // display electors votes
        reviewVotes();

        // ask confirmation from president
        if ( DisplayTwoButtonDialog( "", M_VOUCHER, B_NO, B_YES))
        {
            // president confirmed
            iRet = TRUE;
        }
        else
        {
            // president canceled
            iRet = FALSE;
        }
    }

    return iRet;    // returns cancel/confirm
}
//</code></function>


//<function id="reviewVotes">
/*<heading>
   <description><en>Display votes in order to check consistency with the ticket.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  <comment>Votes are displayed as in the 'review' votes mode.
  </comment>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static void reviewVotes ( void)
// ---------------------------------------------------------------------------
{
    // initialize buffer for new vote
    Init_Card_Buffer();

/* :TODO: avn 20021216 - avoid magic numbers here! */
    // simulate 'review' mode
    Allready_Voted = 2;     // 0=Vote mode, 1=Visu mode, 2=Verify mode

    for ( ElectionList LOOP = Root.FirstElection;
          LOOP != NULL; LOOP = LOOP->Next)
    {

        // copy votes for this election from card buffer
        switch( LOOP->Type )
        {
            case 0:
                memcpy( &Vote_0, &Card[Offset], sizeof( Vote_0 ));
                Offset += sizeof( Vote_0 );
                Vote_0_Type = 1;
                break;
            case 1:
                memcpy( &Vote_1, &Card[Offset], sizeof( Vote_1 ));
                Offset  += sizeof( Vote_1 );
                Vote_0_Type = 0;
                break;
        }

        // processes the college selection for the election
        Process_Election( LOOP);

        // clears the graphics screen
        Screen_White();
        sleep(1);

        // clear election data from memory ( was build in Process_Election)
        if ( Destroy_Election( LOOP -> Ind) == ABORTED)
        {
           Error_Exit( M_HALTED); //JRU: Destroy Election From Memory (Candidates)
        }

    }

    // set 'review' mode off
    Allready_Voted = 0;

    return;
}
//</code></function>

//<function id="sendPrinterCommand">
/*<heading>
   <description><en>Send a command to the printer.</en></description>
   <parameters>
     <parameter type="return">
       <datatype>int</datatype></parameter>
     <parameter type="arg">
       <datatype>int</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int sendPrinterCommand ( int iCmd)
// ---------------------------------------------------------------------------
{
      /* :TODO: avn 20020917 - use a bitfield here to save memory (?) */
      static char cReverse = 0x01;        // toggle the different effects
      static char cUpsideDown = 0x01;
      static char cCompressed = 0x01;
      static char cBig = 0x11;
      static char cCenter = 0x01;
      static char cBold = 0x01;

    switch ( iCmd)
    {
        case ( BC_HEADER): // initialises barcode printing
            // select Printing Position of HRI Characters ( 0 = suppress, 2 = below)
            printChar( 0x1D); printChar( 0x48); printChar( 0x00);
            // select Bar Code Height
            printChar( 0x1D); printChar( 0x68); printChar( 0x50); // 80 dots
            // select Bar Code Width
            printChar( 0x1D); printChar( 0x77); printChar( 0x02); // Min=2, Def=3, Max=6
            // selects bar code type
            printChar( 0x1D); printChar( 0x6B); printChar( 0x05); // interleaved 2 of 5 ( ASCII numerals only 48-57, NULL terminated!)
            break;

        case ( BC_FOOTER): // terminates barcode printing
            printChar( 0x00);
            // cancel rotated print
            //printChar( 0x1B); printChar( 0x56); printChar( 0x00);
            break;

        case ( RESET):
            // Initialize Printer
            //printChar( 0x1B); printChar( 0x40);

            // Clear Printer
            printChar( 0x10);
            // allow printer to reboot
            delay( 500);

            // reset the different effects
            printChar( 0x1D); printChar( 0x42); printChar( 0x00);
            cReverse = 0x01;

            printChar( 0x1B); printChar( 0x7B); printChar( 0x00);
            cUpsideDown = 0x01;

            printChar( 0x1B); printChar( 0x16); printChar( 0x00);
            cCompressed = 0x01;

            printChar( 0x1B); printChar( 0x21); printChar( 0x00);
            cBig = 0x11;

            printChar( 0x1B); printChar( 0x47); printChar( 0x00);
            cBold = 0x01;

            printChar( 0x1B); printChar( 0x61); printChar( 0x00);
            cCenter = 0x01;
            break;

        case ( BEEP): // Generate Tone
            printChar( 0x1B); printChar( 0x07);
            break;

        case ( TESTFORM): // Print Test Form
            printChar( 0x1F); printChar( 0x74);
            break;

        case ( CUT): // Perform Full/Partial Knife Cut
            printChar( 0x1B); printChar( 0x69); // full cut
#if 0
            printChar( 0x1B); printChar( 0x6D); // partial cut
#endif
            break;

        case ( UPSIDEDOWN): // Select or Cancel Upside Down Print Mode
            printChar( 0x1B); printChar( 0x7B); printChar( cUpsideDown);
            if ( cUpsideDown)
                cUpsideDown = 0x00;
            else
                cUpsideDown = 0x01;
            break;

#if 0
        case ( REVERSE): // Select or Cancel White/Black Reverse Print Mode
            printChar( 0x1D); printChar( 0x42); printChar( cReverse);
            if ( cReverse)
                cReverse = 0x00;
            else
                cReverse = 0x01;
            break;
#endif

        case ( COMPRESSED): // Select Pitch ( Standard/Compressed)
            printChar( 0x1B); printChar( 0x16); printChar( cCompressed);
            if ( cCompressed)
                cCompressed = 0x00;
            else
                cCompressed = 0x01;
            break;

        case ( BIG): // Select Character Size
            printChar( 0x1B); printChar( 0x21); printChar( cBig);
            if ( cBig)
                cBig = 0x00;
            else
                cBig = 0x11; // double width & height
            break;

        case ( CODEPAGE): // Select Character Code Table
            printChar( 0x1B); printChar( 0x74); printChar( 0x01); // Code page 850
            break;

        case ( MARGINS):
            // Set Left Margin ( at 1 cm)
            printChar( 0x1D); printChar( 0x4C); printChar( 0x45); printChar( 0x00);
            // Set Printing Area Width ( at 5,7 cm)
            printChar( 0x1D); printChar( 0x57); printChar( 0xCC); printChar( 0x01);
            break;

        case ( BOLD): // Select or Cancel Double Strike
            printChar( 0x1B); printChar( 0x47); printChar( cBold);
            if ( cBold)
                cBold = 0x00;
            else
                cBold = 0x01;
            break;

        case ( CENTERED): // Select Justification ( center)
            printChar( 0x1B); printChar( 0x61); printChar( cCenter);
            if ( cCenter)
                cCenter = 0x00;
            else
                cCenter = 0x01;
            break;

        case ( PRLOGO): // Print Downloaded Bit Image
            printChar( 0x1D); printChar( 0x2F); printChar( 0x00);
            break;

        default:

#if _DEBUGMODE
            sprintf( gcDcio, "sendPrinterCommand ( ?)."); dcio();
#endif  // _DEBUGMODE

            return UNDEFINED_COMMAND;
    }   // switch

    return NO_ERRORS;
}
//</code></function>


//<function id="printLine">
/*<heading>
    <description><en>Writes a string to the printer ( spooled or directly). </en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg"><datatype>char</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int printLine ( char text[])
// ---------------------------------------------------------------------------
{
        int i = 0;

    // send text
    while ( text[i])
    {
        if ( printChar( text[i]))
        {
            return PRINT_ERROR;  // printChar failed, quit writing
        }
        i++;
    }

    // terminate lines with LF
    return printChar( 0x0A);
}
//</code></function>


//<function id="printChar">
/*<heading>
    <description><en>Sends one character to the printer ( spooled or directly).</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>char</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int printChar ( char c)
// ---------------------------------------------------------------------------
{
        int iRet = 0;

    switch ( gs.iMode)
    {
        case ( SPOOL):
            // send one char to spoolfile
                /* :TODO: avn 20020927 - test return code */
            fprintf( gs.SpoolFile, "%c", c);
            break;

        case ( PRINT):
            // send one char to port
            iRet = ComSend( c);
            if ( iRet)
            {
                // send failed, communication error
                gs.iMode = OFFLINE;
            }
            break;

        case ( OFFLINE):
            // ignore the char 'cause de printer went offline for some reason...
            break;

    }   // eo switch

#if _DEBUGMODE
    if ( iRet)
    {
        sprintf( gcDcio, "printChar returns %d, mode %d", iRet, gs.iMode); dcio();
    }
#endif // _DEBUGMODE

    return iRet;
}
//</code></function>


/*<function id="uploadLogo">
<heading>
  <description><en>Read bmp file and send it to the printer.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int uploadLogo ( void)
// ---------------------------------------------------------------------------
{
      unsigned char c;  // holds one byte
      FILE* BmpFile;    // handle to the bitmap
      char cBmpFileName[13]; // holds dynamic file name

#if _DEBUGMODE
      sprintf( gcDcio, "uploadLogo.begin"); dcio();
#endif // _DEBUGMODE

   // :TODO: avn 20020113 - get ramdrive from function ?
   sprintf( cBmpFileName, "C:\\BIZA.BMP");

   // open file ( binary to avoid CRLF)
   if ( (BmpFile = fopen( cBmpFileName, "rb")) == NULL)
   {
      // open file failed

#if _DEBUGMODE
      sprintf( gcDcio, "uploadLogo.Cannot open [%s].", cBmpFileName); dcio();
#endif // _DEBUGMODE

      return BMP_NOT_FOUND;
   };

   // send escape char
   ComSend( 0x1B);

   // send bmpfile contents to device
   for (;;)
   {
      c = fgetc( BmpFile);
      if ( feof( BmpFile))
      {
         break;
      }
      else
      {
         if ( ComSend( c) < 0)
         {

#if _DEBUGMODE
      sprintf( gcDcio, "uploadLogo.ComSend error!"); dcio();
#endif // _DEBUGMODE

            return UPLOAD_ERROR;
         }
      }
   }

   // close spoolfile
   fclose( BmpFile);

   return NO_ERRORS;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

//<function id="initializeTicketing">
/*<heading>
   <description><en>Initialize the ticketing proces.</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
   </parameters>
   <comment>This function is called ONCE before we enter the infinite election loop!</comment>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int initializeTicketing ( void)
// ---------------------------------------------------------------------------
{
      int iError;

   // printers available?
   if ( !Root.iBureauWithTicketing) return NO_ERRORS;

   // setup and check printer
   iError = setupTicketing();

   if ( !iError)
   {
      // printer is up and running, so we can send the logo ( must be done only once)
      iError = uploadLogo();
   }

   return iError;
}
//</code></function>


//<function id="setupTicketing">
/*<heading>
   <description><en>Initializes the ticketing proces.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en>
      </description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
   <comment>This function is called ONCE every time an elector votes!</comment>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int setupTicketing ( void)
// ---------------------------------------------------------------------------
{
      int iError;

   // printers available?
   if ( !Root.iBureauWithTicketing) return NO_ERRORS;

   // no spooling here
   gs.iMode = PRINT;

   // get rid of the pending ticket if any
   if ( gIsTicketPending)
   {
      dropTicket();
   }

   // setup and check printer
   iError = initializePrinter();

   if ( !iError) gIsTicketPending = FALSE;

#if _DEBUGMODE
   if ( iError)
   {
      sprintf( gcDcio, "initializeTicketing returns %d", iError); dcio();
   }
#endif // _DEBUGMODE

   return iError;
}
//</code></function>


//<function id="printTicket">
/*<heading>
    <description><en>Prints out the electors choices.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iFunc</name>
      <description><en>Print or spool?</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BYTE</datatype><name>PhysicalCardData</name>
      <description><en>Card's physical data ( barcode)</en></description>
    </parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int printTicket ( int iFunc, BYTE* PhysicalCardData)
// ---------------------------------------------------------------------------
{
        int iRet = 0;

    // if there's no ticketing we always return 'confirmed' because the elector's
    //  answer has to be recorded on the card in order to distinguish between
    //  cards that are confirmed, canceled or have not been entered in the mav.
    if ( !Root.iBureauWithTicketing) return 1;

    if ( gInitReport)
    {
       // set drive for spooling (should be ramdrive C)
       gs.cRamDrive = 'C';

       // reset election counter
       gs.iInd = 0;

       // remember reporting is initialized
       gInitReport = FALSE;
    }

    switch ( iFunc)
    {
        case ( SPOOL_VOTES):
        {
            iRet = spoolVotes();
            break;
        }
        case ( PRINT_VOTES):
        {
            iRet = printVotes( PhysicalCardData);
            if (iRet < 0)
            {
                // returned an error
                Red_Light_On( M_NO_PRINTER, NOT_FATAL);
            }
            break;
        }
        case ( DROP_VOTES):
        {
            iRet = dropVotes();
            gInitReport = TRUE;     // next report must be initialized first
            if (iRet < 0)
            {
                // returned an error
                Red_Light_On( M_NO_PRINTER, NOT_FATAL);
            }
            break;
        }
        default:
        {
            iRet = UNDEFINED_FUNCTION;
            break;
        }
    }

#if _DEBUGMODE
    if ( iRet)
    {
        sprintf( gcDcio, "printTicket ( %d) returns %d.", iFunc, iRet); dcio();
    }
#endif  // _DEBUGMODE

    return iRet;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->


