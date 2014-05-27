/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="urnreco.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   URNRECO.H  $</workFile>
<revision>$Revision: 1.4 $</revision>
<workFileTimeStamp>$Modtime:   Oct 19 2004 12:02:58  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Urn/URNRECO.H-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/02 15:04:23 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>URNRECO.H</fileName>
  <creationDate>11/06/93</creationDate>
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
#ifndef _urnreco_h              // avoids multiple inclusions of this file
   #define _urnreco_h

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
   RET_CODE Initialize_Direct_Access( void);

   void Setup_Registration( void);

   RET_CODE Create_Journals( void);

   // void Process_Card_Formatting( char Blanco_Card[]);
   RET_CODE Process_Card_Formatting( char Blanco_Card[], int  ielect);

   void Reset_Counters();                 //STEF: reset counters values
   int Get_BACcounter( int CounterType); //JRU : get a counter of cards in BAC

   void Process_Card_In_BAC( char Card_Buffer []);
   void Process_Card_Fast( char Card_Buffer []);

   void Find_Out_Disk( void);
   BOOL Card_Is_Initialized( Select_Reader * Reader);

RET_CODE Count_Votes (
   void
   );
RET_CODE Clear_Image_Files (
   void
   );

RET_CODE Create_Journals_Formatted( void);

//</prototypes>

#endif // _urnreco_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->