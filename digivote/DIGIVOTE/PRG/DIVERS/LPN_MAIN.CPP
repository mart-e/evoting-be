#include <stdio.h>
#include <string.h>

#include "..\gen\genlpn.h"
#include "..\Diagnost\Lightpen.h"
#include "..\Diagnost\DosError.h"

unsigned int TEST_PXL = 65535;

typedef enum lpn_status {
	NOT_PRESENT,
	LOADED,
	NOT_LOADED,
	SYNC_ERROR,
	WRONG_DRIVER
};

lpn_status main( int argc, char *argv[])
{
	lpn_status status      = NOT_PRESENT;
	BOOLEAN    VerboseMODE = FALSE;

	if( argc == 2)
		VerboseMODE = ( !strcmp( argv[1], "-v") ||
						!strcmp( argv[1], "-V")   );

	if( interfacePXL() != NO_PXL)
	{
		// if PXL-BIOS is already loaded,
		// don't bother to try loading it again
		if( lpnDriverLoaded())
		{
			retcodeLightPen = 0;
			status          = LOADED;
		}
		else
		{
			InstallDosErrorHandler();
			status = DrawLightpenDriverLoaded( NULL, TEST_PXL)
					 ? LOADED
					 : NOT_LOADED;
		}
		if( VerboseMODE)
		{
			switch( retcodeLightPen)
			{
				case 14:
					printf( "Wrong PXL-BIOS driver !\n");
					status = WRONG_DRIVER;
					break;

				case 15:
					printf( "Sync problem ! Check cable !\n");
					status = SYNC_ERROR;
					break;
			}
			printf( "Lightpen driver loaded = %s\n", status == LOADED
													 ?"TRUE"
													 :"FALSE");
		}
	}
	else
		if( VerboseMODE)
			printf( "No Lightpen Interface Card present !\n");

	return( status);
}

