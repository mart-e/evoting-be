/* START CONFIDENTIAL */

/****************************************************************************/
/*                                                                          */
/*   PC-VOTE: Magnetic Card Reader functions                                */
/*                                                                          */
/*                                                                          */
/*   E. Franssen                n.v. dZine         Created: 08-Feb-1993     */
/*                                                                          */
/****************************************************************************/

#ifndef MCR_INCLUDED
#define MCR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "dZdef.h"

// MCR last status (cleared each time McrCheckState() is called)
#define McrTIMEOUT           (0x0001)   // Timeout during Read or Write
#define McrREAD_BUF_OVERFLOW (0x0002)   // Rx data buffer full

int McrInit(int ioaddr, int irq, uchar *rxbuffer, int rxsize,
	    uchar *txbuffer, int txsize);
void McrBg(void);
int Mcr_End(void);
int Mcr_Flush(void);
int McrRx(void);
int McrTx(uchar c);
int McrCheckState(void);
void McrExit(void);

#ifdef __cplusplus
}
#endif

#endif
/* END CONFIDENTIAL */
