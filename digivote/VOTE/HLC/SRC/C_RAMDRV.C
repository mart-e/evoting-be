/************************************************************/
/*                                                          */
/*  Filename            : C_RAMDRV.C                        */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 09/03/20000                       */
/*                                                          */
/*  Functionality       : get ramdrive-letter in var        */
/*                        ramdrive                          */
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

#include "hlc.h"

c_ramdrv()
{
    int ret;
    char RamDrive[4];

    if( getenv( "RAM") == NULL)
        sprintf( RamDrive, "C:\\");
    else
        sprintf( RamDrive, "%s:\\", getenv( "RAM"));

    if( ( ret = prowtc( "ramdrive", 0, RamDrive, 0)) != 0)
        promsgd( "Could not write the shared data in ramdrive.");
    return( 0);
}
