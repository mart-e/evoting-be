#include <bios.h>
#include <stdio.h>
#include "hlc.h"

#define LPT1 0

long int prts;
char reset[1];
FILE *printer;

c_prstat( argc, argv)
    int argc;
    char *argv[];
{
    prts = _bios_printer( _PRINTER_STATUS, LPT1, 'x');
    if( prts == 144)
        if( ( printer = fopen( "prn", "wb")) == NULL)
            prts = 128;
        else
        {
            reset[0] = 0x00;
            if( fwrite( &reset[0], 1, sizeof(reset), printer) < sizeof( reset))
                prts = 128;
            fclose( printer);
        }

    if( prowti( "stat", 0, prts, 0))
        promsgd( "Could not write the shared data 'stat'.");

    return(0);
}
