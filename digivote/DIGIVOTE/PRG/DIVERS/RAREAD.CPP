/* Program: RAREAD
   Version: 1.1
   Written by: Brian E. Reifsnyder
   Copyright: 2001 by Brian E. Reifsnyder, under the terms of the GNU GPL 2.

   Note: This is a clone of the RAREAD program.

   Program Use: This program is used to create disk images that can be
		written back to a floppy disk with RAWRITE.
*/

#include <bios.h>
#include <dos.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys\stat.h>

#define  VERSION "1.1"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef unsigned char byte;
typedef byte ERRCODE;


/*======================================================================*/
static byte	driveno = 0;
static unsigned	max_track, max_head, max_sector;

static char	buffer[32u*2u*512u];
static size_t	bufsz;

static FILE	*fp;

static char CONTENTS[12] = "?:\\CONTENTS";


/*----------------------------------------------------------------------*/
ERRCODE	BIOS_drive_info(void);
ERRCODE	BIOS_reset_drive(void);
int	disk2file(void);
int	examine_boot_sector(void);
void	_fgets(void);
void	handle_error(ERRCODE);
void	HELP(void);

/*======================================================================*/
ERRCODE BIOS_drive_info(void)
{
	_DL = driveno, _AH = 8;
	geninterrupt(0x13);

	if((_FLAGS & 1) == 0) _AH = 0; /* no errors if no carry flag */

	_DL = _DH, _DH = 0, max_head = _DX,
	_DL = _CL & 0x3F, max_sector = _DX;
	asm xchg cl,ch;
	asm rol  ch,1;
	asm rol  ch,1;
	_CH &= 3, max_track = _CX;

	return _AH;
}

ERRCODE BIOS_reset_drive(void)
{
	_AH = 0;
	geninterrupt(0x13);
	if((_FLAGS & 1) == 0) _AH = 0;
	return _AH;
}

void log2contents( char* message) {
	struct dosdate_t curr_date;
	struct dostime_t curr_time;

	if( access( CONTENTS, 0) == 0)
	{
		chmod( CONTENTS, S_IWRITE|S_IREAD);
		if((fp = fopen( CONTENTS, "at")) != NULL)
		{
			_dos_getdate(&curr_date);
			_dos_gettime(&curr_time);
			sprintf( buffer, "\n%.2d/%.2d/%.4d-%.2d:%.2d:%.2d - %s",
							 curr_date.day,  curr_date.month,  curr_date.year,
							 curr_time.hour, curr_time.minute, curr_time.second,
							 message);
			fwrite(buffer, strlen( buffer), 1, fp);
			fclose(fp);
		}
		chmod( CONTENTS, S_IREAD);
	}
}

void handle_error(ERRCODE errcode){
	static byte err_list[] = { 1, 2, 3, 4, 8, 9, 16, 32, 64, 128, 255 };
	static char *err_text[] = {
		/*  1*/ "invalid argument",
		/*  2*/ "bad sector ID",
		/*  3*/ "write protect error",
		/*  4*/ "read error",
		/*  8*/ "DMA overrun",
		/*  9*/ "DMA 64K boundary error",
		/* 16*/ "CRC error",
		/* 32*/ "controller failure",
		/* 64*/ "seek failure",
		/*128*/ "device timeout",
	};
	int errno = 0;

	do errno++;
	while(err_list[errno-1] < errcode);

	if(err_list[errno-1] == errcode && errcode != 255)
		printf("\n%s\n", err_text[errno-1]);
	else
		printf("\nerror #%u\n", errcode);

	/* if no errorcode (errcode == 0), prevent wrong interpretation ! */
	if( errcode == 0) errcode = -1;

	log2contents("FLOPPY FAILURE DETECTED !");
}

void HELP(void)
{
    printf(
	"RAREAD " VERSION ", Copyright 2001 by Brian E. Reifsnyder\n"
	"under the terms of the GNU GPL version 2.\n\n"
	"RAREAD option information:\n\n"
	//"  -f <file>\t- specify disk image file\n"
	"  -d <drive>\t- specify diskette drive to use;\n"
	"\t\t\tmust be either A or B\n"
	"  -n\t\t- don't wait for user to insert diskette --\n"
	"\t\t\tassumes diskette is waiting in selected drive\n"
	"  -h\t\t- print this help message and exit\n\n");
}

int disk2file(void)
{
	ERRCODE errcode;

	unsigned track = 0;

	/* If there is a FAT12 boot sector use the BPB parameters for
	   accessing the floppy disk, otherwise use the BIOS information. */
	if(0!=examine_boot_sector() )
	{
		errcode = BIOS_drive_info();
		if(errcode)
		{
			handle_error(errcode);
			return EXIT_FAILURE;
		}
	}
	bufsz = max_sector * 512;

	/* write diskette to file 1 track at a time */
	/* ==> writing to diskette has been disabled : main purpose is to test the diskette */
	do
	{
	    unsigned head = 0;
	    do
	    {
		printf("\rReading-->    Cyl:  %2u   Head:  %1u", track, head);

		if(0!=biosdisk(2,driveno,head,track,1,max_sector,buffer) )
		{
			BIOS_reset_drive();
			if(0!=biosdisk(2,driveno,head,track,1,max_sector,buffer) )
			{
				BIOS_reset_drive();
				if(0!=biosdisk(2,driveno,head,track,1,max_sector,buffer) )
				{
					handle_error(errcode);
					return EXIT_FAILURE;
				}
			}
		}
		/*
		if(fwrite(buffer, 1, bufsz, fp) < bufsz)
		{
			printf("\nwrite failure\n");
			return EXIT_FAILURE;
		}
		*/
		head++;
	    }while(head <= max_head);
	    track++;
	}while(track <= max_track);
	printf("\rOperation Complete.\t\t\t\t\n");
	return EXIT_SUCCESS;
}

void _fgets(/*char[] = buffer, size_t = sizeof(buffer), FILE* = stdin*/)
{
	size_t i = 0;
	do
	{
		int c = fgetc(stdin);
		if(c == EOF || c == '\n') break;
		buffer[i] = c, i++;
	}while(i < sizeof(buffer)-1);
	buffer[i] = '\0';
}

int examine_boot_sector(void)
/* Returns 0 if a valid FAT12 boot sector was found. */
{
	int valid_media_descriptor=TRUE;

	unsigned int  * total_sectors     =(unsigned int *) (&buffer[0x13]);
	unsigned char * media_descriptor  =(unsigned char *)(&buffer[0x15]);
	unsigned int  * sectors_per_track =(unsigned int *) (&buffer[0x18]);
	unsigned char * sides             =(unsigned char *)(&buffer[0x1a]);
	unsigned char * file_system_p     =(unsigned char *)(&buffer[0x36]);

	_ES = FP_SEG(buffer), _BX = FP_OFF(buffer),
	_DH = 0,
	_CH = 0,
	_CL = 1,
	_AL = 1,
	_DL = driveno,
	_AH = 2;
	geninterrupt(0x13);

	if((_FLAGS & 1) != 0) handle_error(_AH);

	max_track     =((unsigned int)  total_sectors[0]/
			(unsigned char) sides[0]/
			(unsigned int)  sectors_per_track[0])-1;
	max_head      = (unsigned) sides[0]-1;
	max_sector    = (unsigned int)sectors_per_track[0];

	/* Ensure that the media descriptor is correct. */
	if( (media_descriptor[0]!=0xf0)
	 && (media_descriptor[0]< 0xf9) )
	  valid_media_descriptor=FALSE;

	if(
	    ( (media_descriptor[0]==0xf0 )
	    &&(total_sectors[0]!=(2*80*18) ) )
	 ||
	    ( (media_descriptor[0]==0xf9 )
	    &&(total_sectors[0]!=(2*80*15) ) )
	 ||
	    ( (media_descriptor[0]==0xf9 )
	    &&(total_sectors[0]!=(2*80*9 ) ) )
	 ||
	    ( (media_descriptor[0]==0xfa )
	    &&(total_sectors[0]!=(1*80*8 ) ) )
	 ||
	    ( (media_descriptor[0]==0xfb )
	    &&(total_sectors[0]!=(2*80*8 ) ) )
	 ||
	    ( (media_descriptor[0]==0xfc )
	    &&(total_sectors[0]!=(1*40*9 ) ) )
	 ||
	    ( (media_descriptor[0]==0xfd )
	    &&(total_sectors[0]!=(2*40*9 ) ) )
	 ||
	    ( (media_descriptor[0]==0xfe )
	    &&(total_sectors[0]!=(1*40*8 ) ) )
	 ||
	    ( (media_descriptor[0]==0xff )
	    &&(total_sectors[0]!=(2*40*8 ) ) )
	  ) valid_media_descriptor=FALSE;

	if(valid_media_descriptor==FALSE)
	{
		/* If the media descriptor is unlisted or invalid, do one
		   final sanity check before assuming that the disk is not
		   FAT12.                                                   */
		if( (file_system_p[0] !='F')
		 || (file_system_p[1] !='A')
		 || (file_system_p[2] !='T')
		 || (file_system_p[3] !='1')
		 || (file_system_p[4] !='2') ) return(1);
	}

	return(0);
}

/*======================================================================*/
int cdecl main(int argc, char **argv)
{
	int wait = 1;
	char ch;
	char *drive = NULL, *result = NULL/*, *fname = NULL*/;

	/*--- parse command line ---*/
	for(;;)
	{
		argv++;
		if(--argc == 0) break;
		ch = (*argv)[0];
		if((ch == '-' || ch == '/') && (*argv)[2] == 0)
		{
			ch = (*argv)[1];
			/* filename is obsolete due to the fact we want to test the 'quality' of the diskette
			if(ch == 'F' || ch == 'f')
			{
				if(--argc == 0)
				{
					printf("filename must follow -f option\n");
					return EXIT_FAILURE;
				}
				fname = *++argv;
				continue;
			}
			*/
			if(ch == 'D' || ch == 'd')
			{
				if(--argc == 0)
				{
					printf("drive letter must follow -d option\n");
					return EXIT_FAILURE;
				}
				drive = *++argv;
				CONTENTS[0] = drive[0];
				continue;
			}
			if(ch == 'N' || ch == 'n')
			{
				wait = 0;
				continue;
			}
			if(ch == 'R' || ch == 'r')
			{
				if(--argc == 0)
				{
					printf("filename must follow -r option\n");
					return EXIT_FAILURE;
				}
				result = *++argv;
				continue;
			}
			if(ch == 'H' || ch == 'h' || ch == '?')
			{
				HELP();
				return EXIT_SUCCESS;
			}
		}
		printf("raread: %s - unknown option.\n"
		       "use 'raread -h' for instructions.\n", *argv);
		return EXIT_FAILURE;
	}

	/*--- check input parameters ---*/
	if(drive == NULL)
	{
		printf("Enter source diskette drive: ");
		_fgets();
		drive = buffer;
	}
	ch = drive[0];
	if(/*ch == 'A' ||*/ ch == 'a') /*driveno = 0,*/ ch = 'A';
	if(  ch == 'B' ||   ch == 'b')   driveno = 1,   ch = 'A';
	if(ch != 'A' || drive[1] != 0)
	{
		printf("Invalid drive letter.\n");
		return EXIT_FAILURE;
	}
	/* filename is obsolete due to the fact we want to test the 'quality' of the diskette
	if(fname == NULL)
	{
		printf("Enter disk image destination file name: ");
		_fgets(); fname = buffer;
	}
	if((fp = fopen(fname, "wb")) == NULL)
	{
		printf("Unable to create file.\n");
		return EXIT_FAILURE;
	}
	*/
	/*--- go ---*/
	if(wait)
	{
		printf("Please insert the source diskette"
			" into drive %c: and press -ENTER- :", 'A'+driveno);
		_fgets();
	}
	{
		log2contents("START FLOPPY CHECK");

		register int ret = disk2file();

		log2contents("END FLOPPY CHECK");
		/*
		fclose(fp);
		*/
		if(result != NULL)
		{
			if((fp = fopen(result, "wt")) == NULL)
			{
				printf("Unable to create file.\n");
				return EXIT_FAILURE;
			}
			else
			{
				sprintf( buffer, "%d", ret);
				fwrite(buffer, strlen( buffer), 1, fp);
				fclose(fp);
			}
		}
		return ret;
	}
}
