/*R*/
//======================== Revision history ===============================
/*
	 04/05/94   RVH: Split of config.cpp in setup.cpp and config.cpp.
					 Setup.cpp contains the functions that access cmos.
					 Setup is used primary by config.cpp, diagxx.cpp, cmos.cpp.
					 Config.cpp is used for functions that display the configuration
*/
//=========================================================================
/*r*/

//======================== Include files ==================================
#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <ctype.h>
#include <fstream.h>
#include <graphics.h>
#include <process.h>	// spawnlp
#include <errno.h>		// errno (when spawnlp generates an error)
#include <sys\stat.h>

//=============================================================================

#include "..\Diagnost\Config.h"

#if !defined(_LIGHTPEN_) && !defined(CMOS)

#include "General.h"
#include "Diagnost.h"
#include "Msg.h"
#include "Sound.h"
#include "TimeScal.h"
#include "Setup.h"
#include "DosError.h"
#include "Disk.h"
#include "Lightpen.h"
#include "IDE_Info.h"

#include "..\Gen\GenLPN.h"

//=============================================================================

BOOLEAN DrivesReady = FALSE;
char Buffer[80];

#endif // CMOS

//=============================================================================

#if !defined(_LIGHTPEN_) && !defined(CMOS)

void DrawStatusDiskInfo( int Task, int Tasks)
{
   settextjustify( LEFT_TEXT, BOTTOM_TEXT);
   settextstyle( DEFAULT_FONT, HORIZ_DIR, 0);

   if( Task > 0)
   {
	  setcolor( BLACK);
	  outtextxy( 0, TimeScaleTop - 2, Get_Msg( DiskInformation + Task - 1));
   }
   if( Task > Tasks)
	  ClearTimeScale();
   else
   {
	  setcolor( LIGHTGRAY);
	  outtextxy( 0, TimeScaleTop - 2, Get_Msg( DiskInformation + Task));
	  DrawTimeScale( Task, Tasks);
   }
}

//=============================================================================

char *Get_DiskInfo( int Drive, BOOLEAN LoadOnly)
{
	float x = 1024,
	      y =  512;
	long  result;

	if( !DrivesReady)
	{
		Import_Drives();
		DrivesReady = TRUE;
	}
	if( LoadOnly)
		return( NULL);
	else
	{
		if( IDE[Drive].Buro == FALSE)
		{
			result = IDE[Drive].Cyl  *
			         IDE[Drive].Hd   *
			         IDE[Drive].Sect * y / x / x;
			if( result > 0)
				sprintf( Buffer, "Cyl:%5ld Head:%3ld Sect:%5ld = %ldMb",
				                 IDE[Drive].Cyl,
				                 IDE[Drive].Hd,
				                 IDE[Drive].Sect,
				                 result);
			else
				strcpy( Buffer, Get_Msg( NotPresent));
		}
		else
			strcpy( Buffer, IDE[Drive].Msg);
		return( ( char *) Buffer);
	}
}

//=============================================================================

char Get_DrvLetter( int Drive)
{
	return( IDE[Drive].DrvLetter);
}

//=============================================================================

char *Get_DrvType( int Drive)
{
   return( ( char *) IDE[Drive].Msg);
}

//=============================================================================

static void DrawTypeFloppy( int Heading,
							unsigned int Y,
							unsigned int DriveType)
{
   char Buffer[20];

   switch(DriveType)
   {
	  case 0x0:
		 PrintTestResult( Heading, Y, NotPresent);
		 break;

	  case 0x1:
	  case 0x2:
	  case 0x3:
	  case 0x4:
		 PrintTestResult( Heading, Y, DriveTypes + DriveType - 1);
		 break;

	  default:
		 sprintf( Buffer, "Type %d", DriveType);
		 PrintResult( Heading, Y, noIndex, Buffer);
   }
}
#pragma argsused
int DummyFilter2( char *string )
{
	return 1;
}

//=============================================================================

				// MAIN ROUTINES //
				///////////////////

BOOLEAN DrawMachineID( int Heading, unsigned int Y)
{
	char Character;
	char MachineIDName[8+1],
		 chkMachineID[8+1],
	     msgThereCanBeOnlyOne[256];
	long l;
	BOOLEAN CorrectID = FALSE,
			NewID     = FALSE,
			CancelID  = FALSE;

	Bios_Test();
	GetMachineName( MachineIDName);

	if( strcmp( MachineIDName, "00000000")==0 ||
		strcmp( MachineIDName, "M0000000")==0 ||
		strcmp( MachineIDName, "U0000000")==0 ||
		strcmp( MachineIDName, "P0000000")==0 ||
		strcmp( MachineIDName, "T0000000")==0)
		strcpy( MachineIDName, "");

	if( strcmp( MachineIDName, "") == 0)
		UnknownMachineID = TRUE;

	// Force MAV if URN/PRP/TOT => LIGHTPEN.EXE problem/bug !!!
	///////////////////////////////////////////////////////////
	// if( forcedMachineID( MachineIDName))
	// 	strcpy( MachineIDName, "");
	///////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////
	// Only for _TSTMAVD1_
	///////////////////////////////////////////////////////////
	// If another system detected than MAV,
	// but a lightpen card has been detected,
	// add 9000000 to the current MachineID
	///////////////////////////////////////////////////////////
	if( !UnknownMachineID              &&
	    CurrExe          == _TSTMAVD1_ &&
	    MachineIDName[0] != 'M'        &&
	    interfacePXL()   != NO_PXL        )
	{
		sprintf( MachineIDName, "M%07lu", atoi( &MachineIDName[1]) + 9000000);
	}

	do
	{
		if( strcmp( MachineIDName, "") == 0 )
		{
			NewID = TRUE;
			switch( bios)
			{
				case AMI_REPLACEMENT :
				case AMI_1  :
				case AMI_2  :
				case AMI_3  :
				case BAD_AMI:
					CorrectID = TRUE;
					switch( _system)
					{
						case MAV :
							MachineIDName[0] = 'M';
							break;
/*
 * Following systems for Digivote I have been depricated :
 *    - URN
 *    - PRP
 *    - TOT
 *
						case URN :
							MachineIDName[0] = 'U';
							break;

						case PRP :
							MachineIDName[0] = 'P';
							break;

						case TOT :
							MachineIDName[0] = 'T';
							break;
*/
						default  :
							MachineIDName[0] = '?';
					}

					MachineIDName[1] = 0;
					EditInstruction( Get_Msg( GiveYourMachineID),
									 MachineIDName, 1, sizeof(MachineIDName),
									 FALSE, TRUE, TRUE,
									 DummyFilter2
								   );
					CancelID = !strncmp( &MachineIDName[1], "ESC", 3);
					break;

				case FS_SCENIC_D1561 :
				case FS_SCENICO_D1711:
				case AWARD_UPGRADE   :
				case AWARD_W6178MS   :
				case AWARD_P5SJ_B    :
					unsigned char machine_type;

					MachineIDName[0] = 0;
					EditInstruction( Get_Msg( GiveYourMachineID),
									 MachineIDName, 0, sizeof(MachineIDName),
									 FALSE, TRUE, TRUE,
									 DummyFilter2);
					CancelID = !strncmp( MachineIDName, "ESC", 3);
					machine_type = Determine_System( atol( MachineIDName ));
					switch( _system )
					{
						case MAV :
							if( machine_type == 'M')
								CorrectID = TRUE;
							break;

						case URN :
							if( machine_type == 'U')
								CorrectID = TRUE;
							break;

						case PRP :
							if( machine_type == 'P')
								CorrectID = TRUE;
							// break;

						case TOT :
							if( machine_type == 'T')
								CorrectID = TRUE;
							break;

						default  :
							CorrectID = FALSE;
							break;
					}

					break;

			}
			if( !CancelID)
			{
				switch( bios)
				{
					case AMI_REPLACEMENT :
					case AMI_1  :
					case AMI_2  :
					case AMI_3  :
					case BAD_AMI:
						l = atol( MachineIDName + 1);
						sprintf( MachineIDName + 1, "%07lu", l);
						break;

					case AWARD_UPGRADE:
					case AWARD_W6178MS:
					case AWARD_P5SJ_B :
						l = atol( MachineIDName);
						sprintf( MachineIDName, "%08lu", l);
						break;
				}

				if( CorrectID )
					WriteMachineName( MachineIDName);
				else
					strcpy( MachineIDName, "");
			}

		}

		if( strcmp( MachineIDName, "00000000") == 0 ||
			strcmp( MachineIDName, "M0000000") == 0 ||
			strcmp( MachineIDName, "U0000000") == 0 ||
			strcmp( MachineIDName, "P0000000") == 0 ||
			strcmp( MachineIDName, "T0000000") == 0 ) {
			strcpy( MachineIDName, "");
		}
		if( !strcmp( MachineIDName, ""))
		{
			EraseInstruction( Get_Msg( GiveYourMachineID ) );
			DrawInstruction( Get_Msg( BAD_ID ) );
			sleep( 2 );
			EraseInstruction( Get_Msg( BAD_ID ) );
		}
		/* Force Digivote I Systems to be a MAV instead of being a URN, PRP or TOT */
		/* URN, PRP and TOT aren't supported anymore !                             */
		switch( bios)
		{
			case AMI_REPLACEMENT :
			case AMI_1  :
			case AMI_2  :
			case AMI_3  :
			case BAD_AMI:
				if( MachineIDName[0] != 'M') {
					MachineIDName[0]  = 'M';
					NewID = TRUE; /* force writing new ID to CMOS */
				}
		}
	} while( !strcmp(MachineIDName,""));

	if( !CancelID)
	{
		if( NewID)
		{
			#ifdef YELLOW_DISK
			if( Check_Yellow_Configuration())
			{
				RestartCONFIG = TRUE;
				return( FALSE);
			}
			GetMachineName( chkMachineID);

			if( strcmp( chkMachineID, "00000000")==0 ||
				strcmp( chkMachineID, "M0000000")==0 ||
				strcmp( chkMachineID, "U0000000")==0 ||
				strcmp( chkMachineID, "P0000000")==0 ||
				strcmp( chkMachineID, "T0000000")==0   )
				return( FALSE);
			#endif
		}
		if( CurrExe == _TSTMAVD1_)
		{
			sprintf(  MessageDirectory, "A:\\%s.RES", MachineIDName);
			sprintf( TraceLogDirectory, "A:\\%s.LOG", MachineIDName);
		}
		// MessageFileName  -> Wincor
		// Only last file (good or bad)
		///////////////////////////////
		sprintf( fnMessage, "%s\\%s\0",  MessageDirectory,
										 MachineIDName);
		// TraceLogFileName -> Steria
		// ALL the files !! (good and bad)
		//////////////////////////////////
		sprintf( fnTraceLog, "%s\\%s\0", TraceLogDirectory,
										 MachineIDName);
		// Start registration ...
		if( !ResultLogHeader( Heading, Y, MachineIDName))
		{
			EraseScreen();
			DrawTitle( Get_Msg( Limitation));
			sprintf( msgThereCanBeOnlyOne, Get_Msg( ThereCanBeOnlyOne), MachineIDName);
			DrawInstruction( msgThereCanBeOnlyOne);
			LockSystem( _IsWizard);
		}
		WriteToFile = FALSE;
		PrintResult( Heading, Y, noIndex, MachineIDName);

		// Force MachineIDName if not MAV !!
        ////////////////////////////////////
		NewID = FALSE;
		switch( bios)
		{
			case AMI_1  :
			case AMI_2  :
			case AMI_3  :
			case BAD_AMI:
				if( MachineIDName[0] != 'M')
					NewID = TRUE;
				break;

			case AWARD_UPGRADE:
			case AWARD_W6178MS:
			case AWARD_P5SJ_B :
				if( Determine_System( atol( MachineIDName)) != 'M')
					NewID = TRUE;
				break;
		}
		if( NewID)
			WriteMachineName( MachineIDName);
		////////////////////////////////////

		if( snFailed) {
			ofstream ForceID( "A:\\POST.BAT");
			if( ForceID) {
				ForceID << "Echo " << MachineIDName << " | CMOS.EXE -reset";
				ForceID.close();
				EraseScreen();
				DrawTitle( Get_Msg( SystemSetup));
				DrawInstruction( Get_Msg( SystemWrongConfigured));
				Wait( 3);
				RebootMachine();
			}
		}

		strncpy( currentMachineID, MachineIDName, 8);
		currentMachineID[8] = 0;

		return( TRUE);
	}
	else
		return( FALSE);
}

//=============================================================================

BOOLEAN DrawMemory( int Heading, unsigned int Y)
{
	unsigned BaseMemory, ExtMemory;
	char Buffer[20];

	BaseMemory = GetBaseMemorySize();
	ExtMemory  = GetExtMemorySize();
	sprintf( Buffer, "%uKB + %uKB", BaseMemory, ExtMemory);
	PrintResult( Heading, Y, noIndex, Buffer);
	return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeFloppyA( int Heading, unsigned int Y)
{
   DrawTypeFloppy( Heading, Y, GetByteFromCMOS(0x10)>>4);
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeFloppyB( int Heading, unsigned int Y)
{
   DrawTypeFloppy( Heading, Y, GetByteFromCMOS(0x10)&0xF);
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeDiskPM( int Heading, unsigned int Y)
{
   PrintResult( Heading, Y, noIndex, Get_DiskInfo( 0, FALSE));
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeDiskPS( int Heading, unsigned int Y)
{
   PrintResult( Heading, Y, noIndex, Get_DiskInfo( 1, FALSE));
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeDiskSM( int Heading, unsigned int Y)
{
   PrintResult( Heading, Y, noIndex, Get_DiskInfo( 2, FALSE));
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawTypeDiskSS( int Heading, unsigned int Y)
{
   PrintResult( Heading, Y, noIndex, Get_DiskInfo( 3, FALSE));
   return( TRUE);
}

//=============================================================================

BOOLEAN DrawKeyboardConnected( int Heading, unsigned int Y)
{
   unsigned char far *keyb=(unsigned char *)MK_FP(0x40,0x97);

   PrintTestResult( Heading, Y, *keyb&16 ? Connected
										 : NotConnected);
   return( TRUE);
}

//=============================================================================
//rvh bios type passed as parameter

BOOLEAN ResetMachineName( BIOS_TEST bios)
{
   char MachineIDName[8+1];
   char buffer[320];
   char c;
//   char default_name[7] = { 0x1b, 0x7b, 0x21, 0, 0, 0, 0};
   char default_name[7] = { 0, 0, 0, 0, 0, 0, 0};

   DrawTitle( Get_Msg( MachineName));

   switch( bios)
   {
	  case FS_SCENIC_D1561  :
	  case FS_SCENICO_D1711 :
	  case AWARD_UPGRADE    :
	  case AWARD_W6178MS    :
	  case AWARD_P5SJ_B 	:
	  case AMI_1     		:
	  case AMI_2     		:
	  case AMI_3     		:

		 GetMachineName( MachineIDName);

		 sprintf( buffer, Get_Msg( MachineNameReset),
				  ( strcmp( MachineIDName, "") ? MachineIDName
											   : "Not Defined"));

		 if( SelectItem( buffer       			,
						 AcceptPossibilities	,
						 getmaxy() - 30 		 ))
			WriteMachineName( default_name);

		 break;

	  case BAD_AMI :

		 DrawInstruction( Get_Msg( BadRelease));
		 ObjTimeOut( 5, WaitOnTimeOut, NULL);
		 break;

	  default:

		 DrawInstruction( Get_Msg( NotAmi));
		 ObjTimeOut( 5, WaitOnTimeOut, NULL);
		 break;
   }

   return TRUE;
}

//=============================================================================

#endif // CMOS