//=======================================================================

#ifndef DISK_H
#define DISK_H

#include "..\Diagnost\General.h"

#ifdef DIAGNOST
#define sizeTestFile  10 * 1024
#define avgTestFile  512 + 2048
#endif // DIAGNOST

extern char Get_RAMdrive( BOOLEAN Force_Drive);

#ifdef DIAGNOST
char *Drive_RemovableDisk( void);
char *Type_RemovableDisk( void);

extern BOOLEAN DrawRamDriveTest( int Heading, unsigned int Y);
extern BOOLEAN DrawFloppyATest( int Heading, unsigned int Y);
extern BOOLEAN DrawFloppyBTest( int Heading, unsigned int Y);
extern BOOLEAN DrawRemovableDisk1Test( int Heading, unsigned int Y);
extern BOOLEAN DrawRemovableDisk2Test( int Heading, unsigned int Y);
#endif // DIAGNOST

#endif
