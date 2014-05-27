#ifndef MCR_INCLUDED
#define MCR_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/* START CONFIDENTIAL */  

/* MCR last status (cleared each time McrCheckState() is called)*/
#define McrTIMEOUT           (0x0001)   /* Timeout during Read or Write*/
#define McrREAD_BUF_OVERFLOW (0x0002)   /* Rx data buffer full*/

int McrInit(int ioaddr, int irq, uchar *rxbuffer, int rxsize,
            uchar *txbuffer, int txsize);
void McrBg(void);
int McrRx(void);
int McrTx(uchar c);
int McrCheckState(void);
void McrExit(void);

#ifdef __cplusplus
}
#endif

#endif
/*----------------------------------------------------------------------------*/
/* Global definitions*/
/*----------------------------------------------------------------------------*/
typedef struct ui                       /* The irq handler's data*/
{
    int hirq;                           /* The hardware irq nr (-1 = not init.)*/
    int sirq;                           /* Corresponding soft irq number*/
    void (interrupt far *old_irq_vector)(); /* Old irq vector*/
    int old_pic_enabled;                /* 1 if was already enabled, 0 if not*/
} UI;

/*----------------------------------------------------------------------------*/
/* External functions*/
/*----------------------------------------------------------------------------*/
int ui_init(int hirq, void (interrupt far *irq_handler)(), UI *ui);
void ui_exit(UI *ui);
void ui_eoi(UI *ui);

/*----------------------------------------------------------------------*/
typedef struct
  {
    uchar far *next_write; /* ptr to the next element to write       */
    uchar far *next_read;  /* ptr to the next element to read        */
    uchar far *start;      /* ptr to the begin of the queue          */
    uchar far *end;        /* ptr to the last element of the queue   */
    uint element_count;     /* amount of elements in the queue        */
    uint max_elements;      /* max elements possible in the queue     */
  } UQMC;

int  uqmc_init( uint count, uchar far *ptr, UQMC *uqmc);
int  uqmc_read( UQMC *uqmc);
int  uqmc_read_ahead( UQMC *uqmc);
int  uqmc_write( uchar c, UQMC *uqmc);
uint uqmc_check_used( UQMC *uqmc);
uint uqmc_check_free( UQMC *uqmc);
void uqmc_flush( UQMC *uqmc);
void uqmc_exit( UQMC *uqmc);

/***************************************************************************
*
* Functions using BIOS Timer TICK
*
* KC, Apr-92: Creation
*
**********/
#define UTT_TICK_IN_A_DAY  (1573040L)

ulong utt_get(void);
ulong utt_elapsed( ulong reference_tick);
/* END CONFIDENTIAL */  
