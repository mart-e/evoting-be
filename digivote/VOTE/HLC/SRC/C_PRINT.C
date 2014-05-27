/************************************************************/
/*                                                          */
/*  Filename            : C_PRINT.C                         */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 09/06/2000                        */
/*                                                          */
/*  Functionality       : print a file to LPT1-port         */
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
#include <stdlib.h>
#include <stdio.h>
#include <bios.h>
#include <dos.h>

#include "hlc.h"

/************************************************************/
/*                                                          */
/*      CONSTANT DEFINITIONS                                */
/*                                                          */
/************************************************************/
#define LPT1         0

#define TIME_OUT     0x01
#define IO_ERROR     0x08
#define SELECTED     0x10
#define OUT_OF_PAPER 0x20
#define ACKNOWLEDGE  0x40
#define NOT_BUSY     0x80

#define OPSYS        -1
#define FAST         register

#define LINELENGTH   132

const char PageBreak = '\012';

long prts, offset, flength, init_r, init_n;
FILE *streamin,
     *streamout;

c_print( argc, argv)
    int   argc;
    char *argv[];
{
    unsigned      status;
    char          abyte;
    struct find_t ffblk;

    prts = 0;
    if( ! _dos_findfirst( argv[1],
                          _A_NORMAL | _A_HIDDEN | _A_RDONLY,
                          &ffblk))
        flength = ffblk.size;

    if( ( streamin = fopen( argv[1], "rt")) == NULL)
    {
        prts = -1;
        if( prowti( "stat", 0, prts, 0))
            promsgd( "Could not write the shared data 'stat'.");
        return( 0);
    }
    if( prordi( "offset", 0, &offset, 0))
    {
        promsgd( "Could not read the shared data 'offset'.");
        return( 0);
    }
    if( offset == -1)
    {
        if( FixPrint( argv[1]))
        {
            if( ! _dos_findfirst( argv[1],
                                  _A_NORMAL | _A_HIDDEN | _A_RDONLY,
                                  &ffblk))
                flength = ffblk.size;

            if( ( streamin = fopen( argv[1], "rt")) == NULL)
            {
                prts = -6;
                if( prowti( "stat", 0, prts, 0))
                    promsgd( "Could not write the shared data 'stat'.");
                return( 0);
            }
            offset = 0;
        }
        else
        {
            if( prowti( "stat", 0, prts, 0))
                promsgd( "Could not write the shared data 'stat'.");
            return( 0);
        }
    }
    else
    {
        if( offset > flength)
        {
            prts = -2;
            if( prowti( "stat", 0, prts, 0) != 0)
                promsgd( "Could not write the shared data 'stat'.");
            return( 0);
        }
    }
    init_r = offset ? 1
                    : 0;
    init_n = init_r;
    fseek( streamin, offset, SEEK_SET);
    while( !feof( streamin) && !prts)
    {
        abyte  = getc( streamin);
        status = _bios_printer( _PRINTER_WRITE, LPT1, abyte);
        if( status & TIME_OUT)      prts += 1;
        if( status & IO_ERROR)      prts += 2;
        if( status & OUT_OF_PAPER)  prts += 4;

        if( !prts)
        {
            if(      abyte == '\r') init_r = 1;
            else if( abyte == '\n') init_n = 1;
        }
    }
    if( prts)
    {
        if( init_r &&
            init_n   )
        {
            offset = ftell( streamin);
            if( status & 0x01)
                offset--;
        }
        else
            offset = 0;
        if( prowti( "flength", 0, flength, 0))
            promsgd( "Could not write the shared data 'flength'.");
        if( prowti( "offset",  0, offset,  0))
            promsgd( "Could not write the shared data 'offset'.");
    }
    fclose( streamin);
    if( !prts)
        remove( argv[1]);

    if( prowti( "stat", 0, prts, 0))
        promsgd( "Could not write the shared data 'stat'.");
    return( 0);
}

FixLine( char Contents[LINELENGTH])
{
   int x, y;

   for( x = 0 ; x < strlen(Contents) ; x++)
   {
       if( x > 0)
           y = x - 1;
       else
           y = x;

       if( Contents[y] != '\r'   &&
           Contents[y] != '\012' &&
           Contents[x] == '\n'     )
           putc( '\r', streamout);
       putc( Contents[x], streamout);
   }
}

FixPrint( char OrigFile[80])
{
    char Buffer     [LINELENGTH],
         LastBuffer [LINELENGTH],
         Prev1Buffer[LINELENGTH],
         Prev2Buffer[LINELENGTH],
         Prev3Buffer[LINELENGTH],
         tmpDrive   [_MAX_DRIVE],
         tmpDir     [_MAX_DIR  ],
         tmpFname   [_MAX_FNAME],
         TempFile   [    80    ];
    int  Printing = 0, i = 0, j;

    _splitpath( OrigFile, tmpDrive, tmpDir, tmpFname, NULL);
    _makepath(  TempFile, tmpDrive, tmpDir, tmpFname, ".$$$");

    if( ( streamout = fopen( TempFile, "wt")) == NULL)
    {
        prts = -3;
        return( 0);
    }
    while( !feof( streamin))
    {
        switch( i)
        {
            case 0 :                            /* Before reading line 1 */
                break;                             

            case 1 :                            /* Before reading line 2 */
                strcpy( Prev1Buffer, Buffer);      
                break;

            case 2 :                            /* Before reading line 3 */
                strcpy( Prev2Buffer, Prev1Buffer); 
                strcpy( Prev1Buffer, Buffer);
                break;

            case 3 :                            /* Before reading line 4 */
                strcpy( Prev3Buffer, Prev2Buffer); 
                strcpy( Prev2Buffer, Prev1Buffer);
                strcpy( Prev1Buffer, Buffer);
                break;

            default:                            /* Printing is enabled now */
                Printing = 1;                   
                strcpy( LastBuffer,  Prev3Buffer);
                strcpy( Prev3Buffer, Prev2Buffer);
                strcpy( Prev2Buffer, Prev1Buffer);
                strcpy( Prev1Buffer, Buffer);
                break;
        }
        fgets( Buffer, sizeof(Buffer), streamin);
        i++;
        if( Printing)
            FixLine( LastBuffer);
    }
    fclose( streamin);
    if( !strncmp( PageBreak, Buffer, 1) ||
        !strncmp( "PG",      Buffer, 2))
        if( !strncmp( PageBreak, Prev1Buffer, 1) ||
            !strncmp( "PG",      Prev1Buffer, 2))
            if( !strncmp( PageBreak, Prev2Buffer, 1) ||
                !strncmp( "PG",      Prev2Buffer, 2))
                if( !strncmp( PageBreak, Prev3Buffer, 1) ||
                    !strncmp( "PG",      Prev3Buffer, 2))
                    {}
                else
                    FixLine( Prev3Buffer);
            else
            {
                FixLine( Prev3Buffer);
                FixLine( Prev2Buffer);
            }
        else
        {
            FixLine( Prev3Buffer);
            FixLine( Prev2Buffer);
            FixLine( Prev1Buffer);
        }
    else
    {
        FixLine( Prev3Buffer);
        FixLine( Prev2Buffer);
        FixLine( Prev1Buffer);
        FixLine( Buffer);
    }
    putc( PageBreak, streamout);
    fclose( streamout);
    if( remove( OrigFile))
    {
        prts = -4;
        return( 0);
    }
    if( rename( TempFile, OrigFile))
    {
        prts = -5;
        return( 0);
    }
    return( 1);
}

