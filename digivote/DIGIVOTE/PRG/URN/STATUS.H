/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="status.h">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   status.h  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp>$Modtime:   Feb 26 2003 20:50:24  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/status.h-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/02 15:04:23 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête</fr>
    <nl>Header bestand</nl></description>
  <comment />
</comments>*/
//<header>
//============================================================= BOF HEADER ===

#ifndef _status_h                 // avoids multiple inclusions of this file
    #define _status_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\gen\gentype.h"
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
// possible image states
typedef enum {
   IMAGE_STATE_RESET = 0,              // 00
   IMAGE_STATE_BROKEN = 2,             // 10
   IMAGE_STATE_CONSISTENT = 3          // 11
   } IMAGE_STATE;

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
RET_CODE CreateStatusFile (         // create status file physically
   void
   );
RET_CODE SetImageStatus (           // set actual status
   IMAGE_STATE State                // status to memorize
   );
RET_CODE GetImageStatus (
   IMAGE_STATE* ImageState          // status value
   );
RET_CODE RemoveStatusFile (
	void
	);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---


#endif // _status_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->