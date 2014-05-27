#include <io.h>
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>
#include <string.h>

#include "..\GEN\genlpn.h"
#include "..\MAV\LPasm.h"
#include "..\Diagnost\Setup.h"
#include "..\Diagnost\Disk.h"
#include "..\Diagnost\Lightpen.h"
#include "..\Diagnost\Config.h"

#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

#ifdef _LIGHTPEN_

	#include "..\Diagnost\General.h"

#else

	#include <graphics.h>

	#include "..\MAV\mavasm.h"   // Use the same LIGHTPEN functions as MAV
	#include "..\GEN\genvideo.h"
	#include "Diagnostic.h"
	#include "Msg.h"
	#include "Sound.h"
	#include "DosError.h"
	#include "TimeScale.h"

#endif // _LIGHTPEN_

//=============================================================================

// see order of presentation in DIAGNOST.??-files (different languages)
///////////////////////////////////////////////////////////////////////
enum lpnStatus {
	STATUS_UNKNOWN = -1,
	DRIVER_LOADED,
	OUTPUT_NOT_FOUND,
	DRIVER_NOT_LOADED,
	WRONG_CARD,
	LIGHTPEN_NOT_CONNECTED,
	LIGHTPEN_MAKES_ERRORS,
	LIGHTPEN_NOT_CALIBRATED,
	DRIVER_NOT_FOUND,
	CONFIG1_NOT_FOUND,
	CONFIG2_NOT_FOUND,
	CONFIG3_NOT_FOUND,
	IRQ10_NOT_FREE,
	WRONG_CARD_AND_IRQ10_NOT_FREE,
	NOT_APPLICABLE,
	WRONG_DRIVER,
	SYNC_CABLE_PROBLEM,
	CARD_NOT_PRESENT
};

#define VERSIONS 3

static BOOLEAN lpnTested = FALSE;

typedef struct {
	char SearchFor[10];
	BOOLEAN Found;
} pxlData;

struct {
	pxlData CARD;
	pxlData IRQ;
} PXL[VERSIONS] =
{
/*0*/{ { "PXL-385", FALSE }, { "IRQ 10", FALSE } }, // Digivote I
/*1*/{ { "PXL-395", FALSE }, { "IRQ 10", FALSE } }, // Digivote II
/*2*/{ { "PXL-595", FALSE }, { "IRQ ",   FALSE } }  // Digivote III
};

char	       *PXLdriver 			       = "PXL-BIOS rel.4.10L";
BOOLEAN        correctPXLdriver		 	   = FALSE;

//=============================================================================

char           DriverFile[]                = "A:\\PXL-BIOS.COM";
char		   DataFile[]                  = "A:\\PXLBIOS?.DTA";
char           OutputFile[]                = "?:\\PXL-BIOS.NUL";

int            i,
			   retcodeLightPen             = -1;

#ifndef _LIGHTPEN_
BOOLEAN		   Pointed                     = FALSE,
			   Moved                       = FALSE;

const      int ButtonBorder                =  2,
			   ButtonWidth                 = 10,
			   ButtonAdjustment            = ( ButtonWidth + 1) / 2,
			   ScreenBorderX               = 13,
			   ScreenBorderY               = 23;

const      int ButtonsForLightpenTest      = 16;     // 16 buttons
const long int NumberSelectedForLightpen   = 0xFFFF; // 16 bits

const          ButtonsForFastDiagTest      =  4;     //  4 buttons
const long int NumberSelectedForFastDiag   = 0x000F; //  4 bits

int			   Buttons                     = ButtonsForFastDiagTest;
long int	   NumberSelected              = NumberSelectedForFastDiag;

int            ButtonPos[16][2];

enum LIGHTPENSTATUS { CONNECTED,
					  NOT_CONNECTED,
					  WRONG_VERSION };

#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
void Set_Mode( void)
{
	videom( VGA_MODE, CARACT14x8);
}
#endif // _LIGHTPEN_

//=============================================================================

void Load_Driver( void)
{
	char CommandLine[100];

	if( !access( OutputFile, 0x00))
		remove( OutputFile);
	// Load driver
	sprintf( CommandLine, "%s @%s j6 p5 f0 a0 > %s",
						  DriverFile, DataFile, OutputFile);

	#ifdef DEBUG
	dFile << "PXL:" << CommandLine << "\n";
	dFile.flush();
	#endif //DEBUG

	system( CommandLine);
	lpnTested = FALSE;
}

//=============================================================================

#if _DEBUGMODE
   // traps return code for Driver_Loaded...
   int myDriver_Loaded( int X);
   int Driver_Loaded( int X)
   {
         int rc;
         rc = myDriver_Loaded( X);
         #if _DEBUGMODE
            sprintf( gcDcio, "DL returns [%d]!", rc); dcio();
         #endif // _DEBUGMODE

         return rc;
   }

int myDriver_Loaded( int X)
#else

int Driver_Loaded( int X)
#endif

{
	char   *LoadingPXL        = {
		"...Loading light pen driver for " // followed by CARD and IRQ
	};
	char   *MemoryResidentPXL = {
		"...Light pen driver is memory-resident and enabled - "
	};
	char   *TimeoutError      = {
		"PXL-BIOS ERROR: PXL-780 timeout error.  Unit not responding."
	};
	char   *SyncError         = {
		"PXL-BIOS ERROR: Sync problem.  Check cable."
	};
	char    Buffer[100];
	int     i;

	if( !lpnTested)
	{
		lpnTested = TRUE;
		// Initialize 'Found'-elements
		for( i = 0 ; i < VERSIONS ; i++)
		{
			PXL[i].CARD.Found = FALSE;
			PXL[i].IRQ.Found  = FALSE;
		}
		if( access( OutputFile, 0x00))
		{
			#ifdef DEBUG
			dFile << "PXL:" << OutputFile << " does not exist !\n";
			dFile.flush();
			#endif //DEBUG
			return( OUTPUT_NOT_FOUND);
		}
		ifstream RESULTS( OutputFile);
		while( !RESULTS.eof() && RESULTS)
		{
			RESULTS.getline( Buffer, sizeof( Buffer));
			if( RESULTS)
			{
            // check driver version
				if( StringPartOf( Buffer, PXLdriver))
					correctPXLdriver = TRUE;

				if( StringPartOf( Buffer, TimeoutError))
					return( STATUS_UNKNOWN);

				if( StringPartOf( Buffer, SyncError))
					return( SYNC_CABLE_PROBLEM);

				if( StringPartOf( Buffer, LoadingPXL)       ||
					StringPartOf( Buffer, MemoryResidentPXL)  )
				{
					for( i = 0 ; i < VERSIONS ; i++)
					{
						if( StringPartOf( Buffer, PXL[i].CARD.SearchFor))
							PXL[i].CARD.Found = TRUE;
						if( StringPartOf( Buffer, PXL[i].IRQ.SearchFor))
							PXL[i].IRQ.Found = TRUE;
					}
					break;
				}
			}
		}
		RESULTS.close();
	}
	#ifdef DEBUG
	char dMsg[100];
	sprintf( dMsg, "Pxldriver[<%s><%s>]\nPXL:[X=<%d>]\n",
				   PXLdriver, correctPXLdriver?"TRUE":"FALSE", X);
	dFile << dMsg;
	for( i = 0 ; i < VERSIONS ; i++)
	{
		sprintf( dMsg, "PXL:[i=<%d>][<%s><%s>][<%s><%s>]\n", i,
					   PXL[i].CARD.SearchFor,
					   PXL[i].CARD.Found ? "TRUE" : "FALSE",
					   PXL[i].IRQ.SearchFor,
					   PXL[i].IRQ.Found ? "TRUE" : "FALSE");
		dFile << dMsg;
	}
	dFile.flush();
	#endif //DEBUG

	if( !correctPXLdriver)
		return( WRONG_DRIVER);

	if( PXL[X].CARD.Found)
		if( PXL[X].IRQ.Found)
			return( DRIVER_LOADED);
		else
			return( IRQ10_NOT_FREE);
	else
		if( PXL[X].IRQ.Found)
			return( WRONG_CARD);
		else
			return( WRONG_CARD_AND_IRQ10_NOT_FREE);
}

//=============================================================================

#ifndef _LIGHTPEN_
static LIGHTPENSTATUS Get_LightpenStatus( void)
{
	if( type_lpn == UNKNOWN)
		interfacePXL();

	if( type_lpn == NO_PXL)
		return( NOT_CONNECTED);
	else
		return( lightpenconnected()
				? CONNECTED
				: NOT_CONNECTED);
}
#endif // _LIGHPTPEN

//=============================================================================

#ifndef _LIGHTPEN_
static BOOLEAN LightpenPushed( unsigned int *Column,
							   unsigned int *Row)
{
	unsigned int prevColumn,
				 prevRow;
	g_pen( 1/*mode*/, 1/*onderbr*/, 1/*PenCursor*/);

	prevColumn = *Column;
	prevRow    = *Row;

	*Column = p_ko(); //Get the latest lightpen column position
	*Row    = p_ri(); //Get the latest lightpen row    position

	switch( ((int) detection() & 3))
	{
		case 1:
			Pointed = FALSE;
			Moved   = TRUE;
			return( FALSE);

		case 3:
			if( Pointed)
			{
				if( prevColumn != *Column ||
					prevRow    != *Row      )
					Moved = TRUE;
			}
			else
				if( prevColumn == 0 &&
					prevRow    == 0)
					Moved = TRUE;
				else
					Moved = FALSE;
			Pointed = TRUE;
			return( TRUE);

		default:
			Pointed = TRUE;
			Moved   = TRUE;
			return( FALSE);
	}
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
// LightpenUsed(...) : new function by TMT 15/03/95
// rc : 0 Lightpen not used
// rc : 1 Lightpen pointed
// rc : 3 Lightpen pointed & pushed
int LightpenUsed( unsigned int *Column,
				  unsigned int *Row)
{
	int Status;

	g_pen( 1/*mode*/, 1/*onderbr*/, 1/*PenCursor*/);

	*Column    = p_ko(); //Get the latest lightpen column position
	*Row       = p_ri(); //Get the latest lightpen row    position

	switch( Status = (int) detection() & 3)
	{
		case 1:  //Light, NO Switch
		case 3:  //Light,    Switch
			return( Status);

		default:
			return( 0);
	}
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
static BOOLEAN LightpenHit( unsigned int Column,
							unsigned int Row,
							unsigned int Left,
							unsigned int Top,
							unsigned int Right,
							unsigned int Bottom)
{
	if( Column >= Left  &&
		Column <= Right &&
		Row    >= Top   &&
		Row    <= Bottom  )
	{
		BeepAccepted();
		return( TRUE);
	}
	else
		return( FALSE);
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
static void DrawButton( unsigned int X,
						unsigned int Y,
            int FGcolor )
{
	int ButtonHalf = ButtonWidth / 2;

	setfillstyle( SOLID_FILL, YELLOW); // YELLOW
	bar( X - ButtonHalf,                Y - ButtonHalf,
		 X + ButtonHalf,                Y + ButtonHalf);
	setfillstyle( SOLID_FILL, FGcolor); // WHITE
	bar( X - ButtonHalf,                Y - ButtonHalf,
		 X + ButtonHalf - ButtonBorder, Y + ButtonHalf - ButtonBorder);
	rectangle( X - ButtonHalf, Y - ButtonHalf,
			   X + ButtonHalf, Y + ButtonHalf);
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
static void EraseButton( unsigned int X,
						 unsigned int Y )
{
	int ButtonHalf = ButtonWidth / 2;

	setfillstyle( EMPTY_FILL, 0);
	bar( X - ButtonHalf, Y - ButtonHalf,
		 X + ButtonHalf, Y + ButtonHalf);
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
static BOOLEAN ButtonHit( unsigned int Column,
						  unsigned int Row,
						  unsigned int X,
						  unsigned int Y)
{
	return( LightpenHit( Column,          Row,
						 X-ButtonWidth/2, Y-ButtonWidth/2,
						 X+ButtonWidth/2, Y+ButtonWidth/2));
}
#endif // _LIGHTPEN_

//=============================================================================

#ifndef _LIGHTPEN_
#pragma argsused
static TerminatorType TestLightpen( void *SelectedButtons,
									unsigned long NrCalls,
									unsigned long MaxNrCalls)
{
	unsigned int Column,Row;

	if( LightpenPushed( &Column, &Row) &&
		Pointed && !Moved)
	{
		for( i = 0 ; i < Buttons ; i++)
		{
			if( ButtonHit( Column, Row, ButtonPos[i][0], ButtonPos[i][1]))
			{
				EraseButton(ButtonPos[i][0],ButtonPos[i][1]);
				*(unsigned int *)SelectedButtons |= 1<<i;
			}
		}
	}
	if( kbconnected())
		if( kbhit() && getch() == '\r')
			return(CANCELLED);

	return( *(unsigned int *) SelectedButtons == NumberSelected ? TERMINATE_OK
																: CONTINUE);
}
#endif // _LIGHTPEN_

//=============================================================================

				  ///////////////////
				  // MAIN ROUTINES //
				  ///////////////////

//=============================================================================

#pragma argsused
BOOLEAN DrawLightpenDriverLoaded( int Heading, unsigned int Y)
{
	#ifndef _LIGHTPEN_
	BOOLEAN JustTestIt = ( Y == TEST_PXL);
	#endif
	int driver[VERSIONS];
	int i, j;

	// Determine location of OutputFile
	if( OutputFile[0] == '?')
		OutputFile[0] = Get_RAMdrive( TRUE);

	// Determine type of lightpen interface (if any)
	if( type_lpn == UNKNOWN)
		interfacePXL();

	if( type_lpn == NO_PXL)
		retcodeLightPen = CARD_NOT_PRESENT;
	else if( lpnDriverLoaded())
		retcodeLightPen = DRIVER_LOADED;
	else
	{
		retcodeLightPen = STATUS_UNKNOWN;

		if( access( DriverFile, 0x00))
			retcodeLightPen = DRIVER_NOT_FOUND;
		else
		{
			driver[0] = 0;
			driver[1] = 0;
			driver[2] = 0;
			switch( type_lpn)
			{
				case PXL385_395:
					Bios_Test();
					switch( bios)
					{
						case AWARD_W6178MS  : // Normally PXL-595
											  // -> has ISA-slots, so ...
						case AWARD_P5SJ_B   :
							driver[0] = 2;
							driver[1] = 1;
							break;

						case AMI_REPLACEMENT:
						case AMI_1          :
						case AMI_2          :
						case AMI_3          :
						case BAD_AMI        :
							driver[0] = 1;
							driver[1] = 2;
							break;
					}
					break;

				case PXL385:
					driver[0] = 1;
					break;

				case PXL395:
					driver[0] = 2;
					break;

				case PXL595:
					// Normally : AWARD_W6178MS
					//            FUJITSU_SIEMENS
					driver[0] = 3;
					break;
			}

			i = 0;
			#ifndef _LIGHTPEN_
			// To make a difference between testing if the driver
			// has been correctly loaded (LIGHTPEN.EXE) and really
			// trying to reload the driver (e.g. Sync problem)
			if( JustTestIt)
			{
				if( lpnDriverLoaded()         &&
					!access( OutputFile, 0x00)  )
				{
					for( ; i < VERSIONS ; i++)
						if( !( retcodeLightPen = Driver_Loaded( driver[i] - 1)))
							break;
						else
							if( retcodeLightPen == STATUS_UNKNOWN    ||
								retcodeLightPen == IRQ10_NOT_FREE    ||
								retcodeLightPen == WRONG_DRIVER      ||
								retcodeLightPen == SYNC_CABLE_PROBLEM  )
								break;
				}
				i = VERSIONS;
			}
			#endif // _LIGHTPEN_
			for( ; i < VERSIONS ; i++)
			{
				// Force end of test if driver[i] == 0
				if( driver[i] == 0)
					break;

				if( lpnTested)
				{
					// Force next test to correct DTA-file
					for( j = 0 ; j < VERSIONS ; j++)
						if( PXL[j].CARD.Found &&
							PXL[j].IRQ.Found    )
						{
							driver[i] = j;
							break;
						}
					// Clear all remaining drivers ...
					for( j = i + 1 ; j < VERSIONS ; j++)
						driver[j] = 0;
				}

				DataFile[10] = '0' + driver[i];
				if( access( DataFile, 0x00))
					return( CONFIG1_NOT_FOUND + driver[i] - 1);

				// Protecting DTA-file from being overwritten by the driver
				// in case problems may occur.
				_dos_setfileattr( DataFile, FA_RDONLY | FA_HIDDEN);
				Load_Driver();
				if( !( retcodeLightPen = Driver_Loaded( driver[i] - 1)))
					// Driver correctly loaded !
					// Force end of test !
					break;
				else
					if( retcodeLightPen == STATUS_UNKNOWN    ||
						retcodeLightPen == IRQ10_NOT_FREE    ||
						retcodeLightPen == WRONG_DRIVER      ||
						retcodeLightPen == SYNC_CABLE_PROBLEM  )
						// No point to continue loading/testing driver !
						break;
			}
		}
	}
	if( retcodeLightPen == STATUS_UNKNOWN)
	{
		retcodeLightPen = DRIVER_NOT_LOADED;
	}
	#ifndef _LIGHTPEN_
	if( !JustTestIt)
		PrintTestResult( Heading, Y, retcodeLightPen
									 ? LightpenStatus + retcodeLightPen - 1
									 : LightpenDriverCorrectLoaded);
	#endif // _LIGHTPEN_

	return( retcodeLightPen == NOT_APPLICABLE
							   ? TRUE
							   : lpnDriverLoaded());
}

//=============================================================================

#ifndef _LIGHTPEN_
BOOLEAN DrawLightpenTest( int Heading, unsigned int Y)
{
	TerminatorType LightpenOK;
	unsigned int SelectedButtons = 0;
	int i;

	if( !lpnDriverLoaded())
		DrawLightpenDriverLoaded( NULL, TEST_PXL);

	if( retcodeLightPen)
	{
		PrintTestResult( Heading, Y, LightpenStatus + retcodeLightPen - 1);
		if( retcodeLightPen == NOT_APPLICABLE  ||
			retcodeLightPen == CARD_NOT_PRESENT  )
		{
			IgnoreRetryTestOnce = TRUE;
		}
		return( FALSE);
	}

	switch( Get_LightpenStatus())
	{
		case WRONG_VERSION:
			EraseTestResult( Y);
			PrintTestResult( Heading, Y, LightpenStatus + 2);
			return( FALSE);

		case NOT_CONNECTED:
			EraseTestResult( Y);
			PrintTestResult( Heading, Y, LightpenStatus + 3);
			return( FALSE);

		case CONNECTED:
			int inWindowX   = MaxPixelX - ScreenBorderX;
			int inWindowY   = MaxPixelY - ScreenBorderY;

			ButtonPos[0][0] = ButtonAdjustment + inWindowX * 0 / 4;
			ButtonPos[0][1] = ButtonAdjustment + inWindowY * 0 / 4;
			ButtonPos[1][0] = ButtonPos[0][0];
			ButtonPos[1][1] = ButtonAdjustment + inWindowY * 4 / 4;
			ButtonPos[2][0] = ButtonAdjustment + inWindowX * 4 / 4;
			ButtonPos[2][1] = ButtonPos[1][1];
			ButtonPos[3][0] = ButtonPos[2][0];
			ButtonPos[3][1] = ButtonPos[0][0];
			// 4 corners are defined at this moment
			if( _IsWizard != WIZZARD)
			{
				Buttons        = ButtonsForLightpenTest;
				NumberSelected = NumberSelectedForLightpen;
				for( int i = 0 ; i < 3 ; i ++)
				{
					// Top line between corners
					ButtonPos[i +  4][0] = ButtonAdjustment +
										   inWindowX * ( i + 1) / 4;
					ButtonPos[i +  4][1] = ButtonPos[0][1];
					// Bottom line between corners
					ButtonPos[i +  7][0] = ButtonPos[i +  4][0];
					ButtonPos[i +  7][1] = ButtonPos[1][1];
					// Left line between corners
					ButtonPos[i + 10][0] = ButtonPos[1][0];
					ButtonPos[i + 10][1] = ButtonAdjustment +
										   inWindowY * ( i + 1) / 4;
					// Right line between corners
					ButtonPos[i + 13][0] = ButtonPos[2][0];
					ButtonPos[i + 13][1] = ButtonPos[i + 10][1];
				}
			}
			DrawInstruction( Get_Msg( PushWithLightpenOnAllButtons));
			for( i = 0 ; i < Buttons ; i++)
				DrawButton( ButtonPos[i][0], ButtonPos[i][1], ( i % 2 ? WHITE : YELLOW));

			BeepInput();
			Pointed = FALSE;
			Moved   = FALSE;
			LightpenOK = ObjTimeOut( TimeInteraction,
									 TestLightpen,
									&SelectedButtons);
			for( i = 0 ; i < Buttons ; i++)
				EraseButton( ButtonPos[i][0], ButtonPos[i][1]);

			EraseInstruction( Get_Msg( PushWithLightpenOnAllButtons));
			EraseTestResult( Y);
			if( LightpenOK == TERMINATE_OK)
				return( PrintTestResult( Heading, Y, OK));
			else
				if( LightpenOK == CANCELLED)
					return( PrintTestResult( Heading, Y, Cancelled));
				else
					return( PrintTestResult( Heading, Y,
											 SelectedButtons
											 ? LightpenStatus + 5
											 : LightpenStatus + 4));
		default:
			EraseTestResult( Y);
			return( PrintTestResult( Heading, Y, InternalError));
	}
}
#endif // _LIGHTPEN_
