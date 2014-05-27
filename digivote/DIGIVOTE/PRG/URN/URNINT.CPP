/*H*/
//=============================================================================
/*

   File        : URNINT.CPP
   Creation    : 25/10/93

   Author      : B.S.S.

   Description : Interrupt handlers

*/
//=============================================================================
/*h*/
/*R*/
//=============================================================================
/*

   Revision History
   ################

   25/10/93: revision history starting date

*/
//=============================================================================
/*r*/
/*#*/

//======================== I N C L U D E   F I L E S ==========================

#include <conio.h>
#include <dos.h>

//=================== E X T E R N A L   V A R I A B L E S =====================

extern void interrupt( * Old_Keyboard_Handler)(...);

//====================== L O C A L   F U N C T I O N S ========================

static void Forget_Key( void);

/*{*/
//=============================================================================
//
// Function : Control_Break
//
// Purpose  : control break interrupt handler
//
// Return   : always 1 = continue
//
// Arg List : void
//
// Globals  : none
//
// Usage    : ctrlbrk( Control_Break);
//
// Checks   : none
//
// Comment  :
//
//-----------------------------------------------------------------------------
/*}*/

int Control_Break( void)
{
   return 1;
}

/*{*/
//=============================================================================
//
// Function : Forget_Key
//
// Purpose  : clears a key from the keyboards buffer
//
// Return   : static void
//
// Arg List : void
//
// Globals  : none
//
// Usage    : Forget_Key();
//
// Checks   :
//
// Comment  : This function has to be called from the keyboard interrupt
//            handler.
//            The information was mainly found in HelpTech, with little
//            adaptation. Sending 0xfa to the 0x60 keyboards port enables the
//            keyboard, causing it to clear its output buffer and last
//            typematic key and then responds with an ACK. The keyboard then
//            begins scanning.
//            Then we inform the 8259, that the keyboard interrupt is finished
//            and that he can proceed. (The software iret is not enough)
//
//-----------------------------------------------------------------------------
/*}*/

static void Forget_Key( void)
{
   int value = inp( 0x61);    // get the current value of keyboard control lines
   outp( 0x61, value | 0x80); // aknowledge the scan code (= bit 7)
   outp( 0x61, value);        // fetch the original control port value

   outp( 0x20, 0x20);    // send end of interrupt signal to the 8259
}

/*{*/
//=============================================================================
//
// Function : Keyboard_Handler
//
// Purpose  : keyboard interrupt handler, to filter the keys
//
// Return   : void interrupt  , interrupt function
//
// Arg List : no matter
//
// Globals  : none
//
// Usage    : setvect( 0x09, Keyboard_Handler);
//
// Checks   :
//
// Comment  : The function intercepts the keys pressed on the keyboard and
//            'forget' all the keys we don't want to process.
//            The codes used below are the keyboard make codes (found in
//            HelpPc) they are different from the scan codes. The 'pause' key
//            is different from the others because its last scan code is not
//            unique (we must trigger the E1 and then read all the remaining
//            pause codes) while trapping the last scan code for the other
//            keys is sufficient.
//
//-----------------------------------------------------------------------------
/*}*/

void interrupt Keyboard_Handler(...)
{
   switch( inp(0x60))
   {
   case 0x1d:        // left ctrl (or right E0 1D)
   case 0x37:        // PrtSc  (E0 2A E0 37, E0 37)
   case 0x38:        // left alt (or right E0 38)
   case 0x3a:        // caps lock
   case 0x45:        // num lock
   case 0x46:        // scroll lock
   case 0x54:        // sys req

        Forget_Key();
        break;

   case 0xE1:        // pause  (E1 1D 45 E1 9D C5)

        for( int i=5; i; i--)
        {
            delay(20);
            inp( 0x60);
        }

        Forget_Key();
        break;

   default:

        Old_Keyboard_Handler();
   }
}

//=== URNINT.CPP ==============================================================
