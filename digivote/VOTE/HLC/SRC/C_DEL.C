/************************************************************/
/*                                                          */
/*  Filename            : C_DEL.C                           */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 13/03/2000                        */
/*                                                          */
/*  Functionality       : Verwijderen van een bestand       */
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

c_del( argc, argv)
    int argc;
    char *argv[];
{
    int ret;

    fileok = !access( argv[1], 00);
    if( fileok == 1)
        fileok = !remove( argv[1]);
    if( ( ret = prowti( "fileok", 0, fileok, 0)) != 0)
        promsgd( "Could not write the shared data in fileok.");
}
