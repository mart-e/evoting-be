/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="debug.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   debug.cpp  $</workFile>
  <revision>$Revision: 1.4 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 16:23:34  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/Tools/debug.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2008/10/21 12:38:51 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>debug.cpp</fileName>
  <creationDate>20010901</creationDate>

  <rights/>
  <author>AVN</author>

  <description>
    <fr></fr>
    <nl></nl></description>
  <history></history>
  <comment>Debugging library for internal use</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <conio.h>  // gotoxy, putch, clreol
#include <string.h>

#include "..\tools\debug.h"
#ifdef _APPLICATION_MAV
   #include "..\gen\gentype.h"
   #include "..\mav\mavlight.h"
#endif // _APPLICATION_MAV
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
                                                // these functions are private
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
char cDbugBuf[256];                    // buffer to hold logging/dbg info
char gcDcio[256];                       // message buffer for debugging

//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===
//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Dbug">
<heading>
  <description><en>Log debugging messages in a text file</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>void</datatype></parameter>
  </parameters>
  <comment>Logfile is opened on the first call, to close the file put 'close'
    in the global buffer.
  </comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int Dbug ( void)
// ---------------------------------------------------------------------------
{
      static FILE*   stream;
      static int     firstCall = 1;

#if 0
   if ( firstCall)
   {
      // remind us logging is active
      printf( "\a");
      printf( "\a");
      printf( "\a");
      firstCall = 0;
   }

   // mode: append + text
   if ( ( stream = fopen( "DEBUG.LOG", "at")) != NULL )
   {
      fprintf( stream, cDbugBuf);
      fflush( stream);
      fclose( stream);
   }

#else

   if ( firstCall)
   {
      if ( ( stream = fopen( "DEBUG.LOG", "at")) != NULL )
      {
         // remind us logging is active
         sprintf( gcDcio, "Logging...[%s]", cDbugBuf); dcio();
      }
      firstCall = 0;
   }

   if ( !strncmp( "close", cDbugBuf, 5))
   {
      fprintf( stream, "--- eof ---\n");
      fclose( stream);
      firstCall = 1;
   }
   else
   {
      fprintf( stream, cDbugBuf);
      fflush( stream);
   }
#endif

   return 0;
}
//</code></function>


/*<function id="DbugHex">
<heading>
  <description><en>Convert bytes to hex text and log.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>Always 0.</en></description></parameter>
    <parameter type="arg">
      <datatype>unsigned char*</datatype><name>bytes</name>
      <description><en>address of bytes to convert</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>len</name>
      <description><en>number of bytes</en></description>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int DbugHex (
   unsigned char* bytes,               // address of bytes to convert
   int len                             // number of bytes
   )
// ---------------------------------------------------------------------------
{
      int j;

   strcat( cDbugBuf, "[");
   for ( j = 0; j < len; j++)
   {
      char temp[3+1];
      sprintf( temp, "%02X ", bytes[j]);
      strcat( cDbugBuf, temp);
   }
   strcat( cDbugBuf, "]\n");
   Dbug();

   return 0;
}
//</code></function>


/*<function id="dcio">
<heading>
  <description><en>Handle debug messages</en></description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
    </parameter>
    <parameter type="arg">
      <datatype>void</datatype>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void dcio ( void)
// ---------------------------------------------------------------------------
{
        int iCol, iRow;

#ifdef _APPLICATION_MAV
   #ifdef _PREVIEW
      // keyboard must be present for this part!
      gotoxy( 1, 24);
      printf( "%s. Press any key to continue!", gcDcio);
      getch();
      gotoxy( 1, 24); clreol();
   #else
      // MAV as no keyboard, use lightpen instead
      gotoxy( 1, 24); clreol();
      printf( " %s ", gcDcio);

      gotoxy( 1, 25); clreol();
      printf( " Press the lightpen to continue... ");
      Get_Pen( &iCol, &iRow );
   #endif
#endif

#ifdef _APPLICATION_URN

   // save previous position
   iCol = wherex();
   iRow = wherey();

   textcolor( BLACK);
   textbackground( WHITE);

   // keyboard must be present for this part!
   gotoxy( 1, 24);
   printf( "%s. Press any key to continue!", gcDcio);
   getch();
   gotoxy( 1, 24); clreol();

   // restore previous position
   gotoxy( iCol, iRow);

#endif

    return;
}
//</code></function>

//------------------------------------------------------ PRIVATE FUNCTIONS ---

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->


