/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavmes.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVMES.H  $</workFile>
<revision>$Revision: 1.4 $</revision>
<workFileTimeStamp>$Modtime:   Jan 12 2005 16:29:32  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVMES.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/07/01 15:13:18 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVMES.H</fileName>
  <creationDate>20/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _mavmes_h                   // avoids multiple inclusions of this file
    #define _mavmes_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <graphics.h>

#include "..\gen\genmes.h"
//</includes>

//----------------------------------------------------------------------------
//                                         declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
void Display_Message (
   int Mnemo,                 // position of the message in the message array
   BOOL confirm = NO,         // put a confirm button ?
   int timeout = 0,           // number of seconds to display the screen
   BOOL bypass = NO,          // ability to bypass the timeout?
   int size_x = 1,            // controls the font size: FontWidth  *= size_x/Size_X
   int Size_X = 1,            //
   int size_y = 1,            //                         FontHeight *= size_y/Size_Y
   int Size_Y = 1,            //
   int x_min = 0,             // coordinates of the message box, default = fullscreen
   int y_min = 0,             //
   int x_max = getmaxx(),     //
   int y_max = getmaxy()      //
   );
void Display_Strings(
   int Mnemo,
   int Language,
   int mid_x,
   int min_y,
   int height
   );

void Error_Exit( int M_Index);
void Error_Message( int M_Index);

void DisplayElectionNameSmall(
   char* cpElectionName
   );

#ifndef _PREVIEW

int Ask_User_For_Language(
   void
   );

void DisplayElectionName(
   char* cpElectionName
   );

int DisplayTwoButtonDialog (
   char* cpTitle,             // Title of the dialog, use NULL when no title is required
   int   iMessage,            // Mnemonic for the message
   int   iCancels,            // Mnemonic for the cancel message
   int   iConfirms            // Mnemonic for confirm message
   );

#endif //ifndef _PREVIEW
//</prototypes>

#endif // _mavmes_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->