/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavmes.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVMES.CPP  $</workFile>
<revision>$Revision: 1.8 $</revision>
<workFileTimeStamp>$Modtime:   Jan 14 2005 15:44:52  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVMES.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/27 13:06:28 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVMES.CPP</fileName>
  <creationDate>01/01/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Librairie de fonctions gestion des messages et langues</fr>
    <nl>Functielibrary beheer boodschappen en talen</nl></description>
  <history>
   01/06/93: revision history starting date</history>
 <comment>Functions for displaying messages on the screen and for the languages.</comment>
</comments>
*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <conio.h>                     // gotoxy
#include <string.h>                    // strcpy
#include <dos.h>                       // sleep
#include <time.h>                      // time

#include "..\gen\gentype.h"
#include "..\gen\gencard.h"
#include "..\gen\gendata.h"
#include "..\gen\genstr.h"             // Split_Text

#include "..\mav\mavasm.h"
#include "..\mav\mavvideo.h"
#include "..\mav\mavlight.h"           // Get_Pen
#include "..\mav\mavmes.h"

#ifdef _PREVIEW
   #include "..\divers\logging.h"
#endif // _PREVIEW

//</includes>

//----------------------------------------------------------------------------
//                                         declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define MARGIN          5   // Margin/2 between columns
#define B_FROM_BOTTOM   40  // Place button at .. from bottom of window

#define INFO_SLEEP      5   // Time to sleep after a guidance screen ( in sec.)
#define MAX_LABEL       80  // Size of labels to display (in characters)
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>

static void Display_Languages(
   void);
static void DisplayProgramVersion(
   void);

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern struct ElectionMain Root;

//----------------------------------------------------------------------------
//                                              definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
const int HALF_HEIGHT=16,
          HALF_WIDTH=74,
          BORDER=3;

struct sButtonInfo { short WidthBox;
                     short HeightBox;
                     short DiffCircleHor;
                     short DiffCircleVer;
                     struct { short X;
                              short Y;
                              BOOL AllignLeft;
                              BOOL AllignTop;
                            } Position[4];
                   } ButtonInfo
= { 31, 28, -4, -8, { { 0, 0, TRUE,  TRUE  },
                      { 0, 0, FALSE, TRUE  },
                      { 0, 0, TRUE,  FALSE },
                      { 0, 0, FALSE, FALSE } }
  };
//</globals>

//============================================================= EOF HEADER ===
//</header>

//=============================================================== BOF BODY ===
//<body>

/*<function id="Display_Strings">
<heading>
  <description>
    <en>Displays strings in line centered on a column</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Mnemo</name>
      <description><en>mnemonic of the text to display</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Language</name>
      <description><en>
        the column of the text in the message array
        (this corresponds to the language)</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>mid_x</name>
      <description><en>the column where to center the string on</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>min_y</name>
      <description><en>the starting top position of the text</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>height</name>
      <description><en>the height of the character set used</en></description></parameter>
    <parameter type="global">
      <datatype>sMSG</datatype><name>MSG_GEN[MAX_MSG_GEN]</name>
      <description><en>the message array</en></description></parameter>
  </parameters>
  <usage>
    Display_Strings( M_WAIT, Working_Language(ASK), mid, min_y, height);</usage>
  <checks>
    none, the mnemo value is supposed to be possible</checks>
  <comment>
    This function displays the message that corresponds to the mnemonic in a
    columnar style. The text is centered on the provided column.

    The message array is a 2 dimensional array. There is an entry for every
    mnemonic and this entry contains the text of the message for each of the
    supported languages. A message can be a single line message or a multi-line
    message using \n as the line separator, blank lines are represented by a space.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Strings (
   int Mnemo,
   int Language,
   int mid_x,
   int min_y,
   int height
   )
// ---------------------------------------------------------------------------
{

       char cMess[350];         // holds the message text (cfr. M_DEMO_MENU_DISPLAY)
       char cToken[] = "\n";    // holds the line delimiter
       char* cpLine;

    // we need a local copy of the message text because strtok() will
    // replace the delimiters with null bytes
    strcpy( cMess, Get_Text( Mnemo, Language));

    // do we have delimiters in the message?
    cpLine = strtok( cMess, cToken);

    if ( cpLine)    // yep!
    {
        do
        {
            // display the line
            outtextxy( mid_x, min_y, cpLine);
            min_y += height;
            // get next line
            cpLine = strtok( NULL, cToken);
        } while ( cpLine);
    }
    else            // nop!
    {
        // display the line
        outtextxy( mid_x, min_y, cpLine);
        min_y += height;
    }

    return;
}
//</code></function>


/*<function id="Display_Message">
<heading>
  <description><en>
    This function displays messages in a window and in a columnar
    style (1 or 2, according to the number of languages to display).</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Mnemo</name>
      <description><en>position of the message in the message array</en></description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Confirm</name>
      <description><en>put a confirm button ?</en></description>
      <defaultValue>NO</defaultValue></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Timeout</name>
      <description><en>number of seconds to display the screen</en></description>
      <defaultValue>0</defaultValue></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Bypass</name>
      <description><en>ability to bypass the timeout?</en></description>
      <defaultValue>NO</defaultValue></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>size_x</name>
      <description><en>
        controls the font size:
            FontWidth  *= size_x/Size_X
            FontHeight *= size_y/Size_Y
        defautl values = X: 1/1 , Y: 1/1</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Size_X</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>size_y</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Size_Y</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x_min</name>
      <description><en>
        window coordinates to use
        default value = fullscreen</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y_min</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x_max</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y_max</name></parameter>

    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype><name>Root</name>
      <description><en>contains the used languages</en></description></parameter>
  </parameters>
  <usage>
      Display_Message (M_WAIT)  to use the default settings</usage>
  <comment>
    You can control the window caracteristics, the font size, the
    ability to wait for confirmation from the user, the display
    time, the ability to bypass the display time.
    When a confirmation is asked, the function displays and waits
    on that button, when the user confirms, the button is erased but
    message is left on the screen.
    When a timeout is selected, there is also the possibility to
    shortcut it if the Bypass flag is set, this allows the user by
    pressing the lightpen on the screen to go further immediately.
    If the Confirm and Timeout options are selected, only the
    confirm option will be processed.</comment>
</heading>
<code>*/
#ifdef _PREVIEW

#pragma argsused
// ---------------------------------------------------------------------------
void Display_Message (
   int Mnemo, BOOL Confirm, int Timeout, BOOL Bypass,
   int size_x, int Size_X, int size_y, int Size_Y,
   int x_min, int y_min, int x_max, int y_max )
// ---------------------------------------------------------------------------
{
   // dummy function
}

#else

// ---------------------------------------------------------------------------
void Display_Message (
   int Mnemo,                 // position of the message in the message array
   BOOL Confirm,              // put a confirm button ?
   int Timeout,               // number of seconds to display the screen
   BOOL Bypass,               // ability to bypass the timeout?
   int size_x,                // controls the font size: FontWidth  *= size_x/Size_X
   int Size_X,                //
   int size_y,                //                         FontHeight *= size_y/Size_Y
   int Size_Y,                //
   int x_min,                 // window coordinates to use, default = fullscreen
   int y_min,                 //
   int x_max,                 //
   int y_max                  //
   )
// ---------------------------------------------------------------------------
{
    int i,                     // counter
        min_x, min_y, max_x,   // used to recalculate coordinates
        mid,                   // mid position of a column
        height,                // text height
        penrow, pencol;        // position of lightpen on screen

    // switch to VGA mode. This will no longer be usefull if all the messages
    // are displayed using VGA mode
    Set_Mode( VGA_MODE ,CARACT16x8);

    // fill the screen in white
    Screen_White();

    // make a border with a shadow effect
    Make_Border( y_min, x_min, y_max, x_max);

    if ( M_TEST_LIGHTPEN == Mnemo || M_REMOVE_CARD_AND_DISK == Mnemo)
    {
       DisplayProgramVersion();
    }

    // set text characteristics
    setcolor(0);
    settextjustify( CENTER_TEXT, TOP_TEXT);
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
    setusercharsize( size_x, Size_X, size_y, Size_Y);
    height = textheight( "p‚");

    // display the messages according to the number of languages

    if( Working_Language( ASK) != NONE)  // one language, one column
    {
        min_x = x_min + MARGIN;
        min_y = y_min + MARGIN;
        max_x = x_max - MARGIN;
        mid   = (min_x + max_x) / 2;

        // display each line of the array on a single line
        Display_Strings( Mnemo, Working_Language( ASK), mid, min_y, height );
    }
    else  // two languages, in two columns
    {
      // left side
        mid   = (x_min + x_max) / 2;
        min_x = x_min + MARGIN;
        max_x = mid - MARGIN;
        min_y = y_min + MARGIN;
        mid   = (min_x + max_x) / 2;

        Display_Strings( Mnemo, Root.Language [0], mid, min_y, height );

      // right side
        mid   = (x_min + x_max) / 2;
        min_x = mid + MARGIN;
        max_x = x_max - MARGIN;
        min_y = y_min + MARGIN;
        mid   = (min_x + max_x) / 2;

        Display_Strings( Mnemo, Root.Language [1], mid, min_y, height );
    }

    // Now process the flags

   if( Confirm)
   {
      // 20041109 avn - removed the TEST_LIGHTPEN_BUTTONS bloc

      // draw a confirmation button and wait on it
      min_x = (x_min + x_max)/ 2;
      min_y = y_max - B_FROM_BOTTOM;
      Make_Ok_Button( "OK", min_x, min_y, HALF_WIDTH, HALF_HEIGHT, BORDER );

      for(;;)
      {
          Get_Pen( & pencol, & penrow );

          if( Pen_In_Window( pencol, penrow,
                             min_x - HALF_WIDTH, min_y - HALF_HEIGHT,
                             min_x + HALF_WIDTH, min_y + HALF_HEIGHT) )
              break;
      }

      Erase_Ok_Button( min_x, min_y, HALF_WIDTH, HALF_HEIGHT);
   }
   else
   if( Timeout && Bypass )
   {
      time_t t = time(NULL) + (long) Timeout;

      // When lightpen is still activated, we have to wait until
      // it is released.

      for(; ( time( NULL) < t) && (  Lightpen_Hit() );); // Loop till released
      for(; ( time( NULL) < t) && ( !Lightpen_Hit() );); // Loop till hit
   }
   else
   if( Timeout )
      sleep( Timeout);
}
#endif // _PREVIEW
//</code></function>


/*<function id="Ask_User_For_Language">
<heading>
  <description><en>
    function to selected a language between two choices</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>
        the selected language (corresponding to the column
        in the message array)</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype><name>Root</name>
      <description><en>contains the used languages</en></description></parameter>
  </parameters>
  <usage>
    Working_Language( Ask_User_For_Language());</usage>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int Ask_User_For_Language
   (  void
   )
// ---------------------------------------------------------------------------
{

   if (  DisplayTwoButtonChoice(
            Get_Text( H_CHOOSE_LANGUAGE, Root.Language [0]),
            Get_Text( H_CHOOSE_LANGUAGE, Root.Language [1]),
            Get_Text( B_LANGUAGE, Root.Language [0]),
            Get_Text( B_LANGUAGE, Root.Language [1]),
            CENTER) == 1
      )
   {
      return Root.Language [0];
   }
   else
   {
      return Root.Language [1];
   }
}
#endif
//</code></function>

/*<function id="Error_Exit">
<heading>
  <description><en>
   displays an error message (and lock the program)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the message mnemonic</en></description></parameter>
  </parameters>
  <usage>
   Error_Exit (M_ERROR);
   Error_Message (M_ERROR);</usage>
 <comment>
   The only difference between Error_Exit and Error_Message is that
   Error_Exit locks the program by running an infinite loop.
    These two functions where created to distinguish common messages
   from error messages. In fact they do the same as Display_Message
    using the default values.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Error_Exit ( int M_Index)
// ---------------------------------------------------------------------------
{
#ifdef _PREVIEW
   char cString[80];

   sprintf( cString, "PREVIEW : Memory allocation error (%d)", M_Index);
   Add_To_Logging( ERR, cString);
#else
   Error_Message( M_Index);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#ifdef TRACE
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   sleep(5);

   if( M_Index == M_BAD_READER1     || M_Index == M_BAD_READER2     ||
      M_Index == M_INTERNAL_ERROR1 || M_Index == M_INTERNAL_ERROR2 ||
      M_Index == M_ERR_READ1       || M_Index == M_ERR_READ2)

      Print_Trace_Buffer( stdout);

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#endif
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   for(;;);

#endif
}
//</code></function>

//-----------------------------------------------------------------------------

/*<function id="Error_Message">
<heading>
  <description><en>
   displays an error message (and lock the program)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the message mnemonic</en></description></parameter>
  </parameters>
  <usage>
   Error_Exit (M_ERROR);
   Error_Message (M_ERROR);</usage>
 <comment>
   The only difference between Error_Exit and Error_Message is that
   Error_Exit locks the program by running an infinite loop.
    These two functions where created to distinguish common messages
   from error messages. In fact they do the same as Display_Message
    using the default values.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Error_Message ( int M_Index)
// ---------------------------------------------------------------------------
{
   Display_Message( M_Index);
}
//</code></function>


/*<function id="DisplayTwoButtonDialog">
<heading>
   <description>
     <en>Display a dialog with a title, a message and two buttons. The title, the
     message and the content of the buttons are provided by message mnemonics. The
     left button contains the 'cancel' message and returns 0 when pressed, the
     right one the 'confirm' message and returns 1.</en>
   </description>
   <parameters>
     <parameter type="return"><datatype>int</datatype></parameter>
     <parameter type="arg"><datatype>char</datatype></parameter>
     <parameter type="arg"><datatype>char*</datatype><name>cTitle</name>
       <description><en>Title of the dialog, use NULL when no title is
       required</en></description><defaultValue/>
     </parameter>
     <parameter type="arg"><datatype>int</datatype><name>iMessage</name>
       <description><en>Mnemonic for the message</en></description><defaultValue/>
     </parameter>
     <parameter type="arg"><datatype>int</datatype><name>iCancels</name>
       <description><en>Mnemonic for the cancel message</en></description><defaultValue/>
     </parameter>
     <parameter type="arg">
       <datatype>int</datatype><name>iConfirms</name>
       <description><en>Mnemonic for confirm message</en></description><defaultValue/>
     </parameter>
   </parameters>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
int DisplayTwoButtonDialog (
   char* cpTitle,              // Title of the dialog, use NULL when no title is required
   int   iMessage,            // Mnemonic for the message
   int   iCancels,            // Mnemonic for the cancel message
   int   iConfirms            // Mnemonic for confirm message
   )
// ---------------------------------------------------------------------------
{
      // avn 20040103 - bigger button's for german text labels
      //    B_CANCEL_WHITE_VOTE1 and B_CONFIRM_WHITE_VOTE1

      areaType Box = { 0, 0, 550, 300};   // message box width and height
      const int BoxBorder = 7;            // cfr. Make_Border

      const areaType Button = { 270, 0, 258, 100};
      const int Border = 3;
      const int Buttons = 2;
      const int Gaps = Buttons + 1;

      int iPenCol, iPenRow, iLang, iTextY;
      int iLeftX;
      int iRightX;

   Set_Mode( VGA_MODE, CARACT14x8);

   // center message box on screen
   Box.Position.Row = ( getmaxy() - Box.Height) / 2;
   Box.Position.Col = ( getmaxx() - Box.Width) / 2;

   // compute horizontal position of the buttons
   iLeftX = Box.Position.Col + BoxBorder +
      ( Box.Width - ( BoxBorder * 2) - ( Button.Width * Buttons)) / Gaps;
   iRightX = iLeftX + Button.Width +
      ( Box.Width - ( BoxBorder * 2) - ( Button.Width * Buttons)) / Gaps;

   // display message box with the message in the top half
   Display_Message( iMessage, NO, 0, NO, 8,10,8,10,
      Box.Position.Col, Box.Position.Row,
      Box.Position.Col + Box.Width, Box.Position.Row + Box.Height);

   if ( cpTitle)
   {
      // display title above the message box
      DisplayElectionName( cpTitle);
   }
   settextjustify( CENTER_TEXT, BOTTOM_TEXT);

   Draw_Button(  iLeftX, Button.Position.Row, Button.Width, Button.Height, Border);
   Draw_Button( iRightX, Button.Position.Row, Button.Width, Button.Height, Border);

   setusercharsize( 7, 10, 7, 10);

   /* :TODO: avn 20020916 - Display_Strings should take care of this */
   iLang = Working_Language( ASK );
   switch ( iMessage)
   {
      case M_REVIEW_VOTE:
         // fall thru intended!
      case M_SELECTLANG3:
      {
         iTextY = 325;   // button text holds on a single line, center it
         break;
      }
      default:                      // others
      {
         iTextY = 295;   // button text needs 3 lines
         break;
      }
   } // eo switch ( ElectionId)

   Display_Strings( iCancels, iLang, iLeftX + ( Button.Width / 2), iTextY, 30);
   Display_Strings( iConfirms, iLang, iRightX + ( Button.Width / 2), iTextY, 30);

   // wait for the electors response
   for (;;)
   {
      Get_Pen( & iPenCol, & iPenRow);
      if ( Pen_In_Window( iPenCol, iPenRow, iLeftX, Button.Position.Row,
                          iLeftX + Button.Width, Button.Position.Row + Button.Height))
           return 0;  // canceled

      if ( Pen_In_Window( iPenCol, iPenRow, iRightX, Button.Position.Row,
                          iRightX + Button.Width, Button.Position.Row + Button.Height))
         return 1;  // confirmed
   }
}
//</code></function>


/*<function id="DisplayElectionName">
<heading>
  <description>
    <en>displays election name on top of info screens.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>cpElectionName</name>
      <description><en>Name of the election</en></description>
    </parameter>
  </parameters>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplayElectionName (
   char* cpElectionName
   )
// ---------------------------------------------------------------------------
{
      int iMarginTop;
      int iCol;
      char* Text[3];

   setcolor( 0);
   setusercharsize( 1, 1, 1, 1);
   settextjustify( CENTER_TEXT, TOP_TEXT);

   // display header
   int n_lines = Split_Text( cpElectionName, Text, 3, 24, CHARS);
   if ( n_lines > 3)
   {
      // the text is too long for the display, so it will be cut
      n_lines = 3;
   }

   if ( n_lines != 0)
   {
      iCol = getmaxx() / 2;
      iMarginTop = textheight( Text[0]) / 2;

      for ( int i = 0; i < n_lines; i++)
      {
         DisplayTextBold(
            CENTER, iCol, iMarginTop + ( textheight( Text[i]) * i), Text [i],
            FALSE);                    // 20040601 avn
         free( Text [i]);
      }
   }
   settextjustify( CENTER_TEXT, TOP_TEXT);

   return;
}
//</code></function>
#endif //ifndef _PREVIEW


/*<function id="DisplayElectionNameSmall">
<heading>
  <description>
    <en>displays election name on candidate screens ( top right).</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>cpElectionName</name>
      <description><en>Name of the election</en></description>
    </parameter>
  </parameters>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplayElectionNameSmall (
   char* cpElectionName
   )
// ---------------------------------------------------------------------------
{
      int iMarginTop;
      int iCol;
      char* Text[3];

   setcolor( 0);
   setusercharsize( 1, 2, 1, 2);
   settextjustify( CENTER_TEXT, TOP_TEXT);

   // display header
   int n_lines = Split_Text( cpElectionName, Text, 3, 24, CHARS);
   if ( n_lines > 3)
   {
      // the text is too long for the display, so it will be cut
      n_lines = 3;
   }

   if ( n_lines != 0)
   {
      iCol = 510;
      iMarginTop = textheight( Text[0]) / 3;

      for ( int i = 0; i < n_lines; i++)
      {
         DisplayTextBold(
            CENTER, iCol, iMarginTop + ( textheight( Text[i]) * i), Text [i],
            TRUE);                     // 20040601 avn
         free( Text [i]);
      }
   }
   settextjustify( CENTER_TEXT, TOP_TEXT);

   return;
}
//</code></function>


/*<function id="DisplayProgramVersion">
<heading>
  <description>
    <en>display program version on screen.</en>
  </description>
  <parameters/>
  </parameters>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
#ifndef _PREVIEW
void DisplayProgramVersion()
// ---------------------------------------------------------------------------
{
   int  iPadding = 7*2;                // border times 2
   char cString[16];

   sprintf( cString, "v%4.1f", BUILD_NUMBER);

   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
   settextjustify( LEFT_TEXT, TOP_TEXT);
   setcolor( BLACK);
   setusercharsize( 1, 2, 1, 2);

   // display version number bottom left
   outtextxy( iPadding,
      getmaxy() - ( iPadding + textheight( cString)), cString);
   return;
}
#endif //ifndef _PREVIEW
//</code></function>


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

