/************************************************************/
/*  Copyright Alcatel Bell Telephone 1991                   */
/*                                                          */
/*                                                          */
/*  Filename            : C_LABEL.C                         */
/*                                                          */
/*  Author              : Jacky Cuylen.                     */
/*                                                          */
/*  Date of creation    : 13/05/1991                        */
/*                                                          */
/*  Functionality       : get disk label in var dsklbl      */
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
/*      AVAILABLE FUNCTIONS                                 */
/*                                                          */
/*      function_name           author      date            */
/*                                                          */
/*      ffirst.c                 J.C.        9/04/93        */
/*                                                          */
/************************************************************/

/************************************************************/
/*                                                          */
/*      INCLUDE FILES                                       */
/*                                                          */
/************************************************************/
#include <dos.h>
#include <errno.h>
#include "hlc.h"

c_label()
{
    int ret;
    struct find_t finfo;
    char volname[12];
    strcpy( finfo.name, "            ");

    ret = _dos_findfirst( "a:\*.*", (unsigned) _A_VOLID, &finfo);
        
    strncpy( volname, finfo.name, 8);
    if( finfo.name[8] == '.')
        strncpy( &volname[8], &finfo.name[9], 3);
    else
        volname[8] = 0x00;
      
    volname[11] = 0x00;
        
    if( ( ret = prowtc( "dsklbl", 0, volname, 0)) != 0) 
        promsgd( "Could not write the shared data in dsklbl.");
    return( 0);
}

