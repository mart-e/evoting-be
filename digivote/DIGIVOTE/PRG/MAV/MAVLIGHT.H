/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavlight.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVLIGHT.H  $</workFile>
<revision>$Revision: 1.3 $</revision>
<workFileTimeStamp>$Modtime:   Nov 19 2004 11:36:00  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVLIGHT.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/07/01 15:13:17 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVLIGHT.H</fileName>
  <creationDate>20/02/93</creationDate>
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
#ifndef _mavlight_h              // avoids multiple inclusions of this file
   #define _mavlight_h

const BOOL FATAL     = TRUE;
const BOOL NOT_FATAL = FALSE;

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
void Get_Pen( int * Col, int * Row);
BOOL Pen_In_Window( int Pen_Col, int Pen_Row,
                    int Win_X_Min, int Win_Y_Min, int Win_X_Max, int Win_Y_Max);

#ifndef _PREVIEW
   BOOL Lightpen_Hit( void);
   void Test_Lightpen( void);

   void Red_Light_On( int M_Index, BOOL Fatal = FATAL);
   void Green_Light_On( int M_Index);
   void WaitForPresident( int M_Index);
   void Reset_Lights( void);
   void ConnectAlarmBox (
      void
      );
   void SleepUnlessLightpenHit (
      int Timeout                // seconds to sleep
      );
#endif
//</prototypes>

#endif // _mavlight_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->
