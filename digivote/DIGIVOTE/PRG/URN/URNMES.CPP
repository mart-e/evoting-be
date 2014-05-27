/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="urnmes.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   URNMES.CPP  $</workFile>
<revision>$Revision: 1.10 $</revision>
<workFileTimeStamp>$Modtime:   Oct 28 2004 11:52:00  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Urn/URNMES.CPP-arc  $</archive>
<archiveTimeStamp>$Date: 2008/11/04 14:23:10 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>URNMES.CPP</fileName>
  <creationDate>01/01/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Librairie de fonctions gestion des messages et langues</fr>
    <nl>Functielibrary beheer boodschappen en talen</nl></description>
  <history>
   Revision History
   ################

   01/06/93: revision history starting date
   07/09/93: Previous_Message handling changed, we now use the Prev_M index
             as global variable
             Fixed : the Prev_M was never incremented in Display_Messages
   11/10/93: Display_Strings_URN was adapted so the names of the MASTER and BACKUP
             disks are set dynamically in the messages, to allow multiple
             BACKUP disks.
   17/11/93: The password is converted to uppercase either than lowercase,
             due to a change in the Bell's programs.</history>
 <comment>Functions for displaying messages on the screen and for the languages.</comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <dos.h>
#include <graphics.h>
#include <ctype.h>

#include "..\gen\gentype.h"
#include "..\gen\genlang.h"
#include "..\gen\gencryp.h"
#include "..\gen\gencard.h"
#include "..\gen\gendata.h"

#include "..\urn\urnmes.h"

#include "..\sha1\hashapi.h"
//</includes>
//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#ifndef _RECOUNT
   #define PROGRAM_TITLE   "URNE"
#else
   #define PROGRAM_TITLE   "Urne"
#endif

const int MAX_MESSAGES  = 18;   // maximum number of message we can retain
const int SPACE_BETWEEN = 40;   // space between the messages for 2 languages

//</defines>

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static void Draw_Button( int x_min, int y_min, int x_max, int y_max);
static void Display_Strings( int M_Index, int Language, int & Row );
static void Reverse_Line( int Row);
//stefG : removeX25 static void Display_Phone( int language, int & row );
static void Display_Counter( int cx, int line, int counter);
static void Clear_Output();
static void Get_Phone_Number( char * Phone_Number);
static BOOL Get_Password( char * pass, BOOL permit_escape, int len,
                           int pos_x, int y, int height, int width, int groupSize);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern char Floppy_MASTER[];
extern char Floppy_BACKUP[];
extern INTEGRITY_STATE Flag_Integrity;
extern struct ElectionMain Root;
extern int TAB_C_Formatted []; //STEF:// NEW counters array (cards formatted)
extern int TAB_C_InBAC[];      //STEF:// NEW counters array (cards in bac)
extern BOOL Background_URN;    //STEF:// FLAG display background
extern BOOL End_Of_Program;
extern SCREEN_TYPE Glob_Screen_Type;
extern int NbrOfCounters;
extern ELECTOR_TYPE ElectorTypesUsed[16];

extern MODE_DISP modedisp;     //STEF:// NEW display mode (summay counters/one counter)
extern IndexCountElector CurrentTypeElector; //STEF:// NEW type elector
extern int MsgElector;                       //STEF:// NEW index message to display
extern int CodeSituation;                    //stefG

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
static int Prev_M = 0;
int Previous_Messages [MAX_MESSAGES] = { 0,0,0,
                                         0,0,0,
                                         0,0,0,
                                         0,0,0,
                                         0,0,0,
                                         0,0,0
                                       };
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

/*<function id="Draw_Button_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description><en>
    draws a button with shadow effect</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x_min</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y_min</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>x_max</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y_max</name></parameter>
  </parameters>
  <usage>Draw_Button_URN( x_min, y_min, x_max, y_max );</usage>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Draw_Button ( int x_min, int y_min, int x_max, int y_max)
// ---------------------------------------------------------------------------
{
   setfillstyle( SOLID_FILL, LIGHTGRAY );      //STEF: draw button shadow first
   bar( x_min+2 , y_min+2, x_max+2, y_max+2 ); //STEF: upper left lower right corners

   setfillstyle( SOLID_FILL, WHITE );          //STEF: draw button
   bar( x_min, y_min, x_max, y_max );          //STEF:

   setcolor( BLACK );                         //STEF:
   rectangle( x_min +2,  y_min +2,  x_max -2,  y_max -2 ); //STEF:
}
//</code></function>


/*<function id="Display_Strings_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description>
    <en>displays strings in line</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Mnemo</name>
      <description><en>mnemonic of the text to display</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Language</name>
      <description><en>the column of the text in the message array (this
      corresponds to the language)</en></description></parameter>
    <parameter type="arg">
      <datatype>int&</datatype><name>row</name>
      <description><en>the starting line where to display the text, the
      value is updated by the function</en></description></parameter>
    <parameter type="global">
      <datatype>sMSG</datatype><name>MSG_GEN[MAX_MSG_GEN]</name>
      <description><en>the message array</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype><name>Floppy_MASTER[]</name>
      <description><en>the label of the MASTER disk</en></description>
    </parameter>
    <parameter type="global">
      <datatype scope="extern">char</datatype><name>Floppy_BACKUP[]</name>
      <description><en>the label of the BACKUP disk</en></description>
    </parameter>
  </parameters>
  <usage>Display_Strings( M_WAIT,Working_Language(ASK), row);</usage>
  <checks>none, the mnemo value is supposed to be possible</checks>
  <comment>
    This function displays the message that corresponds to the mnemonic in
    rows.

    When the message contains the "®®®®®®" string it is replaced by
    the Floppy_MASTER name, if it is "¯¯¯¯¯¯" it is replaced by
    the Floppy_BACKUP name. These names must always begin with the
    drive letter and ':' as in "a:MASTER".

    The message array is a 2 dimensional array. There is an entry for every
    mnemonic and this entry contains the text of the message for each of the
    supported languages. A message can be a single line message or a multi-line
    message using \n as the line separator, blank lines are represented by a
    space.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Display_Strings ( int Mnemo, int Language, int & Row )
// ---------------------------------------------------------------------------
{

       char cMess[350];         // holds the message text (cfr. M_DEMO_MENU_DISPLAY)
       char cToken[] = "\n";    // holds the line delimiter
       char* cpLine;
       int Height;              // the text height
       char* ptr;

    Height = textheight( "³" );

    // we need a local copy of the message text because strtok() will
    // replace the delimiters with null bytes
    strcpy( cMess, Get_Text( Mnemo, Language));

    // replace placeholders with floppy labels (?)
    if ( ( ptr = strchr( cMess, 174)) != NULL)
    {
       memcpy( ptr, Floppy_MASTER + 2, strlen( Floppy_MASTER) - 2);
    }
    if ( ( ptr = strchr( cMess, 175)) != NULL)
    {
       memcpy( ptr, Floppy_BACKUP + 2, strlen( Floppy_BACKUP) - 2);
    }

    // do we have delimiters in the message?
    cpLine = strtok( cMess, cToken);

    if ( cpLine)    // yep!
    {
        do
        {
            // display part of the multi-line message
               Reverse_Line( Row);
               outtextxy( 10, Row - 10, cpLine);
               Row += Height + 10;
            // get next part
            cpLine = strtok( NULL, cToken);
        } while ( cpLine);
    }
    else            // nop!
    {
        // display the single line message
        Reverse_Line( Row);
        outtextxy( 10, Row - 10, cpLine);
        Row += Height + 10;
    }

    return;
}
//</code></function>


/*<function id="Reverse_Line">
<heading>
  <description><en>
    draws a line in reverse video (white)</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Row</name>
      <description><en>the bottom coordinate of the line to reverse</en>
      </description>
    </parameter>
  </parameters>
  <usage>Reverse_Line( Row);</usage>
  <comment>
   The reverse line denomination is not completely correct because
   this is only the final visual effect. In fact we draw a
   rectangular in white color and we display the text in black to
   get a reverse color effect.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Reverse_Line ( int Row)
// ---------------------------------------------------------------------------
{
   setfillstyle( SOLID_FILL, WHITE );
   bar( 0, Row, getmaxx(), Row - textheight("³") - 10 );
   setcolor( BLACK );
}
//</code></function>


/*<function id="Restore_Messages">
<heading>
  <description><en>
    restores the message that was displayed just before the
    the error message arrives</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Previous_Messages [MAX_MESSAGES]</name>
      <description><en>the messages that were displayed</en></description>
    </parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Prev_M</name>
      <description><en>the number of messages in the Previous_Message array</en>
      </description>
    </parameter>
  </parameters>
  <usage>Restore_Message();</usage>
  <comment>
   This function was written, because it was impossible to save
   a screen to memory. When an error message arrives, we need to
   know what was the previously displayed message, because when
   the error is corrected it must be redisplayed. Each time a
   message or the background is displayed, the message number
   is stored in the Previous_Message array.
   This functions extract the message from the array and redisplays
   them, according to the current active language.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Restore_Messages ( void )
// ---------------------------------------------------------------------------
{
    Clear_Output();

    switch( Glob_Screen_Type )
    {
        case ONE_COUNTER:
            ScreenOneCounter();
            break;

        case ALL_COUNTERS:
            ScreenAllCounters();
            break;

        case NO_COUNTER:

            settextjustify( LEFT_TEXT, BOTTOM_TEXT );
            settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0 );
            setusercharsize( 2, 3, 2, 3 );

            int row = 150;

            for( int i = 0; i < Prev_M; ++i)
            {
                if( Previous_Messages [i] == 0)       // protection
                    return;

                if( Working_Language( ASK) != NONE )  // one language
                    Display_Strings( Previous_Messages [i],
                                     Working_Language( ASK),
                                     row );

                else                                 // two languages
                {
                    Display_Strings( Previous_Messages [i],
                                     Root.Language [0], row );
                    row += SPACE_BETWEEN;
                    Display_Strings( Previous_Messages [i],
                                     Root.Language [1], row );
                }
            }
            break;
    }
}
//</code></function>


/*<function id="Display_Message_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description><en>process the display of messages</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>the new row position</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the mnemonic of the message to display</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Confirm</name>
      <description><en>flag indicating if message needs a confirmation</en>
      </description>
      <defaultValue>FALSE</defaultValue></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Append</name>
      <description>
        <en>flag indicating if the text must be appended to the currently
        displayed text</en></description>
      <defaultValue>FALSE</defaultValue></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Exception</name>
      <description><en>flag indicating if the message must be retained for
      redisplay after an error</en></description>
      <defaultValue>FALSE</defaultValue></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Previous_Messages [MAX_MESSAGES]</name>
      <description>
        <en>the array to retain the messages currently displayed</en>
      </description></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype>
      <name>Root</name>
      <description><en>the structure with the used languages</en>
      </description></parameter>
    <parameter type="global">
      <datatype scope="static">int</datatype><name>Prev_M</name>
      <description>
        <en>the number of messages in the Previous_Message array</en>
      </description></parameter>
  </parameters>
  <usage>Display_Message_URN( M_WAIT);</usage>
  <checks>the Prev_M index is checked for its limit value</checks>
  <comment>
   If the message index corresponds to the M_PHONE mnemonic, then
   we call the Display_Phone function either than the normal code.
   We must call it from display message, because it needs the
   same functionnalities than Display_Message_URN but with the extra
   capability of displaying a phone number.
   When two languages are active, we display them both, separated
   by SPACE_BETWEEN. The Root.Language [0] is displayed first and
   then the Root.Language [1] below it.
   If the Append flag is not specified, then we reset the current
   row position to the first displayable line erasing what was
   displayed previously.
   If the Confirm flag is set then we display an extra line (for
   each language) asking the user to confirm by pressing the
   RETURN key.
   If the Exception flag is specified, then the mnemonic is not
   retained in the Previous_Message array, this will be the case
   when we display an error message. Of course we want to redisplay
   the message before the error and not the error message itself.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Display_Message ( int M_Index, BOOL Confirm, BOOL Append, BOOL Exception)
// ---------------------------------------------------------------------------
{
    static int Row =  150;     // this value must be retained

    if( ! Append )
    {
        if( ! Exception)
        {
            Prev_M = 0;
            for( int i = 0; i < MAX_MESSAGES; ++i)
                Previous_Messages [i] = 0;
        }

        Row = 150;
        Clear_Output();
    }

    if( ! Exception)
    {
        if( Prev_M >= MAX_MESSAGES )           // try to save more messages
            Error_Exit( E_OUT_OF_MEMORY);      // than possible
            Previous_Messages [Prev_M++] = M_Index;
            Glob_Screen_Type = NO_COUNTER;     // used by restore_messages
                                               // to distinguish between the different types of screen
    }

    settextjustify( LEFT_TEXT, BOTTOM_TEXT);
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
    setusercharsize( 2, 3, 2, 3 );

    if( Working_Language( ASK) != NONE )  // one language
        Display_Strings( M_Index, Working_Language( ASK), Row );

    else
    {
        Display_Strings( M_Index, Root.Language [0], Row );
        Row += SPACE_BETWEEN;
        Display_Strings( M_Index, Root.Language [1], Row );
    }


    if( Confirm )
    {
        Display_Message( M_ENTER, NO, YES, Exception);

        while( Test_Key_Pressed( RETURN) == FALSE);
    }

    return Row;
}
//</code></function>


/*<function id="Error_Message_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description><en>displays an error message</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the mnemonic of the error message</en></description>
    </parameter>
  </parameters>
  <usage>Error_Message_URN (M_ERROR);</usage>
  <comment>
   This function is only there to distinguish common messages from
   error messages. It calls Display_Message_URN with the default values.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Error_Message ( int M_Index)
// ---------------------------------------------------------------------------
{
   Display_Message( M_Index);
}
//</code></function>


/*<function id="Ask_User_For_Language_URN"><!-- _URN avoids duplicate id's -->
<heading>
  <description><en>ask the user to determine the language he wants to use</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>the column number corresponding to the specified language
      in the message array</en></description></parameter>
   <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="extern">struct ElectionMain</datatype><name>Root</name>
      <description><en>the structure with the used languages</en></description>
    </parameter>
  </parameters>
  <usage>Working_Language( Ask_User_For_Language_URN());</usage>
  <comment>The response character depends on the used languages.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Ask_User_For_Language ( void)
// ---------------------------------------------------------------------------
{
   unsigned int Inkey;
   unsigned int Mask [2] = { 0, 0} ;

   // ask the user to choose between the two languages
   Display_Message( M_LANG_INP1, 0, 0);

   switch( Root.Language [0])
   {
      case 0 : Mask [0] = 'F'; break;    // French
      case 1 : Mask [0] = 'N'; break;    // Dutch
      case 2 : Mask [0] = 'D'; break;    // German
   }

   switch( Root.Language [1])
   {
      case 0 : Mask [1] = 'F'; break;    // French
      case 1 : Mask [1] = 'N'; break;    // Dutch
      case 2 : Mask [1] = 'D'; break;    // German
   }

   while( kbhit() ) getch();        // flush input buffer

   // wait until the user has given a correct response
   for(;;)
   {
      Inkey = toupper( Get_Key());

      if( Inkey == Mask [0] )    return Root.Language [0];
      if( Inkey == Mask [1] )    return Root.Language [1];
   }
}
//</code></function>


/*<function id="Ask_Question">
<heading>
  <description><en>
    asks a question which can only be answered by YES or NO</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = the answer was YES,
      FALSE = the answer was NO</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the mnemonic of the question message</en></description>
    </parameter>
  </parameters>
  <usage>if( Ask_Question( M_RESTART_MAV))</usage>
  <checks>we check if the working language is already specified</checks>
  <comment>
   The responses depends on the used language because YES and NO
   are translated for each language.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL Ask_Question ( int M_Index)
// ---------------------------------------------------------------------------
{
   unsigned int Inkey;
   unsigned int Mask [2] = { 0, 0} ;               // admitted responses mask

   Display_Message( M_Index, 0, 0);

   switch( Working_Language( ASK))
   {
      case 0 : Mask [0] = 'O'; Mask [1] = 'N'; break;    // French Oui/Non
      case 1 : Mask [0] = 'J'; Mask [1] = 'N'; break;    // Dutch  Ja /Nee
      case 2 : Mask [0] = 'J'; Mask [1] = 'N'; break;    // German Ja /Nein

      // WorkingLanguage not defined or not supported
      default: Working_Language(0);
               Error_Exit( M_BAD_LANGUAGE);
   }

   while( kbhit() ) getch();                        // flush output buffer

   // wait for a correct input
   for(;;)
   {
      Inkey = toupper( Get_Key());

      if( Inkey == Mask[0])
        return TRUE;
      if( Inkey == Mask[1])
        return FALSE;
   }
}
//</code></function>


/*<function id="Get_Password">
<heading>
  <description><en>
    Process the password input</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">BOOL</datatype>
      <description><en></en></description></parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>pass</name>
      <description>
        <en>the string that will contain the password</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>permit_escape</name>
      <description><en>indicates if user may escape from screen</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>len</name>
      <description><en>the number of characters to give</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>pos_x</name>
      <description><en>starting position of text</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y</name>
      <description><en>bottom coordinate of window</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>height</name>
      <description><en>font height</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>width</name>
      <description><en>font width</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>groupSize</name>
      <description><en>character grouping size</en></description>
    </parameter>
  </parameters>
  <usage>if( Get_Password( pass, TRUE, len, pos_x, y, height, width, groupSize) == FALSE)</usage>
  <comment>
   This function gets the password from the user, each admitted
   character is represented by a '*'. It is possible to perform a
   backspace on the already given characters, the '*' is then
   removed. The last given character can not be corrected because
   typing the last character means that the password is complete.
   The allowed characters for the password are a-z,A-Z and 0-9.
   The permit_escape flag can permit the user to escape at each
   moment, the interpretation of the escape is left on the upper
   functions, but it means that the password was NOT given.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Get_Password ( char * pass, BOOL permit_escape, int len,
                           int pos_x, int y, int height, int width, int groupSize)
// ---------------------------------------------------------------------------
{
   int n_chars = 0;
   unsigned char c;

   while( kbhit() ) getch();        // flush output buffer

   while( n_chars != len)
   {
      // read key
      c = toupper( Get_Key() );     // all chars are converted to uppercase

      // escape key pressed?
      if( c == ESCAPE && permit_escape)
          return FALSE;

      // check key, only [a-z], [A-Z], [0-9] characters are allowed
      if( n_chars < len && ( (c >  96 && c < 123) ||        // a-z
                             (c >  63 && c <  91) ||        // A-Z
                             isdigit( c)) )                 // 0-9
      {

         pass [n_chars++] = c;

         // display a '*' to represent the character
         outtextxy( pos_x, y - (height/2) - 7, "*" );

         // move 'cursor' to next character position
         if (n_chars % groupSize) {
            pos_x += width;
         }
         else {
            pos_x += width*2;   // skip separator
         }
      }

      else if( c == BACKSPACE && n_chars > 0)
      {

         // move 'cursor' to prev character
         if (n_chars % groupSize) {
            pos_x -= width;
         }
         else {
            pos_x -= width*2;   // skip separator
         }

         // overwrite the '*' by inverting the color
         setcolor( WHITE);
         outtextxy( pos_x, y - (height/2) - 7, "*");
         setcolor( BLACK);

         pass [--n_chars] = 0;

      }
   } // endwhile

   pass[ n_chars] = 0;

   return TRUE;
}
//</code></function>


/*<function id="Ask_Password">
<heading>
  <description><en>
    interface to get and check the password from user input</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>TRUE  = password was given and is correct
        FALSE = password was not given or was not correct</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Nr_Mes</name>
      <description>
        <en>the mnemonic of the message to display while asking the password</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>y</name>
      <description>
        <en>where to position the password input window on the screen</en>
      </description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>permit_escape</name>
      <description><en>indicates if user may escape from screen</en>
      </description>
      <defaultValue>FALSE</defaultValue></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>exception</name>
      <description><en>used by the Display_Message_URN function</en>
      </description>
      <defaultValue>FALSE</defaultValue></parameter>
  </parameters>
  <usage>if( Ask_Password( M_MENU_CALL, 270 , TRUE) != FALSE)</usage>
  <comment>
   This function displays a message asking the user to type the
   password in, draws a box for the password input, checks the
   given password and let the user retry if the given password
   was not good.
   The position of the password window is not computed dynamicaly
   because we don't know the message length.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Ask_Password ( int Nr_Mes, int y, BOOL permit_escape, int exception)
// ---------------------------------------------------------------------------
{
    int given = FALSE,
        trys = 0,
        pos_x,
        x = getmaxx()/2,                            // set middle of screen in pixels
        len = AESKEYLEN,
        max_trys = 3,                               // set maximum password retry's
        groupSize = 4;                              // a group contains 4 characters
    AESKEY pass;

    // initialyze some values depending on text and screen characteristics
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
    setusercharsize( 1, 1, 1, 1);

    int height = textheight( "|");                  // set height of current font in pixels
    int width  = textwidth( "-");
    int space  = textwidth( "--") - (2 * width);    // set pixels between characters

    // set dimensions of password window
    int win_width  =                                // adjacent characters:
        ( ((len - 1) + (AESKEYLEN/groupSize - 1))   //  (all except last char) + (group separators)
        * (width + space) )
        + (3 * width);                              // non-adjacent: space before & after + last char
    int win_height = 2 * height;

    // set coordinates of password window
    int x_min = x - (win_width / 2);
    int x_max = x + (win_width / 2);
    int y_min = y - (win_height / 2);
    int y_max = y + (win_height / 2);

   // process the max_trys password inputs
    while( given == FALSE && trys < max_trys)
    {
        // prompt user for password
        Display_Message( Nr_Mes, 0, 0, exception);

        settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
        settextjustify( LEFT_TEXT, TOP_TEXT);
        setusercharsize( 1, 1, 1, 1);

        // draw the password window
        Draw_Button( x_min, y_min, x_max, y_max );
        pos_x = x_min + width;                                    // set begin of text area

        // display placeholders
        for ( int i = 0; i < (AESKEYLEN/groupSize); i++) {        // for each group
            for ( int j = 0; j < groupSize; j++) {                // for each character in a group
                outtextxy( pos_x                                  // begin of text area
                           + (i * (groupSize+1) * (width+space))  // + size of previous groups
                           + (j * (width+space)),                 // + size of previous characters
                           y - (height/2), "-");
            }
        }

        // get the password from the user
        trys++;
        memset( pass.data, 0, AESKEYLEN+1);                       // clear pasword buffer
        if ( Get_Password( pass.data, permit_escape, len, pos_x, y, height, (width+space), groupSize)
            == FALSE)
            return FALSE;

        // hide the password window before checking the password
        setfillstyle( SOLID_FILL, BLACK );
        bar( x_min, y_min, x_max+2, y_max+2 );                    // +2 for the shadow

        // check the password
        if( Verify_Password( pass.data) == SUCCEEDED)
            given = TRUE;

        // give another chance to give the correct password
        else if( trys < max_trys)
        {
            int row = 450;
            settextjustify( LEFT_TEXT, BOTTOM_TEXT);
            setusercharsize( 2, 3, 2, 3 );
            Display_Strings( M_PSWD_BAD, Working_Language( ASK), row);
            sleep(2);
            setfillstyle( SOLID_FILL, BLACK );
            bar( 0, 300 - textheight("³") - 10, getmaxx(), getmaxy() );
            settextjustify( LEFT_TEXT, TOP_TEXT);
            setusercharsize( 1, 1, 1, 1);
        }
    }

    return given;
}
//</code></function>


/*<function id="Display_Title_URN">
<heading>
  <description><en>
    displays the program title</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Display_Title_URN</usage>
  <comment>
   The program title depends on the current running program.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Title_URN ( void) //STEF:
// ---------------------------------------------------------------------------
{
   int mid_x = getmaxx()/2;

   clearviewport();

   // display version number top right corner
   gotoxy( 76, 1); printf( "v%4.1f", BUILD_NUMBER);

   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0 );
   settextjustify( CENTER_TEXT, BOTTOM_TEXT );
   setusercharsize( 1, 1, 1, 1 );

   Draw_Button( mid_x - 100, 10, mid_x + 100, 50 );
   outtextxy( mid_x, 40, PROGRAM_TITLE);
}
//</code></function>


/*<function id="Clear_Output">
<heading>
  <description><en>
    clears the screen under the URN title</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Clear_Output ();</usage>
  <comment>
   This function fills the area under the URN title with black to
   clear the screen. '50' is one more than the title window
   bottom coordinate.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void Clear_Output ( void)
// ---------------------------------------------------------------------------
{
    setfillstyle( SOLID_FILL, BLACK);
    if( NbrOfCounters > 4 )
        bar( 0, 54, getmaxx(), getmaxy());
    else
        bar( 0, 71, getmaxx(), getmaxy());
}
//</code></function>


/*<function id="Draw_Message">
<heading>
  <description></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char</datatype><name>text[]</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>cx</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>cy</name></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Draw_Message ( char text[], int cx, int cy )
// ---------------------------------------------------------------------------
{
    int width, height;

    int x_border = 10, y_border = 10;
    width  = textwidth (text);
    height = textheight(text);

    width  = width / 2 + x_border;
    height += y_border;

    Draw_Button( cx - width, cy - 5, cx + width, cy + height);
    outtextxy( cx, cy, text);
}
//</code></function>


/*<function id="ScreenOneCounter">
<heading>
  <description><en>
    </en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void ScreenOneCounter ()
// ---------------------------------------------------------------------------
{
    int x_mid = getmaxx()/2;
    int index;

    Clear_Output();                     // clears the output screen

    #ifndef _RECOUNT
    index = NbrOfCounters ? CurrentTypeElector : Totaux ;
    #else
    index = Totaux;
    #endif

    settextjustify( CENTER_TEXT, TOP_TEXT );
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0 );
    setusercharsize( 1, 1, 1, 1);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    #ifndef _RECOUNT
   // draw the Formatted Cards title
    Draw_Message( Get_Text( ElectorTypesUsed[index].ValidationMessageID,
                            Working_Language(ASK)), x_mid, 130 );
    #endif
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // draw the Card in BAC title
    Draw_Message( Get_Text( ElectorTypesUsed[index].UrnMessageID,
                            Working_Language(ASK)), x_mid, 300 );

    // display the counter values
    setcolor( BLACK);

#ifndef _RECOUNT
    Display_Counter( -1, 195, TAB_C_Formatted[index] );

    if( CurrentTypeElector > 1 )
    {
       DisplayUseraid( Get_Text( M_RETURN_TO_VOTE, Working_Language( ASK)));
    }
#endif
    Display_Counter( -1, 360, TAB_C_InBAC[index]);

    Glob_Screen_Type = ONE_COUNTER;

}
//</code></function>


/*<function id="Draw_Pattern">
<heading>
  <description><en>
    display a 'pattern' : draw messages (counters labels,title)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>MsgTypeElector</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>cx</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>cy</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>deltax</name></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Draw_Pattern ( int MsgTypeElector, int cx,int cy, int deltax )
// ---------------------------------------------------------------------------
{

    Draw_Message( Get_Text( MsgTypeElector, Working_Language(ASK)),
                  cx, cy );

    Draw_Message( Get_Text( M_CARDS_READY, Working_Language(ASK)),
                  cx - deltax , cy + 38 );

    Draw_Message( Get_Text( M_CARDS_IN_BAC, Working_Language(ASK)),
                  cx + deltax , cy + 38 );

}
//</code></function>


/*<function id="ScreenAllCounters">
<heading>
  <description><en>
    </en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
#ifndef _RECOUNT
// ---------------------------------------------------------------------------
void ScreenAllCounters ()
// ---------------------------------------------------------------------------
{

        settextjustify( CENTER_TEXT, TOP_TEXT );
        settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0 );

        setcolor(15);

        int x_size = getmaxx();
        int start_y;
        int between_y;
        int start_totals;
        int start_pat;
        int start_totals_title;
        int x_center;
        int Column;
        int Row;
        int NbrOfColumns;

        Clear_Output();


        switch( NbrOfCounters )
        {
            case 0 :
                ScreenOneCounter();
                Glob_Screen_Type = ALL_COUNTERS;
                return;
            case 1 :
            case 2 :
            case 3 :
            case 4 :
                start_y = 170;
                between_y = 120;
                start_totals = 420;
                start_totals_title = 380;
                line (10, 340, 640,340);
                start_pat = 100;
                break;

            case 5 :
            case 6 :
                start_y = 130;
                between_y = 110 - 5; // 5 pixels less on 3 rows, hence we win 15 pixels for totals
                start_totals = 436 - 15;
                start_totals_title = 407 - 15;
                line (0, 395 - 15, 639, 395 - 15);
                start_pat = 70;
                break;

            case 7 :
            case 8 :
                break;
        }

        for( int i = 1; i <= NbrOfCounters; i++)
        {
            Column = ( i - 1 ) % 2 + 1;
            NbrOfColumns = ( i == NbrOfCounters ) ? ( i - 1 ) % 2 + 1 : 2;
            Row = ( i - 1 ) / 2;

            x_center = x_size / ( 2 * NbrOfColumns )
                       + x_size / 8 * ( Column - 1 ) * 4;

            if( NbrOfCounters < 5 )
                setusercharsize( 2, 3, 1, 2);

            else
                setusercharsize( 1, 2, 1, 2 );

            Draw_Pattern( ElectorTypesUsed[i].AllCountersTitleID,
                          x_center,
                          start_pat + Row * between_y,
                          x_size / 8 );

            Display_Counter( x_center - x_size / 8,
                             Row * between_y + start_y,
                             TAB_C_Formatted[i] );
            Display_Counter( x_center + x_size / 8,
                             Row * between_y + start_y,
                             TAB_C_InBAC[i] );
        }

        setusercharsize(2, 3, 2, 3);

        Draw_Message( Get_Text( M_CARDS_READY, Working_Language(ASK)),
                      x_size / 3,
                      start_totals_title );

        Draw_Message( Get_Text( M_CARDS_IN_BAC, Working_Language(ASK)),
                      x_size * 2 / 3,
                      start_totals_title );


        Display_Counter( ( x_size / 3  ),
                         start_totals,
                         TAB_C_Formatted[Totaux] );

        Display_Counter( ( x_size * 2 / 3 ),
                         start_totals,
                         TAB_C_InBAC[Totaux] );

   if ( !End_Of_Program)
   {
      DisplayUseraid( Get_Text( M_VALIDATE_OTHER_CARD, Working_Language( ASK)));
   }

   Glob_Screen_Type = ALL_COUNTERS;

   return;
}
#else
// ---------------------------------------------------------------------------
void ScreenAllCounters ()
// ---------------------------------------------------------------------------
{
    ScreenOneCounter();
    setcolor( BLACK);
    Glob_Screen_Type = ALL_COUNTERS;
}
#endif
//</code></function>


/*<function id="Display_Counter">
<heading>
  <description><en>
    draws and fill the window where to put the counter in</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>cx</name>
      <description><en>(New Parameter -&gt; DV II)</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>line</name>
      <description><en>the line where to draw the counter window</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>counter</name>
      <description><en>the value to display in the window</en></description></parameter>
    <parameter type="global">
      <datatype scope="extern">INTEGRITY_STATE</datatype><name>Flag_Integrity</name>
      <description><en>Integrity state</en></description></parameter>
  </parameters>
  <usage>Display_Counter( cx, 360, counter);</usage>
  <comment>
   According to the integrity state, the counters are or are not
   displayed. It makes no sense to continue displaying counters
   while they are not correct. If the flag integrity is CORRUPTED
   we replace the normal counter by 'XXXX'.
  </comment>
</heading>
<code>
*/
// ---------------------------------------------------------------------------
void Display_Counter ( int cx, int line, int counter)
// ---------------------------------------------------------------------------
{
    char text[10];

    settextjustify( CENTER_TEXT, TOP_TEXT);
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
    if( cx == -1 )
    {
        setusercharsize( 2, 1, 2, 1);
        setfillstyle( SOLID_FILL, BLACK );       // clear previous display
        bar( 0, line, getmaxx(), line +75);
    }
    else
        setusercharsize( 1, 1, 1, 1);

    if( Flag_Integrity == CORRUPTED)
        strcpy( text, "xxxx");
    else
        sprintf( text, "%d", counter);

    setcolor( WHITE);
    if( cx == -1 )
        outtextxy( getmaxx()/2, line, text);
    else
        outtextxy( cx, line, text);
}
//</code></function>


/*<function id="Test_Key_Pressed">
<heading>
  <description><en>
    check if a definite key was pressed</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  = the key was effectively pressed
        FALSE = the key was not pressed or another key was</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Key</name>
      <description><en>the key we want to check</en></description></parameter>
  </parameters>
<usage>
while( Test_Key_Pressed( RETURN) == FALSE);</usage>
<comment>
This function is to be used in a loop to check if a definite
key was pressed.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL Test_Key_Pressed ( int Key)
// ---------------------------------------------------------------------------
{
   if( kbhit() && Get_Key() == Key)
      return TRUE;

   return FALSE;
}
//</code></function>


/*<function id="Get_Key">
<heading>
  <description><en>
    gets a key from the keyboard and also process extended keys</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>the scan code of the pressed key</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
<usage>
if( kbhit() &amp;&amp; Get_Key() == key)</usage>
<comment>
The difference between this function and getch() is that in
case of an extended code, there is still something in the
keyboard buffer with getch(). Get_Key process this extra code
so you willn't have it the next time you invoke Get_Key.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Get_Key ( void)
// ---------------------------------------------------------------------------
{
   int c = getch();

   if( c)
      return c;                             // the key was not an extended one

   c = getch();                                     // read the extended value

   return 0;
}
//</code></function>


/*<function id="Get_ExtKey">
<heading>
  <description><en>
    get a key pressed (extended key also)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype></parameter>
    <parameter type="global">
      <datatype>BOOL*</datatype><name>isExt</name></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Get_ExtKey ( BOOL *isExt)
// ---------------------------------------------------------------------------
{
   int c = getch();

   if( ! c)
   {
    c = getch();                                     // read the extended value
    *isExt=TRUE;
    return c;
   }

   *isExt=FALSE;
   return c;
}
//</code></function>


/*<function id="Num_Lock_On">
<heading>
  <description><en>put the numlock key on</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>Num_Lock_On ();</usage>
  <comment>
   Sets the keyborad numlock flag on. Found in Helppc under the
   KB flags topic.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Num_Lock_On ( void)
// ---------------------------------------------------------------------------
{
   unsigned char far * keyb = (unsigned char *) MK_FP( 0x40, 0x17);

   * keyb |= 32;
}
//</code></function>


/*<function id="DisplayUseraid">
<heading>
  <description>
    <en>Display text on bottom of the URN screen</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>char*</datatype></parameter>
  </parameters>
  <usage/>
  <comment>The text is ment to help or guide the user.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplayUseraid (
   char* pString                       // text to display
   )
// ---------------------------------------------------------------------------
{

   // 20040319 avn - default font doesn't show with SVGA16.OBJ :-((
   settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0 );
   settextjustify( CENTER_TEXT, CENTER_TEXT );
   setusercharsize( 1, 2, 1, 2);

   setcolor( WHITE);
   outtextxy( getmaxx()/2, getmaxy() - textheight( pString), pString);
   setcolor( BLACK);

   return;
}
//</code></function>

void substr ( char* dest, char* src, int position, int length)
{
	dest[0] = '\0';
	strncat( dest, ( src + position), length);
}

/*<function id="DisplayDigest">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplayDigest (
	int aRow,				// start display at pixel row
	char* aDigest,			// the digest to display
	int aGroupBy			// 4 or 8
	)
// ---------------------------------------------------------------------------
{
	const int group = 5;

	int x, y, s, m, blockStart;
	char block[5 +1];

    settextjustify( LEFT_TEXT, BOTTOM_TEXT);
    settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
    //setusercharsize( 2, 3, 2, 3 );
    setusercharsize( 1, 1, 1, 1 );
    setcolor( BLACK);

	s = (( aGroupBy == 4) ? 2 : 1);
	m = ( getmaxx() - ( textwidth("A") * ( group + s) * aGroupBy)) / 2;

	x = m;
	y = aRow;

	for ( blockStart = 0; blockStart < strlen( aDigest); blockStart += group)
	{
	   if ( blockStart == group * aGroupBy)
	   {
			y += textheight("A"); 		// line feed
			x = m; 	// left margin
	   }

	   substr( block, aDigest, blockStart, group);
	   outtextxy( x, y, block);

	   x += textwidth("A") * (group + s); // next block position

	}

    return;
}
//</code></function>


/*<function id="DisplayCountersForEver">
<heading>
  <description>Display counter screen and digest alternately</description>
  <parameters/>
  <comment>Final screens after closing procedure</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void DisplayCountersForEver ()
// ---------------------------------------------------------------------------
{
	// show all counters and wait for enter
	ScreenAllCounters();
  	DisplayUseraid( Get_Text( M_URN_END4, Working_Language( ASK)));

    for( ;;);	// loop forever
}
//</code></function>
//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
