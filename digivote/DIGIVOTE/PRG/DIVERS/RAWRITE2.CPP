/*
 rawrite.c	Write a binary image to a diskette.
		Originally by Mark Becker

 Heavily modified by Guy Helmer (4/29/91) to improve performance and
 add features.

 Compiling:
	Appeared to have been written for Turbo C, so I've surrounded
	compiler-specific code in "#if defined(__TURBOC__)" and added
	code in the "#else" clauses for Microsoft C.  Under MSC, code
	should be compiled in the Large memory model.

 Usage:
	MS-DOS prompt> RAWRITE
		and follow the prompts, -or-

	MS-DOS prompt> RAWRITE [-f <file>] [-d <drive>] [-n(owait)] [-h(elp)]
		where:	-f <file> - name of disk image file
			-d <drive> - diskette drive to use, must be A or B
			-n - don't prompt for user to insert diskette
			-h - print usage information to stdout

History
-------

  1.0	-	Initial release
  1.1	-	Beta test (fixing bugs)				4/5/91
  		Some BIOS's don't like full-track writes.
  1.101	-	Last beta release.				4/8/91
  		Fixed BIOS full-track write by only
		writing 3 sectors at a time.
  1.2	-	Final code and documentation clean-ups.		4/9/91
  2.0 (ghelmer@dsuvax.dsu.edu)					4/30/92
  	-	Performance improvements
		Added command line options
		Now compiles under Microsoft C (version 5.1)

Version 2.0 Copyright 1992 Guy Helmer
    Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appears in all copies and
that both the above copyright notice and this permission notice appear
in supporting documentation.  This software is made available "as is",
and
GUY HELMER DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, WITH
REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
AND IN NO EVENT SHALL GUY HELMER BE LIABLE FOR ANY SPECIAL, INDIRECT
OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
(INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/

#include <malloc.h>
#include <bios.h>
#include <dos.h>
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FALSE	0
#define TRUE	(!FALSE)

#define SECTORSIZE	512

#define RESET  _DISK_RESET
#define LAST   _DISK_STATUS
#define READ   _DISK_READ
#define WRITE  _DISK_WRITE
#define VERIFY _DISK_VERIFY
#define FORMAT _DISK_FORMAT

FILE *image_file, *feedback_file;
int done;

static void feedback( int status, char ( *message))
{
	FILE *output_file;

	if( feedback_file)
		output_file = feedback_file;
	else
		output_file = stderr;

	fprintf( output_file, "%d\t\"%s (0x%.2X)\"", status, message, status);

	if( image_file)    fclose( image_file);
	if( feedback_file) fclose( feedback_file);
	exit( 1);
}

/*
 * Identify the error code with a real error message.
 */
void Error( int status)
{
	switch( status) {
		case 0x00:	feedback( status, "Operation Successful");
		case 0x01:	feedback( status, "Bad command");
		case 0x02:	feedback( status, "Address mark not found");
		case 0x03:	feedback( status, "Attempt to write on write-protected disk");
		case 0x04:	feedback( status, "Sector not found");
		case 0x05:	feedback( status, "Reset failed (hard disk)");
		case 0x06:	feedback( status, "Disk changed since last operation");
		case 0x07:	feedback( status, "Drive parameter activity failed");
		case 0x08:	feedback( status, "DMA overrun");
		case 0x09:	feedback( status, "Attempt to DMA across 64K boundary");
		case 0x0A:	feedback( status, "Bad sector detected");
		case 0x0B:	feedback( status, "Bad track detected");
		case 0x0C:	feedback( status, "Unsupported track");
		case 0x10:	feedback( status, "Bad CRC/ECC on disk read");
		case 0x11:	feedback( status, "CRC/ECC corrected data error");
		case 0x20:	feedback( status, "Controller has failed");
		case 0x40:	feedback( status, "Seek operation failed");
		case 0x80:	feedback( status, "Attachment failed to respond");
		case 0xAA:	feedback( status, "Drive not ready (hard disk only");
		case 0xBB:	feedback( status, "Undefined error occurred (hard disk only)");
		case 0xCC:	feedback( status, "Write fault occurred");
		case 0xE0:	feedback( status, "Status error");
		case 0xFF:	feedback( status, "Sense operation failed");
		/*
		 * Custom error messages
		 */
		case 0xA01: feedback( status, "Incorrect number of arguments");
		case 0xA02: feedback( status, "Unable to create feedback file");

		case 0xF01: feedback( status, "Can't figure out how many sectors/track for this diskette.");
		case 0xF02: feedback( status, "Cannot open Disk Image File");
		case 0xF03: feedback( status, "Drive must be A or B");
		case 0xF04: feedback( status, "Couldn't allocate track buffer");

	}
	feedback( status, "Unknown status");
}

/*
 * Identify what kind of diskette is installed in the specified drive.
 * Return the number of sectors per track assumed as follows:
 *  9	-	360 K and 720 K 5.25".
 * 15	-	1.2 M HD	5.25".
 * 18	-	1.44 M		3.5".
 */
int nsects( int drive)
{
	static int nsect[] = {18, 15, 9};

	char *buffer;
	int i, status;

	/*
	 * Read sector 1, head 0, track 0 to get the BIOS running.
	 */
	buffer = ( char *) malloc( SECTORSIZE);
	biosdisk( RESET, drive, 0, 0, 0, 0, buffer);
	status = biosdisk( READ, drive, 0, 10, 1, 1, buffer);
	if( status == 0x06)			/* Door signal change?	*/
		status = biosdisk( READ, drive, 0, 0, 1, 1, buffer);

	for( i = 0 ; i < sizeof( nsect) / sizeof( int) ; ++i) {
		biosdisk( RESET, drive, 0, 0, 0, 0, buffer);
		status = biosdisk( READ, drive, 0, 0, nsect[i], 1, buffer);
		if( status == 0x06)
			status = biosdisk( READ, drive, 0, 0, nsect[i], 1, buffer);
		if( status == 0x00)
			break;
	}
	free( buffer);

	if( i == sizeof( nsect) / sizeof( int))
		Error( 0xF01);

	return( nsect[i]);
}

void main(int argc, char *argv[])
{
	char *buffer, *bufbase;
	int drive, head, track, status, spt, buflength, ns;
	unsigned long addrtmp;
	int i;

	done = 0;

	if( argc != 4)                                           Error( 0xA01);
	if( ( feedback_file = fopen( argv[3], "wt")) == NULL)    Error( 0xA02);
	if( ( image_file = fopen( argv[1], "rb")) == NULL)       Error( 0xF02);

	drive = toupper( argv[2][0]) - 'A';
	if( drive != 0 && drive != 1)                            Error( 0xF03);

	/*
	 * Determine number of sectors per track and allocate buffers.
	 */
	spt = nsects( drive);
	buflength = spt * SECTORSIZE;
	/*
	 * Allocate double the necessary space to make the 64K DMA boundary
	 * adjustment easy.
	 */
	if( ( buffer = ( char *) malloc(buflength * 2)) == NULL) Error( 0xF04);
	bufbase = buffer;
	/*
	 * Now mangle the buffer base address to avoid physical 64Kb boundary
	 * problems with DMA.  If the end of the track buffer is not in the same
	 * physical 64Kb block as the start of the buffer, then start the track
	 * buffer in the second half of our allocated memory.
	 */
	addrtmp = ( ( ( unsigned long) FP_SEG( bufbase)) << 4) + FP_OFF( bufbase);
	if( ( addrtmp + ( unsigned long) buflength) & 0xffff0000L !=
		addrtmp & 0xffff0000L)
		/*
		 * Use the second half of the buffer.  This will work until
		 * diskette tracks exceed 64 sectors, which shouldn't be in the
		 * near future.
		 */
		bufbase += buflength;

	/*
	 * Start writing data to diskette until there is no more data to write.
	 */

	head = track = 0;
	while( fread( bufbase, buflength, 1, image_file) == 1 && !done) {
		if( ( status = biosdisk( WRITE, drive, head, track, 1, spt, bufbase)) != 0)
			Error( status);

		if( !( head = ( head + 1) & 1))
			++track;
	}
	if( feof( image_file))
		biosdisk( READ, drive, 0, 0, 1, 1, buffer);		/* Retract head	*/

	/*
	 * Following instruction doesn't indicate an error,
	 * just a simple way to quit the program
	 *
	 * 0x00 = Operation Successful
	 */
	Error( 0x00);
} /* end main */