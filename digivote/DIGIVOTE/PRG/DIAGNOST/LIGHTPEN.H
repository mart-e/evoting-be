//=======================================================================

#ifndef LIGHTPEN_H
   #define LIGHTPEN_H

#include "..\Diagnost\General.h"

extern int          retcodeLightPen;
extern unsigned int TEST_PXL;

extern void Set_Mode( void);

// Checks if the driver is correctly loaded
extern BOOLEAN DrawLightpenDriverLoaded( int Heading, unsigned int Y);

// Checks if lightpen is connected.
// Draws 4 buttons in the corners of the screen and checks if the lightpen
// can select them.
extern BOOLEAN DrawLightpenTest( int Heading, unsigned int Y);
extern BOOLEAN DrawLightpenDriverLoaded( int Heading, unsigned int Y);
#endif
