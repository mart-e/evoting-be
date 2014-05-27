/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavvideo.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   MAVVIDEO.H  $</workFile>
<revision>$Revision: 1.4 $</revision>
<workFileTimeStamp>$Modtime:   Jan 17 2005 12:37:42  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/MAVVIDEO.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/07/01 15:13:18 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>MAVVIDEO.H</fileName>
  <creationDate>10/01/933</creationDate>
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
#ifndef _mavvideo_h              // avoids multiple inclusions of this file
   #define _mavvideo_h

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
   typedef enum { DONE, OTHER_PARTY, OTHER_COL, OTHER_LANG} SELECT;
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>

   int InitPartyData(void);
   void Set_Mode( int mode, int char_size);
   void CalculatGapAndSize(void);
   void FillParty(void);
   void Display_Parties( int);

   void Centre( char * in_text, int out_len, char * out_text);
   void Draw_Button( int x, int y, int b_width, int b_height, int b_border);
   void Screen_White( void);
   void Info_Box(int line, char * text, ALIGN align);

   void DisplayTextBold (
      ALIGN TextAlign,
      int   iCol,
      int   iRow,
      char* cpText,
      BOOL BeautifyTrema               // 20040601 avn
      );

#ifndef _PREVIEW
   // functions not used in PREVIEW.EXE
   void Set_Party_Display_Mode(void);
   void Make_Ok_Button( char * text, int mid_x, int mid_y,
                        int half_width, int half_height, int border);
   void Erase_Ok_Button( int mid_x, int mid_y, int half_width, int half_height);
   int Decide_To_Review_Vote( void );
   SELECT Input_Parties( int);

   int ConfirmVoucher( void);
   int Process_Blanco_Vote (
      void
      );
   void Display_Election (
      char* cpElectionName,
      int   iElectionHasColleges,
      int   iElectionHasLanguageGroups
      );
   int SelectLanguageGroup (
      void
      );
   int Select_College (
      void
      );
   int DisplayTwoButtonChoice (
      char* cpHeader,            // Header label
      char* cpFooter,            // Footer label
      char* cpFirst,             // First button label
      char* cpSecond,            // Second button label
      ALIGN Aligned              // Header and footer alignment type
      );
   void Make_Border (
      int top,
      int left,
      int bottom,
      int right
      );
#endif

#ifdef _DEMO
   int isInitializeDemoCardOk (
      void
      );
#endif
//</prototypes>

#endif // _mavvideo_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->