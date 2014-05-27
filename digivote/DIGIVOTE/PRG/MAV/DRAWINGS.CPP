/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="drawings.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   DRAWINGS.CPP  $</workFile>
<revision>$Revision: 1.8 $</revision>
<workFileTimeStamp>$Modtime:   Oct 27 2004 16:10:00  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/DRAWINGS.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2007/11/22 11:00:55 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName></fileName>
  <creationDate></creationDate>
  <rights></rights>
  <author></author>
  <description>
    <fr/>
    <nl/></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <string.h>                    // strlen

#include "..\mav\drawings.h"
#include "..\mav\mavasm.h"
#include "..\mav\mavmes.h"
#include "..\mav\mavvideo.h"
#include "..\mav\logo.h"

#define _DEBUGMODE      FALSE   // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define HDRPADDINGTOP   2              // defines top padding screen header
#define HEADVOTEHEIGHT  38             // Height of head vote square
                                          // Width = Height * 1.2
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static char* GetCandidateName (
   char* pNames                        // contains candidate name and surname
   );
static char* GetCandidateSurName (
   char* pNames                        // contains candidate name and surname
   );
static void Draw_Box_Candidate (
   POSITION Position,
   struct   sReferences* Reference,
   int      pColor,                    // the displaybox's background
   DISP_PARTY_MODE PartyMode
   );
static void Update_One_Candidate (
   POSITION Position,
   DISP_PARTY_MODE PartyMode,
   BOOL Mark
   );
static int GetMarginBottom (
   DISP_PARTY_MODE PartyMode
   );
static int GetMarginTop (
   DISP_PARTY_MODE PartyMode
   );
static int GetHeadVotePos (
   DISP_PARTY_MODE PartyMode
   );
static int GetBoxHeight (
   DISP_PARTY_MODE PartyMode
   );
static int GetBoxWidth (
   DISP_PARTY_MODE PartyMode
   );
static int GetSquareWidth (
   DISP_PARTY_MODE PartyMode
   );

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
//</globals>

static enum {UNKNOWN, NEW_PARTY, CANCEL_CONFIRM} FooterDisplayed = UNKNOWN;

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

/*<function id="GetCandidateName" confidential="no">
<heading>
  <description>
    <en>Extracts name from a string containing both name and surname</en></description>
  <parameters>
    <parameter type="return"><datatype>char*</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pNames</name>
      <description><en>contains candidate name and surname</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks>buffer overrun</checks>
  <comment/>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static char* GetCandidateName (
   char* pNames                        // contains candidate name and surname
   )
// ---------------------------------------------------------------------------
{
      static CandidateNamesType Names;
      static char* cpName;

      char  cToken[] = "@";    // holds the line delimiter

   // check buffer overrun
   if ( strlen( pNames)+1 > sizeof( Names))
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "GetName buffer overun"); dcio();
      #endif // _DEBUGMODE

      return NULL;
   }

   // we need a local copy of the message text because strtok() will
   // replace the delimiters with null bytes
   strcpy( Names, pNames);

   // do we have delimiters in the message?
   cpName = strtok( Names, cToken);

   if ( cpName)    // yep!
   {
      // return the delimited string
      return cpName;
   }
   else            // nop!
   {
      // return original content
      return pNames;
   }
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="GetCandidateSurName" confidential="no">
<heading>
  <description>
    <en>Extracts name from a string containing both name and surname</en></description>
  <parameters>
    <parameter type="return"><datatype>char*</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pNames</name>
      <description><en>contains candidate name and surname</en></description>
    </parameter>
  </parameters>
  <usage/>
  <checks>buffer overrun</checks>
  <comment/>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static char* GetCandidateSurName (
   char* pNames                        // contains candidate name and surname
   )
// ---------------------------------------------------------------------------
{
      static CandidateNamesType Names;
      static char* cpName;

      char  cToken[] = "@";    // holds the line delimiter

   // check buffer overrun
   if ( strlen( pNames)+1 > sizeof( Names))
   {
      #if _DEBUGMODE
         sprintf( gcDcio, "GetSName buffer overun"); dcio();
      #endif // _DEBUGMODE

      cpName = NULL;
   }
   else
   {

      // we need a local copy of the message text because strtok() will
      // replace the delimiters with null bytes
      strcpy( Names, pNames);

      // do we have delimiters in the message?
      cpName = strtok( Names, cToken);

      if ( cpName)    // yep!
      {
         // return the second delimited string
         cpName = strtok( NULL, cToken);
      }
      else            // nop!
      {
         cpName = NULL;
      }

      #if _DEBUGMODE
         sprintf( gcDcio, "GetSName p[%s] r[%s]", pNames, cpName); dcio();
      #endif // _DEBUGMODE

   }

   return cpName;
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="Draw_Box_Candidate">
<heading>
  <description>
    <en>Draws the background and the border of a candidate displaybox.</en>
  </description>
  <parameters/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Draw_Box_Candidate (
   POSITION Position,
   int      pColor,                     // the displaybox's background
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
      int Border = 1;                  // displaybox bordersize in pixels
      int iTopCandidates;
      int iHeightBox;
      int iWidthBox;

   iHeightBox = GetBoxHeight( PartyMode);
   iWidthBox = GetBoxWidth( PartyMode);

   // fill the complete displaybox
   achtersp( Position.Y + Border, Position.X + Border,
      Position.Y + iHeightBox - Border,
      Position.X + iWidthBox - Border, pColor);

   iTopCandidates = GetMarginTop( PartyMode);

   if ( ( Position.Y == iTopCandidates) || LIGHTGRAY == pColor)
      lijnhor( Position.Y, Position.X,                       /* Top line */
         Position.X + iWidthBox, BLACK);

   lijnhor( Position.Y + iHeightBox,Position.X,    /* Bottom line */
      Position.X + iWidthBox, BLACK);

   lijnvert( Position.Y, Position.X,                         /* Left line */
      Position.Y + iHeightBox, BLACK);
   lijnvert( Position.Y, Position.X + iWidthBox,   /* Right line */
      Position.Y + iHeightBox, BLACK);

   return;
}
//</code></function>


/*<function id="Update_One_Candidate">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>POSITION</datatype><name>Position</name></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Mark</name></parameter>
  </parameters>
  <comment>
    Updates the corresponding circle.
    The VGA card has to be initialised in VGA mode (independant of font).
    When Mark is TRUE than the circle is filled RED otherwise WHITE.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Update_One_Candidate (
   POSITION Position,
   DISP_PARTY_MODE PartyMode,
   BOOL Mark
   )
// ---------------------------------------------------------------------------
{
   int iColor, iX, iY;

   // set fill color
   iColor = ( Mark ? BROWN : WHITE);

   // compute coordinates of the circles centre
   iX = Position.X + ( GetBoxWidth( PartyMode) - ( GetSquareWidth( PartyMode)/2));
   iY = Position.Y + ( GetBoxHeight( PartyMode)/2);

   // draw the circle from C
   setfillstyle( SOLID_FILL, iColor);
   fillellipse( iX, iY, 6, 6); // draws filled circle with radius of 6 pixels

   // the produced circle is rather 'square' so we draw a few additional
   // pixels to make it rounder
   putpixel( iX - 4, iY - 4, iColor); // top left
   putpixel( iX + 4, iY - 4, iColor); // top right
   putpixel( iX - 4, iY + 4, iColor); // bottom left
   putpixel( iX + 4, iY + 4, iColor); // bottom right

   return;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Calculate_Positions">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>struct ElectionMain*</datatype><name>Root</name></parameter>
    <parameter type="arg">
      <datatype>struct Select_Candidate*</datatype><name>Candidates</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ElectionId</name></parameter>
  </parameters>
  <comment>Candidates.PartyMode has to be filled before calling this routine.
Numbers of candidates/suppliants have to be filled in advance.
</comment>
</heading>
<code>*/
#pragma argsused // avn 20030226 - added for 2003
// ---------------------------------------------------------------------------
void Calculate_Positions (
   struct ElectionMain*       Root,
   struct Select_Party*       Parties,
   struct Select_Candidate*   Candidates,
   int                        ElectionId
   )
// ---------------------------------------------------------------------------
{
   struct sReferences {

      short ColumnPos[4];     // Starting coordinates for each column

      } References[ /*DISP_PARTY_MODE*/ ]

   = {
      { {1}            }, // MODE14_1
      { {1,436}        }, // MODE14_2
      { {1,219,436}    }, // MODE14_3

      { {1}            }, // MODE22_1
      { {1,436}        }, // MODE22_2
      { {1,219,436}    }, // MODE22_3
      { {1,161,321,481}}, // MODE22_4

      // MODE23 for BXL - always 18 positions for names!
      { {0}            }, // MODE23_1
      { {0,479}        }, // MODE23_2
      { {0,161,479}    }, // MODE23_3
      { {0,160,320,480}}  // MODE23_4
     };

   short PlacesPerColumn[4]={0,0,0,0}; // for current list (Maximum 4 columns)
   short Index;                        // index used to access candidates list
   short Row;                          // counters
   short Column;                       // column number of current calculation

   int   iLastColumn;
   int   iMaxCandidatesPerColumn;
   int   iTopCandidates;
   int   iNbrColumns = UNDEFINED_PARTY_MODE;
   int   iHeightBox;

   switch ( Candidates->PartyMode)
   {
      case MODE14_1:
      case MODE22_1:
      case MODE23_1: // fall thru intended!
      {
         iNbrColumns = 1;
         break;
      }
      case MODE14_2:
      case MODE22_2:
      case MODE23_2: // fall thru intended!
      {
         iNbrColumns = 2;
         break;
      }
      case MODE14_3:
      case MODE22_3:
      case MODE23_3: // fall thru intended!
      {
         iNbrColumns = 3;
         break;
      }
      case MODE22_4:
      case MODE23_4: // fall thru intended!
      {
         iNbrColumns = 4;
         break;
      }
   } // eo switch

   struct sReferences* Reference =
      ( struct sReferences*) References + Candidates->PartyMode;

   iLastColumn = iNbrColumns - 1;
   iMaxCandidatesPerColumn = Parties->ElectionMode;

   if ( 14 == iMaxCandidatesPerColumn)
   {
      if ( Candidates->NbrOfEffectives <= iMaxCandidatesPerColumn)
      {
         PlacesPerColumn[0] = Candidates->NbrOfEffectives;
         PlacesPerColumn[1] =
            ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
      }
      else if ( Candidates->NbrOfEffectives <= ( iMaxCandidatesPerColumn * 2))
      {
          float tt = (float)Candidates->NbrOfEffectives / (float)2;
          PlacesPerColumn[0] = tt + 0.5;
          PlacesPerColumn[1] = tt;
          PlacesPerColumn[2] =
            ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
      }
      else
      {
          float tt = (float)Candidates->NbrOfEffectives / (float)3;
          PlacesPerColumn[0] = tt + 0.9;
          PlacesPerColumn[1] = tt + 0.6;
          PlacesPerColumn[2] = tt + 0.3;
      }
   }
   else
   {
      if ( Candidates->NbrOfEffectives <= iMaxCandidatesPerColumn)
      {
         PlacesPerColumn[0] = Candidates->NbrOfEffectives;
         PlacesPerColumn[1] =
            ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
      }
      else if ( Candidates->NbrOfEffectives <= ( iMaxCandidatesPerColumn * 2))
      {
         float tt = (float)Candidates->NbrOfEffectives / (float)2;
         PlacesPerColumn[0] = tt + 0.5;
         PlacesPerColumn[1] = tt;
         PlacesPerColumn[2] =
            ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
      }
      else if ( Candidates->NbrOfEffectives <= ( iMaxCandidatesPerColumn * 3))
      {
         float tt = (float)Candidates->NbrOfEffectives / (float)3;
         PlacesPerColumn[0] = tt + 0.9;
         PlacesPerColumn[1] = tt + 0.6;
         PlacesPerColumn[2] = tt + 0.3;
         PlacesPerColumn[3] =
            ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
      }
      else
      {
         float tt = (float)Candidates->NbrOfEffectives / (float)4;
         if ( 22 == iMaxCandidatesPerColumn)
         {
            // no suppliants, spread evenly over all columns
            PlacesPerColumn[0] = tt + 0.75;
            PlacesPerColumn[1] = tt + 0.5;
            PlacesPerColumn[2] = tt + 0.25;
            PlacesPerColumn[3] = tt;
         }
         else
         {
            if ( Candidates->NbrOfSuppliants)
            {
               // special case in BXL effectives and suppliants end up together in last column!
               PlacesPerColumn[0] = iMaxCandidatesPerColumn;
               PlacesPerColumn[1] = iMaxCandidatesPerColumn;
               PlacesPerColumn[2] = iMaxCandidatesPerColumn;
               PlacesPerColumn[3] =
                  ( Candidates->NbrOfEffectives - ( iMaxCandidatesPerColumn * 3)) +
                  ( Candidates->NbrOfSuppliants ? Candidates->NbrOfSuppliants+1 : 0);
            }
         }
      }
   }

#if _DEBUGMODE
   sprintf( gcDcio, "PlacesPerColumn[%d][%d][%d][%d] iLastCol[%d]",
      PlacesPerColumn[0], PlacesPerColumn[1], PlacesPerColumn[2],
      PlacesPerColumn[3], iLastColumn); dcio();
#endif // _DEBUGMODE

   iTopCandidates = GetMarginTop( Candidates->PartyMode);
   iHeightBox = GetBoxHeight( Candidates->PartyMode);

   // compute positions of the candidate boxes
   Index = 0;
   for ( Column = 0; Column < iNbrColumns; Column++) // per column
   {
      for ( Row = 0;
            Row < PlacesPerColumn[Column] && Index < Candidates->NbrOfEffectives;
            Row++, Index++)
      {
         Candidates->Effective[Index].DisplayBox.X = Reference->ColumnPos[Column];
         Candidates->Effective[Index].DisplayBox.Y =
            iTopCandidates + iHeightBox * Row;
      }
      if ( Row < PlacesPerColumn[Column])
         // not enough candidates to complete the column
         break;
   }

   // compute positions of the suppliant boxes
   if ( Candidates->NbrOfSuppliants)
   {
      // always begin at the beginning of the last column except for Brussels Councel
      if ( ElectionId != CONSEIL_BXL) Row = 0;

      // suppliants data always in last column
      Column = iLastColumn;

      // suppliants headerbox
      Candidates->TitleSuppliant.X = Reference->ColumnPos[Column];
      Candidates->TitleSuppliant.Y =
         iTopCandidates + iHeightBox * Row;
      Row++;

      // compute positions of the suppliants boxes
      for ( Index = 0;
            Row < PlacesPerColumn[Column] && Index < Candidates->NbrOfSuppliants;
            Row++, Index++)
      {
         Candidates->Suppliant[Index].DisplayBox.X =
            Reference->ColumnPos[Column];
         Candidates->Suppliant[Index].DisplayBox.Y =
            iTopCandidates + iHeightBox * Row;

      }
   }

   return;
}
//</code></function>


/*<function id="Display_One_Candidate">
<heading>
  <description>
    <en>Displays all information available in a candidate displaybox.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>POSITION</datatype><name>Position</name></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>NamePart_1[]</name></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Mark</name></parameter>
  </parameters>
  <comment>
    The VGA card can be re-initialised.
    When the candidate is selected the circle of the votebox is filled RED otherwise WHITE,
    for 2004 the background of the candidate's name is turned grey also.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_One_Candidate (
   POSITION          Position,      // top left of the candidate's box in px
   int               Number,        // candidate's number
   char*             PackedName,    // name and first name packed
   DISP_PARTY_MODE   PartyMode,
   BOOL              Mark           // is candidate selected?
   )
// ---------------------------------------------------------------------------
{
      const int   CharWidth = 8;       // character width in px

      char  CenteredName[80];
      char  NamePart1[22+1] = "";
      char  NamePart2[22+1] = "";
      int   iDiffTextHor;
      int   iBoxHeight;
      int   iBoxWidth;
      int   iSquareWidth;
      char  StrNumber[3+1] = "";

   // unpack name and first name
   if ( PartyMode >= MODE22_4)
   {
      if ( GetCandidateName( PackedName))
         strncpy( NamePart1, GetCandidateName( PackedName), 18);
      if ( GetCandidateSurName( PackedName))
         strncpy( NamePart2, GetCandidateSurName( PackedName), 18);
   }
   else
   {
      if ( GetCandidateName( PackedName))
         strncpy( NamePart1, GetCandidateName( PackedName), 22);
      if ( GetCandidateSurName( PackedName))
         strncpy( NamePart2, GetCandidateSurName( PackedName), 22);
   }

   // draw name box
   if ( Mark)
   {
      // yellow produces a lightgrey on monochrome monitors
      Draw_Box_Candidate( Position, YELLOW, PartyMode);
   }
   else
   {
      Draw_Box_Candidate( Position, WHITE, PartyMode);
   }

   // get name box metrics
   iBoxHeight = GetBoxHeight( PartyMode);
   iBoxWidth = GetBoxWidth( PartyMode);
   iSquareWidth = GetSquareWidth( PartyMode);   // former vote box size
   if ( PartyMode >= MODE22_1)
   {
      iDiffTextHor = -3;
   }
   else
   {
      iDiffTextHor = -8;
   }

   // draw candidate's number, centered in a number box
   sprintf( StrNumber, "%d", Number);
   if ( PartyMode >= MODE22_4)
   {
      Set_Mode( VGA_MODE, CARACT8x8);
      if ( Number < 10)
      {
         zegsp( Position.Y + (( iBoxHeight - 8) / 2) + 1,
            Position.X + 1 + 5,
            StrNumber, BLACK, strlen( StrNumber), 1);
      }
      else
      {
         zegsp( Position.Y + (( iBoxHeight - 8) / 2) + 1,
            Position.X + 1,
            StrNumber, BLACK, strlen( StrNumber), 1);
      }
      lijnvert( Position.Y, Position.X + 16,
         Position.Y + iBoxHeight, BLACK);
   }
   else
   {
      // character mode
      Set_Mode( VGA_MODE, CARACT16x8);
      if ( Number < 10)
      {
         zegsp( Position.Y + (( iBoxHeight - 16) / 2) + 1,
            Position.X + 2 + 5,
            StrNumber, BLACK, strlen( StrNumber), 1);
      }
      else
      {
         // vertically centered in the name box
         zegsp( Position.Y + (( iBoxHeight - 16) / 2) + 1,
            Position.X + 2,
            StrNumber, BLACK, strlen( StrNumber), 1);
      }
      lijnvert( Position.Y, Position.X + 2 + 16 + 2,
         Position.Y + iBoxHeight, BLACK);
   }

   // draw name
   if ( PartyMode >= MODE22_1)
   {
      Set_Mode( VGA_MODE, CARACT8x8);
   }
   else
   {
      Set_Mode( VGA_MODE, CARACT14x8);
   }

    if( NamePart2==NULL || !*NamePart2)
    {                                   /* Part1:Centered and Part2="" */
        Centre( NamePart1,
                (iBoxWidth - iSquareWidth) / CharWidth,
                CenteredName);
        zegsp( Position.Y + iDiffTextHor + iBoxHeight/2,
               Position.X + 2 + iSquareWidth,
               CenteredName, 0, strlen(CenteredName), 1);
    }
    // rvh/wbn 22/09/94
    // Separate handling for MODE14 or MODE22
    // Y-positon starts 1 lower in MODE14 because of ?...caracter size in 14x8
    // Candidate-box had also to be one pixel higher for same reason
    // This results in smaller blanc line above first row of candidates
    // This also results in smaller headvote zone in MODE14, now equal to MODE22

    else if(( PartyMode == MODE14_1) ||
            ( PartyMode == MODE14_2) ||
            ( PartyMode == MODE14_3))
    {                                    /* Part1:Left and Part2:Centered */
        zegsp( Position.Y + iDiffTextHor + iBoxHeight/4.0+2,
               Position.X + 2 + iSquareWidth,
               NamePart1, 0, strlen(NamePart1), 1);
        Centre( NamePart2,
                (iBoxWidth - iSquareWidth) / CharWidth,
                CenteredName);
        zegsp( Position.Y + iDiffTextHor + iBoxHeight*3/4.0+1,
               Position.X + 2 + iSquareWidth,
               CenteredName, 0, strlen(CenteredName), 1);
    }
    else
    {                                    /* Part1:Left and Part2:Centered */
        zegsp( Position.Y + iDiffTextHor + iBoxHeight/4.0+1,
               Position.X + 2 + iSquareWidth,
               NamePart1, 0, strlen(NamePart1), 1);
        Centre( NamePart2,
                (iBoxWidth - iSquareWidth) / CharWidth,
                CenteredName);
        zegsp( Position.Y + iDiffTextHor + iBoxHeight*3/4.0,
               Position.X + 2 + iSquareWidth,
               CenteredName, 0, strlen(CenteredName), 1);
    }

   return;
}
//</code></function>




/*<function id="Is_Candidate_Selected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>POSITION</datatype><name>Position</name></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
  <comment>Returns TRUE when the indicated candidate is indeed selected.
    FALSE when the pen coordinates are not in the selectable area.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Is_Candidate_Selected (
   POSITION          Position,      // top left of the name box
   DISP_PARTY_MODE   PartyMode,
   short             PenX,
   short             PenY
   )
// ---------------------------------------------------------------------------
{
   // for 2007 the hotspot is the name box ( without borders)
   if ( PenX > Position.X + 1                            &&
        PenX < Position.X + GetBoxWidth( PartyMode) - 1  &&
        PenY > Position.Y + 1                            &&
        PenY < Position.Y + GetBoxHeight( PartyMode) - 1
      )
   {
      return TRUE;   // in
   }

   return FALSE;     // out
}
#endif
//</code></function>


/*<function id="Display_Title_Suppliant">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>POSITION</datatype><name>Position</name></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
  </parameters>
  <comment>
    GLOBALS: Working_Language( ASK)
    The VGA card can be re-initialised.
    Displays one titlebox on the given position and in the given mode.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Title_Suppliant (
   POSITION Position,
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
    char CenteredName[80];

    Draw_Box_Candidate( Position, LIGHTGRAY, PartyMode);
    Set_Mode( VGA_MODE, 14);              // Always in 14x8 whatever the mode

    Centre( Get_Text( SUPPLIANT, Working_Language( ASK)),
            GetBoxWidth( PartyMode) / 8,
            CenteredName);
    zegsp( Position.Y -8 + GetBoxHeight( PartyMode)/2,
           Position.X -0, CenteredName, 0, strlen(CenteredName), 1);
}
//</code></function>


//////////////////////////FOOTERS////////////////////////////////////

/*<function id="Display_Footer_New_Party">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
  </parameters>
  <comment>
    GLOBALS: Working_Language( ASK)
    The VGA card can be re-initialised.
    Displays the corresponding footer.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Footer_New_Party (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
      char  CenteredLabel[80];
      int   iBottomCandidates;

   iBottomCandidates = GetMarginBottom( PartyMode);

   if ( PartyMode < MODE23_1)
   {
      lijnhor( iBottomCandidates, 0, getmaxx(), BLACK);
   }

   // clear footer's button area
   achtersp( iBottomCandidates + 16, 0, getmaxy(), getmaxx(), WHITE);

   Draw_Button( 106, iBottomCandidates + 16,
      428, getmaxy() - iBottomCandidates - 17, 3);

   Set_Mode( VGA_MODE, 14);
   Centre( Get_Text( B_GO_BACK, Working_Language( ASK)), 428/8, CenteredLabel);
   zegsp( iBottomCandidates + 16
        + ( getmaxy() - iBottomCandidates - 17) / 2 - 8,
        106+4, CenteredLabel, 0, strlen( CenteredLabel), 1);
   FooterDisplayed = NEW_PARTY;
}
//</code></function>


/*<function id="Is_New_Party_Selected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
  <comment>
    Returns TRUE when the button "new party" is selected.
            FALSE when the function "Display_Footer_NewParty" not is executed
                  when the button "new party" not is selected.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Is_New_Party_Selected (
   DISP_PARTY_MODE PartyMode,
   short PenX,
   short PenY
   )
// ---------------------------------------------------------------------------
{

    return( (FooterDisplayed==NEW_PARTY) &&
            !(PenX < 106 || PenX > 106 + 428 ||
              PenY < GetMarginBottom( PartyMode) + 16));
}
#endif
//</code></function>


/*<function id="Display_Footer_Cancel_Confirm">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
  </parameters>
  <comment>
    GLOBALS: Working_Language( ASK)
    The VGA card can be re-initialised.
    Displays the corresponding footer.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Display_Footer_Cancel_Confirm (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
      char  CenteredLabel[80];
      int   iBottomCandidates;

   iBottomCandidates = GetMarginBottom( PartyMode);

   if ( FooterDisplayed != CANCEL_CONFIRM)
   {
      // 20031229 avn - draw button after first selection only otherwise you get
      // some ugly flickering in this area everytime a candidate gets selected...
      Set_Mode( VGA_MODE,14);

      // clear footer's button area
      achtersp( iBottomCandidates + 16, 0,
         getmaxy(), getmaxx(), WHITE);

      Draw_Button( 10,iBottomCandidates + 16,
                 304, getmaxy() - iBottomCandidates - 17, 3);
      Centre( Get_Text( B_CANCEL,Working_Language( ASK)), 304/8, CenteredLabel);
      zegsp( iBottomCandidates + 16
           + ( getmaxy() - iBottomCandidates - 17) / 2 - 8,
           10+4, CenteredLabel, 0, strlen(CenteredLabel), 1);

      Draw_Button( 326,iBottomCandidates + 16,
                 304, getmaxy() - iBottomCandidates - 17, 3);
      Centre( Get_Text( B_CONFIRM,Working_Language( ASK)), 304/8, CenteredLabel);
      zegsp( iBottomCandidates + 16
           + ( getmaxy() - iBottomCandidates - 17) / 2 - 8,
           326+4, CenteredLabel,0, strlen(CenteredLabel), 1);

      FooterDisplayed = CANCEL_CONFIRM;
   }
}
#endif
//</code></function>


/*<function id="Is_Cancel_Selected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
  <comment>
    Returns TRUE when the button "Cancel" is selected.
       FALSE when the function "Display_Footer_Cancel_Confirm" not is executed
       when the button "Cancel" not is selected.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Is_Cancel_Selected (
   DISP_PARTY_MODE PartyMode,
   short PenX,
   short PenY
   )
// ---------------------------------------------------------------------------
{

    return( (FooterDisplayed==CANCEL_CONFIRM) &&
            !(PenX < 10 || PenX > 10 + 304 ||
              PenY < GetMarginBottom( PartyMode) + 16));
}
#endif
//</code></function>

/*<function id="Is_Confirm_Selected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
  <comment>
    Returns TRUE when the button "Confirm" is selected.
       FALSE when the function "Display_Footer_Cancel_Confirm" not is executed
             when the button "Confirm" not is selected.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Is_Confirm_Selected (
   DISP_PARTY_MODE PartyMode,
   short PenX,
   short PenY
   )
// ---------------------------------------------------------------------------
{

    return( (FooterDisplayed==CANCEL_CONFIRM) &&
            !(PenX < 326 || PenX > 326 + 304 ||
              PenY < GetMarginBottom( PartyMode) + 16));
}
#endif
//</code></function>

//////////////////////////HEADERS////////////////////////////////////

/*<function id="Display_Header">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>PartyInd</name></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>PartyName[]</name></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>ElectionName[]</name></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Mark</name></parameter>
  </parameters>
  <comment>
  Displays the header and the circle for the party vote on the candidates screen.
  - The VGA card can be re-initialised.
  - When Mark is TRUE than the circle is filled RED otherwise WHITE.
  - The strings have to be given in the used language.
  </comment>
</heading>
<code>*/
#pragma argsused // avn 20030226 - no logo's in demo
// ---------------------------------------------------------------------------
void Display_Header (
   int ElectionInd,
   int CollegeInd,
   int PartyInd,
   char PartyName[],
   logoType PartyLogo,
   char ElectionName[],
   DISP_PARTY_MODE PartyMode,
   BOOL Mark)
// ---------------------------------------------------------------------------
{
      char           cLabel[80];
      int            iCol;
      positionType   Position;
      int            iPosHeadVote;

   // clear header
   achtersp( 0, 0, HEADVOTEHEIGHT + HDRPADDINGTOP, getmaxx(), WHITE);
   lijnhor( HEADVOTEHEIGHT + HDRPADDINGTOP + 2, 0, getmaxx(), BLACK);

   iPosHeadVote = GetHeadVotePos( PartyMode);

   // voting box
   achtersp( HDRPADDINGTOP,
      iPosHeadVote - (short)((HEADVOTEHEIGHT)*0.6),
      HDRPADDINGTOP + HEADVOTEHEIGHT,
      iPosHeadVote + (short)((HEADVOTEHEIGHT)*0.6),
      BLACK);
   Update_Head_Vote( PartyMode, Mark);

   setcolor( BLACK);
   setusercharsize( 5,8,1,1);

   // party number
   sprintf( cLabel,"%2d",PartyInd);
   settextjustify( CENTER_TEXT, TOP_TEXT);
   iCol =
      ( iPosHeadVote - ( HEADVOTEHEIGHT * 1.2 / 2)) / 2;
   outtextxy( iCol, 2, cLabel);

#ifndef _DEMO
   // in production version we display the logo instead of the name when
   // it's available
   if ( PartyLogo.Width && PartyLogo.Height)
   {
      // display party logo
      Position.Row = HDRPADDINGTOP +
         ( ( HEADVOTEHEIGHT - PartyLogo.Height) / 2);
      Position.Col =
         ( iPosHeadVote + HEADVOTEHEIGHT / 2) + iCol - 10;
      displayLogo( Position, PartyLogo,
         ElectionInd, CollegeInd, PartyInd);
   }
   else
#endif
   {
      // no logo's allowed in the demo, just display party's name
      sprintf(cLabel,"%s",PartyName);
      settextjustify( LEFT_TEXT, TOP_TEXT);
      iCol =
         ( iPosHeadVote + HEADVOTEHEIGHT / 2) + iCol - 20;
      if ( strlen( PartyName) > 15)
      {
         // very long partynames display smaller and closer to the box
         // otherwise they overlap with long election names...
         setusercharsize( 1, 2, 1, 1);
         iCol =
            iPosHeadVote + ( HEADVOTEHEIGHT * 1.2 / 2) + 15;
      }
      outtextxy( iCol, 2, cLabel);
   }

   DisplayElectionNameSmall( ElectionName);

   return;
}
//</code></function>


/*<function id="Update_Head_Vote">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Mark</name></parameter>
  </parameters>
  <comment>
    Updates the corresponding circle.
    The VGA card has to be initialised in VGA mode (independant of font).
    When Mark is TRUE than the circle is filled RED otherwise WHITE.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Update_Head_Vote (
   DISP_PARTY_MODE PartyMode,
   BOOL Mark
   )
// ---------------------------------------------------------------------------
{

   setfillstyle( SOLID_FILL, ( Mark ? BROWN : WHITE));
   fillellipse(
      GetHeadVotePos( PartyMode),
      HDRPADDINGTOP + ( HEADVOTEHEIGHT / 2),
      7, 7); // draws filled circle with radius of 6 pixels

   return;
}
//</code></function>


/*<function id="Is_Head_Vote_Selected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
  <comment>
    Returns TRUE when the party circle is selected.
    FALSE when then pen coordinates are not in the selectable area.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Is_Head_Vote_Selected (
   DISP_PARTY_MODE PartyMode,
   short PenX,
   short PenY
   )
// ---------------------------------------------------------------------------
{

    PenX -= GetHeadVotePos( PartyMode) - (short)(HEADVOTEHEIGHT*0.6);

    return( !(PenX < 0 || PenX > (HEADVOTEHEIGHT) ||
            PenY < 2 || PenY > HEADVOTEHEIGHT+2));
}
#endif
//</code></function>


/*<function id="DisplayButtonNextElection">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
  </parameters>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void DisplayButtonNextElection (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
      const int iMarginTop = 1;
      const int iMarginBottom = 16;

      char CenteredLabel[80];
      int iButtonHeight;
      int iBottomCandidates;

   iBottomCandidates = GetMarginBottom( PartyMode);

   iButtonHeight = getmaxy() - iBottomCandidates -
      iMarginTop - iMarginBottom;

   Draw_Button( 106, iBottomCandidates + iMarginTop,
     428, iButtonHeight, 3);
   Set_Mode( VGA_MODE, 14);
   Centre( Get_Text( B_NEXT, Working_Language( ASK)), 428/8, CenteredLabel);
   zegsp( iBottomCandidates + iMarginTop
        + (iButtonHeight) / 2 - 8,
        106+4, CenteredLabel, BLACK, strlen( CenteredLabel), 1);

    return;
}
#endif
//</code></function>


/*<function id="IsNextElectionSelected">
<heading>
  <description/>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype></parameter>
    <parameter type="arg">
      <datatype>DISP_PARTY_MODE</datatype><name>PartyMode</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenX</name></parameter>
    <parameter type="arg">
      <datatype>short</datatype><name>PenY</name></parameter>
  </parameters>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL IsNextElectionSelected (
   DISP_PARTY_MODE PartyMode,
   short PenX,
   short PenY
   )
// ---------------------------------------------------------------------------
{
      const int iMarginTop = 1;
      const int iMarginBottom = 16;

      int iButtonHeight;
      int iBottomCandidates;

   iBottomCandidates = GetMarginBottom( PartyMode);

   iButtonHeight = getmaxy() - iBottomCandidates
      - iMarginTop - iMarginBottom;

    return ( 106 < PenX && PenX < ( 106 + 428)
             && ( iBottomCandidates + iMarginTop) < PenY
             && PenY < ( iBottomCandidates + iMarginTop + iButtonHeight));
}
#endif
//</code></function>

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="GetBoxWidth" confidential="no">
<heading>
  <description>
    <en>Returns width of the name box in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetBoxWidth (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   switch ( PartyMode)
   {
      case MODE14_1:
      case MODE14_2:
      case MODE22_1:
      case MODE22_2: // fall thru intended!
      {
         return 201;
      }
      case MODE14_3:
      case MODE22_3: // fall thru intended!
      {
         return 201;
      }
   } // eo switch

   return 160;
}
//</code></function>

/*<function id="GetBoxHeight" confidential="no">
<heading>
  <description>
    <en>Returns height of the name box in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetBoxHeight (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   if ( PartyMode >= MODE22_1)
   {
      return 18;
   }
   else
   {
      return 28;
   }
}
//</code></function>

/*<function id="GetHeadVotePos" confidential="no">
<heading>
  <description>
    <en>Returns position (middle) of head vote square in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetHeadVotePos (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   if ( PartyMode >= MODE22_4)
   {
      return 80;
   }
   else
   {
      return 106;
   }
}
//</code></function>

/*<function id="GetMarginTop" confidential="no">
<heading>
  <description>
    <en>Returns top margin in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetMarginTop (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   if ( PartyMode >= MODE23_1)
   {
      return 42;
   }
   else
   {
      return 44;
   }
}
//</code></function>

/*<function id="GetMarginBottom" confidential="no">
<heading>
  <description>
    <en>Returns bottom margin in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetMarginBottom (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   if ( PartyMode >= MODE22_1)
   {
      return 442;              // was 479 - 37 in the old references table
   }
   else
   {
      return 438;              // was 479 - 41 in the old references table
   }
}
//</code></function>

/*<function id="GetSquareWidth" confidential="no">
<heading>
  <description>
    <en>Returns width of the black square at the right (in the box) in pixels</en></description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Function replaces part of the old references table</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int GetSquareWidth (
   DISP_PARTY_MODE PartyMode
   )
// ---------------------------------------------------------------------------
{
   switch ( PartyMode)
   {
      case MODE14_1:
      case MODE14_2:
      case MODE22_1:
      case MODE22_2: // fall thru intended!
      {
         return 21;
      }
      case MODE14_3:
      case MODE22_3: // fall thru intended!
      {
         return 21;
      }
   } // eo switch

   return 16;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->