/*******************************************************************/
/* MODULE DE GESTION DU FLASH EPROM                                */
/*          Simulation en m‚moire vive et sauvegarde sur disquette */
/*******************************************************************/
#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <dos.h>
#include <memory.h>
#include <string.h>
#include "defs.h"
#include "lcd.h"
#include "erreur.h"
#include "eprom.h"

/* << 10.02h */
#define DataSize 1024

/*char choix;*/
/*void Lcd_Affiche(int , int , char *);*/
/* 10.02h >> */

/***********************************************/

unsigned char _huge *MEMORY_EPROM;   /* Adresse des donnees de l'EPROM  */
					 /*  en memoire vive                */
unsigned char _huge *EPROM;          /* Adresse des donnees de l'EPROM  */
unsigned char _huge *EPROM_CONTROL;  /* Adresse du registre de control  */
					 /*  de l'EPROM                     */
long  EPROM_TAILLE;                 /* Taille en caracteres de l'EPROM */

FILE *FEprom;

/* << 10.03g */
extern int iDiskId;
/* 10.03g >> */
/* START CONFIDENTIAL */

/***********************************************/
boolean Eprom_Init(void)
/*
	 Initialise l'EPROM
	 Parametre: taille de la memoire utilisee
	 Rend le test sur l'initialisation
*/
{

  EPROM_TAILLE = (long)(1024L * (long)_EPROM_TAILLE);
  FP_SEG(EPROM_CONTROL) =EPROM_SEG;
  FP_OFF(EPROM_CONTROL) = 0x0;
  FP_SEG(EPROM) = EPROM_SEG;
  FP_OFF(EPROM) = 0x0;
  return(true);
} /* Eprom_Init */

/***********************************************/

boolean Eprom_Read(unsigned char *Valeur, long Emplacement)
/*
	 Lit la valeur d'une adresse de l'EPROM
	 Parametres: adresse sur valeur a lire
		position de l'EPROM a lire
	 Rend le test sur la lecture
*/
{
  unsigned char _huge *p;
  unsigned char _huge *q;
  unsigned char val;

  if (Emplacement >= EPROM_TAILLE) return(false);
  p = EPROM_CONTROL;
  q = EPROM + (long)Emplacement;
  *p = 0x00;
  val = *q;
  if (val == 255) val = 0;
  *Valeur = val ;

  return(true);
} /* Eprom_Read */

/***********************************************/
boolean Eprom_Write(unsigned char Valeur, long Emplacement)
/*
	 Ecrit une valeur a une adresse de l'EPROM
	 Parametres: valeur a ecrire
		position de l'EPROM a ecrire
	 Rend le test sur l'ecriture
*/
{
  unsigned char _huge *p;
  unsigned char _huge *q;
  unsigned char val, c, choix;
  int i, PLSCNT, Boucle, status;
  int Compt1, Compt2, Compt_Boucle;

  Compt1 = 0;
  PLSCNT = 0;
  do
    {
      Compt1++;
      for (Compt2 = 0; Compt2 < (timing / 10);Compt2--,Compt2++,Compt2++);
    } while ((Eprom_Writed(Emplacement)) && (Compt1 < 20));


	if (Compt1 >= 20)
	 {
		Lcd_Affiche(0,0,Message_Point[2]);
		Lcd_Affiche(1,0,Message_Point[5]);
		status = Ram_Eprom_Load();
		if(status)
			Lcd_Affiche(0,0,Message_Point[6]);
		Lcd_Affiche(1,0," ");
		return(false);
	 }
	do
	 {
		p = EPROM_CONTROL;
		*p = 0x40;
		q = EPROM + (long)Emplacement;
		*q = Valeur;
		for (i = 0; i < timing ; i--,i++,i++); /* 2000  ns*/
		*p = 0xC0;
		for (i = 0; i < timing ; i--,i++,i++);
		val = *q;
		PLSCNT++;
	 } while ((val != Valeur) && (PLSCNT < 100));

	 p  = EPROM_CONTROL;
	 *p = 0x00;
	 if ((val != Valeur) || (PLSCNT >= 100))
		{
		  Lcd_Affiche(0,0,Message_Point[2]);
		  Lcd_Affiche(1,0,Message_Point[5]);
		  Ram_Eprom_Load();
		  if(status)
			 Lcd_Affiche(0,0,Message_Point[6]);
		  Lcd_Affiche(1,0," ");
		  return(false);
		}
  return(true);

} /* Eprom_Write */

/***************************************************************************/
int Ram_Eprom_Load()
/*
	 Ecrit une valeur a une adresse de l'EPROM
	 Parametres: valeur a ecrire
		position de l'EPROM a ecrire
	 Rend le test sur l'ecriture
*/
{
 unsigned char _huge *p;
 unsigned char _huge *q;
 unsigned char val[1024], c, choix;
 unsigned long Pos, Emplacement;
 int i, j, PLSCNT, status,boucle;
 FILE *Fichier;

 for(boucle = false; !boucle;)
  {
	 Lcd_Affiche(0,0,Message_Point[8]);
	 Lcd_Affiche(1,0,Message_Point[9]);
	 status =  Eprom_Flash();
	 Wait_Time(2);
	 if(!status)
	  {
		 Lcd_Affiche(0,0,Message_Point[2]);
		 Lcd_Affiche(1,0,Message_Point[10]);
		 choix = ' ';
		 while((choix != 'O') && (choix != 'N'))
												choix = Getch(COLOR);
		 if (choix == 'O')
		  {
			 SUPPORT = 'R';
			 Ram_Disk_Load();
			 return(false);
		  }
		 Lcd_Affiche(1,0," ");
	  }
	else
	 {
	  FEram = open(Chemin_R,O_RDWR | O_BINARY);
	  if (FEram == -1)
		{
		 Lcd_Affiche(0,0,Message_Point[3]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
		}
	  for(Pos = 0;Pos < (long)_EPROM_TAILLE;Pos++)
		  {
			  read(FEram,val,1024);
			  for(i = 0; i < 1024; i++)
				{
					PLSCNT = 0;
					Emplacement =(long)((long)Pos * 1024L) + (long)i;
					do
					{
					  p = EPROM_CONTROL;
					  *p = 0x40;
					  q = EPROM + (long)Emplacement;
					  *q = val[i];
					  for (j = 0; j < (timing / 80) ; j--,j++,j++); /* 25 ns */
					  *p = 0xC0;
					  for (j = 0; j < (timing / 80) ; j--,j++,j++);
					  c = *q;
					  PLSCNT++;
					} while ((val[i] != c) && (PLSCNT < 200));
				  p  = EPROM_CONTROL;
				  *p = 0x00;
				  if (PLSCNT >= 200)
					 {
						Lcd_Affiche(0,0,Message_Point[11]);
						Lcd_Affiche(1,0,Message_Point[10]);
						choix = ' ';
						while((choix != 'O') && (choix != 'N'))
						choix = Getch(COLOR);
						if (choix == 'O')
						  {
							close(FEram);
							SUPPORT = 'R';
							Ram_Disk_Load();   /* */
							return(false);
						  }
						Lcd_Affiche(1,0," ");
					 }
				  else
					 boucle = true;
				}
		  }
		close(FEram);
	 }
 }

/*  EFFACE LE FICHIER SUR DISQUETTE PUISQU'IL EST BIEN CHARGE DANS L'EPROM */

 Fichier = fopen(Chemin_D,"wb");
 if (Fichier != NULL)
  {
	  fclose(Fichier);
	  remove(Chemin_D);
  }

 return(true);

} /* Ram_Eprom_Load */
/***********************************************/
boolean Eprom_Writed( long Emplacement)
/*
	Determine si l'emplacement de l'EPROM est occup‚
	Parametre: position de l'EPROM a tester
	Rend le test
*/
{
  unsigned char val;
  unsigned char _huge *p;
  unsigned char _huge *q;
  int test;

  p = EPROM_CONTROL;
  q = EPROM + (long)Emplacement;
  *p = 0x00;
  val = *q;
  test = (val != 255);
  return(test);

} /* Eprom_Writed */

/***********************************************/
boolean Eprom_Flash()
/*
	Rend libre toutes les positions de l'EPROM
	Parametre: sans
	Rend le test sur le 'flashage'
*/
{
  unsigned char _huge *p;
  unsigned char _huge *q;
  unsigned char val;
  unsigned long emplacement;
  unsigned int  i, PLSCNT;
  unsigned long TAILLE_EPROM;
  TAILLE_EPROM = (long)((long)_EPROM_TAILLE * 1024L);

 for(emplacement = 0;emplacement < TAILLE_EPROM; emplacement++)
	{
	   if((emplacement % 400) == 0)
	     {
		if(GCOLOR ==_ON_GREEN)
			 GCOLOR = _ON_RED;
		else
			GCOLOR = _ON_GREEN;
		Light_LED(GCOLOR);
	     }
	  PLSCNT = 0;
	  do
		{
		  p = EPROM_CONTROL;
		  *p = 0x40;
		  q = EPROM + (long)emplacement;
		  *q = 0x00;
		  for (i = 0; i < (timing / 80) ; i--,i++,i++); /* 25 ns*/
		  *p = 0xC0;
		  for (i = 0; i < (timing / 80) ; i--,i++,i++);
		  val = *q;
		  PLSCNT++;
		 } while ((val != 0x00) && (PLSCNT < 1000));
		p = EPROM_CONTROL;
	  *p = 0x00;
	  if ((val != 0x00) || (PLSCNT >= 1000))
			return(false);
	}

  emplacement = 0;
  PLSCNT = 0;
  do
	 {
		p = EPROM_CONTROL;
		*p = 0x20;
		for (i = 0; i < (timing / 10); i++, i--, i++); /* 200 ns */
		p = EPROM_CONTROL;
		*p = 0x20;
		for (i = 0; i < (timing / 10); i++, i--, i++);
		do
		  {
			 p = EPROM_CONTROL;
			 *p = 0xA0;
			 for (i = 0; i < (timing / 80); i++, i--, i++); /* 25 ns jj*/
			 q = EPROM + (long)emplacement;
			 val = *q;
			 if(val == 0xFF) emplacement++;
			 if((emplacement % 600) == 0)
				{
				  if(GCOLOR ==_ON_GREEN)
					GCOLOR = _ON_RED;
				  else
					GCOLOR = _ON_GREEN;
				  Light_LED(GCOLOR);
				}
		  } while ((val == 0xFF) && (emplacement < TAILLE_EPROM));
		PLSCNT++;
	 } while ((val != 0xFF) && (PLSCNT < 4000));

	 p = EPROM_CONTROL;
	 *p = 0x00;
	 if ((val != 0xFF) || (PLSCNT >= 4000))
			return(false);
	 else
			return(true);

} /* Eprom_Flash */
/****************************************************************************/
/* << 10.02 */
/*
   <FUNCTION>
		<NAME>Check_EPROM</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Check if the EPROM contains other characters than FF except JITES inscription
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
boolean Check_EPROM()
{
	unsigned char val;
	unsigned char _huge *p;
	unsigned char _huge *q;
	unsigned long lpos;
	int iCpt;

	Lcd_Affiche(0,0,Message_Point[12]);
	Lcd_Affiche(1,0," ");

	for (lpos = 0; lpos < (long)_EPROM_TAILLE; lpos++)
	{
	    /*GCOLOR = _ON_RED;
		Light_LED(GCOLOR);*/
		if((lpos % 25) == 0)
        {
		  if(GCOLOR ==_ON_GREEN)
			 GCOLOR = _ON_RED;
		  else
			GCOLOR = _ON_GREEN;
		  Light_LED(GCOLOR);
        }

		for (iCpt = 0; iCpt < 1024; iCpt++)
		{
			p = EPROM_CONTROL;
			q = EPROM + (long)((long)lpos * 1024L) + (long)iCpt;
			*p = 0x00;
			val = *q;
			if(lpos==0)
			{
				switch (iCpt)
				{
				case 400:
					if(val != 0x4A)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
					break;

				case 401:
					if(val != 0x49)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
					break;

				case 402:
					if(val != 0x54)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
					break;
				case 403:
					if(val != 0x45)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
					break;

				case 404:
					if(val != 0x53)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
					break;

				default :
					if(val != 0xFF)
					{
						GCOLOR = _ON_NO_LED;
						Light_LED(GCOLOR);
						return false;
					}
				}
			}
			else
			{
				if(val != 0xFF)
				{
					GCOLOR = _ON_NO_LED;
					Light_LED(GCOLOR);
					return false;
				}
			}
		}
	}
	GCOLOR = _ON_GREEN;
	Light_LED(GCOLOR);
	return true;
}
/* 10.02 >> */
/****************************************************************************/
/*                           GESTION DE LA RAM                              */
/****************************************************************************/

boolean Ram_Read(unsigned char *Valeur, long Emplacement)
/*
	 Lit la valeur d'une adresse de l'EPROM
	 Parametres: adresse sur valeur a lire
		position de l'EPROM a lire
	 Rend le test sur la lecture
*/
{
  unsigned char val;

  lseek(FEram,Emplacement,0);
  read(FEram,&val,1);
  if (val == 255) val = 0;
  *Valeur = val;
  return(true);
} /* Eprom_Read */

/****************************************************************************/

boolean Ram_Write(unsigned char Valeur, long Emplacement)
/*
	 Ecrit une valeur a une adresse de la Ram
	 Parametres: valeur a ecrire
		position de l'EPROM a ecrire
	 Rend le test sur l'ecriture
*/
{

  lseek(FEram,Emplacement,0);
  write(FEram,&Valeur,1);

  return(true);
} /* Eprom_Write */

/****************************************************************************/

boolean Ram_Writed( long Emplacement)
/*
	Determine si l'emplacement de la Ram est occup‚
	Parametre: position de la Ram a tester
	Rend le test
*/
{
  unsigned char val;
  int test;

  lseek(FEram,Emplacement,0);
  read(FEram,&val,1);
  test = (val != 255);
  return(test);

} /* Eprom_Writed */


/****************************************************************************/

int Eprom_Ram_Load()
/*
	Sauve l'EPROM dans la Ram
*/
{
  unsigned char val[1024];
  unsigned char _huge *p;
  unsigned char _huge *q;
  unsigned long pos;
  int i;
  FILE *Fichier;

  Fichier = fopen(Chemin_R,"wb");
  if (Fichier == NULL)
	 {
		 Lcd_Affiche(0,0,Message_Point[3]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
	 }
  fclose(Fichier);

  FEram = open(Chemin_R, O_RDWR | O_BINARY);
  if (FEram == -1)
	 {
		 Lcd_Affiche(0,0,Message_Point[3]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
	 }

  Lcd_Affiche(0,0,Message_Point[12]);
  Lcd_Affiche(1,0," ");
  for (pos = 0; pos < (long)_EPROM_TAILLE; pos++)
	{
	  for (i = 0; i < 1024; i++)
		 {
			p = EPROM_CONTROL;
			q = EPROM + (long)((long)pos * 1024L) + (long)i;
			*p = 0x00;
			val[i] = *q;
		 }
	  write(FEram,val,i);
	}
 close(FEram);
 return(true);

}  /* Backup_Eprom vers ram */
/****************************************************************************/
/* << 10.02h */
/*
   <FUNCTION>
		<NAME>Compare_Ram_Eprom</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>Compares FE_RAM and EPROM contents</DESCRIPTION>
        <PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
int Compare_Ram_Eprom()
{
	unsigned char szBuffRam[DataSize+1], szBuffEPROM[DataSize+1];
	unsigned char _huge *p;
	unsigned char _huge *q;
	unsigned long lpos;
	int iCpt, iRes;
	char szMsgError[80];

	FEram = open(Chemin_R, O_RDONLY | O_BINARY);
	if (FEram == -1)
	{
		sprintf(szMsgError,"Error opening file %s in Compare_Ram_Eprom function.\nFile source: %s\n",
						Chemin_R, __FILE__);
		Erreur(szMsgError);
		return -1;
	}


	Lcd_Affiche(0,0,Message_Point[12]);
	Lcd_Affiche(1,0," ");
	for (lpos = 0; lpos < (long)_EPROM_TAILLE; lpos++)
	{
		/* Leds */
		GCOLOR = _ON_RED;
		Light_LED(GCOLOR);

		iRes = read(FEram,szBuffRam,DataSize);
		if(iRes == -1)
		{
			GCOLOR = _ON_NO_LED;
			Light_LED(GCOLOR);
			sprintf(szMsgError,"Error reading from the file %s in Compare_Ram_Eprom function.\nFile source: %s\n",
						Chemin_R, __FILE__);
			Erreur(szMsgError);
			close(FEram);
			Lcd_Affiche(0,0," ");
			return -1;
		}

		for (iCpt = 0; iCpt < 1024; iCpt++)
		{
			p = EPROM_CONTROL;
			q = EPROM + (long)((long)lpos * 1024L) + (long)iCpt;
			*p = 0x00;
			szBuffEPROM[iCpt] = *q;
		}

		szBuffRam[DataSize] = '\0';
		szBuffEPROM[DataSize] = '\0';
		if(strcmp(szBuffRam,szBuffEPROM) != 0)
		{
			close(FEram);
			return 1;
		}

	}
	close(FEram);
	Lcd_Affiche(0,0," ");

	return 0;

}
/* 10.02h >> */
/****************************************************************************/

int Disk_Ram_Load()
/*
	Sauve le disque dans la Ram
*/
{
  unsigned char val[1024], str[20];
  unsigned long pos;
  int i;
  FILE *Fichier;

  Fichier = fopen(Chemin_R,"wb");
  if (Fichier == NULL)
	 {
		 Lcd_Affiche(0,0,Message_Point[3]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
	 }
  fclose(Fichier);

  FEram = open(Chemin_R, O_RDWR | O_BINARY );
  if (FEram == -1)
	 {
		 Lcd_Affiche(0,0,Message_Point[3]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
	 }

  FEdsk = open(Chemin_D, O_RDWR | O_BINARY);
  if (FEdsk == -1)
	 {
		 Lcd_Affiche(0,0,Message_Point[4]);
		 Lcd_Affiche(1,0,Message_Point[7]);
		 Boucle_Erreur();
	 }

  Lcd_Affiche(0,0,Message_Point[12]);
  Lcd_Affiche(1,0," ");
  for (pos = 0; pos < (long)_EPROM_TAILLE; pos++)
	{
	  i = read(FEdsk,val,1024);
	  if(i < 1024)
		  {
			 Lcd_Affiche(0,0,Message_Point[4]);
			 Lcd_Affiche(1,0,Message_Point[7]);
			 Boucle_Erreur();
		  }
	  write(FEram,val,i);
	}

 close(FEram);
 close(FEdsk);
 return(true);

}  /* Backup disk vers  ram */

/****************************************************************************/

int Ram_Disk_Load()
/*
	Sauve la Ram sur disque
*/
{
  unsigned char val[1024];
  unsigned long pos;
  int i, status, Carry, iNum;
  FILE *Fichier;
  /* << 10.03g */
  boolean bCheckDisk, bError;

  bError = false;
  /* 10.03g >> */
 status = true;

 while(status)
       {
	   /* << 10.03g */
	   bCheckDisk = true;
	   /*while((Fichier = fopen(Chemin_D, "wb")) == NULL)*/
	   while(bCheckDisk)
	   {
			if((Fichier = fopen(Chemin_D, "wb")) == NULL)
			{
                Change_Disque(Message_Point[19]);
                iNum = Disk_Num() ;
                iDiskId = Check_NumDisk(iNum);
			}
			else
			{
                bCheckDisk = false;
			    fclose(Fichier);
			}
	   }
	   /* 10.03g >> */

       /* << 10.03g */
	   bCheckDisk = true;
	   /*while((FEdsk = open(Chemin_D, O_RDWR | O_BINARY)) == -1)*/
	   while(bCheckDisk)
	   {
			if((FEdsk = open(Chemin_D, O_RDWR | O_BINARY)) == -1)
			{
                Change_Disque(Message_Point[19]);
                iNum = Disk_Num() ;
                iDiskId = Check_NumDisk(iNum);
			}
			else
                bCheckDisk = false;
	   }
       /* 10.03g >> */

	   FEram = open(Chemin_R, O_RDWR | O_BINARY);
	   if (FEram == -1)
	     {
		Lcd_Affiche(0,0,Message_Point[3]);
		Lcd_Affiche(1,0,Message_Point[7]);
		Boucle_Erreur();
	     }

	   Lcd_Affiche(1,0,Message_Point[12]);
	   Lcd_Affiche(1,0," ");
	   for (pos = 0; pos < (long)_EPROM_TAILLE; pos++)
       {
            i = read(FEram,val,1024);
            Carry = write(FEdsk,val,i);
            if(Carry < i)
              {
                Change_Disque(Message_Point[19]);
                 /* << 10.03g */
                iNum = Disk_Num() ;
                iDiskId = Check_NumDisk(iNum);
                /*status = false;*/
                bError = true;
                break;
                 /* 10.03g >> */
              }
            /* << 10.03g */
            /*else
              status = false;*/
            /* 10.03g >> */
       }
       /* << 10.03g */
       if(!bError)
          status = false;
       /* 10.03g >> */
	   close(FEram);
	   close(FEdsk);
       }
return(true);

}  /* Backup disk vers  ram */
/****************************************************************************/
/* << 10.02h */
/*
   <FUNCTION>
		<NAME>Compare_Ram_Disk</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>Compares files FE_RAM and FE_DSK</DESCRIPTION>
        <PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
int Compare_Ram_Disk()
{
	unsigned char szBuffRam[DataSize+1], szBuffDisk[DataSize+1];
	unsigned long lpos;
	int iRes;
	char szMsgError[80];

	FEram = open(Chemin_R, O_RDONLY | O_BINARY);
	if (FEram == -1)
	{
		sprintf(szMsgError,"Error opening file %s in Compare_Ram_Disk function.\nFile source: %s\n",
						Chemin_R, __FILE__);
		Erreur(szMsgError);
		return -1;
	}

	FEdsk = open(Chemin_D, O_RDONLY | O_BINARY);
	if (FEdsk == -1)
	{
		sprintf(szMsgError,"Error opening file %s in Compare_Ram_Disk function.\nFile source: %s\n",
						Chemin_D, __FILE__);
		Erreur(szMsgError);
		close(FEram);
		return -1;
	}

	Lcd_Affiche(0,0,Message_Point[12]);
	Lcd_Affiche(1,0," ");
	for (lpos = 0; lpos < (long)_EPROM_TAILLE; lpos++)
	{
		/* Leds */
		GCOLOR = _ON_RED;
		Light_LED(GCOLOR);

		iRes = read(FEram,szBuffRam,DataSize);
		if(iRes == -1)
		{
			GCOLOR = _ON_NO_LED;
			Light_LED(GCOLOR);
			sprintf(szMsgError,"Error reading from the file %s in Compare_Ram_Disk function.\nFile source: %s\n",
						Chemin_R, __FILE__);
			Erreur(szMsgError);
			close(FEram);
			close(FEdsk);
			Lcd_Affiche(0,0," ");
			return -1;
		}

		iRes = read(FEdsk,szBuffDisk,DataSize);
		if(iRes == -1)
		{
			GCOLOR = _ON_NO_LED;
			Light_LED(GCOLOR);
			sprintf(szMsgError,"Error reading from the file %s in Compare_Ram_Disk function.\nFile source: %s\n",
						Chemin_D, __FILE__);
			Erreur(szMsgError);
			close(FEram);
			close(FEdsk);
			Lcd_Affiche(0,0," ");
			return -1;
		}

		szBuffRam[DataSize] = '\0';
		szBuffDisk[DataSize] = '\0';
		if(strcmp(szBuffRam,szBuffDisk) != 0)
		{
			close(FEram);
			close(FEdsk);
			return 1;
		}

	}
	close(FEram);
	close(FEdsk);
	Lcd_Affiche(0,0," ");

	return 0;

}
/* 10.02h >> */
/****************************************************************************/
/*                           GESTION DU DISK                                */
/****************************************************************************/

boolean Dsk_Write(unsigned char Valeur, long Emplacement)
/*
	 Ecrit une valeur a une adresse du DISK
	 Parametres: valeur a ecrire
		position de l'EPROM a ecrire
	 Rend le test sur l'ecriture
*/
{
  int status;

  lseek(FEdsk,Emplacement,0);
  status = write(FEdsk,&Valeur,1);

  return(status);

} /* Disk_Write */

/****************************************************************************/
/*                    TRANSFERT DES DONNEES ENTRE SUPPORTS                  */
/****************************************************************************/

int Witch_Support()
/*
	Sur quel support nous allons travailler ?
*/
{
  int status;

  FEdsk = open(Chemin_D, O_RDONLY | O_BINARY);
  if(FEdsk != -1)
	 {
		close(FEdsk);
		status = Test_Ep();
		if(status)
		 {
		    Lcd_Affiche(1,0,Message_Point[13]);
		    Disk_Ram_Load();
		    Ram_Eprom_Load();
		    return(0);
		 }
		else
		 {
		   Disk_Ram_Load();
		   SUPPORT = 'R';
		   return(0);
		 }
	 }

	Eprom_Ram_Load();
	return(true);

}  /* Backup_Eprom vers ram */

/*******************************************************************/

void Down_Load_Ind(void)
/*
Descendre les parametres de l'eprom
*/
{
  unsigned char _huge *p;
  unsigned char _huge *q;
  unsigned char val[32];
  unsigned long pos;
  int i, j, status, Carry,ok;

 status = false;

 while(status == false)
	{
		while((FEprom = fopen("a:urne.Ind", "wt")) == NULL)
		   {
		     Lcd_Affiche(0,0,Message_Point[3]);
		     Lcd_Affiche(1,0,Message_Point[7]);
		   }
		for (pos =(long)_E_PAR_PLACE,i = 0,ok = true; ok && (i < 31) ; pos++,i++)
		   {
		     p = EPROM_CONTROL;
		     q = EPROM + (long)pos;
		     *p = 0x00;
		     if(*q != 0xFF)
			    val[i] = *q;
		     else
			     ok = false;
		   }
		Carry = fwrite(val,i,1,FEprom);
		if(!Carry)
		   {
		      Lcd_Affiche(0,0,Message_Point[3]);
		      Lcd_Affiche(1,0,Message_Point[7]);
		      while(1);
		    }
		else
		  status = true;
		i++;
	 }
	Carry = fwrite("\n",1,1,FEprom);
	if(!Carry)
	  {
	   Lcd_Affiche(0,0,Message_Point[3]);
	   Lcd_Affiche(1,0,Message_Point[7]);
	   while(1);
	  }
	 fclose(FEprom);
}



/* END CONFIDENTIAL */
/*******************************************************************/
/*  FIN DU MODULE DE GESTION DU FLASH EPROM                         /
/*******************************************************************/
