/* START CONFIDENTIAL */

/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE VARIABLE.C                                     */
/*******************************************************************/

#include <ctype.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "eprom.h"
#include "variable.h"
#include "lcd.h"
#include "vote.h"
/***********************************************/

char FERMETURE;
char SUPPORT;
char OUVERTURE;
char TEST;
char RECOMPTAGE;
char NC;
/* << 10.02b */

/*int  NOMBRE_VOTES;*/
int  NOMBRE_VAL;
/*int  NOMBRE_NATIO,NOMBRE_EURO,NOMBRE_STR;*/
int  NOMBRE_NATIO_VAL,NOMBRE_EURO_VAL,NOMBRE_STR_VAL;

/* Vote counters */
int  *piNOMBRE_VOTES1, *piNOMBRE_VOTES2, *piNOMBRE_VOTES3;
int  *piNOMBRE_NATIO_VOTES1, *piNOMBRE_NATIO_VOTES2, *piNOMBRE_NATIO_VOTES3;
int  *piNOMBRE_EURO_VOTES1, *piNOMBRE_EURO_VOTES2, *piNOMBRE_EURO_VOTES3;
int  *piNOMBRE_STR_VOTES1, *piNOMBRE_STR_VOTES2, *piNOMBRE_STR_VOTES3;

int  *piERROR_COUNT_VOTE;
/* 10.02b >> */
int  GLOBAL_NATIO,GLOBAL_EURO,GLOBAL_STR;
int  DEMANDE_FERMETURE;
char JETON_ETENDU[_JETON_ETENDU_MAX_BYTE+1];
char JETON[_JETON_MAX_BYTE+1];
char CLEF_URNE[_CLEF_MAX_BYTE+1];
char CLEF_URNE_BIDON[_CLEF_MAX_BYTE+1];
int  USER;
int  POSITION_VOTE;
int  FILE_EPROM;
int  SAUVE_TO_EPROM;
int  NOMBRE_SCRUTIN;
int  PC_LISTE_BLANC[_NB_MAX_SCRUTIN];
int  *TABLE_PCC;
char strtmp[12];

unsigned int timing;
unsigned int EPROM_SEG;

struct Liste_T LISTE[_NB_MAX_LISTE];
size_t TAILLE_TABLE_PCC;

/* << 10.03g */
extern int iDiskId;
/* 10.03g >> */

/***********************************************/

void Encode_E_Variable(long Position, char Valeur)
/*
   Modifie la valeur des variables FERMETURE, OUVERTURE
   Parametres: position de la variable dans l'EPROM
               valeur a ecrire dans la variable
   Rend void
*/
{


  if ((Position == _E_FERMETURE) || (Position == _E_OUVERTURE))
                                 {
                                   FEram = open(Chemin_R, O_RDWR | O_BINARY);
                                   Ram_Write(Valeur, Position);
                                   close(FEram);

                                   if(SUPPORT == 'E')
                                         Eprom_Write(Valeur, Position);
                                 }
}  /* Encode_E_Variable */

/***********************************************/

void Recupere_E_Variable(long Position, char *Valeur)
/*
   Recupere la valeur des variables FERMETURE OUVERTURE
   Parametres: position de la variable dans l'EPROM
               valeur recuperee de la variable
   Rend void
*/
{
  int i;


  if ((Position == _E_FERMETURE) || (Position == _E_OUVERTURE))
                               {
                                 if(SUPPORT == 'E')
                                        Eprom_Read(Valeur, Position);

                                 if(SUPPORT == 'R')
                                    {
                                        FEram = open(Chemin_R, O_RDWR | O_BINARY);
                                        Ram_Read(Valeur, Position);
                                        close(FEram);
                                    }
                               }
} /* Recupere_E_Variable */

/***********************************************/

void Encode_E_Var_String(long Position, char *Valeur, int Longueur)
/*
   Modifie la valeur d'une variable de type string
   Parametres: position du string dans l'EPROM
               valeur du string
               longueur du string
   Rend void
*/
{
  int i, status, iNum;
  char *Valeur1;
  long Position1;

  Valeur1   = Valeur;
  Position1 = Position;

         FEram = open(Chemin_R, O_RDWR | O_BINARY);
         for (i = 0; i < Longueur; i++)
                Ram_Write(*Valeur1++, Position1++);
         close(FEram);

         if(SUPPORT == 'E')
           for (i = 0, status = true;(i < Longueur) && (status == true); i++)
                {
                  status = Eprom_Write(*Valeur++, Position++);
                }
         else
              {
                 FEdsk = open(Chemin_D, O_RDWR | O_BINARY);
                 for (i = 0, status = true;(i < Longueur) && (status == true); i++)
                       {
                          status = Dsk_Write(*Valeur++, Position++);
                       }
                 close(FEdsk);
                 if(status != true)
                       {
                         Change_Disque(Message_Point[19]);
                         /* << 10.03g */
                         iNum = Disk_Num() ;
                         iDiskId = Check_NumDisk(iNum);
                         /* 10.03g >> */
                         Ram_Disk_Load();
                        }
              }

}  /* Encode_E_Var_String */

/***********************************************/

void Recupere_E_Var_String(long Position, char *Valeur, int Longueur)
/*
   Recupere la valeur d'une variable de type string
   Parametre: position du string dans l'EPROM
              valeur recuperee de la variable
              longueur du string
   Rend void
*/
{
  int i;
  long Position1;

  if(SUPPORT == 'R')
    {
      FEram = open(Chemin_R, O_RDWR | O_BINARY);
      for (i = 0; i < Longueur; i++)
               Ram_Read(Valeur++, Position++);
      close(FEram);
    }
  else
    for (i = 0; (i < Longueur) && (SUPPORT != 'R'); i++)
                         Eprom_Read(Valeur++, Position++);

  *Valeur = '\0';

}  /* Recupere_E_Var_String */

/*****************************************************************************/
void Recupere_E_Vote(int Position, char *Valeur, int Longueur)
/*
    Recupere la valeur d'un vote
    Parametre: nombre associe a un vote
                 (entree dans la table allant de 1 … 2500)
               information du vote
               Numero : numero du vote repris de la carte
    Rend void
*/
{
  long E_Position;


  E_Position = (long)_E_TABLE;
  E_Position +=(long)((long)((long)C_VOTE_MAX_BYTE + 5L) * (long)Position);
  Recupere_E_Var_String(E_Position, Valeur, Longueur);

} /* Recupere_E_Vote */

/***********************************************/

void Encode_E_Vote(int Position, char *Valeur, int Longueur)
/*
   Sauvegarde le contenu d'un vote
   Parametres: numero associe au vote
               informations du vote
               Numero repris de la carte
   Rend void
*/
{
  long E_Position;


  E_Position = (long)_E_TABLE;
  E_Position +=(long)((long)((long)C_VOTE_MAX_BYTE + 5L) * (long)Position);
  Encode_E_Var_String(E_Position, Valeur, Longueur);
} /* Encode_E_Vote */

/***********************************************/
boolean Est_Valide_E_Vote(int Position)
/*
   Determine si un vote a ete valide
   Parametre: numero associe au vote
   Rend la valeur du test de validite
*/
{
  long E_Position;
  unsigned char Valide;
  int FEram2;
  /* << 10.03c */
  char szTempVote[76], szPassword[16];
  unsigned int iVoteSize;
  /* 10.03c >> */

  E_Position = (long)_E_TABLE + _C_CRC_MAX_BYTE;
  E_Position +=(long)((long)((long)C_VOTE_MAX_BYTE + 5L) * (long)Position);

  if(SUPPORT == 'E')
        Eprom_Read(&Valide, E_Position);

  if(SUPPORT == 'R')
    {
      FEram2 = open(Chemin_R, O_RDWR | O_BINARY);
      Ram_Read(&Valide, E_Position);
      close(FEram2);
    }

  /* << 10.03c */
  if(Valide)
  {
	iVoteSize = C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE;
	Recupere_E_Vote(Position,szTempVote,iVoteSize);
	Generate_Password(szPassword,Position,true);
	Encrypt_Decrypt(szTempVote,szPassword,iVoteSize);
	Valide = szTempVote[_C_CRC_MAX_BYTE];
  }
  /* 10.03c >> */

  return(Valide);

} /* Est_Valide_E_Vote */

/***********************************************/
boolean Est_Valide_E_Val(int Position)
/*
   Determine si une validation existe
   Parametre: numero associe a la validation
   Rend la valeur du test de validite
*/
{
  long E_Position;
  unsigned char Valide;
  int FEram2;


  E_Position = (long)_E_TABLE_VAL;
  E_Position +=(long)((long)((long)1L) * (long)Position);

  if(SUPPORT == 'E')
        Eprom_Read(&Valide, E_Position);

  if(SUPPORT == 'R')
    {
      FEram2 = open(Chemin_R, O_RDWR | O_BINARY);
      Ram_Read(&Valide, E_Position);
      close(FEram2);
    }

  return(Valide);

} /* Est_Valide_E_Vote */
/***********************************************/
void Valide_E_Val(int Position)
/*
   Valide une validation
   Parametre: numero associe a la validation
   Rend void
*/
{
  long E_Position;
  int status, iNum;

  E_Position = (long)_E_TABLE_VAL;
  E_Position +=(long)((long)((long)1L) * (long)Position);

  FEram = open(Chemin_R, O_RDWR | O_BINARY);
  Ram_Write(Type_Multi, E_Position);
  close(FEram);

  if(SUPPORT == 'E')
       Eprom_Write(Type_Multi, E_Position);
  else
     {
       FEdsk = open(Chemin_D, O_RDWR | O_BINARY);
       status = Dsk_Write(Type_Multi, E_Position);
       close(FEdsk);
       if(status != true)
                {
                   Change_Disque(Message_Point[19]);
                   /* << 10.03g */
                   iNum = Disk_Num() ;
                   iDiskId = Check_NumDisk(iNum);
                   /* 10.03g >> */
                   Ram_Disk_Load();
                 }
     }

} /* Valide_E_Val */
/***********************************************/
boolean Est_Libre_E_Position(int Position)
/*
   Determine si une entree de la table des votes est disponible
   Parametre: entree de la table
   Rend la valeur du test de disponibilite
*/
{
  long E_Position;
  char c;

  E_Position = (long)_E_TABLE;
  E_Position +=(long)((long)((long)C_VOTE_MAX_BYTE + 5L) * (long)Position);

  if(SUPPORT == 'E')
       c = (! Eprom_Writed(E_Position));

  else
       c = (! Ram_Writed(E_Position));
  return(c);

} /* Est_Libre_E_Position */

/****************************************************************************/
/****************************************************************************/
boolean Est_Libre_E_Position_Val(int Position)
/*
   Determine si une entree de la table des votes est disponible
   Parametre: entree de la table
   Rend la valeur du test de disponibilite
*/
{
  long E_Position;
  char c;

  E_Position = (long)_E_TABLE_VAL;
  E_Position +=(long)((long)((long)1L) * (long)Position);

  if(SUPPORT == 'E')
       c = (! Eprom_Writed(E_Position));
  else
       c = (! Ram_Writed(E_Position));
  return(c);

} /* Est_Libre_E_Position_Val */

/****************************************************************************/
/* << 10.02b  */
/*
   <FUNCTION>
		<NAME>Alloc_Mem</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Carry out the memory allocation of pointers
		</DESCRIPTION>
        <PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>boolean</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/

boolean Alloc_Mem()
{
   if ((piNOMBRE_VOTES1 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_VOTES2 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_VOTES3 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_NATIO_VOTES1 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_NATIO_VOTES2 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_NATIO_VOTES3 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_EURO_VOTES1  = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_EURO_VOTES2  = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_EURO_VOTES3  = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_STR_VOTES1 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_STR_VOTES2 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piNOMBRE_STR_VOTES3 = (int *)malloc(sizeof(int))) == NULL)
      return(0);

   if ((piERROR_COUNT_VOTE = (int *)malloc(sizeof(int))) != NULL)
     *piERROR_COUNT_VOTE = 0;
   else
      return(0);


   return(1);
}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Free_Mem</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		deallocates a memory block allocated
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

void Free_Mem()
{
   if (piNOMBRE_VOTES1)
      free(piNOMBRE_VOTES1);

   if (piNOMBRE_VOTES2)
      free(piNOMBRE_VOTES2);

   if (piNOMBRE_VOTES3)
      free(piNOMBRE_VOTES3);

   if (piNOMBRE_NATIO_VOTES1)
      free(piNOMBRE_NATIO_VOTES1);

   if (piNOMBRE_NATIO_VOTES2)
      free(piNOMBRE_NATIO_VOTES2);

   if (piNOMBRE_NATIO_VOTES3)
      free(piNOMBRE_NATIO_VOTES3);

   if (piNOMBRE_EURO_VOTES1)
      free(piNOMBRE_EURO_VOTES1);

   if (piNOMBRE_EURO_VOTES2)
      free(piNOMBRE_EURO_VOTES2);

   if (piNOMBRE_EURO_VOTES3)
      free(piNOMBRE_EURO_VOTES3);

   if (piNOMBRE_STR_VOTES1)
      free(piNOMBRE_STR_VOTES1);

   if (piNOMBRE_STR_VOTES2)
      free(piNOMBRE_STR_VOTES2);

   if (piNOMBRE_STR_VOTES3)
      free(piNOMBRE_STR_VOTES3);

   if (piERROR_COUNT_VOTE)
      free(piERROR_COUNT_VOTE);


}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Init_Vote_Counters</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Initializes vote counters
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

void Init_Vote_Counters()
{
     *piNOMBRE_VOTES1 = 0;
     *piNOMBRE_VOTES2 = 0;
     *piNOMBRE_VOTES3 = 0;

     *piNOMBRE_NATIO_VOTES1 = 0;
     *piNOMBRE_NATIO_VOTES2 = 0;
     *piNOMBRE_NATIO_VOTES3 = 0;

     *piNOMBRE_EURO_VOTES1 = 0;
     *piNOMBRE_EURO_VOTES2 = 0;
     *piNOMBRE_EURO_VOTES3 = 0;

     *piNOMBRE_STR_VOTES1 = 0;
     *piNOMBRE_STR_VOTES2 = 0;
     *piNOMBRE_STR_VOTES3 = 0;

}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Check_Vote_Counters</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>Check vote counters</DESCRIPTION>
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

void Check_Vote_Counters()
{
   
    short int sIndic1, sIndic2, sIndic3, sIndic4 , sIndic5;

    if ((*piNOMBRE_VOTES1 == *piNOMBRE_VOTES2) && (*piNOMBRE_VOTES2 == *piNOMBRE_VOTES3))
    {
        sIndic1 = 1;
    }
    else
    {
        sIndic1 = 0;

       if (*piNOMBRE_VOTES1 == *piNOMBRE_VOTES2)
       {
          *piNOMBRE_VOTES3 = *piNOMBRE_VOTES1;
          sIndic1 = 1;
       }
       else  if (*piNOMBRE_VOTES1 == *piNOMBRE_VOTES3)
       {
          *piNOMBRE_VOTES2 = *piNOMBRE_VOTES1;
          sIndic1 = 1;
       }
       else  if (*piNOMBRE_VOTES2 == *piNOMBRE_VOTES3)
       {
          *piNOMBRE_VOTES1 = *piNOMBRE_VOTES2;
          sIndic1 = 1;
       }
       else
       {
            *piERROR_COUNT_VOTE += 1;
       }
    }

    if ((*piNOMBRE_NATIO_VOTES1 == *piNOMBRE_NATIO_VOTES2) && (*piNOMBRE_NATIO_VOTES2 == *piNOMBRE_NATIO_VOTES3))
    {
        sIndic2 = 1;
    }
    else
    {
        sIndic2 = 0;

       if (*piNOMBRE_NATIO_VOTES1 == *piNOMBRE_NATIO_VOTES2)
       {
          *piNOMBRE_NATIO_VOTES3 = *piNOMBRE_NATIO_VOTES1;
          sIndic2 = 1;
       }
       else  if (*piNOMBRE_NATIO_VOTES1 == *piNOMBRE_NATIO_VOTES3)
       {
          *piNOMBRE_NATIO_VOTES2 = *piNOMBRE_NATIO_VOTES1;
          sIndic2 = 1;
       }
       else if (*piNOMBRE_NATIO_VOTES2 == *piNOMBRE_NATIO_VOTES3)
       {
          *piNOMBRE_NATIO_VOTES1 = *piNOMBRE_NATIO_VOTES2;
          sIndic2 = 1;
       }
       else
       {
            *piERROR_COUNT_VOTE += 1;
       }

    }


    if(MULTI_ELEC == 'P')
	{
		if ((*piNOMBRE_EURO_VOTES1 == *piNOMBRE_EURO_VOTES2) && (*piNOMBRE_EURO_VOTES2 == *piNOMBRE_EURO_VOTES3))
        {
            sIndic3 = 1;
        }
        else
        {
            sIndic3 = 0;

           if (*piNOMBRE_EURO_VOTES1 == *piNOMBRE_EURO_VOTES2)
           {
              *piNOMBRE_EURO_VOTES3 = *piNOMBRE_EURO_VOTES1;
              sIndic3 = 1;
           }
           else  if (*piNOMBRE_EURO_VOTES1 == *piNOMBRE_EURO_VOTES3)
           {
              *piNOMBRE_EURO_VOTES2 = *piNOMBRE_EURO_VOTES1;
              sIndic3 = 1;
           }
           else if (*piNOMBRE_EURO_VOTES2 == *piNOMBRE_EURO_VOTES3)
           {
              *piNOMBRE_EURO_VOTES1 = *piNOMBRE_EURO_VOTES2;
              sIndic3 = 1;
           }
           else
           {
                *piERROR_COUNT_VOTE += 1;
           }
        }

		if ((*piNOMBRE_STR_VOTES1 == *piNOMBRE_STR_VOTES2) && (*piNOMBRE_STR_VOTES2 == *piNOMBRE_STR_VOTES3))
        {
            sIndic4 = 1;
        }
        else
        {
            sIndic4 = 0;

           if (*piNOMBRE_STR_VOTES1 == *piNOMBRE_STR_VOTES2)
		   {
              *piNOMBRE_STR_VOTES3 = *piNOMBRE_STR_VOTES1;
              sIndic4 = 1;
		   }
		   else
		   if (*piNOMBRE_STR_VOTES1 == *piNOMBRE_STR_VOTES3)
		   {
              *piNOMBRE_STR_VOTES2 = *piNOMBRE_STR_VOTES1;
              sIndic4 = 1;
		   }
		   else
		   if (*piNOMBRE_STR_VOTES2 == *piNOMBRE_STR_VOTES3)
		   {
              *piNOMBRE_STR_VOTES1 = *piNOMBRE_STR_VOTES2;
              sIndic4 = 1;
		   }
		   else
		   {
				*piERROR_COUNT_VOTE += 1;
		   }
        }

		if(*piNOMBRE_VOTES1 == (*piNOMBRE_NATIO_VOTES1 + *piNOMBRE_EURO_VOTES1 + *piNOMBRE_STR_VOTES1))
		{
			sIndic5 = 1;
		}
		else
		{
			sIndic5 = 0;
			*piERROR_COUNT_VOTE += 1;
		}
	}
	else
	{
		sIndic3 = 1;
		sIndic4 = 1;

		if(*piNOMBRE_VOTES1 == *piNOMBRE_NATIO_VOTES1)
		{
			sIndic5 = 1;
		}
		else
		{
			sIndic5 = 0;
			*piERROR_COUNT_VOTE += 1;
		}

	}

    if ((sIndic1 == 0) || (sIndic2 == 0) || (sIndic3 == 0) || (sIndic4 == 0) || (sIndic5 == 0))
    {
		Lcd_Affiche(0, 0, Message_Point[12]);
		Recupere_Nb_Cartes();
		Lcd_Affiche(0, 0, "");
    }


}
/****************************************************************************/
void Recupere_Nb_Cartes()
/*
   Calcule le nombre de votes (valides )
   Parametre: adresse du nombre de cartes
   Rend void
*/
{
  int Position, i;
 

  for (i = 0; i < _E_NB_MAX_VOTE;i++)
                        Table[i] = 0;

  FEram = open(Chemin_R, O_RDWR | O_BINARY);
  /* << 10.02b */
  Init_Vote_Counters();
  /*for (NOMBRE_NATIO = 0,NOMBRE_EURO = 0,NOMBRE_STR = 0,NOMBRE_VOTES = 0,Position = 0; Position < _E_NB_MAX_VOTE; Position++)*/
  for (Position = 0; Position < _E_NB_MAX_VOTE; Position++)
  /* 10.02b >> */
     if (!Est_Libre_E_Position(Position))
     {
        /* << 10.02b */
          *piNOMBRE_VOTES1 += 1;
          *piNOMBRE_VOTES2 += 1;
          *piNOMBRE_VOTES3 += 1;
        /* 10.02b >> */

          Table[Position] = 1;
          if(Est_Valide_E_Vote(Position) == 'N')
          /* << 10.02b */
          {
                /*NOMBRE_NATIO++;*/
                *piNOMBRE_NATIO_VOTES1 += 1;
                *piNOMBRE_NATIO_VOTES2 += 1;
                *piNOMBRE_NATIO_VOTES3 += 1;
          }
          /* 10.02b >> */
          else if(Est_Valide_E_Vote(Position) == 'E')
         /* << 10.02b */
          {
                /*NOMBRE_EURO++;*/
                *piNOMBRE_EURO_VOTES1 += 1;
                *piNOMBRE_EURO_VOTES2 += 1;
                *piNOMBRE_EURO_VOTES3 += 1;
          }
          /* 10.02b >> */
          else if(Est_Valide_E_Vote(Position) == 'S')
          /* << 10.02b */
          {
                /*NOMBRE_STR++;*/
                *piNOMBRE_STR_VOTES1 += 1;
                *piNOMBRE_STR_VOTES2 += 1;
                *piNOMBRE_STR_VOTES3 += 1;
          }
          /* 10.02b >> */

     }
  close(FEram);
} /* Recupre_Nb_Cartes */
/****************************************************************************/
void Recupere_Nb_Validations()
/*
   Calcule le nombre de validations
*/
{
  int Position, i;

  for (i = 0; i < _E_TABLE_VAL_PLACE;i++)
                        Table_Val[i] = 0;

  FEram = open(Chemin_R, O_RDWR | O_BINARY);

  for (NOMBRE_NATIO_VAL = 0,NOMBRE_EURO_VAL = 0, NOMBRE_STR_VAL = 0,
                NOMBRE_VAL = 0,
                Position = 0; Position < _E_TABLE_VAL_PLACE; Position++)
    if (!Est_Libre_E_Position_Val(Position))
      {
          
		   NOMBRE_VAL++;

           Table_Val[Position] = 1;
           if(Est_Valide_E_Val(Position) == 'N')
		   {
                NOMBRE_NATIO_VAL++;
		   }
           else if(Est_Valide_E_Val(Position) == 'E')
		   {
                NOMBRE_EURO_VAL++;
		   }
           else if(Est_Valide_E_Val(Position) == 'S')
		   {
                NOMBRE_STR_VAL++;
		   }

		  

      }

  close(FEram);

} /* Recupre_Nb_Validations */
/****************************************************************************/
/* << 10.03c */
/*
   <FUNCTION>
		<NAME>Encrypt_Decrypt</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>Encrypt or decrypt vote contents</DESCRIPTION>
        <PARAMETER>
			<NAME>pzInputData,pzPassword,iSize</NAME>
			<TYPE>char *,char *,unsigned int</TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>void</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Encrypt_Decrypt(char *pzInputData, char *pzPassword, unsigned int iSize)
{
	unsigned int i, iKeySize;

	iKeySize = strlen(pzPassword);

	for (i= 0; i < iSize; i++)
    {
		pzInputData[i] ^=  pzPassword[i % iKeySize];
    }
    pzInputData[iSize] = 0x00;
}
/****************************************************************************/
/* << 10.04 */
int Encrypt_Vote(char *pzInputData, char *pzPassword, unsigned int iSize)
{
	unsigned int i, iKeySize;
	unsigned char car;

	iKeySize = strlen(pzPassword);

	for (i= 0; i < iSize; i++)
    {
		pzInputData[i] ^=  pzPassword[i % iKeySize];
		car = pzInputData[i];
		if(car == 0xFF)
            return false;
    }
    pzInputData[iSize] = 0x00;

    return true;
}
/* << 10.04 */
/****************************************************************************/

/*
   <FUNCTION>
		<NAME>Generate_Password</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>Generate password to encrypt or decrypt vote contents</DESCRIPTION>
        <PARAMETER>
			<NAME>pzPassword,Position,bIndic</NAME>
			<TYPE>char *,long,boolean</TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>void</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Generate_Password(char *pzPassword, long Position, boolean bIndic)
{
	long E_Position;
	char szPos[8];

	if(bIndic)
	{
		E_Position = (long)_E_TABLE;
		E_Position +=(long)((long)((long)C_VOTE_MAX_BYTE + 5L) * (long)Position);
	}
	else
		E_Position = Position;

	sprintf(szPos,"%07ld",E_Position);
	if(szPos[0] == '-')
		szPos[0] = '0';

	pzPassword[0] = CMinipassword[0];
	pzPassword[1] = szPos[3];
	pzPassword[2] = CMinipassword[2];
	pzPassword[3] = szPos[4];
	pzPassword[4] = CMinipassword[7];
	pzPassword[5] = szPos[5];
	pzPassword[6] = CMinipassword[4];
	pzPassword[7] = szPos[6];
	pzPassword[8] = szPos[0];
	pzPassword[9] = CMinipassword[1];
	pzPassword[10] = CMinipassword[3];
	pzPassword[11] = szPos[2];
	pzPassword[12] = CMinipassword[6];
	pzPassword[13] = szPos[1];
	pzPassword[14] = CMinipassword[5];
    pzPassword[15] = 0x00;

}
/* 10.03c >> */
/****************************************************************************/
/*                      FIN DU MODULE VARIABLE.C                            */
/****************************************************************************/

/* END CONFIDENTIAL */
