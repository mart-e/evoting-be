//
// /!\ deprecated source, see divers\logging.cpp instead /!\
//

#error ..\divers\debug.cpp deprecated, use ..\divers\logging.cpp instead!

#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <alloc.h>
#include <string.h>
#include <fstream.h>

#include "debug.h"

/*<function id="Add_To_Logging">
<heading>
  <description>
   <fr>Enregistrer message dans logging</fr>
   <nl>Boodschap registreren in logfile</nl></description>
  <parameters></parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Add_To_Logging(
   RESULTS ErrorCode,
   char *LogInfo)
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
   // Make LogFile hidden
   if( !_dos_findfirst( LogFile, _A_NORMAL|_A_RDONLY|_A_HIDDEN, &ffblk))
   {
      attrib = ffblk.attrib;
      if( attrib | _A_HIDDEN)
      {
         attrib |= _A_HIDDEN;
         _dos_setfileattr( LogFile, attrib);
      }
   }
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
void logOnStartup(
   void)
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
void logOnExit(
   void)
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
//
// /!\ deprecated source, see divers\logging.cpp instead /!\
//
