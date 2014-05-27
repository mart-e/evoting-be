/************************************************************/
/*                                                          */
/*  Filename            : C_TSTFN.C                         */
/*                                                          */
/*  Author              : Jacky Cuylen.                     */
/*                                                          */
/*  Date of creation    : 13/05/1991                        */
/*                                                          */
/*  Functionality       : TESTEN OF EEN FILE BESTAAT        */
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

#include "hlc.h"

/************************************************************/
/*                                                          */
/*  GLOBAL VARIABLES FOR THIS FILE ONLY                     */
/*                                                          */
/************************************************************/
long fileok;
FILE *streamin;

c_tstfn( argc, argv)
    int argc;
    char *argv[];
{
    int ret;

    fileok = !access( argv[1], 00);
    if( fileok == 0)
        if( ( streamin = fopen( argv[1], "rb")) != NULL)
        {
            fileok = 1;
            fclose( streamin);
        }
    if( ( ret = prowti( "fileok", 0, fileok, 0)) != 0)
        promsgd( "Could not write the shared data in fileok.");
}
