//==============================================================================
/*

  File       : KEYBOARD.CPP
  Creation   : 14/04/93

  Project    : Bull DIGIVOTE
  Author     : B.S.S.

  Description: Program to detect keyboard's presence.

  Exit value : 0 keyboard present
               1 no keyboard

*/
//==============================================================================


//============================== I N C L U D E =================================

#include <dos.h>
#include <stdlib.h>


//==============================================================================
//================================= M A I N ====================================
//==============================================================================

int main()
{
   // bit 4 of Keyboard flag byte 2 at address 40:97 indicates
   // ACK Received (info taken from Helppc : Keyboard Flags)
   unsigned char far * keyb = (unsigned char *) MK_FP( 0x40, 0x97);

   if( * keyb & 16)
     return 0;          // ACK received -> keyboard present

   return 1;            // no ACK       -> no keyboard
}

//=== KEYBOARD.CPP =============================================================
