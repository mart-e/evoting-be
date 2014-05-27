/************************************************************/
/*                                                          */
/*  Filename            : C_ATTRIB.C                        */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 13/03/2000                        */
/*                                                          */
/*  Functionality       : Maak bestand Hidden               */
/*                               en/of Read-Only            */
/*                        Parameter 1 : bestandsnaam        */
/*                        Parameter 2 : te bekomen attrib   */
/*                           - 0 : Visible + Writable       */
/*                           - 1 : Read-Only                */
/*                           - 2 : Hidden                   */
/*                           - 3 : Read-Only + Hidden       */
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
/*  INCLUDE FILES                                           */
/*                                                          */
/************************************************************/
#include <stdio.h>
#include <dos.h>

#include "hlc.h"

/************************************************************/
/*                                                          */
/*  GLOBAL VARIABLES FOR THIS FILE ONLY                     */
/*                                                          */
/************************************************************/
long fileok;

c_attrib( argc, argv)
    int argc;
    char *argv[];
{
    int ret, param;
    unsigned attrib;
    struct find_t ffblk;

    param = atoi( argv[2]);
    if( !_dos_findfirst( argv[1], _A_NORMAL|_A_RDONLY|_A_HIDDEN, &ffblk))
    {
        attrib = ffblk.attrib;
        if( attrib | _A_HIDDEN)
        {
            if( param == 2 || param == 3)
                attrib |= _A_HIDDEN;
        }
        if( attrib & _A_HIDDEN)
        {
            if( param == 0 || param == 1)
                attrib &= ~_A_HIDDEN;
        }

        if( attrib | _A_RDONLY)
        {
            if( param == 1 || param == 3)
                attrib |= _A_RDONLY;
        }
        if( attrib & _A_RDONLY)
        {
            if( param == 0 || param == 2)
                attrib &= ~_A_RDONLY;
        }
        fileok = !_dos_setfileattr( argv[1], attrib);
    }
    else
        fileok = 0;

    if( ( ret = prowti( "fileok", 0, fileok, 0)) != 0)
        promsgd( "Could not write the shared data in fileok.");
}
