
#include <gxlib.h>
#include <pcxlib.h>
#define    BUFMAX       5000                 /* Kernel buffer size         */

GXLIB lib;
GXHEADER vhptr;
PCXHEADER pcxhdr;

char buffer[BUFMAX];

Initpcx()
{
  short      pcxtype;
  
  gxSetBuffer(buffer, BUFMAX);
  gxInit();
  pcxtype = 7; /* on force le 640.480.256 */
  gxSetDisplay(pcxtype); 

}
/***************************************************************************/
