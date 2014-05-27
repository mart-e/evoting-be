/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="lpasm.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   lpasm.h  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp>$Modtime:   Apr 24 2003 11:56:08  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/lpasm.h-arc  $</archive>
<archiveTimeStamp>$Date: 2005/07/15 10:04:20 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>lpasm.h</fileName>
  <creationDate>04/2005</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _lpasm_h              // avoids multiple inclusions of this file
   #define _lpasm_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
extern "C" {                                            // only needed for C++
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

int detection (
   void
   );
int lightpenconnected (
   void
   );
int g_pen (                         // request for pen input
   int mode,                           // return value: 0 lightpen OK
   int onderbr,                        //               1 broken wire
   int pijl
   );
int p_ri (                          // returns row
   void
   );
int p_ko (                          // and column position when pen pressed
   void
   );

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}                                                            // end extern "C"
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//</prototypes>


#endif // _lpasm_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->