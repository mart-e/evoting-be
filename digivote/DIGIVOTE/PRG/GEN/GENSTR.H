/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genstr.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENSTR.H  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp>$Modtime:   Mar 27 2004 12:09:48  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENSTR.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/28 15:33:24 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENSTR.H</fileName>
  <creationDate>22/09/93</creationDate>
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
#ifndef _genstr_h                   // avoids multiple inclusions of this file
   #define _genstr_h
//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---
typedef enum { PIXELS, CHARS} UNITS;

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
   char* Alloc_String( char* String);

#ifdef _APPLICATION_MAV
   int Split_Text (
      char* InText,                       // text to split
      char* OutText [],                   // pointers to the text bits
      int   Dim,                          // max number of text bits
      int   Length,                       // max length
      UNITS Units                         // units of length
      );
#endif // _APPLICATION_MAV
//</prototypes>

const int MAX_LEN = 80;

#endif // _genstr_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->
