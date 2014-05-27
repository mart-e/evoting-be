//======================== Include files ==================================

#include <io.h>
#include <dir.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream.h>
#include <process.h>	// spwanlp, system
#include <errno.h>		// errno (when spawnlp generates an error)
#include <sys/stat.h>

//=============================================================================
#include "..\diagnost\IDE_Info.h"

#if defined(_LIGHTPEN_) || defined(TEST)
   // LIGHTPEN.EXE or TEST.EXE
#else
   #include "Disk.h"    // Get_RAMdrive()
   #include "Diagnost.h"

   //=============================================================================

   IDE_struct IDE[NbrDrive];

   char IDE_Info_File[]  = "?:\\IDE_INFO.LST";
   char RamDrv           = Get_RAMdrive( TRUE);

   #if !defined( DIAGNOST) && !defined( CMOS)

   char Buffer[100];

   char CommandATAPICD[] = "?:\\ATAPI-CD.EXE",
       CommandIDEATA[]  = "?:\\IDE-ATA.EXE",
       CommandFDISK[]   = "?:\\FDISK.EXE",
       CommandGUEST[]   = "?:\\GUEST.EXE";

   char OutputSCRDUMP[]  = "?:\\SCRDUMP.NUL",
       OutputATAPICD[]  = "?:\\ATAPI-CD.NUL",
       OutputIDEATA[]   = "?:\\IDE-ATA.NUL",
       OutputFDISK[]    = "?:\\FDISK.NUL",
       OutputGUEST[]    = "?:\\GUEST.NUL";

   char BatchFDISK[]     = "?:\\FDISK.BAT";

   #endif // !DIAGNOST && !CMOS
   //=============================================================================

   void Initialize_IDE_Info( void)
   {
      // Initialize DRIVE-structure / table
      for( int i = 0 ; i < NbrDrive ; i++)
      {
         IDE[i].Present   = FALSE;
         IDE[i].DrvLetter = '?';
         IDE[i].Cyl       = -1;
         IDE[i].Hd        = -1;
         IDE[i].Sect      = -1;
         IDE[i].BIOS_Cyl  = -1;
         IDE[i].BIOS_Hd   = -1;
         IDE[i].BIOS_Sect = -1;
         IDE[i].Buro      = FALSE;
         memset( IDE[i].Msg, '*', sizeof( IDE[i].Msg) - 1);
      }
   }

   //=============================================================================
#endif //defined(_LIGHTPEN_) || defined(TEST)

void LoadProgram( char *ProgramFile,
				  char *ProgramParams,
				  char *OutputFile)
{
	char CommandLine[80];
	int currDrive = getdisk();

	remove( OutputFile);
	if( !access( ProgramFile, 0x00))
	{
		if( ProgramFile[1] == ':')
		   if( toupper( ProgramFile[0]) - 'A' != currDrive)
		      setdisk( toupper( ProgramFile[0]) - 'A');
		sprintf( CommandLine, "%s %s", ProgramFile, ProgramParams);
		freopen( OutputFile, "w+t", stdout);
		freopen( "NUL",      "w+t", stderr);
		system( CommandLine);
		freopen( "CON", "wt", stderr);
		freopen( "CON", "wt", stdout);
        setdisk( currDrive);
	}
	else
	{
		ofstream FORCE( OutputFile, ios::trunc);
		FORCE << "Missing: " << ProgramFile << "\n";
		FORCE.close();
	}
}

//=============================================================================

#if defined(TEST)
   // TEST.EXE
#else

BOOLEAN StringPartOf( char *Source, char *Search)
{
	char *TempString;
	int i;

	for( i=0 ; i<strlen( Source) ; i++)
	{
		TempString=&Source[i];
		if( strncmp( TempString, Search, strlen(Search))==0)
			return( TRUE);
	}
	return( FALSE);
}

#endif //defined(TEST)

#if defined(_LIGHTPEN_) || defined(TEST)
   // LIGHTPEN.EXE or TEST.EXE
#else

//=============================================================================

#if !defined( DIAGNOST) && !defined( CMOS)

void Detect_Drives( void)
{
	long t, c, h, s, result;
	int currDrive, currFloptical, fixedDrives, i, j, k;
	char currDrvLetter;
	BOOLEAN CriticalErrors = FALSE,
		    ZIP_Found      = FALSE,
		    sectDRIVE      = FALSE,
		    sectPARTITION  = FALSE,
		    IDE_devices    = FALSE;
	char Message[100], token[40];
	ifstream CfgFile;

	// Analyze result of SCRDUMP (detection of CD-ROM, CD-RW, DVD, ...)
	currDrive        = -1;
	CriticalErrors   = FALSE;
	OutputSCRDUMP[0] = RamDrv;
	CfgFile.open( OutputSCRDUMP);
	if( !CfgFile) CriticalErrors = TRUE;
	while( CfgFile && !CfgFile.eof() && !CriticalErrors)
	{
		CfgFile.getline( Buffer, sizeof( Buffer));
		if( !CfgFile && !CfgFile.eof())
			CriticalErrors=TRUE;
		else if( StringPartOf( Buffer, "Primary Channel")   &&
				 StringPartOf( Buffer, "Master"))
				currDrive = 0;

		else if( StringPartOf( Buffer, "Primary Channel")   &&
				 StringPartOf( Buffer, "Slave"))
				currDrive = 1;

		else if( StringPartOf( Buffer, "Secondary Channel") &&
				 StringPartOf( Buffer, "Master"))
				currDrive = 2;

		else if( StringPartOf( Buffer, "Secondary Channel") &&
				 StringPartOf( Buffer, "Slave"))
				currDrive = 3;

		else if( StringPartOf( Buffer, "CD-ROM Model"))
		{
			sprintf( IDE[currDrive].Msg, "CD type %s", &Buffer[29]);
			IDE[currDrive].Buro = TRUE;
		}
	}
	CfgFile.close();

	// Scan IDE-ports using IDE-ATA (detection of HDD's)
	CommandIDEATA[0] = RamDrv;
	OutputIDEATA[0]  = RamDrv;
	LoadProgram( CommandIDEATA, "-lines:0 -dvi+", OutputIDEATA);
	remove( CommandIDEATA);
	// Analyze result of ATA-IDE ...
	currDrive      = -1;
	fixedDrives    = 0;
	CriticalErrors = FALSE;
	CfgFile.open( OutputIDEATA);
	if( !CfgFile) CriticalErrors = TRUE;
	while( CfgFile && !CfgFile.eof() && !CriticalErrors)
	{
		CfgFile.getline( Buffer, sizeof( Buffer));
		if( !CfgFile && !CfgFile.eof())
			CriticalErrors=TRUE;
		else if( StringPartOf( Buffer, "IDE/ATA-Info:  ")     &&
				 StringPartOf( Buffer, "drive 0")             &&
				 StringPartOf( Buffer, "primary controller"))
			currDrive = 0;

		else if( StringPartOf( Buffer, "IDE/ATA-Info:  ")     &&
				 StringPartOf( Buffer, "drive 1")             &&
				 StringPartOf( Buffer, "primary controller"))
			currDrive = 1;

		else if( StringPartOf( Buffer, "IDE/ATA-Info:  ")     &&
				 StringPartOf( Buffer, "drive 0")             &&
				 StringPartOf( Buffer, "secondary controller"))
			currDrive = 2;

		else if( StringPartOf( Buffer, "IDE/ATA-Info:  ")     &&
				 StringPartOf( Buffer, "drive 1")             &&
				 StringPartOf( Buffer, "secondary controller"))
			currDrive = 3;

		else if( StringPartOf( Buffer, "Issue IDENTIFY command."))
			IDE[currDrive].Present = TRUE;

		else if( StringPartOf( Buffer, "Model Number"))
		{
			i = strlen( Buffer) - 39;
			if( i >= sizeof( IDE[currDrive].Msg))
				i = sizeof( IDE[currDrive].Msg);
			strncpy( IDE[currDrive].Msg, &Buffer[39], i);
			IDE[currDrive].Msg[i] = '\0';
			fixedDrives++;
		}
		else if( StringPartOf( Buffer, "Current Cylinders Number")         ||
				 StringPartOf( Buffer, "Current Heads Number")             ||
				 StringPartOf( Buffer, "Current Sectors Per Track Number") ||
				 StringPartOf( Buffer, "Current BIOS cylinders number")    ||
				 StringPartOf( Buffer, "Current BIOS heads number")        ||
				 StringPartOf( Buffer, "Current BIOS sectors number"))
		{
			i = strlen( Buffer) - 39;
			if( i >= sizeof( IDE[currDrive].Msg))
				i = sizeof( IDE[currDrive].Msg);
			strncpy( Message, &Buffer[39], i);
			Message[i] = '\0';
			for( i = 0, j = 0 ; i < strlen( Message) ; i++)
				if( Message[i] >= '0' && Message[i] <= '9')
					Message[j++] = Message[i];
			Message[j] = '\0';
			if(      StringPartOf( Buffer, "Cylinders"))
				IDE[currDrive].Cyl       = atol( Message);
			else if( StringPartOf( Buffer, "Heads"))
				IDE[currDrive].Hd        = atol( Message);
			else if( StringPartOf( Buffer, "Sectors"))
				IDE[currDrive].Sect      = atol( Message);
			else if( StringPartOf( Buffer, "BIOS cylinders"))
				IDE[currDrive].BIOS_Cyl  =
					StringPartOf( IDE[currDrive].Msg, "SyQuest")
						?IDE[currDrive].Cyl
						:atol( Message);
			else if( StringPartOf( Buffer, "BIOS heads"))
				IDE[currDrive].BIOS_Hd   =
					StringPartOf( IDE[currDrive].Msg, "SyQuest")
						?IDE[currDrive].Hd
						:atol( Message);
			else if( StringPartOf( Buffer, "BIOS sectors"))
				IDE[currDrive].BIOS_Sect =
					StringPartOf( IDE[currDrive].Msg, "SyQuest")
						?IDE[currDrive].Sect
						:atol( Message);
		}
	}
	CfgFile.close();

	// Scan IDE-ports using ATAPI-CD (detection of ATAPI devices)
	CommandATAPICD[0] = RamDrv;
	OutputATAPICD[0]  = RamDrv;
	strcpy( Message, "-lines:0");
	for( currDrive = 0, IDE_devices = FALSE ; currDrive < 4 ; currDrive++)
	{
		if( IDE[currDrive].Present == FALSE)
		{
			sprintf( token, " -ds%d:0", currDrive);
			strcat( Message, token);
		}
		else
			IDE_devices = TRUE;
	}
	if( IDE_devices)
	{
		LoadProgram( CommandATAPICD, Message, OutputATAPICD);
		remove( CommandATAPICD),
		// Analyze result of ATAPI-CD ...
		currDrive      = -1;
		ZIP_Found      = FALSE;
		CriticalErrors = FALSE;
		CfgFile.open( OutputATAPICD);
		if( !CfgFile) CriticalErrors = TRUE;
		while( CfgFile && !CfgFile.eof() && !CriticalErrors)
		{
			CfgFile.getline( Buffer, sizeof( Buffer));
			if( !CfgFile && !CfgFile.eof())
				CriticalErrors = TRUE;
			else
				if( StringPartOf( Buffer, "primary drive")       &&
					StringPartOf( Buffer, "primary controller"))
					currDrive = 0;

			else if( StringPartOf( Buffer, "secondary drive")     &&
					 StringPartOf( Buffer, "primary controller"))
				currDrive = 1;

			else if( StringPartOf( Buffer, "primary drive")       &&
					 StringPartOf( Buffer, "secondary controller"))
				currDrive = 2;

			else if( StringPartOf( Buffer, "secondary drive")     &&
					 StringPartOf( Buffer, "secondary controller"))
				currDrive = 3;

			else if( StringPartOf( Buffer, "Model number"))
			{
				i = strlen( Buffer) - 36;
				if( i > 0 && ( ( toupper( Buffer[36]) >= 'A' &&
					             toupper( Buffer[36]) <= 'Z')  ||
					           (          Buffer[36]  >= '0' &&
					                      Buffer[36]  <= '9')    ))
				{
					if( i >= sizeof( IDE[currDrive].Msg))
						i = sizeof( IDE[currDrive].Msg);
					strncpy( IDE[currDrive].Msg, &Buffer[36], i);
					IDE[currDrive].Msg[i] = '\0';
					IDE[currDrive].Buro = TRUE;
					if( !ZIP_Found)
						ZIP_Found = StringPartOf( IDE[currDrive].Msg, "ZIP");
				}
			}
		}
		CfgFile.close();
	}
	if( fixedDrives > 0 &&
		RamDrv > 'C')
	{
		BatchFDISK[0]   = RamDrv;
		OutputFDISK[0]  = RamDrv;
		CommandFDISK[0] = RamDrv;
		currDrvLetter   = 'C';
		// Scan detected HDD(s) for known FAT12/FAT16 partitions ...
		ofstream BatchFile( BatchFDISK);
		BatchFile << "@Echo OFF\n";
		for( i = 0 ; i < fixedDrives ; i++)
			BatchFile << CommandFDISK << " " << ( i + 1) << " /INFO /TECH\n";
		BatchFile.close();
		LoadProgram( BatchFDISK, NULL, OutputFDISK);
		// Analyze result of FDISK ...
		currDrive      = -1;
		sectDRIVE      = FALSE;
		sectPARTITION  = FALSE;
		CriticalErrors = FALSE;
		CfgFile.open( OutputFDISK);
		if( !CfgFile) CriticalErrors = TRUE;
		while( CfgFile && !CfgFile.eof() && !CriticalErrors)
		{
			CfgFile.getline( Buffer, sizeof( Buffer));
			if( !CfgFile && !CfgFile.eof())
				CriticalErrors = TRUE;
			else if( strlen( Buffer) > 0)
			{
		 		if( StringPartOf( Buffer, "Current")      &&
					StringPartOf( Buffer, "fixed")        &&
					StringPartOf( Buffer, "disk")         &&
					StringPartOf( Buffer, "drive:")       &&
					StringPartOf( Buffer, "TC:")          &&
					StringPartOf( Buffer, "TH:")          &&
					StringPartOf( Buffer, "TS:"))
				{
					sectDRIVE     = TRUE;
					sectPARTITION = FALSE;
					token[0] = '\0';
					c = 0;
					h = 0;
					s = 0;
					for( i = 0, j = 0, k = 0 ; i < strlen( Buffer) ; i++)
					{
						if( Buffer[i] == ' ' ||
							Buffer[i] == ')'   )
						{
							if( k > 0)
							{
								token[k] = '\0';
								k = 0;
								switch( j)
								{
									case 7: //Cylinders;
										c = atol( token);
										break;

									case 9: //Heads
										h = atol( token);
										break;

									case 11: //Sectors
										s = atol( token);
										break;
								}
							}
						}
						else
						{
							if( k == 0) j++;
							token[k++] = Buffer[i];
						}
					}
					if( k > 0) token[k] = '\0';
						currDrive = -1;
					for( i = 0 ; i < NbrDrive && currDrive == -1 ; i++)
						if(   IDE[i].DrvLetter  == '?'      &&
						    ( IDE[i].BIOS_Cyl  ==  c    ||
						      IDE[i].BIOS_Cyl  ==  c + 1  ) &&
						    ( IDE[i].BIOS_Hd   ==  h    ||
						      IDE[i].BIOS_Hd   ==  h + 1  ) &&
						    ( IDE[i].BIOS_Sect ==  s    ||
						      IDE[i].BIOS_Sect ==  s + 1  )   )
							currDrive = i;
					if( currDrive == -1) sectPARTITION = FALSE;
				}
				else if( sectDRIVE     == TRUE                 &&
						 sectPARTITION == FALSE                &&
						 StringPartOf( Buffer, "Partition")    &&
						 StringPartOf( Buffer, "Status")       &&
						 StringPartOf( Buffer, "Mbytes")       &&
						 StringPartOf( Buffer, "Description")  &&
						 StringPartOf( Buffer, "Usage")        &&
						 StringPartOf( Buffer, "Start Cyl")    &&
						 StringPartOf( Buffer, "End Cyl"))
				{
					sectDRIVE     = FALSE;
					sectPARTITION = TRUE;
				}
				else if( sectDRIVE     == FALSE &&
						 sectPARTITION == TRUE  &&
						 Buffer[1]     != ' '   &&
						 StringPartOf( Buffer, "FAT1")) // FAT12 & FAT16
				// To avoid using driveletters assigned to FAT32 partitions
				// we use are own drive letter numbering !
				// see : currDrvLetter;
				if( IDE[currDrive].DrvLetter == '?')
					IDE[currDrive].DrvLetter = currDrvLetter++;
			}
		}
		CfgFile.close();
	}

	// Assigning drive letter(s) to ZIP device(s)
	// if there are any ...
	if( ZIP_Found)
	{
		CommandGUEST[0] = RamDrv;
		OutputGUEST[0]  = RamDrv;
		// Scan & assign driveletters to IOMEGA ZIP devices
		LoadProgram( CommandGUEST, "", OutputGUEST);
		//Analyze result of IOMEGA ZIP drives ...
		CriticalErrors = FALSE;
		CfgFile.open( OutputGUEST);
		if( !CfgFile) CriticalErrors = TRUE;

		while( CfgFile && !CfgFile.eof() && !CriticalErrors)
		{
		CfgFile.getline( Buffer, sizeof( Buffer));
		if( !CfgFile && !CfgFile.eof())
			CriticalErrors = TRUE;
		else
			if( StringPartOf( Buffer, "is drive"))
			{
				if( StringPartOf( Buffer, "drive is drive letter"))
					currFloptical = Buffer[strlen( Buffer) - 2] ;
				else
					currFloptical = Buffer[4];

				for( i = 0 ; i < NbrDrive ; i++)
					if( StringPartOf( IDE[i].Msg, "ZIP"))
						IDE[i].DrvLetter = currFloptical++;
				break;
			}
		}
		CfgFile.close();
	}
	//End of Detection

	#ifdef _DEBUG
	ofstream DEBUG ( "IDE_INFO.LOG");
	DEBUG << "#===== IDE Information =====\n";
	for( i = 0 ; i < NbrDrive ; i++)
	{
	 DEBUG << "# Port" << i
		   << "(" << IDE[i].DrvLetter << ") "
		   << IDE[i].Msg       << "\n#       "
		   << IDE[i].Cyl       << "/"
		   << IDE[i].Hd        << "/"
		   << IDE[i].Sect      << "     "
		   << IDE[i].BIOS_Cyl  << "/"
		   << IDE[i].BIOS_Hd   << "/"
		   << IDE[i].BIOS_Sect << "\n";
	}
	DEBUG << "#===========================\n";
	DEBUG.close();
	#endif // _DEBUG
}

#endif // !DIAGNOST && !CMOS

//=============================================================================

void Import_Drives( void)
{
	int  fHandle;

	_fmode = O_BINARY;
	IDE_Info_File[0] = RamDrv;
	Initialize_IDE_Info();
	if( ( fHandle = open( IDE_Info_File, S_IREAD)) == -1)
	{
#if !defined( CMOS) && !defined( DIGTOOLS)
		if( CurrExe != _TSTMAVD1_)
#endif
			printf( "\nError reading IDE_Info from IDE_INFO.LST !\n");
	}
	else
	{
		read( fHandle, IDE, sizeof( IDE));
		close( fHandle);
	}
}

//=============================================================================

#if !defined( DIAGNOST) && !defined( CMOS)

void Export_Drives( void)
{
	int fHandle;

	_fmode = O_BINARY;
	IDE_Info_File[0] = RamDrv;
	if( ( fHandle = creat( IDE_Info_File, S_IWRITE)) == -1)
		printf( "\nError writing IDE_Info to IDE_INFO.LST !\n");
	else
	{
		write( fHandle, IDE, sizeof( IDE));
		close( fHandle);
	}
}

//=============================================================================

void main( void)
{
    char CabinetParameters[] = "-gD1@gn0st1c -y ?:\\";
	int i;

	printf( "\nQuerying system ... please be patient ...");
	Initialize_IDE_Info();
	for( i = 0 ; i < strlen( CabinetParameters) ; i++)
		if( CabinetParameters[i] == '?')
			CabinetParameters[i] = RamDrv;
	LoadProgram( "A:\\TOOLSCAB.EXE", CabinetParameters, "NUL");
	Detect_Drives();
	Export_Drives();
	printf( "\n");
}
#endif // !DIAGNOST && !CMOS

#endif //defined(_LIGHTPEN_) || defined(TEST)

// ===================================== EOF ==================================