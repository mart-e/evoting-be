/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="chrono.h">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   chrono.h  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp>$Modtime:   Feb 26 2003 20:50:24  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/chrono.h-arc  $</archive>
<archiveTimeStamp>$Date: 2006/10/20 15:09:56 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête</fr>
    <nl>Header bestand</nl></description>
  <comment>

   :WARNING: avn 20060915 - this code requires that the 'Floating Point' option
   in Options, Advanced Code Generation is set to 'Emulation', otherwise an
   'undefined symbol FIDRQQ' will be generated at link time.

  </comment>
</comments>*/
//<header>
//============================================================= BOF HEADER ===

#ifndef _chrono_h                 // avoids multiple inclusions of this file
    #define _chrono_h

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
int ChronoStart (             // start chronometer
	void
	);
int ChronoStop (              // stop chronometer
   void
   );
float ChronoEllapsed (       // return time between start and stop
   void
   );
int ChronoInitiate (              // initialize chrono
   void
   );
int ChronoTerminate (              // cleanup chrono
   void
   );

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---


#endif // _chrono_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->