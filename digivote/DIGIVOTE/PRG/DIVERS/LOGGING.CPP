/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="logging.cpp">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2006/10/02 12:55:40 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>logging.cpp</fileName>
  <creationDate>01/01/2003</creationDate>
  <rights>Steria Benelux</rights>
  <author></author>
  <description>
    <fr></fr>
    <nl></nl></description>
  <history></history>
  <comment>was debug.cpp, renamed to logging.cpp</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <fstream.h>

#include "..\divers\logging.h"

#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
char gcLogBuf[256];                       // message buffer for logging
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

/*<function id="Add_To_Logging">
<heading>
  <description>
   <fr>Enregistrer message dans logging</fr>
   <nl>Boodschap registreren in logfile</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Add_To_Logging (
   RESULTS  ErrorCode,
   char*    LogInfo
   )
// ---------------------------------------------------------------------------
{
   #ifdef _PREVIEW
    #define _APPLICATION_NAME   "PREVIEW "
   #else
    #ifdef _PARTLOGO
     #define _APPLICATION_NAME  "PARTLOGO "
    #else
     #ifdef _DIGIMENU
      #define _APPLICATION_NAME "DIGIMENU "
     #else
      #define _APPLICATION_NAME ""
     #endif
    #endif
   #endif
   struct dosdate_t curr_date;
   struct dostime_t curr_time;
   char LogLine[254],
       MsgCode[6];
   // Variables to make LogFile hidden
   struct find_t ffblk;
   unsigned attrib;

   _dos_getdate(&curr_date);
   _dos_gettime(&curr_time);

   ofstream LOG( LogFile, ios::app);
   if( LOG)
   {
      switch( ErrorCode)
      {
         case EMPTY:
            strcpy( MsgCode, "     ");
            break;

         case OK :
            strcpy( MsgCode, "<OK >");
            break;

         case NOK:
            strcpy( MsgCode, "<NOK>");
            break;

         case ERR:
            strcpy( MsgCode, "<ERR>");
            break;

         default :
            strcpy( MsgCode, "<UNK>");
      }
      sprintf( LogLine, "%.2d/%.2d/%.4d-%.2d:%.2d:%.2d%s%s%s",
                    curr_date.day,  curr_date.month,  curr_date.year,
                    curr_time.hour, curr_time.minute, curr_time.second,
                    MsgCode, _APPLICATION_NAME, LogInfo);
      LOG << LogLine << "\n";
   }
   LOG.close();

#if _DEBUGMODE
   // dump message on screen also
   sprintf( gcDcio, "%s", LogInfo); dcio();
   // gotoxy( 1, 14); clreol(); printf( "%s", LogInfo);
#else
   // make logfile hidden in production
   if( !_dos_findfirst( LogFile, _A_NORMAL|_A_RDONLY|_A_HIDDEN, &ffblk))
   {
      attrib = ffblk.attrib;
      if( attrib | _A_HIDDEN)
      {
         attrib |= _A_HIDDEN;
         _dos_setfileattr( LogFile, attrib);
      }
   }
#endif

}
//</code></function>


/*<function id="logOnStartup">
<heading>
  <description>
   <fr>Enregistrer message dans logging</fr>
   <nl>Boodschap registreren in logfile</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void logOnStartup ( void)
// ---------------------------------------------------------------------------
{
   char strCoreLeft[80];

   sprintf( strCoreLeft,"Startup coreleft = %luKb (Far Heap = %luKb)",
                  ( unsigned long)(    coreleft() / 1024),
                  ( unsigned long)( farcoreleft() / 1024));
   Add_To_Logging( EMPTY, strCoreLeft);
   return;
}
//</code></function>


/*<function id="logOnExit">
<heading>
  <description>
   <fr>Enregistrer message dans logging</fr>
   <nl>Boodschap registreren in logfile</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void logOnExit ( void)
// ---------------------------------------------------------------------------
{
   char strCoreLeft[80];

   sprintf( strCoreLeft,"Exit    coreleft = %luKb (Far Heap = %luKb)",
                  ( unsigned long)(    coreleft() / 1024),
                  ( unsigned long)( farcoreleft() / 1024));
   Add_To_Logging( EMPTY, strCoreLeft);
   return;
}
//</code></function>

//=============================================================== EOF BODY ===
/*</body>
</source>
<!-- eof -->*/
