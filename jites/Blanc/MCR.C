/* START CONFIDENTIAL */  
/****************************************************************************/
/*                                                                          */
/*   PC-VOTE: Magnetic Card Reader functions                                */
/*                                                                          */
/*                                                                          */
/*   E. Franssen                n.v. dZine         Created: 08-Feb-1993     */
/*                                                                          */
/****************************************************************************/

/*** This source contains a driver for the magnetic card reader interface on
     the voting terminal, which is set up in a parallel way.
     It acts +/- in the same way as the com library, which means this driver
     does input/output buffering.
     For more information, see the hardware description from J. Favorel.
***/

#include <dos.h>
#include <conio.h>
#include <string.h>
/*--------------------------------------------------------------------------*/
/*                          Constant definitions                            */
/*--------------------------------------------------------------------------*/
enum tMcrMode                           /* The current mode for read/write*/
{
    McrDISABLED,                        /* No McrInit() yet...             */
    McrIDLE,                            /* Nothing (but always try to read)*/
    McrWRITE,                           /* Currently writing               */
    McrREAD                             /* Currently reading               */
};

#define McrTIMEOUT_VALUE  (10)          /* Rx/Tx Timeout in timer tick count*/

/*--------------------------------------------------------------------------*/
/*                          Local variables*/
/*--------------------------------------------------------------------------*/
static UI       uiHandler;              /* The hardware irq var's             */
static int      McrIO;                  /* The I/O address for the card reader*/
static UQMC     uqmTx;                  /* Tx queue handler                   */
static UQMC     uqmRx;                  /* Rx queue handler                   */
static uchar    CurrentTxChar;          /* Write mode, waiting response while Tx*/
static ulong    McrTimer;               /* Timer for max. Rx/Tx                 */
static enum tMcrMode McrMode = McrDISABLED; /* The current read/write mode      */
static int      McrState;               /* State for last operation             */

/*--------------------------------------------------------------------------*/
/*                          Local functions*/
/*--------------------------------------------------------------------------*/
static void interrupt far McrIrqHandler(void);

/*--------------------------------------------------------------------------*/
/*                          Implementation global functions*/
/*--------------------------------------------------------------------------*/

/***************************************************************************
*
* Init mcr handler.
* This function must be called at the begin of the program. Furthermore,
* before program exit, be sure to call mcr_exit(), or your PC may crash.
*
* ioaddr:   I/O address, 0x300 for the PC-VOTE board
* irq:      hardware irq number, 5 for the PC-VOTE board.
*
* Return:   0   ok
*          -1   invalid irq
*
*/
int McrInit(int ioaddr, int irq, uchar *rxbuffer, int rxsize,
	    uchar *txbuffer, int txsize)
{
    int c, ret = -1;

    McrMode = McrDISABLED;              /* Suppose won't succeed */
    if (uqmc_init(rxsize, rxbuffer, &uqmRx))
    {
	if (uqmc_init(txsize, txbuffer, &uqmTx))
	{
	    if (ui_init(irq, McrIrqHandler, &uiHandler) >= 0)
	    {
		McrIO = ioaddr;
		inp(McrIO);     /* Read char, clears interrupt flag */

		McrMode = McrIDLE;
		McrState = 0;
		ret = 0;
	    }
	}
    }
    return(ret);
}

/***************************************************************************
*
* MCR Bg function.
* This function must be called repeatedly from the main loop. It will
* start byte transmission to the card reader, and will handle timeout's.
*
*/
void McrBg()
{
    disable();
    /* If Rx or Tx busy, and timeout, re-set to idle mode */
    if ((McrMode == McrREAD || McrMode == McrWRITE) &&
	utt_elapsed(McrTimer) >= McrTIMEOUT_VALUE)
    {
	/* Only error for Write mode (for read mode, it simply means
	   that the mcr has no more data to transmit). */
	if (McrMode == McrWRITE)
	    McrState |= McrTIMEOUT;
	McrMode = McrIDLE;
    }
    if (McrMode == McrIDLE && uqmc_check_used(&uqmTx))
    {
	/* Idle state, and something available to Tx... */
	CurrentTxChar = (uchar) uqmc_read(&uqmTx);
	outp(McrIO, CurrentTxChar);     /* Tx it...*/
	McrTimer = utt_get();   /* Restart Tx timer*/
	McrMode = McrWRITE;
    }
    enable();
}

/***************************************************************************
*
* Read a byte from the card reader
*
* Return:   0..0xff     a character from the mcr
*          -1           nothing available
*
*/
int McrRx()
{
    int ret;

    disable();
    ret = uqmc_read(&uqmRx);
    enable();
    return(ret);
}

/***************************************************************************
*
* Send a character to the card reader
*
* c:        the character to send
*
* Return:   0..0xff     character written in output buffer
*          -1           if buffer full (ch NOT in output buffer)
*
*/
int McrTx(uchar c)
{
    int ret = -1;

    disable();
    if (uqmc_write((uchar) c, &uqmTx))
	ret = c;                        /* char. in output buffer */
    enable();
    return(ret);
}

/***************************************************************************
*
* Get current Mcr status. Note that the Mcr status is cleared upon return.
*
* Return:   0   no errors yet...
*           McrTIMEOUT | McrWRITE_BAD_ECHO | McrREAD_BUF_OVERFLOW
*
*/
int McrCheckState()
{
    int ret;

    disable();
    ret = McrState;
    McrState = 0;
    enable();
    return(ret);
}

/***************************************************************************
*
* Exit the mcr handler.
* This function MUST be called before program exit!
*
*/
void McrExit()
{
    uqmc_exit(&uqmRx);
    uqmc_exit(&uqmTx);
    ui_exit(&uiHandler);
    McrMode = McrDISABLED;
}

/*--------------------------------------------------------------------------*/  
/*                          Implementation local functions*/
/*--------------------------------------------------------------------------*/

/***************************************************************************
*
* This functions acts as the irq handler for the mcr, and thus performs
* the low level read/write functions.
*
*/
static void interrupt far McrIrqHandler()
{
    int c, i;

    c = inp(McrIO);                     /* Read char, clears interrupt flag */
    ui_eoi(&uiHandler);                 /* Generate EOI to allow irq again  */

    if (McrMode == McrWRITE)
    {
	/* Busy writing. Check if the Z80 response corresponds with the */
	/* transmitted byte.                                            */
	if ((uchar) c == CurrentTxChar)
	{
	    /* response ok, so tx next byte... */
	    i = uqmc_read(&uqmTx);
	    if (i != -1)
	    {
		CurrentTxChar = (uchar) i;
		outp(McrIO, i); /* and tx it... */
		McrTimer = utt_get();   /* Restart Tx timer*/
	    }
	    else
		McrMode = McrIDLE;      /* Nothing more to send...       */
	    c = -1;                     /* Remember character processed! */
	}
	else
	{
	    /* Wrong echo during write*/
	    /* This means that the mcr is trying to send data, so*/
	    /* switch to read mode... */
	    McrMode = McrIDLE;          /* Will read it just below */
	}
    }
    /* If char. not processed yet, try READ mode */
    if (c != -1 && McrMode == McrIDLE || McrMode == McrREAD)
    {
	if (McrMode == McrIDLE)
	{
	    /* We were in idle mode, this is first Rx character... */
	    McrMode = McrREAD;          /* Switch to read mode     */
	}
	if (uqmc_write((uchar) c, &uqmRx) == 0) /* Save the character */
	    McrState |= McrREAD_BUF_OVERFLOW;

	outp(McrIO, c);         /* and echo it         */
	McrTimer = utt_get();           /* Restart Rx timer... */
    }
}
/* END CONFIDENTIAL */  
