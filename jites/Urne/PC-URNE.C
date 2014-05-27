/* START CONFIDENTIAL */

/*******************************************************************/
/*  DEBUT DU MODULE PC-URNE.C                                      */
/*******************************************************************/

#include "cfgscrut.h"

CFGSCRUT  cfgS[NMX_SCRUTIN_T+1] = { {0, 0, 0},
                                {VM_EUROPE,   NMXV_EUROPE,   NMXB_EUROPE},
                                {VM_CHAMBRE,  NMXV_CHAMBRE,  NMXB_CHAMBRE},
                                {VM_SENAT,    NMXV_SENAT,    NMXB_SENAT},
                                {VM_REGION,   NMXV_REGION,   NMXB_REGION},
                                {VM_PROVINCE, NMXV_PROVINCE, NMXB_PROVINCE},
                                {VM_COMMUNE,  NMXV_COMMUNE,  NMXB_COMMUNE},
                                {VM_CONSEIL_FLAMANT, NMXV_CONSEIL_FLAMANT,
                                NMXB_CONSEIL_FLAMANT}
                                };

#include <dos.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "defs.h"
#include "lcd.h"
#include "eprom.h"
#include "variable.h"
#include "erreur.h"
#include "vote.h"
#include "pc-urne.h"
#include "urnbe.h"

/*******************************/
#define  _Blanc_Non_Exp  0
#define  _Blanc_Exp      1
#define  _Vote           2
#define  _Pas_De_Vote    3
/*******************************/

int  TYPE_SCRUTIN[5];
int  BK_PC = 0;
int  SW_EURO = 0;
int  SW_STR  = 0;
/*******************************/

char MULTI_ELEC;

/*******************************/

int Nb_Liste,Nb_Candidat;
char *Buffer_Par,Car,Type_Multi;

char Table_VM[100];  /* Table de stockage des Votes Multiples */
                     /* Table_VM[0] = TETE DE LISTE           */
                     /* Table_[1..n]= AUTRES CANDIDATS        */

struct liste_t *ps, *adrl;

/* << 10.02b */
/* Vote counters */
extern int  *piNOMBRE_VOTES1, *piNOMBRE_VOTES2, *piNOMBRE_VOTES3;
extern int  *piNOMBRE_NATIO_VOTES1, *piNOMBRE_NATIO_VOTES2, *piNOMBRE_NATIO_VOTES3;
extern int  *piNOMBRE_EURO_VOTES1, *piNOMBRE_EURO_VOTES2, *piNOMBRE_EURO_VOTES3;
extern int  *piNOMBRE_STR_VOTES1, *piNOMBRE_STR_VOTES2, *piNOMBRE_STR_VOTES3;
/* 10.02b >> */

/********************** GARNIR TABLE LISTE *********************************/
void Initialise_Variables(char *Buffer)
/*
    Analyse la table provenant du PC de controle, initialise les tables
      contenant les scrutins, les mots de passe, le jeton
    Parametre: chaine de caracteres lue du fichier
    Rend void
*/
{

  char strtmp[4];
  char bidon[16];
  int i, j, k, Type;

/****   Initialisations des votes multiples  ********/

  GLOBAL_EURO = 0;
  GLOBAL_STR  = 0;


  C_VOTE_MAX_BYTE = 0;
  _LECT_CHAINE(JETON_ETENDU,Buffer,k,_JETON_ETENDU_MAX_BYTE);
  _LECT_CHAINE(bidon,Buffer,k,_JETON_ETENDU_MAX_BYTE);
  _LECT_NOMBRE(NOMBRE_SCRUTIN,strtmp,Buffer,k,1);

  for (i = 0; i < NOMBRE_SCRUTIN; i++)
    {
      _LECT_NOMBRE(Type,strtmp,Buffer,k,1);
      TYPE_SCRUTIN[i] = Type;
      if (cfgS[TYPE_SCRUTIN[i]].swVM == 1)
             {
               C_VOTE_MAX_BYTE += (cfgS[TYPE_SCRUTIN[i]].nmxB +2);
             }
      else                                   /*         VOTE            */
               C_VOTE_MAX_BYTE += 6;

      if ((TYPE_SCRUTIN[i] == 6) || (TYPE_SCRUTIN[i] == 5))
                                                    BK_PC++;

      strncpy(strtmp,JETON_ETENDU,3);
      strtmp[3] = 0x00;

      if((TYPE_SCRUTIN[i] == 2) || (TYPE_SCRUTIN[i] == 3))       SW_STR  = 1;
      /* << 10.02b */
      /*if((TYPE_SCRUTIN[i] == 1) || (TYPE_SCRUTIN[i] == 6) || (TYPE_SCRUTIN[i] == 6))*/
      if((TYPE_SCRUTIN[i] == 1) || (TYPE_SCRUTIN[i] == 6))  SW_EURO = 1;
      /* 10.02b >> */

      _LECT_NOMBRE(Nb_Liste,strtmp, Buffer,k,2);
      scrutin[i].Nombre_Listes    = Nb_Liste;
      scrutin[i].Votes_Bl_Exp     = 0;
      scrutin[i].Votes_Bl_Non_Exp = 0;

      ps = (struct liste_t *) malloc(sizeof(struct liste_t)* scrutin[i].Nombre_Listes);
      /* << 10.02 */
	  if(ps == NULL)
	  {
		  Lcd_Affiche(0,0, Message_Point[121]);
		  Lcd_Affiche(1,0, Message_Point[7]);
		  Boucle_Erreur();
	  }
	  /* 10.02 >> */

      scrutin[i].ptlist = ps;
      for(j=0;j < scrutin[i].Nombre_Listes;j++)
      {
       _LECT_NOMBRE(Nb_Candidat,strtmp,Buffer,k,3);
       ps->Nombre_Effectifs = Nb_Candidat;
       _LECT_NOMBRE(Nb_Candidat,strtmp,Buffer,k,2);
       ps->Nombre_Suppleants = Nb_Candidat;
       ps->Votes_Effectifs   = 0;
       ps->Votes_Suppleants  = 0;
       ps->Votes_Tete_Liste  = 0;
       ps->Votes_Liste       = 0;
       for(k = 0; k < 90;k++)
       ps->Votes_Candidats[k] = 0;
       ps++;
     }
    }
 MULTI_ELEC = 'N';
 if((i > 1) && (SW_EURO == 1))            /*  Plusieures Scrutins  */
                   {
                     MULTI_ELEC = 'P';
                     GLOBAL_EURO = 1;
                   }
 if((i > 2) && (SW_STR == 1))             /* Lgislatives + autres */
                   {
                     MULTI_ELEC = 'P';
                     GLOBAL_STR = 1;
                   }
} /* Initialise_Variables */

/**************************************************************************/
int Lit_Fichier_ARR(char *Nom_Fichier)
/*
    Lit le fichier provenant du PC de conbrole
    Parametre: non du fichier
               clef pour decrypter le fichier
    Rend message d'erreur
*/
{
   char Buffer[20];
   int  Numero_Fichier, Erreur, nbcar,k;
   FILE *Fichier_PCC;

   Fichier_PCC = fopen(Nom_Fichier, "rt");
   if (Fichier_PCC == NULL)
     {
       Erreur = _W_FICHIER_PAS_TROUVE;
       return(Erreur);
     }
   Numero_Fichier = fileno(Fichier_PCC);
   nbcar = fread(Buffer, 1, 15, Fichier_PCC);

   if (nbcar != 15)
     {
       Erreur = _W_LECTURE;
       fclose(Fichier_PCC);
       return(Erreur);
     }
   Buffer[15]= 0x00;
   strcpy(JETON_ETENDU,Buffer);
   fclose(Fichier_PCC);
   return(_PAS_ERREUR);
} /* Lit_Fichier_ARR */
/**************************************************************************/
/***********************************************/
int Lit_Fichier_PCC(char *Nom_Fichier)
/*
    Lit le fichier provenant du PC de conbrole
    Parametre: non du fichier
               clef pour decrypter le fichier
    Rend message d'erreur
*/
{
   char *Buffer,*Buffer1,*Buffer2;
   size_t Taille_Fichier;
   int  Numero_Fichier, Erreur, nbcar;
   FILE *Fichier_PCC;


   Fichier_PCC = fopen(Nom_Fichier, "rt");
   if (Fichier_PCC == NULL)
     {
       Erreur = _W_FICHIER_PAS_TROUVE;
       return(Erreur);
     }
   Numero_Fichier = fileno(Fichier_PCC);
   Taille_Fichier = filelength(Numero_Fichier);
   if (Taille_Fichier == -1L)
     {
       Erreur = _W_LECTURE;
       fclose(Fichier_PCC);
       return(Erreur);
     }

   Buffer1= malloc(Taille_Fichier);
   if (Buffer1 == NULL)
     {
       Erreur = _W_MEMOIRE;
       fclose(Fichier_PCC);
       return(Erreur);
     }

   Buffer_Par = Buffer1;
   nbcar = fread(Buffer1, 1, Taille_Fichier, Fichier_PCC);
   /* << 10.02 */
   if(nbcar == 0)
   {
	   Erreur = _W_LECTURE;
	   free(Buffer1);
       fclose(Fichier_PCC);
       return(Erreur);
   }
   /* 10.02 >> */

   Buffer = malloc(Taille_Fichier);
   if (Buffer == NULL)
     {
       /* << 10.02 */
	   free(Buffer1);
       fclose(Fichier_PCC);
	   /* 10.02 >> */
       Erreur = _W_MEMOIRE;
       return(Erreur);
     }

   Buffer2 = Buffer;
   for(;*Buffer1;Buffer1++)
        if(*Buffer1 > 0x0D)
                  *Buffer2++ = *Buffer1;

   fclose(Fichier_PCC);
   Initialise_Date(Buffer);
   Initialise_Variables(Buffer);
   free(Buffer);

   return(_PAS_ERREUR);
} /* Lit_Fichier_PCC */

/**************************************************************************/

/**************************************************************************/

void Additione_Vote(int Scrutin, int Liste, int Eff, int Sup)
/*
     Prend en compte un vote
    Parametre: scrutin du vote
               liste du vote
               effectif du vote  (0: non exprime, 1: tete de liste,
                                  i: candidat - 1)
               suppleant du vote (0: non exprime, i: candidat - 1
                                - nombre d'effectif pour la liste)
    Rend void
*/

{
  int j;

  if(Liste == 0)                                /*     Vote Blanc    */
         {
            if((Car == 'N') ||
             ((TYPE_SCRUTIN[Scrutin] == 1) && (Car == 'E')) ||
             ((TYPE_SCRUTIN[Scrutin] == 6) && (Car == 'E')) ||
             (((TYPE_SCRUTIN[Scrutin] == 2) || (TYPE_SCRUTIN[Scrutin] == 3))&&
             (Car == 'S')))
                                scrutin[Scrutin].Votes_Bl_Non_Exp++;
         }
  else
     if(Liste == 99)                        /*     Vote Blanc    */
           scrutin[Scrutin].Votes_Bl_Exp++;
     else
        if(Liste != 98)                        /*     Vote Blanc    */
         {
             Liste--;
             Eff--;
             Sup--;
             ps = scrutin[Scrutin].ptlist;
             for(j = 0;j < Liste;j++)ps++;
             ps->Votes_Liste++;
             if(Eff == -1)                   /* Suppleant seul  */
                   {
                     ps->Votes_Suppleants++;
                     ps->Votes_Candidats[Sup -1]++;
                    }
             else
                    {
                      if(Eff == 0)                   /* Tete de Liste */
                      ps->Votes_Tete_Liste++;
                      else
                        {
                          ps->Votes_Candidats[Eff -1]++;
                          if(Sup > 0)
                              ps->Votes_Candidats[Sup -1]++;
                         }
                     }
         }
} /* Additione_Vote */

/**************************************************************************/
void Additione_Vote_Multiple(int Scrutin, int Liste,int Vote)
/*
    Prend en compte un vote
    Parametre: scrutin du vote
               liste du vote
               effectif du vote  (0: non exprime, 1: tete de liste,
                                  i: candidat - 1)
               suppleant du vote (0: non exprime, i: candidat - 1
                                - nombre d'effectif pour la liste)
    Rend void
*/

{
  int j,Eff,i,Var_Int, SW;
  int nVeff = 0, nVsup = 0;


if(Vote != _Pas_De_Vote)
  {
   if(Vote == _Blanc_Non_Exp)
            scrutin[Scrutin].Votes_Bl_Non_Exp++;
   else
       if(Vote == _Blanc_Exp)
                scrutin[Scrutin].Votes_Bl_Exp++;
       else
         {
           Liste--;
           ps = scrutin[Scrutin].ptlist;
           for(j = 0;j < Liste;j++)ps++;
           ps->Votes_Liste++;
           SW = 0;

           for(i = 1 ; ((i < 90) && (i <= ps->Nombre_Effectifs)); i++)
             {
              if(Table_VM[i] == '1')
                {
                  Var_Int = i - 1;
                  ps->Votes_Candidats[Var_Int]++;
                  SW = 1; nVeff++;
                 }
             }

           for(i = (1 + ps->Nombre_Effectifs);
            ((i<90) && (i<=(ps->Nombre_Effectifs + ps->Nombre_Suppleants)));i++)
            {
             if(Table_VM[i] == '1')
              {
                Var_Int = i - 1;
                ps->Votes_Candidats[Var_Int]++;
                SW = 1; nVsup++;
              }
            }
           if((Table_VM[0] == '1') && (SW == 0))  /* en regard du Tte de */
                          ps->Votes_Tete_Liste++;  /* Liste uniquement.    */
           if ((nVeff>0) && (nVsup==0)) ps->Votes_Effectifs++;  /* Eff. seul(s) */

           if ((nVsup>0) && (nVeff==0)) ps->Votes_Suppleants++; /* Sup. seul(s) */

         }
  }

} /* Additione_Vote */
/**************************************************************************/

int Ecrit_Table_PCC(char *Nom_Fichier)
/*
    Ecrit la table dans un fichier
    Parametres: nom du fichier
    Rend message d'erreur
*/
{
   int Erreur,j, i, k,Tot1,Tot2;
   long TotVote;
   int liste;
   size_t nbcar;
   char strtmp[12],CTotal[5];
   FILE *Fichier,*Fichier2;

   TotVote = 0;

   /* << 10.02b */
   Check_Vote_Counters();
   /* 10.02b >> */

   Fichier = fopen(Nom_Fichier, "wt");
   if (Fichier == NULL)
     {
       Erreur = _W_CREATION_FICHIER;
       return(Erreur);
     }
   fwrite(JETON_ETENDU, 1, _JETON_ETENDU_MAX_BYTE, Fichier);
   fprintf(Fichier,"\n");
   for (i = 0; i < NOMBRE_SCRUTIN; i++)
     {
         if(GCOLOR ==_ON_GREEN)
              GCOLOR = _ON_RED;
         else
             GCOLOR = _ON_GREEN;
         Light_LED(GCOLOR);

        ps = scrutin[i].ptlist;
        for(j = 0;j < scrutin[i].Nombre_Listes;j++,ps++)
          {
           TotVote += ps->Votes_Liste;
           sprintf(strtmp, "%4d", ps->Votes_Liste);
           nbcar = fwrite(strtmp, 1, 4, Fichier);

           if (nbcar != 4)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }
           TotVote += ps->Votes_Suppleants;
           sprintf(strtmp, "%4d",ps->Votes_Suppleants);
           nbcar = fwrite(strtmp, 1, 4, Fichier);

           if (nbcar != 4)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }
           TotVote += ps->Votes_Tete_Liste;
           sprintf(strtmp, "%4d",ps->Votes_Tete_Liste);
           nbcar = fwrite(strtmp, 1, 4, Fichier);

           if (nbcar != 4)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }
           TotVote += ps->Votes_Effectifs;
           sprintf(strtmp, "%4d",ps->Votes_Effectifs);
           nbcar = fwrite(strtmp, 1, 4, Fichier);

           if (nbcar != 4)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }
           for(k = 0;k < (ps->Nombre_Effectifs + ps->Nombre_Suppleants);k++)
             {
               TotVote += ps->Votes_Candidats[k];
               sprintf(strtmp, "%4d",ps->Votes_Candidats[k]);
               nbcar = fwrite(strtmp, 1, 4, Fichier);

               if (nbcar != 4)
                  {
                    Erreur = _W_ECRITURE;
                    fclose(Fichier);
                    return(Erreur);
                  }
              }
          }
           sprintf(strtmp, "--------");
           nbcar = fwrite(strtmp, 1, 8, Fichier);

           if (nbcar != 8)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }

           sprintf(strtmp, "%4d",scrutin[i].Votes_Bl_Exp);

           nbcar = fwrite(strtmp, 1, 4, Fichier);
           if (nbcar != 4)
            {
             Erreur = _W_ECRITURE;
             fclose(Fichier);
             return(Erreur);
            }

           sprintf(strtmp, "%4d",scrutin[i].Votes_Bl_Non_Exp);
           nbcar = fwrite(strtmp, 1, 4, Fichier);

           if (nbcar != 4)
             {
               Erreur = _W_ECRITURE;
               fclose(Fichier);
               return(Erreur);
             }
     }
  /* << 10.2b */
   /*sprintf(strtmp,"%4d", NOMBRE_NATIO);*/
   sprintf(strtmp,"%4d", *piNOMBRE_NATIO_VOTES1);
  /* 10.02b >> */
   nbcar = fwrite(strtmp, 1, 4, Fichier);
   if (nbcar != 4)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }
  /* << 10.2b */
   /*sprintf(strtmp,"%4d", NOMBRE_EURO);*/
   sprintf(strtmp,"%4d", *piNOMBRE_EURO_VOTES1);
  /* 10.02b >> */
   nbcar = fwrite(strtmp, 1, 4, Fichier);
   if (nbcar != 4)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }
  /* << 10.2b */
   /*sprintf(strtmp,"%4d", NOMBRE_STR);*/
   sprintf(strtmp,"%4d", *piNOMBRE_STR_VOTES1);
  /* 10.02b >> */
   nbcar = fwrite(strtmp, 1, 4, Fichier);
   if (nbcar != 4)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }

   /* << 10.2b */
   /*sprintf(strtmp,"%4d", NOMBRE_VOTES);*/
   sprintf(strtmp,"%4d", *piNOMBRE_VOTES1);
   /* 10.02b >> */
   nbcar = fwrite(strtmp, 1, 4, Fichier);
   if (nbcar != 4)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }

   sprintf(strtmp,"%8ld", TotVote); /* Total Votes de Prfrences */
   nbcar = fwrite(strtmp, 1, 8, Fichier);

   if (nbcar != 8)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }
    sprintf(strtmp, " ");
    nbcar = fwrite(strtmp, 1, 1, Fichier);

   if (nbcar != 1)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }
   Tot1 = atoi(Minicodage);
/* << 10.02e */
   /*CTotal[0] = fullPassword[0];
   CTotal[1] = fullPassword[2];
   CTotal[2] = fullPassword[4];
   CTotal[3] = fullPassword[6];*/
   CTotal[0] = fullPassword[MINI_POS[0]-49];
   CTotal[1] = fullPassword[MINI_POS[1]-49];
   CTotal[2] = fullPassword[MINI_POS[2]-49];
   CTotal[3] = fullPassword[MINI_POS[3]-49];
/* 10.02e >> */
   CTotal[4] = 0x00;
   Tot2 = atoi(CTotal);

   sprintf(strtmp,"%5d", Tot1 + Tot2); /* Total calculable */
   nbcar = fwrite(strtmp, 1, 5, Fichier);

   if (nbcar != 5)
     {
       Erreur = _W_ECRITURE;
       fclose(Fichier);
       return(Erreur);
     }
   fclose(Fichier);
   return(_PAS_ERREUR);
}
 /* Ecrit_Table_PCC */
/****************************************************************************/
/* << 10.02b */
/*
    <FUNCTION>
		<NAME>Init_Listes</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Initializes the list parameters
		</DESCRIPTION>
        <PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE></TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Init_Listes()
{
     int j, i, k;

    for (i = 0; i < NOMBRE_SCRUTIN; i++)
    {
         if(GCOLOR ==_ON_GREEN)
              GCOLOR = _ON_RED;
         else
             GCOLOR = _ON_GREEN;
         Light_LED(GCOLOR);

        ps = scrutin[i].ptlist;
        for(j = 0;j < scrutin[i].Nombre_Listes;j++,ps++)
        {
           ps->Votes_Liste = 0;
           ps->Votes_Suppleants = 0;
           ps->Votes_Tete_Liste = 0;
           ps->Votes_Effectifs = 0;

           for(k = 0;k < (ps->Nombre_Effectifs + ps->Nombre_Suppleants);k++)
           {
               ps->Votes_Candidats[k] = 0;
           }
        }

        scrutin[i].Votes_Bl_Exp = 0;
        scrutin[i].Votes_Bl_Non_Exp = 0;
    }
}
/****************************************************************************/
/* 10.02b >> */
void Transfert_Table_PCC(char *Nom_Fichier)
/*
   Cree la table fournie au PC de controle
   Parametres: nom du fichier
   Rend void
*/
{
  unsigned int Position, Parite;
  int Erreur, Numero, Liste, Eff, Sup,Param;
  char Carte_Tmp[76],Nul_StrL[3],Nul_StrE[3],Nul_StrS[3];
  FILE *Fichier;
  int  i,Compt,CI,j,Var_Int;
  char strtmp[21],strtmp3[65];
  /* << 10.03c */
  char szPassword[16];
  unsigned int iVoteSize;
  /* 10.03c >> */

  Nul_StrL[2] = 0x00;
  Nul_StrE[2] = 0x00;
  Nul_StrS[2] = 0x00;

  /* << 10.02b */
  Init_Listes();
  /*NOMBRE_VOTES = 0;*/
  Init_Vote_Counters();
  /* 10.02b >> */

  for (Position = 0; Position < _E_NB_MAX_VOTE; Position++)
   {
     Car = Est_Valide_E_Vote(Position);
    if (Car)
     {
       /* << 10.02b */
       /*NOMBRE_VOTES++;*/
       /* 10.02b >> */

       /* << 10.02f */
       if(GCOLOR ==_ON_GREEN)
            GCOLOR = _ON_RED;
       else
            GCOLOR = _ON_GREEN;
       Light_LED(GCOLOR);
       /* 10.02f >> */

       /* << 10.03c */
       /*Recupere_E_Vote(Position,Carte_Tmp,C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE +_C_TEST_MAX_BYTE);*/
       iVoteSize = C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE;
       Recupere_E_Vote(Position,Carte_Tmp,iVoteSize);

	   Generate_Password(szPassword,Position,true);
	   Encrypt_Decrypt(Carte_Tmp,szPassword,iVoteSize);
	   /* 10.03c >> */

       strncpy(strtmp3,&Carte_Tmp[_C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE ],C_VOTE_MAX_BYTE);
       strtmp3[C_VOTE_MAX_BYTE]= 0x00;

       Calcul_Crc(Carte_Tmp,_C_CRC_MAX_BYTE);
       Recupere_Crc(Carte_Tmp,0);
       if (strcmp(Crc_Calcul,Crc_Recup)!= 0)
         {
             /* << 10.02b */
             /*Lcd_Affiche(0,0,Message_Point[2]);*/
             Lcd_Affiche(0,0,Message_Point[27]);
             /* 10.02b >> */
             Lcd_Affiche(1,0,Message_Point[7]);
             Boucle_Erreur();
         }

       /* << 10.02b */
       *piNOMBRE_VOTES1 += 1;
       *piNOMBRE_VOTES2 += 1;
       *piNOMBRE_VOTES3 += 1;

       if(Car == 'N')
       {
            *piNOMBRE_NATIO_VOTES1 += 1;
            *piNOMBRE_NATIO_VOTES2 += 1;
            *piNOMBRE_NATIO_VOTES3 += 1;
       }
       else if(Car == 'E')
       {
            *piNOMBRE_EURO_VOTES1 += 1;
            *piNOMBRE_EURO_VOTES2 += 1;
            *piNOMBRE_EURO_VOTES3 += 1;
       }
       else if(Car == 'S')
       {
            *piNOMBRE_STR_VOTES1 += 1;
            *piNOMBRE_STR_VOTES2 += 1;
            *piNOMBRE_STR_VOTES3 += 1;
       }
       /* 10.02b >> */

       strcpy(Carte_Tmp,strtmp3);
       for (Compt = 0, i = 0; i < NOMBRE_SCRUTIN; i++)
         {
          if (cfgS[TYPE_SCRUTIN[i]].swVM == 0)
           {
             strncpy(Nul_StrL,&Carte_Tmp[Compt],2);
             Liste = atoi(Nul_StrL);
             Compt +=2;
             strncpy(Nul_StrE,&Carte_Tmp[Compt],2);
             Eff = atoi(Nul_StrE);
             Compt +=2;
             strncpy(Nul_StrS,&Carte_Tmp[Compt],2);
             Sup = atoi(Nul_StrS);
             Compt +=2;
             Additione_Vote(i, Liste, Eff, Sup);
           }
          else
           {
            strncpy(Nul_StrL,&Carte_Tmp[Compt],2);
            Liste = atoi(Nul_StrL);
            if(Liste == 0)
              {
               if((Car == 'N') ||
                ((TYPE_SCRUTIN[i] == 1) && (Car == 'E')) ||
                 ((TYPE_SCRUTIN[i] == 6) && (Car == 'E')) ||
                  (((TYPE_SCRUTIN[i] == 2) || (TYPE_SCRUTIN[i] == 3))
                  && (Car == 'S')))
                        {
                           Param = _Blanc_Non_Exp;
                        }
               else
                        {
                           Param = _Pas_De_Vote;
                        }
              }
            else
                if(Liste == 99)
                        Param = _Blanc_Exp;
                else
                  if(Liste == 98)
                        Param = _Pas_De_Vote;
                  else
                        Param = _Vote;
            Compt +=2;
            for(CI = 0; (CI < cfgS[TYPE_SCRUTIN[i]].nmxV); Compt++)
                 {
                  switch(Carte_Tmp[Compt])
                   {

                    case '1' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               break;
                    case '2' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               break;
                    case '3' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               break;
                    case '4' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               break;
                    case '5' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               break;
                    case '6' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               break;
                    case '7' : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               break;
                    case '8' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               break;
                    case '9' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               break;
                    case 'A' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               break;
                    case 'B' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               break;
                    case 'C' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               break;
                    case 'D' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '1';
                               break;
                    case 'E' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '0';
                               break;
                    case 'F' : Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               Table_VM[CI++] = '1';
                               break;
                    default  : Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                               Table_VM[CI++] = '0';
                   }
                 }
               Additione_Vote_Multiple(i, Liste,Param);
             }
          }
     }
   }
    while ((Ecrit_Table_PCC(Nom_Fichier)) != _PAS_ERREUR)
    {
         /* << 10.02b */
         /*Lcd_Affiche(0,0, Message_Point[25]);*/
         Lcd_Affiche(0,0, Message_Point[122]);
         /* 10.02b >> */
         Lcd_Affiche(1,0, Message_Point[14]);
         /* << 10.03f */
         /*Getch(COLOR);*/
         Getchar(COLOR,Message_Point[84],Message_Point[14],1,0);
        /* 10.03f >> */
         Lcd_Affiche(0,0, Message_Point[12]);
         Lcd_Affiche(1,0," ");
    }
} /* Tansfert_Table_PCC */
/***************************************************************************/
int Choix_Langue(int choix)
{
  char bufm[40], strtmp[20];
  int status ,i, Taille,j;
  FILE *fpm;

  if ( choix == 1)
     {
       while((fpm = fopen("a:MSG_1","rt")) == NULL)
            {
              Lcd_Affiche(0,0,"MESSAGE ERROR OTHER");
              Lcd_Affiche(1,0,"DISK AND PRESS KEY");
              Getch(COLOR);
              Lcd_Affiche(0,0," A MOMENT PLEASE");
              Lcd_Affiche(1,0," ");
            }
       i = 0;
       while((fgets(bufm,30,fpm) != NULL))
            {
              bufm[24] = 0x00;
              for(j=23;(j > 4) && (bufm[j] < 32);)
                 {
                   bufm[j] = 0x20;
                   j--;
                 }
              strncpy(Message_Point[i],bufm + 4,20);
              i++;
            }
       fclose(fpm);
       return(0);
     }
  else
    {
       strncpy(strtmp,JETON_ETENDU,3);
       strtmp[3] = 0x00;
       if((strcmp (strtmp,"002")) == 0)
          {
             status = Saisie_Langue();
             if(!status)
               {
                  if((fpm = fopen("a:MSG_2","rt")) !=NULL)
                    {
                       i = 0;
                       while((fgets(bufm,30,fpm)!= NULL))
                           {
                              bufm[24] = 0x00;
                              for(j=23;(j > 4) && (bufm[j] < 32 );)
                                  {
                                    bufm[j] = 0x20;
                                    j--;
                                  }
                              strncpy(Message_Point[i],bufm + 4,20);
                              i++;
                           }
                       fclose(fpm);
                    }
               }
          }
        return(0);
    }
}
/****************************************************************************/
int Verifie_E_Vote(void)
/*
   Rend int
*/
{
  int E_Position, E_scrutin, E_list;
  char C_list[3];

  for( E_scrutin = 0, E_Position = (_C_CRC_MAX_BYTE +_C_TEST_MAX_BYTE); E_scrutin < NOMBRE_SCRUTIN;E_scrutin++)
    {
     /* << 10.04 */
     /*strncpy(C_list,&Strtmp[E_Position] ,2);*/
     strncpy(C_list,&szVoteContents[E_Position] ,2);
     /* 10.04 >> */
     C_list[2] = 0x00;
     E_list = atoi(C_list);
     if ((E_list < 98) && ( E_list >  scrutin[E_scrutin].Nombre_Listes))
         return(1);
     E_Position += cfgS[TYPE_SCRUTIN[E_scrutin]].nmxB + 2;
    }
 return(0);
}
/* END CONFIDENTIAL */
