#include <conio.h>
#include <stdio.h>
#include <bios.h>
#include "hlc.h"

c_tstdsk( argc, argv) /* Main loop of program */
    int argc;
    char *argv[];
{
    long exists,ret;
    unsigned status = 0;
    struct diskinfo_t disk_info;

    disk_info.drive    = atoi( argv[1]);
    disk_info.head     = 0;
    disk_info.track    = 1;
    disk_info.sector   = 1;
    disk_info.nsectors = 8;

    status = _bios_disk( _DISK_RESET,  &disk_info);
    status = _bios_disk( _DISK_VERIFY, &disk_info);
    status = 0;
    status = _bios_disk( _DISK_VERIFY, &disk_info);
    if ( status == 0x8000 )
    {
/*      printf( "Status disk : %x : No disk inserted\n", status);       */
        exists = 0L;
        if( ( ret = prowti( "fileok", 0, exists, 0)) != 0)
            promsgd( "Could not write the shared data in fileok.");
    }
    else
    {
/*      if( status == 0x200 )                                           */
/*          printf( "Status disk : %x : Disk not formatted\n", status); */

        status = _bios_disk( _DISK_WRITE, &disk_info);
        if( status == 0x0300) /* Diskette is write protected */
            exists = 2L;
        else
            exists = 1L;
        if( ( ret = prowti( "fileok", 0, exists, 0)) != 0)
            promsgd( "Could not write the shared data in fileok.");
    }
    status = _bios_disk( _DISK_RESET, &disk_info);
    return( 0);
}
