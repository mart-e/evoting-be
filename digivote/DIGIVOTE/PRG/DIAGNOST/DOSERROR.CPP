#include <dos.h>
#include "..\DIAGNOST\DosError.h"

int DosErrorNr = 0;

//=============================================================================

#pragma argsused
static int DosErrorHandler( int errval, int ax, int bp, int si)
{
	DosErrorNr = _DI & 0x00FF;
	return( _HARDERR_IGNORE);
}

//=============================================================================

void InstallDosErrorHandler( void)
{
	harderr( DosErrorHandler);
}

//=============================================================================
