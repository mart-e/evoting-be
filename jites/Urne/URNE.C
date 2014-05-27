/*******************************************************************/
/*  DEBUT DU MODULE PRINCIPAL URNE.C                               */
/*******************************************************************/
#include "defs.h"

#include <ctype.h>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dos.h>

#include "com.inc"
#include "fullspec.h"
#include "api-fst.h"
#include "alg-fst.h"

#include "urne.h"
#include "lcd.h"
#include "eprom.h"
#include "variable.h"
#include "erreur.h"
#include "vote.h"
#include "pc-urne.h"
#include "lecteur.h"
#include "zeit.h"
unsigned long far *utimer = (unsigned long *) 0x46C;

#define _WAITL { unsigned long inc;                     \
 for(inc=*utimer;*utimer - inc < (unsigned long) 5;);    \
			}

/***************************************************************************/
int Table[2000],Table_Val[3000];
int Cmpt_Gen,Cmpt_Gen_Val;

int ETAT_URNE, C_VOTE_MAX_BYTE = 0;
int BATTERY;
int Total_Erreur_Eprom = 0;
char Chemin_R[25];
char Chemin_D[25];
/* << 10.02 */
/*char Minicodage[] = "4630"; */
char Minicodage[] = MINI_PWD;
/* 10.02 >> */
int FEram, FEdsk;
int Additionne;
/* << 10.01 */
/*char Message_point[118][21];*/
char Message_Point[130][21];
int iDiskId ;/*Disk number*/
int iMAXDISK ;
/* 10.01 >>  */
char LANGUE;
int Ordre_Dsk[2];
/* << 10.03g */
int iERRORCLOSE;
/* 10.03g >> */

/* << 10.04 */
char szIDCreation[12];
/* 10.04 >> */

/* << 10.02b */
/* Vote counters */
extern int  *piNOMBRE_VOTES1, *piNOMBRE_VOTES2, *piNOMBRE_VOTES3;
extern int  *piNOMBRE_NATIO_VOTES1, *piNOMBRE_NATIO_VOTES2, *piNOMBRE_NATIO_VOTES3;
extern int  *piNOMBRE_EURO_VOTES1, *piNOMBRE_EURO_VOTES2, *piNOMBRE_EURO_VOTES3;
extern int  *piNOMBRE_STR_VOTES1, *piNOMBRE_STR_VOTES2, *piNOMBRE_STR_VOTES3;
/* 10.02b >> */
/***************************************************************************/
/* << 10.03b */
extern void Maj_IvBit(char);
extern void Read_Be2(void);
/* 10.03b >> */

/* << 10.02e*/
/*void Demande_Identite(char *Clef)*/
boolean Demande_Identite(char *pzClef, boolean bRequire)
/* 10.02e >> */
/*
	Demande d'un mot de passe
	Parametre: chaine de carateres contenant le mot de passe introduit
	Rend void
*/
{
  int i, touche;
  char strtmp[11], car;

  strcpy(strtmp, "..........");
  Lcd_Affiche(0, 0, Message_Point[29]);
  Lcd_Affiche(1, 0, "");
  Lcd_Affiche(1, 5, strtmp);
  /* << 10.02e */
  /*for (i = 0; i < _CLEF_MAX_BYTE; i++)*/
  i = 0;
  while(i < _CLEF_MAX_BYTE)
  /* 10.02e >> */
	 {
		/* << 10.02e */
		car = Getch(COLOR);
		if((car != 'O') && (car != 'N'))
		{
            /*Clef[i] = Getch(COLOR);*/
            pzClef[i] = car;
            /*strtmp[i] = '*';*/
            strtmp[i] = pzClef[i];
            /* 10.02e >> */
            Lcd_Affiche(1, 5, strtmp);
            i++;
		}

		if(!bRequire)
		{
			if(car == 'N')
				return false;
		}
	 }
  /* << 10.02e */
  /*Clef[_CLEF_MAX_BYTE] = '\0';*/
  pzClef[_CLEF_MAX_BYTE] = '\0';

  return true;
  /* 10.02e >> */
} /* Demande_Identite */
/***************************************************************************/
int Verifie_Urne(char *Clef)
/*
	Verifie le mot de passe par rapport a ceux fournis par le PC de controle
	Parametre: nom du fichier de donnee du PC de controle
			mot de passe introduit
	Rend le test sur le mot de passe
*/
{
  if (strcmp(Clef, CLEF_URNE) == 0)

	 return(true);
  else
	 return(false);
}
/***************************************************************************/
/* << 10.04 */
int Check_Key(char *key)
{
    int cpt;
    char sztemp[21];

    for(cpt=0; cpt < _CLEF_MAX_BYTE; cpt++)
    {
        if((cpt < 8) && (key[cpt] != CLEF_URNE[cpt]))
        {
            return false;
        }
        else if((cpt >= 8) && (key[cpt] != '0'))
        {
            return false;
        }
    }

    return true;
}
/* 10.04 >> */
/***********************************************/

boolean Init_Systeme()
/*
	 Initialise la machine et teste les peripheriques
	 Parametre: sans
	 Rend le test sur les peripheriques
*/
{
int i,fd_crc;
unsigned char tab_verif[16];
unsigned char far *pointy;
/* START CONFIDENTIAL */

 pointy = (uchar * ) 0xC0000087;
 for(i = 0;i < 15;i++)
		  tab_verif[i] = *pointy++;
		  tab_verif[i] = 0x00;

 if ((strcmp(tab_verif,"Voting Terminal")) == 0)
   {
     outp(0x308,0xf4);
     while(1)
      {
	putch(0x07);
	/* << 10.01 */
	/*printf(" Ceci n'est pas une URNE JITES");
	printf(" Dit is geen JITES URNE");*/
	Lcd_Affiche(0,0, Message_Point[115]);
    Lcd_Affiche(1,0, Message_Point[7]);
	/* 10.01 >> */
	Wait_Time(1);
      }
   }
 if (! Lecteur_1_Init() || ! Lecteur_2_Init())
    {
      Lcd_Affiche(0,0, Message_Point[30]);
      Lcd_Affiche(1,0, Message_Point[7]);
      Boucle_Erreur();
    }

/* Lecture de la version le 6/12/1997 */

 pointy = (uchar * ) 0xFE9D0022;
 for(i = 0;i < 13;i++)
		 tab_verif[i] = *pointy++;
 tab_verif[i] = 0x00;
 if ((strcmp(tab_verif,"Version 1.0.1")) == 0) /*  dans la langue active  */
      {
	 EPROM_SEG =  0xC000;
	 timing    =  2000;
      }
 else
    {
      pointy = (uchar * ) 0xC7FC0019;
      for(i = 0;i < 13;i++)
		tab_verif[i] = *pointy++;
      tab_verif[i] = 0x00;
	 if ((strcmp(tab_verif,"Version 2.0.0")) == 0)
	    {
		 EPROM_SEG =  0xD000;
		 timing    = 3000;
	    }
	 else
	   {
	     Lcd_Affiche(0,0, "Version error...");
	     Boucle_Erreur();
	   }

      }
 Lcd_Affiche(1,0, "V:");
 Lcd_Affiche(1,4, tab_verif);
 Wait_Time(4);
/* END CONFIDENTIAL */

 if (!Eprom_Init())
   {
     Lcd_Affiche(0,0, Message_Point[2]);
     Lcd_Affiche(1,0, Message_Point[7]);
     Boucle_Erreur();
   }

 Lcd_Init();
 return(true);

} /* Init_Systeme */
/****************************************************************************/

boolean Init_Version()
/*
	 Initialise la machine et teste les peripheriques
	 Parametre: sans
	 Rend le test sur les peripheriques
*/
{
int i;
unsigned char tab_verif[16];
unsigned char far *pointy;
/* START CONFIDENTIAL */

pointy = (uchar * ) 0xFE9D0003;
 /* C0000087;*/
 for(i = 0;i < 28;i++)
		  tab_verif[i] = *pointy++;
		  tab_verif[i] = 0x00;
  Lcd_Affiche(0,0, &tab_verif[9]);
/* END CONFIDENTIAL */

Wait_Time(50);
return(true);

} /* Init_Systeme */
/****************************************************************************/
int Test_Ep()
{
/*
	Saisie du choix du support
	Parametre:sans
	Rend test sur la mise en mode test
*/
  char choix;

  Lcd_Affiche(0,0,Message_Point[32]);
  Lcd_Affiche(1,0,Message_Point[33]);
  choix = ' ';

  while((choix != 'O') && (choix != 'N'))
				choix = Getch(COLOR);
  if (choix == 'O')
	  return(true);
  else
	  return(false);
}
/****************************************************************************/

int Saisie_langue()
{
  char choix;
  FILE *Fichier_BK;

  Lcd_Affiche(0,0, Message_Point[0]);
  Lcd_Affiche(1,0, Message_Point[1]);
  choix = ' ';
  while((choix != '1') && (choix != '3'))
				choix = Getch(COLOR);
  if (choix == '1')
	  return(true);
  else
	  return(false);

} /* CHOIX DE LA LANGUE */

/****************************************************************************/

int Test_Bk()
{
  char choix;
  FILE *Fichier_BK;

  Fichier_BK = fopen("JT_BAK", "rt");
  if (Fichier_BK != NULL)
	 {
		fclose(Fichier_BK);
		Lcd_Affiche(0,0, Message_Point[34]);
		Lcd_Affiche(1,0, Message_Point[35]);
		choix = ' ';
		while((choix != 'O') && (choix != 'N'))
				choix = Getch(COLOR);
		if (choix == 'O')
			 return(true);
		else
		  {
		    Lcd_Affiche(0,0, Message_Point[25]);
		    Lcd_Affiche(1,0, Message_Point[36]);
		    Boucle_Erreur();
		  }
	  }
return(false);
} /* Test_BK */

/****************************************************************************/

int Test_Bk_Aut()
{
  FILE *Fichier_BK;

  Fichier_BK = fopen("JT_BAK", "rt");
  if (Fichier_BK != NULL)
	 {
		fclose(Fichier_BK);
		return(true);
	 }
return(false);
} /* Test_BK_Aut */
/****************************************************************************/
/* << 10.01 */

/*
   <FUNCTION>
		<NAME>Load_Message</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Selects the messages file to be loaded, and call Read_File function
		</DESCRIPTION>
        <PARAMETER>
			<NAME>iNum</NAME>
			<TYPE>int</TYPE>
			<IO>IN</IO>
			<DESCRIPTION>parameter by value is the messages file identifier</DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>void</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Load_Message(int iNum)
{
	switch(iNum)
	{
		case 1 :
			Read_File("a:MSG_1");
			break;

		case 2 :
			Read_File("a:MSG_2");
			break;
    }


}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Read_File</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Reads the file of the messages and loads them into the buffer of messages
		</DESCRIPTION>
        <PARAMETER>
			<NAME>pzFileName, iNum</NAME>
			<TYPE>char *</TYPE>
			<IO>INOUT</IO>
			<DESCRIPTION>First parameter is filename, second parameter is messages file identifier</DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>void</TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Read_File(char *pzFileName)
{
	char szbufm[45];
	int i, j;
	FILE *fpm;

	while((fpm = fopen(pzFileName,"rt")) == NULL)
	{
		Lcd_Affiche(0,0,"MESSAGE ERROR OTHER");
		Lcd_Affiche(1,0,"DISK AND PRESS KEY");
		Getch(COLOR);
		Lcd_Affiche(0,0," A MOMENT PLEASE");
		Lcd_Affiche(1,0," ");
	}

	i = 0;
    while((fgets(szbufm,30,fpm) != NULL))
    {
        szbufm[24] = 0x00;
        for(j=23;(j > 4) && (szbufm[j] < 32);)
        {
            szbufm[j] = 0x20;
            j--;
        }

        strcpy(Message_Point[i],szbufm + 4);
        i++;
    }

	if(fpm)
		fclose(fpm);
}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Select_Language_ID</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Selects the language identifier between french or dutch
		</DESCRIPTION>
		<PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION>the language identifier</DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION>return language identifier</DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
int Select_Language_ID()
{
	char choix;
	int iRes;

	Lcd_Affiche(0,0, "1 -> FRANCAIS  ");
    Lcd_Affiche(1,0, "2 -> NEDERLANDS");

	choix = ' ';
	while(choix != '1' && choix != '2')
        choix = Getch(COLOR);

	switch(choix)
	{
		case '1':
			iRes = 1;
			break;
		case '2':
			iRes = 2;
			break;
   }

  return iRes;
}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Language_ID</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Determines the language identifier from machvots.Tbl file
		</DESCRIPTION>
		<PARAMETER>
			<NAME></NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION>return language identifier</DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
int Language_ID()
{
	FILE *fps;
	char szbufs[33], cID_Lang;
	int iRet;

	fps = fopen("a:machvots.Tbl","rt");
	if(fps != NULL)
	{

		fgets(szbufs,32,fps);
		fclose(fps);

		/* Init Code Language*/
		cID_Lang = szbufs[15];
		switch(cID_Lang)
		{
			case '1': /* French */
                iRet =  1;
				break;

			case '2': /* Dutch */
                iRet =  2;
				break;

			case '4': /* French - Dutch */
				iRet =  Select_Language_ID();
				break;

            default :
                iRet = -1;
		}
	}
	else
		iRet = -1;

	return iRet;
}
/****************************************************************************/
/*
   <FUNCTION>
		<NAME>Disk_Num</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Check if file which indicates floppy number exists and retun a number
		</DESCRIPTION>
        <PARAMETER>
			<NAME>Nothing</NAME>
			<TYPE></TYPE>
			<IO></IO>
			<DESCRIPTION></DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE>int</TYPE>
			<DESCRIPTION>return floppy number, otherwise return 0</DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
/****************************************************************************/
int Disk_Num()
{

    if (access("a:1.DSK",0) == 0)
		return 1;

	if (access("a:2.DSK",0) == 0)
		return 2;

	if (access("a:3.DSK",0) == 0)
		return 3;

    if (access("a:4.DSK",0) == 0)
		return 4;

	return 0;
}
/* 10.01 >> */
/****************************************************************************/
/* << 10.04 */
int Get_DiskID()
{
    FILE *fp;
	char bufs[12];
	int idsk, nbcar;
	boolean indic = false;

    fp = fopen("a:1.dsk","r");
	if(fp != NULL)
	{
	    indic = true;
	    idsk = 1;
	}

	if(!indic)
    {
        fp = fopen("a:2.dsk","r");
        if(fp != NULL)
        {
            indic = true;
            idsk = 2;
        }
    }

	if(!indic)
    {
        fp = fopen("a:3.dsk","r");
        if(fp != NULL)
        {
            indic = true;
            idsk = 3;
        }
    }

    if(!indic)
    {
        fp = fopen("a:4.dsk","r");
        if(fp != NULL)
        {
            indic = true;
            idsk = 4;
        }
    }

    if(indic)
    {
        nbcar = fread(bufs,1,11,fp);
        if(nbcar == 11)
        {
            strncpy(szIDCreation,bufs,11);
            szIDCreation[11] = '\0';
            fclose(fp);
            return idsk;
        }
        else
        {
            fclose(fp);
            return 0;
        }
    }
    else
    {
        return 0;
    }

}
/* 10.04 >> */
/****************************************************************************/

void Ouverture_Urne()
/*
	 Ouvre l'urne
	 Parametre: sans
	 Rend void
*/
{
  Lcd_Affiche(0,0, Message_Point[37]);
  Lcd_Affiche(1,0, Message_Point[38]);
  Encode_E_Variable((long)_E_OUVERTURE, true);
  OUVERTURE = true;
  Encode_E_Var_String((long)_E_JETON_ETENDU, JETON_ETENDU,
						  _JETON_ETENDU_MAX_BYTE);
  Encode_E_Var_String((long)_E_PAR_PLACE,Buffer_Par,strlen(Buffer_Par));
} /* Ouverture */
/**************************************************************************/
/* << 10.02e */
boolean Check_Vote_CRC()
{
    unsigned int iPos;
    char car, szVote_Tmp[76];
    /* << 10.03c */
    char szPassword[16];
    unsigned int iVoteSize;
    /* 10.03c >> */

    Lcd_Affiche(0,0,Message_Point[12]);
    Lcd_Affiche(1,0,"");

    /* << 10.02f */
    /*GCOLOR = _ON_RED;
    Light_LED(GCOLOR);*/
    /* 10.02f >> */

    for (iPos = 0; iPos < _E_NB_MAX_VOTE; iPos++)
    {

        car = Est_Valide_E_Vote(iPos);
        if (car)
        {
            /* << 10.02f */
            if(GCOLOR ==_ON_GREEN)
              GCOLOR = _ON_RED;
            else
             GCOLOR = _ON_GREEN;
            Light_LED(GCOLOR);
            /* 10.02f >> */

           /* << 10.03c */
           /*Recupere_E_Vote(iPos,szVote_Tmp,C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE +_C_TEST_MAX_BYTE);*/
           iVoteSize = C_VOTE_MAX_BYTE + _C_CRC_MAX_BYTE + _C_TEST_MAX_BYTE;
           Recupere_E_Vote(iPos,szVote_Tmp,iVoteSize);

           Generate_Password(szPassword,iPos,true);
           Encrypt_Decrypt(szVote_Tmp,szPassword,iVoteSize);
           /* 10.03c >> */

            Calcul_Crc(szVote_Tmp,_C_CRC_MAX_BYTE);
            Recupere_Crc(szVote_Tmp,0);
            if (strcmp(Crc_Calcul,Crc_Recup) != 0)
            {
                COLOR = _ON_NO_LED;
                Light_LED(COLOR);
                return false;
            }
        }

    }

    COLOR = _ON_NO_LED;
    Light_LED(COLOR);

    return true;
}
/* 10.02e >> */
/***********************************************/
int Test_Contenu_Urne()
/*
	 Verifie le Contenue de l'urne
	 Parametre: sans
*/
{
 int i, Action, nbvotes, status;
 boolean test, testaction;
 boolean Erreur, bExit;
 /* << 10.02b */
 /*char Clef[11], strtmp[21],choix;*/
 char Clef[11], strtmp[25],choix;
 /* 10.02b >> */

 ETAT_URNE = 1;
 /* << 10.02b*/
 /*if(NOMBRE_VOTES > 0)*/
 if(*piNOMBRE_VOTES1 > 0)
 /* 10.02b >> */
  {
    /* 10.02e >> */
    if(!Check_Vote_CRC())
    {
        Lcd_Affiche(0,0, Message_Point[123]);
        Lcd_Affiche(1,0,"");
        Boucle_Erreur();
    }
    /* 10.02e >> */

    Lcd_Affiche(0,0," ");
   /* << 10.02b*/
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

    Lcd_Affiche(1,0, Message_Point[41]);
    /* << 10.02e */
    choix = ' ';
    while((choix != 'O') && (choix != 'N'))
    /* 10.02e >> */
        choix = Getch(COLOR);

    if (choix == 'N')
    {
       ETAT_URNE = 2;
       if(SUPPORT == 'R')  /* mode disquette  pas de flash */
	   {
            Lcd_Affiche(1,0, Message_Point[7]);
            Boucle_Erreur();
	   }
       Lcd_Affiche(0,0, Message_Point[42]);
       Lcd_Affiche(1,0, Message_Point[21]);

       /* << 10.02e */
        choix = ' ';
        while((choix != 'O') && (choix != 'N'))
      /* 10.02e >> */
            choix = Getch(COLOR);

       if (choix == 'O')
       {
            /* << 10.02e */
            bExit = false;
            /* 10.02e >> */
            /* << 10.04 */
            /*for (test = false,i = 0; i < 3 && (! test); i++)*/
            test = false;
            while(true)
            /* 10.04 >> */
            {
                /* << 10.02e */
                /*Demande_Identite(Clef);*/
                test = Demande_Identite(Clef,false);
                if(!test)
                {
                    bExit = true;
                    break;
                }
                else
                /* 10.02e>> */
                /* << 10.04 */
                {
                      /*test = Verifie_Urne(Clef);*/
                      test = Check_Key(Clef);
                      if(!test)
                      {
                          Lcd_Affiche(0,0, Message_Point[54]);
                          Lcd_Affiche(1,0, Message_Point[7]);
                          Wait_Time(10);
                      }
                      else
                      {
                           break;
                      }
                }
                /* 10.4 >> */
            /* << 10.02e */
            }
            /* 10.02e >> */
            if(test)
            {
                Lcd_Affiche(0,0, Message_Point[43]);
                Lcd_Affiche(1,0, Message_Point[40]);
                status = Eprom_Flash();
                if(!status)
                {
                    Lcd_Affiche(0,0, Message_Point[44]);
                    Lcd_Affiche(1,0,"");
                    Boucle_Erreur();
                }
/* START CONFIDENTIAL */
                /* << 10.02 */
                Eprom_Ram_Load();
                ETAT_URNE = 3;
                /* 10.02 >> */
                Encode_E_Var_String(400L,"JITES", 5);
/* END CONFIDENTIAL */
            }
            /* << 10.02e */
            /*}*/
            else if(!bExit)
            {
                Lcd_Affiche(0,0, Message_Point[54]);
                Lcd_Affiche(1,0,"");
                Boucle_Erreur();
            }
            /* 10.02e >> */
        }
    }
  }
 return(0);
} /* main */
/***********************************************/
int Test_Contenu_Urne2()
/*
	 Verifie le Contenue de l'urne
	 Parametre: sans
*/
{
 int i, Action, nbvotes, status;
 boolean test, testaction;
 boolean Erreur, bExit;
 char Clef[11], strtmp[21],choix;

 while(true)
  {
    Lcd_Affiche(0, 0, Message_Point[55]);
    Lcd_Affiche(1, 0, Message_Point[56]);
    Wait_Time(2);
    Lcd_Affiche(0,0, Message_Point[41]);
    Lcd_Affiche(1,0, " ");

    /* << 10.02e */
    choix = ' ';
    while((choix != 'O') && (choix != 'N'))
    /* 10.02e >> */
        choix = Getch(COLOR);

    if (choix == 'O')
    {
       if(SUPPORT == 'R')  /* mode disquette  pas de flash */
       {
           Lcd_Affiche(1,0, Message_Point[7]);
           Boucle_Erreur();
	   }
       Lcd_Affiche(0,0, Message_Point[42]);
       Lcd_Affiche(1,0, Message_Point[21]);

       /* << 10.02e */
        choix = ' ';
        while((choix != 'O') && (choix != 'N'))
      /* 10.02e >> */
            choix = Getch(COLOR);

       if (choix == 'O')
	   {
           /* << 10.02e */
            bExit = false;
            /* 10.02e >> */

           /* << 10.04 */
            /*for (test = false,i = 0; i < 3 && (! test); i++)*/
            test = false;
            while(true)
            /* 10.04 >> */
            {
                /* << 10.02e */
                /*Demande_Identite(Clef);*/
                test = Demande_Identite(Clef,false);
                if(!test)
                {
                    bExit = true;
                    break;
                }
                else
                /* 10.02e>> */
                /* << 10.04 */
                {
                      /*test = Verifie_Urne(Clef);*/
                      test = Check_Key(Clef);
                      if(!test)
                      {
                          Lcd_Affiche(0,0, Message_Point[54]);
                          Lcd_Affiche(1,0, Message_Point[7]);
                          Wait_Time(10);
                      }
                      else
                      {
                           break;
                      }
                }
                /* 10.4 >> */
           }

           if(test)
           {
                Lcd_Affiche(0,0, Message_Point[43]);
                Lcd_Affiche(1,0, Message_Point[40]);
                status = Eprom_Flash();
                if(!status)
                {
                    Lcd_Affiche(0,0, Message_Point[44]);
                    Lcd_Affiche(1,0,"");
                    Boucle_Erreur();
                }
    /* START CONFIDENTIAL */
                /* << 10.02 */
                Eprom_Ram_Load();
                /* 10.02 >> */
                Encode_E_Var_String(400L,"JITES", 5);
    /* END CONFIDENTIAL */

                return(0);
           }
           /* << 10.02e */
	       else if(!bExit)
	       /* 10.02e >> */
	       {
               Lcd_Affiche(0,0, Message_Point[54]);
               /* << 10.02e */
               Lcd_Affiche(1,0,"");
               /* 10.02e >> */
               Boucle_Erreur();
	       }
	   }
    }
  }
} /* main */


/***************************************************************************/
/* << 10.02 */
/*
   <FUNCTION>
		<NAME>Test_Eprom</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Check if JITES inscription is present in the EPROM and compares if the election date
		contained in the token of the polling station registered in the EPROM corresponding
		to the election date contained in the token obtained from the file urne.tbl.
		Check if the EPROM contains other characters than FF when the token don't exists
		</DESCRIPTION>
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
/* 10.02 >> */
int Test_Eprom()
/*
	 Verifie le Contenue de l'eprom
	 Parametre: sans
*/
{
 char tab_Epror[10];
 char tab_Dat_Epr[7];
 char tab_Dat_Dsk[7];
 char Jeton_E[_JETON_ETENDU_MAX_BYTE];
 int status;
 long  Dat_Dsk, Dat_Epr;

/* START CONFIDENTIAL */

  Recupere_E_Var_String(400L, tab_Epror, 5);
  if((strcmp("JITES",tab_Epror)) != 0)
  {
    /* << 10.02 */
     Lcd_Affiche(0,0,Message_Point[8]);
     Lcd_Affiche(1,0,Message_Point[9]);
    /* 10.02 >> */

     status = Eprom_Flash();
     if (!status)
	 {
	   Lcd_Affiche(0,0, Message_Point[44]);
	   Lcd_Affiche(1,0,"");
	   Boucle_Erreur();
	 }

    /* << 10.02 */
	 Eprom_Ram_Load();
    /* 10.02 >> */
     Encode_E_Var_String(400L,"JITES", 5);
  }
/* END CONFIDENTIAL */

  else
  {
     Recupere_E_Var_String(_E_JETON_ETENDU, Jeton_E,_JETON_ETENDU_MAX_BYTE);
     tab_Dat_Epr[0] = Jeton_E[13];
     tab_Dat_Epr[1] = Jeton_E[14];
     tab_Dat_Epr[2] = Jeton_E[11];
     tab_Dat_Epr[3] = Jeton_E[12];
     tab_Dat_Epr[4] = Jeton_E[9];
     tab_Dat_Epr[5] = Jeton_E[10];
     tab_Dat_Epr[6] = 0x00;

     tab_Dat_Dsk[0] = JETON_ETENDU[13];
     tab_Dat_Dsk[1] = JETON_ETENDU[14];
     tab_Dat_Dsk[2] = JETON_ETENDU[11];
     tab_Dat_Dsk[3] = JETON_ETENDU[12];
     tab_Dat_Dsk[4] = JETON_ETENDU[9];
     tab_Dat_Dsk[5] = JETON_ETENDU[10];
     tab_Dat_Dsk[6] = 0x00;

     Dat_Dsk = atol(tab_Dat_Dsk);
     Dat_Epr = atol(tab_Dat_Epr);
     if(( Dat_Epr < Dat_Dsk ) && ( Dat_Epr > 0))
     {
        /* << 10.02 */
        Lcd_Affiche(0,0,Message_Point[8]);
        Lcd_Affiche(1,0,Message_Point[9]);
        /* 10.02 >> */

        status = Eprom_Flash();
        if (!status)
	    {
             Lcd_Affiche(0,0, Message_Point[44]);
             Lcd_Affiche(1,0,"");
             Boucle_Erreur();
	    }
/* START CONFIDENTIAL */
        /* << 10.02 */
        Eprom_Ram_Load();
        /* 10.02 >> */
        Encode_E_Var_String(400L,"JITES", 5);
/* END CONFIDENTIAL */
    }
    /* << 10.02 */
    else
    {
		 Recupere_E_Variable(_E_FERMETURE, &FERMETURE);
		 Recupere_E_Variable(_E_OUVERTURE, &OUVERTURE);
		 if((strlen(Jeton_E) != 15) || (!FERMETURE && !OUVERTURE))
		 {
			status = Check_EPROM();
			if(!status)
			{
				Lcd_Affiche(0,0,Message_Point[8]);
                Lcd_Affiche(1,0,Message_Point[9]);

				status = Eprom_Flash();
				if (!status)
				{
					Lcd_Affiche(0,0, Message_Point[44]);
					Lcd_Affiche(1,0,"");
					Boucle_Erreur();
				}
				Eprom_Ram_Load();
				Encode_E_Var_String(400L,"JITES", 5);
			}
		 }
	 }
    /* 10.02 >> */
  }
 return(0);
}
/************************************************************************/
/* 10.03d >> */
void Check_Relais()
/*
    Checks whether it is under batteries
*/
{
   unsigned long far *timer = (unsigned long *) 0x46C;
   int SECU = 0, iNum;
   long  SECU_TIMER;
   char strtmp[21], c, ckeyboard = ' ';
   int tmp, cpt=1;
   boolean Indic = false;

   while(!Indic)
   {
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
                  ckeyboard = Kbhit(0x30,0);

                  tmp = 25 - (( *timer - SECU_TIMER ) /  19);
                  if ((tmp <= 0) || (ckeyboard == '8'))
                  {
                    tmp = 0;
                    Battery_Off();
                    Indic = true;
                  }
                  /* << 10.03e */
                  if((cpt % 3) == 0)
                    Affiche_Result();
                  /* 10.03e >> */
                   sprintf(strtmp, Message_Point[89], tmp);
                   Lcd_Affiche(1, 0, strtmp);
                   putch(0x07);
                   cpt++;
              }
       }
       else
       {
            Indic = true;
       }
   }

}
/* 10.03d >> */
/****************************************************************************/
/* << 10.01
   <FUNCTION>
		<NAME>Write_Data_Disk</NAME>
        <TYPE>LOCAL</TYPE>
		<DESCRIPTION>
		Writes cipher results of the election on the floppy
		</DESCRIPTION>
        <PARAMETER>
			<NAME>iNum</NAME>
			<TYPE>int</TYPE>
			<IO>IN</IO>
			<DESCRIPTION>Parameter by value is the number of the start floppy</DESCRIPTION>
		</PARAMETER>
		<RETURN>
			<TYPE></TYPE>
			<DESCRIPTION></DESCRIPTION>
		</RETURN>
   </FUNCTION>
*/
void Write_Data_Disk(int iNum)
{
    int iDif, iCpt, iRes, iDsk, iCount, iExitCpt;
    char szTempo[21], ckeyboard, szMessage[21];
    boolean bIndic, bExit;
    char choix;


    if (iNum > 0)
    {
       iDif = iNum - 1;
       iCpt = iNum; /* disk number */
    }
    else
    {
       iDif = 0;
       iCpt = 1;
    }

    bIndic = true;
    /* << 10.02h */
    iExitCpt = 0;
    /* 10.02h >> */

    do
    {
		iDsk = 0;
		choix = ' ';
		bExit = false;

		if(bIndic)
		{
           Lcd_Affiche(0,0,Message_Point[12]);
           Lcd_Affiche(1,0," ");

           remove("a:TABLE.URN");
           remove("a:BE.3");
		   Transfert_Table_PCC("C:TABLE.URN");

			/* << 10.03c */
			/* << 10.03d */
            /*if(iExitCpt > 0)
                Battery_On();*/
            /* 10.03d >> */
            /* 10.03c >> */

            /*      write crc      */
			iRes = Crc_Rijndael("W","3","c:","a:");
			if(iRes == -1)
			{
				/* << 10.02b */
                /*Lcd_Affiche(0,0, Message_Point[27]);*/
                Lcd_Affiche(0,0, Message_Point[122]);
                /* 10.02b >> */
				Lcd_Affiche(1,0, Message_Point[14]);
				/* << 10.03d */
				//Getch(COLOR);
				/* << 10.03f */
                /*Getchar(0x30,Message_Point[84],1,0);*/
                Getchar(COLOR,Message_Point[84],Message_Point[14],1,0);
                /* 10.03f >> */
				/* 10.03d >> */
			}
			/* << 10.02h */
			else
			{
			    iExitCpt++;
                /* << 10.03f */
			    if((iExitCpt == 1) && (SUPPORT == 'E'))
                {
                    Encode_E_Variable((long)_E_FERMETURE, true);
                }
			    /* 10.03f >> */
			    //the code below is used for certain elections
			    if(iExitCpt == 2)
			    {
			        iDif  = 0;
			        break;
			    }
			}
			/* 10.02h >> */

		}

		/* << 10.03c */
		/* << 10.03d */
        //Battery_Off();
        /* 10.03d >> */
        /* 10.03c >> */

		iCpt++;
		if (iCpt <= iMAXDISK)
		{
			iCount = 0;
			while(!bExit)
			{
				if(iCount < 3)
				{
					iCount++;
					sprintf(szTempo,Message_Point[117],iCpt);
					Lcd_Affiche(0,0, szTempo);
					Lcd_Affiche(1,0, Message_Point[64]);

                    choix = ' ';
                    while(choix != 'O')
                    /* << 10.03d */
                        //choix = Getch(COLOR);
                        /* << 10.03f */
                        /*choix = Getchar(0x30,Message_Point[125],1,0);*/
                        choix = Getchar(COLOR,Message_Point[125],Message_Point[64],1,0);
                        /* 10.03f >> */
                    /* 10.03d >> */

					iDsk = Disk_Num();
					if(iDsk == iCpt)
					{
						bIndic = true;
						bExit  = true;
					}
					else if((iDsk > 0) && (iDsk <= iMAXDISK) && (iDsk > iCpt))
					{
						sprintf(szTempo,Message_Point[118],iDsk);
						Lcd_Affiche(0,0, szTempo);
						Lcd_Affiche(1,0, Message_Point[35]);

						choix = ' ';
						while((choix != 'O') && (choix != 'N'))
							/* << 10.03d */
                            //choix = Getch(COLOR);
                            /* << 10.03f */
                            /*choix = Getchar(0x30,Message_Point[114],0,iDsk);*/
                            choix = Getchar(COLOR,Message_Point[114],szTempo,0,iDsk);
                            /* 10.03f >> */
                            /* 10.03d >> */

						if(choix == 'O')
						{
							iCpt = iDsk;
							bIndic = true;
							bExit  = true;
						}
					}

				}
				else
				{
					bIndic = false;
					bExit  = true;
				}
			}
		}

		/* << 10.03c */
		/*case only whether the first Crc_Rijndael return -1*/
        /* << 10.03d */
        /*if((iExitCpt == 0) && (iCpt <= iMAXDISK))
            Battery_On();*/
        /* 10.03d >> */
        /* 10.03c >> */

    }
    while(iCpt <= iMAXDISK);

    /* if start disk number is up to 1 */
    if(iDif > 0)
    {
       for(iCpt = 1; iCpt <= iDif; iCpt++)
       {
			iDsk = 0;
			choix = ' ';
			bExit = false;

			/* << 10.03c */
			/* << 10.03d */
            //Battery_Off();
            /* 10.03d >> */
            /* 10.03c >> */

			iCount = 0;
			while(!bExit)
			{
				if(iCount < 3)
				{
					iCount++;
					sprintf(szTempo,Message_Point[117],iCpt);
					Lcd_Affiche(0,0, szTempo);
					Lcd_Affiche(1,0, Message_Point[64]);

                    choix = ' ';
                    while(choix != 'O')
                        /* << 10.03d */
                        //choix = Getch(COLOR);
                         /* << 10.03f */
                        /*choix = Getchar(0x30,Message_Point[125],1,0);*/
                        choix = Getchar(COLOR,Message_Point[125],Message_Point[64],1,0);
                        /* 10.03f >> */
                       /* 10.03d >> */

					iDsk = Disk_Num();
					if(iDsk == iCpt)
					{
						bIndic = true;
						bExit  = true;
					}
					else if((iDsk > 0) && (iDsk <= iMAXDISK) && (iDsk > iCpt) && (iCpt < iDif))
					{
						sprintf(szTempo, Message_Point[118],iDsk);
						Lcd_Affiche(0,0, szTempo);
						Lcd_Affiche(1,0, Message_Point[35]);

						choix = ' ';
						while((choix != 'O') && (choix != 'N'))
							/* << 10.03d */
                            //choix = Getch(COLOR);
                            /* << 10.03f */
                            /*choix = Getchar(0x30,Message_Point[114],0,iDsk);*/
                            choix = Getchar(COLOR,Message_Point[114],szTempo,0,iDsk);
                            /* 10.03f >> */
                            /* 10.03d >> */

						if(choix == 'O')
						{
							iCpt = iDsk;
							bIndic = true;
							bExit  = true;
						}
					}

				}
				else
				{
					bIndic = false;
					bExit  = true;
				}
			}

			if(bIndic)
			{
				Lcd_Affiche(0,0,Message_Point[12]);
                Lcd_Affiche(1,0," ");

				remove("C:TABLE.URN");
				remove("a:TABLE.URN");
                remove("a:BE.3");

                /* << 10.03c */
                /*case only whether the first Crc_Rijndael return -1*/
                /* << 10.03d */
                /*if(iExitCpt == 0)
                    Battery_On();*/
                /* 10.03d >> */
                /* 10.03c >> */

                Transfert_Table_PCC("C:TABLE.URN");


				/* << 10.03c */
                /* << 10.03d */
                /*if(iExitCpt > 0)
                    Battery_On();*/
                /* 10.03d >> */
                /* 10.03c >> */
                /*      write crc      */
				iRes = Crc_Rijndael("W","3","c:","a:");
				if(iRes == -1)
				{
					/* << 10.02b */
					/*Lcd_Affiche(0,0, Message_Point[27]);*/
					Lcd_Affiche(0,0, Message_Point[122]);
                    /* 10.02b >> */
					Lcd_Affiche(1,0, Message_Point[14]);
					/* << 10.03d */
                    //Getch(COLOR);
                    /* << 10.03f */
                    /*Getchar(0x30,Message_Point[84],1,0);*/
                    Getchar(COLOR,Message_Point[84],Message_Point[14],1,0);
                    /* 10.03f >> */
                    /* 10.03d >> */
				}
				/* << 10.02h */
                else
                {
                    iExitCpt++;
                    /* << 10.03f */
                    if((iExitCpt == 1) && (SUPPORT == 'E'))
                    {
                        Encode_E_Variable((long)_E_FERMETURE, true);
                    }
                    /* 10.03f >> */
                    //the code below is used for certain elections
                    if(iExitCpt == 2)
                        break;
                }
                /* 10.02h >> */
			}

       }
    }

}
/* 10.01 >>*/
/****************************************************************************/
/* << 10.03g */
int Check_NumDisk(int iNum)
{
    int iRes;
    boolean bCanLoop;
    char choix;

    iRes = iNum;
    if((iNum >= 1) && (iNum <= iMAXDISK))
        bCanLoop = false;
    else
        bCanLoop = true;

    while(bCanLoop)
    {
        Lcd_Affiche(0,0,Message_Point[116]);
        Wait_Time(3);
        Lcd_Affiche(0,0,Message_Point[25]);
        Lcd_Affiche(1,0,Message_Point[64]);

        choix = Getch(COLOR);
        if(choix == 'O')
        {
            iRes = Disk_Num() ;
            if((iRes >= 1) && (iRes <= iMAXDISK))
                bCanLoop = false;
        }

        Lcd_Affiche(0,0,"");
        Lcd_Affiche(1,0,"");
    }

    return iRes ;
}
/* 10.03g >> */

/***************************************************************************/
/* << 10.01 */
/*int Fermeture_Urne(char *NomFichier)*/
int Fermeture_Urne()
/* 10.01 >> */
/*
	Ferme l'urne
	Parametre: nom du fichier fourni par le PC de controle
	Rend le test sur la fermeture
*/
{
  /* << 10.01 */
  /*int test, i, Erreur;*/
  int test, i, iNum;
  boolean bExit, bExitLoop, bCanSave;
  /*int Clef_Id, fd2;*/
  /* 10.01 >> */
  char Clef[11];
  char choix;
  char tab_Epror[10], strtmp[21];
  /* << 10.01 */
  /*FILE *Fichier_BK;*/
  /* 10.01 >> */

/* << 10.03b */
  Maj_IvBit(1);
/* 10.03b >> */
  Lcd_Affiche(0,0, Message_Point[45]);
  Lcd_Affiche(1,0, Message_Point[21]);
  Wait_Time(1);
   /* << 10.04 */
  /*choix = Getch(COLOR);*/

  while(true)
  {
      choix = Getch(COLOR);
      if(choix == '8')
      {
          Battery_Off();
          Lcd_Affiche(0,0, Message_Point[46]);
          Lcd_Affiche(1,0, Message_Point[47]);
          Boucle_Erreur();
      }
      /* << 10.01 */
      /*while((choix != 'O') && (choix != 'N'))*/
      else if((choix == 'O') || (choix == 'N'))
      {
          break;
      }
  }
  /* 10.04 >> */

  /*if ((choix == 'O') && (NOMBRE_VOTES > 10))*/
  /* << 10.02b */
   /*if (((choix == 'O') || (choix == 'J')) && (NOMBRE_VOTES > 10))*/
   if ((choix == 'O') && (*piNOMBRE_VOTES1 > 10))
  /* 10.02b >> */
  /* 10.01 >> */
    {
      COLOR = _ON_NO_LED;
      Light_LED(COLOR);
      test = false;
      /* << 10.01 */
      bExit = false;
      /* 10.01 >> */

      /* << 10.03c */
      Battery_Off();
      /* 10.03c >> */

      for (i = 0; i < 3 && (! test); i++)
	 {
	   test = Demande_Identite(Clef,false);
       if(!test)
       {
            bExit = true;
            break;
       }
       else
	   /*  */
            test = Verifie_Urne( Clef);
	 }


      if (test)
	 {
/* START CONFIDENTIAL */
	   Recupere_E_Var_String(400L, tab_Epror, 5);
	   if((strcmp("JITES",tab_Epror)) != 0)
/* END CONFIDENTIAL */
		   Ram_Eprom_Load();

       Lcd_Affiche(0,0, Message_Point[48]);
       Lcd_Affiche(1,0, Message_Point[49]);

/* START CONFIDENTIAL */
    /* << 10.01 */
		i = 0;
        /* 10.03b */
        bExitLoop = false;
        bCanSave = true;
        /* 10.03b */
        while(!bExitLoop && (i < 3))
        {
            iNum = Disk_Num() ;

            if(iNum != iDiskId)
            {
                sprintf(strtmp,Message_Point[117],iDiskId);
                Lcd_Affiche(0,0, strtmp);
                Lcd_Affiche(1,0,Message_Point[64]);

                choix = ' ';
                while(choix != 'O')
                    choix = Getch(COLOR);

                i += 1;
            }
            else
                bExitLoop = true;

        }


        /* << 10.03b */
        /* << 10.03g*/
        /* while(iNum == 0)
        {
            Lcd_Affiche(0,0,Message_Point[116]);
            Wait_Time(3);
            Lcd_Affiche(0,0,Message_Point[25]);
            Lcd_Affiche(1,0,Message_Point[64]);

            choix = Getch(COLOR);
            if(choix == 'O')
            {
                iNum = Disk_Num() ;
            }

            Lcd_Affiche(0,0,"");
            Lcd_Affiche(1,0,"");

        }*/

        /* 10.03b >>  */

        /*iDiskId = iNum;*/
        iDiskId = Check_NumDisk(iNum);
        /* 10.03g >> */

        /* << 10.03b */

        /* << 10.03c */
        Battery_On();
        /* 10.03c >> */

        if(SUPPORT == 'E')
        {
            if(Compare_Ram_Eprom() != 0)
                bCanSave = false;
        }

        if(SUPPORT == 'R')
        {
            if((access(Chemin_D, 0) == 0) && bExitLoop)
            {
                if(Compare_Ram_Disk() != 0)
                  bCanSave = false;

            }
            else
            {
                Ram_Disk_Load();
            }
        }

        /*Lcd_Affiche(0,0, Message_Point[48]);
        Lcd_Affiche(1,0, Message_Point[49]);*/

        if(bCanSave)
        {
            Lcd_Affiche(0,0, Message_Point[48]);
            Lcd_Affiche(1,0, Message_Point[49]);

            if(SUPPORT == 'R')
                Encode_E_Variable((long)_E_FERMETURE, true);
        }
        else
        {
            Lcd_Affiche(0,0, Message_Point[124]);
            Lcd_Affiche(1,0, Message_Point[7]);
            /* << 10.03c */
            Battery_Off();
            /* 10.03c >> */
            Boucle_Erreur();
        }

        /* 10.03b >>*/

		/*if(Disq_BK == 0)
		   {
		     while(Test_Bk_Aut())
		      {
			putch(0x07);
			sprintf(strtmp, "");
			Lcd_Affiche(0,0, strtmp);
			Lcd_Affiche(1,0, strtmp);
			Lcd_Affiche(0,0, Message_Point[90]);
			Lcd_Affiche(1,0, Message_Point[64]);
			choix = ' ';
			while(choix != 'O')
				choix = Getch(COLOR);
		      }
		   }
		 else
		   {
		     while(!Test_Bk_Aut())
		      {
			putch(0x07);
			sprintf(strtmp, "");
			Lcd_Affiche(0,0, strtmp);
			Lcd_Affiche(1,0, strtmp);
			Lcd_Affiche(0,0, Message_Point[63]);
			Lcd_Affiche(1,0, Message_Point[64]);
			choix = ' ';
			while(choix != 'O')
			   choix = Getch(COLOR);
		      }
		    }
		   Lcd_Affiche(0,0, Message_Point[48]);
		   Lcd_Affiche(1,0, Message_Point[49]);

		   remove("a:TABLE.URN");

		   Transfert_Table_PCC("C:TABLE.URN");
		   Additionne = true; */

		   /*fd2 = open("a:BE.INI", O_RDONLY | O_BINARY);
		   if(fd2 == -1)
		   {
		    close(fd2);
		    rename("a:BE","BE.INI");
		   }
		  close(fd2);
		  remove("a:BE");*/

/* END CONFIDENTIAL */

/*                 Ecrit du crc      */
		   /*i = Crc_Rijndael("W","3","c:","a:");
		   if(i == -1)
		     {
		       Lcd_Affiche(0,0, Message_Point[27]);
		       choix = Getch(COLOR);
		     }*/

/*                  Fin Ecrit du Crc   */

		   /*if(SUPPORT == 'R')*/
/* START CONFIDENTIAL */
		   /*rename(Chemin_D,"FE_Dsk.BK");*/
/* END CONFIDENTIAL */
   /* 10.01 >> */
          /* << 10.03e */
		   /*COLOR = _OFF_NO_LED;*/
          /* 10.03e >> */
		   return(true);
	 }
	 /* << 10.01 */
	 else if(!bExit)
     {
		Lcd_Affiche(0,0, Message_Point[54]);
		Lcd_Affiche(1,0, Message_Point[14]);
		Getch(COLOR);
		/* << 10.03c */
        Battery_On();
        /* 10.03c >> */
     }
	 /* 10.01 >> */
    }

  /* << 10.01 */
  /* << 10.02b */
  /*if (((choix == 'O') || (choix == 'J')) && (NOMBRE_VOTES < 11))*/
  if ((choix == 'O') && (*piNOMBRE_VOTES1 < 11))
  /* 10.02b >> */
  {
	  sprintf(strtmp,Message_Point[119],11);
	  Lcd_Affiche(0,0, strtmp);
      Lcd_Affiche(1,0, Message_Point[120]);
      Wait_Time(4);
  }
  /* 10.01 >> */

return(false);
} /* Fermeture */

/***************************************************************************/
int Affiche_Result()
{
 char strtmp[21];

 if(MULTI_ELEC == 'N')
 {
/* << 10.02b */
    /*if(NOMBRE_VOTES > 1)
	 sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
    else
	 sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);*/
    /* << 10.03e */
      switch(rang)
      {
          case 0 :
                if((NOMBRE_VAL) > 1)
                    sprintf(strtmp,Message_Point[108] , NOMBRE_VAL);
                else
                    sprintf(strtmp,Message_Point[109] , NOMBRE_VAL);
                rang++;
          break;

          case 1 :
                if(*piNOMBRE_VOTES1 > 1)
                    sprintf(strtmp,Message_Point[88] , *piNOMBRE_VOTES1);
                else
                    sprintf(strtmp,Message_Point[110] , *piNOMBRE_VOTES1);
                rang = 0;
          break;
      }
    /* 10.03e >> */
/* 10.02b >> */
     Lcd_Affiche(0, 0, strtmp);
 }
 else
 if((GLOBAL_EURO == 1) && (GLOBAL_STR == 0))
   {
    switch(rang)
     {
	case 0 :
        /* << 10.02b */
        /*if(NOMBRE_NATIO > 1)
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
         /*if(NOMBRE_EURO > 1)
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
          /*if(NOMBRE_NATIO > 1)
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
          /*if(NOMBRE_STR > 1)
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
             /*if(NOMBRE_NATIO > 1)
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
             /*if(NOMBRE_EURO > 1)
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
             /*if(NOMBRE_STR > 1)
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
 _WAITL;
 return(0);
}
/***************************************************************************/

int Demande_Action()
/*
	Premier menu
	Parametre: sans
	Rend le choix
*/
{
	int touche;

  /* << 10.02 */
    if (ETAT_URNE != 3)
    {
  /* 10.02 >> */
        Lcd_Affiche(0,0, Message_Point[51]);
        Lcd_Affiche(1,0, Message_Point[52]);
        touche = Getch(COLOR);
  /* << 10.02 */
    }
    else
        touche = '1';
  /* 10.02 >> */
	return(touche);
} /* Demande_Action */

/****************************************************************************/

main()
{
  int i,j, Action, nbvotes,fd2;
  boolean test, testaction;
  boolean Erreur;
  char Clef[12], strtmp[21],stat,cport,c;
  char tab_Epror[10];
  char Jeton_E[_JETON_ETENDU_MAX_BYTE];
  char choix;
  /* << 10.01 */
  int iLang_ID;
  /* 10.01 >> */
  FILE *Fichier_BK;
  /* << 10.02 */
  char *pzScrutin;
  /* 10.02 >>*/

/* << 10.03b */
  Read_Be2();
/* 10.03b >> */
/* START CONFIDENTIAL */
  cport = inp(0x61);       /*           PARITEE            */
  cport = cport | 0x08;    /*                              */
  outp(0x61,cport);        /*                              */
/* END CONFIDENTIAL */
 /* << 10.02b */
  /*NOMBRE_VOTES      = 0;*/
 /* 10.02b >> */
  /* << 10.01 */
  iMAXDISK = 4;
  /* 10.01 >> */
  SAUVE_TO_EPROM    = false;
  DEMANDE_FERMETURE = false;
  Additionne = false;
  SUPPORT = 'E';
  COLOR   = _OFF_NO_LED;
  /* << 10.02 */
  ETAT_URNE = 0;
  /* 10.02 */

  /* << 10.03g */
  iERRORCLOSE = false;
  /* 10.03g >> */

/* START CONFIDENTIAL */

  strcpy(Chemin_R,"c:\FE_Ram");
  strcpy(Chemin_D,"a:\FE_Dsk");

/* END CONFIDENTIAL */
  _harderr(handler);
  /* << 10.01 */
  /*Choix_Langue(1);*/
  iLang_ID = Language_ID();
  if(iLang_ID == -1)
  {
    Lcd_Affiche(0,0, "INIT. ERROR CODE -1");
	Lcd_Affiche(1,0, "PREVENT ASSISTANCE");
	Boucle_Erreur();
  }
  Load_Message(iLang_ID);

  /* << 10.04 */
  /*iDiskId = Disk_Num() ;*/
  iDiskId = Get_DiskID();
  /* 10.04 >> */
  if((iDiskId == 0) || (iDiskId > iMAXDISK))
  {
    Lcd_Affiche(0,0, Message_Point[116]);
    Lcd_Affiche(1,0, Message_Point[7]);
    Boucle_Erreur();
  }
  /* 10.01 >> */

  /* << 10.02b */
  if (!Alloc_Mem())
  {
    Lcd_Affiche(0,0, Message_Point[121]);
    Lcd_Affiche(1,0, Message_Point[7]);
    Boucle_Erreur();
  }
  /* 10.02b >> */

/***********/

  while((Lit_Fichier_ARR("A:URNE.TBL")) != _PAS_ERREUR)
/* END CONFIDENTIAL */
     {
	Lcd_Affiche(0, 0, Message_Point[25]);
	Lcd_Affiche(1,0,  Message_Point[14]);
	Getch(COLOR);
     }
  /* << 10.01 */
  /*Choix_Langue(2);*/
  /* 10.01 >> */

/***********/

  /* << 10.03c */
  /*Disq_BK = Test_Bk();*/
  /* 10.03c >> */
  for (i = 0, test = false; (i < 3) && (! test); i++)
	 {
		/* << 10.02e */
		/*Demande_Identite(fullPassword);*/
		Demande_Identite(fullPassword,true);
		/* 10.02e >> */
		if(Verifie_Password())
		 {
		  /* the indices of the string fullPassword were modified from
		    the version 10.02 */
		  strcpy(CLEF_URNE,fullPassword);
		  /* << 10.02e */
		  /*CMinipassword[0] = fullPassword[1]; /0/
		  CMinipassword[1] = fullPassword[3]; /2/
		  CMinipassword[2] = fullPassword[4];
		  CMinipassword[3] = fullPassword[7]; /6/*/
		  CMinipassword[0] = fullPassword[MINI_POS[0]-49];
		  CMinipassword[1] = fullPassword[MINI_POS[1]-49];
		  CMinipassword[2] = fullPassword[MINI_POS[2]-49];
		  CMinipassword[3] = fullPassword[MINI_POS[3]-49];
	/* << 10.02e */
		  CMinipassword[4] = 0x00;
		  strcat(CMinipassword,Minicodage);

		  while(!Demande_Mac());
		  /* << 10.03c */
		  /*if(Disq_BK == 0)
          {
			sprintf(strtmp, "");
			Lcd_Affiche(0,0, strtmp);
			Lcd_Affiche(1,0, strtmp);*/
			/* << 10.01*/
			/*if(access("a:urne.tbl", 0) != 0)
			{*/
			/* 10.01 >> */
			/*Lcd_Affiche(0,0, Message_Point[90]);
			Lcd_Affiche(1,0, Message_Point[64]);*/
			/* << 10.01*/
			/*choix = ' ';
		    while(choix != 'O')
			choix = Getch(COLOR);
            }*/
            /* 10.01 >> */
          /*}
          else
          {
			sprintf(strtmp, "");
			Lcd_Affiche(0,0, strtmp);
			Lcd_Affiche(1,0, strtmp);
			Lcd_Affiche(0,0, Message_Point[63]);
			Lcd_Affiche(1,0, Message_Point[64]);*/
			/* << 10.01*/
			/*choix = ' ';
		    while(choix != 'O')
			choix = Getch(COLOR);*/
			/* 10.01 >> */
          /*}*/
          /* 10.03c >> */

		  /* << 10.01 */
          /*choix = ' ';
          while(choix != 'O')
			choix = Getch(COLOR);*/
          Lcd_Affiche(0,0, Message_Point[12]);
          Lcd_Affiche(1,0,"");
          /* 10.01 >> */
		   while((access("a:urne.tbl", 0)) != 0);

		   /* << 10.01 */
		   /*
		   Lcd_Affiche(0,0, Message_Point[12]);
		   Lcd_Affiche(1,0,"");
		   */
		   /* 10.01 >> */

		   j = Crc_Rijndael("r","2","a:","a:");
		   strcpy(strtmp ,"");
		   if(j == -1)
		      test = false;
		   else
		      test = true;
		 }
	 }
  if(!test)
	 {
		Lcd_Affiche(0,0, Message_Point[54]);
		Boucle_Erreur();
	 }


/* START CONFIDENTIAL */
  while((Lit_Fichier_PCC("C:URNE.TBL")) != _PAS_ERREUR)
/* END CONFIDENTIAL */
     {
	Lcd_Affiche(0, 0, Message_Point[3]); /* ERREUR RAM */
	Lcd_Affiche(1,0,  Message_Point[7]); /* PREVENIR ASSISTANCE */
	Getch(COLOR);
     }
  Lcd_Affiche(0,0, Message_Point[53]);
  Lcd_Affiche(1,0,"");
  Init_Systeme();
  Witch_Support();
  /* << 10.04 */
  if(SUPPORT == 'E')
  /* 10.04 >> */
    Test_Eprom();

  Lecteur_Put_Tempo();
  Battery_Ok();  /* Test l'etat des Batteries */

  Recupere_E_Var_String(_E_JETON_ETENDU, Jeton_E,_JETON_ETENDU_MAX_BYTE);
  Recupere_E_Variable(_E_FERMETURE, &FERMETURE);
  Recupere_E_Variable(_E_OUVERTURE, &OUVERTURE);
  Recupere_Nb_Cartes();

/* << 10.02 */
  /*if ((FERMETURE || OUVERTURE ) && (strcmp(JETON_ETENDU, Jeton_E) != 0))
			 Test_Contenu_Urne2();*/
  if (FERMETURE || OUVERTURE )
  {
      pzScrutin = (char *)malloc(strlen(Buffer_Par));
      if(pzScrutin == NULL)
      {
          Lcd_Affiche(0,0, Message_Point[121]);
          Lcd_Affiche(1,0, Message_Point[7]);
          Boucle_Erreur();
      }
      Recupere_E_Var_String((long)_E_PAR_PLACE,pzScrutin,strlen(Buffer_Par));

      if ((strcmp(JETON_ETENDU, Jeton_E) != 0) || (strcmp(Buffer_Par, pzScrutin) != 0))
                 Test_Contenu_Urne2();

      if(pzScrutin != NULL)
      {
        free(pzScrutin);
        pzScrutin = NULL;
      }
  }

/* 10.02 >> */

  Recupere_E_Variable(_E_FERMETURE, &FERMETURE);

  if ( FERMETURE )
    {
     /* << 10.03e */
     Recupere_Nb_Validations();
     /* 10.03e >> */
     for( rang = 0;true;)
        {
          Affiche_Result();
          Lcd_Affiche(1,0, Message_Point[57]);
          Wait_Time(3);
          if((c = Kbhit(COLOR,2000)) == '2')
                  Ecriture_force();
        }
    }

/*  Disq_BK = Test_Bk();*/
  for(testaction = true; testaction;)
   {
     Recupere_E_Variable(_E_FERMETURE, &FERMETURE);
     Recupere_E_Variable(_E_OUVERTURE, &OUVERTURE);
     Recupere_Nb_Cartes();

     Action = Demande_Action();
     if(Action == '1')
      {
       if (! OUVERTURE)
	   {
	     Ouverture_Urne();
	     testaction = false;
	   }
       else
	  {
	    Test_Contenu_Urne();
	    if(ETAT_URNE == 1)
	       {
		 Lcd_Affiche(0,0,Message_Point[39]);
		 Lcd_Affiche(1,0,Message_Point[40]);
		 testaction = false;
	       }
	  }
      }
   else
      {
	Lcd_Affiche(0, 0, Message_Point[61]);
	Lcd_Affiche(1, 0," ");
	Wait_Time(4);
      }
   }

 Calcul_Jeton(JETON_ETENDU, JETON);
 test = true;
 Battery_On();
 COLOR = _ON_GREEN;
/* << 10.02b */
 /*Recupere_Nb_Validations();*/
/* 10.02b >> */

 Lcd_Affiche(0,0,"");

 while (test)
	{
	   Boucle_Lecteur();
/* START CONFIDENTIAL */
       /* << 10.01 */
	   /*test = ! Fermeture_Urne("a:URNE.TBL");*/
	   test = ! Fermeture_Urne();
	   /* 10.01 >> */
/* END CONFIDENTIAL */
	}
/* << 10.02b */
 /*Recupere_Nb_Cartes();*/
/* 10.02b >> */
/* << 10.03f */
COLOR = _ON_GREEN;
/* 10.03f >> */
 Write_Data_Disk(iDiskId);
/* << 10.01 */
/*
 if (NOMBRE_VOTES > 0)
   {
    if(NOMBRE_VOTES > 1)
	 sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
    else
	 sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);
    Lcd_Affiche(0, 0, strtmp);
   }
 else
   {
    sprintf(strtmp, "");
    Lcd_Affiche(0,0, strtmp);
   }

 Lcd_Affiche(1,0," ");
 Wait_Time(4);

 if(BK_PC == 2)
  {
     sprintf(strtmp, "");
     Lcd_Affiche(0,0, strtmp);
     Lcd_Affiche(1,0, strtmp);
     Lcd_Affiche(0,0, Message_Point[60]);
     Lcd_Affiche(1,0, Message_Point[64]);
     if(Disq_BK == 0)
      {
       while(!Test_Bk_Aut())
	{
	  putch(0x07);
	  choix = ' ';
	  while(choix != 'O')
		  choix = Getch(COLOR);
	  }
      }
     else
      {
       while(choix != 'O')
	    choix = Getch(COLOR);
      }
     Lcd_Affiche(0,0, Message_Point[48]);
     Lcd_Affiche(1,0, Message_Point[62]);
   */

/* START CONFIDENTIAL */
	  /*remove("a:TABLE.URN");
	  while((Ecrit_Table_PCC("C:TABLE.URN")) != _PAS_ERREUR)
		 {
		  Lcd_Affiche(0,0, Message_Point[25]);
		  Lcd_Affiche(1,0, Message_Point[14]);
		  Getch(COLOR);
		 }*/
/* END CONFIDENTIAL */

/*     Ecrit du crc      */
	/*
	fd2 = open("a:BE.INI", O_RDONLY | O_BINARY);
	if(fd2 == -1)
	 {
	   close(fd2);
    */
/* START CONFIDENTIAL */
	//   rename("a:BE","BE.INI");
/* END CONFIDENTIAL */
	 //}
/* START CONFIDENTIAL */
	 /*remove("a:BE");
	 close(fd2);*/

/* END CONFIDENTIAL */
	/*i = Crc_Rijndael("W","3","c:","a:");
	if(i == -1)
	 {
	   Lcd_Affiche(0,0, Message_Point[27]);
	   choix = Getch(COLOR);
	 }*/
/*     Fin Ecrit du Crc   */
/*
  }

 Lcd_Affiche(0,0," ");
 if(NOMBRE_VOTES > 1)
	 sprintf(strtmp,Message_Point[88] , NOMBRE_VOTES);
 else
	 sprintf(strtmp,Message_Point[110] , NOMBRE_VOTES);
 Lcd_Affiche(0, 0, strtmp);
 Lcd_Affiche(1,0," ");

 Wait_Time(4);
 if(BK_PC != 2)
  {
   if(Disq_BK == 0)
      {
	while(!Test_Bk_Aut())
	 {
	   putch(0x07);
	   sprintf(strtmp, "");
	   Lcd_Affiche(0,0, strtmp);
	   Lcd_Affiche(1,0, strtmp);
	   Lcd_Affiche(0,0, Message_Point[63]);
	   Lcd_Affiche(1,0, Message_Point[64]);
	   choix = ' ';
	   while(choix != 'O')
	     choix = Getch(COLOR);
	 }
      }
   else
     {
	   putch(0x07);
	   sprintf(strtmp, "");
	   Lcd_Affiche(0,0, strtmp);
	   Lcd_Affiche(1,0, strtmp);
	   Lcd_Affiche(0,0, Message_Point[90]);
	   Lcd_Affiche(1,0, Message_Point[64]);
	   choix = ' ';
	   while(choix != 'O')
	     choix = Getch(COLOR);
      }
  }

 Lcd_Affiche(0,0, Message_Point[48]);
 if(BK_PC == 2)
      Lcd_Affiche(1,0, Message_Point[65]);
 else
      Lcd_Affiche(1,0, Message_Point[66]);

/* START CONFIDENTIAL */
 /*remove("a:TABLE.URN");

 while((Ecrit_Table_PCC("C:TABLE.URN")) != _PAS_ERREUR)
		{
		  Lcd_Affiche(0,0, Message_Point[25]);
		  Lcd_Affiche(1,0, Message_Point[14]);
		  Getch(COLOR);
		}
*/
/* END CONFIDENTIAL */

/* START CONFIDENTIAL */

/*     Ecrit du crc      */
	/*
	fd2 = open("a:BE.INI", O_RDONLY | O_BINARY);
	if(fd2 == -1)
	 {
	   close(fd2);
	   rename("a:BE","BE.INI");
	 }
	 close(fd2);
	 remove("a:BE");
	 */


/* END CONFIDENTIAL */

	/*i = Crc_Rijndael("W","3","c:","a:");
	if(i == -1)
	 {
	   Lcd_Affiche(0,0, Message_Point[27]);
	   choix = Getch(COLOR);
	 }*/

/* 10.01 >> */

/*     Fin Ecrit du Crc   */
 /* << 10.03g */
 /*Wait_Time(1);*/
 Lcd_Affiche(0,0, Message_Point[12]);
 Lcd_Affiche(1,0, " ");
 /* 10.03g >> */

 /*<< 10.03c */
 if(SUPPORT == 'E')
 /* 10.03c >> */
 {
    /* << 10.03g */
    Recupere_E_Variable(_E_FERMETURE, &FERMETURE);
    if (!FERMETURE)
    {
        Encode_E_Variable((long)_E_FERMETURE, true);
        iERRORCLOSE = true;
    }
    /* 10.03g >> */
 }

/*<< 10.03d */
 //Battery_Off();
/* 10.03d >> */

/* << 10.03e */
 Recupere_Nb_Validations();
/* 10.03e >> */
 Lcd_Affiche(0,0," ");

 for(i = 0, rang = 0;true; i++)
 {
    Affiche_Result();
    /* << 10.03g */
    if(!iERRORCLOSE)
    /* 10.03g >> */
        Lcd_Affiche(1,0, Message_Point[67]);
    /* << 10.03g */
    else
        Lcd_Affiche(1,0, Message_Point[90]);
     /* 10.03g >> */

    Wait_Time(3);
    /* << 10.03e */
    if(i < 10)
    {
        Check_Relais();
    }
    else if(i == 10)
    {
        Battery_Off();
        COLOR = _OFF_NO_LED;
    }
    /* 10.03e >> */

    if((c = Kbhit(COLOR,2000)) == '2')
        Ecriture_force();
 }

} /* main */

/*******************************************************************/
/*******************************************************************/

void Ecriture_force()
{

 char strtmp[21],choix,c;
 int  i, fd2;

 Lcd_Affiche(0,0, Message_Point[58]);
 Lcd_Affiche(1,0, Message_Point[64]);
 choix = ' ';
 while(choix != 'O')
       choix = Getch(COLOR);

 Lcd_Affiche(0,0, Message_Point[12]);
 Lcd_Affiche(1,0, Message_Point[13]);
/* START CONFIDENTIAL */

 /* << 10.01 */
 /*remove("a:BE");*/
 remove("C:TABLE.URN");
 remove("a:BE.3");
 /* 10.01 >> */
 remove("a:TABLE.URN");
 remove("a:URNE.IND");

 Down_Load_Ind();
/* << 10.01 */
 /*
 if(Additionne == false)
	 {
	  Transfert_Table_PCC("C:TABLE.URN");
	  Additionne = true;
	 }
 else
   while((Ecrit_Table_PCC("C:TABLE.URN")) != _PAS_ERREUR)
		{
		  Lcd_Affiche(0,0, Message_Point[25]);
		  Lcd_Affiche(1,0, Message_Point[14]);
		  Getch(COLOR);
		}
  */
/* END CONFIDENTIAL */
/* START CONFIDENTIAL */

/*     Ecrit du crc      */
	/*
	fd2 = open("a:BE.INI", O_RDONLY | O_BINARY);
	if(fd2 == -1)
	 {
	   close(fd2);
	   rename("a:BE","BE.INI");
	 }
	 close(fd2);
	 remove("a:BE");
	 */
/* 10.01 >> */

/* END CONFIDENTIAL */
   Transfert_Table_PCC("C:TABLE.URN");
/* 10.01 >> */
	i = Crc_Rijndael("W","3","c:","a:");
	if(i == -1)
	/* << 10.02b */
	{
	   /*while(1)
        Lcd_Affiche(0,0, Message_Point[27]);*/
        Lcd_Affiche(0,0, Message_Point[122]);
        Lcd_Affiche(1,0, Message_Point[14]);
        Getch(COLOR);
        Lcd_Affiche(0,0, "");
        Lcd_Affiche(1,0, "");
	}
	else
	{
	    Lcd_Affiche(0,0, Message_Point[82]);
        Lcd_Affiche(1,0, Message_Point[14]);
        Getch(COLOR);
        Lcd_Affiche(0,0, "");
        Lcd_Affiche(1,0, "");

        /* << 10.03g */
        if(iERRORCLOSE)
            iERRORCLOSE = false;
        /* 10.03g >> */
	}
    /* 10.02b >> */
/*     Fin Ecrit du Crc   */

}
/*******************************************************************/
void Battery_On()
/*
	 Mettre les batteries on
	 Parametre: sans
	 Rend void
*/
{
  outp(0x3E0,0x30);
} /* Recupere */
/*******************************************************************/
void Battery_Off()
/*
	 Mettre les batteries off
	 Parametre: sans
	 Rend void
*/
{
  outp(0x3E0,0x00);
} /* Recupere */
/*******************************************************************/
/*******************************************************************/

void Battery_Ok()
/*
	 Etat des Batteries
	 Parametre: sans
	 Rend void
*/
{
  char c,choix;
  outp(0x3E0,0x20);
  Wait_Time(2);
  c = inp(0x3E0);
  if(c  == 0x1F)
	 {
		Lcd_Affiche(0,0, Message_Point[68]);
		Lcd_Affiche(1,0," ");
		outp(0x3E0,0x00);
		BATTERY = 1;
	 }
  else
	 {
		Lcd_Affiche(0,0,Message_Point[69]);
		Lcd_Affiche(1,0,Message_Point[35]);
		outp(0x3E0,0x00);
		choix = ' ';
		BATTERY = 0;
		while((choix != 'O') && (choix != 'N'))
				choix = Getch(COLOR);
		if (choix == 'N')
	 {
	  Lcd_Affiche(0,0, Message_Point[70]);
	  Lcd_Affiche(1,0, Message_Point[71]);
	  Boucle_Erreur();
	 }
	  }
} /* Recupere */

/*******************************************************************/
/*******************************************************************/

int Demande_Mac()
{
  /* << 10.03d*/
  //char choix;
  /* 10.03d >> */
  int test;

/* << 10.03d*/
  /*if((access("a:Floppy.be", 0)) != 0)
  {

      Lcd_Affiche(0,0, Message_Point[114]);
      Lcd_Affiche(1,0, Message_Point[64]);

      choix = ' ';

      while(choix != 'O')
           choix = Getch(COLOR);
      choix = ' ';

  }*/

  if((access("a:Floppy.be", 0)) == 0)
    {

	 test = Initialise_Rijndael();
	 Initialise_RijndaelMini();
	 if (test)
		return(true);
	  else
		return(false);

  }
/* 10.03d >> */

return(false);
}

/*******************************************************************/
/*  FIN DU MODULE PRINCIPAL URNE.C                                 */
/*******************************************************************/
