/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavlight.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVLIGHT.CPP  $</workFile>
<revision>$Revision: 1.7 $</revision>
<workFileTimeStamp>$Modtime:   Nov 30 2004 17:03:12  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVLIGHT.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2007/11/22 11:01:24 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVLIGHT.CPP</fileName>
  <creationDate>20/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Fonctions crayon optique et alarme</fr>
    <nl>Lichtpen- en alarmfuncties</nl></description>
  <history>
   01/06/93: revision history starting date
   16/07/93: card reader trace printing is added (activated with TRACE)
   20/10/93: Make_Noise was modified to allow more flexibility
   20/10/93: The card reader is immediately disabled when the Red_Light_On or
             the Green_Light_On is called.
  </history>
  <comment>Functions for the lightpen and the alarm box</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <dos.h>
#include <setjmp.h>
#include <conio.h>
#include <time.h>

#include "..\gen\gentype.h"
#include "..\gen\gencard.h"
#include "..\gen\card.h"

#include "..\mav\mavasm.h"
#include "..\mav\mavlight.h"
#include "..\mav\mavmes.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---
typedef enum { SOUND1, SOUND2, NO_ALARMBOX } SOUND_TYPE;

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
#ifndef _PREVIEW
   static void Make_Noise( SOUND_TYPE Type);
   static BOOL Reset_Pressed();
//   static void connectAlarmBox();
#endif
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern Select_Reader * Intern_Reader;       // the intern reader definition
extern int PenCursor ;                    // if 1 pen cursor will be activated
extern JUMP_POINT Jump;          // indicates if jump point is already defined
extern jmp_buf JMP_Restart_Loop;                     // the jump point himself

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
int Box_Address = * (int far *)((long) 0x400008);      // the box base address
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Make_Noise">
<heading>
  <description><en>
    emits a sound</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>SOUND_TYPE</datatype><name>Type</name>
      <description><en>the type of sound to emit</en></description></parameter>
  </parameters>
  <usage>Make_Noise( SOUND1);</usage>
  <comment>
    This functions is intended to be used in a loop.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
static void Make_Noise ( SOUND_TYPE Type)
// ---------------------------------------------------------------------------
{
   switch( Type)
   {
      case SOUND1:
      {
         sound( 3000);
         delay( 100);
         sound( 2000);
         delay( 100);
         break;
      }
      case NO_ALARMBOX:
      {
         sound( 440);
         delay( 200);
         break;
      }
      default:
      {
         sound( 2000);
         delay( 200);
         break;
      }
   }

   nosound();                                       //         reset the sound
   delay( 100);                                     //  continue the sound gap
}
#endif
//</code></function>


/*<function id="Reset_Pressed">
<heading>
  <description><en>
    accesses the alarm box red button to check if it is pressed</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  = red button was pressed ( OR no alarmbox present!)
        FALSE = red button was not pressed</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Box_Address</name>
      <description><en>the box base address</en></description></parameter>
  </parameters>
  <usage>while( ! Reset_Pressed())</usage>
  <comment>
   if _ALARM was defined during compile time, the red button will
   be replaced by any keyboard key.
   /!\ This function returns TRUE whether the button is pressed OR the alarmbox
   is not present! Therefore the alarmbox's presence must be tested first.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
static BOOL Reset_Pressed ()
// ---------------------------------------------------------------------------
{

#ifndef _ALARM
   if( inp( Box_Address + 1) & 0x08 )              // use the normal alarm box
#else
   if( kbhit())                                    // use the keyboard instead
#endif

      return TRUE;

   return FALSE;
}
#endif
//</code></function>


/*<function id="Get_Pen">
<heading>
  <description><en>
    waits for a lightpen input and returns the coordinates of the
    point that was hit on the screen</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int*</datatype><name>Col</name>
      <description><en>will contain the lightpen column position</en></description></parameter>
    <parameter type="arg">
      <datatype>int*</datatype><name>Row</name>
      <description><en>will contain the lightpen row position</en></description></parameter>
  </parameters>
  <usage>Get_Pen( &amp; Col, &amp; Row);</usage>
  <checks>
    if g_pen returns with a non 0 value, this is due to a
    disconnected lightpen; the function goes then in the Red_Light_On
    routine, which may cause a long jump to a restart point</checks>
  <comment>
    During lightpen interaction, there is no arrow present on the
    screen</comment>
</heading>
<code>*/
#ifdef _PREVIEW

#pragma argsused
// ---------------------------------------------------------------------------
void Get_Pen ( int * Col, int * Row)
// ---------------------------------------------------------------------------
{
   // dummy function
}

#else

// ---------------------------------------------------------------------------
void Get_Pen ( int * Col, int * Row)
// ---------------------------------------------------------------------------
{

   if( ! g_pen( 1 /*not used*/, 0 /*not used*/, PenCursor ) )
      Red_Light_On( M_NO_LIGHTPEN, NOT_FATAL);          // lightpen error

   * Col = p_ko();                  // get the latest lightpen column position
   * Row = p_ri();                  // get the latest lightpen row    position
}

#endif
//</code></function>


/*<function id="Pen_In_Window">
<heading>
  <description><en>
    determines if the given lightpen position is inside the given
    window coordinates</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  = Pen is in the specified window
        FALSE = Pen is not in the specified window</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Pen_Col</name>
      <description><en>the lighpen position</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Pen_Row</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Win_X_Min</name>
      <description><en>the window coordinates</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Win_Y_Min</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Win_X_Max</name></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Win_Y_Max</name></parameter>
  </parameters>
  <usage>
    if( Pen_In_Window( Col, Row, 10, 10, 50, 50))</usage>
</heading>
<code>*/
#ifdef _PREVIEW

#pragma argsused
// ---------------------------------------------------------------------------
BOOL Pen_In_Window (
   int Pen_Col, int Pen_Row,
   int Win_X_Min, int Win_Y_Min, int Win_X_Max, int Win_Y_Max)
// ---------------------------------------------------------------------------
{
   return 1;
}

#else

// ---------------------------------------------------------------------------
BOOL Pen_In_Window (
   int Pen_Col, int Pen_Row,
   int Win_X_Min, int Win_Y_Min, int Win_X_Max, int Win_Y_Max)
// ---------------------------------------------------------------------------
{

   return( ( Pen_Col > Win_X_Min &&
             Pen_Col < Win_X_Max &&
             Pen_Row > Win_Y_Min &&
             Pen_Row < Win_Y_Max ) );
}

#endif
//</code></function>


/*<function id="Lightpen_Hit">
<heading>
  <description><en>
    determines if the lightpen hit the screen at the moment we are
    asking the question</en></description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>
        TRUE  = lightpen was hit
        FALSE = lightpen was not hit</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    if( Lightpen_Hit())</usage>
  <checks>
    the function checks if the lightpen is connected by first calling
    the Test_Lightpen function</checks>
  <comment>
    We enforce switch AND light to ensure that clicking happens on the
    screen and not for instance on the desk or the computer...</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
BOOL Lightpen_Hit ()
// ---------------------------------------------------------------------------
{
   Test_Lightpen();                       // check for lightpen's presence

   switch( ( (int) detection() & 3))      // determine the lightpen status
   {
      case 0: return FALSE;               // No light, No switch
      case 1: return FALSE;               // Light, No switch
      case 2: return FALSE;               // No light, Switch
      case 3: return TRUE;                // Light, Switch
   };

   return FALSE;                          // you should never reach this
}
#endif
//</code></function>


/*<function id="Test_Lightpen">
<heading>
  <description><en>
    checks if the lightpen is connected. If the lightpen is not
    connected then the Red_Light_On function is called, which may
    cause a long jump to a restart point</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Test_Lightpen();</usage>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Test_Lightpen ()
// ---------------------------------------------------------------------------
{
   if( ! lightpenconnected() )
      Red_Light_On( M_NO_LIGHTPEN, NOT_FATAL);       // lightpen not connected
}
#endif
//</code></function>


/*[*/
//==============================================================================
//======================= A L A R M   R O U T I N E S ==========================
//==============================================================================
/*]*/

/*[*/
//
//  The alarm box is connected to the parallel printer port and powered by the
//  keyboard connector. Actions on the box are done by accessing the hardware
//  port address; in reading to get a status (Red button) or writing to put the
//  box in another state (re/set the lights). When writing on the parallel port
//  the status remains active until it is reset.
//  During compile time you may define _ALARM to simulate the alarm box
//  presence, the Red button is then replaced by the keyboard, and no light
//  is set.
//
/*]*/


/*<function id="Red_Light_On">
<heading>
  <description><en>
    this function is called when a hardware problem has occured, it
    sets the alarm box red light on and make noise until the Red
    button is pressed; it then normaly jumps to another point</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>an error message index position</en></description></parameter>
    <parameter type="arg">
      <datatype>BOOL</datatype><name>Fatal</name>
      <description><en>
        indicates if it is a fatal error
        default value is FATAL -&gt; no jump
        FATAL     = TRUE
        NOT_FATAL = FALSE</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Box_Address</name>
      <description><en>the box base address</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Intern_Reader</name>
      <description><en>the intern reader structure</en></description></parameter>
    <parameter type="global">
      <datatype>JUMP_POINT</datatype><name>Jump</name>
      <description><en>indicates if a jump point exist</en></description></parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_Restart_Loop</name>
      <description><en>the long jump restart point</en></description></parameter>
  </parameters>
  <usage>Red_Light_On( M_NO_LIGHTPEN, NOT_FATAL);</usage>
  <comment>
   If a fatal error has occured or no jump point exist, we try to
   eject the card (if any) before locking the system by calling a
   eject command without error handling.
   If _ALARM was defined during compile time, the Red light is
   not set.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Red_Light_On ( int M_Index, BOOL Fatal)
// ---------------------------------------------------------------------------
{
   Send_Command( Intern_Reader, DISABLE, NULL);

   ConnectAlarmBox();

   Error_Message( M_Index);                       // display the error message

#ifdef TRACE
   Print_Trace_Buffer(stdout);
#endif

#ifndef _ALARM
   outp( Box_Address, 0x7D);                           // set the red light on
#endif

   while( ! Reset_Pressed())                // wit until Red button is pressed
      Make_Noise( SOUND2);                                       // make noise
   Reset_Lights();                                         // reset the lights

   if( ! Fatal && Jump != NO_JUMP)
      longjmp( JMP_Restart_Loop, 1);  // perform the jump to the restart point

   Send_Command( Intern_Reader, EJECT, NULL);         // try to eject the card
   Error_Exit( M_HALTED);                                   // lock the system
}
#endif
//</code></function>


/*<function id="Green_Light_On">
<heading>
  <description><en>
    this function is called when a manipulation error has occured, it
    sets the alarm box green light on and make noise until the Red
    button is pressed; it then normaly jumps to another point</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>an error message index position</en></description></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Box_Address</name>
      <description><en>the box base address</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype><name>Intern_Reader</name>
      <description><en>the intern reader structure</en></description></parameter>
    <parameter type="global">
      <datatype>JUMP_POINT</datatype><name>Jump</name>
      <description><en>indicates if a jump point exist</en></description></parameter>
    <parameter type="global">
      <datatype>jmp_buf</datatype><name>JMP_Restart_Loop</name>
      <description><en>the long jump restart point</en></description></parameter>
  </parameters>
  <usage>Green_Light_On( M_BAD_CARD);</usage>
  <comment>
   If no jump point exist, we try to eject the card before
   locking the system by calling a Eject command without error
   handling.
   If _ALARM was defined during compile time, the Green light is
   not set.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Green_Light_On ( int M_Index)
// ---------------------------------------------------------------------------
{
   Send_Command( Intern_Reader, DISABLE, NULL);

   ConnectAlarmBox();

   Error_Message( M_Index);                       // display the error message

#ifndef _ALARM
   outp( Box_Address, 0x7E);                         // set the green light on
#endif

   while( ! Reset_Pressed())                // wit until Red button is pressed
      Make_Noise( SOUND1);                                       // make noise
   Reset_Lights();                                         // reset the lights

   if( Jump != NO_JUMP)
      longjmp( JMP_Restart_Loop, 1);  // perform the jump to the restart point

   Send_Command( Intern_Reader, EJECT, NULL);         // try to eject the card
   Error_Exit( M_HALTED);                                   // lock the system
}
#endif
//</code></function>


/*<function id="WaitForPresident">
<heading>
  <description>
    <en>Display a message and wait until president unlocks the machine by
    pressing the alarmbox's button.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>M_Index</name>
      <description><en>the message's Mnemonic</en></description>
      <defaultValue/>
    </parameter>
  </parameters>
  <author>AVN</author>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void WaitForPresident ( int M_Index)
// ---------------------------------------------------------------------------
{
   ConnectAlarmBox();

   // display the error message
   Error_Message( M_Index);

#ifndef _ALARM
   // set the green light on
   outp( Box_Address, 0x7E);
#endif

   // make noise until red button is pressed
   while ( !Reset_Pressed())
     Make_Noise( SOUND1);

   Reset_Lights();

   return;
}
#endif
//</code></function>


/*<function id="Reset_Lights">
<heading>
  <description><en>
    resets the alert box lights</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype>int</datatype><name>Box_Address</name>
      <description><en>the box base address</en></description></parameter>
  </parameters>
  <usage>Reset_Lights();</usage>
  <comment>
   This function should be called at the very beginning of the
   program, because on boot up the lights are on.
   If _ALARM was defined during compile time, nothing is done.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Reset_Lights ()
// ---------------------------------------------------------------------------
{

#ifndef _ALARM
   outp( Box_Address, 0x7F);                               // reset the lights
#endif

}
#endif
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="ConnectAlarmBox">
<heading>
  <description><en>Test if the alarm box is present.</en></description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
  <comment>if no alarm box, we display a message and emit a sound until it get's reconnected.
   /!\ see Reset_Pressed function for important details.</comment>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void ConnectAlarmBox (
   void
   )
// ---------------------------------------------------------------------------
{

   if ( Reset_Pressed())
   {
      // display the error message
      Error_Message( M_NO_ALARMBOX);

      // make some noise until it get's connected
      while( Reset_Pressed())
         Make_Noise( NO_ALARMBOX);
      // reset the alarm leds
      Reset_Lights();
   }

   return;
}
#endif
//</code></function>


/*<function id="SleepUnlessLightpenHit">
<heading>
  <description>
    <en>Sleep a number of seconds and quit if there's a lightpen hit.</en>
  </description>
  <parameters/>
  <comment/>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void SleepUnlessLightpenHit (
   int Timeout                // seconds to sleep
   )
// ---------------------------------------------------------------------------
{
   // 20061020 avn - lightpen interaction activated on general demand...

   time_t t;

   t = time( NULL) + (long) Timeout;

   // When lightpen is still activated, we have to wait until it is released.

   for( ; ( time( NULL) < t) && (  Lightpen_Hit()); ); // Loop till released
   for( ; ( time( NULL) < t) && ( !Lightpen_Hit()); ); // Loop till hit

   return;
}
#endif
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
