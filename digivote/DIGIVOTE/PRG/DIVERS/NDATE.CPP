/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ndate.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   boilerplate.cpp  $</workFile>
  <revision>$Revision: 1.7 $</revision>
  <workFileTimeStamp>$Modtime:   Dec 17 2003 12:32:10  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/boilerplate.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2008/10/14 08:10:13 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>ndate.cpp</fileName>
  <creationDate>01/02/2005</creationDate>
  <rights>Steria Benelux</rights>
  <author>avn</author>
  <description>
    <fr>Initialisation de la date et heure système.</fr>
    <nl>Initialisatie van systeem datum en tijd.</nl></description>
  <history></history>
  <comment>Get date and time from user and initialize the system date and time.</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <dos.h>
#include <process.h>
#include <stdlib.h>                    // atoi()
#include <stdio.h>
#include <conio.h>                     // wherex(), wherey()
#include <string.h>                    // strtok()

#define _DEBUGMODE      0      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define DATELEN   10          // user may use DATELEN characters to enter date
#define YEAROFF   2000        // years entered as 1-99 are offset using YEAROFF
#define MINYEAR   2010        // year must be in the MINYEAR - MAXYEAR interval
#define MAXYEAR   2015

#define TIMELEN   8           // user may use TIMELEN characters to enter time
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---
typedef enum {
   FALSE = 0,
   TRUE = 1
   } BOOL;

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
   struct dosdate_t gCurrentDate;
   struct dosdate_t gNewDate;

   struct dostime_t gCurrentTime;
   struct dostime_t gNewTime;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

#pragma argsused
int handler( int errval, int ax, int bp, int si)
{
   hardresume( _HARDERR_IGNORE);   // ignore the error
}

int c_break( void)
{
   return 1;                        // continue the program
}

/*<function id="IsStringNumeric">
<heading>
  <description>
    <en>Check if string is strictly numeric.</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Returns 1 if time is invalid, 0 if valid</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL IsStringNumeric (
   char* String                  // points to string to be checked
   )
// ---------------------------------------------------------------------------
{
   int   Status;
   char* Temp;

   Status = 0;

   #if _DEBUGMODE
      sprintf( gcDcio, "IsStringNumeric - String[%s] len[%d]",
         String, strlen( String)); dcio();
   #endif // _DEBUGMODE

   if ( ! String)
   {
      Status = 10;
      goto error;
   }
   else
   {
      Temp = String;          // we don't want to mess with the original
   }

   while ( *Temp)
   {
      if ( ! ( '0' <= *Temp && *Temp <= '9'))
      {
         Status = 20;
         goto error;
      }
      Temp++;
   }

   return TRUE;

error:
   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "IsStringNumeric - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return FALSE;
}
//</code></function>


/*<function id="GetDate">
<heading>
  <description>
    <en>Get date from user.</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void GetDate (
   void
   )
// ---------------------------------------------------------------------------
{
      char  cString[DATELEN + 3];
      char* cpUserInput;
      char* cpToken;
      char  cSeparator[] = "/";        // separates the tokens


   gNewDate.day = 0;
   gNewDate.month = 0;
   gNewDate.year = 0;

   printf("\nEnter the new date: (dd/mm/yyyy) ");

   cString[0] = DATELEN + 1; // read at most DATELEN characters + terminator
   cpUserInput = cgets( cString);

   #if _DEBUGMODE
      printf( "\ncgets read %d/%d characters : UI[%s] cString[%s]",
         cString[1], cString[0], cpUserInput, cString);
   #endif // _DEBUGMODE

   if ( 0 != (int) cString[1])
   {
      // split user input into day, month and year
      cpToken = strtok( cpUserInput, cSeparator);
      if ( cpToken)
      {
         gNewDate.day = atoi( cpToken);
      }

      cpToken = strtok( NULL, cSeparator);
      if ( cpToken)
      {
         gNewDate.month = atoi( cpToken);
      }

      cpToken = strtok( NULL, cSeparator);
      if ( cpToken)
      {
         gNewDate.year = atoi( cpToken);
         if ( 0 < gNewDate.year && gNewDate.year < 100)
         {
            gNewDate.year += YEAROFF;
         }
      }
   }
   else
   {
      // no date provided, use current date...
      _dos_getdate( & gCurrentDate);

      gNewDate.day   = gCurrentDate.day;
      gNewDate.month = gCurrentDate.month;
      gNewDate.year  = gCurrentDate.year;
   }

   return;
}
//</code></function>


/*<function id="IsInvalidDate">
<heading>
  <description>
    <en>Check if a date is invalid.</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Returns 1 if date is invalid, 0 if valid</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL IsInvalidDate (
   void
   )
// ---------------------------------------------------------------------------
{
      int   Status      = 0;
      BOOL  IsLeapYear;

   // year must be in the 2000-2008 interval
   if ( ! ( MINYEAR <= gNewDate.year && gNewDate.year <= MAXYEAR))
   {
      Status = 10; // invalid year :-(
      goto error;
   }
   else
   {
      // test leap years
      if ( 0 != ( gNewDate.year % 4))
      {
         IsLeapYear = FALSE;
      }
      else if ( 0 == ( gNewDate.year % 400))
      {
         IsLeapYear = TRUE;
      }
      else if ( 0 == ( gNewDate.year % 100))
      {
         IsLeapYear = FALSE;
      }
      else
      {
         IsLeapYear = TRUE;
      }
   }

   // month must be in the 1-12 interval
   if ( ! ( 1 <= gNewDate.month && gNewDate.month <= 12))
   {
      Status = 20; // invalid month :-(
      goto error;
   }

   // days
   switch ( gNewDate.month)
   {
      // for these months day must be in the 1-31 interval
      case  1:         // fall thru intended!
      case  3:         // fall thru intended!
      case  5:         // fall thru intended!
      case  7:         // fall thru intended!
      case  8:         // fall thru intended!
      case 10:         // fall thru intended!
      case 12:         // fall thru intended!
      {
         if ( ! ( 1 <= gNewDate.day && gNewDate.day <= 31))
         {
            Status = 30;
            goto error;
         }
         break;
      }

      // for these months day must be in the 1-30 interval
      case  4:         // fall thru intended!
      case  6:         // fall thru intended!
      case  9:         // fall thru intended!
      case 11:         // fall thru intended!
      {
         if ( ! ( 1 <= gNewDate.day && gNewDate.day <= 30))
         {
            Status = 33;
            goto error;
         }
         break;
      }

      case  2:
      {
         if ( IsLeapYear)
         {
            // in leap years days must be in 1-29 interval
            if ( ! ( 1 <= gNewDate.day && gNewDate.day <= 29))
            {
               Status = 36;
               goto error;
            }
         }
         else
         {
            // else days must be in 1-28 interval
            if ( ! ( 1 <= gNewDate.day && gNewDate.day <= 28))
            {
               Status = 39;
               goto error;
            }
         }
         break;
      }

      default:
      {
         // we never get here since month is in 1-12 interval
         Status = 99;
         goto error;
         break;
      }
   } // eo switch

   // date is ok so it is NOT invalid
   return FALSE;

error:

   // date is invalid
   printf("\nThe system cannot accept the date entered.");

   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "IsInvalidDate - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return TRUE;
}
//</code></function>


/*<function id="GetTime">
<heading>
  <description>
    <en>Get time from user.</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void GetTime (
   void
   )
// ---------------------------------------------------------------------------
{
      char  cString[TIMELEN + 3];
      char* cpUserInput;
      char* cpToken;
      char  cSeparator[] = ":";        // separates the tokens


   // trigger bad time
   gNewTime.hour     = 99;
   gNewTime.minute   = 99;
   gNewTime.second   = 0;
   gNewTime.hsecond  = 0;

   printf("\nEnter the new time: (hh:mm:ss) ");

   cString[0] = TIMELEN + 1; // read at most TIMELEN characters + terminator
   cpUserInput = cgets( cString);

   #if _DEBUGMODE
      printf( "\ncgets read %d/%d characters : UI[%s] cString[%s]",
         cString[1], cString[0], cpUserInput, cString);
   #endif // _DEBUGMODE

   if ( 0 != (int) cString[1])
   {
      // split user input into day, month and year
      cpToken = strtok( cpUserInput, cSeparator);
      if ( cpToken)
      {
         if ( IsStringNumeric( cpToken) && atoi( cpToken) <= 255)
         {
            gNewTime.hour = atoi( cpToken);
         }
      }

      cpToken = strtok( NULL, cSeparator);
      if ( cpToken)
      {
         if ( IsStringNumeric( cpToken) && atoi( cpToken) <= 255)
         {
            gNewTime.minute = atoi( cpToken);
         }
      }

      cpToken = strtok( NULL, cSeparator);
      if ( cpToken)
      {
         if ( IsStringNumeric( cpToken) && atoi( cpToken) <= 255)
         {
            gNewTime.second = atoi( cpToken);
         }
      }
   }
   else
   {
      // no time provided, use current time...
      _dos_gettime( & gCurrentTime);

      gNewTime.hour    = gCurrentTime.hour;
      gNewTime.minute  = gCurrentTime.minute;
      gNewTime.second  = gCurrentTime.second;
      gNewTime.hsecond = gCurrentTime.hsecond;
   }

   return;
}
//</code></function>


/*<function id="IsInvalidTime">
<heading>
  <description>
    <en>Check if a time is invalid.</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>Returns 1 if time is invalid, 0 if valid</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL IsInvalidTime (
   void
   )
// ---------------------------------------------------------------------------
{
      int   Status      = 0;

   // hour must be in the 0-23 interval
   if ( ! ( 0 <= gNewTime.hour && gNewTime.hour <= 23))
   {
      Status = 10; // invalid hour :-(
      goto error;
   }

   // minute must be in the 0-59 interval
   if ( ! ( 0 <= gNewTime.minute && gNewTime.minute <= 59))
   {
      Status = 20; // invalid minute :-(
      goto error;
   }

   // second must be in the 0-59 interval
   if ( ! ( 0 <= gNewTime.second && gNewTime.second <= 59))
   {
      Status = 30; // invalid second :-(
      goto error;
   }

   // time is ok so it is NOT invalid
   return FALSE;

error:

   // date is invalid
   printf("\nThe system cannot accept the time entered.");

   Status = -Status;
   #if _DEBUGMODE
      sprintf( gcDcio, "IsInvalidTime - aborts [%d]", Status); dcio();
   #endif // _DEBUGMODE

   return TRUE;
}
//</code></function>


//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="ndate" type="main">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int main( void)
// ---------------------------------------------------------------------------
{

   ctrlbrk( c_break);   // trap the control break;
   harderr( handler);   // redirects hardware errors


   clrscr();

   // get current date from dos
   _dos_getdate( & gCurrentDate);

   printf("The current date is: %02.02d/%02.02d/%04.04d",
      gCurrentDate.day, gCurrentDate.month, gCurrentDate.year);

   do {
      GetDate();
   } while ( IsInvalidDate());

   if ( _dos_setdate( & gNewDate))
   {
      printf( "\nSorry, the system date could not be updated...\n");
   }
   else
   {
      printf("\nThe new current date is: %02.02d/%02.02d/%04.04d",
         gNewDate.day, gNewDate.month, gNewDate.year);
   }


   // get current time from dos
   _dos_gettime( & gCurrentTime);

   printf("\nThe current time is: %02.02d:%02.02d",
      gCurrentTime.hour, gCurrentTime.minute);

   do {
      GetTime();
   } while ( IsInvalidTime());

   // :WARNING: avn 20050207 - settime function does not work under w2k and wxp...
   if ( _dos_settime( & gNewTime))
   {
      printf( "\nSorry, the system time could not be updated...\n");
   }
   else
   {
      printf("\nThe new current time is: %02.02d:%02.02d:%02.02d",
         gNewTime.hour, gNewTime.minute, gNewTime.second);
   }


   return 0;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
