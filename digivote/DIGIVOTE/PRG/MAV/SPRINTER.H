/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="sprinter.h">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   sprinter.h  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Feb 26 2003 20:50:24  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/sprinter.h-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:29 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête</fr>
    <nl>Header bestand</nl></description>
</comments>*/
//<header>
//============================================================= BOF HEADER ===

#ifndef _sprinter_h                 // avoids multiple inclusions of this file
    #define _sprinter_h

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
int initializePrinter ( void);

int ComSend( char chr);
unsigned char ComRead( void);

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---


#endif // _sprinter_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->