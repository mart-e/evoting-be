/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE LCD.C                                          */
/*******************************************************************/
#include <conio.h>
#include <stdio.h>
#include "defs.h"

#include "lcd.h"

/*******************************************************************/

void Lcd_Attente()
/*
    Boucle de temporisation 
    Parametre: sans
    Rend void  
*/
{
  int i;

  for (i = 0; i < _LCD_TEMPS_ATTENTE; i++);
} /* Lcd_Attente */

/*******************************************************************/

void Lcd_Caractere(unsigned char Controle, unsigned char Donnee)
/*
   Envoie une commande au LCD
   Parametre: commande a executer
              donnee propre a la commande
   Rend void           
*/
{
  outp(_LCD_REG_DONNEE, Donnee);
  Lcd_Attente();
  outp(_LCD_REG_CONTROLE, Controle & 0x03);
  Lcd_Attente();
  outp(_LCD_REG_CONTROLE, (Controle & 0xFE) & 0x03);
  Lcd_Attente();
  outp(_LCD_REG_CONTROLE, Controle & 0x03);
  Lcd_Attente();
  outp(_LCD_REG_CONTROLE, 0x03);
  Lcd_Attente();
} /* Lcd_Caractere */

/*******************************************************************/

void Lcd_Affiche(int Ligne, int Colonne, char *Chaine)
/*
    Affiche une chaine de caracteres sur le LCD, efface le reste de 
      la ligne
    Parametre: numero de ligne 0 ou 1
               numero de colonne  0 a 20
               chaine de caracteres a afficher
    Rend void
*/                                                        
{
   char *strtmp; 

   strtmp = Chaine;

   Lcd_Caractere(3, (char) (0x80 | (Ligne * 0x40 + Colonne)));
   for (;*Chaine;)
     { ++Colonne;
       Lcd_Caractere(1, *Chaine++);
     }
   for (;++Colonne < 21;) Lcd_Caractere(1, ' ');
   

} /* Lcd_Affiche */

/*******************************************************************/

void Lcd_Init()
/*
    Initialise le LCD
    Parametre: sans
    Rend void
*/
{
   Lcd_Caractere(3, 0x38);
   Lcd_Caractere(3, 0x80);
   Lcd_Caractere(3, 0x10);
   Lcd_Caractere(3, 0x0F);
   Lcd_Caractere(3, 0x01);
   Lcd_Caractere(3, 0x06);
} /* Lcd_Initialise */

/*******************************************************************/
/*  FIN DU MODULE LCD.C                                            */
/*******************************************************************/
/*******************************************************************/
