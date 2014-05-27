/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="gencard.cpp">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.7 $</revision>
  <archiveTimeStamp>$Date: 2007/02/05 10:36:09 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENCARD.CPP</fileName>
  <creationDate>20/02/93</creationDate>

  <rights></rights>
  <author>B.S.S.</author>

  <description>
    <fr>Librairie de fonctions lecteur de carte magnétiques</fr>
    <nl>Library functies voor magneetkaartlezer</nl></description>
  <history>
   Revision History
   ################

   01/06/93: revision history starting date
   10/06/93: Reader_Move_Card() added
   05/08/93: New error code "24:read error" added
   05/08/93: Reader_Physical_Read_Card() added.
   20/10/93: A delay was added before the Test_Lightpen function (for the MAV,
             in the Reader_Request_Formatted and Reader_Request_Unformatted
             functions) so we left the time to the pxl-bios to notice the
             disconnection.
             We suppose that the pxl-bios driver clears the lightpens state
             when we disable and enable it in the reader function.
   07/03/94: The routines Reader_Eject_Card and Reader_Only_Eject_Card use
             now the INITIALIZE command for moving a card instead of the
             MOVE command.  It seems that the INITIALIZE command is better
             in freeing a stucked double card.
   22/03/94  rvh: The function "Reader_Eject_Card" and "Reader_Only_Eject_Card"
             may not send an INITIALISE as last command of the retrys.
   24/03/94: rvh: Function "Reader_Take_Card" modified for clearing
             entry in journal when a card cant get CAPTUREd.
             Functions "Clear_Entry_In_Journal" and "Error_Procedure" added.</history>
  <comment></comment>
</comments>
<header>*/

//======================== I N C L U D E   F I L E S ==========================
//<includes>
#include <conio.h>    // JRU : WatchDog
#include <string.h>
#include <time.h>
#include <dos.h>

#include "..\gen\gentype.h"
#include "..\gen\gencard.h"

#ifdef _APPLICATION_URN
   #include "..\urn\urnmes.h"
#endif

#ifdef _APPLICATION_MAV
   #include "..\mav\mavlight.h"             // we test for lightpen's presence
   #include "..\mav\mavmes.h"
#endif

#define _DEBUGMODE      FALSE      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//<globals>
//===================== G L O B A L   V A R I A B L E S =======================

char* Reader_Reply;
char* PreviousReader_Reply;

//=================== E X T E R N A L   V A R I A B L E S =====================

extern Select_Reader * Intern_Reader, * Extern_Reader;

//</globals>
//====================== L O C A L   F U N C T I O N S ========================
//<prototypes>
static void Reader_Error_Message( Select_Reader * Reader,
                                  int M_Index1, int M_Index2);
static CARD_POSITION Reader_P_Response( char * Reader_Reply);
static CARD_POSITION Reader_N_Response( char * Reader_Reply,
                                        Select_Reader * Reader);
/*</prototypes>
</header>

<body>*/
//================================= B O D Y ===================================
/*
   General remark: if the applcation is compiled with _NOREADER defined, then
   ==============  each function will simulate the card reader's presence by
                   returning the expected value.

   Error handling: the low level reader routines directly address the reader
   ==============  via the serial port; if an hardware error occurs, this is
                   treated by the Floppy_Error_Handler routines which sends
                   the IGNORE to the DOS.

   Reader reply composition:
   ========================

   The reader reply is composed from 5 chars
   - the first is P or N for Positive and Negative response
   - the second and third are the code of the function that was called
   - the two last are the readers reply code

   CARD_POSITION values:  NO_CARD    , reader is empty           (reply P00)
   ====================   SLOT       , card is at reader's entry (reply P01)
                          HOME       , card is in reader         (reply P02)
                                       or operation was successfull
                          CARD_ERROR , reader sends an error     (reply NXX)

   Negative respone error codes:
   ============================

    00   a given command code is unidentified    , program error     FATAL
    01   parameter is not correct                , program error     FATAL
    02   command execution is impossible         , program error     FATAL
    04   command data error                      , program error     FATAL
    09   other command errors                    , program error     FATAL
    10   card jam                                , card    error NOT FATAL
    11   shutter failure                         , reader  error     FATAL
    12   sensor failure                          , reader  error     FATAL
    13   irregular card lenght (long)            , card    error NOT FATAL
    14   irregular card lenght (short)           , card    error NOT FATAL
    15   EEPROM error                            , reader  error     FATAL
    16   card position move                      , card    error NOT FATAL
    17   jam error at retrieve                   , card    error NOT FATAL
    19   other mechanical irregularities         , reader  error     FATAL
    20   read error (parity error)               , card    error NOT FATAL
    21   read error (other errors)               , card    error NOT FATAL
    22   write error                             , card    error NOT FATAL
    23   read error (only STX-ETX-LRC card)      , card    error NOT FATAL
    30   power down                              , reader  error     FATAL
    31   DSR signal was turned off               , reader  error     FATAL
    40   card was removed during capture         , card    error NOT FATAL
    XX   error code related to IC                , reader  error     FATAL

*/

/*<function id="Save_Reader_Reply">
<heading>
  <description>
    <en>Make a copy of the current reader reply</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment />
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Save_Reader_Reply (
   void
   )
// ---------------------------------------------------------------------------
{
   strcpy( PreviousReader_Reply, Reader_Reply);
   return;
}
//</code></function>

/*<function id="Restore_Reader_Reply">
<heading>
  <description>
    <en>Restore previous reader reply</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment />
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Restore_Reader_Reply (
   void
   )
// ---------------------------------------------------------------------------
{
   strcpy( Reader_Reply, PreviousReader_Reply);
   return;
}
//</code></function>

/*<function id="Reader_Error_Message"><heading>
  <description>
    <en>Displays a message and lock the system when a fatal error occurs
on the card reader.</en></description>
  <parameters>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader that causes the problem</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>M_Index1</name>
      <description><en>the message to display if the reader
is the intern reader</en></description></parameter>
    <parameter>
      <datatype>int</datatype>
      <name>M_Index2</name>
      <description><en>the message to display if the reader
is the extern one</en></description></parameter>
    <parameter type="return">
      <datatype>static void</datatype>
      <description><en>local function</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype>
      <name>Intern_Reader</name>
      <description><en>the intern reader definition</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader*</datatype>
      <name>Extern_Reader</name>
      <description><en>the extern reader definition</en></description></parameter></parameters>
  <usage>Reader_Error_Message( Reader, M_BAD_READER1, M_BAD_READER2);</usage>
  <checks></checks>
  <comment>Before locking the system or performing the jump, we first
try to disable the readers and to eject all remaining cards
from inside the readers; these commands are only sent, no
error checking is made. This is to avoid that the readers
still accepts a card when the system is locked.
For the MAV the locking take place after that the alarm box
Red light was set.</comment></heading>

<code>*/
static void Reader_Error_Message( Select_Reader * Reader,
                                  int M_Index1, int M_Index2)
{
#ifdef _NOREADER
   return;
#endif

   // try to disable the readers and to eject the remaining cards

#ifdef _APPLICATION_URN

 #ifndef _RECOUNT

   Send_Command( Intern_Reader, DISABLE, NULL);
   Send_Command( Intern_Reader, EJECT, NULL);

 #endif  // _RECOUNT

   Send_Command( Extern_Reader, DISABLE, NULL);
   Send_Command( Extern_Reader, EJECT, NULL);

   if( Reader == Intern_Reader)
      Error_Exit( M_Index1);
   else
      Error_Exit( M_Index2);

#endif  // _APPLICATION_URN

#ifdef _APPLICATION_MAV

   if( Reader == Intern_Reader)
      Red_Light_On( M_Index1, NOT_FATAL);
   else
      Red_Light_On( M_Index2, NOT_FATAL);

#endif  // _APPLICATION_MAV
}
/*</code>
</function>

<function id="Reader_P_Response"><heading>
  <description>
    <en>processes a reader POSITIVE response and returns the
corresponding card position.</en></description>
  <parameters>
    <parameter>
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's response string</en></description></parameter>
    <parameter type="return">
      <datatype scope="static">CARD_POSITION</datatype>
      <description><en>local function</en></description></parameter></parameters>
  <usage>return Reader_P_Response( Reader_Reply);</usage>
  <checks>if the function was called for a non positive response, or if
the reply code is not recognized, the function returns a
CARD_ERROR value</checks>
  <comment></comment></heading>

<code>*/
static CARD_POSITION Reader_P_Response( char * Reader_Reply)
{
   if( Reader_Reply [0] != 'P')
      return CARD_ERROR;

   if( Reader_Reply [3] == '0' && Reader_Reply [4] == '0')    // no card
      return NO_CARD;
   if( Reader_Reply [3] == '0' && Reader_Reply [4] == '1')    // reader gate
      return SLOT;
   if( Reader_Reply [3] == '0' && Reader_Reply [4] == '2')    // home position
      return HOME;

   return CARD_ERROR;
}
/*</code>
</function>

<function id="Reader_N_Response"><heading>
  <description>
    <en>processes a reader's NEGATIVE response, for a FATAL error the
system is locked otherwise control is left to the program.</en></description>
  <parameters>
    <parameter>
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's response string</en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="return">
      <datatype scope="static">CARD_POSITION</datatype>
      <description><en>local function</en></description></parameter></parameters>
  <usage>return Reader_N_Response( Reader_Reply, Reader);</usage>
  <checks></checks>
  <comment>The messages passed to the Reader_Error_Message function are from
two types, the first talks about a BAD COMMAND and the second
about a READER FAILURE.</comment></heading>

<code>*/
static CARD_POSITION Reader_N_Response( char * Reader_Reply,
                                        Select_Reader * Reader)
{
        if( Reader_Reply [3] == '0' && Reader_Reply [4] == '0')
      Reader_Error_Message( Reader, M_BAD_READER1, M_BAD_READER2);
   else if( Reader_Reply [3] == '0' && Reader_Reply [4] == '1')
      Reader_Error_Message( Reader, M_BAD_READER1, M_BAD_READER2);
   else if( Reader_Reply [3] == '0' && Reader_Reply [4] == '2')
      Reader_Error_Message( Reader, M_INTERNAL_ERROR1, M_INTERNAL_ERROR2);
   else if( Reader_Reply [3] == '0' && Reader_Reply [4] == '4')
      Reader_Error_Message( Reader, M_INTERNAL_ERROR1, M_INTERNAL_ERROR2);
   else if( Reader_Reply [3] == '0' && Reader_Reply [4] == '9')
      Reader_Error_Message( Reader, M_INTERNAL_ERROR1, M_INTERNAL_ERROR2);
   else if( Reader_Reply [3] == '1' && Reader_Reply [4] == '0')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '1' && Reader_Reply [4] == '3')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '1' && Reader_Reply [4] == '4')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '1' && Reader_Reply [4] == '6')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '1' && Reader_Reply [4] == '7')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '2' && Reader_Reply [4] == '0')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '2' && Reader_Reply [4] == '1')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '2' && Reader_Reply [4] == '2')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '2' && Reader_Reply [4] == '3')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '2' && Reader_Reply [4] == '4')
      return CARD_ERROR;
   else if( Reader_Reply [3] == '4' && Reader_Reply [4] == '0')
      return CARD_ERROR;
   else
      Reader_Error_Message( Reader, M_ERR_READ1, M_ERR_READ2);

   return CARD_ERROR;
}
//</code>
//</function>

//==============================================================================
//======================= R E A D E R   R O U T I N E S ========================
//==============================================================================

/*<function id="Reader_Card_Position"><heading>
  <description>
    <en>request the reader for card position</en></description>
  <parameters>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>if( Reader_Card_Position( Intern_Reader) == HOME)</usage>
  <checks>the reader replies are checked</checks>
  <comment></comment></heading>

<code>*/
CARD_POSITION Reader_Card_Position( Select_Reader * Reader)
{
#ifdef _NOREADER
   return NO_CARD;
#endif

   Reader_Reply = Send_Command( Reader, STATUSREQUEST, NULL);

   if( Reader_Reply [0] == 'N')
      return Reader_N_Response( Reader_Reply, Reader);

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Type"><heading>
  <description>
    <en>request the reader type (PANASONIC or SANKYO)</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_ERROR</datatype>
      <description><en>if reader type is SANKYO</en></description></parameter>
    <parameter type="return">
      <datatype>NO_CARD / SLOT / HOME</datatype>
      <description><en>if reader type is PANASONIC</en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Type( Extern_Reader) == CARD_ERROR)
   Version=1;
else
   Version=2;</usage>
  <checks>the reader type is checked</checks>
  <comment>Only PANASONIC Reader recognizes this command and will
reply POSITIVE. SANKYO Reader doesn't know this command
and will automatically reply NEGATIVE.</comment></heading>

<code>*/
CARD_POSITION Reader_Type( Select_Reader * Reader)
{
#ifdef _NOREADER
   return NO_CARD;
#endif

   Reader_Reply = Send_Command( Reader, TYPEREQUEST, NULL);

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Unformatted_Card"><heading>
  <description>
    <en>enables the reader to accept every card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if( Reader_Unformatted_Card( Intern_Reader) == HOME)</usage>
  <checks>the reader replies are checked</checks>
  <comment>every card, even a 'carte de tram' is accepted, but the reply
codes point out the problem when trying to do something with it.</comment></heading>

<code>*/
CARD_POSITION Reader_Unformatted_Card( Select_Reader * Reader)
{
#ifdef _NOREADER
   return HOME;
#endif

   Reader_Reply = Send_Command( Reader, ENABLE_READ, NULL);

   if( Reader_Reply [0] == 'N')
      return Reader_N_Response( Reader_Reply, Reader);

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Formatted_Card"><heading>
  <description>
    <en>enables the reader to accept formatted cards</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if( Reader_Formatted_Card( Intern_Reader) == HOME)</usage>
  <checks>the reader replies are checked</checks>
  <comment>If the reader has no shutter, he can't disagree unformated cards.</comment></heading>

<code>*/
CARD_POSITION Reader_Formatted_Card( Select_Reader * Reader)
{
#ifdef _NOREADER
   return HOME;
#endif

   Reader_Reply = Send_Command( Reader, ENABLE_FORMATTED_READ, NULL);

// JRU : N??99 might be a result of 'playing' with the shutter (in, out, ...)
// JRU : because of an electric peak (3A, which is greater than allowed -> 1,7A)
// JRU : We force to INITIALIZE reader !
   if( !strcmp(Reader_Reply, "N??99") ||                      // JRU : WatchDog
      !strcmp(Reader_Reply, "Pz")      )                     // JRU : WatchDog
     Reader_Reply = Send_Command( Reader, INITIALIZE, NULL); // JRU : WatchDog

   if( Reader_Reply [0] == 'N')
     return Reader_N_Response( Reader_Reply, Reader);

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Request_Unformatted"><heading>
  <description>
    <en>waits for a card, any kind of card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Request_Unformatted( Intern_Reader) != HOME)
   Error();</usage>
  <checks>the reader replies are checked, if HOME is the response we have
a card in the reader, if CARD_ERROR was the response we have an
error, these two responses causes a loop break. Every other
response (NO_CARD, SLOT) causes the function to continue looping.</checks>
  <comment>It first enables the reader in card acceptance then when the
program is the MAV, the function first checks the lightpen's
presence, then it check if there is a card in the reader.</comment></heading>

<code>*/
CARD_POSITION Reader_Request_Unformatted( Select_Reader * Reader)
{
#ifdef _NOREADER
   return HOME;
#endif

   Reader_Unformatted_Card( Reader);                      // enable the reader

   for(;;)
   {
#ifdef _APPLICATION_MAV

      delay( 20);
      Test_Lightpen();                                    // test the lightpen

#endif  // _APPLICATION_MAV

      CARD_POSITION Ret = Reader_Unformatted_Card( Reader); // ask unformatted

      if( Ret != NO_CARD && Ret != SLOT)   // HOME or CARD_ERROR cause a break
         return Ret;
   }
}
/*</code>
</function>

<function id="Reader_Request_Formatted"><heading>
  <description>
    <en>waits for a formatted card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Request_Formatted( Intern_Reader) != HOME)
   Error();</usage>
  <checks>the reader replies are checked, if HOME is the response we have
a card in the reader, if CARD_ERROR was the response we have an
error, these two responses causes a loop break. Every other
response (NO_CARD, SLOT) causes the function to continue looping.</checks>
  <comment>It first enables the reader in card acceptance then when the
program is the MAV, the function first checks the lightpen's
presence, then it check if there is a card in the reader.</comment></heading>

<code>*/
CARD_POSITION Reader_Request_Formatted( Select_Reader * Reader)
{
#ifdef _NOREADER
   return HOME;
#endif

   Reader_Formatted_Card( Reader);                        // enable the reader

   for(;;)
   {
#ifdef _APPLICATION_MAV

      delay( 20);
      Test_Lightpen();                                    // test the lightpen

#endif  // _APPLICATION_MAV

      CARD_POSITION Ret = Reader_Formatted_Card( Reader);     // ask formatted

      if( Ret != NO_CARD && Ret != SLOT)   // HOME or CARD_ERROR cause a break
         return Ret;
   }
}
/*</code>
</function>

<function id="Reader_Write_Card"><heading>
  <description>
    <en>writes a magnetic card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter>
      <datatype>char*</datatype>
      <name>Data</name>
      <description><en>pointer to the string to write</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Write_Card( Intern_Reader, "abcd") != HOME)
   Error();</usage>
  <checks>the reader replies are checked, if the write was successfull the
function returns HOME</checks>
  <comment></comment></heading>

<code>*/
CARD_POSITION Reader_Write_Card (
   Select_Reader*    Reader,
   char*             Data
   )
{
   CARD_POSITION  Status;
   BYTE           ReadCardData[MAX_CARD_BUF+1];

#ifdef _NOREADER
   return HOME;
#endif

   Reader_Reply = Send_Command( Reader, WRITE, Data);

   // 20040927 avn - check what has been written ( are we getting paranoid?)
   if ( Reader_Reply [0] == 'N')
   {
      Status = Reader_N_Response( Reader_Reply, Reader);
   }
   else
   {
      Status = Reader_P_Response( Reader_Reply);
   }
   if ( HOME == Status)
   {
      Status = Reader_Physical_Read_Card( Intern_Reader, ReadCardData);
      if ( HOME == Status)
      {
         if ( strcmp( ReadCardData, Data))
         {
            Status = CARD_ERROR;
         }
      }
   }

   return Status;
}
/*</code>
</function>

<function id="Reader_Physical_Read_Card"><heading>
  <description>
    <en>reads a magnetic card also those not entered with ENABLE</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter>
      <datatype>char*</datatype>
      <name>Data</name>
      <description><en>pointer to the string to read, if equal to NULL, no data will be
returned.</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Physical_Read_Card( Intern_Reader, Buffer) != HOME)
   Error();</usage>
  <checks>the reader replies are checked, if the read was successfull the
function returns HOME</checks>
  <comment>
Control is left to the real read, he will detect if the card
is formatted, but if it is not it will take more time due to
the reader retries.
The reader returns the read data in the Reader_Reply string, so
the data begins after the reply code at position 5 of
Reader_Reply.
This function can be used as initialisation of the reader,
because the Reader_*Formatted_Card function does not read the
card when it was in place before the mentioned call.

This routine does 5 retrys
It gives priority to reading not to speed.</comment></heading>

<code>*/
CARD_POSITION Reader_Physical_Read_Card( Select_Reader * Reader, char * Data)
{
#ifdef _NOREADER
   extern char Blanco_Card [];
   if ( Data)
   {
      // provide some coherent data
      memcpy( Data, Blanco_Card, MAX_CARD_BUF+1);
   }

   return SUCCEEDED;
#endif

   Reader_Reply = Send_Command( Reader, READ, NULL);

   if( Reader_Reply [0] == 'N')
      return Reader_N_Response( Reader_Reply, Reader);

   if( Data)
     memcpy( Data, Reader_Reply + 5, strlen( Reader_Reply) - 4 );

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Read_Card"><heading>
  <description>
    <en>reads a magnetic card</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter>
      <datatype>char*</datatype>
      <name>Data</name>
      <description><en>pointer to the string to read</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
if ( Reader_Read_Card( Intern_Reader, Buffer) != HOME)
   Error();</usage>
  <checks>the reader replies are checked, if the read was successfull the
function returns HOME</checks>
  <comment>
With the MCT145_1A, the STATUSREQUEST_READ detects if the
card is formatted. If the card is not formatted, the function
directly returns. For all the other readers, the control is left
to the real read, he will detect if the card is formatted, but if
it is not it will take more time due to the reader retries.
The reader returns the read data in the Reader_Reply string, so
the data begins after the reply code at position 5 of
Reader_Reply.

This routine does no retrys it gives priority to speed</comment></heading>

<code>*/
CARD_POSITION Reader_Read_Card( Select_Reader * Reader, char * Data)
{
#ifdef _NOREADER
   extern char Blanco_Card [];

   // provide some coherent data
   memcpy( Data, Blanco_Card, MAX_CARD_BUF+1);

   return SUCCEEDED;
#endif

   // test if the card is formatted MCT145_1A

   Reader_Reply = Send_Command( Reader, STATUSREQUEST_READ, NULL);

   if( Reader_Reply [0] == 'N')
      return Reader_N_Response( Reader_Reply, Reader);

   // perform the read
   return( Reader_Physical_Read_Card( Reader,Data));
}
//</code>
//</function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reader_Move_Card"><heading>
  <description>
    <en>moves the card in the reader</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Move_Card( Intern_Reader);</usage>
  <checks></checks>
  <comment>
this function is only used by the MAV after a card is
introduced in the reader. By moving the card we bring it on the
reader gate so the users see there is a card in the reader.</comment></heading>

<code>*/
void Reader_Move_Card( Select_Reader * Reader)
{
   Reader_Reply = Send_Command( Reader, MOVE, NULL);

   if( Reader_Reply [0] == 'N')
      Reader_Error_Message( Reader, M_ERR_READ1, M_ERR_READ2);
}
//</code>
//</function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif // _APPLICATION_MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reader_Eject_Card"><heading>
  <description>
    <en>Reader_Eject_Card disables the reader, ejects the card and makes
some retries if it missed the first time.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Eject_Card( Intern_Reader);</usage>
  <checks>the reader replies are checked, if the eject was successfull the
function returns a positive response depending on card position</checks>
  <comment>
If a negative response is send, we make some retries and we try
to move the card, hoping this will resolve the problem or reset
the readers logic.
There is no need to test the Reader_Eject_Card return code
because if after all the retries the reader still respond with
a negative response the system is locked. The return code is
there only to provide some coherence.
The reader is disabled before calling Eject, because the reader
can still accept cards after the ejection. For the Only_Eject
function we want to have the reader accepting cards as soon as
possible, so the reader is not disabled. This is used in the URN
for performance reasons.</comment></heading>

<code>*/
CARD_POSITION Reader_Eject_Card( Select_Reader * Reader)
{
   int Retrys = 5;

#ifdef _NOREADER
   return NO_CARD;
#endif

   Send_Command( Reader, DISABLE, NULL);

//rvh 22/03/94 for loop instead of while to have one exit point
//             extra test in if:
//                 if retrys reached, no initialise is send anymore
   for( ;;)
   {
      Reader_Reply = Send_Command( Reader, EJECT, NULL);

      if( Reader_Reply [0] == 'P' || Retrys-- <=1 )
         break;

      Send_Command( Reader, INITIALIZE, NULL);         // try to move the card
   }

   if( Reader_Reply [0] == 'N')                                  // we give up
   {
      Reader_N_Response( Reader_Reply, Reader);
      Reader_Error_Message( Reader, M_ERR_READ1, M_ERR_READ2);
   }

   return Reader_P_Response( Reader_Reply);       // we only return on success
}
//</code>
//</function>

//==============================================================================
/*<function id="Reader_Only_Eject_Card"><heading>
  <description>
    <en>Reader_Eject_Card disables the reader, ejects the card and makes
some retries if it missed the first time.
Reader_Only_Eject_Card does the same but does not perform a
preliminary Disable.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Eject_Card( Intern_Reader);</usage>
  <checks>the reader replies are checked, if the eject was successfull the
function returns a positive response depending on card position</checks>
  <comment>
If a negative response is send, we make some retries and we try
to move the card, hoping this will resolve the problem or reset
the readers logic.
There is no need to test the Reader_Eject_Card return code
because if after all the retries the reader still respond with
a negative response the system is locked. The return code is
there only to provide some coherence.
The reader is disabled before calling Eject, because the reader
can still accept cards after the ejection. For the Only_Eject
function we want to have the reader accepting cards as soon as
possible, so the reader is not disabled. This is used in the URN
for performance reasons.</comment></heading>

<code>*/
CARD_POSITION Reader_Only_Eject_Card( Select_Reader * Reader)
{
   int Retrys = 5;

#ifdef _NOREADER
   return NO_CARD;
#endif

//rvh 22/03/94 for loop instead of while to have one exit point
//             extra test in if:
//                 if retrys reached, no initialise is send anymore
   for( ;;)
   {
      Reader_Reply = Send_Command( Reader, EJECT, NULL);

      if( Reader_Reply [0] == 'P' || Retrys-- <=1 )
        break;

      Send_Command( Reader, INITIALIZE, NULL);          // try to move the card
   }

   if( Reader_Reply [0] == 'N')                                   // we give up
   {
      Reader_N_Response( Reader_Reply, Reader);
      Reader_Error_Message( Reader, M_ERR_READ1, M_ERR_READ2);
   }

   return Reader_P_Response( Reader_Reply);       // we only return on success
}
/*</code>
</function>

<function id="Reader_Disable"><heading>
  <description>
    <en>disables the reader, he will no longer accept cards</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Disable( Intern_Reader);</usage>
  <checks>the reader replies are checked</checks>
  <comment>
there is no need to test the function's return code. The function
return a code only for coherence.</comment></heading>

<code>*/
CARD_POSITION Reader_Disable( Select_Reader * Reader)
{
#ifdef _NOREADER
   return NO_CARD;
#endif

   Reader_Reply = Send_Command( Reader, DISABLE, NULL);

   if( Reader_Reply [0] == 'N')
      return Reader_N_Response( Reader_Reply, Reader);

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Reader_Wait_Card_Removed"><heading>
  <description>
    <en>waits until an ejected card is removed, it reejects the card
after 10 seconds</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Eject_Card( Intern_Reader);
Reader_Wait_Card_Removed( Intern_Reader);</usage>
  <checks>the reader replies are not checked here, the error processing
is left to Reader_Eject_Card that will make the retries and lock
the machine if the problem persist.</checks>
  <comment>
This function must be called after an eject.
If the card is not removed after 10 seconds or if there is a
CARD_ERROR, the card is reejected (because the user may have
pushed the card into the reader).
This function can only end with success or with a system lock
if the Eject fails.</comment></heading>

<code>*/
void Reader_Wait_Card_Removed( Select_Reader * Reader)
{
   time_t t_init = time( NULL);

   for(;;)
   {
      switch( Reader_Card_Position( Reader))
      {
         case CARD_ERROR :

             Reader_Eject_Card( Reader);
             t_init = time( NULL);
             break;

         case HOME:
         case SLOT:

             if( time(NULL) > t_init + 10)
             {
               Reader_Eject_Card( Reader);
               t_init = time( NULL);
             }
             break;

         case NO_CARD: return;
      }
   }
}
//</code>
//</function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef _APPLICATION_URN       // this code is used in both URN and REC, not MAV
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reader_Take_Card"><heading>
  <description>
    <en>disables the reader, take the card and makes some retries if it
missed the first time.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>CARD_POSITION</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Take_Card( Intern_Reader);</usage>
  <checks>the reader replies are checked, if the take was successfull the
function returns a positive response depending on card position</checks>
  <comment>
If a negative response is send, we make some retries and we try
to move the card, hoping this will resolve the problem or reset
the readers logic.
There is no need to test the Reader_Take_Card return code
because if after all the retries the reader still respond with
a negative response the system is locked. The return code is
there only to provide some coherence.
The reader is disabled before calling Take, because the reader
can still accept cards after the take.</comment></heading>

<code>*/
CARD_POSITION Reader_Take_Card( Select_Reader * Reader)
{
   int Retrys = 1;

#ifdef _NOREADER
   return NO_CARD;
#endif

   Send_Command( Reader, DISABLE, NULL);

//rvh 22/03/94 for loop instead of while to have one exit point
//             extra test in if:
//                 if retrys reached, no initialise is send anymore
   for( ;;)
   {
      Reader_Reply = Send_Command( Reader, CAPTURE, NULL);

      if( Reader_Reply [0] == 'P' || Retrys-- <=1 )
         break;

      Send_Command( Reader, INITIALIZE, NULL);               // try to move the card
   }

   if( Reader_Reply [0] == 'N')                                   // we give up
//rvh-ebe
       return CARD_ERROR;

   return Reader_P_Response( Reader_Reply);
}
/*</code>
</function>

<function id="Error_Procedure"><heading>
  <description>
    <en>Error procedure called when an error occcures in Process_Card_In_Bac().</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Error_Procedure()</usage>
  <checks>the reader replies are not checked</checks>
  <comment>
rvh-ebe  24/03/94 author ebe</comment></heading>

<code>*/
void Error_Procedure( Select_Reader * Reader)
{
      Reader_N_Response( Reader_Reply, Reader);
      Reader_Error_Message( Reader, M_ERR_READ1, M_ERR_READ2);
}
//</code>
//</function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifndef _RECOUNT                               // this code is used in REC only
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Reader_Deformat_Card"><heading>
  <description>
    <en>write some data on the card, so it can not be used for voting
without being reformated</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
      <description><en></en></description></parameter>
    <parameter>
      <datatype>Select_Reader*</datatype>
      <name>Reader</name>
      <description><en>pointer to the reader</en></description></parameter>
    <parameter type="global">
      <datatype>char*</datatype>
      <name>Reader_Reply</name>
      <description><en>the reader's reply string</en></description></parameter></parameters>
  <usage>
Reader_Deformat_Card( Intern_Reader);</usage>
  <checks>the reader replies are not checked</checks>
  <comment>
the card is completely overwritten with '0' because this can't
be a valid vote</comment></heading>

<code>*/
void Reader_Deformat_Card (
   Select_Reader* Reader
   )
{
#ifdef _NOREADER
   return;
#endif

   char Buffer[MAX_CARD_BUF+1];

   memset( Buffer, '0', sizeof( Buffer));
   Buffer[MAX_CARD_BUF] = NULL;

   Reader_Write_Card( Reader, Buffer);
}
//</code>
//</function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _RECOUNT
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // _APPLICATION_URN
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Check_Reader_Type">
<heading>
  <description>
    <en>Checks the external reader generation.</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment>The system locks up if an old generation reader is detected.
  Old readers are obsolete as off october 2006.</comment>
  <author>avn</author>
</heading>
<code>*/
#ifdef _APPLICATION_URN       // this code is used in both URN and REC, not MAV
// ---------------------------------------------------------------------------
void Check_Reader_Type (
   TYPE_READER Extern_Type    // reader type
   )
// ---------------------------------------------------------------------------
{
   if ( Reader_Type( Extern_Reader) == CARD_ERROR)
   {
      // all readers will respond negative if power was lost, so we have to
      // investigate a bit further by initializing the reader
      if ( ( Extern_Reader = Open_Command( Extern_Type, COM1)) == NULL)
      {
         Error_Exit( M_ERR_READ2);
      }
      else
      {
         // reader initialized, ask type again
         if ( Reader_Type( Extern_Reader) == CARD_ERROR)
         {
            // old reader generation detected
            Error_Exit( FE_BAD_EXT_READER);
         }
      }
   }

   // reader responded correctly, ok to proceed
   return;
}
#endif  // _APPLICATION_URN
//</code></function>

//=== GENCARD.CPP =============================================================
//</body>
//</source>