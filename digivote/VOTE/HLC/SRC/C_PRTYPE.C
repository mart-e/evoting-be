#include <bios.h>
#include <stdio.h>
#include "hlc.h"

#define LPT1         0

#define TIME_OUT     0x01
#define IO_ERROR     0x08
#define SELECTED     0x10
#define OUT_OF_PAPER 0x20
#define ACKNOWLEDGE  0x40
#define NOT_BUSY     0x80

long int prts;

c_prtype( )
{
    switch( _bios_printer( _PRINTER_INIT, LPT1, 'x'))
    {
        case SELECTED+NOT_BUSY: /* HP LaserJet 1100 */
            prts = 1;
            break;

        case SELECTED         : /* Bull CompuPrint  */
            prts = 0;
            break;

        default               :
            prts = -1;
            break;
    }
    if( prowti( "printer", 0, prts, 0))
        promsgd( "Could not write the shared data 'printer'.");

    return(0);
}
