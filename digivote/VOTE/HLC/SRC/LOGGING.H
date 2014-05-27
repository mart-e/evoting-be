/* global data */
   /* Error handling */
extern char gcModuleName[];   // Holds module name that raised error
extern char gcFunctionName[]; // Holds function name that raised error
extern long int giStatus;     // Holds errorcode to write back to Progress
extern int giError;           // Holds errorcode to write in log
extern char gcMessage[];      // Holds buffer to display messages

/* prototyping */
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
