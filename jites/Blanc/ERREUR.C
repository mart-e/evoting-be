
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
#include "erreur.h"

#define IGNORE 0
#define RETRY  1
#define ABORT  2
#define BREAK  3
/*******************************************************************/

void Wait_Time(int sec)
{
    time_t debut, fin;

    time(&debut);
    time(&fin);
    while (difftime(fin, debut) < sec) time(&fin);

} /* Wait_Time */

/*******************************************************************/
/* START CONFIDENTIAL */


/* << 10.07 */
/***************************  Lightpen Divide Error *****************************/
void interrupt Divide_Error_Handler()
{
	/* Blank Screen */
	setfillstyle(SOLID_FILL, WHITE);
	bar(0,0,639,479);
	
	/* Msg */
	setcolor(BLACK);
	settextstyle(SMALL_FONT, HORIZ_DIR,10);
	
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	outtextxy(320, 180, Message_point[2]);
	outtextxy(320, 181, Message_point[2]);

	/* "Prévenez le président du bureau." */
	settextstyle(SMALL_FONT, HORIZ_DIR, 7);
	outtextxy(320,400, Message_point[44]);
	outtextxy(320,401, Message_point[44]);

	/* Keep the card + bips */
	fraude(RG,1);
	while (klavier) point();
	status = Ejectons();
	if (status != 0)
	{
		msg_error = status;
		ptfunct = fin;
		return(0);
	}
	
	/* Don't exit */
	ptfunct = recom;
	return RETRY;
}
/* 10.07 >> */

int error_win(int numerror)
{
 int retval;
 
 while(1)
   {
     switch(numerror)
        {
          case 0 :
                  
                   retval = RETRY;
                   break;
          case 2 :
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
         hardretn(ABORT);
 errorno = di & 0x00FF;
 hardresume(error_win(errorno));

 return BREAK;
}  
/****************************************************************************/
/* END CONFIDENTIAL */
