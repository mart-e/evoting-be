/*******************************************************************/
/*******************************************************************/
/*  MODULE LECTEUR.C Gestion des deux lecteurs de cartes           */
/*******************************************************************/
/* START CONFIDENTIAL */

/*******************************************************************/
#include <dos.h>
#include <conio.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"

#include "com.inc"
#include "variable.h"
#include "vote.h"
#include "erreur.h"
#include "lcd.h"
#include "eprom.h"
#include "zeit.h"
#include "Mcr.h"
#include "klavier.h"
#include "lecteur.h"

/***********************************************/

unsigned long far *timer = (unsigned long *) 0x46C;

#define _WAIT { unsigned long inc;                     \
 for(inc=*timer;*timer - inc < (unsigned long) 1;);    \
              }
#define _WAIT2 { unsigned long inc;                     \
 for(inc=*timer;*timer - inc < (unsigned long) 2;);    \
              }

#define _WAITL { unsigned long inc;                     \
 for(inc=*timer;*timer - inc < (unsigned long) 5;);    \
              }
#define _WAIT_TIME_S  (unsigned long) 100
#define _WAIT_TIME  (unsigned long) 200

int   SECU;
long  SECU_TIMER;
unsigned long Timer_1,Timer_2;
int rang,valrang;

char  etat;
char  Command_R1[100];
char  Command_W1[100];
char  Command_R2[100];
char  Command_W2[100];
char  CARTE_VALID[100];

/* << 10.04 */
int VOTE_POS;
/* 10.04 >> */

/***********************************************/
#define _DUMMY  0

#define _RX_SIZE  4096
#define _TX_SIZE  1024

#define _PORT_BAUDRATE      COM_B9600
#define _PORT_FORMAT        COM_F8BOP1S
#define _PORT_DTR           1
#define _PORT_RTS           1
#define _PORT_HANDSHAKE     0

#define _PORT_1  1
#define _PORT_2  2

uchar PORT_1_BUFFER_IN[_RX_SIZE];
uchar PORT_1_BUFFER_OUT[_TX_SIZE];
int NB_RECOM_1;
COM PORT_1;

COM PORT_2;
int PORT_ACTIVE;

/* << 10.02b */
/* Vote counters */
extern int  *piNOMBRE_VOTES1, *piNOMBRE_VOTES2, *piNOMBRE_VOTES3;
extern int  *piNOMBRE_NATIO_VOTES1, *piNOMBRE_NATIO_VOTES2, *piNOMBRE_NATIO_VOTES3;
extern int  *piNOMBRE_EURO_VOTES1, *piNOMBRE_EURO_VOTES2, *piNOMBRE_EURO_VOTES3;
extern int  *piNOMBRE_STR_VOTES1, *piNOMBRE_STR_VOTES2, *piNOMBRE_STR_VOTES3;

/* 10.02b >> */

/* << 10.04 */
extern char szIDCreation[12];
/* 10.04 >> */

/***********************************************/

#define _DEBUT  0

/*  VARIABLES

   si WAIT alors attente de 2 secondes avant de revenir a la premiere
                 commande
   SENS determine le sens d'ejection de la carte
   PORT_NEXT_COMMAND donne l'instruction a executer (indice du taleau)
   si PORT_DO_NEXT_SEND alors possibilite envoie une nouvelle commande
                              au lecteur
   PORT_COMMAND tableau contenant les instructions a executer par le
                lecteur. Les indices impairs representent l'envoie
                de commande au lecteur, les paires la recuperation
                du message et des actions en consequence
                la fonction Fin() permet de revenir au debut du tableau
                la fonction Do_Nothing() ne sert a rien
*/

int SENS_1;
int PORT_1_NEXT_COMMAND;
int PORT_1_PAST_COMMAND;

int PORT_1_DO_NEXT_SEND;
int (* PORT_1_COMMAND[]) (COM *) =
                   {
                     Lecteur_Status_Feed,  /* Attente d'une carte */
                     Lecteur_Read,         /* Lecture de la carte */
                     Lecteur_Valide,       /* V‚rification de la carte */
                     Lecteur_Write,        /* Ecriture sur la carte */
                     Lecteur_Status_Write, /* Ecriture c'est bien pass‚e ? */
                     Lecteur_Eject,        /* Eject front ou rear */
                     Lecteur_Status_Eject, /* Carte eject‚e ? */
                     Fin
                   };

int SENS_2;
int PORT_2_NEXT_COMMAND;
int PORT_2_DO_NEXT_SEND;
int ( * PORT_2_COMMAND[]) (COM *) =
                   {
                     Lecteur_Status_Feed,  /* Attente d'une carte */
                     Lecteur_Read,         /* lecture de la carte */
                     Lecteur_Enregistre,   /* R‚cup‚ration du contenu */
                     Lecteur_Eject,        /* Eject front ou rear  */
                     Lecteur_Status_Eject, /* 0x00 ou incident 0a ? */
                     Lecteur_Valide_Vote,  /* OK on enregistre */
                     Lecteur_Get_Position,
                     Lecteur_Vide,
                     Fin,
                     Lecteur_Deblock,
                     Lecteur_Status_Deblock
                   };

/***************************************************************************/

#define  _LECTEUR_GOOD     0
#define  _LECTEUR_BUSY    -1
#define  _TIME_OUT        -2
#define  _CARTE_OVERFLOW  -3
#define  _INVALID_CARD    -4
#define  _EJECT_CARD      -5
#define  _NON_EJECT_CARD  -6
#define  _DONNEES         -7
#define  _DO_CLEAR        -8
#define  _DO_RECOMM       -9
#define  _CARTE_BLOCKE    -10
#define  _EJECT_CARD_REAR -11
#define _PASS_OUT_FRONT  1
#define _PASS_OUT_REAR   2

uchar C_BLOQ[]             = "m\x0D";
uchar C_POSITION[]         = "s\x0D";
uchar C_EJECT_FRONT[]      = "f\x0D";
uchar C_EJECT_FRONT_FAST[] = "x\x0D";
uchar C_EJECT_REAR[]       = "o\x0D";
uchar C_READ[]             = "r\x0D";
uchar C_WRITE[]            = "w%s\x0D";

uchar CARTE_1[_CARTE_MAX_BYTE+1];
extern   int  C_VOTE_MAX_BYTE;
int First = 0;
int VAR_DEB = 0;


/***************************************************************************/

void Light_LED(char LED)
{
 outp(_LED_PORT, LED);
} /* Light_LED */

/***************************************************************************/

int Do_command(int Command, COM *Port)
/*
   Execute la fonction
   Parametres  indice du tableau de commande
               port actif
   Rend un status sur l'execution
*/
{
  int (*funct) (COM *);
  int status;

  if (Port == &PORT_2)
    {
      funct = PORT_2_COMMAND[Command];
      PORT_ACTIVE = _PORT_2;
      status = (funct)(Port);
    }
  else
    {
      funct = PORT_1_COMMAND[Command];
      PORT_ACTIVE = _PORT_1;
      status = (funct)(Port);
     _WAIT;
    }
  return(status);
} /* do_command */

/***************************************************************************/
/*
  Toutes les fonctions ont comme parametre le port actif
  et rende un status sur l'execution de la fontion
  Ce status a soit une valeur positive et correspond au message
  d'erreur rendu par le lecteur. Les valeurs negatives sont
     _LECTEUR_BUSY : lecteur occup‚
     _TIME_OUT : time out sur le temps de reponse du lecteur
                 sauf pour l'avalage
*/
/****************************************************************************/
int Fin(COM *Port)
/*
   Fonction pour boucler sur la premiere commande du tableau
*/
{
  if (PORT_ACTIVE == _PORT_2)
    {
      COLOR = _ON_GREEN;
      Light_LED(COLOR);
      PORT_2_NEXT_COMMAND = _DEBUT;
      PORT_2_DO_NEXT_SEND = true;
      putch(0x07);
    }
  else
    {
      PORT_1_NEXT_COMMAND = _DEBUT;
      PORT_1_DO_NEXT_SEND = true;
    }

  return(_LECTEUR_BUSY);
}
/****************************************************************************/

int Port_receive_completed(COM *Port)
/*
   Determine si la reponse du lecteur est complete
   C'est-a-dire termin‚ par CR
   Cette fonction utilise la structure d‚finie par le driver
   et doit etre modifi‚e si celle-ci change
*/
{
  uchar *plastcar;
  char c;
  int nbcar;

 nbcar = com_rx_available(Port);
 _disable();
 if (Port->r_put == PORT_1_BUFFER_IN)
           plastcar = PORT_1_BUFFER_IN + _RX_SIZE - 1;
 else
           plastcar = Port->r_put - 1;
 _enable();
 return((*plastcar == '\x0D') && (nbcar > 0));

} /* Port_receive_completed */
/****************************************************************************/
int Lecteur_Set_Led(COM *Port,int color)
/*
   Positionnement des differentes leds
   Valeurs possibles de l'argument color :
   0 : Rouge OFF / Vert OFF
   1 : Rouge OFF / Vert ON
   2 : Rouge ON / Vert OFF
   3 : Rouge ON / Vert ON
*/
{
  uchar *command;
  char str_color[10];

  sprintf(Command_W1,"%d\r",color);
  for (command = Command_W1; *command; command++)
                                com_tx(*command, Port);

  return(_LECTEUR_GOOD);
} /* Lecteur_Set_Led */
/****************************************************************************/
int Lecteur_Read(COM *Port)
/*
   Ordre d'avalage et de lecture
*/
{
  uchar *command;
  if(PORT_ACTIVE == _PORT_1)
     {
          while(com_rx_available(Port))
                                 com_rx(Port);
         for (command = C_READ; *command; command++)
                                com_tx(*command, Port);
    }
  else
       Tcr_Snd(C_READ);

 return(_LECTEUR_GOOD);

} /* Lecteur_Read */

/***************************************************************************/

int Lecteur_Write(COM *Port)
/*
   Ordre d'ecriture
*/
{
  uchar *command;

   sprintf(Command_W1,C_WRITE,CARTE_VALID);
   Command_W1[_C_TEST_PLACE + 1] = Type_Multi;
                   /* Ecritre du type d'election */
   Command_W1[_C_PASSAGE_PLACE + 1] ='0';

/*****/
   Calcul_Crc(Command_W1,_C_TEST_PLACE + 1);
   Encode_Crc(Command_W1);
/*****/
   Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[72]);
   while(com_rx_available(Port))
                          com_rx(Port);
   for (command = Command_W1; *command; command++)
                               com_tx(*command, Port);
   return(_LECTEUR_GOOD);

} /* Lecteur Write */

/***************************************************************************/

int Lecteur_Eject(COM *Port)
/*
    Ordre d'ejection, le sens est determine par les variables
      SENS_?
*/
{
  uchar *command;
  char tab_Epror[10];
  int status;

  if (PORT_ACTIVE == _PORT_1)
         {
            while(com_rx_available(Port))
                                    com_rx(Port);
            for (command = C_EJECT_FRONT; *command; command++)
                                        com_tx(*command, Port);
         }
  else
         {
             if (SENS_2 == _PASS_OUT_REAR)
                   Tcr_Snd(C_EJECT_REAR);
             else
                   Tcr_Snd(C_EJECT_FRONT_FAST);
         }

  return(_LECTEUR_GOOD);
} /* Lecteur_Eject */
/***************************************************************************/

int Lecteur_Vide(COM *Port)
/*
    Lecteur est vide ?
*/
{
 int c;

 if(Fin_Rx(Command_R2))
                 return(_LECTEUR_BUSY);;

 if (Command_R2[1] == '8')
    {
      if ((Command_R2[2] == '4') || (Command_R2[2] == '0'))
        {
           PORT_2_DO_NEXT_SEND = true;
           return(_LECTEUR_GOOD);
        }

      Timer_2 = *timer;
      PORT_2_NEXT_COMMAND = _DEBUT;
      PORT_2_DO_NEXT_SEND = true;
      return(_LECTEUR_GOOD);
    }

 McrExit();
 Lecteur_2_Init();
 PORT_2_NEXT_COMMAND = 6;
 PORT_2_DO_NEXT_SEND = true;
 return(_LECTEUR_BUSY);


} /* Lecteur_Vide */
/***************************************************************************/

int Lecteur_Deblock(COM *Port)
/*
    Lecteur est BLOQUE.
*/
{
 _WAITL;
 COLOR = COLOR | 0x20;
 Tcr_Snd(C_BLOQ);
 return(_LECTEUR_GOOD);
}
/***************************************************************************/

int Lecteur_Status_Deblock(COM *Port)
/*
*/
{
  char *buffer;
  char choix,Clef[11];
  int status = 0, i, j;

  buffer = Command_R2;
  if (Tcr_Rx(Command_R2)) return(_LECTEUR_BUSY);
  status = Test_Retour(Command_R2);
  switch (status)
        {
          case 0   : if(VAR_DEB >= 15)
                        {
                          Lcd_Affiche(0,0, Message_Point[20]);
                          Lcd_Affiche(1,0, Message_Point[21]);
                          COLOR = _OFF_RED_GREEN;
                        /* << 10.02f  */
                          /*choix = Getch(COLOR);*/
                          choix = ' ';
                          while((choix != 'O') && (choix != 'N'))
                                choix = Getch(COLOR);
                        /* 10.02f >> */

                          _WAITL;
                          VAR_DEB = 0;
                          if ( choix  == 'O') /* OUI*/
                             {

                               /*
                               << 10.03b
                               for(i = 0;(i < 3) && status == 0;i++)
                                 {
                                   //<< 10.02e
                                   //Demande_Identite(Clef);
                                   status = Demande_Identite(Clef,false);
                                   if(!status)
                                        break;
                                   else
                                   // 10.02e >>
                                        status = Verifie_Urne(Clef);
                                 }
                                 10.03b >>
                                 */

                                 /* << 10.02f */
                                 Lcd_Affiche(0,0, " ");
                                 Lcd_Affiche(1,0, " ");
                                /* 10.02f >> */

                                   Timer_2 = *timer;
                                /* << 10.02h */
                                   /*if (status) */
                                   /* << 10.02f */
                                   //{
                                /* 10.02h >> */
                                      /*return(_EJECT_CARD_REAR);*/
                                    SAUVE_TO_EPROM = false;
                                    return(_EJECT_CARD);
                                   //}
                                   /* 10.02f */
                             }
                          Lcd_Affiche(0,0, Message_Point[87]);
                          Lcd_Affiche (1, 0, " ");
                          _WAITL;
                        }
                     break;
      }
  VAR_DEB++;
  SENS_2 = _PASS_OUT_FRONT ;
  return(_EJECT_CARD);

} /* Lecteur_Status_Write */
/***************************************************************************/

int Lecteur_Get_Position(COM *Port)
/*
    Ordre pour determiner la position de cartes dans le lecteur
*/
{
  int status;

  Tcr_Snd(C_POSITION);
  return(_LECTEUR_GOOD);

} /* Lecteur_Position */
/***************************************************************************/

int Lecteur_Put_Tempo()
/*
    Ordre pour temporiser
*/
{
  uchar *command;
  char *buffer;
  char status;

  strcpy(Command_W1 , "t0");
  strcat(Command_W1,"\x0D");
  for (command = Command_W1; *command; command++)
                             com_tx(*command, &PORT_1);
  strcpy(Command_W2 , "t2");
  strcat(Command_W2,"\x0D");
  McrBg();
  Tcr_Snd(Command_W2);
  _WAITL;

  Timer_1 = *timer;
  while((!Port_receive_completed(&PORT_1)) &&
                 ((*timer - Timer_2) <= (_WAIT_TIME_S)));
  buffer = Command_R1;

  if(Port_receive_completed(&PORT_1))
                 while ((*buffer = com_rx(&PORT_1)) != 0x0D) buffer++;
  if( Command_R1[0] == 'x' && Command_R1[2] == '0')
           status = 0;
  else
           status = 1;

  switch (status)
        {
          case 0   : break;
          default  : Lcd_Affiche(0,0, Message_Point[73]);
                     Lcd_Affiche(1,0, Message_Point[75]);
                     Boucle_Erreur();
        }

  Timer_2 = *timer;
  do
   {
     McrBg();
   } while((Tcr_Rx(Command_R2)) && ((*timer - Timer_2) <= (_WAIT_TIME_S)));

  if( Command_R2[0] == 'x' && Command_R2[2] == '0')
           status = 0;
  else
           status = 1;

  switch (status)
        {
          case 0   : break;
          default  : Lcd_Affiche(0,0,  Message_Point[74]);
                     Lcd_Affiche(1,0,  Message_Point[75]);
                     Boucle_Erreur();
        }

 return(_LECTEUR_GOOD);
} /* Lecteur_Position */
/***************************************************************************/

int Lecteur_Status_Feed(COM *Port)
/*
   Determine si une carte est entree
*/
{
  int c;

  if ( PORT_ACTIVE == _PORT_2 )
       {
         c = McrRx();
         if( c == 'a')
                {
                  /* << 10.03b */
                    Maj_IvBit(1);
                  /* 10.03b >> */

                  Timer_2 = *timer;
                  COLOR = _ON_RED;
                  Light_LED(COLOR);
                  return(_LECTEUR_GOOD);
                 }
        }
  else
        {
            c = com_rx(Port);
            if(c == 'a')
               {

                 /* << 10.03b */
                    Maj_IvBit(1);
                  /* 10.03b >> */

                 Timer_1 = *timer;
                 return(_LECTEUR_GOOD);
               }
        }

  return(_LECTEUR_BUSY);

} /* Lecteur_Status_Feed */
/***************************************************************************/

int Lecteur_Status_Write(COM *Port)
/*
  Ecrit une carte (ici valideuse uniquement)
*/
{
  int status,nbcar;
  char *buffer,strtmp[25];
  nbcar  = 0;
  buffer = Command_R1;


  if (Port_receive_completed(&PORT_1))
   {
    while (((*buffer = com_rx(Port)) != 0x0D) &&
                          (nbcar++ < _CARTE_MAX_BYTE)) buffer++;
    status = Test_Retour(Command_R1);
    switch (status)
        {
          case 0   :
                     Lecteur_Set_Led(&PORT_1,0);
                     Lcd_Affiche(1, 0, " ");

                     if ((Type_Multi == 'N') && (MULTI_ELEC == 'P'))
                          {
                            Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[99]);
                            NOMBRE_NATIO_VAL++;
                            NOMBRE_VAL++;
                          }

                     if ((Type_Multi == 'N') && (MULTI_ELEC != 'P'))
                          {
                            Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[76]);
                            NOMBRE_VAL++;
                            /* << 10.02b */
                            NOMBRE_NATIO_VAL++;
                            /* 10.02b >> */
                          }


                     if (Type_Multi == 'E')
                      {
                        Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[100]);
                        NOMBRE_EURO_VAL++;
                        NOMBRE_VAL++;
                      }

                     if (Type_Multi == 'S')
                      {
                        Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[101]);
                        NOMBRE_STR_VAL++;
                        NOMBRE_VAL++;
                      }

                     if(Cmpt_Gen_Val < _E_TABLE_VAL_PLACE)
                      {
                        Valide_E_Val(Cmpt_Gen_Val);
                        Table_Val[Cmpt_Gen_Val] = 1;
                        Cmpt_Gen_Val++;
                      }

                     Type_Multi = 'N';
                     return(_LECTEUR_GOOD);
                     break;
          default  : Lecteur_Set_Led(&PORT_1,2);   /* Led Rouge allumee */
                     return(status);
                     break;
        }
   }
  return(_LECTEUR_BUSY);
} /* Lecteur_Status_Write */
/***************************************************************************/

int Lecteur_Status_Eject(COM *Port)
/*
*/
{
  int status,nbcar;
  char *buffer;

  nbcar = 0;

  if ( PORT_ACTIVE == _PORT_1 )
        {
          if (! Port_receive_completed(&PORT_1))
                            return(_LECTEUR_BUSY);
           buffer = Command_R1;
           while (((*buffer = com_rx(Port)) != 0x0D)
                    && (nbcar++ < _CARTE_MAX_BYTE)) buffer++;
          *buffer = '\0';
          status = Test_Retour(Command_R1);
        }
  else
        {
          if (Tcr_Rx(Command_R2))
                   return(_LECTEUR_BUSY);
          status = Test_Retour(Command_R2);
        }

   switch (status)
        {
          case  _LECTEUR_GOOD   : if ( PORT_ACTIVE == _PORT_2 )
                                                    VAR_DEB = 0;
                                  return(_LECTEUR_GOOD);
                                  break;
          case  _NON_EJECT_CARD : if( PORT_ACTIVE == _PORT_1 )
                                      Lcd_Affiche(1,0, Message_Point[87]);
                                     else
                                        Lcd_Affiche(0,0, Message_Point[87]);
                                  return(_EJECT_CARD);
                                  break;
          case  _CARTE_BLOCKE : if( PORT_ACTIVE == _PORT_2 )
                                   {
                                    Lcd_Affiche(0,0, Message_Point[87]);
                                    status = _DO_RECOMM;
                                    }
                                 return(status);
                                 break;
          case  _DONNEES       : if( PORT_ACTIVE == _PORT_2 )
                                    status = _DO_RECOMM;
                                 return(status);
                                 break;

                     default    : if ( PORT_ACTIVE == _PORT_1  && First > 0)
                                      {
                                        if ((*timer - Timer_1) > _WAIT_TIME)
                                                  Lecteur_Set_Led(&PORT_1,3);
                                        else
                                            Lecteur_Set_Led(&PORT_1,2);
                                       }
                                   First = 1;
                                   return(status);
        }
return(_LECTEUR_BUSY);
} /* Lecteur_Status_Write */
/***************************************************************************/

void Valide()
/*
   Prepare le format de la carte a ecrire
*/
{
  Initialise_Carte(CARTE_VALID);
  Encode_Jeton(CARTE_VALID, JETON);

} /* Valide */
/****************************************************************************/
/* << 10.02f */
/*
   <FUNCTION>
		<NAME>Check_Election</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Determines whether the card comes from another polling station of the same election
		</DESCRIPTION>
        <PARAMETER>
			<NAME>pzJeton_B, pzJeton_C</NAME>
			<TYPE>char *, char *</TYPE>
			<IO>IN,IN</IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
int Check_Election(char *pzJeton_B, char *pzJeton_C)
{
	char szJeton_B[4], szJeton_C[4];

	szJeton_B[0] = pzJeton_B[0];
	szJeton_B[1] = pzJeton_B[2];
	szJeton_B[2] = pzJeton_B[4];
	szJeton_B[3] = '\0';

	szJeton_C[0] = pzJeton_C[0];
	szJeton_C[1] = pzJeton_C[2];
	szJeton_C[2] = pzJeton_C[4];
	szJeton_C[3] = '\0';

	return strcmp(szJeton_B,szJeton_C);

}
/* 10.02f >> */
/***************************************************************************/

int Lecteur_Valide(COM *Port)
/*
   Lit une carte a valider
*/
{
  uchar *buffer,strtmp[25];
  char Jeton[_JETON_MAX_BYTE+1],strtype[3];
  int status,nbcar;

  nbcar = 0;
  buffer = CARTE_1;
  if (Port_receive_completed(&PORT_1))
    {
       while (((*buffer = com_rx(Port)) != 0x0D) &&
                          (nbcar++ < _CARTE_MAX_BYTE)) buffer++;
       *buffer = '\0';

       strcpy(Command_R1,CARTE_1);
       status = Test_Retour(Command_R1);

       if((status == _EJECT_CARD) && (etat == '2')) return(_LECTEUR_GOOD);
       else
         {
            if(status == _EJECT_CARD)
                           return(status);
         }

       Recupere_Jeton(Command_R1,Jeton);
       Calcul_Crc(Command_R1,_C_TEST_PLACE);
       Recupere_Crc(Command_R1,_C_CRC_PLACE );

       if((strcmp(JETON, Jeton) == 0) && (strcmp(Crc_Calcul, Crc_Recup) == 0))
       {
             Recupere_stat_Val(Command_R1);
             if((Statut_Val == 'N') && (MULTI_ELEC == 'P'))
                   Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[102]);
             if((Statut_Val == 'N') && (MULTI_ELEC != 'P'))
                   Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[77]);
             if(Statut_Val == 'E')
                   Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[103]);
             if(Statut_Val == 'S')
                        Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[104]);
             if(Statut_Val == ' ')
                        Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[77]);

             Lecteur_Set_Led(&PORT_1,1); /* Led Verte allumee */
             PORT_1_NEXT_COMMAND = 4;
             /* << 10.02f */
             putch(0x07);
             /* 10.02f >> */

       }
       /* << 10.02f */
       else if(Check_Election(JETON, Jeton) == 0)
       {
            Lcd_Affiche(PORT_ACTIVE, 0, Message_Point[80]);
			Lecteur_Set_Led(&PORT_1,1); /* green led lit */
			PORT_1_NEXT_COMMAND = 4;
			putch(0x07);
       }
      /* 10.02f >> */
       return(_LECTEUR_GOOD);
    }
   return(_LECTEUR_BUSY);

} /* Lecteur_Valide */

/***************************************************************************/

int Determine_Position()
/*
    Determine la position de l'EPROM ou sera conserve le prochain vote
    Parametre: sans
    Rend la position
*/
{
  /* << 10.02b */
  /*if (NOMBRE_VOTES >= _E_NB_MAX_VOTE)*/
  if (*piNOMBRE_VOTES1 >= _E_NB_MAX_VOTE)
  /* 10.02b >> */
        return(-1);
  else
      /* << 10.02b */
        /*return(NOMBRE_VOTES);*/
        return(*piNOMBRE_VOTES1);
      /* 10.02b >> */
} /* Determine_Position */
/***************************************************************************/

int Enregistre(char *Carte)
/*
   Enregistre un vote
   Parametre: sans
   Rend numero d'erreur
*/
{
  int  Passage,status,i;
  char Jeton[_JETON_MAX_BYTE+1];
  char strtmp[4];
  /* << 10.04 */
  unsigned int iVoteSize;
  char szPassword[16];
  /* 10.04 >> */

  Recupere_Jeton(Carte, Jeton);
  Calcul_Crc(Carte,_C_TEST_PLACE);
  Recupere_Crc(Carte,_C_CRC_PLACE);

  status = strcmp(Jeton, JETON); /* != 0; */

  if((status != 0) || (strcmp(Crc_Calcul,Crc_Recup) != 0))
     {
           Lcd_Affiche(0,0, Message_Point[83]);
           SENS_2 = _PASS_OUT_FRONT;
           return(_EJECT_CARD);
    }
  else
    {
      Lcd_Affiche(0, 0, Message_Point[85]);
      POSITION_VOTE = Determine_Position();
      if (POSITION_VOTE == -1)
        {
          Lcd_Affiche(0, 0, Message_Point[22]);
          Wait_Time(1);
          return(_CARTE_OVERFLOW);
        }
      Recupere_stat(Carte); /* N ou E ou S */
      SAUVE_TO_EPROM = true;
      Recupere_Vote(Carte);
      status = Verifie_E_Vote();
      if(status != 0)
        {
           Lcd_Affiche(0,0, Message_Point[83]);
           SENS_2 = _PASS_OUT_FRONT;
           return(_EJECT_CARD);
        }

      /* << 10.04 */
      while(Table[Cmpt_Gen] != 0)
      {
          Cmpt_Gen = ( Cmpt_Gen >= (_E_NB_MAX_VOTE - 1)) ? 0 : Cmpt_Gen++;
      }

       iVoteSize = C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE;
       Generate_Password(szPassword,Cmpt_Gen,true);
       status = Encrypt_Vote(szVoteContents,szPassword,iVoteSize);
       if(!status)
       {
           Lcd_Affiche(0,0, Message_Point[79]);
           SENS_2 = _PASS_OUT_FRONT;
           return(_EJECT_CARD);
       }

       VOTE_POS = Cmpt_Gen;
      /* 10.04 >> */


      SAUVE_TO_EPROM = true;
      SENS_2 = _PASS_OUT_REAR;
    }
return(_LECTEUR_GOOD);
} /* Enregistre */

/***************************************************************************/

int Lecteur_Enregistre(COM *Port)
/*
   Lit une carte contenant un vote
*/
{
  uchar *buffer2;
  int status;
  char plastcar;


  buffer2 = Command_R2;
  if(Tcr_Rx(buffer2))
                   return(_LECTEUR_BUSY);
  status = Test_Retour(Command_R2);
  switch (status)
         {
             case _DONNEES    : status = Enregistre(buffer2);
                                return(status);
                                break;
             case _EJECT_CARD : SENS_2 = _PASS_OUT_FRONT;
                                if (PORT_ACTIVE == _PORT_2 && etat == '2')
                                  {
                                   COLOR =_ON_RED_GREEN;
                                   Light_LED(COLOR);
                                   Lcd_Affiche(0, 0, Message_Point[79]);
                                  }
                                return(status);
                                break;
                      default : return(status);
                                break;
         }

return(_LECTEUR_BUSY);
} /* Lecteur_Enregistre */

/***************************************************************************/

int Lecteur_Valide_Vote(COM *Port)
/*
   Validation d'un vote: apres ejection on teste la presence d'une carte
   si la carte est bien ejectee, on valide le vote
   si non message d'erreur, et le vote n'est pas pris en compte
*/
{
  int i,j;
  /* << 10.03c */
  /* << 10.04 */
  /*char szPassword[16];*/
  /* 10.04 >> */
  unsigned int iVoteSize;
  /* 10.03c >> */

  if ((SENS_2 == _PASS_OUT_REAR) && (SAUVE_TO_EPROM == true))
     {

    /* << 10.04 */
       /*while(Table[Cmpt_Gen] != 0)
          {
           Cmpt_Gen = ( Cmpt_Gen >= (_E_NB_MAX_VOTE - 1)) ? 0 : Cmpt_Gen++;
          }

       Table[Cmpt_Gen] = 1;*/
       Table[VOTE_POS] = 1;
       /* << 10.02b */
       Check_Vote_Counters();
       /* 10.02b >> */

       /* << 10.03c */
       iVoteSize = C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE;
       /*Generate_Password(szPassword,Cmpt_Gen,true);
       Encrypt_Decrypt(Strtmp,szPassword,iVoteSize);*/
       /*Encode_E_Vote(Cmpt_Gen, Strtmp,strlen(Strtmp));*/
       /*Encode_E_Vote(Cmpt_Gen, Strtmp,iVoteSize);*/
       /* 10.03c >> */
       Encode_E_Vote(VOTE_POS, szVoteContents,iVoteSize);
    /* 10.04 >> */

       if(Statut_Vote == 'E')
       /* << 10.02b */
       {
            /*NOMBRE_EURO++;*/
            *piNOMBRE_EURO_VOTES1 += 1;
            *piNOMBRE_EURO_VOTES2 += 1;
            *piNOMBRE_EURO_VOTES3 += 1;
       }
      /* 10.02b >> */
       else if(Statut_Vote == 'S')
       /* << 10.02b */
       {
            /*NOMBRE_STR++;*/
            *piNOMBRE_STR_VOTES1 += 1;
            *piNOMBRE_STR_VOTES2 += 1;
            *piNOMBRE_STR_VOTES3 += 1;
       }
      /* 10.02b >> */
       else
        /* << 10.02b */
            /*NOMBRE_NATIO++;*/
        if(Statut_Vote == 'N')
        {
            *piNOMBRE_NATIO_VOTES1 += 1;
            *piNOMBRE_NATIO_VOTES2 += 1;
            *piNOMBRE_NATIO_VOTES3 += 1;
        }

       /*NOMBRE_VOTES++;*/
       *piNOMBRE_VOTES1 += 1;
       *piNOMBRE_VOTES2 += 1;
       *piNOMBRE_VOTES3 += 1;
     /* 10.02b >> */
       Lcd_Affiche(0,0,Message_Point[86]);
     }

 /* << 10.02f */
  /*if ((SENS_2 == _PASS_OUT_REAR) && (SAUVE_TO_EPROM == false))
   {
     Lcd_Affiche(0,0,Message_Point[80]);
     Lcd_Affiche(1,0," ");
   }*/
/* 10.02f >> */

 SAUVE_TO_EPROM = false;
 SENS_2 = _PASS_OUT_FRONT;
 return(_LECTEUR_GOOD);
} /* Lecteur_Valide_Vote */

/***************************************************************************/

int Lecteur_1_Init()
{
   uint status;

   status = com_define(COM_1, _DUMMY, _DUMMY, PORT_1_BUFFER_IN,
                      _RX_SIZE, PORT_1_BUFFER_OUT, _TX_SIZE, &PORT_1);
   status = com_open(_PORT_BAUDRATE, _PORT_FORMAT,
                                      _PORT_DTR, _PORT_RTS, &PORT_1);
   status = com_handshake(_PORT_HANDSHAKE, &PORT_1);

   return (status);
} /* Lecteur 1 */

/***************************************************************************/
int Lecteur_2_Init()
{
   uint status;
   status = TcrInit();
   return (status);
} /* Lecteur 2  */

/***************************************************************************/
void Boucle_Lecteur()
{
   int Status_1, Status_2;
   int  testkey,i;
   /* << 10.02b */
   /*char strtmp[21],strtmp2[21], Klavier;*/
   char strtmp[25], Klavier;
   /* 10.02b >> */
   uchar *command;

   rang = 0;


   Type_Multi = 'N';
   Valide();
   Lcd_Affiche(1, 0, " ");
   /* << 10.03b */
   /*Lcd_Affiche(1, 0, Message_Point[93]);*/
   /* 10.03b >> */
   putch(0x07);

   SECU = 0;

   PORT_1_NEXT_COMMAND = 5;
   PORT_1_DO_NEXT_SEND = true;
   PORT_2_NEXT_COMMAND = 6;
   PORT_2_DO_NEXT_SEND = true;
   SENS_1 = _PASS_OUT_FRONT;
   SENS_2 = _PASS_OUT_FRONT;
   Timer_1 = *timer;
   Timer_2 = *timer;
   First = 0;
   DEMANDE_FERMETURE = false;

   Lcd_Affiche (0, 0, Message_Point[91]);

   Recupere_Nb_Validations();   /**/

   Recupere_Nb_Cartes();
   /* << 10.02b */
   /*Cmpt_Gen = NOMBRE_VOTES;*/
   Cmpt_Gen = *piNOMBRE_VOTES1;
   /* 10.02b >> */
   Cmpt_Gen_Val = NOMBRE_VAL;

   while (!DEMANDE_FERMETURE)
     {
        McrBg();

        Secu_Stat();
        Cmpt_Gen = ( Cmpt_Gen >= (_E_NB_MAX_VOTE - 1)) ? 0 : Cmpt_Gen++;

        if (PORT_2_DO_NEXT_SEND)
          {
            Status_2 = Do_command(PORT_2_NEXT_COMMAND, &PORT_2);
            switch(Status_2)
             {
               case _DO_CLEAR:
                 PORT_2_NEXT_COMMAND = 8; /* fonction FIN */
                 PORT_2_DO_NEXT_SEND = true;
                 SENS_2 = _PASS_OUT_FRONT;
                 break;

               case _DO_RECOMM:
                 PORT_2_NEXT_COMMAND = 1;
                 PORT_2_DO_NEXT_SEND = true;
                 SENS_2 = _PASS_OUT_FRONT;
                 break;

               case _CARTE_OVERFLOW:
                      PORT_2_NEXT_COMMAND = 3;
                      SENS_2 = _PASS_OUT_FRONT;
                      PORT_2_DO_NEXT_SEND = true;
                      break;

               case _CARTE_BLOCKE:
                      PORT_2_NEXT_COMMAND = 9;
                      SENS_2 = _PASS_OUT_FRONT;
                      PORT_2_DO_NEXT_SEND = true;
                      break;


               case _INVALID_CARD:
                      Lcd_Affiche(0, 0, Message_Point[81]);
                      PORT_2_NEXT_COMMAND = 3;
                      PORT_2_DO_NEXT_SEND = true;
                      SENS_2 = _PASS_OUT_FRONT;
                      break;

               case _EJECT_CARD :
                      PORT_2_NEXT_COMMAND = 3;
                      SENS_2 = _PASS_OUT_FRONT;
                      PORT_2_DO_NEXT_SEND = true;
                      break;
             /* << 10.02f */
               /*case _EJECT_CARD_REAR :
                      PORT_2_NEXT_COMMAND = 3;
                      SENS_2 = _PASS_OUT_REAR;
                      SAUVE_TO_EPROM = false;
                      PORT_2_DO_NEXT_SEND = true;
                      VAR_DEB = 0;
                      break;*/
             /* 10.02f >> */
              default           :
                      if(Status_2 != _LECTEUR_BUSY)
                        {
                          PORT_2_NEXT_COMMAND++;
                          PORT_2_DO_NEXT_SEND = true;
                        }
             }
           }

/***************************************************************************/

        if (PORT_1_DO_NEXT_SEND)
          {
            Status_1 = Do_command(PORT_1_NEXT_COMMAND, &PORT_1);
            switch (Status_1)
             {
               case _DO_CLEAR:
                 PORT_1_NEXT_COMMAND = _DEBUT;
                 PORT_1_DO_NEXT_SEND = true;
                 SENS_1 = _PASS_OUT_FRONT;
                 break;

               case _CARTE_BLOCKE:

               case _DO_RECOMM:
                 PORT_1_NEXT_COMMAND = 1;
                 PORT_1_DO_NEXT_SEND = true;
                 SENS_1 = _PASS_OUT_FRONT;
                 break;

               case _LECTEUR_BUSY:
                      break;

               case _INVALID_CARD :
                      Lcd_Affiche(1, 0, Message_Point[81]);
                      Lecteur_Set_Led(&PORT_1,2);   /* Led Rouge allumee */
                      /* << 10.02f */
                       putch(0x07);
                      /* 10.02f >> */
                      PORT_1_NEXT_COMMAND = 5;
                      PORT_1_DO_NEXT_SEND = true;
                      SENS_1 = _PASS_OUT_FRONT;
                      break;

               case _EJECT_CARD :
                      Lcd_Affiche(1, 0, Message_Point[79]);
                      Lecteur_Set_Led(&PORT_1,2);   /* Led Rouge allumee */
                      /* << 10.02f */
                       putch(0x07);
                      /* 10.02f >> */
                      PORT_1_NEXT_COMMAND = 5;
                      PORT_1_DO_NEXT_SEND = true;
                      SENS_1 = _PASS_OUT_FRONT;
                      break;
               default:
                 PORT_1_NEXT_COMMAND++;
                 PORT_1_DO_NEXT_SEND = true;
             }
          }

      if ((*timer - Timer_1) > _WAIT_TIME)
        if(PORT_1_NEXT_COMMAND > _DEBUT)
          {
            Lecteur_Set_Led(&PORT_1,3);
            PORT_1_NEXT_COMMAND = 5;
            PORT_1_DO_NEXT_SEND = true;
            SENS_1 = _PASS_OUT_FRONT;
            Timer_1 = *timer;
          }

      if ((*timer - Timer_2) > _WAIT_TIME)
        if(PORT_2_NEXT_COMMAND > _DEBUT)
          {
            McrExit();
            Lecteur_2_Init();
            _WAIT;
            PORT_2_NEXT_COMMAND = 1;
            PORT_2_DO_NEXT_SEND = true;
            Timer_2 = *timer;
          }


      Klavier = Kbhit(COLOR,0);

        switch(Klavier)
         {
           case 0x00 :break;
           case '0' :if(MULTI_ELEC == 'N')
                      {
                       if((NOMBRE_VAL) > 1)
                         sprintf(strtmp,Message_Point[108] , NOMBRE_VAL);
                       else
                         sprintf(strtmp,Message_Point[109] , NOMBRE_VAL);
                       Lcd_Affiche(1, 0, strtmp);
                      }
                     else
                      if((GLOBAL_EURO == 1) && (GLOBAL_STR == 0))
                       {
                        switch(valrang)
                         {
                          case 0 :
                                   sprintf(strtmp,Message_Point[105],NOMBRE_NATIO_VAL);
                                   Lcd_Affiche(1, 0, strtmp);
                                   valrang++;
                                   break;
                          case 1 :
                                   sprintf(strtmp,Message_Point[106],NOMBRE_EURO_VAL);
                                   Lcd_Affiche(1, 0, strtmp);
                                   valrang++;
                                   break;
                          case 2 :
                                  if((NOMBRE_VAL) > 1)
                                        sprintf(strtmp,Message_Point[108] , NOMBRE_VAL);
                                  else
                                        sprintf(strtmp,Message_Point[109] , NOMBRE_VAL);
                                 Lcd_Affiche(1, 0, strtmp);
                                  valrang = 0;
                                  break;
                          }
                       }
                      else
                       if((GLOBAL_EURO == 0) && (GLOBAL_STR == 1))
                        {
                        switch(valrang)
                         {
                          case 0 :
                                   sprintf(strtmp,Message_Point[105],NOMBRE_NATIO_VAL);
                                   Lcd_Affiche(1, 0, strtmp);
                                   valrang++;
                                   break;
                          case 1 :
                                   sprintf(strtmp,Message_Point[107],NOMBRE_STR_VAL);
                                   Lcd_Affiche(1, 0, strtmp);
                                   valrang++;
                                   break;
                          case 2 :
                                  if((NOMBRE_VAL) > 1)
                                        sprintf(strtmp,Message_Point[108] , NOMBRE_VAL);
                                  else
                                        sprintf(strtmp,Message_Point[109] , NOMBRE_VAL);
                                  Lcd_Affiche(1, 0, strtmp);
                                  valrang = 0;
                                  break;
                          }
                        }
                       else
                        if((GLOBAL_EURO == 1) && (GLOBAL_STR == 1))
                         {
                          switch(valrang)
                           {
                            case 0 :
                                     sprintf(strtmp,Message_Point[105],NOMBRE_NATIO_VAL);
                                     Lcd_Affiche(1, 0, strtmp);
                                     valrang++;
                                     break;
                            case 1 :
                                     sprintf(strtmp,Message_Point[106],NOMBRE_EURO_VAL);
                                     Lcd_Affiche(1, 0, strtmp);
                                     valrang++;
                                     break;
                            case 2 :
                                     sprintf(strtmp,Message_Point[107],NOMBRE_STR_VAL);
                                     Lcd_Affiche(1, 0, strtmp);
                                     valrang++;
                                     break;
                            case 3 :
                                    if((NOMBRE_VAL) > 1)
                                        sprintf(strtmp,Message_Point[108] , NOMBRE_VAL);
                                    else
                                        sprintf(strtmp,Message_Point[109] , NOMBRE_VAL);
                                    Lcd_Affiche(1, 0, strtmp);
                                    valrang = 0;
                                    break;
                           }
                         }
                      _WAITL;
                      break;
           case '2' : if((PORT_1_NEXT_COMMAND == _DEBUT )
                         && (PORT_2_NEXT_COMMAND == _DEBUT ))
                                      DEMANDE_FERMETURE = true;
                      break;
           case '1' : Lcd_Affiche(0, 0, _Version);
                      break;
/*           case '4' : if (PORT_1_NEXT_COMMAND == _DEBUT )
                                  PORT_1_NEXT_COMMAND++;
                      break;  */
           case '5' :if(MULTI_ELEC == 'N')
                      {
                      /* << 10.02b  */
                        /*if(NOMBRE_VOTES > 1)
                          sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
                        else
                          sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);*/
                        if(*piNOMBRE_VOTES1 > 1)
                          sprintf(strtmp,Message_Point[88] , *piNOMBRE_VOTES1);
                        else
                          sprintf(strtmp,Message_Point[110] , *piNOMBRE_VOTES1);
                     /* 10.02b >>  */
                        Lcd_Affiche(0, 0, strtmp);
                      }
                     else
                      if((GLOBAL_EURO == 1) && (GLOBAL_STR == 0))
                       {
                        switch(rang)
                         {
                          case 0 :
                                 /* << 10.02b */
                                   /*if( NOMBRE_NATIO > 1)
                                      sprintf(strtmp,Message_Point[96],NOMBRE_NATIO);
                                   else
                                      sprintf(strtmp,Message_Point[111],NOMBRE_NATIO);*/
                                  if(*piNOMBRE_NATIO_VOTES1 > 1)
                                      sprintf(strtmp,Message_Point[96],*piNOMBRE_NATIO_VOTES1);
                                  else
                                      sprintf(strtmp,Message_Point[111],*piNOMBRE_NATIO_VOTES1);
                                /* 10.02b >> */
                                   Lcd_Affiche(0, 0, strtmp);
                                   rang++;
                                   break;
                          case 1 :
                                /* << 10.02b */
                                   /*if( NOMBRE_EURO > 1)
                                        sprintf(strtmp,Message_Point[97],NOMBRE_EURO);
                                   else
                                        sprintf(strtmp,Message_Point[112],NOMBRE_EURO);*/
                                   if(*piNOMBRE_EURO_VOTES1 > 1)
                                        sprintf(strtmp,Message_Point[97],*piNOMBRE_EURO_VOTES1);
                                   else
                                        sprintf(strtmp,Message_Point[112],*piNOMBRE_EURO_VOTES1);
                                /* 10.02b >> */
                                   Lcd_Affiche(0, 0, strtmp);
                                   rang++;
                                   break;
                          case 2 :
                                /* << 10.02b */
                                  /*if(NOMBRE_VOTES > 1)
                                     sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
                                  else
                                      sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);*/
                                  if(*piNOMBRE_VOTES1 > 1)
                                     sprintf(strtmp,Message_Point[88] , *piNOMBRE_VOTES1);
                                  else
                                      sprintf(strtmp,Message_Point[110] , *piNOMBRE_VOTES1);
                                /* 10.02b >> */
                                  Lcd_Affiche(0, 0, strtmp);
                                  rang = 0;
                                  break;
                          }
                       }
                      else
                       if((GLOBAL_EURO == 0) && (GLOBAL_STR == 1))
                        {
                        switch(rang)
                         {
                          case 0 :
                                /* << 10.02b */
                                   /*if( NOMBRE_NATIO > 1)
                                        sprintf(strtmp,Message_Point[96],NOMBRE_NATIO);
                                   else
                                        sprintf(strtmp,Message_Point[111],NOMBRE_NATIO);*/
                                   if(*piNOMBRE_NATIO_VOTES1 > 1)
                                      sprintf(strtmp,Message_Point[96],*piNOMBRE_NATIO_VOTES1);
                                   else
                                      sprintf(strtmp,Message_Point[111],*piNOMBRE_NATIO_VOTES1);
                                /* 10.02b >> */
                                   Lcd_Affiche(0, 0, strtmp);
                                   rang++;
                                   break;
                          case 1 :
                                /* << 10.02b */
                                   /*if( NOMBRE_STR > 1)
                                        sprintf(strtmp,Message_Point[98],NOMBRE_STR);
                                   else
                                        sprintf(strtmp,Message_Point[113],NOMBRE_STR);*/
                                   if(*piNOMBRE_STR_VOTES1 > 1)
                                        sprintf(strtmp,Message_Point[98],*piNOMBRE_STR_VOTES1);
                                   else
                                        sprintf(strtmp,Message_Point[113],*piNOMBRE_STR_VOTES1);
                                /* 10.02b >> */
                                   Lcd_Affiche(0, 0, strtmp);
                                   rang++;
                                   break;
                          case 2 :
                                /* << 10.02b */
                                  /*if(NOMBRE_VOTES > 1)
                                     sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
                                  else
                                      sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);*/
                                  if(*piNOMBRE_VOTES1 > 1)
                                     sprintf(strtmp,Message_Point[88] , *piNOMBRE_VOTES1);
                                  else
                                      sprintf(strtmp,Message_Point[110] , *piNOMBRE_VOTES1);
                                /* 10.02b >> */
                                  Lcd_Affiche(0, 0, strtmp);
                                  rang = 0;
                                  break;
                          }
                        }
                       else
                        if((GLOBAL_EURO == 1) && (GLOBAL_STR == 1))
                         {
                          switch(rang)
                           {
                            case 0 :
                                  /* << 10.02b */
                                     /*if( NOMBRE_NATIO > 1)
                                        sprintf(strtmp,Message_Point[96],NOMBRE_NATIO);
                                     else
                                        sprintf(strtmp,Message_Point[111],NOMBRE_NATIO);*/
                                     if(*piNOMBRE_NATIO_VOTES1 > 1)
                                      sprintf(strtmp,Message_Point[96],*piNOMBRE_NATIO_VOTES1);
                                     else
                                      sprintf(strtmp,Message_Point[111],*piNOMBRE_NATIO_VOTES1);
                                 /* 10.02b >> */
                                     Lcd_Affiche(0, 0, strtmp);
                                     rang++;
                                     break;
                            case 1 :
                                   /* << 10.02b */
                                     /*if( NOMBRE_EURO > 1)
                                        sprintf(strtmp,Message_Point[97],NOMBRE_EURO);
                                     else
                                        sprintf(strtmp,Message_Point[112],NOMBRE_EURO);*/
                                     if(*piNOMBRE_EURO_VOTES1 > 1)
                                        sprintf(strtmp,Message_Point[97],*piNOMBRE_EURO_VOTES1);
                                     else
                                        sprintf(strtmp,Message_Point[112],*piNOMBRE_EURO_VOTES1);
                                  /* 10.02b >> */
                                     Lcd_Affiche(0, 0, strtmp);
                                     rang++;
                                     break;
                            case 2 :
                                   /* << 10.02b */
                                     /*if( NOMBRE_STR > 1)
                                        sprintf(strtmp,Message_Point[98],NOMBRE_STR);
                                     else
                                        sprintf(strtmp,Message_Point[113],NOMBRE_STR);*/
                                     if(*piNOMBRE_STR_VOTES1 > 1)
                                        sprintf(strtmp,Message_Point[98],*piNOMBRE_STR_VOTES1);
                                     else
                                        sprintf(strtmp,Message_Point[113],*piNOMBRE_STR_VOTES1);
                                  /* 10.02b >> */
                                     Lcd_Affiche(0, 0, strtmp);
                                     rang++;
                                     break;
                            case 3 :
                                    /*if(NOMBRE_VOTES > 1)
                                      sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
                                    else
                                       sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);*/
                                    if(*piNOMBRE_VOTES1 > 1)
                                      sprintf(strtmp,Message_Point[88] , *piNOMBRE_VOTES1);
                                    else
                                       sprintf(strtmp,Message_Point[110] , *piNOMBRE_VOTES1);
                                    Lcd_Affiche(0, 0, strtmp);
                                    rang = 0;
                                    break;
                           }
                         }
                      _WAITL;
                      break;
           case '6' : if((GLOBAL_STR == 1)&&(PORT_1_NEXT_COMMAND == _DEBUT))
                          {
                             Type_Multi = 'S';
                             Valide();
                             Lcd_Affiche(1, 0, " ");
                             Lcd_Affiche(1, 0, Message_Point[95]);
                             putch(0x07);
                          }
                      break;
           case '7' : if((MULTI_ELEC == 'P')&&(PORT_1_NEXT_COMMAND == _DEBUT))
                          {
                             Type_Multi = 'N';
                             Valide();
                             Lcd_Affiche(1, 0, " ");
                             Lcd_Affiche(1, 0, Message_Point[93]);
                             putch(0x07);
                          }
                      break;
           case '8' : strcpy(strtmp,JETON_ETENDU);
                      Lcd_Affiche(0, 0, strtmp);
                      /* << 10.04 */
                      Lcd_Affiche(1, 0, szIDCreation);
                      /* 10.04 >> */
                      break;
           case '9' : if((GLOBAL_EURO == 1)&&(PORT_1_NEXT_COMMAND == _DEBUT))
                          {
                             Type_Multi = 'E';
                             Valide();
                             Lcd_Affiche(1, 0, " ");
                             Lcd_Affiche(1, 0, Message_Point[94]);
                             putch(0x07);
                         }
                      break;
         }
     }
} /* Boucle_Lecteur */

/***************************************************************************/
int Test_Retour(char *Command_Rx)
{
  int status;
  uchar Command_Tmp[100];

  if (*Command_Rx != 'x')
                   return(_DONNEES);
  if (*(Command_Rx + 1) != '0')
                   return(_DO_RECOMM); /* X8n */

  etat = *(Command_Rx + 2);
  switch(etat)
         {
           case '0' : status = _LECTEUR_GOOD;              /*  ACK   */
                      break;
           case '1' : status = _DO_RECOMM;  /* X01 EJECT Error         */
                      if ( PORT_ACTIVE == _PORT_2 )
                         {
                             McrExit();
                             Lecteur_2_Init();
                             _WAIT;
                         }
                      break;
           case '6' : status = _CARTE_BLOCKE;    /*  CARTE BLOCKE     */
                      break;
           case '7' : status = _DO_RECOMM;
                      break;
           case '8' : status = _DO_CLEAR;        /*     LECTEUR VIDE   */
                      break;
           case 'a' : status = _NON_EJECT_CARD;  /*  EJECT Error         */
                      break;
           case 'b' : status =  _CARTE_BLOCKE;  /*  EJECT Error         */
                      if ( PORT_ACTIVE == _PORT_2 )
                         {
                             McrExit();
                             Lecteur_2_Init();
                         }
                      break;
           case 'c' : status =  _CARTE_BLOCKE;  /*  EJECT Error         */
                      break;
           default  : status = _EJECT_CARD;  /*     All other cases are rej   */
                      break;
         }
  return(status);

}
/*******************************************************************/
void Secu_Stat()
/*
    Verifie si on est sous batteries
    Parametre: sans
    Rend void
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
              else
                 {
                    if((*timer - SECU_TIMER) > 1200)
                                            SECU = 2;
                    else
                       {
                          Lcd_Affiche(0,0,Message_Point[24]);
                          tmp = 15 - (( *timer - SECU_TIMER ) /  19);
                          if (tmp <= 0)
                              {
                                tmp = 0;
                                SECU = 2;
                                if ((PORT_1_NEXT_COMMAND == _DEBUT )
                                     && ((PORT_2_NEXT_COMMAND == _DEBUT )
                                /* << 10.03b */
                                     || (PORT_2_NEXT_COMMAND == 9 )
                                     || (PORT_2_NEXT_COMMAND == 10 )
                                /* 10.03b >> */
                                        ))
                                                             Battery_Off();
                              }

                           sprintf(strtmp, Message_Point[89], tmp);
                           Lcd_Affiche(1, 0, strtmp);
                           putch(0x07);
                       }
                  }
           }
   else
      {
        if(SECU > 0)
          {
            Lcd_Affiche (0, 0, Message_Point[91]);
            Lcd_Affiche (1, 0, Message_Point[92]);
          }
        SECU  = 0;
      }
} /* Recupere */
/*******************************************************************/
/* Fin du module LECTEUR.C                                         */
/*******************************************************************/
/* END CONFIDENTIAL */
