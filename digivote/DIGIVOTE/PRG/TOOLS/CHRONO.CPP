/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="chrono.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   chrono.cpp  $</workFile>
<revision>$Revision: 1.3 $</revision>
<workFileTimeStamp>$Modtime:   Mar 03 2003 14:55:38  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/chrono.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2006/10/20 15:09:56 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>chrono.cpp</fileName>
  <creationDate>08/2002</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>Librairie de fonctions utilitaires</fr>
    <nl>Utility functielibrary</nl></description>
  <history/>
  <comment>This code provides functions to time an interval between two events.

  :WARNING: uses gettime(), resolution is limited at 55ms or 5/100 sec!!
  :WARNING: requires setting Floating Point option to 'Emulation' iso 'None' in
  the project Options, Code Generation, Advanced Code Generation dialog!!

  </comment>
</comments>*/
//<header>
//============================================================= BOF HEADER ===

//<includes>
//--------------------------------------------------------------- INCLUDES ---
#include <time.h>   // clock_t
#include <stdio.h>
#include <dos.h>

#include "..\tools\chrono.h"

//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

// Error codes returned by this library
#define NO_ERRORS             0        // so far so good :-)


//<prototypes>                            <!-- these functions are private -->
//------------------------------------------------------------- PROTOTYPES ---
//</prototypes>

//                                     <!-- definitions DO allocate memory -->
//---------------------------------------------------------------- GLOBALS ---
static   int                  gIsStarted = 0;         // 0 = no, 1 = yes.
static   struct time          gStart;
static   struct time          gEnd;


//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

//------------------------------------------------------- PUBLIC FUNCTIONS ---

//<function id="ChronoStart">
/*<heading>
   <description><en>Start chronometer function</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int ChronoStart (             // start chronometer
   void
   )
//----------------------------------------------------------------------------
{

   gIsStarted = 1;
   gettime( &gStart);

   #if _DEBUGMODE
      printf( " gStart [%d %d]", gStart.ti_sec, gStart.ti_hund);
   #endif  // _DEBUGMODE

   return 0;
}
//</code></function>

//<function id="ChronoStop">
/*<heading>
   <description><en>Stop chronometer function</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int ChronoStop (              // stop chronometer
   void
   )
//----------------------------------------------------------------------------
{
   gettime( &gEnd);
   gIsStarted = 0;

   #if _DEBUGMODE
      printf( " gEnd [%d %d]", gEnd.ti_sec, gEnd.ti_hund);
   #endif  // _DEBUGMODE

   return 0;
}
//</code></function>

//<function id="ChronoEllapsed">
/*<heading>
   <description><en>Stop chronometer function</en></description>
   <parameters>
    <parameter type="   return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
float ChronoEllapsed (       // return time between start and stop
   void
   )
//----------------------------------------------------------------------------
{
      int  Ellapsed, EllapsedSec;

   if ( gIsStarted)
   {
      ChronoStop();

      if ( gEnd.ti_hund < gStart.ti_hund)
      {
         gEnd.ti_sec--;
         gEnd.ti_hund += 100;
      }
      Ellapsed = gEnd.ti_hund - gStart.ti_hund;

      if ( gEnd.ti_sec < gStart.ti_sec)
      {
         gEnd.ti_min--;
         gEnd.ti_sec += 60;
      }
      EllapsedSec = gEnd.ti_sec - gStart.ti_sec;
   }
   else
   {
      Ellapsed = 99; // logic error, not started
      EllapsedSec = Ellapsed;
   }

   #if _DEBUGMODE
      printf( " hsecs[%d].\n", Ellapsed);
   #endif  // _DEBUGMODE

   return ( EllapsedSec + ( (float) Ellapsed / 100));
}
//</code></function>

//<function id="ChronoInitiate">
/*<heading>
   <description><en>Initialise chronometer</en></description>
   <parameters>
    <parameter type="   return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int ChronoInitiate (
   void
   )
//----------------------------------------------------------------------------
{
   return 0;
}
//</code></function>

//<function id="ChronoTerminate">
/*<heading>
   <description><en>Terminates chronometer</en></description>
   <parameters>
    <parameter type="   return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int ChronoTerminate (
   void
   )
//----------------------------------------------------------------------------
{
   return 0;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
