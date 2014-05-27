/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="urnmes.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   URNMES.H  $</workFile>
<revision>$Revision: 1.5 $</revision>
<workFileTimeStamp>$Modtime:   Mar 19 2004 11:23:48  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Urn/URNMES.H-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/21 12:38:51 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>URNMES.H</fileName>
  <creationDate>01/01/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _urnmes_h                   // avoids multiple inclusions of this file
   #define _urnmes_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
   #include "..\gen\gentype.h"
   #include "..\gen\genmes.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
   void Restore_Messages( void);
   int Display_Message( int M_Index, BOOL Confirm=FALSE, BOOL Append=FALSE,
                                     BOOL Exception=FALSE);
   void Error_Message( int M_Index);
   int Ask_User_For_Language( void);
   BOOL Ask_Question( int M_Index);
   int Ask_Password( int Nr_Mes, int y, BOOL permit_escape=FALSE,
                                        int exception=FALSE);
   void Display_Title_URN( void);
   void ScreenAllCounters( void);
   void ScreenOneCounter(void);

   void Display_Background_URN();
   void DisplayAllCounter();

   void Display_Cards_Formatted( int cx, int cy, int Counter);
   void Display_Cards_In_BAC( int cx, int cy, int Counter);
   BOOL Test_Key_Pressed( int Key);
   int Get_Key( void);           // get normal key: if extended return 0
   int Get_ExtKey( BOOL *isExt); // get key: if extened key, isExt=TRUE
   void Num_Lock_On( void);

   void Display_Counter( int cx, int line, int counter); //JRU

   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   void Error_Exit( int mnemo);                         // defined in URNMAIN.CPP
   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void DisplayUseraid (
   char* pString                       // text to display
   );

void DisplayCountersForEver ();
//</prototypes>

#endif // _urnmes_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->