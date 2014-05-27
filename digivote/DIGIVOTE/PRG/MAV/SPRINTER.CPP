/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="sprinter.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   sprinter.cpp  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Mar 03 2003 14:55:38  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/sprinter.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:28 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>sprinter.cpp</fileName>
  <creationDate>08/2002</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>Librairie de fonctions RS-232 (bios)</fr>
    <nl>RS-232 Functielibrary (bios)</nl></description>
  <history/>
  <comment>This code provides serial comm's thru bios functions.</comment>
</comments>*/
//<header>
//============================================================= BOF HEADER ===

//<includes>
//--------------------------------------------------------------- INCLUDES ---
#include <dos.h>    // delay()
#include <bios.h>   // _bios_serialcom()
#include <stdio.h>  // printf()
#include <conio.h>  // gotoxy(), putch(), clreol()

#include "..\mav\sprinter.h"
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
// :WARNING: avn 20020201 -> turn OFF for production code!
#define _DEBUGMODE      0       // debug mode OFF = 0, ON = 1
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

// Com port parameters
#define COM1            0
#define SETTINGS        (_COM_9600 | _COM_CHR8 | _COM_STOP1 | _COM_NOPARITY)

// Line Status Register bits
#define TIME_OUT_ERROR  0x8000  // Time-out error
#define TSR_EMPTY       0x4000  // Transfer shift register empty
#define THR_EMPTY       0x2000  // Transfer holding register empty
#define BREAK_ERROR     0x1000  // Break-detect error
#define FRAMING_ERROR   0x0800  // Framing error
#define PARITY_ERROR    0x0400  // Parity error
#define OVERRUN_ERROR   0x0200  // Overrun Error
#define DATA_READY      0x0100  // Data ready

// Modem Status Register bits
#define DCD             0x80    // 10000000 Received line signal detect
#define RI              0x40    // 01000000 Ring indicator
#define DSR             0x20    // 00100000 Data Set Ready
#define CTS             0x10    // 00010000 Clear To Send
#define DDCD            0x08    // 00001000 Change in receive line signal
#define DRI             0x04    // 00000100 Trailing edge ring indicator
#define DDSR            0x02    // 00000010 Change in Data Set Ready
#define DCTS            0x01    // 00000001 Change in Clear To Send

// Error codes returned by this library
#define NO_ERRORS             0        // so far so good :-)

#define ERROR_LIST_OFFSET     -1000    // should be different for every lib
#define NO_DSR                -1001    // no DSR signal detected
#define NO_CTS                -1002    // no CTS signal detected
#define SERIALCOM_ERROR       -1003    // bios_serialcom() returned an error

#define BAD_PRINTER_ID        -1009    // received wrong id from the device

#define COVER_OPEN_ERROR      -1022    // printer errors
#define FF_PRESSED_ERROR      -1023
#define KNIFE_ERROR           -1033
#define UNRECOVERABLE_ERROR   -1035
#define HARDWARE_ERROR        -1036
#define PAPER_LOW_ERROR       -1042
#define PAPER_OUT_ERROR       -1045

#define BAD_PRINTER_STATUS    -1092    // byte received is not a printer status
#define BAD_ERROR_STATUS      -1093    // byte received is not an error status
#define BAD_PAPER_STATUS      -1094    // byte received is not an error status

#define LOGIC_ERROR           -1999


//<prototypes>                            <!-- these functions are private -->
//------------------------------------------------------------- PROTOTYPES ---
static int queryPrinterStatus( void);
static int queryErrorStatus( void);
static int queryPaperStatus( void);

static int ComOpen( void);
//</prototypes>

//                                     <!-- definitions DO allocate memory -->
//---------------------------------------------------------------- GLOBALS ---
int giReadErrors;               // track comm errors
int giSendErrors;

union
{
    unsigned char ReturnByte;
    struct {
        unsigned int bit0 : 1;
        unsigned int bit1 : 1;
        unsigned int bit2 : 1;
        unsigned int bit3 : 1;
        unsigned int bit4 : 1;
        unsigned int bit5 : 1;
        unsigned int bit6 : 1;
        unsigned int bit7 : 1;
    } Value;
} guStatus;

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

//<function id="queryPrinterStatus">
/*<heading>
    <description><en>Query's the printer's status. If an error condition exists
    we question the printer further to find out what's wrong.</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure:
      -20 thru -50 hardware errors
      -90 thru -99 logic/comm errors.
      </en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int queryPrinterStatus ( void)
// ---------------------------------------------------------------------------
{
   // send 'Real Time Status' command
   ComSend( 0x1D); ComSend( 0x04); ComSend( 0x02);

   // must return one byte
   guStatus.ReturnByte = ComRead();

   // test fixed bits (cfr. A794 owner's guide p.113)
   if ( !( !guStatus.Value.bit0 && guStatus.Value.bit1 &&
            guStatus.Value.bit4 && !guStatus.Value.bit7))
   {
      // byte read is not an RS232 busy status
      return BAD_PRINTER_STATUS;
   }
   // test status bits
   if ( guStatus.Value.bit2)
   {
      // cover open
      return COVER_OPEN_ERROR;
   }
   if ( guStatus.Value.bit3)
   {
      // paper feed button is pressed
      return FF_PRESSED_ERROR;
   }
   if ( guStatus.Value.bit5)
   {
      // printing stopped due to paper condition
      return queryPaperStatus();
   }
   if ( guStatus.Value.bit6)
   {
      // error condition exist in the printer
      return queryErrorStatus();
   }

   return NO_ERRORS;   // printer is OK
}
//</code></function>


//<function id="queryErrorStatus">
/*<heading>
    <description><en>Query's the error status.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int queryErrorStatus ( void)
// ---------------------------------------------------------------------------
{

   // send 'Error Status' command
   ComSend( 0x1D); ComSend( 0x04); ComSend( 0x03);

   // must return one byte
   guStatus.ReturnByte = ComRead();

   // test fixed bits (cfr. A794 owner's guide p.113)
   if ( !( !guStatus.Value.bit0 && guStatus.Value.bit1 &&
           !guStatus.Value.bit2 && guStatus.Value.bit4 && !guStatus.Value.bit7))
   {
      // byte read is not an error status
      return BAD_ERROR_STATUS;
   }
   // test status bits
   if ( guStatus.Value.bit3)
   {
      // knife error occurred
      return KNIFE_ERROR;
   }
   if ( guStatus.Value.bit5)
   {
      // unrecoverable error occurred
      return UNRECOVERABLE_ERROR;
   }
   if ( guStatus.Value.bit6)
   {
      // printhead temp or power supply voltage are out of range
      return HARDWARE_ERROR;
   }

   // should never be reached since the printer status indicated an error
   return LOGIC_ERROR;
}
//</code></function>


//<function id="queryPaperStatus">
/*<heading>
    <description><en>Query's the paper status.</en></description>
   <parameters>
    <parameter type="return"><datatype>int</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int queryPaperStatus( void)
// ---------------------------------------------------------------------------
{

   // send 'Paper Status' command
   ComSend( 0x1D); ComSend( 0x04); ComSend( 0x04);

   // must return one byte
   guStatus.ReturnByte = ComRead();

   // test fixed bits (cfr. A794 owner's guide p.113)
   if ( !( !guStatus.Value.bit0 && guStatus.Value.bit1 &&
            guStatus.Value.bit4 && !guStatus.Value.bit7))
   {
      // byte read is not a paper status
      return BAD_PAPER_STATUS;
   }
   // test status bits
   if ( ( guStatus.Value.bit2) || ( guStatus.Value.bit3))
   {
      return PAPER_LOW_ERROR;
   }
   if ( ( guStatus.Value.bit5) || ( guStatus.Value.bit6))
   {
      return PAPER_OUT_ERROR;
   }

   // should never be reached since the printer status indicated an error
   return LOGIC_ERROR;
}
//</code></function>


//<function id="ComOpen">
/*<heading>
   <description><en>Setup serial communication port</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
static int ComOpen ( void)
//----------------------------------------------------------------------------
{
      int iError;
      unsigned int iStatus;

    // init stats
    giReadErrors = 0;
    giSendErrors = 0;

    // initialize serial port
    iStatus = _bios_serialcom( _COM_INIT, COM1, SETTINGS);
        // return value contains line status information in the high order byte,
        // and modem status information in the low order byte

    if ( iStatus & DSR)
    {
        if ( iStatus & CTS)
        {
            // DSR and CTS are both on, something is connected
            iError = NO_ERRORS;
        }
        else
        {
            iError = NO_CTS;
        }
    }
    else
    {
        iError = NO_DSR;
    }

#if _DEBUGMODE
   if ( iError)
   {
      sprintf( gcDcio, "ComOpen returns %d.", iError); dcio();
   }
#endif // _DEBUGMODE

    return iError;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

//<function id="initializePrinter">
/*<heading>
   <description><en>Setup serial communication and check the printer's id.</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int initializePrinter ( void)
// ---------------------------------------------------------------------------
{
      int iError;

   // set comm port parameters and check DSR/CTS
   iError = ComOpen();

   if ( !iError)
   {
      // comm port was succesfully opened, now we can test the connected
      // printer by asking his printer id, the A794 should return 0x24.
      ComSend( 0x1D); ComSend( 0x49); ComSend( 0x01); // Transmit Printer ID

      if ( 0x24 == ComRead())
      {
         // printer id is ok, check printer status
         iError = queryPrinterStatus();
      }
      else
      {
         // wrong answer
         iError = BAD_PRINTER_ID;
      }
   }

#if _DEBUGMODE
   if ( iError)
   {
      sprintf( gcDcio, "initializePrinter returns %d.", iError); dcio();
   }
#endif // _DEBUGMODE

   return iError;
}
//</code></function>


//<function id="ComSend">
/*<heading>
    <description><en>Send one character to the comm port.</en></description>
   <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>0 indicates success, negative numbers indicate a failure.</en></description></parameter>
    <parameter type="arg"><datatype>char</datatype>
      <description><en>The character that has to be send out.</en></description></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
int ComSend ( char chr)
//----------------------------------------------------------------------------
{
      int iError;
      unsigned int iStatus;

    // in case of a printer problem the printer drops DSR when we try to
    // print something, sending more characters when DSR is low results in a
    // time out error...

    // initialize serial port
    iStatus = _bios_serialcom( _COM_STATUS, COM1, 0);
        // return value contains line status information in the high order byte,
        // and modem status information in the low order byte

    if ( iStatus & DSR)
    {
        if ( iStatus & CTS)
        {
            // DSR and CTS are both on, send one char to port
            if ( _bios_serialcom( _COM_SEND, COM1, chr) & TIME_OUT_ERROR)
            {
                giSendErrors++;
                iError = SERIALCOM_ERROR;
            }
            else
            {
                iError = NO_ERRORS;   // char has been send out
            }
        }
        else
        {
            iError = NO_CTS;
        }
    }
    else
    {
        iError = NO_DSR;
    }

#if _DEBUGMODE
   if ( iError)
   {
      sprintf( gcDcio, "ComSend returns %d.", iError); dcio();
   }
#endif // _DEBUGMODE

   return iError;
}
//</code></function>


//<function id="ComRead">
/*<heading>
    <description><en>Read one character from port.</en></description>
   <parameters>
    <parameter type="return">
      <datatype>unsigned char</datatype>
      <description><en>The byte read or NULL when the function times out.</en></description></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
   </parameters>
  </heading>*/
//<code>
// ---------------------------------------------------------------------------
unsigned char ComRead ( void)
//----------------------------------------------------------------------------
{
      const int ciTenthOfSecond = 100;
      const int ciThreeSeconds = 30;

      unsigned int iResult;
      unsigned char cOut = 0x00;

    int i = 0;
    do // until we have a character or a timeout ( about 3 seconds )
    {

        iResult = _bios_serialcom( _COM_STATUS, COM1, 0);
        if ( iResult & DATA_READY)
        {
            // receive character
            iResult = _bios_serialcom( _COM_RECEIVE, COM1, 0);
            if ( iResult & 0xFF00)  // status bits set to 1 means errors
            {
                giReadErrors++;
            }
            else
            {
                // the byte read is in de lower bits of the return value,
                // if there is no error ( no upper bits set to 1 )!
                cOut = iResult & 0x00FF;
            }
        }
        i++;

#if _DEBUGMODE
        putch( '.');
#endif  // _DEBUGMODE

        delay( ciTenthOfSecond);

    } while ( (0x00 == cOut ) && ( i < ciThreeSeconds));

#if _DEBUGMODE
    gotoxy( 2, 15); printf( "Received 0x%02X after %d retry's and %d errors", cOut, i, giReadErrors);
    gotoxy( 1, 24); clreol();
#endif  // _DEBUGMODE

    return cOut;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
