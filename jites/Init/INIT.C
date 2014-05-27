/*******************************************************************/
/* MODULE DE DECHARGE DE LA  FLASH EPROM                           */
/*******************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <dos.h>
#include "defs.h"
#include "dzdef.h"
#include "lcd.h"
/***********************************************/

void main()
{
  FILE *fps;
  char szbufs[33];

  Lcd_Init();

  fps = fopen("a:machvots.Tbl","rt");
  if(fps != NULL)
  {

    fgets(szbufs,32,fps);
    fclose(fps);

    /* Init Code Language*/
    switch(szbufs[15])
    {
      case '0': /* Dutch only */
      case '2': /* Dutch (French) */
      case '4': /* Dutch <> French */
        Lcd_Affiche(0,0, " PROGRAMMALADING ");
        Lcd_Affiche(1,0, "  EVEN WACHTEN   ");
        printf("  PROGRAMMALADING\tEVEN WACHTEN\n");
        break;

      case '1': /* French (Dutch) */
      case '7': /* French only */
        Lcd_Affiche(0,0, "CHARGEMENT PROGRAMME");
        Lcd_Affiche(1,1, "VEUILLEZ PATIENTER");
        printf("CHARGEMENT PROGRAMME\tVEUILLEZ PATIENTER\n");
        break;

      case '3': /* German only */
        Lcd_Affiche(0,0, "  PROGRAMM STARTET  ");
        Lcd_Affiche(1,1, "  GEDULDEN SICH SIE ");
        printf("PROGRAMM STARTET\tGEDULDEN SICH SIE\n");
        break;

      case '5': /* French <> German */
        Lcd_Affiche(0,0, "CHARGEMENT PROGRAMME");
        Lcd_Affiche(1,1, "  PROGRAMM STARTET  ");
        printf("CHARGEMENT PROGRAMME\tPROGRAMM STARTET\n");
        break;

     case '6': /* French <> Dutch */
        Lcd_Affiche(0,0, "CHARGEMENT PROGRAMME");
        Lcd_Affiche(1,1, " PROGRAMMALADING ");
        printf("CHARGEMENT PROGRAMME\tPROGRAMMALADING\n");
        break;

      case '8': /* German <> French */
        Lcd_Affiche(0,0, "  PROGRAMM STARTET  ");
        Lcd_Affiche(1,1, "CHARGEMENT PROGRAMME");
        printf("PROGRAMM STARTET\tCHARGEMENT PROGRAMME\n");
        break;

      default:
        Lcd_Affiche(0,0, "CHARGEMENT PROGRAMME");
        Lcd_Affiche(1,1, "VEUILLEZ PATIENTER");
        printf("CHARGEMENT PROGRAMME\tVEUILLEZ PATIENTER\n");
    }
  }
  else
  {
	Lcd_Affiche(0,0, "CHARGEMENT PROGRAMME");
        Lcd_Affiche(1,1, "VEUILLEZ PATIENTER");
        printf("CHARGEMENT PROGRAMME\tVEUILLEZ PATIENTER\n");
  }

}

/*******************************************************************/
