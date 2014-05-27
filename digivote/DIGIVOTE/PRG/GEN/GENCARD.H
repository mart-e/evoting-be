/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gencard.h">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENCARD.H  $</workFile>
<revision>$Revision: 1.5 $</revision>
<workFileTimeStamp>$Modtime:   Oct 27 2004 12:38:24  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENCARD.H-arc  $</archive>
<archiveTimeStamp>$Date: 2007/02/05 10:36:09 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments><!-- xml fragment from source code -->
  <fileName>GENCARD.H</fileName>
  <creationDate>20/02/93</creationDate>

  <rights/>
  <author>B.S.S.</author>

  <description>
    <fr>Fichier d'entête</fr>
    <nl>Header file</nl></description>
  <history></history>
  <comment></comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>
#ifndef _GENCARD                      // avoid mutiple inclusions of this file
   #define _GENCARD

//======================== I N C L U D E   F I L E S ==========================
//<includes>
#include "..\gen\card.h"
//</includes>

//=========================== C O N S T A N T S ===============================
//<globals>
const int MAX_CARD_BUF = 104;

//============================ T Y P E D E F S ================================
typedef enum { NO_CARD, SLOT, HOME, CARD_ERROR} CARD_POSITION;
//</globals>

//=========================== P R O T O T Y P E S =============================
//<prototypes>
CARD_POSITION Reader_Type(                Select_Reader * Reader);
CARD_POSITION Reader_Card_Position(       Select_Reader * Reader);
CARD_POSITION Reader_Unformatted_Card(    Select_Reader * Reader);
CARD_POSITION Reader_Formatted_Card(      Select_Reader * Reader);
CARD_POSITION Reader_Request_Unformatted( Select_Reader * Reader);
CARD_POSITION Reader_Request_Formatted(   Select_Reader * Reader);
CARD_POSITION Reader_Write_Card(          Select_Reader * Reader, char * Data);
CARD_POSITION Reader_Physical_Read_Card(  Select_Reader * Reader, char * Data);
CARD_POSITION Reader_Read_Card(           Select_Reader * Reader, char * Data);
void          Reader_Move_Card(           Select_Reader * Reader);
CARD_POSITION Reader_Eject_Card(          Select_Reader * Reader);
CARD_POSITION Reader_Only_Eject_Card(     Select_Reader * Reader);
CARD_POSITION Reader_Disable(             Select_Reader * Reader);
CARD_POSITION Reader_Take_Card(           Select_Reader * Reader);
void Reader_Wait_Card_Removed(            Select_Reader * Reader);
void Reader_Deformat_Card(                Select_Reader * Reader);
void Error_Procedure(                     Select_Reader * Reader);
void Check_Reader_Type (
   TYPE_READER Extern_Type    // reader type
   );
void Save_Reader_Reply (
   void
   );
void Restore_Reader_Reply (
   void
   );
//</prototypes>

#endif                                              // end of #ifndef _GENCARD
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->