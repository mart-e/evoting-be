/* START CONFIDENTIAL */

/****************************************************************************/
/*                                                                          */
/*   PC-VOTE: Test Card Reader main program                                 */
/*                                                                          */
/*                                                                          */
/*   J. AL-KHANSA                                                            */
/*                                                                          */
/****************************************************************************/
#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

#include "dzdef.h"
#include "zeit.h" 

#define  false     0
#define  true      1
#define  time_out  2
/***************************************************************************/
/***************************************************************************/

uchar RxBuffer[100],TxBuffer[100];
int compt;

int TcrInit()
{
return(McrInit(0x3E8,5,RxBuffer,sizeof(RxBuffer),
		       TxBuffer,sizeof(TxBuffer)));
}
/***************************************************************************/
/***************************************************************************/
int Tcr_Snd(char *Buff_Commande)
{

int c;
 while((c = McrRx()) != -1);
 while (*Buff_Commande)
	  McrTx(*Buff_Commande++);
return(true);
}
/***************************************************************************/

int Tcr_Rx(char *Buff_Commande)
{
char *p;
int c;
unsigned long far *timer = ( unsigned long * )0x46C;
unsigned long temptimes;

int k = 0;   
char ok; 
   
   ok = '1';
   p = Buff_Commande;
   
   if(Mcr_End() == -1) return(1);  /* Il y'a pas d'elements dans la queue */

   temptimes = *timer;         /* Valeur du temps */
   
   while ((*timer - temptimes <= (unsigned long)45) && (ok != 0x0D) && (k < 75) )
      {
	while (((c = McrRx()) != -1) && (k < 75) && (ok != 0x0D))
	   {
	      ok = c ;
	      *p++ = (c == 0x0D) ? 0x00 : c ;
	       k++;
	   }
       }
if(ok != 0x0D)
  {
    *p = 0x00;
    while ((c = McrRx()) != -1);
  }
if(k <= 2) 
    return(1);
else 
    return(0);
}
/***************************************************************************/

int Fin_Rx(char *Buff_Commande)
{
char *p;
int c;
unsigned long far *timer = ( unsigned long * )0x46C;
unsigned long temptimes;

int k = 0;   
char ok; 
   
   ok = '1';
   p = Buff_Commande;
   
   if(Mcr_End() == -1) return(1);  /* Il y'a pas d'elements dans la queue */
   
   temptimes = *timer;         /* Valeur du temps */
   
   while ((*timer - temptimes <= (unsigned long)45) && (ok != 0x0D) && (k < 75) )
      {
	while (((c = McrRx()) != -1) && (k < 75) && (ok != 0x0D))
	   {
	      ok = c ;
	      *p++ = (c == 0x0D) ? 0x00 : c ;
	       k++;
	   }
       }
if(ok != 0x0D)
  {
    *p = 0x00;
    while ((c = McrRx()) != -1);
  }
if(k <= 1) 
    return(1);
else 
    return(0);
}
/***************************************************************************/

/* END CONFIDENTIAL */
