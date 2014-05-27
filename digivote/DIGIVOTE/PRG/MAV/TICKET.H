/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="ticket.h">
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <revision>$Revision: 1.2 $</revision>
  <archiveTimeStamp>$Date: 2005/07/01 15:13:18 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête</fr>
    <nl>Header bestand</nl></description>
</comments>*/
//<header>
//============================================================= BOF HEADER ===
#ifndef _ticket_h                  // avoids multiple inclusions of this file
    #define _ticket_h

//----------------------------------------------------------------------------
//                                         declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
int initializeTicketing ( void);
int setupTicketing ( void);
int printTicket ( int iFunc, BYTE* PhysicalCardData);
//</prototypes>


enum TICKETING_MODES
   {
   SPOOL_VOTES, PRINT_VOTES, DROP_VOTES
   };

#endif // _ticket_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->