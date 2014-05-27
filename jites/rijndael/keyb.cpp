#include <conio.h>
#include <dos.h>
#include <stdio.h>

void interrupt( * Old_Keyboard_Handler)(...);
static void Forget_Key( void);

/*
<FUNCTION>
	<NAME>Control_Break</NAME>
	<TYPE>LOCAL</TYPE>
	<DESCRIPTION>
		control break interrupt handler
	</DESCRIPTION>
	<RETURN>
		<TYPE>int</TYPE>
		<DESCRIPTION>
			always 1 = continue
		</DESCRIPTION>
	</RETURN>
</FUNCTION>
*/
int Control_Break( void)
{
	return 1;
}

/*
<FUNCTION>
	<NAME>Forget_Key</NAME>
	<TYPE>LOCAL</TYPE>
	<DESCRIPTION>
		clears a key from the keyboards buffer
		Comment  : This function has to be called from the keyboard interrupt
		           handler.
		           The information was mainly found in HelpTech, with little
		           adaptation. Sending 0xfa to the 0x60 keyboards port enables the
		           keyboard, causing it to clear its output buffer and last
		           typematic key and then responds with an ACK. The keyboard then
		           begins scanning.
		           Then we inform the 8259, that the keyboard interrupt is finished
		           and that he can proceed. (The software iret is not enough)
	</DESCRIPTION>
	<RETURN>
		<TYPE>void</TYPE>
	</RETURN>
</FUNCTION>
*/
static void Forget_Key( void)
{
   int value = inp( 0x61);    // get the current value of keyboard control lines
   outp( 0x61, value | 0x80); // aknowledge the scan code (= bit 7)
   outp( 0x61, value);        // fetch the original control port value

   outp( 0x20, 0x20);    // send end of interrupt signal to the 8259
}

/*
<FUNCTION>
	<NAME>Keyboard_Handler</NAME>
	<TYPE>LOCAL</TYPE>
	<DESCRIPTION>
		keyboard interrupt handler, to filter the keys
		Comment  : The function intercepts the keys pressed on the keyboard and
		           'forget' all the keys we don't want to process.
		           The codes used below are the keyboard make codes (found in
		           HelpPc) they are different from the scan codes. The 'pause' key
		           is different from the others because its last scan code is not
		           unique (we must trigger the E1 and then read all the remaining
		           pause codes) while trapping the last scan code for the other
		           keys is sufficient.
	</DESCRIPTION>
	<PARAMETER>
		<NAME>...</NAME>
		<TYPE>...</TYPE>
		<IO>INPUT</IO>
		<DESCRIPTION>no matter</DESCRIPTION>
	</PARAMETER>
	<RETURN>
		<TYPE>void interrupt</TYPE>
		<DESCRIPTION>interrupt function</DESCRIPTION>
	</RETURN>
</FUNCTION>
*/
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

/*
<FUNCTION>
	<NAME>Control_Keyboard</NAME>
	<TYPE>GLOBAL</TYPE>
	<DESCRIPTION>
		Start the keyboard management
	</DESCRIPTION>
	<PARAMETER>
</FUNCTION>
*/
#ifdef __cplusplus
extern "C" {
#endif
void Control_Keyboard(void)
{
	ctrlbrk( Control_Break);         // Install control break handler
	Old_Keyboard_Handler = getvect( 0x09);
	setvect( 0x09, Keyboard_Handler);
}
#ifdef __cplusplus
}
#endif

/*
<FUNCTION>
	<NAME>Control_Keyboard_Restore</NAME>
	<TYPE>GLOBAL</TYPE>
	<DESCRIPTION>
		Restore the standard keyboard management
	</DESCRIPTION>
	<PARAMETER>
</FUNCTION>
*/
#ifdef __cplusplus
extern "C" {
#endif
void Control_Keyboard_Restore(void)
{
	setvect( 0x09, Old_Keyboard_Handler); // restore the default keyboard
}
#ifdef __cplusplus
}
#endif
