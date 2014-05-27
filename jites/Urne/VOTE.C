/* START CONFIDENTIAL */
/*******************************************************************/
/*  DEBUT DU MODULE VOTE.C                                         */
/*******************************************************************/

/* #include "cfgscrut.h" */
/* #include "cfgscrut.ext" */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

#include "variable.h"
#include "vote.h"

/* << 10.04 */
/*char     Strtmp[75]; */
char     szVoteContents[76];
/* 10.04 >> */
char     Statut_Vote;
char     Statut_Val;
char     Crc_Calcul[5],Crc_Recup[5];

/*
   Recupere_? : recupere la valeur de la variable ? de la carte
   Encode_?   : modifie la  valeur de la variable ? de la carte

   Jeton: jeton d'itentification du bureau de vote
   Vote: vote exprime par l'electeur
   NL: nombre de lecture de la carte par l'urne
   Numero: numero de la carte
   Passage: determine si la carte est passee dans la machine … voter
*/
char _Code[] = "0EC3ZN678LAB2DFRH1IJK9M5OPQGSTUVWXY4";

/***********************************************/

void Recupere_Jeton(char *Carte, char *Jeton)
{
  int i;

  for (i = 0; i < _C_JETON_MAX_BYTE;
                          Jeton[i] = Carte[_C_JETON_PLACE + i], i++);
  Jeton[_C_JETON_MAX_BYTE] = '\0';
} /* Recupere_Jeton */

/***********************************************/
Calcul_Jeton(char *Jeton_Etendu, char *Jeton)
{
    int i;
    int jj, mm,aa;
    int cant,buv1,buv2,tmp;
    char Cjj[3],Cmm[3],Caa[3];
    char Ccant[4], Cbuv1[4],Cbuv2[4];

    strncpy(Ccant,Jeton_Etendu + 3,3);
    strncpy(Cbuv1,Jeton_Etendu  + 6,2);
    strncpy(Cbuv2,Jeton_Etendu  + 8,1);
    strncpy(Cjj,Jeton_Etendu   + 9,2);
    strncpy(Cmm,Jeton_Etendu  + 11,2);
    strncpy(Caa,Jeton_Etendu  + 13,2);

    Ccant[3] = '\0';
    Cbuv1[2] = '\0';
    Cbuv2[1] = '\0';
    Cjj[2]   = '\0';
    Cmm[2]   = '\0';
    Caa[2]   = '\0';

    cant     = atoi(Ccant);
    buv1     = atoi(Cbuv1);
    buv2     = atoi(Cbuv2);
    jj       = atoi(Cjj);
    mm       = atoi(Cmm);
    aa       = atoi(Caa);

    for(i = 0; i < 5;i++)
    {
     Jeton[1] = _Code[buv1 % 35];
     Jeton[3] = _Code[buv2 % 35];
     Jeton[2] = _Code[(jj + cant) % 35];
     Jeton[0] = _Code[mm   % 35];
     Jeton[4] = _Code[aa   % 35];
    }
   Jeton[5] = '\0';
}
/***********************************************/
void Encode_Jeton(char *Carte, char *Jeton)
{
   int i;

   for (i = 0; i < _C_JETON_MAX_BYTE;
                          Carte[_C_JETON_PLACE + i] = Jeton[i], i++);
} /* Encode_Jeton */

/***********************************************/
void Recupere_Vote(char *Carte)
{
   int  debut, i;

   debut = _C_CRC_PLACE ;
   for (i = 0; i < (_C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE + C_VOTE_MAX_BYTE);
         /* << 10.4 */
         /*Strtmp[i] = Carte[debut++], i++);*/
         szVoteContents[i] = Carte[debut++], i++);

   /*Strtmp[i] = '\0';*/
   szVoteContents[i] = '\0';
   /* 10.4 >> */
} /* Recupere_Vote */
/***********************************************/
Recupere_stat(char *Carte)  /* Recupere statut de la carte … enregister */
{
  Statut_Vote =  Carte[_C_TEST_PLACE];
} /* Recupere_Vote */

/***********************************************/

Recupere_stat_Val(char *Carte)  /* Recupere statut de la valideuse */
{
  Statut_Val =  Carte[_C_TEST_PLACE];
} /* Recupere_Statut_Valideuse */
/***********************************************/
void Initialise_Carte(char *Carte)
/*
    Initialise une carte avec des blancs
    Parametre: adresse de la carte
    Rend void
*/
{
   int i;
   /* << 10.07 */
   /*for (i = 0; i < (C_VOTE_MAX_BYTE  + _C_VOTE_PLACE + 1 ); i++)SECURITEE */
   for (i = 0; i < (C_VOTE_MAX_BYTE  + _C_VOTE_PLACE); i++)
   /* 10.07 >> */
     Carte[_C_DATA_PLACE + i] = ' ';
} /* Initialise_Carte */

/***********************************************/
void Encode_Crc(char *Carte)
{
  int i;

  for (i = 0; i < _C_CRC_MAX_BYTE;
                         Carte[_C_CRC_PLACE + i + 1] = Crc_Calcul[i],i++);
}
/***********************************************/
void Recupere_Crc(char *Carte,int Place)
{
  int i;

  for (i = 0; i < _C_CRC_MAX_BYTE;
                        Crc_Recup[i] = Carte[Place + i], i++);
  Crc_Recup[4] = 0x00;
}
/*******************************************************************/
/*  FIN DU MODULE VOTE.C                                           */
/*******************************************************************/
/* END CONFIDENTIAL */
