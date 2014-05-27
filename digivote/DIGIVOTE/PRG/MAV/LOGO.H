/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="logo.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   logo.h  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Feb 27 2003 10:17:56  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/logo.h-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:28 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName></fileName>
  <creationDate>01/02/2003</creationDate>
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
#ifndef _logo_h              // avoids multiple inclusions of this file
    #define _logo_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//<system></system>
//<user></user>

#include "..\gen\gentype.h"            // defines logoType
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>

#define MAX_LOGO_WIDTH  78
#define MAX_LOGO_HEIGHT 26
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
void setLogoPath (
   char* cpPath                  // the path string
   );
int displayLogo (
   positionType Position,        // position where to put the logo ( top-left)
   logoType Logo,                // logo properties ( width & height)
   int ElectionId,               // used to identify the raster data file
   int CollegeId,                //  "
   int PartyId                   //  "
   );
//</prototypes>


#endif // _logo_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->