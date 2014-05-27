#include <malloc.h>
#include <stdio.h>                               /* sprintf(), fopen(), fwrite(), fclose() */
#include <dos.h>                                 /* date/time */

#include "logging.h"

#define FILENAME_LOG "\\VOTE\\MENU.LOG"
#define NAMEMAXLEN   25                    // maximum length of modulename

char gcModuleName[ NAMEMAXLEN + 1] = "";   // module name that raised error
char gcFunctionName[ NAMEMAXLEN + 1] = ""; // function name that raised error
long int giStatus = 0;                     // errorcode to write back to Progress
int giError = 0;                           // errorcode to write in log
char gcMessage[128];                       // buffer to display messages

typedef enum {
   NONE        = 0,
   NOK         = 1,
   ERR         = 2,
   DBG         = 3
   } LOG_LEVEL;

/*==========================================================================*/
void LogFunctionId(
   char* cpModuleName,        // library name
   char* cpFunctionName       // function name
   )
/*==========================================================================*/
{
   sprintf( gcModuleName, "%.25s", cpModuleName);
   sprintf( gcFunctionName, "%.25s", cpFunctionName);

   return;
}

/*==========================================================================*/
void LogError(
   void
   )
/*==========================================================================*/
{
   giError = ERR;
   write2log();

   return;
}

/*==========================================================================*/
void LogDebug(
   void
   )
/*==========================================================================*/
{
   long debugmode = 0;

   // store debug info only when this file exists
   if( prordi( "debugmode", 0, debugmode, 0))
      promsgd( "Could not read the shared data in debugmode.");

   if ( debugmode)
   {
      // :TODO: avn 20050225 - way to easy for hackers to get at very sensitive data
      // maybe a swith in the db would be a better idea...
      giError = DBG;
      write2log();
   }

   return;
}

/*==========================================================================*/
void LogHexa(
   unsigned char* bytes,      // bytes to dump
   int len                    // number of bytes
   )
/*==========================================================================*/
{
      char  cString[ 3 + 1];
      int   j;

   strcat( gcMessage, "[");
   for ( j = 0; j < len; j++)
   {
      sprintf( cString , " %02X", bytes[j]);
      strcat( gcMessage, cString);
   }
   strcat( gcMessage, "]");

   return;
}


/*
==============================================================================

   Function : Writes a message in \VOTE\MENU.LOG

   Input    : none (uses global variables defined above)

   Output   : none

   Remarks  : none

==============================================================================
*/
void write2log( void)
{
   struct dosdate_t curr_date;
   struct dostime_t curr_time;
   FILE *fp;

   char cMessage[256]   = "";
   char errCode[6]      = "";


   // timestamp
   if ( giError != DBG)
   {
      _dos_getdate( &curr_date);
      _dos_gettime( &curr_time);
      sprintf( cMessage, "%.2d/%.2d/%.4d-%.2d:%.2d:%.2d",
         curr_date.day,  curr_date.month,  curr_date.year,
         curr_time.hour, curr_time.minute, curr_time.second);
   }

   // errorlevel
   switch( giError)
   {
      case NONE : strcpy( errCode, "     "); break;
      case NOK :  strcpy( errCode, "<NOK>"); break;
      case ERR :  strcpy( errCode, "<ERR>"); break;
      case DBG :  strcpy( errCode, "<DBG>"); break;
      default:    strcpy( errCode, "<???>"); break;
   }
   strcat( cMessage, errCode);

   // trace info
   sprintf( cMessage+strlen( cMessage), "%s", gcModuleName);
   if ( strlen( gcFunctionName) > 0)
   {
      sprintf( cMessage+strlen( cMessage), ".%s()", gcFunctionName);
      if ( strlen( gcMessage) == 0)
         strcpy( gcMessage, "raised error");
   }

   // message
   sprintf( cMessage+strlen( cMessage), " %s", gcMessage);

   if ( giError != DBG)
   {
      sprintf( cMessage+strlen( cMessage), " [%d]", giStatus);
   }

   strcat( cMessage, "\n");


   fp = fopen( FILENAME_LOG, "at");
   fwrite( cMessage, strlen( cMessage), 1, fp);
   fclose( fp);
} // eof write2log()

/*
==============================================================================

   Function : Writes a message in \VOTE\MENU.LOG

   Input    : PROGRAM-NAME(2),
              The message,
              Error code   * 0 = everything OK   "     "
                           * 1 = not OK status   "<NOK>"
                           * 2 = ERROR !!        "<ERR>"

   Output   : none

   Remarks  : none

==============================================================================
*/
int
addtolog( argc, argv)
   int  argc;
   char *argv[];
/*----------------------------------------------------------------*/
{
   strcpy( gcModuleName,     argv[1]);
   strcpy( gcMessage,        argv[2]);
   giStatus          = atoi( argv[3]);
   giError           = giStatus;
   gcFunctionName[0] = '\0';
   write2log();

   return( 0);
} // eof addtolog()

int
memavail( argc, argv)
   int  argc;
   char *argv[];
/*----------------------------------------------------------------*/
{
   long lmemAvail = stackavail();

   if ( prowti( "memAvail", 0, lmemAvail, 0) != 0)
   {
       promsgd( "Could not write the shared data 'memAvail'.");
   }
} // eof memavail()
