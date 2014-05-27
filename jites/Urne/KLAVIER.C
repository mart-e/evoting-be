#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <dos.h>
#include "defs.h"
#include "klavier.h"
char COLOR;
/* << 10.03b */
extern void Maj_IvBit(char);
/* 10.03b >> */
/* << 10.03d */
int   SECU;
long  SECU_TIMER;
/* 10.03d >> */
/*****************************************************************************/
/*****************************************************************************/
int Getch(char color)
{
 char c;

 while((c = Kbhit(color,2000)) > ' ');    /* tant qu'on appuye pas sur une touche */
 while((c = Kbhit(color,2000)) <= ' ');    /* tant qu'on appuye pas sur une touche */
/* << 10.03b */
	Maj_IvBit(0);
/* 10.03b >> */
 return(c);
}
/*****************************************************************************/
/*****************************************************************************/

int Kbhit(char color,int loop)
{
 char c;
 char tc,tc1;
 int i;

 outp(0x3E0,color | 0x01);
 for (i = 0; i < loop ; i--,i++,i++);
 tc = inp(0x3E0) & 0x0F;
 tc = 0x0F - tc;
 if(tc != 0x00)
      {
        tc1 = Trait_Tc(tc,0x00,color);
        return(tc1);
      }

  outp(0x3E0,color | 0x02);
  for (i = 0; i < loop ; i--,i++,i++);
  tc = inp(0x3E0) & 0x0F;
  tc = 0x0F - tc;
  if(tc != 0x00)
      {
        tc1 = Trait_Tc(tc,0x01,color);
        return(tc1);
      }

 outp(0x3E0,color | 0x04);
 for (i = 0; i < loop ; i--,i++,i++);
 tc = inp(0x3E0) & 0x0F;
 tc = 0x0F - tc;
 if(tc != 0x00)
     {
        tc1 = Trait_Tc(tc,0x02,color);
        return(tc1);
     }

  return(0);
}

/****************************************************************************/
/****************************************************************************/
int Trait_Tc(char tc,char ordre,int color)
{
 char  ctc ;
 switch(tc)
  {
    case 0x01 : ctc = '1' + ordre;
               break;
    case 0x02 : ctc = '4' + ordre;
               break;
    case 0x04 : ctc = '7' + ordre;
               break;
    case 0x08 : switch(ordre)
                 {
                   case 0x00 : ctc ='O';
                               break;
                   case 0x01 : ctc ='0';
                               break;
                   case 0x02 : ctc = 'N';
                               break;
                 }
                 break;
    default  : ctc = 'x';
  }

outp(0x3E0,color);
return(ctc);
}
/****************************************************************************/
/****************************************************************************/
/* << 10.03d */
/* << 10.03f */
/*void Ctrl_Relais(char *pzMessage, int iLine, int iCpt)*/
void Ctrl_Relais(char *pzMessage, char *pzMsg, int iLine, int iCpt)
/* 10.03f >> */
/*
    Checks whether it is under batteries
*/
{
   unsigned long far *timer = (unsigned long *) 0x46C;
   char strtmp[21], c;
   int tmp;

   c = inp(0x3E0);
   if((c & 0x20) == 0x20)
   {
      if(SECU == 0)
         {
           SECU_TIMER = *timer;
           SECU = 1;
         }
      else if((*timer - SECU_TIMER) <= 1200)
      {
          tmp = 15 - (( *timer - SECU_TIMER ) /  19);
          if (tmp <= 0)
          {
            tmp = 0;
            outp(0x3E0,0x00);
          }

           if(iCpt > 0)
            sprintf(strtmp, pzMessage, iCpt, tmp);
           else
            sprintf(strtmp, pzMessage, tmp);

           Lcd_Affiche(iLine, 0, strtmp);
           putch(0x07);
      }
   }
   else
        Lcd_Affiche(iLine, 0, pzMsg);


}

/****************************************************************************/
/****************************************************************************/
/* << 10.03f */
/*int Getchar(char color, char *pzMessage, int iLine, int iCpt)*/
int Getchar(char color, char *pzMessage, char *pzMsg, int iLine, int iCpt)
/* 10.03f >> */
{
 char c;

 while((c = Kbhit(color,2000)) > ' ')    /* tant qu'on appuye pas sur une touche */
 {
     /* << 10.03f */
     /*Ctrl_Relais(pzMessage,iLine, iCpt);*/
     Ctrl_Relais(pzMessage, pzMsg, iLine, iCpt);
     /* 10.03f >> */
 }
 while((c = Kbhit(color,2000)) <= ' ')    /* tant qu'on appuye pas sur une touche */
 {
     /* << 10.03f */
     /*Ctrl_Relais(pzMessage,iLine, iCpt);*/
     Ctrl_Relais(pzMessage, pzMsg, iLine, iCpt);
     /* 10.03f >> */
 }

 return(c);
}

/****************************************************************************/
/****************************************************************************/
/* 10.03d >> */