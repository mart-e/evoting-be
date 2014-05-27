/* START CONFIDENTIAL  */
/****************************************************************/
/*								*/
/*	Communication package		       Definitions	*/
/*	=====================		       -----------	*/
/*								*/
/*								*/
/*	E. Franssen	Devlonics Terminals	04-Sep-1990	*/
/*								*/
/****************************************************************/

#ifndef uchar
   #define uchar unsigned char
   #define uint  unsigned int
   #define ulong unsigned long
#endif

/*----------------------------------------------------------------------*/
/*			COMMUNICATION structure 			*/
/*----------------------------------------------------------------------*/
typedef struct
  {
    /*------------------------------------------------------------------*/
    uchar far *r_begin; 	/* ptr to buffer receive begin		*/
    uchar far *r_end;		/* ptr to buffer receive end		*/
    uchar far *r_put;		/* ptr to put next byte received	*/
    uchar far *r_get;		/* ptr to get next byte received	*/
    /*------------------------------------------------------------------*/
    uchar far *t_begin; 	/* ptr to buffer transmit begin 	*/
    uchar far *t_end;		/* ptr to buffer transmit end		*/
    uchar far *t_put;		/* ptr to put next transmit byte	*/
    uchar far *t_get;		/* ptr to get next transmit byte	*/
    /*------------------------------------------------------------------*/
    uint  r_count;		/* count of unread bytes received	*/
    uint  t_count;		/* count of untransmitted bytes 	*/
    uint  r_error;		/* error count of received bytes	*/
    uchar l_ctrl;		/* 8250 line control			*/
    uchar l_stat;		/* 8250 line status			*/
    uchar m_ctrl;		/* 8250 modem control			*/
    uchar m_stat;		/* 8250 modem status			*/
    uint  i8250;		/* 8250 I/O base address		*/
    int unget;			/* value from unget_8250()		*/
    uchar irq;			/* 8250 interrupt level 		*/
    uchar handshake;		/* buffer hardware handshake		*/
    uchar f_tstop;		/* transmit stop flow			*/
    uchar f_tstart;		/* transmit restart flow		*/
    uchar f_rstop;		/* receive stop flow			*/
    uchar f_rstart;		/* receive restart flow 		*/
    uchar f_status;		/* flow soft status - b0: rx, b1: tx	*/
    uchar t_idle;		/* 1 = idle, 0 = Tx-ing under interrupt */
    /*------------------------------------------------------------------*/
    void (interrupt far *oldhandler)();
    /*void far *oldhandler; */	    /* Old interrupt handler		    */
    uchar old_ier;		/* Old registers, saved in com_open(),	*/
    uchar old_l_ctrl;		/* and restored in com_close()		*/
    uchar old_m_ctrl;		/*	  Old values			*/
    uchar old_pic_state;	/*	   "    "			*/
    uint  old_baudrate; 	/*	   "    "			*/
  } COM;

/*----------------------------------------------------------------------*/
/*		Definitions used in communication driver		*/
/*----------------------------------------------------------------------*/
enum COM_PORT
  {					/* Com port definitions 	*/
    COM_1 = 0,
    COM_2,
    COM_3,
    COM_4,
    COM_5,
    COM_6,
    COM_7,
    COM_8,
    COM_PLAST
  };

enum COM_BAUDRATE
  {					/* Baudrate definitions 	*/
    COM_B50 = 0,
    COM_B75,
    COM_B110,
    COM_B150,
    COM_B300,
    COM_B600,
    COM_B1200,
    COM_B2400,
    COM_B4800,
    COM_B7200,
    COM_B9600,
    COM_B11520,
    COM_B12800,
    COM_B14400,
    COM_B19200,
    COM_B38400,
    COM_B57600,
    COM_B115200,
    COM_BLAST
  };

enum COM_FORMAT
  {					/* Format definitions		*/
    COM_F7BNP1S = 0,
    COM_F7BNP2S,
    COM_F7BEP1S,
    COM_F7BEP2S,
    COM_F7BOP1S,
    COM_F7BOP2S,
    COM_F8BNP1S,
    COM_F8BNP2S,
    COM_F8BEP1S,
    COM_F8BOP1S,
    COM_FLAST
  };

#define COM_CURRENT	(-1)		/* Current value for dtr, rts, ...  */
#define COM_RESTORE	(-2)		/*    "      "	 ,see com_close()   */

#define COM_CTS 	0x10
#define COM_DSR 	0x20
#define COM_RI		0x40
#define COM_DCD 	0x80
#define COM_RTS 	0x02

/*----------------------------------------------------------------------*/
/*				Macro's                                 */
/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/
/*		Functions declarations for strong type checking		*/
/*----------------------------------------------------------------------*/
extern uint com_define(uint port,uint irq,uint addr,uchar *rxbuffer,
		       uint rxsize,uchar *txbuffer,uint txsize,COM *com);
extern uint com_open(int baudrate,int format,int dtr,int rts,COM *com);
extern uint com_close(int baudrate,int format,int dtr,int rts,COM *com);
extern uint com_undefine(COM *com);
extern int  com_rx(COM *com);
extern int  com_tx(uchar c,COM *com);
extern uint com_rx_available(COM *com);
extern uint com_tx_free(COM *com);
extern uint com_tx_busy(COM *com);
extern uint com_baudrate(int baudrate,COM *com);
extern uint com_format(int format,COM *com);
extern uint com_dtr(int dtr,COM *com);
extern uint com_rts(int rts,COM *com);
extern uint com_handshake(uint mode,COM *com);
/*extern void com_flowcontrol(uint mode,COM *com);*/
extern uint com_status(COM *com);
extern uint com_check(uint i8250);

/* END CONFIDENTIAL */






              
