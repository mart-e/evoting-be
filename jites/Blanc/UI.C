/* START CONFIDENTIAL */  
/****************************************************************************/
/*                                                                          */
/*   dZine utilities: hardware irq related functions                        */
/*                                                                          */
/*                                                                          */
/*   E. Franssen                n.v. dZine         Created: 01-Mar-1993     */
/*                                                                          */
/****************************************************************************/

#include <dos.h>
#include <string.h>
#include <conio.h>

/*--------------------------------------------------------------------------*/
/*                          Constant definitions*/
/*--------------------------------------------------------------------------*/
#define INTA00          (0x20)          /* 8259 port (irq 0..7)*/
#define INTA01          (0x21)
#define INTB00          (0xa0)          /* 2nd 8259  (irq 8..15)*/
#define INTB01          (0xa1)
#define EOI             (0x20)          /* End-of-Interrupt*/

#define IRQ0_BASE   8                   /* Convert hardware irq to soft irq*/
#define IRQ8_BASE   0x70

/*----------------------------------------------------------------------------*/
/* Local functions*/
/*----------------------------------------------------------------------------*/
static int ui_enable_pic(int hirq);
static void ui_disable_pic(int hirq);
static int ui_h2s_irq(int hirq);

/*----------------------------------------------------------------------------*/
/* Implementation Global functions*/
/*----------------------------------------------------------------------------*/

/***************************************************************************
*
* Initialize the hardware irq handler.
* This function will:
*       - set the irq vector to the given C irq_handler
*       - save the old pic state concerning the given interrupt
*         (to restore the bit in the pic on ui_exit())
*       - enable the pic
*
* hirq:         hardware irq number, [0..15]
* irq_handler:  the C function to call on interrupt (must be an
*               __interrupt function, see _dos_setvect() info from MSC help)
* ui:           ptr to the caller's UI structure
*
* Return:       0 if ok
*              -1 if invalid hardware irq number
*
*/
int ui_init(int hirq, void (interrupt far *irq_handler)(), UI *ui)
{
    int ret = -1;                       /* Suppose invalid irq number*/

    memset(ui, 0, sizeof(UI));
    ui->hirq = -1;                      /* Suppose init will fail*/
    if (hirq >= 0 && hirq <= 15)
    {
	ui->hirq = hirq;
	ui->sirq = ui_h2s_irq(hirq);    /* Get soft irq number*/
	disable();
	ui->old_irq_vector = getvect(ui->sirq); /* Save old vector*/
	setvect(ui->sirq, irq_handler);/* And set to caller's function*/
	enable();

	/* Enable pic to have real irqs, and remember if was already enabled...*/
	ui->old_pic_enabled = ui_enable_pic(hirq);
	ret = 0;                        /* Everything done now...*/
    }
    return(ret);
}

/***************************************************************************
*
* Exit the hardware irq handler.
* This function will:
*       - restore the irq vector to its previous value
*       - restore the pic (disable if was not enabled before)
*
* ui:           ptr to the caller's UI structure
*
*/
void ui_exit(UI *ui)
{
    if (ui->hirq != -1)
    {
	/* If initialized before*/
	disable();
	setvect(ui->hirq, ui->old_irq_vector); /* Restore old function*/
	enable();
	if (!ui->old_pic_enabled)
	    ui_disable_pic(ui->hirq);   /* Disable interrupt controller*/
	ui->hirq = -1;                  /* Remember exit done...*/
    }
}

#pragma check_stack(off)                /* Will be called from interrupt*/
/***************************************************************************
*
* Generate non specific end-of-interrupt.
* From that moment on, the PIC can cause a new interrupt.
*
*/
void ui_eoi(UI *ui)
{
    /* Generate non specific EOI to allow irq again*/
    if (ui->hirq >= 8)
	outp(INTB00, EOI);
    outp(INTA00, EOI);
}
#pragma check_stack()

/*----------------------------------------------------------------------------*/
/* Implementation Local functions*/
/*----------------------------------------------------------------------------*/

/***************************************************************************
*
* Enable Interrupt Controller (pic 8259)
*
* hirq:     Hardware interrupt number [0..15]
*
* Return:   1 if irq was already set before
*           0 if was not set
*
*/
static int ui_enable_pic(int hirq)
{
    int ret = 0;                        /* Suppose irq not programmed yet...*/
    int c, pic = (hirq < 8) ? INTA01 : INTB01;
    unsigned char bit;

    disable();              /* Disable IRQ*/
    c = inp(pic);                   /* Get current mask of PIC*/
    bit = 1 << (hirq % 8);          /* make correct bit value*/
    if ((c & bit) == 0)
	ret = 1;                    /* Irq was already set (thus bit == 0)*/
    else
    {
	c &= ~bit;                  /* Enable IRx by setting bit to 0 (mask)*/
	outp(pic, c);               /* Program the PIC now...*/
    }
    if (hirq >= 8)
    {                               /* If irq 8..15,*/
	c = inp(INTA01);            /* ensure Master Level 2 enabled*/
	c &= 0xfb;                  /* 2nd pic in cascade with 1st through irq 2*/
	outp(INTA01, c);
    }
    enable();

    return(ret);
}

/***************************************************************************
*
* Disable Interrupt Controller (pic 8259)
*
* irq:      Hardware interrupt number [0..15]
*
*/
static void ui_disable_pic(int hirq)
{
    int c, pic = (hirq < 8) ? INTA01 : INTB01;
    unsigned char bit;

    /* Valid hardware irq number...*/
    disable();                  /* Disable IRQ*/
    c = inp(pic);                       /* Get current mask of PIC*/
    bit = 1 << (hirq % 8);              /* make correct bit value*/
    c |= bit;                           /* Disable IRx by setting bit to 1*/
    outp(pic, c);                       /* Program the PIC now...*/
    enable();
}

/***************************************************************************
*
* Convert hardware irq number to software irq.
* This conversion is necessary to know the soft irq number for setting
* the Irq vector (Microsoft _dos_setvect() function).
*
* hirq:     hardware irq number, [0..15]
*
* Return:   software irq: 0..7 becomes  0x08..0x0f,
*                         8..15 becomes 0x70..0x77
*           -1  if invalid hardware irq number
*
*/
static int ui_h2s_irq(int hirq)
{
    if (hirq >= 0 && hirq <= 15)
	return(hirq + ((hirq < 8) ? IRQ0_BASE : IRQ8_BASE));
    else
	return(-1);
}
/* END CONFIDENTIAL */  
