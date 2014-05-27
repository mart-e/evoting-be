/* START CONFIDENTIAL */  
/****************************************************************************/
/*                                                                          */
/*   dZine utilities: hardware irq related functions                        */
/*                                                                          */
/*                                                                          */
/*   E. Franssen                n.v. dZine         Created: 01-Mar-1993     */
/*                                                                          */
/****************************************************************************/

//----------------------------------------------------------------------------
// Global definitions
//----------------------------------------------------------------------------
typedef struct ui                       // The irq handler's data
{
    int hirq;                           // The hardware irq nr (-1 = not init.)
    int sirq;                           // Corresponding soft irq number
    void (interrupt far *old_irq_vector)(); // Old irq vector
    int old_pic_enabled;                // 1 if was already enabled, 0 if not
} UI;

//----------------------------------------------------------------------------
// External functions
//----------------------------------------------------------------------------
int ui_init(int hirq, void (interrupt far *irq_handler)(), UI *ui);
void ui_exit(UI *ui);
void ui_eoi(UI *ui);
/* END CONFIDENTIAL */  
