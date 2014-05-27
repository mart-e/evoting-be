/************************************************************/
/*                                                          */
/*                                                          */
/*  Filename            : C_EXEC.C                          */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 08/03/2000                        */
/*                                                          */
/*  Functionality       : Uitvoeren van een DOS-commando    */
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
#include <stdlib.h>
#include <process.h>
#include <dos.h>

#include "hlc.h"

/************************************************************/
/*                                                          */
/*      GLOBAL VARIABLES FOR THIS FILE ONLY                 */
/*                                                          */
/************************************************************/
void ( _interrupt _far *oldinterr)( void);
static void _far Handler( unsigned deverr,
                          unsigned doserr,
                          unsigned _far *hdr);
long fileok;

c_exec( argc, argv) /* Main loop of program */
    int argc;
    char *argv[];
{
    int Result, i;
    char StdIn[20];

    oldinterr = _dos_getvect(36);

    _harderr( Handler);

    fileok = 1;
    if( prowti( "fileok", 0, fileok, 0)!=0)
        promsgd( "Could not write the shared data in fileok.");

    if( getenv( "RAM") == NULL)
        sprintf( StdIn, "C:\\STDIN");
    else
        sprintf( StdIn, "%s:\\STDIN", getenv( "RAM"));

    /* If STDIN does not exist, take 'nul' as stdin */
    if( access( StdIn, 00))
        sprintf( StdIn, "nul");

    freopen( StdIn, "rt", stdin );
    freopen( "nul", "wt", stdout);
    freopen( "nul", "wt", stderr);

    Result = spawnvp( P_WAIT, argv[1], &argv[1]);

    freopen( "con", "rt", stdin );
    freopen( "con", "wt", stdout);
    freopen( "con", "wt", stderr);

    if( strcmp( StdIn, "nul"))
        remove( StdIn);

    if( Result)
    {
        fileok = 0;
        if( prowti( "fileok", 0, fileok, 0) != 0)
            promsgd( "Could not write the shared data in fileok.");
    }

    _dos_setvect( 36, oldinterr);
    return( 0);
}

static void _far Handler( unsigned deverr,
                          unsigned doserr,
                          unsigned _far *hdr )
{
    _hardresume( _HARDERR_ABORT);
}
