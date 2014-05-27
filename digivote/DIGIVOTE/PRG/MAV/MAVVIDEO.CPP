/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavvideo.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVVIDEO.CPP  $</workFile>
<revision>$Revision: 1.5 $</revision>
<workFileTimeStamp>$Modtime:   Jan 26 2005 16:14:04  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVVIDEO.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/21 12:38:50 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVVIDEO.CPP</fileName>
  <creationDate>01/01/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Librairie des fonctions d'affichage</fr>
    <nl>Functielibrary schermbeheer</nl></description>
  <history>
   01/06/93: revision history starting date
   21/09/93: the function were adapted for the new data structure, support
             for long election names was added, support for College
             selection was added.
   21/12/93: selecting a candidate twice is no longer an error.
   22/03/94: RVH Function : User_Confirms_Language added.
   23/03/94: RVH Header "Titularis" has been omitted.
             Function: Input_Candidates has been changed.
             Function: Display_Candidates has been changed.
   24/05/94: WBN Title "Suppleants" is sometimes displayed at the bottom of
             a column while it should be at the top of the next column.
             Function: Input_Candidates has been changed.
             Function: Display_Candidates has been changed.
  </history>
  <comment>Display routines</comment>
</comments>
<header>*/

//======================== I N C L U D E   F I L E S ==========================
//<includes>
#include <process.h>                   // exit
#include <string.h>                    // strlen
#include <dos.h>                       // sleep

#include "..\gen\gentype.h"
#include "..\gen\genstr.h"

#include "..\mav\mavasm.h"
#include "..\mav\mavvideo.h"
#include "..\mav\mavmes.h"
#include "..\mav\mavlight.h"
#include "..\mav\logo.h"
//</includes>

#define _DEBUGMODE      FALSE   // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE


//=========================== C O N S T A N T S ===============================
#define MAXRES 639               // max. screen width in vga mode
#define MAXCOL 3                 // max. # of columns for party list
#define HEADER 16                // height of the header area ( in pixels)
#define BACKBUTTON 20            // height of the back button ( in pixels)
#define FOOTER 16                // height of the footer area ( in pixels)
#define LEFTMARGIN 4             // width of margin between edge and parties

//const int diff=79;
const int MAX_CHARS = 75;  // maximum number of characters in the select party
                           // window title bar

//=================== E X T E R N A L   V A R I A B L E S =====================

extern struct ElectionMain Root;
extern struct Select_College Colleges;
extern struct Select_Party Parties;
extern struct Select_Candidate Candidates;
extern int Allready_Voted;

//====================== L O C A L   F U N C T I O N S ========================
//<prototypes>
static void DrawBox (
   int iRow, int iCol, int iHeight, int iWidth, int iColor);
static int Query_Video_Mode();
static void displayHeader ( int line, char* text, ALIGN align);
static void displayFooter ( int line, char* text, ALIGN align);
static void justifyText ( int line, char* text, ALIGN align);

#ifdef _APPLICATION_MAV
   static void DisplayPartyName (
      char* cpPartyName,                  // party label
      int   iRow,                         // row position in pixels
      int   iCol,                         // col position in pixels
      int   iHeight                       // textheight in pixels
      );
#endif // _APPLICATION_MAV
//</prototypes>


// Display type
enum { ONE_SCREEN, SPLITED_SCREEN, DUAL_SCREEN } WayOfDisplay;
// election type
enum { MONO_LINGUAL, PLURI_LINGUAL } LingualElectionType ;
//structure that contain infos for the display of the party
struct PartyScreen {
  int lang1,lang2;         // main and second language use in case of multi langual election
  int LanguageGroup;       // main language
  int nbrparty1,nbrparty2; // number of party in each lingual group
  int nbrrow1,nbrrow2;     // number of rows for each lingual group
  int x1,y1,x2,y2;         // left and top margins of the party box area
  int vgap,hgap,width,height,height2,vgap2; // position and size of the party display box
  int nbrline,nbrline2;    // number of lines free for party displaying
   } PartyScreen;


//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="DrawBox">
<heading>
  <description>
    <en>Draws an empty box with a border.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
  </usage>
  <checks>
  </checks>
  <comment>Provides a more natural way to draw a box than straight thru
    the assembler function kad(). Where the kad() function requires
    the top-left and bottom-right coordinates, this function instead uses
    the top-left coordinate and the width and height of the box.

    You should agree that DrawBox( row, col, height, width, color) is easier
    to understand than kad( row, col, row + height, col + width, color).
  </comment>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void DrawBox (
   int iRow, int iCol, int iHeight, int iWidth, int iColor)
// ---------------------------------------------------------------------------
{
   kad( iRow, iCol, iRow + iHeight, iCol + iWidth, iColor);

   return;
}
//</code></function>


/*<function id="Query_Video_Mode">
<heading>
  <description><en>
    Select video mode and number of horizontal lines depends on number of
    partys.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static int Query_Video_Mode ()
// ---------------------------------------------------------------------------
{

   // no more mixed video modes, always use vga ( implied by logo's)
   if ( Parties.NbrOfParties < 30)
   {
      Set_Mode( VGA_MODE, CARACT16x8);
   }
   else
   {
      Set_Mode( VGA_MODE, CARACT14x8);
   }

   return 479;
}
//</code></function>


/*<function id="displayHeader">
<heading>
  <description>
    <en>draws a header area with a text in it.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>line</name>
      <description><en>the line where to draw the info box</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>text</name>
      <description><en>the text to display in the box</en></description></parameter>
    <parameter type="arg">
      <datatype>ALIGN</datatype><name>align</name>
      <description><en>where to align the text</en></description></parameter>
  </parameters>
  <usage>
    displayHeader( 0, Get_Text( H_CHOOSE_LANGUAGE, Root.Language [0]));</usage>
  <comment>
    This function is used to display informations a the top of the screen.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void displayHeader ( int line, char* text, ALIGN align)
// ---------------------------------------------------------------------------
{

   // draws the colored background
   achtersp( line, 0, line + HEADER, MAXRES, WHITE);

   // draws a line under the box
   lijnhor( line + HEADER, 0, MAXRES, BLACK);

   // draw text
   justifyText( line, text, align);

   return;
}
//</code></function>


/*<function id="displayFooter">
<heading>
  <description>
    <en>draws a footer area with a text in it.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>line</name>
      <description><en>the line where to draw the area</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>text</name>
      <description><en>the text to display in the area</en></description></parameter>
    <parameter type="arg">
      <datatype>ALIGN</datatype><name>align</name>
      <description><en>how to align the text</en></description></parameter>
  </parameters>
  <usage>
    displayFooter( MAXRES - FOOTER, Get_Text( H_CHOOSE_LANGUAGE, Root.Language [0]));</usage>
  <comment>
    This function is used to display informations a the bottom of the screen.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void displayFooter ( int line, char* text, ALIGN align)
// ---------------------------------------------------------------------------
{

   // draws the colored background
   achtersp( line, 0, line + FOOTER, MAXRES, WHITE);

   // draws a line above the box
   lijnhor( line, 0, MAXRES, BLACK);

   // draw text
   justifyText( line, text, align);

   return;
}
//</code></function>


/*<function id="justifyText">
<heading>
  <description>
    <en>Align a text in an area.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>line</name>
      <description><en>the line where to draw the text</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>text</name>
      <description><en>the text to display</en></description></parameter>
    <parameter type="arg">
      <datatype>ALIGN</datatype><name>align</name>
      <description><en>how to align the text</en></description></parameter>
  </parameters>
  <usage>
    Align( line, Get_Text( H_CHOOSE_LANGUAGE, Root.Language [0]), LEFT);</usage>
  <comment>
    This function is used to display informations on screen.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void justifyText ( int line, char* text, ALIGN align)
// ---------------------------------------------------------------------------
{

   // draw text
   if ( text != '\0')
   {
      switch( align)
      {
         case LEFT  :
            zegsp( line+1, 11, text, 0, strlen( text), 1);
            break;

         case CENTER:
            zegsp( line+1, MAXRES/2 - (strlen( text)/2) * 8,
                     text, 0, strlen( text), 1);
            break;

         case RIGHT :
         default    :
            zegsp( line+1, MAXRES - strlen( text) * 8,
                     text, 0, strlen( text), 1);
      }  // eo switch
   }

   return;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Info_Box">
<heading>
  <description><en>
    draws rectangular area in a different color and with a text in it.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>line</name>
      <description><en>the line where to draw the info box</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>text</name>
      <description><en>the text to display in the box</en></description></parameter>
    <parameter type="arg">
      <datatype>ALIGN</datatype><name>align</name>
      <description><en>where to align the text</en></description></parameter>
  </parameters>
  <usage>
    Info_Box( 7, Get_Text( H_CHOOSE_LANGUAGE, Root.Language [0]));</usage>
  <comment>
    This function is used to display informations a the top and at
    the bottom of the screen when the user is asked to choose a
    language for example.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Info_Box ( int line, char* text, ALIGN align)
// ---------------------------------------------------------------------------
{

   // draws the colored box
   achtersp( line+1, 0, line+16, MAXRES, WHITE);

   // draws a line under the box
   lijnhor( line+17, 0, MAXRES, BLACK);

   // draws a line above the box
   lijnhor( line, 0, MAXRES, BLACK);

   // draw text
   justifyText( line, text, align);

   return;
}
#ifndef _PREVIEW
#endif
//</code></function>


/*<function id="Make_Ok_Button">
<heading>
  <description>
    <en>draws an OK button with 3D shadow effect</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>text</name>
      <description><en>the text to display in the button</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mid_x</name>
      <description><en>the x coordinate of the button's center</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mid_y</name>
      <description><en>the y coordinate of the button's center</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>half_width</name>
      <description><en>the button's half width</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>half_height</name>
      <description><en>the button's half height</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>border</name>
      <description><en>the border width</en></description></parameter>
  </parameters>
  <usage>
    Make_Ok_Button( "OK", min_x, min_y,half_width,half_height,BORDER);</usage>
</heading>
<code>*/
#ifndef _PREVIEW // optimization
// ---------------------------------------------------------------------------
void Make_Ok_Button (
   char * text, int mid_x, int mid_y,
   int half_width, int half_height, int border)
// ---------------------------------------------------------------------------
{
   // draw the button
   setfillstyle( 1, 12);
   bar( mid_x - half_width , mid_y - half_height,
        mid_x + half_width , mid_y + half_height);

   setfillstyle( 1,15/*7*/);
   bar( mid_x - half_width         , mid_y - half_height,
        mid_x + half_width - border, mid_y + half_height - border);

   rectangle( mid_x - half_width, mid_y - half_height,
              mid_x + half_width, mid_y + half_height);

   // set the text caracteristics
   settextjustify( CENTER_TEXT, CENTER_TEXT);
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
   setusercharsize( 8, 10, 8, 10);

   // display the text
   outtextxy( mid_x, mid_y - 6, text);

   return;
}
#endif
//</code></function>


/*<function id="Erase_Ok_Button">
<heading>
  <description>
    <en>'erase' the confirmation button by giving it the MAV background color.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mid_x</name>
      <description><en>the x coordinate of the button's center</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mid_y</name>
      <description><en>the y coordinate of the button's center</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>half_width</name>
      <description><en>the button's half width</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>half_height</name>
      <description><en>the button's half height</en></description></parameter>
  </parameters>
  <usage>
    Erase_Ok_Button( min_x, min_y, HALF_WIDTH, HALF_HEIGHT);</usage>
  <comment>
    The background is supposed to be white, because we fill the
    button area with that color</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Erase_Ok_Button (
   int mid_x, int mid_y, int half_width, int half_height)
// ---------------------------------------------------------------------------
{
   setfillstyle( SOLID_FILL, WHITE);

   bar( mid_x - half_width , mid_y - half_height,
        mid_x + half_width , mid_y + half_height);

   return;
}
#endif
//</code></function>


/*<function id="Centre">
<heading>
  <description>
    <en>centers the given string in a larger string completed with ' '</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>in_text</name>
      <description><en>the text to center</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>out_len</name>
      <description><en>the lenght of the ouput text</en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>out_text</name>
      <description><en>the output text with the input text centered</en></description></parameter>
  </parameters>
  <usage/>
  <comment>
    If the input string is longer than the output string, than the
    input string is truncated to the output string length.
    Otherwise, the input string is copied in the middle of the
    output string.
</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Centre ( char * in_text, int out_len, char * out_text)
// ---------------------------------------------------------------------------
{
   int in_len = strlen( in_text);

   memset( out_text, ' ', out_len);

   if( in_len > out_len)
      memcpy( out_text, in_text, out_len);
   else
      memcpy( out_text + ((out_len - in_len) / 2), in_text, in_len);

   out_text [out_len] = 0;
}
//</code></function>


/*<function id="Set_Mode">
<heading>
  <description>
    <en>sets the graphic mode and the display font size</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mode</name>
      <description><en>
        the mode number CGA_MODE    14
                        EGA_MODE    16
                        VGA_MODE    18</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>char_size</name>
      <description><en>
        the font size   CARACT16x8   16
                        CARACT14x8   14
                        CARACT8x8    8</en>
      </description>
    </parameter>
  </parameters>
  <usage>
    Set_Mode( EGA_MODE, CARACT16x8);</usage>
  <comment>
    This function will no longer be usefull if everything is in VGA,
    it is only there to hide the call to the asm routine.
</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Set_Mode ( int mode, int char_size)
// ---------------------------------------------------------------------------
{
   videom( mode, char_size);
}
//</code></function>


/*<function id="Screen_White">
<heading>
  <description><en>
    clears the screen by making it completely white</en></description>

  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Screen_White();</usage>
  <comment>
    This function is only there to hide the call to the asm routine.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Screen_White ()
// ---------------------------------------------------------------------------
{
   bladvul( 99);
}
//</code></function>


/*<function id="Display_Election">
<heading>
  <description>
    <en>displays the election name in the center of the screen and pause for a
    few seconds</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>cpElectionName</name>
      <description>
        <en>the election name to display</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iElectionNbrOfColleges</name>
      <description>
        <en># of colleges parsed for this election. Watch out : there's always 1
        college record for an election... containing the label 'no college' :-((.</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iElectionHasLanguageGroups</name>
      <description>
        <en>Indicates wether languagegroups have been parsed for this election.</en>
      </description></parameter>
  </parameters>
  <usage/>
  <checks>We check the number of lines to display, if that number is
    greater than the planed maximum, we set it to the maximum value</checks>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Display_Election (
   char* cpElectionName,
   int   iElectionNbrOfColleges,
   int   iElectionHasLanguageGroups
   )
// ---------------------------------------------------------------------------
{
      int iCol;
      int iRow;
      int iTextHeight;

   // 20040429 avn - no election screen this time...
   if ( !Allready_Voted) return;

   Screen_White();

   if ( Allready_Voted)
   {
      Display_Message( DUMMY);
   }
   else
   {
      Display_Message( M_CONFIRM_VOTE);

      iCol = getmaxx()/2;              // centered
      iRow = (int)getmaxy()*.66;       // about 2/3 of screen height
      iTextHeight = textheight( "H");

      if ( 2 == iElectionNbrOfColleges)
      {
         // college choice coming up
         Display_Strings( M_CONFIRM_COLLEGE, Working_Language( ASK),
            iCol, iRow, iTextHeight);
      }
      else if ( iElectionHasLanguageGroups)
      {
         // language group choice coming up
         Display_Strings( M_CONFIRM_GROUP, Working_Language( ASK),
            iCol, iRow, iTextHeight);
      }
      else
      {
         // parties coming up
         Display_Strings( M_CONFIRM_LISTS, Working_Language( ASK),
            iCol, iRow, iTextHeight);
      }
   }

   DisplayElectionName( cpElectionName);

   //-> We need minimum 2 seconds to rebuild election
   //For the older systems of DV I, these 2 seconds are NOT needed,
   //but for the moment there is no check whether it is DV I or II

   //2 seconds is not enough => 5 seconds instead !
   #ifndef _PREVIEW
      if ( !Allready_Voted) sleep( 5);
   #endif

   return;
}
#endif
//</code></function>


/*<function id="Select_College">
<heading>
  <description>
    <en>draws and processes the screen to select a college</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>
        the index number of the selected college</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_College</datatype><name>Colleges</name>
      <description><en>the college list</en></description></parameter>
  </parameters>
  <usage/>
  <checks>
    The maximum number of colleges was checked when reading the
    data, so we don't check it once again.</checks>
  <comment>
    If there is only one college, the function immediately returns
    the index number of that college.
    The Coord_Y array holds the y coordinates for a maximum of
    MAX_COLLEGE selection areas.
    The Coord_N array holds the Coord_Y coordinates to use
    according to the number of colleges. The first array dimension
    corresponds to the number of colleges - 1, and the second dim
    to the corresponding Coord_Y coordinate to use for each college.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int Select_College (
   void
   )
// ---------------------------------------------------------------------------
{

   int i;
   char* cpCollegeName[2];
   int   iCollegeInd[2];

   if ( Colleges.NbrOfColleges == 1)
      return Colleges.Choice [0].Ind;      // there is only 1 college

   // select name and id
   for ( i = 0; i < Colleges.NbrOfColleges; i++)
   {
      if ( 0 <= i && i < 2)
      {
         cpCollegeName[i] = Colleges.Choice [i].Name;
         iCollegeInd[i] = Colleges.Choice [i].Ind;
      }
   }

   if (  DisplayTwoButtonChoice(
            Colleges.ElectionName,
            Get_Text( H_CHOOSE_COLLEGE, Working_Language( ASK )),
            cpCollegeName [0], cpCollegeName [1],
            LEFT) == 1
      )
   {
      return iCollegeInd [0];
   }
   else
   {
      return iCollegeInd [1];
   }
}
#endif
//</code></function>


/*<function id="Draw_Button">
<heading>
  <description>
    <en>draws a colored button with a shadow effect border</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x</name>
      <description><en>the left coordinate of the button</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y</name>
      <description><en>the top coordinate of the button</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>b_width</name>
      <description><en>the button width</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>b_height</name>
      <description><en>the button height</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>b_border</name>
      <description><en>the shadow border thickness</en></description></parameter>
  </parameters>
  <usage>
    Draw_Button( 75, 270, Button_Width, Button_Height, Button_Border);</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Draw_Button ( int x, int y, int b_width, int b_height, int b_border)
// ---------------------------------------------------------------------------
{
    // shadow of button
    setfillstyle( SOLID_FILL, LIGHTRED);
    bar( x, y, x + b_width, y + b_height);

    // background of button
    setfillstyle( SOLID_FILL, WHITE);
    bar( x, y, x + b_width - b_border, y + b_height - b_border);

    // button's border
    rectangle( x, y, x + b_width, y + b_height);

    return;
}
//</code></function>

/*<function id="Decide_To_Review_Vote">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 when left/cancel button is pressed OR 1 when right/accept button is pressed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int Decide_To_Review_Vote (
   void)
// ---------------------------------------------------------------------------
{
   return DisplayTwoButtonDialog( "", M_REVIEW_VOTE,
             B_REVIEW_VOTE_NO, B_REVIEW_VOTE_YES);
}
#endif
//</code></function>


/*<function id="ConfirmVoucher">
<heading>
  <description>
    <en>Ask elector to confirm the votes printed on the voucher.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description>
        <en>0 when left/cancel button is pressed OR 1 when right/accept button is pressed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int ConfirmVoucher (
   void
   )
// ---------------------------------------------------------------------------
{
    return DisplayTwoButtonDialog( "", M_VOUCHER, B_NO, B_YES);
}
#endif // _PREVIEW
//</code></function>


/*<function id="User_Confirms_Language">
<heading>
  <description><en>
    displays and processes a screen to ask for confirmation of the
    selected language.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 when left/cancel button is pressed OR 1 when right/accept button is pressed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
  <usage>
    if( User_Confirms_Language() == 1)</usage>
  <comment>
    RVH: 22/03/1994
    The confirm/cancel button uses 3 lines because the German text
    is very long to display.
    The window doesn't use the whole screen
    as for the bad selection error message.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int User_Confirms_Language (
   void)
// ---------------------------------------------------------------------------
{
    if ( Root.Language[0] == 1 || Root.Language[1] == 1 )
    {
        return DisplayTwoButtonDialog( "", M_LANGUAGE3,
                B_CANCEL_LANGUAGE3, B_CONFIRM_LANGUAGE3);
    }
    else
    {
        return DisplayTwoButtonDialog( "", M_LANGUAGE5,
                B_CANCEL_LANGUAGE5, B_CONFIRM_LANGUAGE5);
    }

}
#endif
//</code></function>


/*<function id="isInitializeDemoCardOk">
<heading>
  <description>
    <en>Ask elector to confirm the votes printed on the voucher.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>int</datatype>
      <description>
        <en>0 when left/cancel button is pressed OR 1 when right/accept button is pressed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
</heading>
<code>*/
#ifdef _DEMO
// ---------------------------------------------------------------------------
int isInitializeDemoCardOk (
   void)
// ---------------------------------------------------------------------------
{
    return DisplayTwoButtonDialog( "", M_DEMO_INITCARD, B_NO, B_YES);
}
#endif // _DEMO
//</code></function>




/*<function id="InitPartyData">
<heading>
  <description><en>
    Initialize the party's display data depending on mono or multi langual
    election, number of party in each langual group.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int InitPartyData ()
// ---------------------------------------------------------------------------
{
   int i;

   for( i = 0; i < 3; i++)
   {
     Parties.WhiteDisplayBox[i].X = 0;
     Parties.WhiteDisplayBox[i].Y = 0;
   }

   memset( & PartyScreen, 0, sizeof( PartyScreen) );
   PartyScreen.lang1 = Working_Language(ASK);

   if ( Parties.Choice[0].LanguageGroup == -1 )
   {
      // mono langual case
      LingualElectionType = MONO_LINGUAL;
      WayOfDisplay = ONE_SCREEN;
      PartyScreen.lang2 = -1;
      PartyScreen.LanguageGroup = -1;

      // party comptage
      PartyScreen.nbrparty1 = Parties.NbrOfParties;

      // nbr of row comptage
      PartyScreen.nbrrow1 = ( PartyScreen.nbrparty1 + 1 ) / 3;
      if ( ( ( PartyScreen.nbrparty1 + 1 ) % 3 ) != 0 )
         PartyScreen.nbrrow1++;

      // compute height of the party list area
      PartyScreen.nbrline = Query_Video_Mode() - HEADER - FOOTER;
      if ( Parties.DisplayCol )
      {
         // with colleges there's an extra back button taking up space!
         PartyScreen.nbrline -= BACKBUTTON;
      }

      return NONE;
   }
   else
   {
      // multi langual case
      LingualElectionType = PLURI_LINGUAL;

      if ( Root.Language[0] == PartyScreen.lang1 )
      {
         PartyScreen.lang2 = Root.Language[1];
      }
      else
      {
         PartyScreen.lang2 = Root.Language[0];
      }

      // party comptage, need comptage for each langual group
      for( i = 0 ; i < Parties.NbrOfParties ; i++)
      {
         if ( Parties.Choice[i].LanguageGroup == PartyScreen.lang1)
         {
            PartyScreen.nbrparty1++;
         }
         if ( Parties.Choice[i].LanguageGroup == PartyScreen.lang2)
         {
            PartyScreen.nbrparty2++;
         }
      }

      // nbr of row comptage, need comptage for each langual group
      PartyScreen.nbrrow1 = ( PartyScreen.nbrparty1 + 1 ) / 3;

      if ( ( ( PartyScreen.nbrparty1 + 1 ) % 3 ) != 0 )
      {
         PartyScreen.nbrrow1++;
      }

      PartyScreen.nbrrow2 = ( PartyScreen.nbrparty2 + 1 ) / 3;

      if ( ( ( PartyScreen.nbrparty2 + 1 ) % 3 ) != 0 )
      {
         PartyScreen.nbrrow2++;
      }

      // choice of the displaying way, if one of the la,ngual group get more
      // than 20 party so the langual group will be display on 2 different screen
      // if( PartyScreen.nbrparty1>=21 || PartyScreen.nbrparty2>=21)
      if ( PartyScreen.nbrparty1 >= 0 || PartyScreen.nbrparty2 >= 0 )
      {
         WayOfDisplay = DUAL_SCREEN; // two different screen
         //PartyScreen.nbrline = Query_Video_Mode() - ( 20*3 + 2*6);

         // compute height of the party list area
         PartyScreen.nbrline = Query_Video_Mode() - HEADER - FOOTER;
            // with languagegroups there's always an extra back button taking up space!
            PartyScreen.nbrline -= BACKBUTTON;

         return PartyScreen.lang1;
      }
      else
      {
         WayOfDisplay = SPLITED_SCREEN; // one screen but splited in 2
         PartyScreen.nbrline = ( Query_Video_Mode() - ( 20*3 + 2*6 ) ) / 2;
         return NONE;
      }
   }
}
//</code></function>


/*<function id="CalculatGapAndSize">
<heading>
  <description><en>
    Calculat the size of the party's display box and the gap between two
    display's box.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void CalculatGapAndSize ()
// ---------------------------------------------------------------------------
{
      int Nbr_row;

   if ( PartyScreen.nbrrow1 > PartyScreen.nbrrow2 )
   {
      Nbr_row = PartyScreen.nbrrow1;
   }
   else
   {
      Nbr_row = PartyScreen.nbrrow2;
   }

   // width of party display box
   PartyScreen.width = ( 0.90 * MAXRES) / MAXCOL;
   // horizontal gap between party display boxes
   PartyScreen.hgap =  ( 0.10 * MAXRES) / (MAXCOL + 1);
   PartyScreen.nbrline2 = 0;

   // the calcul is different in the case of dual, splited or simple screen
   switch ( WayOfDisplay)
   {
      case DUAL_SCREEN :
         // the size and gap are the same
         // position from left upper screen is the same
         //PartyScreen.height = ( 70 * PartyScreen.nbrline / 100 ) / Nbr_row;
         //PartyScreen.vgap = ( 30 * PartyScreen.nbrline / 100 ) / ( Nbr_row + 1 );
         PartyScreen.x1 = LEFTMARGIN + PartyScreen.hgap;
         PartyScreen.y1 = HEADER;
         PartyScreen.x2 = LEFTMARGIN + PartyScreen.hgap;
         PartyScreen.y2 = HEADER;
         if ( Parties.NbrOfParties < 6)
         {
            // reserve 70% of list area for boxes and 30% for white space
            PartyScreen.height =
               ( 0.70 * PartyScreen.nbrline) / Nbr_row;
            // we add 1 virtual row in order to spread the white space evenly
            //  around the party boxes
            PartyScreen.vgap =
               ( 0.30 * PartyScreen.nbrline) / ( Nbr_row + 1);
         }
         else
         {
            // reserve 90% of list area for boxes and 10% for white space
            PartyScreen.height =
               ( 0.92 * PartyScreen.nbrline) / Nbr_row;
            // we add 1 virtual row in order to spread the white space evenly
            //  around the party boxes
            PartyScreen.vgap =
               ( 0.08 * PartyScreen.nbrline) / ( Nbr_row + 1);
         }
         break;

      case SPLITED_SCREEN :
         // the size and gap are the same
         // position from left upper screen are different
         //PartyScreen.nbrrow1+PartyScreen.nbrrow2
         PartyScreen.height = ( 70 * PartyScreen.nbrline / 100 ) / ( Nbr_row);
         PartyScreen.vgap = ( 30 * PartyScreen.nbrline / 100) / ( Nbr_row + 1);
         PartyScreen.x1 = 4 + PartyScreen.hgap;
         PartyScreen.y1 = 26 + PartyScreen.vgap;
         PartyScreen.x2 = 4 + PartyScreen.hgap;
         PartyScreen.y2 = PartyScreen.nbrline + 46 + PartyScreen.vgap;
         PartyScreen.nbrline2 = PartyScreen.nbrline + 26;
         break;

      case ONE_SCREEN :
         // compute top-left position of list area
         PartyScreen.x1 = LEFTMARGIN + PartyScreen.hgap;
         PartyScreen.y1 = HEADER;

         if ( Parties.NbrOfParties < 6)
         {
            // reserve 70% of list area for boxes and 30% for white space
            PartyScreen.height =
               ( 0.70 * PartyScreen.nbrline) / PartyScreen.nbrrow1;
            // we add 1 virtual row in order to spread the white space evenly
            //  around the party boxes
            PartyScreen.vgap =
               ( 0.30 * PartyScreen.nbrline) / ( PartyScreen.nbrrow1 + 1);
         }
         else
         {
            // reserve 90% of list area for boxes and 10% for white space
            PartyScreen.height =
               ( 0.92 * PartyScreen.nbrline) / PartyScreen.nbrrow1;
            // we add 1 virtual row in order to spread the white space evenly
            //  around the party boxes
            PartyScreen.vgap =
               ( 0.08 * PartyScreen.nbrline) / ( PartyScreen.nbrrow1 + 1);
         }

         break;
   }
   // initialize size of party's display box
   Parties.Width = PartyScreen.width;
   Parties.Height = PartyScreen.height;

   return;
}
//</code></function>


/*<function id="FillParty">
<heading>
  <description><en>
    This is a general routine that fill the positions informations
    in the Partis structure according with the data pass trought parameters.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ecartx</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>ecarty</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>nbrrow</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>lang</name></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void FillParty (int x, int y, int ecartx, int ecarty, int nbrrow, int lang)
// ---------------------------------------------------------------------------
{

      int i, ind;
      int Row, Col;

   Row = y;
   Col = x;
   ind = 1;

   for ( i = 0; i < Parties.NbrOfParties; i++)
   {
      if ( Parties.Choice[i].LanguageGroup == lang)
      {
         Parties.Choice[i].DisplayBox.X = Col;
         Parties.Choice[i].DisplayBox.Y = Row;
         Row += ecarty;
         ind++;
         if ( ind > nbrrow)
         {
            ind = 1;
            Row = y;
            Col += ecartx;
         }
      }
   } // end for

   // load the position for the white vote
   if ( lang == -1) lang = 0;
   Parties.WhiteDisplayBox[lang].X = Col;
   Parties.WhiteDisplayBox[lang].Y = Row;

   return;
}

// FillParty () ############################################################
// Uses the FillParty (...) function to fill the position information
// in the Parties structure in the case of each way of display.
// (dual, splited or simple screen).
// Now the position informations are used either for diplaying and
// selecting.
// ########################################################################
void FillParty ()
{
   switch( WayOfDisplay)
   {
      case ONE_SCREEN:
         FillParty ( PartyScreen.x1, PartyScreen.y1+PartyScreen.vgap,
                    PartyScreen.hgap + PartyScreen.width,
                    PartyScreen.vgap + PartyScreen.height,
                    PartyScreen.nbrrow1, PartyScreen.LanguageGroup );
         break;

      case DUAL_SCREEN:
         FillParty ( PartyScreen.x1, PartyScreen.y1+PartyScreen.vgap,
                    PartyScreen.hgap + PartyScreen.width,
                    PartyScreen.vgap + PartyScreen.height,
                    PartyScreen.nbrrow1, PartyScreen.lang1 );

         FillParty ( PartyScreen.x2, PartyScreen.y2+PartyScreen.vgap,
                    PartyScreen.hgap + PartyScreen.width,
                    PartyScreen.vgap + PartyScreen.height,
                    PartyScreen.nbrrow2, PartyScreen.lang2 );
         break;

      case SPLITED_SCREEN:
         FillParty ( PartyScreen.x1, PartyScreen.y1,
                    PartyScreen.hgap + PartyScreen.width,
                    PartyScreen.vgap + PartyScreen.height,
                    PartyScreen.nbrrow1, PartyScreen.lang1 );

         FillParty ( PartyScreen.x2, PartyScreen.y2,
                    PartyScreen.hgap + PartyScreen.width,
                    PartyScreen.vgap + PartyScreen.height,
                    PartyScreen.nbrrow2, PartyScreen.lang2 );
         break;

      default :
         printf("ERREUR");
         exit(1);
         break;
   } // eo switch

   return;
}
//</code></function>


/*<function id="SelectLanguageGroup">
<heading>
  <description>
    <en>This routine allows user to select which lingual group he wants to see.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <comment>The user's language gets always displayed on top</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int SelectLanguageGroup (
   void
   )
// ---------------------------------------------------------------------------
{
      int lang[3]= { M_LANG_F, M_LANG_N, M_LANG_D};

   if ( WayOfDisplay == DUAL_SCREEN)
   {
      if (  DisplayTwoButtonChoice(
               Parties.ElectionName,
               Get_Text( H_CHOOSE_GROUP, Working_Language(ASK)),
               Get_Text( lang[PartyScreen.lang1], PartyScreen.lang1),
               Get_Text( lang[PartyScreen.lang1], PartyScreen.lang2),
               LEFT) == 1
         )
      {
         return PartyScreen.lang1;
      }
      else
      {
         return PartyScreen.lang2;
      }
   }
   else
   {
      return NONE;
   }
}
#endif
//</code></function>


/*<function id="Set_Party_Display_Mode">
<heading>
  <description>
    <en>set the way the party will be displayed.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage/>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Set_Party_Display_Mode ()
// ---------------------------------------------------------------------------
{
   InitPartyData(); // init language way of displaying party, ...
   CalculatGapAndSize();
   FillParty();

}
#endif
//</code></function>


/*<function id="Display_Parties">
<heading>
  <description>
    <en>Displays the party selection screen.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">struct Select_Party</datatype>
      <name>Parties</name>
      <description>
        <en>structure with the election description</en>
      </description>
    </parameter>
  </parameters>
  <usage>Display_Parties();</usage>
  <comment>
    Depending on the number of parties, the display mode and the
    coordinates are adapted.
    The strings used in the window title bar are selected according
    to the MAX_CHARS value. If the long names are too long, we
    switch to the corresponding short names. We don't check if the
    short names are too long.
    The blanco vote cell is defined as the number_of_parties + 1
    cell.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Parties ( int select)
// ---------------------------------------------------------------------------
{
      positionType Position;
      int line;
      int i;
      char label[128], label2[128];
      int lang[3]=
      {
        M_LANG_F,
        M_LANG_N,
        M_LANG_D
      };
      int   TextHeight;

   line = Query_Video_Mode();          //!\ read Set_Video_Mode
   Screen_White();

   // we select the strings that can be displayed in the title bar
   if ( LingualElectionType == MONO_LINGUAL ) // mean mono lingual election
   {
      if ( Parties.DisplayCol)
      {
         sprintf( label, "%s - %s", Parties.ElectionName, Parties.CollegeName);
         if ( strlen( label) > MAX_CHARS)
         {
            sprintf( label, "%s - %s",
               Parties.ElectionSName, Parties.CollegeName);
         }
      }
      else
      {
         sprintf( label, "%s", Parties.ElectionName);
         if ( strlen( label) > MAX_CHARS)
            sprintf( label, "%s", Parties.ElectionSName);
      }
   }
   else
   {
      // plurilingual election
      // check if there is a second language
      if ( Parties.DisplayCol)
      {
         sprintf( label, "%s - %s", Parties.ElectionName,
                  Parties.CollegeName);
         if( strlen( label) > MAX_CHARS)
             sprintf( label, "%s - %s", Parties.ElectionSName,
                      Parties.CollegeName);
      }
      else
      {
         if( select != NONE )
             i = select;
         else
             i = PartyScreen.lang1;
         sprintf( label, "%s - %s", Parties.ElectionName,
                  Get_Text( lang[PartyScreen.lang1], i ));
         if( strlen( label) > MAX_CHARS)
             sprintf( label, "%s - %s", Parties.ElectionSName,
                      Get_Text( lang[ PartyScreen.lang1 ], i ));
      }

      if ( Parties.DisplayCol)
      {
         sprintf( label2, "%s - %s", Parties.ElectionName,
                  Parties.CollegeName );
         if ( strlen( label2) > MAX_CHARS)
            sprintf( label2, "%s - %s", Parties.ElectionSName,
                     Parties.CollegeName);
      }
      else
      {
         sprintf( label2, "%s - %s", Parties.ElectionName,
                  Get_Text( lang[ PartyScreen.lang1], PartyScreen.lang2 ));
         if( strlen( label2) > MAX_CHARS)
             sprintf( label2, "%s - %s", Parties.ElectionSName,
                      Get_Text( lang[ PartyScreen.lang1 ], PartyScreen.lang2 ));
      }

      if ( PartyScreen.nbrline2 != 0 )
      {
         Info_Box( PartyScreen.nbrline2, label2, LEFT);
      }
   }

   // display list header containing election name
   displayHeader( 0, label, LEFT);

   // display list footer containing help text
   displayFooter( line - FOOTER,
      Get_Text( H_CHOOSE_PARTY, Working_Language( ASK)), RIGHT);

   TextHeight = 12;

   // display the party list area
   for ( i = 0 ; i < Parties.NbrOfParties ; i++)
   {
      if ( select != NONE && Parties.Choice[i].LanguageGroup != select )
         continue;

      // party box
      DrawBox( Parties.Choice[i].DisplayBox.Y, Parties.Choice[i].DisplayBox.X,
         Parties.Height, Parties.Width, BLACK);

      // party's number
      sprintf( label, "%2d", Parties.Choice[i].Ind);
      zegsp( Parties.Choice[i].DisplayBox.Y + ( Parties.Height / 2 )
         - ( TextHeight / 2), // row
         Parties.Choice[i].DisplayBox.X + 10, // col
         label, BLACK, strlen( label), 1);

#ifndef _DEMO
      // in production version we display the logo instead of the name when
      // it's available

      if ( Parties.Choice[i].Logo.Width && Parties.Choice[i].Logo.Height)
      {
         // party's logo ( vertically centered in the party box)!
         Position.Row = Parties.Choice[i].DisplayBox.Y +
            ((Parties.Height - Parties.Choice[i].Logo.Height) / 2);
         Position.Col = Parties.Choice[i].DisplayBox.X + 34;
         displayLogo( Position, Parties.Choice[i].Logo,
            Parties.ElectionInd, Parties.CollegeInd, Parties.Choice[i].Ind);
      }
      else
#endif
      {
         // 20040327 avn - display long party names
         sprintf( label, "%s", Parties.Choice [i].Name);
         DisplayPartyName( label,
            Parties.Choice[i].DisplayBox.Y + ( Parties.Height / 2 ),
            Parties.Choice[i].DisplayBox.X + 34, TextHeight);
      }

   } // eo for

   // display the blanco vote box
   for ( i = 0; i < 3; i++)
   {
      if ( select != NONE && i != select)
         continue;

      if ( Parties.WhiteDisplayBox[i].X != 0)
      {
         kad ( Parties.WhiteDisplayBox[i].Y, Parties.WhiteDisplayBox[i].X,
            Parties.WhiteDisplayBox[i].Y + Parties.Height,
            Parties.WhiteDisplayBox[i].X + Parties.Width, 0);
         sprintf( label, "   %s",
            Get_Text( B_WHITE_VOTE, Working_Language(ASK)));
         zegsp( Parties.WhiteDisplayBox[i].Y + ( Parties.Height / 2 ) - 5,
            Parties.WhiteDisplayBox[i].X, label, 0, strlen( label), 1);
      }
   } // eo for

   Parties.BackButton.Position.Row = line - FOOTER - BACKBUTTON;
   Parties.BackButton.Position.Col = 120;
   Parties.BackButton.Width = 400;
   Parties.BackButton.Height = BACKBUTTON;

   // display the back button ( in case of colleges/language groups)
   if ( Parties.DisplayCol)
   {
      knopsp( Parties.BackButton.Position.Row, Parties.BackButton.Position.Col,
         Parties.BackButton.Position.Col + Parties.BackButton.Width,
         Get_Text( B_OTHER_COL , Working_Language( ASK)),
         strlen( Get_Text( B_OTHER_COL , Working_Language( ASK))), 0);
   }
   if ( select  !=  NONE)
   {
      knopsp( Parties.BackButton.Position.Row, Parties.BackButton.Position.Col,
         Parties.BackButton.Position.Col + Parties.BackButton.Width,
         Get_Text( B_OTHER_GROUP, Working_Language( ASK)),
         strlen( Get_Text( B_OTHER_GROUP, Working_Language( ASK))), 0);
   }

   return;
}
//</code></function>


/*<function id="Input_Parties">
<heading>
  <description>
    <en>Process the selection of a party</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>SELECT</datatype>
      <description><en>
          DONE  = a party or blanco vote was selected
          RETRY = blanco vote was canceled and no party is selected
          OTHER = the user wants to select another college</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>struct Select_Party</datatype><name>Parties</name>
      <description><en>structure with the election description</en></description></parameter>
  </parameters>
  <usage>
    Done = Input_Parties();</usage>
  <comment>
    The blanco vote cell is defined as the number_of_parties + 1
    cell. If there is more than one college, the user has the possibility
    to go back to the college selection.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
SELECT Input_Parties (
   int select)
// ---------------------------------------------------------------------------
{
   int pencol, penrow;

   for( ; ; ) // continue until a choice is made
   {
      Get_Pen( & pencol, & penrow);

      // back button area hit?
      if ( Pen_In_Window( pencol, penrow,
              Parties.BackButton.Position.Col, Parties.BackButton.Position.Row,
              Parties.BackButton.Position.Col + Parties.BackButton.Width,
              Parties.BackButton.Position.Row + Parties.BackButton.Height))
      {
         // it's the college choice button
         if ( Parties.DisplayCol) return OTHER_COL;

         // it's the other language group button
         if ( WayOfDisplay == DUAL_SCREEN) return OTHER_LANG;
      }

      // test if the lightpen hit a party cell
      for ( int i = 0 ; i < Parties.NbrOfParties; i++)
      {
         if( select != NONE && Parties.Choice[i].LanguageGroup!=select)
             continue;

         if( pencol > Parties.Choice[i].DisplayBox.X &&
             pencol < Parties.Choice[i].DisplayBox.X+Parties.Width &&
             penrow > Parties.Choice[i].DisplayBox.Y &&
             penrow < Parties.Choice[i].DisplayBox.Y+ Parties.Height)       // party selected
         {
             Parties.Selected = Parties.Choice[i].Ind;
             return DONE;
         }
      } // for NbrOfParties

      // look if blanco vote selected in one of the languagegroups
      for( i = 0; i < 3 ; i++)
      {
         if( select != NONE && i != select)
             continue;

         if( pencol > Parties.WhiteDisplayBox[i].X &&
             pencol < Parties.WhiteDisplayBox[i].X + Parties.Width &&
             penrow > Parties.WhiteDisplayBox[i].Y &&
             penrow < Parties.WhiteDisplayBox[i].Y + Parties.Height &&
             Parties.WhiteDisplayBox[i].X!=0 )
         {
             if( Process_Blanco_Vote() == 1)    // white vote confirmed
             {
                 Parties.Blanco_Vote = TRUE;
                 return DONE;
             }// if

             return OTHER_PARTY;
         }//if
      } // for blanco vote

    } // for ever
}
#endif
//</code></function>

/*<function id="Process_Blanco_Vote">
<heading>
  <description>
    <en>displays and process a screen to ask for 'White Vote' confirmation</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 when left/cancel button is pressed OR 1 when right/accept button is pressed.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
    <parameter type="global">
      <datatype>struct Select_Party</datatype><name>Parties</name>
      <description><en>structure with the election description</en></description></parameter>
  </parameters>
  <usage>
    if( Process_Blanco_Vote() == 1)</usage>
  <comment>
    Each confirm button uses 3 lines because the German text is
    very long to display.
    The 'White Vote' confirm window doesn't use the whole screen
    as for the bad selection error message.
  </comment>
</heading>
<code>*/

#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int Process_Blanco_Vote (
   void)
// ---------------------------------------------------------------------------
{
    return DisplayTwoButtonDialog( Parties.ElectionSName, M_BLANCO,
            B_CANCEL_WHITE_VOTE1, B_CONFIRM_WHITE_VOTE1);
}
#endif
//</code></function>

/*<function id="DisplayTextBold">
<heading>
  <description>
    <en>Display a text using a custom 'bold' effect.</en>
  </description>
  <parameters/>
  <usage/>
  <comment>In order to obtain a bold effect we display each letter twice with
  one pixel offset</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
void DisplayTextBold (
   ALIGN TextAlign,
   int iCol,
   int iRow,
   char* cpText,
   BOOL BeautifyTrema
   )
// ---------------------------------------------------------------------------
{
      int i;
      int iCursor;
      char cLetter[2];
      int iTextLength;

      const int iOffset = 1;
      const int iWhiteSpace = 1;             // in pixels

   iTextLength = strlen( cpText);
   iCursor = iCol;
   settextjustify( LEFT_TEXT, TOP_TEXT);

   switch ( TextAlign)
   {
      case RIGHT:
      {
         // start with last character of text
         i = iTextLength - 1;
         while ( i >= 0)
         {
            // take one character from text
            memcpy( cLetter, cpText+i, 1);
            cLetter[1] = '\0';
            // display it twice with one pixel offset
            iCursor -= ( textwidth( cLetter) + iOffset);
            outtextxy( iCursor, iRow, cLetter);
            outtextxy( iCursor + iOffset, iRow, cLetter);
            // leave 2 pixels between individual letters for readability
            iCursor -= iWhiteSpace;
            // process next character
            i--;
         }
         break;
      }
      case CENTER:
      {
         iCursor -= ( textwidth( cpText) + ( strlen( cpText) * iWhiteSpace)) / 2;
         // fall thru intended!

      }
      default: // align LEFT is default
      {
         // start with first character of text
         i = 0;
         while ( i <= iTextLength)
         {
            // take one character from text
            memcpy( cLetter, cpText+i, 1);
            cLetter[1] = '\0';
            // display it twice with one pixel offset
            if ( cLetter[0] == 'Ž')						// DOS character 142 - A umlaut
            {
               // 20040601 avn
               outtextxy( iCursor, iRow, cLetter);
               outtextxy( iCursor + iOffset, iRow, "A");
               if ( BeautifyTrema)
               {
                  putpixel( iCursor+5, iRow+2, WHITE);
                  putpixel( iCursor+5, iRow+3, WHITE);
               }
            }
            else
            {
               outtextxy( iCursor, iRow, cLetter);
               outtextxy( iCursor + iOffset, iRow, cLetter);
            }
            // leave 2 pixels between individual letters for readability
            iCursor += ( textwidth( cLetter) + iWhiteSpace);
            // process next character
            i++;
         }
         break;
      }
   } // eo switch

   return;
}
#endif
//</code></function>

/*<function id="DisplayPartyName">
<heading>
  <description>
    <en>displays party name in the partybox.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>cpElectionName</name>
      <description><en>Name of the election</en></description>
    </parameter>
  </parameters>
  <comment>We need a dedicated routine because long partynames must wrap,
  truncating is not acceptable :-(.</comment>
  <author>AVN</author>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
static void DisplayPartyName (
   char* cpPartyName,                  // party label
   int   iRow,                         // row position in pixels
   int   iCol,                         // col position in pixels
   int   iHeight                       // textheight in pixels
   )
// ---------------------------------------------------------------------------
{
      const int   MAXL = 2;            // max nof text bits allowed

      char* Text[MAXL];
      int   n_lines;
      int   Cursor;

   n_lines = Split_Text( cpPartyName, Text, MAXL, 19, CHARS);
   if ( n_lines > MAXL)
   {
      // the text is too long for the display, so it will be cut
      n_lines = MAXL;
   }

   // display header
   if ( n_lines != 0)
   {
      Cursor = iRow - ( iHeight * n_lines / 2);
      for ( int i = 0; i < n_lines; i++)
      {
         zegsp( Cursor, iCol, Text[i], BLACK, strlen( Text[i]), 1);
         free( Text [i]);
         Cursor = Cursor + iHeight;
      }
   }

   return;
}
#endif // _APPLICATION_MAV
//</code></function>


/*<function id="DisplayTwoButtonChoice">
<heading>
   <description>
     <en>Displays a screen to make a choice, the screen has a header and footer
     bar and two buttons centered vertically. All the labels are provided to the
     function. The function returns 1 if the first button is pressed or 2 for
     the second one.</en>
   </description>
   <parameters/>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int DisplayTwoButtonChoice (
   char* cpHeader,            // Header label
   char* cpFooter,            // Footer label
   char* cpFirst,             // First button label
   char* cpSecond,            // Second button label
   ALIGN Aligned              // Header and footer alignment type
   )
// ---------------------------------------------------------------------------
{
      const int BorderSize = 7;  // height of the grey screen border

      areaType Scr;              // screen properties
      areaType Header, Footer;
      areaType First, Second;

      int pencol, penrow;        // lightpen coordinates

   // init screen res
   Set_Mode( VGA_MODE, CARACT16x8);

   Scr.Width = getmaxx();
   Scr.Height = getmaxy();

   settextstyle( DEFAULT_FONT, HORIZ_DIR, 2);

   // draw screen
   Screen_White();
   setcolor( BLACK);

   Make_Border( 0, 0, Scr.Height, Scr.Width);

   // draw header
   Header.Position.Row = BorderSize;
   Header.Position.Col = BorderSize;
   Header.Width = Scr.Width - ( 2 * BorderSize);
   Header.Height = 2 * textheight( cpHeader);

   DrawBox( Header.Position.Row, Header.Position.Col,
      Header.Height, Header.Width, BLACK);
   if ( CENTER == Aligned)
   {
      zegsp( Header.Position.Row + (( Header.Height - 16) / 2),
         ( Scr.Width - ( strlen( cpHeader) * 8)) / 2,
         cpHeader, BLACK, strlen( cpHeader), 1);
   }
   else
   {
      zegsp( Header.Position.Row + (( Header.Height - 16) / 2),
         2 * BorderSize,
         cpHeader, BLACK, strlen( cpHeader), 1);
   }

   // draw footer
   Footer = Header;
   Footer.Position.Row = Scr.Height - BorderSize - Header.Height;

   DrawBox( Footer.Position.Row, Footer.Position.Col,
      Footer.Height, Footer.Width, BLACK);
   if ( CENTER == Aligned)
   {
      zegsp( Footer.Position.Row + (( Footer.Height - 16) / 2),
         ( Scr.Width - ( strlen( cpFooter) * 8)) / 2,
         cpFooter, BLACK, strlen( cpFooter), 1);
   }
   else
   {
      zegsp( Footer.Position.Row + (( Footer.Height - 16) / 2),
         Scr.Width - ( 2 * BorderSize) - ( strlen( cpFooter) * 8),
         cpFooter, BLACK, strlen( cpFooter), 1);
   }

   // set body text style
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
   settextjustify( CENTER_TEXT, CENTER_TEXT);
   setusercharsize( 1, 1, 1, 1);
   setcolor( BLACK);

   // draw first button
   if ( textwidth( cpFirst) > textwidth( cpSecond))
   {
      First.Width = textwidth( cpFirst);
   }
   else
   {
      First.Width = textwidth( cpSecond);
   }
   First.Width += textwidth( "    ");
   First.Height = textheight( cpFirst) * 2;
   First.Position.Row = BorderSize + Header.Height +
      ( 1 * (( Scr.Height - ( 2 * Header.Height) - ( 2 * BorderSize)) / 3)) -
      ( First.Height / 2);
   First.Position.Col = ( Scr.Width - First.Width) / 2;

   DrawBox( First.Position.Row, First.Position.Col,
      First.Height, First.Width, BLACK);
   outtextxy( First.Position.Col + ( First.Width / 2),
      First.Position.Row + ( First.Height / 2) - 3, cpFirst);

   // draw second button
   Second = First;

   Second.Position.Row = Header.Height +
      ( 2 * (( Scr.Height - ( 2 * Header.Height) - ( 2 * BorderSize)) / 3)) -
      ( Second.Height / 2);

   DrawBox( Second.Position.Row, Second.Position.Col,
      Second.Height, Second.Width, BLACK);
   outtextxy( Second.Position.Col + ( Second.Width / 2),
      Second.Position.Row + ( Second.Height / 2) - 3, cpSecond);

   // wait for a selection
   for ( ;;)
   {
      Get_Pen( & pencol, & penrow);

      if (  Pen_In_Window( pencol, penrow,
               First.Position.Col, First.Position.Row,
               First.Position.Col + First.Width, First.Position.Row + First.Height)
         ) return 1;
      if (  Pen_In_Window( pencol, penrow,
               Second.Position.Col, Second.Position.Row,
               Second.Position.Col + Second.Width, Second.Position.Row + Second.Height)
         ) return 2;
   }
}
#endif // #ifndef _PREVIEW
//</code></function>


/*<function id="Make_Border">
<heading>
  <description>
    <en>draws a box with a fancy grey border</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>top</name>
      <description><en>the top coordinate of the window (y_min)</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>left</name>
      <description><en>the left coordinate of the window (x_min)</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>bottom</name>
      <description><en>the bottom coordinate of the window (y_max)</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>right</name>
      <description><en>the right coordinate of the window (x_max)</en></description></parameter>
  </parameters>
  <usage>Make_Border(0, 0, 349, 639);</usage>
  <comment>
    border is 8 px wide in all directions, delimited by black lines, filled
    gey, bottom and right have a 3d-ish finishing touch.
  </comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Make_Border (
   int top,
   int left,
   int bottom,
   int right
   )
// ---------------------------------------------------------------------------
{
   kad( top,   left,   bottom,   right,   BLACK);
   kad( top+7, left+7, bottom-7, right-7, BLACK);

   achtersp( top+1,    left+1,  top+6,    right-1, LIGHTGRAY);
   achtersp( bottom-6, left+1,  bottom-1, right-1, LIGHTGRAY);
   achtersp( bottom-2, left+1,  bottom-1, right-1, CYAN);
   achtersp( top+7,    left+1,  bottom-7, left+6,  LIGHTGRAY);
   achtersp( top+7,    right-6, bottom-7, right-1, LIGHTGRAY);
   achtersp( top+1,    right-3, bottom-1, right-1, CYAN);

   return;
}
#endif // #ifndef _PREVIEW
//</code></function>


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
