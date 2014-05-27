/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genlib.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.7 $</revision>
  <archiveTimeStamp>$Date: 2009/01/30 13:53:14 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>genlib.c</fileName>
  <creationDate/>
  <rights></rights>
  <author>JRU</author>
  <description>
    <fr>Librarie de logging</fr>
    <nl>Library voor logging</nl></description>
  <history/>
  <comment/>
</comments>
<header>*/

/*<includes>*/
#include <malloc.h>
#include <stdio.h>                               /* sprintf(), fopen(), fwrite(), fclose() */
#include <dos.h>                                 /* date/time */

#include "hlc.h"
#include "genlib.h"
/*</includes>*/

#define FILENAME_LOG "\\VOTE\\MENU.LOG"
#define NAMEMAXLEN   25                    // maximum length of modulename

/************************************************************/
/*                                                          */
/*  GLOBAL VARIABLES FOR THIS FILE ONLY                     */
/*                                                          */
/************************************************************/
static char gcLibName[] = "GenLib";

char gcModuleName[ NAMEMAXLEN + 1] = "";   // module name that raised error
char gcFunctionName[ NAMEMAXLEN + 1] = ""; // function name that raised error
int  giError = 0;                          // errorcode to write in log
char gcMessage[128];                       // buffer to display messages
char gcSharedVar[20];
long giStatus = 0;                         // errorcode to write back to Progress
char gcStatus[50];
int  gbINTEGER;

typedef enum {
   NONE        = 0,
   NOK         = 1,
   ERR         = 2,
   DBG         = 3
   } LOG_LEVEL;

/*</header>
<body>*/

/*<function id="LogFunctionId">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*==========================================================================*/
void LogFunctionId (
   char* cpModuleName,        // library name
   char* cpFunctionName       // function name
   )
/*==========================================================================*/
{
   sprintf( gcModuleName, "%.25s", cpModuleName);
   sprintf( gcFunctionName, "%.25s", cpFunctionName);

   return;
}
/*</code></function>

<function id="LogError">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*==========================================================================*/
void LogError (
   void
   )
/*==========================================================================*/
{
   giError = ERR;
   write2log();

   return;
}
/*</code></function>

<function id="LogDebug">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*==========================================================================*/
void LogDebug (
   void
   )
/*==========================================================================*/
{
   // :TODO: avn 20050225 - way to easy for hackers to get at very sensitive data
   // maybe a swith in the db would be a better idea...
#if _DEBUGMODE
   giError = DBG;
   write2log();
#endif

   return;
}
/*</code></function>

<function id="LogHexa">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*==========================================================================*/
void LogHexa (
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
/*</code></function>

<function id="write2log">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Writes a message in \VOTE\MENU.LOG

   Input    : none (uses global variables defined above)

   Output   : none

   Remarks  : none

==============================================================================
*/
void write2log ( void)
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
} // eof write2log
/*</code></function>

<function id="addtolog">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Writes a message in \VOTE\MENU.LOG

   Input    : PROGRAM-NAME(2),
              The message,
              Error code   * 0 = everything OK   "   "
                           * 1 = not OK status   "NOK"
                           * 2 = ERROR !!        "ERR"

   Output   : none

   Remarks  : none

==============================================================================
*/
int
addtolog ( argc, argv)
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
} // eof addtolog
/*</code></function>

<function id="memavail">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
int
memavail ( argc, argv)
   int  argc;
   char *argv[];
/*----------------------------------------------------------------*/
{
   long lmemAvail = stackavail();

   if ( prowti( "memAvail", 0, lmemAvail, 0) != 0)
   {
       promsgd( "Could not write the shared data 'memAvail'.");
   }
} // eof memavail
/*</code></function>

<function id="updSharedVar">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Update Progress Shared Variable

   Input    : value to put into Progress Shared Variable.

   Output   : none.

   Remarks  : none

==============================================================================
*/
void updSharedVar (
   void
)
/*----------------------------------------------------------------*/
{
    int ret;

    // Check whether a return variable has been defined
    if( gcSharedVar) {
        if( gbINTEGER) ret = prowti( gcSharedVar, 0, giStatus, 0);
        else           ret = prowtc( gcSharedVar, 0, gcStatus, 0);
        if( ret) {
            sprintf( gcMessage, "Could not write the shared data '%s'.", gcSharedVar);
            LogError();
            promsgd( gcMessage);
            giStatus = -1;
        }
    }
} // eof updSharedVar
/*</code></function>

<function id="InitFunction">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Initializes error handling.

   Input    : function name,
              Progress Shared Variable name,
              type of Shared Variable : TRUE = INTEGER, else CHAR

   Output   : none.

   Remarks  : none

==============================================================================
*/
void InitFunction (
   char* libName,            // Library name
   char* fctName,            // function name
    char* varName,            // name of shared variable in Progress
    int   typeINTEGER         // tyep of shared variable : TRUE = INTEGER, else CHAR
)
/*----------------------------------------------------------------*/
{
    LogFunctionId( libName, fctName);
    strcpy( gcSharedVar, varName);
    strcpy( gcMessage, "");
    strcpy( gcStatus, "");
    giStatus  = 0;
    gbINTEGER = typeINTEGER;
    updSharedVar();
} // eof InitFunction
/*</code></function>

<function id="RaiseError">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Displays error message before returning to Progress.

   Input    : globals gcFunctionName, giStatus.

   Output   : Progress shared variable '???' contains return code.

   Remarks  : none

==============================================================================
*/
void RaiseError (
   int ShowError             // Perform PROMSGD when logging ERROR
)
/*----------------------------------------------------------------*/
{
   LogError();
   if( ShowError) promsgd( gcMessage);
   updSharedVar();
} // eof RaiseError
/*</code></function>

<function id="c_checkint">
<heading>
  <description><en></en></description>
  <parameters/>
  <comment/>
</heading>
<code>*/
/*
==============================================================================

   Function : Checks if contents is numeric or not

   Input    : field to check

   Output   : Progress shared variable 'stat' contains return code.

   Remarks  : none

==============================================================================
*/
int
c_checkint ( argc, argv)
   int  argc;
   char *argv[];
/*----------------------------------------------------------------*/
{
   int i;

    /* Init error handling */
    InitFunction( gcLibName, "c_checkint", "stat", TRUE);
    if( giStatus == -1) return( 0);

    for( i = 0 ; i < strlen( argv[1]) && !giStatus ; i++)
        if( argv[1][i] < '0' || argv[1][i] > '9')
            giStatus = 1;

    updSharedVar();
    return( 0);
} // eof c_checkint
/*</code></function>

</body>
</source>
<!-- eof -->*/
