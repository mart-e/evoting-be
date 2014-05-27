/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="logging.h"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2006/10/02 12:55:40 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>logging.h</fileName>
  <creationDate>01/01/2003</creationDate>
  <rights>Steria Benelux</rights>
  <author></author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment>was debug.cpp, renamed to logging.cpp</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _LOGGING_H_
   #define _LOGGING_H_

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#ifdef _APPLICATION_URN
   #define LogFile "A:\\URN.LOG"
#else
   #define LogFile "\\VOTE\\MENU.LOG"
#endif

   typedef enum RESULTS {
     EMPTY,
     OK,
     NOK,
     ERR
   };
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
   void Add_To_Logging( RESULTS ErrorCode, char *LogInfo);
   void logOnStartup( void);
   void logOnExit( void);
//</prototypes>

extern char gcLogBuf[];                       // message buffer for logging

#endif // _LOGGING_H_
//============================================================= EOF HEADER ===
/*</header>
</source>
<!-- eof -->*/

