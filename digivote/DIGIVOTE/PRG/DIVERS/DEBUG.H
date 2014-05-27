//
// /!\ deprecated source, see divers\logging.h instead /!\
//
#error ..\divers\debug.h deprecated, use ..\divers\logging.h instead!

#ifndef _DEBUG_H_

   #define _DEBUG_H_

   #define LogFile "\\VOTE\\MENU.LOG"

   typedef enum RESULTS {
     EMPTY,
     OK,
     NOK,
     ERR
   };

   void Add_To_Logging( RESULTS ErrorCode, char *LogInfo);
   void logOnStartup( void);
   void logOnExit( void);

#endif // _DEBUG_H_
//
// /!\ deprecated source, see divers\logging.h instead /!\
//


