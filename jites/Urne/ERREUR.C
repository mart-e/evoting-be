
/*******************************************************************/
/* DEBUT DU MODULE ERREUR.C                                        */
/*******************************************************************/

#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <bios.h>
#include <time.h>
#include "defs.h"
#include "lcd.h"
#include "erreur.h"

#define IGNORE 0
#define RETRY  1
#define ABORT  2
#define BREAK  3
/*******************************************************************/
/*******************************************************************/

void Boucle_Erreur()
{
  Battery_Off();
  while(1);
} /* Boucle_Erreur */

/*******************************************************************/

void Change_Disque(char *Message)
{

  while(1)
    {
      Lcd_Affiche(0,0, Message);
      Lcd_Affiche(1,0,Message_Point[14]);
      if(Kbhit(COLOR,0)) break;     
     }  
    
    Lcd_Affiche(0,0,Message_Point[15]);
    Lcd_Affiche(1,0,Message_Point[16]);

 return;
} /* Change disquette */

/***********************************************/

void Wait_Time(int sec)
{
    time_t debut, fin;

    time(&debut);
    time(&fin);
    while (difftime(fin, debut) < sec) time(&fin);

} /* Wait_Time */

/*******************************************************************/
/* START CONFIDENTIAL */

int error_win(int numerror)
{
 int retval;
 
 while(1)
   {
     switch(numerror)
        {
          case 0 :
                  
                   retval = RETRY;
                   Lcd_Affiche(0,0,Message_Point[17]);
                   Lcd_Affiche(1,0,Message_Point[14]);
                   Getch(COLOR);
                   Lcd_Affiche(0,0,Message_Point[15]);
                   Lcd_Affiche(1,0,Message_Point[16]);
                   break;
          case 2 :
                   retval = RETRY;
                   Lcd_Affiche(0,0,Message_Point[18]);
                   Lcd_Affiche(1,0,Message_Point[14]);
                   Getch(COLOR);
                   Lcd_Affiche(0,0,Message_Point[15]);
                   Lcd_Affiche(1,0,Message_Point[16]);
                   retval = RETRY;
                   break;
          default: 
                   retval = BREAK;
        }
      return(retval);
   } 
}

/****************************************************************************/

int handler(unsigned errval,unsigned di,unsigned far *devhdr)
{
 int errorno;
 
 if(errval < 0)
         _hardretn(ABORT);
 errorno = di & 0x00FF;
 _hardresume(error_win(errorno));
 return BREAK;
}  
/****************************************************************************/
/* END CONFIDENTIAL */
