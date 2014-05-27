/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="videoc.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   videoc.h  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Apr 24 2003 11:56:08  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/videoc.h-arc  $</archive>
<archiveTimeStamp>$Date: 2005/04/22 09:03:59 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>videoc.h</fileName>
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
#ifndef _videoc_h              // avoids multiple inclusions of this file
   #define _videoc_h

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
void lijnhor (                      // draws an horizontal line ( in px)
   int Row,                            // on row
   int ColFrom,                        // from column
   int ColTo,                          // to column
   int Color
   );
void lijnvert (                     // draws a vertical line ( in px)
   int RowFrom,                        // from row
   int Col,                            // on column
   int RowTo,                          // to row
   int Color                           // line color
   );
void bladvul (                      // clear screen to background color
   int Color
   );
void achtersp (                     // draws a filled box in given color
   int rij1,
   int kol1,
   int rij2,
   int kol2,
   int Color                        // box color
   );

//</prototypes>


#endif // _videoc_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->