#ifndef CONFIG_H
#define CONFIG_H
#include "..\DIAGNOST\General.h"

extern BOOLEAN StringPartOf( char *Source, char *Search);
extern void Extract_Tools( char *SystemFileToCopy,
						   char *SelfExtractFile,
						   char RamDrive,
						   BOOLEAN SFX_File);
extern char *Get_DiskInfo( int Drive, BOOLEAN LoadOnly);
extern char *Get_DrvType( int Drive);
extern char Get_DrvLetter( int Drive);
extern BOOLEAN DrawMachineID( int Heading, unsigned int Y);
extern BOOLEAN DrawMemory( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeFloppyA( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeFloppyB( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeDiskPM( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeDiskPS( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeDiskSM( int Heading, unsigned int Y);
extern BOOLEAN DrawTypeDiskSS( int Heading, unsigned int Y);
extern BOOLEAN DrawKeyboardConnected( int Heading, unsigned int Y);
#endif
