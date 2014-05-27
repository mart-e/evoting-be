/************************************************************/
/*                                                          */
/*  Filename            : C_DIR.C                           */
/*                                                          */
/*  Author              : Jurgen Rousseau                   */
/*                                                          */
/*  Date of creation    : 13/03/2000                        */
/*                                                          */
/*  Functionality       : Uitlijsten van dir, bestanden, .. */
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
#include <ctype.h>
#include <dos.h>
#include <errno.h>

#include "logging.h"
#include "hlc.h"

c_dir( argc, argv)
    int argc;
    char *argv[];
{
    FILE* dirStream = NULL;
    struct find_t fileinfo;
    int retcode;
    char dirEntry[20];

    strcpy( gcModuleName, "c_dir");
    gcFunctionName[0] = '\0';
    if( ( dirStream = fopen( argv[2], "wt")) == NULL) {
        sprintf( gcMessage, "Could not create outputfile %s !", argv[2]);
        LogError();
        promsgd( gcMessage);
        return( 0);
    }

    retcode = _dos_findfirst( argv[1],
                              _A_ARCH | _A_HIDDEN | _A_NORMAL | _A_RDONLY,
                              &fileinfo);
    while( !retcode) {
        sprintf( dirEntry, "%s\n", fileinfo.name);
        if( fwrite( dirEntry, 1, strlen(dirEntry), dirStream) != strlen(dirEntry)) {
            fclose( dirStream);
            sprintf( gcMessage, "Could not write to %s !", argv[2]);
            LogError();
            promsgd( gcMessage);
            return( 0);
        }
        retcode = _dos_findnext( &fileinfo);
    }
    fclose( dirStream);

    return( 0);
}

c_fsize( argc, argv)
    int argc;
    char *argv[];
{
    struct find_t fileinfo;
    long filesize = -1;

    strcpy( gcModuleName, "c_fsize");
    if( !_dos_findfirst( argv[1],
                         _A_ARCH | _A_HIDDEN | _A_NORMAL | _A_RDONLY,
                         &fileinfo))
    {
        filesize = fileinfo.size;
    }
    if ( prowti( "filesize", 0, filesize, 0) != 0)
    {
        promsgd( "Could not write the shared data 'filesize'.");
    }

    return( 0);
}

c_dfree( argc, argv)
    int argc;
    char *argv[];
{
    struct diskfree_t diskinfo;
    long diskfree = -1;
    int drive;

    strcpy( gcModuleName, "c_dfree");
    drive = toupper( argv[1][0]) - 'A' + 1;
    _dos_getdiskfree( drive, &diskinfo);
    diskfree = (long) diskinfo.avail_clusters      *
               (long) diskinfo.sectors_per_cluster *
               (long) diskinfo.bytes_per_sector;

    if ( prowti( "diskfree", 0, diskfree, 0) != 0)
    {
        promsgd( "Could not write the shared data 'diskfree'.");
    }

    return( 0);
}
