/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="debug.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   debug.h  $</workFile>
  <revision>$Revision: 1.2 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 16:21:24  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/Tools/debug.h-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/07/01 15:13:18 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>debug.h</fileName>
  <creationDate>01/09/2001</creationDate>

  <rights>Steria Belux</rights>
  <author>AVN</author>

  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history></history>
  <comment>Debugging library for internal use</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _debug_h                    // avoids multiple inclusions of this file
    #define _debug_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//----------------------------------------------------------------------------
//                                         declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
   int Dbug( void);
   int DbugHex( unsigned char* bytes, int len);
   void dcio( void);
                                                 // these functions are public
//</prototypes>

   extern char cDbugBuf[];
   extern char gcDcio[];                         // message buffer for debugging

#endif // _debug_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->