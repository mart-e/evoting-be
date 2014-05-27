/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="tcom.cpp"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by pvcs during check in -->
  <revision>$Revision: 1.3 $</revision>
  <archiveTimeStamp>$Date: 2005/07/01 15:13:16 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
<fileName>TCOM.CPP</fileName>
<creationDate>14/04/93</creationDate>
<rights/>
<author>B.S.S.</author>
  <description>
    <fr>Programme pour détecter la présence d' un lecteur de carte.
    Valeurs de sortie : 0 aucun lecteur présent
                        1 lecteur interne présent
                        2 lecteur externe présent
                        3 lecteur interne et externe présent</fr>
    <nl>Programma om de aanwezigheid van een kaartlezer te detecteren.
    terugkeerwaarden :  0 geen lezer aanwezig
                        1 interne lezer aanwezig
                        2 externe lezer aanwezig
                        3 interne en externe lezer aanwezig</nl></description>
<comment>
  Description: Program to detect cardreader's presence.

  Exit value : 0 no intern nor extern reader response
               1 intern reader present, no extern reader
               2 no intern reader, extern reader present
               3 intern and extern readers present</comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>

#include "..\gen\card.h"
#include "..\mav\sprinter.h"
//</includes>

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
int Version;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Reader_Error_Handler_"><!-- underscore to avoid duplicated id -->
<heading>
  <description>
    <fr>Envoie les erreurs d' appareil vers le programme</fr>
    <nl>Stuurt de device errors naar het programma</nl></description>
  <parameters></parameters>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
int Reader_Error_Handler (
   int errval, int ax, int bp, int si)
// ---------------------------------------------------------------------------
{
   hardresume( _HARDERR_IGNORE);
   return 1;
}
//</code></function>


/*<function id="c_break_"><!-- underscore to avoid duplicated id -->
<heading>
  <description>
    <fr>Captation du 'Contol break'</fr>
    <nl>Opvangen van de 'Contol break'</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int c_break (
   void)
// ---------------------------------------------------------------------------
{
   return 1;                        // continue the program
}
//</code></function>


/*<function id="Tcom" type="main">
<heading>
  <description>
    <fr>Programme principal</fr>
    <nl>Hoofdprogramma</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int main(
   int argc, char** argv)
// ---------------------------------------------------------------------------
{
      int iRet;

   harderr( Reader_Error_Handler);  // Redirect device errors to application
   ctrlbrk( c_break);               // trap the control break;

   if( argc == 5)
   {
      TYPE_READER type_I = (TYPE_READER) atoi( argv[1]);    // intern reader
      PORT        com_I  = (PORT) atoi( argv[2]);           // intern com
      TYPE_READER type_E = (TYPE_READER) atoi( argv[3]);    // extern reader
      PORT        com_E  = (PORT) atoi( argv[4]);           // extern com

      // Check com number validity
      if( com_I != com_E                                           &&
        ( com_I == COM1 || com_I == COM2 || com_I == UNKNOWN_PORT) &&
        ( com_E == COM1 || com_E == COM2 || com_E == UNKNOWN_PORT)   )
      {
         int ret = 0;

         // if both params for Internal Reader are UNKNOWN_??? -> don't test
         if( type_I != UNKNOWN_TYPE && com_I != UNKNOWN_PORT)
             if( Open_Command( type_I, com_I) != NULL)  // try to open intern reader
                 ret += 1;

         // setup and check for a printer first, otherwise the card reader
         //  commands make the printer print rubbish
         iRet = initializePrinter();
         if ( 0 == iRet )
         {
            // no return code means printer initialized properly, so there's
            //  definitly no card reader on this port!
            return ret;
         }

         // if both params for External Reader are UNKNOWN_??? -> don't test
         if( type_E != UNKNOWN_TYPE && com_E != UNKNOWN_PORT)
             if( Open_Command( type_E, com_E) != NULL)  // try to open extern reader
                 ret += 2;

         return ret;    // exit the main
      }
   }

   // Bad command line, so give some info and lock the system
   printf( "Bad command : %s %s %s\n", argv[0], argv[1], argv[2], argv[3], argv[4]);
   printf( "Usage : %s <Type cardreader_I> <COM port_I>\n\
                       <Type cardreader_E> <COM port_E>\n", argv[0]);
   printf( "System halted\n");
   for(;;);
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

