// ========================= IDE_INFO.H ============================

#ifndef IDE_INFO_H
   #define IDE_INFO_H

typedef int BOOLEAN;

#define FALSE 0
#define TRUE  1

#if defined(_LIGHTPEN_) || defined(TEST)
   // LIGHTPEN.EXE or TEST.EXE
#else
   #define NbrDrive 4

   typedef struct IDE_struct
   {
      BOOLEAN Present;
      char    Msg[80];
      char    DrvLetter;
      long    Cyl;
      long    Hd;
      long    Sect;
      long    BIOS_Cyl;
      long    BIOS_Hd;
      long    BIOS_Sect;
      BOOLEAN Buro;
   };

   extern IDE_struct IDE[];

   extern void    Import_Drives      ( void);
#endif //defined(_LIGHTPEN_) || defined(TEST)

extern void LoadProgram( char *ProgramFile,
                   char *ProgramParams,
                   char *OutputFile);

#if defined(TEST)
   // TEST.EXE
#else
   extern BOOLEAN StringPartOf       ( char *Source, char *Search);
#endif //defined(TEST)

#endif //#ifndef IDE_INFO_H
