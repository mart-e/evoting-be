/***************************************************************************/
/* JITES                                                                   */
/* prog AL-KHANSA JAMAL                                                    */
/* rvis  partir du 10-9-1992 comme maquette bxls                        */
/* LE 6-2-1991                                                             */
/* LANGAGE TC                                                              */
/*                                                                         */
/*                                VARIABLES                                */
/* Hcadre_candid = hauteur du cadre candidat                               */
/* Interl_candid = Interligne entre les cadres candidats                   */
/* Hsaisie_candid= hauteur de la zone de saisie dans cadre candidat        */
/* Lsaisie_candid = longueur de la zone de saisie candidat                 */
/* Lcadre_candid  = longueur du cadre candidat                             */
/* Interc_candid  = Intercolonne entre les cadres candidats                */
/* x_dep          = valeur de la coordonnee x de la  1 ere colonne         */
/* x_ent_scr      = valeur de x de l'entete du scrutin                     */
/* x_numl         = valeur de x du numero de la liste                      */
/* x_noml         = valeur de x du nom de la liste                         */
/* nbc_col        = nombre de candidats par colonne                        */
/* h    = hauteur à partir de l'entete                                     */
/* lon  = longueur d'une case liste                                        */
/* espl = espacement entre les cases listes                                */
/* recl = hauteur d'une case liste                                         */
/* nbc  = nombre de candidats / liste                                      */
/* recc = hauteur d'une case candidat                                      */
/* memos[3] = tableau des scrutins                        -                */
/* memol[3][40] = tableau des listes / scrutin            - MEMORISATION   */
/* memoc[3][40][100] = tableau des candidats /liste/scrutin -               */
/***************************************************************************/

#include <fcntl.h>
#include <alloc.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <graphics.h>
#include <string.h>
/* << LAH */
#include <fcntl.h>
/* LAH >> */
/* << DIGI */
char JITES = 0;
char DIGIVOTE = 0;
/* DIGI >> */

#include "variable.h"
#include "jamal.c"
#include "pcx.c"
#include "bmp.c"

union REGS rin,rout;

#define  NMAX_MSG  99
#define  LMAX_MSG  70
char Msg[1+NMAX_MSG][LMAX_MSG+1];
char gMsg[(LMAX_MSG *2) +1];
char bufname[50];

char bufc[100],caisse[30],caisse1[30],str_nbre_max[4];
int nbrl,nbr_lignes,Cl;
FILE *fpin, *fpprn;
FILE *ptc, *fpc;

char               fnin[255+1] = "listes";
char               fnprn[255+1] = "prn";
FILE               *fpin = NULL;
FILE               *fpprn = NULL;
char               bufnom[16],buf[30] ;
char               Arrond[3+1]  = "   ";
char               nom_s = ' ';
char               LANGUE = 'F', Choix_College = 'F';
int                swRLg = 0;
int                colonne;
char               RLg = ' ';
char               Choix_RLg = ' ';
int                nListeF = 0, nListeN = 0;

char swPcxOnly;

typedef struct {
  char  college;
  char  num[2+1];
  char  nom[25+1];
} LISTINFO;
LISTINFO  listes[99];
int       iliste;
/****************************************************************************/
/**********************************************/
/**/ int getRLgType(int *p_nlF, int *p_nlN) /**/
/**********************************************/
{
  int  result;

  result = 2;

  if (p_nlF != (int *)NULL)  (*p_nlF) = nListeF;
  if (p_nlN != (int *)NULL)  (*p_nlN) = nListeN;

  return(result);
}
/*****************************************************************************/

/**************************** CORPS DU PROGRAMME ***************************/

void main(int argc, char *argv[])
{
  int ind_arg = 1;
  int              iMsg;
  int val;
/* << el2007 */
  char drive[8];
  char argvdir[256];
  char dir[256];
  char file[16];
  char ext[8];
  char dirbmp[256];
  int iNbListes;
  int iTabX1[30];
  int iTabY1[30];
  int iTabX2[30];
  int iTabY2[30];
  int iW;
  int iH;
  int iSpaceW = 15;
  int iSpaceH;
  int iId, iId2;
  int iCase;

  dirbmp[0] = 0x00;

  if(argc >= 2)
    strcpy(argvdir, argv[1]);
  else
    argvdir[0] = 0x00;

/* << el2007 */
	swPcxOnly = 0;
	if(argc == 3)
		swPcxOnly = 1;
/* el2007 >> */
/* << DIGI */
	/* Jites / Digivote */
	if(strstr(argv[0], "_LISTES2.EXE"))
	{
		JITES = 1;
		DIGIVOTE = 0;
	}
	else
	{
		JITES = 0;
		DIGIVOTE = 1;
	}
/* DIGI >> */

  if(argvdir[0])
    fnsplit(argvdir, drive, dir, file, ext);
/* el2007 >> */

  for (iMsg = 0; (iMsg <= NMAX_MSG); iMsg++) {
    memset(Msg[iMsg], 0, (size_t)(LMAX_MSG +1));
  }

  while (ind_arg < argc) {
    if (ind_arg == 1) {
      strcpy(fnin, argv[ind_arg]);
      ind_arg++; continue;
    }
    ind_arg++; /* other parameter(s) skipped */
  }

  fpin = fopen(fnin,"r");
  if (fpin == (FILE *)NULL) {
    fprintf(stderr, "Open file (%s) ERROR.  press a key ", fnin);
    getch();
    return;
  }

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  nom_s = bufc[0];  /* CODE SCRUTIN */
  strncpy(Arrond,  bufc + 2, 3); Arrond[3]  = '\0';  /* CODE ARRONDISSEMENT */
  LANGUE = bufc[6];  /* LANGUE (SIMULATION DU CHOIX DE L'UTILISATEUR) */
  Choix_College = bufc[7];
/* << el2007 */
/*
  bufnom[0] = nom_s;
  bufnom[1] = LANGUE;
  bufnom[2] = Choix_College;
  bufnom[3] = 0x00;
*/
  bufnom[0] = bufc[0];
  bufnom[1] = bufc[1];
  bufnom[2] = bufc[2];
  bufnom[3] = bufc[3];
  bufnom[4] = bufc[4];
  bufnom[5] = bufc[5];
  bufnom[6] = '.';
  bufnom[7] = bufc[6];
  bufnom[8] = bufc[7];
  if(JITES)
    bufnom[9] = 'L';
  else
    bufnom[9] = 'D';
/* el2007 >> */
  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(caisse,bufc + 25 ,2);
  caisse[2] = '\0';
  nbrl = atoi(caisse); /* NOMBRE TOTAL LISTES SANS VOTE BLANC */
  strncpy(caisse,bufc + 28 ,2);
  caisse[2] = '\0';
  nListeF = atoi(caisse); /* NOMBRE DE LISTES F SANS VOTE BLANC */
  strncpy(caisse,bufc + 31 ,2);
  caisse[2] = '\0';
  nListeN = atoi(caisse); /* NOMBRE DE LISTES N SANS VOTE BLANC */
  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[1],bufc,60);
  for(i = 60;((Msg[1][i] == ' ') || (Msg[1][i] == '\0')) && (i > 0);i--)
    Msg[1][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[2],bufc,70);
  for(i = 69;((Msg[2][i] == ' ') || (Msg[2][i] == '\0')) && (i > 0);i--)
    Msg[2][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[3],bufc,70);
  for(i = 69;((Msg[3][i] == ' ') || (Msg[3][i] == '\0')) && (i > 0);i--)
    Msg[3][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[4],bufc,70);
  for(i = 69;((Msg[4][i] == ' ') || (Msg[4][i] == '\0')) && (i > 0);i--)
    Msg[4][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[5],bufc,70);
  for(i = 69;((Msg[5][i] == ' ') || (Msg[5][i] == '\0')) && (i > 0);i--)
    Msg[5][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[6],bufc,70);
  for(i = 69;((Msg[6][i] == ' ') || (Msg[6][i] == '\0')) && (i > 0);i--)
    Msg[6][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[7],bufc,70);
  for(i = 69;((Msg[7][i] == ' ') || (Msg[7][i] == '\0')) && (i > 0);i--)
    Msg[7][i] = '\0';

  bufc[0] = '\0'; fgets(bufc,81,fpin);
  strncpy(Msg[8],bufc,70);
  for(i = 69;((Msg[8][i] == ' ') || (Msg[8][i] == '\0')) && (i > 0);i--)
    Msg[8][i] = '\0';

  iliste = 0;
  while(iliste < nbrl) {
    bufc[0] = '\0'; fgets(bufc,81,fpin);
    if (strlen(bufc) < 23) {
      fprintf(stderr, "Read file (%s) ERROR.  press a key ", fnin);
      getch();
      return;
    }
    listes[iliste].college = bufc[0];

    strncpy(listes[iliste].num, bufc + 1, (size_t)2);
    listes[iliste].num[2] = '\0';
    if (listes[iliste].num[0] == '0') listes[iliste].num[0] = ' ';

    strncpy(listes[iliste].nom, bufc + 3, (size_t)25);
    listes[iliste].nom[25] = '\0';
    for(i = 24; ((listes[iliste].nom[i] == ' ') ||
		 (listes[iliste].nom[i] == '\0')) && (i > 0);
	i--) listes[iliste].nom[i] = '\0';
    iliste++;
  }
  /* VOTE BLANC (DERNIERE LISTE) */
  bufc[0] = '\0'; fgets(bufc,81,fpin);
  if ((strlen(bufc) < 2) || (bufc[0] != ' ')) {
    fprintf(stderr, "Read file (%s) ERROR.  press a key ", fnin);
    getch();
    return;
  }
  else {
    listes[iliste].college = LANGUE;
    strcpy(listes[iliste].num, "--");
    strncpy(listes[iliste].nom, bufc + 1, (size_t)25);
    listes[iliste].nom[25] = '\0';
    for(i = 24; ((listes[iliste].nom[i] == ' ') ||
		 (listes[iliste].nom[i] == '\0')) && (i > 0);
	i--) listes[iliste].nom[i] = '\0';
  }
  nbrl++; /* Liste Vote BLANC */
  if (fpin != (FILE *)NULL) { fclose(fpin); fpin = (FILE *)NULL; }

  if((strcmp(Arrond,"002")== 0) && ((nom_s == '3')))
   {
    swRLg = 2; RLg = LANGUE; Choix_RLg = ' ';
   }
  else {
    swRLg = 0; RLg = ' '; Choix_RLg = ' ';
  }

  /* AFFICHAGE ***************************************************************/
  registerbgidriver (EGAVGA_driver);    /* Option pour linker avec la  */
  registerbgifont (small_font);         /* librairies GRAPHICS         */
  registerbgifont (sansserif_font);
  Initialize();
  Initpcx();

	if(JITES)
		setfillstyle(SOLID_FILL,BLACK);
	else
		setfillstyle(SOLID_FILL,WHITE);
	bar( 0,0,639,479);

	if(JITES)
	{
		setcolor(WHITE);
	/*else
		setcolor(BLACK);*/

  settextstyle( SMALL_FONT, HORIZ_DIR, 6);
  settextjustify( CENTER_TEXT, CENTER_TEXT );
  outtextxy(320, 10, Msg[1]);
  outtextxy(320, 11, Msg[1]);

  if((strcmp(Arrond,"002")== 0) &&
    ((nom_s == '1') || (nom_s == '3') || (nom_s == '4')))
  {
    setfillstyle(SOLID_FILL,BLACK);
    bar( 0,0,639,479);
    setcolor(WHITE);

    strcpy(gMsg, Msg[1]);
    strcat(gMsg,"  ");
    strcat(gMsg,Msg[4]);
    settextstyle( SMALL_FONT, HORIZ_DIR, 6);
    settextjustify( CENTER_TEXT, CENTER_TEXT );
    outtextxy(320, 10, gMsg);
  }

  if (swRLg )
   {
    iliste = 0;
    swRLg = getRLgType((int *)NULL, (int *)NULL);
    iliste = 0;
    Choix_RLg = Choix_College;

    setfillstyle(SOLID_FILL,BLACK);
    bar( 0,0,639,479);
    setcolor(WHITE);

    strcpy(gMsg, Msg[1]);
    strcat(gMsg,"    ");
    if( Choix_RLg == 'F')
              strcat(gMsg, Msg[7]);
    else
              strcat(gMsg, Msg[8]);
    settextstyle( SMALL_FONT, HORIZ_DIR, 6);
    settextjustify( CENTER_TEXT, CENTER_TEXT );
    outtextxy(320, 10, gMsg);
  }

  outtextxy(320, 35, Msg[2]);
}

if(JITES)
{
  settextjustify( LEFT_TEXT, CENTER_TEXT );
  setfillstyle(SOLID_FILL,WHITE);
  setcolor(BLACK);

  iliste = 0;
  param();
  iliste = 0;

  if (!swRLg)
   {
/* << el2007 */
    /*for(Cl=0,k=0,x=0;(k < nbrl) && (Cl < colonne);x+=(espl) + lon,Cl++)*/
    for(Cl=0,k=0,x=espw;(k < nbrl) && (Cl < colonne);x+=(espw) + lon,Cl++)
/* el2007 >> */
    {
      for(j = 0,y = h;(j < nbr_lignes) && (k < nbrl);k++)
      {
	if ((listes[iliste].college == Choix_College) || (k == (nbrl - 1)))
        {
	  bar(x,y,x + lon,y + recl);
          if(k < (nbrl - 1))
            {
                 buf[0]=nom_s;
                 buf[1]= 0x00;
                 strcat(buf,listes[iliste].num);
                 buf[3]=Choix_College;
                 buf[4]=0x00;
                 if(buf[1] == ' ')
                       buf[1] = '0';

                 if(argvdir[0])
                   fnmerge(dirbmp, drive, dir, buf, ".bmp");
/* << el2007 */                   
				settextjustify( CENTER_TEXT, CENTER_TEXT );
				outtextxy( x + 13, y + recl/2, listes[iliste].num);
				outtextxy( x + 14, y + recl/2, listes[iliste].num);
/* el2007 >> */

                 if(access(dirbmp,0) == 0)
                   {
/* << el2007 
                       outtextxy( x + 3, y + recl/2, listes[iliste].num);
                       outtextxy( x + 4, y + recl/2, listes[iliste].num);
                       bmp(dirbmp, x + 20,y +((recl-26)/2),0);*/
                       bmp(dirbmp, x + 29,y +((recl-26)/2),0);
/* el2007 >> */
                   }
                  else
                  {
/* << el2007 */
                    /*strcpy(caisse,listes[iliste].num);
                    strcat(caisse," ");
                    strcat(caisse,listes[iliste].nom);
                    outtextxy( x + 3, y + recl/2, caisse);
                    outtextxy( x + 4, y + recl/2, caisse);
                    settextjustify( CENTER_TEXT, CENTER_TEXT );
                    outtextxy( x + 13, y + recl/2, listes[iliste].num);
                    outtextxy( x + 14, y + recl/2, listes[iliste].num);*/
                    settextjustify( LEFT_TEXT, CENTER_TEXT );
                    outtextxy( x + 28, y + recl/2, listes[iliste].nom);
                    outtextxy( x + 29, y + recl/2, listes[iliste].nom);
/* el2007 >> */
                  }
            }
          else
            {
/* << el2007 */
                  /*outtextxy(x + 24, y + recl/2, listes[iliste].nom);
                  outtextxy(x + 25, y + recl/2, listes[iliste].nom);*/
                  settextjustify( LEFT_TEXT, CENTER_TEXT );
                  outtextxy(x + 28, y + recl/2, listes[iliste].nom);
                  outtextxy(x + 29, y + recl/2, listes[iliste].nom);
/* el2007 >> */
            }
            j++;
            y+=(espl/2) + recl;
            iliste++;
        }
      }
    }
   }
  else
   if (swRLg)
   {
/* << el2007 */
    /*for(Cl=0,k=0,x=(espl/2);(k < nbrl) && (Cl < colonne);x+=(espl/2) + lon,Cl++)*/
    for(Cl=0,k=0,x=(espw/2);(k < nbrl) && (Cl < colonne);x+=(espw/2) + lon,Cl++)
/* el2007 >> */
    {
      for(j = 0,y = h;(j < nbr_lignes) && (k < nbrl);k++)
      {
	if ((listes[iliste].college == Choix_RLg) || (k == (nbrl - 1)))
        {
	  bar(x,y,x + lon,y + recl);
	  if (k < (nbrl - 1))
          {
                 buf[0]=nom_s;
                 buf[1]= 0x00;
                 strcat(buf,listes[iliste].num);
                 buf[3]=Choix_College;
                 buf[4]=0x00;

                 if(buf[1] == ' ')
                       buf[1] = '0';

                 if(argvdir[0])
                   fnmerge(dirbmp, drive, dir, buf, ".bmp");

/* << el2007 */
				settextjustify( CENTER_TEXT, CENTER_TEXT );
				outtextxy( x + 13, y + recl/2, listes[iliste].num);
				outtextxy( x + 14, y + recl/2, listes[iliste].num);
/* el2007 >> */
                 if(access(dirbmp,0) == 0)
                   {
/* << el2007 
                       outtextxy( x + 3, y + recl/2, listes[iliste].num);
                       outtextxy( x + 4, y + recl/2, listes[iliste].num);
                       bmp(dirbmp, x + 20,y +((recl-26)/2),0);*/
                       bmp(dirbmp, x + 29,y +((recl-26)/2),0);
/* el2007 >> */
                   }
                  else
                  {
/* << el2007 */
                    /*strcpy(caisse,listes[iliste].num);
                    strcat(caisse," ");
                    strcat(caisse,listes[iliste].nom);
                    outtextxy( x + 3, y + recl/2, caisse);
                    outtextxy( x + 4, y + recl/2, caisse);
                    settextjustify( CENTER_TEXT, CENTER_TEXT );
                    outtextxy( x + 13, y + recl/2, listes[iliste].num);
                    outtextxy( x + 14, y + recl/2, listes[iliste].num);*/
                    settextjustify( LEFT_TEXT, CENTER_TEXT );
                    outtextxy( x + 28, y + recl/2, listes[iliste].nom);
                    outtextxy( x + 29, y + recl/2, listes[iliste].nom);
/* el2007 >> */
                  }
          }
       else
          {
/* << el2007 */
            /*outtextxy(x + 24, y + recl/2, listes[iliste].nom);
            outtextxy(x + 25, y + recl/2, listes[iliste].nom);*/
            outtextxy(x + 28, y + recl/2, listes[iliste].nom);
            outtextxy(x + 29, y + recl/2, listes[iliste].nom);
/* el2007 >> */
          }

           j++;
           y+=(espl/2) + recl;
        }
	iliste++;
      }
    }
   }
}
else /* (DIGIVOTE) */
{
	setfillstyle(SOLID_FILL,BLACK);
	bar( 0,16,639,16);

	settextjustify( LEFT_TEXT, CENTER_TEXT );
	setfillstyle(SOLID_FILL,BLACK);
	setcolor(BLACK);
	settextstyle( SMALL_FONT, HORIZ_DIR, 4);
	outtextxy( 14, 6, Msg[1]);

	if(nListeF!=0)
		iNbListes = nListeF;
	else
		iNbListes = nListeN;

#define X2_Y2 \
	iTabX2[iId] = iTabX1[iId] + iW; \
	iTabY2[iId] = iTabY1[iId] + iH;

#define DECALE_BAS \
	++iId; \
	iTabX1[iId] = iTabX1[iId-1]; \
	iTabY1[iId] = iTabY2[iId-1] + iSpaceH; \
	X2_Y2

#define DECALE_DROITE(VAL) \
	++iId; \
	iTabX1[iId] = iTabX2[iId-VAL] + iSpaceW; \
	iTabY1[iId] = iTabY1[iId-VAL]; \
	X2_Y2

	switch(iNbListes)
	{
		case 1:
		case 2:
			iW = 191;
			iH = 312;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 83;
			X2_Y2

			DECALE_DROITE(1)

			DECALE_DROITE(1)

			if(iNbListes == 2)
				DECALE_DROITE(1)
			break;
		case 3:
		case 4:
		case 5:
			iCase = 1;
			iW = 191;
			iH = 156;
			iSpaceH = 46;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 60;
			X2_Y2
			break;
		case 6:
		case 7:
		case 8:
			iCase = 2;
			iW = 191;
			iH = 137;
			iSpaceH = 12;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 24;
			X2_Y2
			break;
		case 9:
		case 10:
		case 11:
			iCase = 3;
			iW = 191;
			iH = 102;
			iSpaceH = 7;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 23;
			X2_Y2
			break;
		case 12:
		case 13:
		case 14:
			iCase = 4;
			iW = 191;
			iH = 82;
			iSpaceH = 5;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 21;
			X2_Y2
			break;
		case 15:
		case 16:
		case 17:
			iCase = 5;
			iW = 191;
			iH = 68;
			iSpaceH = 5;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 21;
			X2_Y2
			break;
		case 18:
		case 19:
		case 20:
			iCase = 6;
			iW = 191;
			iH = 58;
			iSpaceH = 4;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 20;
			X2_Y2
			break;
		case 21:
		case 22:
		case 23:
			iCase = 7;
			iW = 191;
			iH = 51;
			iSpaceH = 3;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 19;
			X2_Y2
			break;
		case 24:
		case 25:
		case 26:
			iCase = 8;
			iW = 191;
			iH = 45;
			iSpaceH = 3;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 19;
			X2_Y2
			break;
		case 27:
		case 28:
		case 29:
			iCase = 9;
			iW = 191;
			iH = 41;
			iSpaceH = 3;
			iId = 0;
			iTabX1[iId] = 19;
			iTabY1[iId] = 19;
			X2_Y2
			break;
	}

	if(iNbListes > 2)
	{
		for(iId2=0;iId2<iCase;++iId2)
		{
			DECALE_BAS
		}

		++iId2;
		DECALE_DROITE(iId2)

		for(iId2=0;iId2<iCase;++iId2)
		{
			DECALE_BAS
		}

		if(iNbListes > 3)
		{
			++iId2;
			DECALE_DROITE(iId2)
		}

		while(iId < iNbListes)
		{
			DECALE_BAS
		}
	}

	settextjustify( LEFT_TEXT, CENTER_TEXT );
	setfillstyle(SOLID_FILL,BLACK);
	setcolor(BLACK);
	settextstyle( SMALL_FONT, HORIZ_DIR, 4);
	for(iliste = 0;iliste <= iNbListes; ++iliste)
	{
		/*bar(iTabX1[iliste],iTabY1[iliste],iTabX2[iliste], iTabY2[iliste]);*/
		/*printf("%d %d %d %d\n", iTabX1[iliste],iTabY1[iliste],iTabX1[iliste], iTabY2[iliste]);*/
		bar(iTabX1[iliste],iTabY1[iliste],iTabX1[iliste], iTabY2[iliste]);
		bar(iTabX2[iliste],iTabY1[iliste],iTabX2[iliste], iTabY2[iliste]);
		bar(iTabX1[iliste],iTabY1[iliste],iTabX2[iliste], iTabY1[iliste]);
		bar(iTabX1[iliste],iTabY2[iliste],iTabX2[iliste], iTabY2[iliste]);

		if(iliste < iNbListes)
		{
			/* Num */
			outtextxy( iTabX1[iliste] + 10, iTabY1[iliste] + (iTabY2[iliste]-iTabY1[iliste]) /2, listes[iliste].num);

			buf[0]=nom_s;
			buf[1]= 0x00;
			strcat(buf,listes[iliste].num);
			buf[3]=Choix_College;
			buf[4]=0x00;

			if(buf[1] == ' ')
					 buf[1] = '0';

			if(argvdir[0])
				fnmerge(dirbmp, drive, dir, buf, ".bmp");

			if(access(dirbmp,0) == 0)
				bmp(dirbmp, iTabX1[iliste] + 34,iTabY1[iliste] + (iTabY2[iliste]-iTabY1[iliste]-26) /2,0);
			else
				outtextxy( iTabX1[iliste] + 34, iTabY1[iliste] + (iTabY2[iliste]-iTabY1[iliste]) /2, listes[iliste].nom);
		}
		else /* BLANC */
		{
			outtextxy( iTabX1[iliste] + 34, iTabY1[iliste] + (iTabY2[iliste]-iTabY1[iliste]) /2, listes[iliste].nom);
		}
	}
}

 val = true;

 while(val)
  {
if(JITES)
{
	setfillstyle(SOLID_FILL,BLACK);
	bar( 0,438,639,479);      /* Draw backgnd */
	setcolor( WHITE);
}

	settextjustify(CENTER_TEXT,CENTER_TEXT);
	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	outtextxy( 320,470, Msg[3]);

/* << el2007 */
		if(!swPcxOnly)
			c = getch();
		else
			c = 'F';
/* el2007 >> */

	if (c == 'P' ||  c == 'p' || c =='I' || c == 'i')
	  {
		rin.h.ah = 2;
		rin.x.dx = 0;
		int86(0x17,&rin,&rout);
		if(rout.h.ah != 0x90)
		  {
			 setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
			 bar( 0,438,639,479);      /* Draw backgnd */
			 setcolor( WHITE);
			 settextjustify(CENTER_TEXT,CENTER_TEXT);
			 settextstyle(SMALL_FONT, HORIZ_DIR, 6);
			 outtextxy( 320,470, "Imprimante occupe, pressez une touche.");
			 c = getch();
		  }
		else
		 {
			int86(0x05,&rin,&rout);
			ptc = fopen("PRN","w");
			bufc[0]  = 0x0C;
			bufc[1] = 0x00;
			fputs(bufc,ptc);
			fclose(ptc);
			val = false;
		 }
	  }
	else
	   if (c == 'F' ||  c == 'f')
	     {
/* << el2007 */
/*
                 strcpy(bufname,bufnom);
		 strcat(bufname,".pcx");
                 pcxDisplayFile(bufname,0,0,639,479,0);
*/
		pcxDisplayFile(bufnom,0,0,639,479,0);
		if(swPcxOnly)
			break;
/* el2007 >> */
	      }
	   else
		  val = false;
  }
/* << el2007 */
  closegraph();
/* el2007 >> */
}
/***************************  POINTAGE CRAYON *****************************/
/***************************************************************************/
param()
{
  int /*nbr_liste_ligne = 0,*/ nbre_Liste_Int = 0;
#include "lparam1.c"

}
/*****************************************************************************/
