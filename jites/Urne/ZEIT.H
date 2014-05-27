/* START CONFIDENTIAL */

#ifndef MCR_INCLUDED
#define MCR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* MCR last status (cleared each time McrCheckState() is called)*/
#define McrTIMEOUT           (0x0001)   /* Timeout during Read or Write*/
#define McrREAD_BUF_OVERFLOW (0x0002)   /* Rx data buffer full*/

int McrInit(int ioaddr, int irq, uchar *rxbuffer, int rxsize,
            uchar *txbuffer, int txsize);
void McrBg(void);
int Mcr_End(void);
int Mcr_Flush(void);
int McrRx(void);
int Mcr_Chk(void);

int McrTx(uchar c);
int McrCheckState(void);
void McrExit(void);

int Fin_Rx(char *Buffer);
int Tcr_Rx(char *Buffer);
int Tcr_End(void);

#ifdef __cplusplus
}
#endif

#endif
/* END CONFIDENTIAL */
