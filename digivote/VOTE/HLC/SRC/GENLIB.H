/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="genlib.h">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2009/01/30 13:50:40 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>GenLib.h</fileName>
  <creationDate/>
  <rights>Steria Benelux</rights>
  <author>JRU</author>
  <description>
    <fr>header file</fr>
    <nl>header file</nl>
  </description>
  <history></history>
  <comment></comment>
</comments>
<header>*/

#define _DEBUGMODE 0                     /* debug mode Off=0, On=1 */
   /* :WARNING: avn 20020201 -> turn OFF for production code!
      Available memory under Progress is low, preview.exe may
      fail to load otherwise... */

/* global data */
#ifndef TRUE
#define TRUE  (1)
#define FALSE (0)
#endif

/* Error handling */
extern char gcModuleName[];   // Holds module name that raised error
extern char gcFunctionName[]; // Holds function name that raised error
extern int giError;           // Holds errorcode to write in log
extern char gcMessage[];      // Holds buffer to display messages
extern char gcSharedVar[20];  // Holds name of shared variable in Progress
extern long giStatus;         // Holds errorcode to write back to Progress
extern char gcStatus[50];     // Holds returnvalue to write back to Progress
extern int  gbINTEGER;        // Holds type of returnvalue : TRUE = INTEGER, else CHAR

/*<prototypes><!-- these functions are public -->*/
void write2log( void);

void LogFunctionId(
   char* cpModuleName,        // library name
   char* cpFunctionName       // function name
   );
void LogError(
   void
   );
void LogDebug(
   void
   );
void LogHexa(
   unsigned char* bytes,      // bytes to dump
   int len                    // number of bytes
   );
void updSharedVar(
   void
);
void InitFunction(
   char* libName,            // Library name
   char* fctName,            // function name
    char* varName,            // name of shared variable in Progress
    int   typeINTEGER         // tyep of shared variable : TRUE = INTEGER, else CHAR
);
void RaiseError(
   int ShowError             // Perform PROMSGD when logging ERROR
);
/*</prototypes>*/

/*</header>
</source>
<!-- eof -->*/
