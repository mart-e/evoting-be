/************************************************************/
/*                                                          */
/*  Filename            : C_GETENV.C                        */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 09/03/20000                       */
/*                                                          */
/*  Functionality       : get value of an ENVIRONMENT       */
/*                        variable                          */
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

#include "logging.h"
#include "hlc.h"

c_getenv( argc, argv)
    int argc;
    char *argv[];
{
    char valEnv[80];

    if( getenv( argv[1]) == NULL)
	strcpy( valEnv, "?");
    else
	strcpy( valEnv, getenv( argv[1]));

    if( prowtc( argv[2], 0, valEnv, 0)) {
		strcpy( gcModuleName, "c_getenv");
		gcFunctionName[0] = '\0';
		sprintf( gcMessage, "Could not write the shared data in '%s'.", argv[2]);
		LogError();
	promsgd( gcMessage);
    }
    return( 0);
}
