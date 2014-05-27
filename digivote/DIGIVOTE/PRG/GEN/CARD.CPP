/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="card.cpp">

<comments>
  <fileName>CARD.CPP</fileName>
  <creationDate>27/04/93</creationDate>

  <rights></rights>
  <author>B.S.S.</author>

  <description>
    <fr>Librairie de fonctions lecteur de cartes</fr>
    <nl>Functielibrary kaartlezer</nl></description>
  <history>
   Revision History
   ################

   01/06/93: revision history starting date
   16/07/93: tracing added + printing of trace buffer
             RETRY is incremented from 2 till 5
             an extra 100msecs is waited before resending the same command.
   20/09/93: Routine Supports_Command is added.
             RETRY_COMMAND and RETRY_ENQ equal to 1 when TRACE is defined.
             The routine Get_Retry_Counter is added.
   01/10/93: The Standard_Command function was completely rewritten to
             obtain the fastest possible read speed. We do no longer use the
             COM port as a file because the read function reenables the
             interrupts which can cause the lost of some characters; we now
             use the _bios_serialcom function instead. All the concerned code
             was updated accordingly.
   19/10/93: Functions where added to disable all the interrupts that could
             cause a lost of character when reading the COM port (the most
             problematic one was caused by the lightpen driver because we
             thought our problem was elsewhere)
   19/10/93: Bug fixed, when no reader is connected, writing to the reader
             took some time because we had to wait until all the timeout
             where complete. There was also a similar problem if no 'etx'
             character was received.
   20/10/93: A 'State' flag was added to Select_Reader so we don't send any
             supplemental command to the reader if he is stuck. (N??03 error)
             It makes no sense to try to eject a card in that case for
             example ... this wins 3 commands (disable, eject, disable that
             are automaticaly send for other errors) and one try duration is
             now 3 sec either than 10 sec.
   26/10/93: Bug fixed in Supports_Command, test on NULL value was not correct.</history>
 <comment>
    Functions for the handling of the cardreader</comment>
</comments>

<header>*/

//======================== I N C L U D E   F I L E S ==========================
//<includes>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <time.h>

#include "..\gen\card.h"

#define _DEBUGMODE      FALSE      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//=============================================================================

/*[*/
/*       Transmission/Control specification for the card readers
         =======================================================

    1) Synchronous method    : asynchronous
     2) Transmission method   : half duplex
     3) Baud rate             : 1200/2400/4800/9600 bps (automatic recognition)
     4) data length           : 7 bit + 1 parity
     5) stop bit              : 1 bit
     6) character code        : ascii
     7) parity check method   : horizontal (even), vertical (even)

      | ST | B0 | B1 | B2 | B3 | B4 | B5 | B6 |  P | SP |

     Readers function by command/response method. MCRW executes a particular
     operation according to text (command) received from HOST, then reports
     result of execution to HOST.
     The TCC (transmission control characters) are defined below.

                           Command/Response structure
                           ==========================

            | S |                                    | E | B | C |
            | T |   Command/Response                 | T | C | R |
            | X |                                    | X | C |   |
                 <-              BCC range             ->

        the BCC and CR are omissible, according to initial mode select
        the BCC is the exclusive OR of all the bytes comprised in the
        BCC range

     Please refer to card reader documentation for transmission protocol
     description and timing chart.
*/
/*]*/

//============================= D E F I N E S =================================

#define SETTINGS (_COM_9600 | _COM_CHR7 | _COM_STOP1 | _COM_EVENPARITY)

//=========================== C O N S T A N T S ===============================

// transmission control characters
const char stx = 2;     // indicate start of text   (02h)
const char etx = 3;     // indicate end of text     (03h)
const char enq = 5;     // request to send response (05h)
const char ack = 6;     // send positive response   (06h)
const char nak = 21;    // send negative response   (15h)
const char del = 16;    // clear the line           (10h)
const char eot = 4;     // and reset MCRW           (04h)
const char CR  = 0x0D;  // carriage return

//====================== L O C A L   F U N C T I O N S ========================
//<prototypes>
static void Disable_Lightpen( void);
static void Enable_Lightpen( void);
static void Write_To_Reader( int Port, char * String, int Len);
static char * Standard_Command( struct Select_Reader * Reader, char * Command);
static char * Write_Command_C73( struct Select_Reader * Reader, char * Data);
// new function: Write_Command_Dummy (for Demo-version MAV only)
// date: 16/08/1994 - author: EBE
static char * Write_Command_Dummy( Select_Reader * Reader, char * Data) ;
// end declaration new function: Write_Command_Dummy
static char * Enable_Write_Command_C78_C24( struct Select_Reader * Reader, char * Data);
static char * Status_Write_Command_C78_C12( struct Select_Reader * Reader, char * Data);
static char * Status_Read_Command( Select_Reader * Reader, char * Data);
static char * Enable_Command_MCT228( struct Select_Reader * Reader, char * Data);
static char * Enable_Formatted_Command_MCT228( struct Select_Reader * Reader, char * Data);
static char * Impossible_Command( struct Select_Reader * Dummy1,char * Dummy2);
//</prototypes>

//===================== G L O B A L   V A R I A B L E S =======================

// we found the number of clock ticks since midnight at the 40:6C address
// in memory (found in helppc in the Bios Data Area description
unsigned long * clock_ticks = (unsigned long *) MK_FP( 0x40, 0x6c);

#ifndef _APPLICATION_MAV
   #ifndef DIAGNOST
      // JRU : WatchDog for Card Readers
      int WatchSign = -1;
      const char WatchSigns[5] = {" _ÜÛÜ"};
      // JRU
   #endif

   extern int Version;
#endif

#ifdef _TEST
  int gRETRY_COMMAND = 1;
  int gRETRY_ENQ     = 1;
#else
  int gRETRY_COMMAND = 5;
  int gRETRY_ENQ     = 2;
#endif

//=============================================================================
//             R E A D E R   F U N C T I O N S   D E F I N I T I O N
//=============================================================================

Select_Reader MCT145_1R_Reader =
{
   Standard_Command,"C023240200120",   // INITIALIZE
   Standard_Command,"C10",             // STATUSREQUEST
   Impossible_Command,NULL,            // STATUSREQUEST_WRITE
   Status_Read_Command,NULL,           // STATUSREQUEST_READ
   Standard_Command,"C30",             // EJECT
   Standard_Command,"C31000",          // CAPTURE
   Standard_Command,"C63",             // READ
   Impossible_Command,NULL,            // WRITE
   Standard_Command,"C:0",             // ENABLE_READ
   Impossible_Command,NULL,            // ENABLE_WRITE
   Standard_Command,"C:2",             // ENABLE_FORMATTED_READ
   Standard_Command,"C:1",             // DISABLE
   Standard_Command,"C60",             // MOVE
   Standard_Command,"C?0",             // TYPEREQUEST
   UNKNOWN_PORT,                       // ComPort
   MCT145_1R,                          // Type_Reader
   USABLE                              // State
};

Select_Reader MCT145_1A_Reader =
{
   Standard_Command,"C023240200120",   // INITIALIZE
   Standard_Command,"C10",             // STATUSREQUEST
   Status_Write_Command_C78_C12,NULL,  // STATUSREQUEST_WRITE
   Standard_Command,"C13",             // STATUSREQUEST_READ
   Standard_Command,"C30",             // EJECT
   Standard_Command,"C31000",          // CAPTURE
   Standard_Command,"C63",             // READ
   Write_Command_C73,NULL,             // WRITE
   Standard_Command,"C:0",             // ENABLE_READ
   Enable_Write_Command_C78_C24,NULL,  // ENABLE_WRITE
   Standard_Command,"C:0",             // ENABLE_FORMATTED_READ = ENABLE_READ
   Standard_Command,"C:1",             // DISABLE
   Standard_Command,"C60",             // MOVE
   Standard_Command,"C?0",             // TYPEREQUEST
   UNKNOWN_PORT,                       // ComPort
   MCT145_1A,                          // Type_Reader
   USABLE                              // State
};

Select_Reader MCT345_Reader =
{
   Standard_Command,"C023240200120",   // INITIALIZE
   Standard_Command,"C10",             // STATUSREQUEST
   Impossible_Command,NULL,            // STATUSREQUEST_WRITE
   Status_Read_Command,NULL,           // STATUSREQUEST_READ
   Standard_Command,"C30",             // EJECT
   Standard_Command,"C31000",          // CAPTURE
   Standard_Command,"C63",             // READ
   Write_Command_C73,NULL,             // WRITE
   Standard_Command,"C:0",             // ENABLE_READ
   Impossible_Command,NULL,            // ENABLE_WRITE
   Standard_Command,"C:2",             // ENABLE_FORMATTED_READ
   Standard_Command,"C:1",             // DISABLE
   Standard_Command,"C60",             // MOVE
   Standard_Command,"C?0",             // TYPEREQUEST
   UNKNOWN_PORT,                       // ComPort
   MCT345,                             // Type_Reader
   USABLE                              // State
};

Select_Reader MCT228_Reader=
{
   Standard_Command,"C0222",             // INITIALIZE
   Standard_Command,"C10",               // STATUSREQUEST
   Impossible_Command,NULL,              // STATUSREQUEST_WRITE
   Status_Read_Command,NULL,             // STATUSREQUEST_READ
   Standard_Command,"C30",               // EJECT
   Standard_Command,"C31",               // CAPTURE
   Standard_Command,"C63",               // READ
   Impossible_Command,NULL,              // WRITE
   Enable_Command_MCT228,NULL,           // ENABLE_READ
   Impossible_Command,NULL,              // ENABLE_WRITE
   Enable_Formatted_Command_MCT228,NULL, // ENABLE_FORMATTED_READ
   Standard_Command,"C:1",               // DISABLE
   Standard_Command,"C60",               // MOVE
   Standard_Command,"C?0",               // TYPEREQUEST
   UNKNOWN_PORT,                         // ComPort
   MCT228,                               // Type_Reader
   USABLE                                // State
};

// MCT145_1A_Demo_Reader: new type of card-reader destined to be used
// with the demo-version of the program
// The only difference with the original reader (MCT145_1A_Reader) is:
// MCT145_1A_Demo_Reader is unable to write on the card
// Write_Command_C73 has been replaced with Write_Command_Dummy
// !!!! This dummy-reader may only be used in the MAV !!!!!
// Date: 18/08/1994 - author: EBE

Select_Reader MCT145_1A_Demo_Reader =
{
   Standard_Command,"C023240200120",   // INITIALIZE
   Standard_Command,"C10",             // STATUSREQUEST
   Status_Write_Command_C78_C12,NULL,  // STATUSREQUEST_WRITE
   Standard_Command,"C13",             // STATUSREQUEST_READ
   Standard_Command,"C30",             // EJECT
   Standard_Command,"C31000",          // CAPTURE
   Standard_Command,"C63",             // READ
   Write_Command_Dummy,NULL,           // SIMULATE WRITE
   Standard_Command,"C:0",             // ENABLE_READ
   Enable_Write_Command_C78_C24,NULL,  // ENABLE_WRITE
   Standard_Command,"C:0",             // ENABLE_FORMATTED_READ = ENABLE_READ
   Standard_Command,"C:1",             // DISABLE
   Standard_Command,"C60",             // MOVE
   Standard_Command,"C?0",             // TYPEREQUEST
   UNKNOWN_PORT,                       // ComPort
   MCT145_1A_DEMO,                          // Type_Reader
   USABLE                              // State
};


static unsigned int RetryCounter = 0;      // Is incremented with each retry
                                           // used in the diagnostic prog

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TRACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct CardReaderCommandTrace
{
   int           ComPort;                  // Com port where to send command
   char          Command [2];              // The two characters following 'C'
   unsigned long TimeCommand;              // Time the command is send
   char          Acknowledge;              // Which character is Acknowledge ?
   unsigned long TimeAck;                  // Time the Acknowledge char is received
   unsigned long TimeEnquires [2];         // Time an enquire is send ( max gRETRY_ENQ)
   char          Response [7];             // The 7 first chars of the response
   unsigned int  FunctionResponse;         // The last value returned by _bios...
   unsigned long TimeResponse;             // Time the response is received
   char          Bcc;                      // Bcc sould be zero
};

const int NR_TRACES = 50;                  // Number of send command to retain

static struct CardReaderCommandTrace TraceBuffer [NR_TRACES] = {0};
static struct CardReaderCommandTrace EmptyCommandTrace = {1};
static unsigned int LastTrace = 0;         // last trace position in the array

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // TRACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//</header>
//================================= B O D Y ===================================
//<body>

/*<function id="Get_Retry_Counter">
<heading>
  <description><en>
    Returns the number of retries.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>unsigned int</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
    <parameter type="global">
      <datatype scope="static">unsigned int</datatype><name>RetryCounter</name></parameter>
  </parameters>
  <usage>
    Counter = Get_Retry_Counter ()</usage>
  <comment>
    With each retry of command of enquiry, this counter is
    incremented.
    This function is only used in the diagnostic program.</comment>
</heading>
<code>*/
unsigned int Get_Retry_Counter( void)
{
   return( RetryCounter);
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef TRACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Print_Trace_Buffer">
<heading>
  <description><en>
    Print interpreted TraceBuffer</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>FILE*</datatype><name>FileID</name>
      <description><en>the output stream where to write the trace</en></description></parameter>
    <parameter type="global">
      <datatype scope="static">unsigned int</datatype><name>LastTrace</name>
      <description><en>last trace position</en></description></parameter>
    <parameter type="global">
      <datatype>const int</datatype><name>NR_TRACES</name>
      <description><en>size of the trace buffer</en></description></parameter>
  </parameters>
  <usage>
    Print_Trace_Buffer ( stdout);</usage>
  <comment>
    The trace buffer is a circular buffer, so the LastTrace value +2
    indicates the beginning of the buffer and LastTrace +1 the end.
    This is only true if the buffer is completely filled at least
    one time, otherwise the beginning of the buffer is 0. We don't
    process that case, the messages are simply void then.</comment>
</heading>
<code>*/
void Print_Trace_Buffer( FILE * FileID)
{

   fprintf( FileID, "\n\n# TRACE OUTPUT of the %d last commands", NR_TRACES - 1);
   fprintf( FileID, " sent to the card readers\n\n", NR_TRACES - 1);

   // Read the circular buffer completely
   for( int CurrentTrace  = ( LastTrace + 2) % NR_TRACES ;
            CurrentTrace != ( LastTrace + 1) % NR_TRACES;
            CurrentTrace  = ++CurrentTrace % NR_TRACES )
   {
      fprintf( FileID, "# %s Cmd=%2.2sú%lu Ack=%dú%lu Enq=",
                       TraceBuffer [CurrentTrace].ComPort ? "INT" : "EXT",
                       TraceBuffer [CurrentTrace].Command,
                    TraceBuffer [CurrentTrace].TimeCommand,
                       TraceBuffer [CurrentTrace].Acknowledge,
                       TraceBuffer [CurrentTrace].TimeAck );

      for( int i=0; i < gRETRY_ENQ; i++)
          fprintf( FileID, "%lu ", TraceBuffer [CurrentTrace].TimeEnquires [i]);

      fprintf( FileID, "Resp=%7.7s(%x)ú%lu bcc=%s\n",
                       TraceBuffer [CurrentTrace].Response,
                       TraceBuffer [CurrentTrace].FunctionResponse,
                       TraceBuffer [CurrentTrace].TimeResponse,
                       TraceBuffer [CurrentTrace].Bcc ? "FAULT" : "OK");
   }
}
//</code></function>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#endif  // TRACE
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*<function id="Disable_Lightpen">
<heading>
  <description><en>
    disables the lightpen driver</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
 </parameters>
  <usage>
    Disable_Lightpen();</usage>
  <comment>
    These functions first save the flags, then disables or enables
    the lightpen driver and then restore the flags.
    These functions are only effective for the MAV program because
    it is the only one that uses the lightpen driver.
    With this method, we don't have to know the IRQ used on the
    ligthpens card; the pxl-bios knows the used IRQ. In fact we
    simply desactivate the pxl-bios program.
    Masking the IRQ 10 is not general, and did'nt even work. The
    solution was to mask IRQ2 (slave 8259) with outp(0x21,0x04).
    As we did'nt know why this works and not simply masking IRQ10,
    we decided not to use that solution.</comment>
</heading>
<code>*/
static void Disable_Lightpen( void)
{
   asm {

       push bx
       push cx
       push dx

       mov  ah,4
       mov  bx,'FT'
       mov  cx,'GD'
       mov  dx,'OF'
       int  10h

       pop dx
       pop cx
       pop bx
   }
}
//</code></function>

/*<function id="Enable_Lightpen">
<heading>
  <description><en>
    enables the lightpen driver</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
 </parameters>
  <usage>
    Enable_Lightpen();</usage>
  <comment>
    These functions first save the flags, then disables or enables
    the lightpen driver and then restore the flags.
    These functions are only effective for the MAV program because
    it is the only one that uses the lightpen driver.
    With this method, we don't have to know the IRQ used on the
    ligthpens card; the pxl-bios knows the used IRQ. In fact we
    simply desactivate the pxl-bios program.
    Masking the IRQ 10 is not general, and did'nt even work. The
    solution was to mask IRQ2 (slave 8259) with outp(0x21,0x04).
    As we did'nt know why this works and not simply masking IRQ10,
    we decided not to use that solution.</comment>
</heading>
<code>*/
static void Enable_Lightpen( void)
{
   asm {

       push bx
       push cx
       push dx

       mov  ah,4
       mov  bx,'FT'
       mov  cx,'GD'
       mov  dx,'ON'
       int  10h

       pop dx
       pop cx
       pop bx
   }
}
//</code></function>

/*<function id="Disable_Keyboard">
<heading>
  <description><en>
    Disables and enables the keyboard by changing the 8259
    programmable interrup controller</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Disable_Keyboard();</usage>
  <comment>
    The 8259 is a programmable interrupt controller (PIC) that is
    located at port 21h. It controls IRQ0 to IRQ7. By setting the
    operation control word/interrupt mask properly, we can disable
    the keyboard's hardware interrupt. The keyboard uses the
    hardware IRQ1 interrupt. When the corresponding bit is to 1 in
    the mask it means thaht the interrupt is disabled, when it is 0
    the corresponding IRQ is active. So using 2 as mask will disable
    the IRQ1 and leave all the other IRQs active.
    Found in HelpPc in the 8259 description.</comment>
</heading>
<code>*/
static void Disable_Keyboard( void)
{
   outp( 0x21, 0x02);
}
//</code></function>

/*<function id="Enable_Keyboard">
<heading>
  <description><en>
    Disables and enables the keyboard by changing the 8259
    programmable interrup controller</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Enable_Keyboard();</usage>
  <comment>
    The 8259 is a programmable interrupt controller (PIC) that is
    located at port 21h. It controls IRQ0 to IRQ7. By setting the
    operation control word/interrupt mask properly, we can disable
    the keyboard's hardware interrupt. The keyboard uses the
    hardware IRQ1 interrupt. When the corresponding bit is to 1 in
    the mask it means thaht the interrupt is disabled, when it is 0
    the corresponding IRQ is active. So using 2 as mask will disable
    the IRQ1 and leave all the other IRQs active.
    Found in HelpPc in the 8259 description.</comment>
</heading>
<code>*/
static void Enable_Keyboard( void)
{
   outp( 0x21, 0x00);
}
//</code></function>

/*<function id="Disable_All">
<heading>
  <description><en>
    Disables and enables all the interrupts that can cause a problem
    while reading the com port</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Disable_All();</usage>
  <comment>
    Disabling the interrupts is not enough because when the
    lightpen interacts with the screen, it generates interrupts
    that are proccessed by the lightpen driver. This take some
    precious time when reading from the COM port. We can have the
    same problem with the keyboard.</comment>
</heading>
<code>*/
static void Disable_All( void)
{
#ifndef _TEST1

   Disable_Keyboard();
   Disable_Lightpen();
   disable();

#endif
}
//</code></function>

/*<function id="Enable_All">
<heading>
  <description><en>
    Disables and enables all the interrupts that can cause a problem
    while reading the com port</en></description>
  <parameters>
    <parameter type="arg">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <usage>
    Enable_All();</usage>
  <comment>
    Disabling the interrupts is not enough because when the
    lightpen interacts with the screen, it generates interrupts
    that are proccessed by the lightpen driver. This take some
    precious time when reading from the COM port. We can have the
    same problem with the keyboard.</comment>
</heading>
<code>*/
static void Enable_All( void)
{
#ifndef _TEST1

   enable();
   Enable_Lightpen();
   Enable_Keyboard();

#endif
}
//</code></function>

/*<function id="Supports_Command">
<heading>
  <description><en>
    Checks if the specified command is supported by the driver.</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>TRUE when command is supported else returns FALSE</en></description></parameter>
    <parameter type="arg">
      <datatype>TYPE_READER</datatype><name>Type_Reader</name>
      <description><en>the reader type</en></description></parameter>
    <parameter type="arg">
      <datatype>COMMAND</datatype><name>Command</name>
      <description><en>the command to check for</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT145_1R_Reader</name>
      <description><en>the readers definitions</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT145_1A_Reader</name></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT345_Reader</name></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT228_Reader</name></parameter>
  </parameters>
  <usage>
    if( Supports_Command ( Intern_Type, WRITE)) ...</usage>
  <comment>
    When this routines returns TRUE, it means that the driver
    supports the specified command.  It does not mean that the
    card reader itself will support this command although it should,
    when the correct driver is specified.</comment>
</heading>
<code>*/
int Supports_Command( Select_Reader *Reader, COMMAND Command)
{
   return( Reader -> Commands [Command].Procedure != Impossible_Command);
}
//</code></function>

/*<function id="Open_Command">
<heading>
  <description><en>
    Open COM port file handler</en></description>
  <parameters>
    <parameter type="return">
      <datatype>Select_Reader *</datatype>
      <description><en>pointer to the structure describing the opened card reader</en></description></parameter>
    <parameter type="arg">
      <datatype>TYPE_READER</datatype><name>Type_Reader</name>
      <description><en>the reader type</en></description></parameter>
    <parameter type="arg">
      <datatype>PORT</datatype><name>Port</name>
      <description><en>the port number (0=COM1, 1=COM2)</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT145_1R_Reader</name>
      <description><en>the readers definitions</en></description></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT145_1A_Reader</name></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT345_Reader</name></parameter>
    <parameter type="global">
      <datatype>Select_Reader</datatype><name>MCT228_Reader</name></parameter>
  </parameters>
  <usage>
    if(( Intern_Reader = Open_Command ( Intern_Type, COM2)) == NULL)</usage>
  <comment>
    The port numbers are defined as follow in Borland: Com1=0,
    Com2=1 ... The ports are defined with an enum typedef, and enum
    starts at 0 (perfect).
    The COM ports are adressed just like other output device, we
    adress them as files with read and write capabilities by using
    their handle.</comment>
</heading>
<code>*/
Select_Reader * Open_Command( TYPE_READER Type_Reader, PORT Port)
{
   char * ComPort = "COM ";         // name of the COM port, used as file name
   char * Response;
   Select_Reader * NewReader = NULL;

   _bios_serialcom( _COM_INIT, Port, SETTINGS);     //Set SETTINGS to COM port

   // allocate the new reader structure
   if( (NewReader = (Select_Reader *) malloc( sizeof( Select_Reader))) == NULL)
      return( NULL);

   // assign the reader description to NewReader
   switch( Type_Reader)
   {
      case MCT145_1R: * NewReader = MCT145_1R_Reader;
                      break;
      case MCT145_1A: * NewReader = MCT145_1A_Reader;
                      break;
      case MCT345:    * NewReader = MCT345_Reader;
                      break;
      case MCT228:    * NewReader = MCT228_Reader;
                      break;
// toegevoegd door EBE (18/08/1994) voor DEMO-versie
      case MCT145_1A_DEMO: * NewReader = MCT145_1A_Demo_Reader;
            break;
// einde toevoeging


      default:        return( NULL);
   }

   NewReader -> ComPort = Port;

   // disable the Capture capability on the internal reader
   if( Port == COM2)
      NewReader -> Commands [CAPTURE].Procedure = Impossible_Command;

   ComPort [3] = Port + '1';

   // initialize the reader
   Response = Send_Command( NewReader, INITIALIZE, NULL);
   // Process comunications error
   if( Response [0] == 'N' && Response [3] == '0' && Response [4] == '3')
   {
       free( NewReader);
       NewReader = NULL;
       return( NULL);
   }

   return( NewReader);
}
//</code></function>

/*<function id="Send_Command">
<heading>
  <description><en>
    sends a command to the reader, by executing the command stored
    in the reader description</en></description>
  <parameters>
    <parameter type="return">
      <datatype>char*</datatype>
      <description><en>the answer string from the reader for the command</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>COMMAND</datatype><name>Command</name>
      <description><en>the command number to perform</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Data</name>
      <description><en>the needed data for that command</en></description></parameter>
  </parameters>
  <usage>
    if( * (Response = Send_Command( Reader, DISABLE, NULL)) == 'N')</usage>
  <checks>
    When no reader is defined, or if the command is not supported
    by the reader, we send "N??00" as response, indicating that
    there was an error.</checks>
  <comment>
    Not all the commands need data, the most of them need parameters,
    these parameters are in the Reader description structure.</comment>
</heading>
<code>*/
char * Send_Command( Select_Reader * Reader, COMMAND Command, char * Data)
{
   char * Reader_Reply;

   // reader does not exist or command execution is impossible
   if( ! Reader || ! Reader -> Commands [Command].Procedure)
     return( "N??00");

   if( Command == INITIALIZE)
      Reader -> State = USABLE;

   Reader_Reply =
      // call function referenced in the reader's profile
      (* Reader -> Commands[Command].Procedure)
          ( Reader, Reader -> Commands[Command].Parameter ?
                  Reader -> Commands[Command].Parameter : Data);

   #if _DEBUGMODE
      sprintf( gcDcio, "Send_Command: in[%d] out[%s]", Command, Reader_Reply); dcio();
   #endif // _DEBUGMODE

   return Reader_Reply;
};
//</code></function>

/*<function id="Close_Command">
<heading>
  <description><en>
    Close COM port file handler</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>Select_Reader **</datatype><name>Reader</name>
      <description><en>structure describing the reader</en></description></parameter>
  </parameters>
  <usage>
    Close_Command ( &amp; Intern_Reader);</usage>
  <comment>
    This function closes the reader access and frees the memory used
    for the reader's description.</comment>
</heading>
<code>*/
void Close_Command( Select_Reader ** Reader)
{
   free( * Reader);
   * Reader = NULL;
}
//</code></function>

/*<function id="Write_To_Reader">
<heading>
  <description><en>
    writes a string on the COM port</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Port</name>
      <description><en>the COM port number</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>String</name>
      <description><en>the string to write</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Len</name>
      <description><en>the length of the string to write</en></description></parameter>
  </parameters>
  <usage>
    Write_To_Reader( Reader -&gt; ComPort, Buffer, strlen( Buffer));</usage>
  <checks>
    The result of the _bios_serialcom is checked, otherwise we will
    have to wait until all the timeouts are performed....and this
    can take a lot of precious time</checks>
</heading>
<code>*/
static void Write_To_Reader( int Port, char * String, int Len)
{
   for( register int i=0; i < Len; i++)
        if( _bios_serialcom( _COM_SEND, Port, String [i]) & TIME_OUT_ERROR)
            return;
}
//</code></function>

/*<function id="Standard_Command">
<heading>
  <description><en>
    perform a standard (not simulated) command on the reader</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Command</name>
      <description><en>the command string to perform</en></description></parameter>
    <parameter type="global">
      <datatype>const char</datatype><name>etx</name>
      <description><en>end of transmission character</en></description></parameter>
    <parameter type="global">
      <datatype>const char</datatype><name>stx</name>
      <description><en>start of transmission character</en></description></parameter>
  </parameters>
  <usage>
    return( Standard_Command( Reader, Buffer));</usage>
  <comment>
    refer to reader documentation for timing and logic explanation.</comment>
</heading>
<code>*/
static char * Standard_Command( Select_Reader * Reader, char * Command)
{
   static union RESULT {  struct VALUE { char value; char error_code;} Value;
                    unsigned int read_result; } Result [120];

   union RESULT * ResultPtr;
   union RESULT * EndResultPtr = &(Result[119]);

   register int Port = Reader -> ComPort;
   int i, RetryCommand, RetryEnquire;
   char tmp, bcc, *CommandPtr, Buffer[120],*BufferPtr;
   unsigned int NrCharactersRead, NrCharactersWritten;
   int ResponseLength;
   static char Response[120];
   unsigned long SysClock;
#ifndef _APPLICATION_MAV
   int ComReceived;
#endif

   if( Reader -> State == NOT_USABLE)
     return( "N??03");  // Communication Error

   // compute the Binary Control Block (bcc), that is an exclusive or on all
   // the bytes of the BCC range
   for( bcc = etx, CommandPtr = Command; * CommandPtr; )
      bcc ^= * CommandPtr++;

   // build the buffer to send to the reader
   BufferPtr = Buffer;
   * BufferPtr++ = stx;
   memcpy( BufferPtr, Command, i = strlen( Command));
   BufferPtr += i;
   * BufferPtr++ = etx;
   * BufferPtr++ = bcc;
   * BufferPtr++ = 0;

   // No interrupts allowed because they can take some time to complete which
   // may cause the lost of characters
   Disable_All();

   // process the command and the error retrys
   for( RetryCommand = 0;
      RetryCommand < gRETRY_COMMAND;
      RetryCommand++, delay(100))
   {
#ifndef _APPLICATION_MAV
#ifndef DIAGNOST
     if( ++WatchSign == 5)                     // JRU : WatchDog
       WatchSign = 1;                         // JRU : WatchDog
     gotoxy(1,1);                              // JRU : WatchDog
     printf("%c", WatchSigns[WatchSign]);      // JRU : WatchDog
#endif
#endif

#ifdef TRACE
     sound(100);
     nosound();
     LastTrace = ++LastTrace % NR_TRACES;
     TraceBuffer [LastTrace] = EmptyCommandTrace;
     TraceBuffer [LastTrace].ComPort = Port;
     TraceBuffer [LastTrace].Command [0] = Buffer [2];
     TraceBuffer [LastTrace].Command [1] = Buffer [3];
     TraceBuffer [LastTrace].TimeCommand = * clock_ticks;
#endif

     Write_To_Reader( Port, Buffer, strlen( Buffer));

//rvh      Result [0].read_result = _bios_serialcom( _COM_RECEIVE, Port, 0);

#ifndef _APPLICATION_MAV
     ComReceived = 0;
#endif
     for( SysClock=*clock_ticks;
        (SysClock+1+2)>=*clock_ticks; )
     {
       Result[0].read_result = _bios_serialcom( _COM_RECEIVE, Port, 0);
       if( Result[0].Value.value == ack ||
          Result[0].Value.value == nak)
       {
#ifndef _APPLICATION_MAV
         ComReceived = 1;
#endif
         break;
       }
     }
#ifndef _APPLICATION_MAV
     // Only for KDE Card Reader, because there can be a peak of 3A when
     // playing with the Card/Shutter
     if( RetryCommand > 0 && Version == 2)
       if( ComReceived           == 1   &&
          Result[0].Value.value == nak   )
       {
         RetryCommand = 99; // Power Shortage (DV II)
         continue;
       }
#endif
#ifdef TRACE
     TraceBuffer [LastTrace].Acknowledge = Result [0].Value.value;
     if( (Result [0].Value.value == ack) ||
        (Result [0].Value.value == nak))
       TraceBuffer [LastTrace].TimeAck = * clock_ticks;
#endif

     if( Result [0].Value.value != ack)
     {
       // Clear the line and reset MCRW
       Write_To_Reader( Port, "\x10\x4", 2);

       i = 4;                     // Maximum 3 characters are left in buffer

       do {

         Result [0].read_result = _bios_serialcom( _COM_RECEIVE, Port, 0) & 0xFF;

       } while( (Result [0].Value.value != 0) && (--i >= 0));

       RetryCounter++;
       continue;                                  // Timeout or nak received
     };

     for( RetryEnquire = 0; RetryEnquire < gRETRY_ENQ; RetryEnquire++)
     {
#ifdef TRACE
       if( RetryEnquire != 0)                            // Retry on enquire
       {
         LastTrace = ++LastTrace % NR_TRACES;
         TraceBuffer [LastTrace] = EmptyCommandTrace;
         TraceBuffer [LastTrace].ComPort = Port;
       }
       TraceBuffer [LastTrace].TimeEnquires [RetryEnquire] = * clock_ticks;
#endif

       ResultPtr = Result;
       ResultPtr -> read_result = 0;

       // 364 = 20 seconds in ticks
       // 1572480 = 24 hours in ticks
       register unsigned long before = * clock_ticks;
       unsigned long after1 = before + 364L;
       unsigned long after2 = (after1 >= 1572480L) ? (after1 - 1572480L) : after1;

       // Send enquire
       Write_To_Reader( Port, "\x5", 1);

       // Wait on first character of response
       // we can wait 364 ticks for a response = 20 seconds
       do {

         ResultPtr -> read_result = _bios_serialcom( _COM_RECEIVE, Port, 0);

       } while ( (ResultPtr -> Value.value == 0) &&
               ((* clock_ticks) < ( (* clock_ticks < before) ? after2 : after1) ) );

       // Read following characters of response
       do {
           (++ ResultPtr) -> read_result = _bios_serialcom( _COM_RECEIVE, Port, 0);

       } while( (ResultPtr -> Value.value != etx) &&
              (ResultPtr -> Value.value != 0) &&
              (ResultPtr < EndResultPtr) );

       // Read bcc
       (++ ResultPtr) -> read_result = _bios_serialcom( _COM_RECEIVE, Port, 0);

       ResponseLength = ResultPtr - Result;              // bcc not included

       // Search stx position in Result
       for( int i=0;
           (Result [i].Value.value != stx) && (i < ResponseLength-1);
           i++);

       // Copy of Result to Response  (stx, ..., without etx)
       for( int j=0; i < ResponseLength-1; i++)
       {

#ifdef TRACE
         if( Result [i].Value.error_code & 0x80)   // time out flag set
            TraceBuffer [LastTrace].FunctionResponse = Result [i].read_result;
#endif
         Response [j++] = Result [i].Value.value;
       }
       bcc = Result [i+1].Value.value;
       Response [j] = 0;

#ifdef TRACE
       memcpy( TraceBuffer [LastTrace].Response, Response, 7);
       TraceBuffer[LastTrace].TimeResponse = * clock_ticks;
#endif

       if( (ResultPtr-1) -> Value.value != etx)
       {
         RetryCounter++;
         break;                                 // Command has to be resend
       }

       for( ResultPtr--; j>0; j--)                // Check BCC
           bcc ^= (ResultPtr--) -> Value.value;

#ifdef TRACE
       TraceBuffer[LastTrace].Bcc = bcc;
#endif

       if( bcc)
       {
         RetryCounter++;
         continue;                        // Require has to be resend
       }

       if( (Response [4] == '1' && Response [5]== '1') || // Shutter failure
          (Response [4] == '1' && Response [5]== '2') || // Sensor failure
          (Response [4] == '1' && Response [5]== '9') || // Other mechanical irregularites
          (Response [4] == '3' && Response [5]== '0') || // Power down
          (Response [4] == '3' && Response [5]== '1'))   // DSR signal was turned to OFF
       {
         RetryCounter++;
         break;
       }

       // enable interrupts
       Enable_All();

       return( Response + 1);
     }

     // Clear the line and reset MCRW
     Write_To_Reader( Port, "\x10\x4", 2);
   }

#ifdef TRACE
   LastTrace = ++LastTrace % NR_TRACES;
   TraceBuffer [LastTrace] = EmptyCommandTrace;
   TraceBuffer [LastTrace].ComPort = Port;
   strcpy( TraceBuffer [LastTrace].Response, "N??03");
#endif

   // enable interrupts
   Enable_All();

   Reader -> State = NOT_USABLE;
   if( RetryCommand == 100)
     return( "N??99");  // Power Shortage (DV II)
   else
     return( "N??03");  // Communication Error
}
//</code></function>

/*<function id="Write_Command_C73">
<heading>
  <description><en>
    format the command buffer with the data to write before calling
    the Standard_Command function</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Data</name>
      <description><en>the data to write on card</en></description></parameter>
  </parameters>
  <comment>
    the only difference with Standard_Command is that the buffer
    is not only constitued by the command string but also with the
    data to write. This data is appended to the command before we
    call Standard_Command.</comment>
</heading>
<code>*/
static char * Write_Command_C73( Select_Reader * Reader, char * Data)
{
   int  i;
   char Buffer [120], * BufferPtr = Buffer;
   char WriteString [] = "C73";

   memcpy( BufferPtr, WriteString, i = strlen( WriteString));
   BufferPtr += i;
   memcpy( BufferPtr, Data, i = strlen( Data));
   BufferPtr += i;
   * BufferPtr = 0;

   return( Standard_Command( Reader, Buffer));
}
//</code></function>

/********************* new function : ebe 10/03/1994 *****************/

/*<function id="Write_Command_Dummy">
<heading>
  <description><en>
    This function is the same as Write_Command_C73 except that
    it doesn't write physically on the card</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Data</name>
      <description><en>the data to write on card</en></description></parameter>
  </parameters>
<comment>
the only difference with Standard_Command is that the buffer
is not only constitued by the command string but also with the
data to write. This data is appended to the command before we
call Standard_Command.
created by EBE (10/03/1994)</comment>
</heading>
<code>*/
static char * Write_Command_Dummy( Select_Reader * Reader, char * Data)
{
  return( Send_Command( Reader, STATUSREQUEST, Data)) ;
}
//</code></function>

/*<function id="Enable_Command_MCT228">
<heading>
  <description><en>
    send the command to enable the MCT228 reader</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Dummy</name>
      <description><en>not used</en></description></parameter>
  </parameters>
</heading>
<code>*/
#pragma argsused
static char * Enable_Command_MCT228( Select_Reader * Reader, char * Dummy)
{
   char * Response;

   Response = Standard_Command( Reader, "C:0");                 // ENABLE_READ

   if( Response [3] == '0' && Response [4] == '2')
       return( Standard_Command( Reader, "C10"));            // STATUS REQUEST
   else
       return( Response);
}
//</code></function>

/*<function id="Enable_Formatted_Command_MCT228">
<heading>
  <description><en>
    enable the MCT228 reader to accept a formatted card</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Dummy</name>
      <description><en>not used</en></description></parameter>
  </parameters>
</heading>
<code>*/
#pragma argsused
static char * Enable_Formatted_Command_MCT228( Select_Reader * Reader,
                                               char * Data)
{
   char * Response;

   Response = Standard_Command( Reader, "C:2");            // ENABLE FORMATTED

   if( Response [0] == 'P' && Response [3] == '0' && Response [4] == '1')
       Response = Standard_Command( Reader, "C60");                    // MOVE

   if( Response [0] == 'N' && Response [3] == '0' && Response [4] == '2')
       return( Standard_Command( Reader,"C10"));             // STATUS REQUEST
   else
       return( Response);
}
//</code></function>
/*<function id="Enable_Write_Command_C78_C24">
<heading>
  <description><en>
    write command for the MCT145_1A reader</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Data</name>
      <description><en>the data to write on card</en></description></parameter>
  </parameters>
<comment>
the difference with Standard_Command is that the buffer
is not only constitued by the command string but also with the
data to write. This data is appended to the command before we
call Standard_Command.
The reader is disabled before writing and enable after.</comment>
</heading>
<code>*/
static char * Enable_Write_Command_C78_C24( Select_Reader * Reader, char * Data)
{
   int  i;
   char Buffer [120], * BufferPtr = Buffer;
   char DataTransferString [] = "C78";
   char EnableWriteString [] = "C:4";
   char * Response;

   if( * (Response = Send_Command( Reader, DISABLE, NULL)) == 'N')
       return( Response);

   memcpy( BufferPtr, DataTransferString, i = strlen( DataTransferString));
   BufferPtr += i;
   memcpy( BufferPtr, Data, i = strlen( Data));
   BufferPtr += i;
   * BufferPtr = 0;

   if( * (Response = Standard_Command( Reader, Buffer)) == 'N')
       return( Response);

   return( Standard_Command( Reader, EnableWriteString));
}
//</code></function>

/*<function id="Status_Write_Command_C78_C12">
<heading>
  <description><en>
    status request on write command for the MCT145_1A reader</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Dummy</name>
      <description><en>not used</en></description></parameter>
  </parameters>
</heading>
<code>*/
#pragma argsused
static char * Status_Write_Command_C78_C12( Select_Reader * Reader, char * Dummy)
{
   char StatusWriteString [] = "C12";

   return( Standard_Command( Reader, StatusWriteString));
}
//</code></function>

/*<function id="Status_Read_Command">
<heading>
  <description><en>
    status request on read command</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>local function returning the reader response string</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Reader</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Dummy</name>
      <description><en>not used</en></description></parameter>
  </parameters>
</heading>
<code>*/
#pragma argsused
static char * Status_Read_Command( Select_Reader * Reader, char * Dummy)
{
   char * Response;
   char StatusReadString [] = "C13";

   Response = Standard_Command( Reader, StatusReadString);

   if( memcmp( Response, "N1301", 5) == 0)
   {
      Response [0] = 'P';
      Response [4] = '2';
   }

   return( Response);
}
//</code></function>

/*<function id="Impossible_Command">
<heading>
  <description><en>
    function that is executed when a reader does not support the
    requested command</en></description>
  <parameters>
    <parameter type="return">
      <datatype scope="static">char *</datatype>
      <description><en>the error string indicating that the reader does not support the command</en></description></parameter>
    <parameter type="arg">
      <datatype>Select_Reader *</datatype><name>Dummy1</name>
      <description><en>the reader description structure</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>Dummy2</name>
      <description><en>the data to write on card</en></description></parameter>
  </parameters>
<usage>
if( * (Response = Impossible_Command( Reader, NULL)) == 'N')</usage>
<comment>
this function is to prevent we send bad commands to the reader
by for example selecting a reader that does not have write
capabilities for the intern reader in the MAV.</comment>
</heading>
<code>
*/
#pragma argsused
static char * Impossible_Command( Select_Reader * Dummy1, char * Dummy2)
{
   return( "N??00");
}
//</code></function>

/*<function id="SetRetryModeOn">
<heading>
  <description>
    <en>Enable Retry Mode on card reader</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
  <usage />
  <checks />
  <comment>Retry Mode is the normal mode for gen 2 readers</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SetRetryModeOn( void)
// ---------------------------------------------------------------------------
{

   #ifdef _TEST
      gRETRY_COMMAND = 1;
      gRETRY_ENQ     = 1;
   #else
      gRETRY_COMMAND = 5;
      gRETRY_ENQ     = 2;
   #endif

   return;
}
//</code></function>

/*<function id="SetRetryModeOff">
<heading>
  <description>
    <en>Disable Retry Mode on card reader</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
  <usage />
  <checks />
  <comment>Avoids retries during critical operations like for instance card capture</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SetRetryModeOff( void)
// ---------------------------------------------------------------------------
{
   gRETRY_COMMAND = 1;
   gRETRY_ENQ     = 1;

   return;
}
//</code></function>


//=== CARD.CPP=================================================================
//</body>
//</source>