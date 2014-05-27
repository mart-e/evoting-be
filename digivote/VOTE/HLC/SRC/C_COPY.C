/************************************************************/
/*                                                          */
/*  Filename            : C_COPY.C                          */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 10/03/2000                        */
/*                                                          */
/*  Functionality       : copy file argv1 to file argv2/3   */
/*                                                          */
/*  Last Reviewed by    :                                   */
/*                                                          */
/*  Date of Last Review :                                   */
/*                                                          */
/*  Based on Change Req :                                   */
/*                                                          */
/*  Approved by         :                                   */
/*                                                          */
/*  Date of Approval    :                                   */
/*                                                          */
/************************************************************/

/************************************************************/
/*                                                          */
/*      INCLUDE FILES                                       */
/*                                                          */
/************************************************************/
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <process.h>
#include <dos.h>

#include "hlc.h"

/************************************************************/
/*                                                          */
/*      CONSTANT DEFINITIONS                                */
/*                                                          */
/************************************************************/
#define TRUE  (1)
#define FALSE (0)
#define BLOCK 5120

/************************************************************/
/*                                                          */
/*      GLOBAL VARIABLES FOR THIS FILE ONLY                 */
/*                                                          */
/************************************************************/
unsigned char   data[BLOCK];
FILE            *streamin,
                *streamout;
long            fileok;

void setverify( int On)
{
    union REGS Reg;

    Reg.h.ah = 0x2E;
    Reg.h.al = On;
    Reg.h.dl = 0x00;
    intdos( &Reg, &Reg);
}

c_copy( argc, argv)
    int argc;
    char *argv[];
{
    unsigned char *infile,
                  *outfile;
    unsigned int  numread;
    long llength;
    char message[80];
    int i, j;

    fileok = TRUE;
    if( prowti( "fileok", 0, fileok, 0) != 0)
    {
        promsgd( "Could not write the shared data in fileok.");
        return( 0);
    }

    outfile = argv[argc - 1]; /* Last  parameter */
    if( ( streamout = fopen( outfile, "wb")) == NULL)
    {
        sprintf( message, "Unable to create %s !", outfile);
        promsgd( message);
        prosleep( 5);
        fileok = FALSE;
        if( prowti( "fileok", 0, fileok, 0) != 0)
            promsgd( "Could not write the shared data in fileok.");
        return( 0);
    }

    setverify( TRUE);
    for( i = 1 ; i < ( argc - 1) ; i++)
    {
        infile  = argv[i];
        if( ( streamin = fopen( infile, "rb")) == NULL)
        {
            fclose( streamout);
            sprintf( message, "Unable to open %s !", infile);
            promsgd( message);
            prosleep( 5);
            fileok = FALSE;
            if( prowti( "fileok", 0, fileok, 0) != 0)
                promsgd( "Could not write the shared data in fileok.");
            return( 0);
        }
        llength  = filelength( fileno( streamin));
        while( llength > 0)
        {
            if( ( numread = fread( &data[0], 1, BLOCK, streamin)) <= 0)
            {
                fclose( streamin);
                fclose( streamout);
                setverify( FALSE);
                sprintf( message, "Unable to read from %s !", infile);
                promsgd( message); 
                prosleep( 5);
                fileok = FALSE;
                if( prowti( "fileok", 0, fileok, 0) != 0)
                    promsgd( "Could not write the shared data in fileok.");
                return( 0);
            }
            llength -= numread;
            if( fwrite( &data[0], 1, numread , streamout) != numread)
            {
                fclose( streamin);
                fclose( streamout);
                setverify( FALSE);
                sprintf( message, "Unable to write to %s !", outfile);
                promsgd( message); 
                prosleep( 5);
                fileok = FALSE;
                if( prowti( "fileok", 0, fileok, 0) != 0)
                    promsgd( "Could not write the shared data in fileok.");
                return( 0);
            }
        } /* end while */
        fclose( streamin);
    } /* end for */
    fclose( streamout);
    setverify( FALSE);
    return( 0);
}
