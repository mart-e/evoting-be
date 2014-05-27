/*H*/
//=============================================================================
/*

   File        : GENLANG.CPP
   Creation    : 01/01/93

   Author      : B.S.S.

   Description : Functions for language dependent implementation.

*/
//=============================================================================
/*h*/
/*R*/
//=============================================================================
/*

   Revision History
   ################

   01/06/93: revision history starting date
   17/09/93: the Init_Language function was modified to allow language display
             permutation

*/
//=============================================================================
/*r*/
/*#*/

//======================== I N C L U D E   F I L E S ==========================

#include <stdlib.h>                    // atoi
#include <graphics.h>                  // closegraph

#include "..\gen\gentype.h"
#include "..\gen\genlang.h"

//=================== E X T E R N A L   V A R I A B L E S =====================

extern struct ElectionMain Root;

//================================= B O D Y ===================================

/*{*/
//=============================================================================
//
// Function : Working_Language
//
// Purpose  : this function stores the working language in a static variable
//            for later use
//
// Return   : int             , a single language code, or NONE
//                              NONE = no language yet defined
//
// Arg List : int Language    , a single language code or ASK
//                              default value = ASK
//
// Globals  : const int NONE = -1   , genlang.h
//            const int ASK  = -2   , genlang.h
//
// Usage    : Working_Language( Language);            to set the language code
//            Language = Working_Language( ASK);      to ask the language code
//
// Checks   : The language code should be a single value, not a combined
//            language value.
//
// Comment  : The initial value for Working_Language is NONE
//
//-----------------------------------------------------------------------------
/*}*/

int Working_Language( int Language)
{
   static int Working_Language = NONE;  // stores the language in the function

   if( Language != ASK)
      Working_Language = Language;        // update the working language value

      return Working_Language;
}

/*{*/
//=============================================================================
//
// Function : Init_Languages
//
// Purpose  : analyses the language code present in the program's command line
//            to determine the different used languages, intializes the working
//            language value and stores the single values in the Root struct.
//
// Return   : void
//
// Arg List : char * Param_Language  ,  a combined language code (argv [1])
//                                      passed as a string (from command line)
//                                      no default value
//
// Globals  : struct ElectionMain Root , gentype.h for composition
//
// Usage    : Init_Language( argv [1]);
//
// Checks   : only allowed combinations are admitted, every other case gives
//            an error message and locks the system
//
// Comment  : The supported languages are : French             1
//                                          Dutch              2
//                                          French + Dutch     3
//                                          German             4
//                                          French + German    5
//
//            This code is converted in the column number in which the
//            messages corresponding to the language are stored.
//
//            The error message is in english because we don't have any
//            acceptable language at this point.
//
//            If the passed language is a composed one, then Working_Language
//            is initialized with NONE. User will be asked later for one.
//            The order for assigning the language column number to
//            Root.Language [0] and Root.Language [1] IS SIGNIFICANT, this order
//            reflects the order of the election names in the B021 file.
//
//                      Language code order : FR, NL, GR
//
//            This order determines the order by witch the columns or the
//            rows are displayed on screen when two languages are active.
//            If the passed language code is negative, it means that we want
//            to display the messages in the opposite order.
//
//-----------------------------------------------------------------------------
/*}*/

#ifdef _APPLICATION_URN
void Init_Languages( char * Param_Language)
{
   Root.Language [0] = Root.Language [1] = NONE;  // initialize the Root struct

   int language = atoi( Param_Language);

   switch( abs( language))
   {
      case 1  :

         Root.Language [0] = 0;
         break;   // Fran‡ais

      case 2  :

         Root.Language [0] = 1;
         break;   // Nederlands

      case 3  :

         Root.Language [0] = 0;
         Root.Language [1] = 1;
         break;

      case 4  :

         Root.Language [0] = 2;
         break;   // Deutsch

      case 5  :

         Root.Language [0] = 0;
         Root.Language [1] = 2;
         break;

     // every other combination is an error
     default :

      closegraph();
      printf( "\7");                                         // beep
      printf( "\nError: The language parameter in the command line\n");
      printf( "       is missing or is not supported.\n");
      printf( "       System is halted\n");
      for(;;);                                    // lock the system
   }

   if( Root.Language [1] == NONE)
      Working_Language( Root.Language [0]); // initialize the working language
   else
   {
      Working_Language( NONE);    // user will be asked for a working language

      if( language < 0)                    // reverse the normal display order
      {
         language = Root.Language [1];
         Root.Language [1] = Root.Language [0];
         Root.Language [0] = language;
      }
   }

}
#endif

#ifdef _APPLICATION_MAV
void Init_Languages( char* Param_Language, char* Param_Adm_Language )
{
   Root.Language [0] = Root.Language [1] = NONE;  // initialize the Root struct

   int language = atoi( Param_Language);
   int adm_language = atoi( Param_Adm_Language);

   if( language == 3 || language == 5 )
   {
      // bilingual mode ( FRNL or FRDE)
      if( adm_language == 3 || adm_language == 5 )
      {
         // first language always FR
         Root.Language [0] = 0;
         // second is NL or DE
         Root.Language [1] = adm_language / 2;
      }
      else
      {
         // first language is presidents language
         Root.Language [0] = adm_language / 2;
         Root.Language [1] = ( language / 2 ) - Root.Language [0] ;
      }
   }
   else
   {
      // monolingual mode ( FR or NL or DE)
      Root.Language [0] = language / 2;
   }

   if ( Root.Language [1] == NONE)
      Working_Language( Root.Language [0]); // initialize the working language
   else
      Working_Language( NONE);    // user will be asked for a working language
}
#endif

//=== GENLANG.CPP =============================================================
