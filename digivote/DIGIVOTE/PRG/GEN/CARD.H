/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="card.h">

<comments>
  <fileName>CARD.H</fileName>
  <creationDate>27/04/93</creationDate>

  <rights></rights>
  <author>B.S.S.</author>

  <description>
    <fr>Fichier d'entête card.cpp</fr>
    <nl>Header file card.cpp</nl></description>
  <history></history>
  <comment></comment>
</comments>

<header>*/

#ifndef _CARD                         // avoid mutiple inclusions of this file
   #define _CARD

//============================= D E F I N E S =================================

#define TIME_OUT_ERROR          0x8000
#define FRAMING_ERROR           0x800
#define PARITY_ERROR            0x400
#define OVERRUN_ERROR           0x200

//============================ T Y P E D E F S ================================

typedef enum { COM1,
            COM2,
            UNKNOWN_PORT } PORT;
// 10/03/94: MCT145_1A_DEMO = 500 added to TYPE_READER - author: EBE
typedef enum { MCT145_1R,
            MCT145_1A,
            MCT345,
            MCT228,
            UNKNOWN_TYPE,
            MCT145_1A_DEMO = 500 } TYPE_READER;

typedef enum { NOT_USABLE,
            USABLE } READER_STATE;

typedef enum { INITIALIZE,
            STATUSREQUEST,
            STATUSREQUEST_WRITE,
            STATUSREQUEST_READ,
            EJECT,
            CAPTURE,
            READ,
            WRITE,
            ENABLE_READ,
            ENABLE_WRITE,
            ENABLE_FORMATTED_READ,
            DISABLE,
            MOVE,
            TYPEREQUEST } COMMAND;

//-----------------------------------------------------------------------------

typedef struct Select_Reader
{
   struct {

    char * ( * Procedure)( Select_Reader *, char *);  // Function name
    char * Parameter;                                 // Parameters

    } Commands [14];

   PORT ComPort;               // COMPORT
   TYPE_READER Type_Reader;
   READER_STATE State;

} Select_Reader;

//-----------------------------------------------------------------------------

/*[*/
/*       Description of the command array
       ================================

          0: Initialize
          1: StatusRequest
          2: StatusRequest_Write
          3: StatusRequest_Read
          4: Eject
          5: Capture
          6: Read
          7: Write
          8: Enable every card
          9: Enable every card and write on entrance
          10: Enable formatted card
          11: Disable
          12: Move
          13: TypeRequest
*/
/*]*/


//=========================== P R O T O T Y P E S =============================
//<prototypes>
Select_Reader * Open_Command( TYPE_READER Type_Reader, PORT Port);
char * Send_Command( Select_Reader * Reader, COMMAND Command, char Data []);
void Close_Command( Select_Reader ** Reader);

void Print_Trace_Buffer( FILE * FileID);
unsigned int Get_Retry_Counter( void);
int Supports_Command( Select_Reader *Reader, COMMAND Command);

void SetRetryModeOn( void);
void SetRetryModeOff( void);
//</prototypes>

#endif                                                 // end of #ifndef _CARD
//</header>
//=== CARD.H ==================================================================
//</source><!-- eof -->