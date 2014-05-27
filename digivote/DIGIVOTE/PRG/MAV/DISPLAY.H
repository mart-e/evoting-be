/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="display.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   DISPLAY.H  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Mar 09 2004 21:22:50  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/DISPLAY.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:25 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête pour display.cpp</fr>
    <nl>Header bestand voor display.cpp</nl></description>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _display_h
   #define _display_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\gen\gentype.h";
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
DISP_ELECT_MODE Calculate_Mode (
   ElectionList   ThisElection
   );

void Set_Candidates_Display_Mode (
   struct ElectionMain*       Root,
   struct Select_Party*       Parties,
   struct Select_Candidate*   Candidates,
   int                        Nbr_Election
   );

void Display_Candidates (
   struct Select_Candidate*   Candidates
   );

#ifndef _PREVIEW
   void Update_Candidates (
      struct Select_Candidate*   Candidates
      );
#endif
//</prototypes>

#endif // _display_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->