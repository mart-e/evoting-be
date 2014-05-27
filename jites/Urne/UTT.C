/* START CONFIDENTIAL */

/***************************************************************************
*
* Functions using BIOS Timer TICK
*
* KC, Apr-92: Creation
*
**********/

#include <dos.h>
#include "utt.h"

/***************************************************************************
*
* Get current bios timer tick
*
* Return: bios timer tick value
*
*/
#pragma  optimize( "egl", off)  /* disable global optimizations */
ulong utt_get()
  {
    ulong tick;
    ulong _far *ptick;

      /* Point to timer tick in low memory */
    FP_SEG( ptick) = 0x40;
    FP_OFF( ptick) = 0x6C;

      /* Get timer tick count,
         disable interrupts to prevent update when we read it */
    _asm  pushf
    _asm  cli
    tick = *ptick;
    _asm  popf

    return( tick);
  }
#pragma  optimize( "", on)  /* re-enable optimizations */

/***************************************************************************
*
* Get elapsed ticks since reference tick
*
* Return: elapsed bios ticks
*
* Remark: This functions should only be used for low resolution timings.
*         Only 1 day-change is supported
*         The BIOS timer tick resolution is 1/18.2 seconds
*
*/
ulong utt_elapsed( ulong reference_tick)
  {
    ulong tick;

    tick = utt_get();

      /* Check if timer-tick overflow, if so add a day */
    if ( tick < reference_tick)
      tick += UTT_TICK_IN_A_DAY;
        
    return( tick - reference_tick);
  }
/* END CONFIDENTIAL */
