 /*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="drawings.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   DRAWINGS.H  $</workFile>
<revision>$Revision: 1.5 $</revision>
<workFileTimeStamp>$Modtime:   Jan 12 2005 15:02:00  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/DRAWINGS.H-arc  $</archive>
<archiveTimeStamp>$Date: 2007/02/05 10:40:40 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName></fileName>
  <creationDate>01/01/93</creationDate>
  <rights>Steria Belux</rights>
  <author></author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _drawings_h              // avoids multiple inclusions of this file
    #define _drawings_h

#define _APPLICATION_MAV

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\gen\gentype.h";
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
void Calculate_Positions(
   struct ElectionMain*       Root,
   struct Select_Party*       Parties,
   struct Select_Candidate*   Candidates,
   int                        ElectionId
   );
void Display_Header(
   int               ElectionInd,
   int               CollegeInd,
   int               PartyInd,
   char              PartyName[],
   logoType          PartyLogo,
   char              ElectionName[],
   DISP_PARTY_MODE   PartyMode,
   BOOL              Mark
   );
void Display_Footer_New_Party( DISP_PARTY_MODE PartyMode);
void Display_One_Candidate (
   POSITION          Position,
   int               Number,        // candidate's number
   char*             PackedName,    // name and first name packed
   DISP_PARTY_MODE   PartyMode,
   BOOL              Mark           // is candidate selected?
   );
void Display_Title_Suppliant( POSITION Position,
                              DISP_PARTY_MODE PartyMode);

void Update_Head_Vote( DISP_PARTY_MODE PartyMode,
                       BOOL Mark);

#ifndef _PREVIEW
   // functions not used in PREVIEW.EXE
   BOOL Is_Candidate_Selected( POSITION Position,
                               DISP_PARTY_MODE PartyMode,
                               short PenX, short PenY);
   BOOL Is_New_Party_Selected( DISP_PARTY_MODE PartyMode,
                               short PenX, short PenY);
   void Display_Footer_Cancel_Confirm( DISP_PARTY_MODE PartyMode);

   BOOL Is_Cancel_Selected( DISP_PARTY_MODE PartyMode,
                            short PenX, short PenY);

   BOOL Is_Confirm_Selected( DISP_PARTY_MODE PartyMode,
                             short PenX, short PenY);

   BOOL Is_Head_Vote_Selected( DISP_PARTY_MODE PartyMode,
                               short PenX, short PenY);

   void DisplayButtonNextElection(
      DISP_PARTY_MODE PartyMode);
   BOOL IsNextElectionSelected(
      DISP_PARTY_MODE PartyMode, short PenX, short PenY);
#endif
//</prototypes>

#endif // _drawings_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->
