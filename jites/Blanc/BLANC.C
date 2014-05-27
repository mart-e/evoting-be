/***************************************************************************/
/* JITES                                                                   */
/*                                                                         */
/*                                VARIABLES                                */
/* Hcadre_candid = hauteur du cadre candidat                               */
/* Interl_candid = Interligne entre les cadres candidats                   */
/* Lsaisie_candid = longueur de la zone de saisie candidat                 */
/* Lcadre_candid  = longueur du cadre candidat                             */
/* Interc_candid  = Intercolonne entre les cadres candidats                */
/* x_dep          = valeur de la coordonnee x de la  1 ere colonne         */
/* x_ent_scr      = valeur de x de l'entete du scrutin                     */
/* x_numl         = valeur de x du numero de la liste                      */
/* x_noml         = valeur de x du nom de la liste                         */
/* nbc_col        = nombre de candidats par colonne                        */
/* h    = hauteur a partir de l'entete                                     */
/* lon  = longueur d'une case liste                                        */
/* espl = espacement entre les cases listes                                */
/* espw = espacement entre les cases listes (w = width)                    */
/* recl = hauteur d'une case liste                                         */
/* nbc  = nombre de candidats / liste                                      */
/* recc = hauteur d'une case candidat                                      */
/***************************************************************************/

#include "cfgscrut.h"
CFGSCRUT cfgS[NMX_SCRUTIN_T+1] = { {0, 0, 0},
	 {VM_EUROPE,   NMXV_EUROPE,   NMXB_EUROPE},
	 {VM_CHAMBRE,  NMXV_CHAMBRE,  NMXB_CHAMBRE},
	 {VM_SENAT,    NMXV_SENAT,    NMXB_SENAT},
	 {VM_REGION,   NMXV_REGION,   NMXB_REGION},
	 {VM_PROVINCE, NMXV_PROVINCE, NMXB_PROVINCE},
	 {VM_COMMUNE,  NMXV_COMMUNE,  NMXB_COMMUNE},
	 {VM_CONSEIL_FLAMANT,NMXV_CONSEIL_FLAMANT,NMXB_CONSEIL_FLAMANT}
	 };

#define BAL_LISTE_INCOMPLETE    0  /* 1=Yes  0=No                            */
#define LPMNGLIB                1  /* 1=Yes  0=No                            */
#define LPuserCURSORchange      0  /* 1=Yes  0=No                            */
/* << el2007 */
/*#define MEMORY_CHECK            1  / 1=Yes  0=No                           */
#define MEMORY_CHECK            0  /* 1=Yes  0=No                            */
/* el2007 >> */
#define TERMINAL_CHECK          1  /* 1=Yes  0=No                            */
#define LOOPWRITE_CARTE         1  /* 1=Yes  0=No                            */
#define NOCARTE_CHECK           1  /* 1=Yes  0=No                            */

#include <bios.h>
#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <graphics.h>
#include <string.h>

char LANGUE = 'F';

union REGS rin,rout;
#include "fullspec.h"
#include "alg-fst.h"
#include "api-fst.h"
#include "dzdef.h"
#include "variable.h"
#include "lecteur.h"
#include "lcd.h"
#include "mcbc.c"
#include "lcd.c"
#include "mcr.c"
#include "ui.c"
#include "uqmc.c"
#include "utt.c"
#include "jamal.c"
#include "zeit.c"
#include "garnir.c"
#include "vote.c"
#include "aff_cand.c"
#include "be.c"
#include "view.c"
#include "aff_clav.c"
#include "erreur.c"
#include "bmp.c"

/****************************************************************************/
int testcrc;
char MSG_COL[50];
char Minicodage[5];
char gmessage_1[N_SCR_MESSAGE][L_SCR_MESSAGE];
char gmessage_2[N_SCR_MESSAGE][L_SCR_MESSAGE];
/**************************** CORPS DU PROGRAMME ***************************/
/* MODIFICATIONS PAR RAPPORT A LA VERSION 10.05

	10.06 (070406)
	-------
	-> Affichage des candidats sur une colonne
		si chambre et nb candidats <= 14
		si sénat   et nb candidats <= 22
*/
void main(int argc, char *argv[])
{
	int ind_arg = 1, i;

	strcpy(Minicodage,MINI_PWD);

	for (i = 0; (i < N_SCR_MESSAGE); i++)
	{
		message_1[i] = gmessage_1[i];
		message_2[i] = gmessage_2[i];
	}

	while(ind_arg < argc)
	{
		#if (NOCARTE_CHECK >= 1)
		if((!strcmp(argv[ind_arg], "-NOCARTE")) ||
		   (!strcmp(argv[ind_arg], "-NOCarte")) ||
		   (!strcmp(argv[ind_arg], "-NOcarte")) ||
		   (!strcmp(argv[ind_arg], "-nocarte")) )
		{
			swNOCarte = _NOCARTE;
			ind_arg++; continue;
		}
		#endif
		if((!strcmp(argv[ind_arg], "-LPCURSOR")) ||
		   (!strcmp(argv[ind_arg], "-LPcursor")) ||
		   (!strcmp(argv[ind_arg], "-LPCursor")) ||
		   (!strcmp(argv[ind_arg], "-LpCursor")) ||
		   (!strcmp(argv[ind_arg], "-lpCursor")) ||
		   (!strcmp(argv[ind_arg], "-lpcursor")) )
		{
			swLPcursor = 1;
			ind_arg++; continue;
		}
		ind_arg++; /* parameter(s) skipped */
	}
	verif_terminal();
	registerbgidriver (EGAVGA_driver);    /* Option pour linker avec la  */
	registerbgifont (small_font);         /* librairies GRAPHICS         */
	registerbgifont (sansserif_font);
	Initialize();
	Lecteur_Init();     /*Attention au retour */
	/* << 10.07 */
	setvect( 0, Divide_Error_Handler );
	/* 10.07 >> */
	harderr(handler);
	Garnir_Ind();
	Message_point = message_1;
	/************************/
	/*      ROUTINE CRC     */

	while(!Aff_Clavier());

	/*  Gnration d'un nouveau password  */
	CMinipassword[0] = fullPassword[MINI_POS[0]-49];
	CMinipassword[1] = fullPassword[MINI_POS[1]-49];
	CMinipassword[2] = fullPassword[MINI_POS[2]-49];
	CMinipassword[3] = fullPassword[MINI_POS[3]-49];
	CMinipassword[4] = 0x00;
	
	strcat(CMinipassword,Minicodage);

	/* Fin gnration d'un nouveau password  */

	Demande_Floppy();

	setfillstyle(SOLID_FILL, WHITE);
	bar(0,0,639,479);
	setcolor(BLACK);
	settextstyle(SMALL_FONT, HORIZ_DIR,13);
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	if(code_langue > '3')
	{
		outtextxy(320, 160, message_1[88]);
		outtextxy(321, 160, message_1[88]);
		outtextxy(320, 340, message_2[88]);
		outtextxy(321, 340, message_2[88]);
	}
	else
	{
		outtextxy(320, 240, message_1[88]);
		outtextxy(321, 240, message_1[88]);
	}


	/* START CONFIDENTIAL */

	if(swNOCarte != _NOCARTE)
	{
		testcrc = Crc_Rijndael();
		if(!testcrc)
		{
			setfillstyle(SOLID_FILL, WHITE);
			bar(0,0,639,479);
			setcolor(BLACK);
			settextstyle(SMALL_FONT, HORIZ_DIR,13);
			while(1)
			{
				if(code_langue > '3')
				{
					settextjustify(CENTER_TEXT, TOP_TEXT);
					outtextxy(320, 160, message_1[87]);
					outtextxy(321, 160, message_1[87]);
					settextjustify(CENTER_TEXT, BOTTOM_TEXT);
					outtextxy (320, 340,message_2[87]);
					outtextxy (321, 340,message_2[87]);
					fraude(RG,1);
				}
				else
				{
					settextjustify(CENTER_TEXT, TOP_TEXT);
					outtextxy(320, 160, message_1[87]);
					outtextxy(321, 160, message_1[87]);
					settextjustify(CENTER_TEXT, BOTTOM_TEXT);
					outtextxy (320, 340,message_1[87]);
					outtextxy (321, 340,message_1[87]);
					fraude(RG,1);
				}
			}
		}
	}
	/* END CONFIDENTIAL */

	settextjustify(CENTER_TEXT, CENTER_TEXT);
	Garnir_scrutin();
	status = Tempo();

	ptfunct = recom;
	while(1)
	(ptfunct)();
}

/* << el2007 */
int Lire_Carte(char _cSwAskLg, char _cSwError)
{
	int val_jeton;
	char C_jeton[6];

	status = Read_Commande();

	/* START CONFIDENTIAL */
	if(strcmp(Arrond,"005") == 0 )
		LANGUE = 'N';

	/* END CONFIDENTIAL */

	/* _cSwAskLg */
	if(_cSwAskLg)
	{
		if(code_langue > '3')
		{
			do
			{
				choix_langue();
			} while(! Confirm_langue());
		}
	}

	if (status != 0)
	{
		if(_cSwError)
		{
			msg_error = status;
			ptfunct = fin;
		}
		return(0);
	}

	strcpy(carte,Lect_Buffer);
	Recupere_Jeton(carte,C_jeton);
	val_jeton = strcmp(jetonmini,C_jeton);

	if(swNOCarte != _NOCARTE)
	{
		Calcul_Crc(carte,_C_TEST_PLACE);
		Recupere_Crc(carte);

		if ((val_jeton != 0) || (strcmp(Crc_Calcul,Crc_Recup) !=0))
		{
			if(_cSwError)
			{
				msg_error = 12;
				ptfunct = fin;
			}
			return(0);
		}
	}

	return(1);
}
/* el2007 >> */

/***************************************************************************/
/*****************************  Gestion d'un VOTE **************************/
/***************************************************************************/
/* << el2007 */
int recom_view()
{

	scr = list = dejasel = typesel = visu = 0;

	swconf = 0;
	status = msg_error = 0;
	klavier = 1;

	fraude(OFF,0);
	nettmem();
	if(!Lire_Carte(1, 0))
	{
		if(!Lire_Carte(0, 1))
			return(0);
	}

	Recupere_Passage(carte,&passe);
	if(swNOCarte != _NOCARTE)
	{
	if (passe > 0)
	{
		msg_error = 11;
		ptfunct = fin;
		return(0);
	}
	}
	/* END CONFIDENTIAL */

	NonBoucle = 0;
	ptfunct       = boucle;
	return(0);
}

int recom()
{

	scr = list = dejasel = typesel = visu = 0;

	swconf = 0;
	status = msg_error = 0;
	klavier = 1;

	fraude(OFF,0);
	nettmem();
	setfillstyle(SOLID_FILL,WHITE);
	bar(0, 0, 639,479);
	setcolor ( BLACK );
	settextstyle( SMALL_FONT, HORIZ_DIR, 9);
	settextjustify(CENTER_TEXT, CENTER_TEXT);

	if(code_langue > '3')
	{
		outtextxy (320, 120,message_1[30]);
		outtextxy (320, 150,message_1[31]);
		outtextxy (320, 180,message_1[107]);
		outtextxy (321, 120,message_1[30]);
		outtextxy (321, 150,message_1[31]);
		outtextxy (321, 180,message_1[107]);

		outtextxy (320, 320,message_2[30]);
		outtextxy (320, 350,message_2[31]);
		outtextxy (320, 380,message_2[107]);
		outtextxy (321, 320,message_2[30]);
		outtextxy (321, 350,message_2[31]);
		outtextxy (321, 380,message_2[107]);
	}
	else
	{
		outtextxy (320, 210,message_1[30]);
		outtextxy (320, 240,message_1[31]);
		outtextxy (320, 270,message_1[107]);
		outtextxy (321, 210,message_1[30]);
		outtextxy (321, 240,message_1[31]);
		outtextxy (321, 270,message_1[107]);
	}

	if(swNOCarte == _NOCARTE) outtextxy (300, 40,"Demo");
	Message_point = message_1;
	while (Wait_Card())
	{
		if(point())
		if(cy > 445)
		{
			if(cx > 500)
			{
				settextjustify(LEFT_TEXT, CENTER_TEXT);
				settextstyle( SMALL_FONT, HORIZ_DIR, 5);
				outtextxy (30, 40, VERSION);
				outtextxy (30, 60, jeton);
				outtextxy (30, 80, idCreation);
			}
		}
		else if ((cy <= 35) && (cx >= 550))
		if(!klavier)
		{
			msg_error = 13;
			ptfunct = fin;
			return(0);
		}
	}
	if(!Lire_Carte(1, 0))
	{
		if(!Lire_Carte(0, 1))
		{
			return(0);
		}
	}

	Recupere_Passage(carte,&passe);
	if(swNOCarte != _NOCARTE)
	{
	if (passe > 0)
	{
		msg_error = 11;
		ptfunct = fin;
		return(0);
	}
	}
	/* END CONFIDENTIAL */

	NonBoucle = 0;
	ptfunct       = boucle;
	return(0);
} /* recom */

/***************************************************************************/
/***************************************************************************/

int boucle()
{
	char strtmp[60];

	while(scr < nbrscrutin)
	{
		if((swNOCarte == _NOCARTE)
			 || (carte[_C_TEST_PLACE] == 'N')
			 || ((carte[_C_TEST_PLACE] == 'E')
			 && ((scrutin[scr].TYPE_SCRUTIN == 1) || (scrutin[scr].TYPE_SCRUTIN == 6)))
			 || ((carte[_C_TEST_PLACE] == 'S') && ((scrutin[scr].TYPE_SCRUTIN== 2) ||
			 (scrutin[scr].TYPE_SCRUTIN == 3))))
		{
			ccol = 0;
			crl = 0;
			inportb(0x308);
			ps = scrutin[scr].ptlist;

			if((scrutin[scr].nom_s == '1'))
				ClOui = 1;
			else
				ClOui = 0;

			if((strcmp(Arrond,"002")==0) && (scrutin[scr].nom_s== '4'))
				swVLR = 1;

			if((strcmp(Arrond,"002")==0) && (scrutin[scr].nom_s== '7')
				 && (swVLR == 0))
			{
				memos[scr] = 2; /* on passe */
				scr++;
				return(0);
			}

			if((strcmp(Arrond,"002")==0) && (scrutin[scr].nom_s== '4'))
			{
				swRLg = 1;
				RLg = LANGUE;
				Choix_RLg = ' ';
				swCLG = 0;
			}
			else
				if((strcmp(Arrond,"002")==0) && ((scrutin[scr].nom_s == '1')
					 || (scrutin[scr].nom_s == '3')))
			{
				swRLg = 0;
				swCLG = 1;
				RLg = ' ';
				Choix_RLg = ' ';
			}
			else
			{
				swRLg = 0;
				RLg = ' ';
				Choix_RLg = ' ';
				swCLG = 0;
			}
			CHOIX = 0;
			Choix_College = 'F';

			ptfunct = les_listes;
			return(0);
		}
		else
		{
			memos[scr] = 2; /* on passe */
			scr++;
			return(0);
		}
	}
	ptfunct = result;
	return(0);
}
/***************************************************************************/
/*  A F F I C H A G E    E T     S E L E C T I O N    D E S   L I S T E S  */
/***************************************************************************/
int les_listes()
{
	char caisse[30],buf[30];

	dejasel = 0;
	typesel = 0;
	swpas   = 0;
	swnoir  = 0;
	swconf  = 0;

	ps = scrutin[scr].ptlist;
	setfillstyle(SOLID_FILL,BLACK);
	bar( 0,0,639,479);
	setcolor(WHITE);
	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify( CENTER_TEXT, CENTER_TEXT );
	code_scr = (((scrutin[scr].TYPE_SCRUTIN) - 1) * 3);
	if(code_scr == 9)
	   {
		if(strcmp(Arrond,"005") == 0 ) code_scr+=1;
		if(strcmp(Arrond,"002") == 0 ) code_scr+=2;
	   }  
	outtextxy(320, 10, Message_point[code_scr]);
	outtextxy(319, 10, Message_point[code_scr]);

	/********************** Y A PLUSIEURS COLLEGES ?*****************************/
	if (swCLG)
	{
		ps = scrutin[scr].ptlist;
		swCLG = 2; /* Force 1 college par page, attendant Choix MI */
	}
	if(swCLG == 2)
	{
		if(ccol == 0)
		{
			if(Select_Col()) /* SELECTION DU COLLEGE */
			{
				msg_error = 13;
				ptfunct = fin;
				return(0);
			}
			ccol = 1;
		}
		setfillstyle(SOLID_FILL,BLACK);
		bar( 0,0,639,479);
		strcpy(Entete_Col,Message_point[code_scr]);
		strcat(Entete_Col," - ");
		if( Choix_College == 'F')
		{
			strcat(Entete_Col,Message_point[69]);
			strcat(Entete_Col," ");
			strcat(Entete_Col,Message_point[70]);
		}
		else
		{
			strcat(Entete_Col,Message_point[71]);
			strcat(Entete_Col," ");
			strcat(Entete_Col,Message_point[72]);
		}
		setcolor(WHITE);
		settextstyle( SMALL_FONT, HORIZ_DIR, 6);
		settextjustify( CENTER_TEXT, CENTER_TEXT );
		outtextxy(320, 10, Entete_Col);
		outtextxy(319, 10, Entete_Col);
	}
	/********************** Y A PLUSIEURS RLG ?**********************************/
	if (swRLg)
	{
		ps = scrutin[scr].ptlist;
		swRLg = 2;
	}

	if (swRLg == 2)
	{
		/* SELECTION DU REGIME LINGUISTIQUE                                      */
		if(crl == 0)
		{
		if(Select_RLg())
		{
			msg_error = 13;
			ptfunct = fin;
			return(0);
		}
		crl = 1;
		}
		setfillstyle(SOLID_FILL,BLACK);
		bar( 0,0,639,479);
		strcpy(Entete_RLg,Message_point[code_scr]);

		strcat(Entete_RLg,"  ");
		if( Choix_RLg == 'F')
			strcat(Entete_RLg,Message_point[84]);
		else
			strcat(Entete_RLg,Message_point[85]);

		setcolor(WHITE);
		settextstyle( SMALL_FONT, HORIZ_DIR, 5);
		settextjustify( CENTER_TEXT, CENTER_TEXT );
		outtextxy(320, 10, Entete_RLg);
		outtextxy(319, 10, Entete_RLg);
		settextstyle( SMALL_FONT, HORIZ_DIR, 6);
		settextjustify( CENTER_TEXT, CENTER_TEXT );
	}

	/************************ FIN PLUSIEURS RLG ?********************************/

	if (!swRLg || (swRLg == 2))
		outtextxy(320, 35, Message_point[54]);
	else
		outtextxy(320, 35, Message_point[54]);

	settextjustify( LEFT_TEXT, CENTER_TEXT );
	setfillstyle(SOLID_FILL,WHITE);
	setcolor(BLACK);

	ps = scrutin[scr].ptlist;
	param();
	ps = scrutin[scr].ptlist;

	poslBL2[0][0] = 0;
	poslBL2[0][1] = 0;
	poslBL2[0][2] = 0;
	poslBL2[0][3] = 0;

	if (swRLg == 2)
	{
	  for(Cl=0,k=0,x=espw;(k < nbrlist[scr]) && (Cl < ncolonne);x+=espw + lon,Cl++)
		for(j = 0,y = h;(j < nbr_lignes) && (k < nbrlist[scr]);k++)
		{
			if((ps->college == Choix_RLg) || (k == (nbrlist[scr] - 1)))
			{
				posl[k][0] = x;
				posl[k][1] = x + lon;
				posl[k][2] = y;
				posl[k][3] = y + recl;
			}
			else
			{
				posl[k][0] = 0;
				posl[k][1] = 0;
				posl[k][2] = 0;
				posl[k][3] = 0;
				ps++;
				continue;
			}

			bar(x,y,x + lon,y + recl);
			if(k < (nbrlist[scr] - 1))
			{
				buf[0]=scrutin[scr].nom_s;
				buf[1]= 0x00;
				strcat(buf,ps->numlist_l);
				if(strcmp(Arrond,"005") == 0 )
					buf[3]='N';
				else
					buf[3]=ps->college;
				buf[4]=0x00;
				strcat(buf,".bmp");
				if(buf[1] == ' ')
					buf[1] = '0';
				settextjustify( CENTER_TEXT, CENTER_TEXT );
				outtextxy( x + 13, y + recl/2, ps->numlist_l);
				outtextxy( x + 14, y + recl/2, ps->numlist_l);
				if(access(buf,0) == 0)
					bmp(buf, x + 29,y +((recl- 26)/2),0);
				else
				{
				 settextjustify( LEFT_TEXT, CENTER_TEXT );
				 outtextxy( x + 28, y + recl/2, ps->nom_l);
				 outtextxy( x + 29, y + recl/2, ps->nom_l);
				}
			}
			else
			{
				outtextxy(x + 24, y + recl/2, Message_point[46]);
				outtextxy(x + 25, y + recl/2, Message_point[46]);
			}
			j++;
			y+=(espl/2) + recl;
			ps++;
		}
	}
	else
	{
	  for(Cl=0,k=0,x=espw;(k < nbrlist[scr]) && (Cl < ncolonne);x+= espw + lon,Cl++)
		for(j = 0,y = h;(j < nbr_lignes) && (k < nbrlist[scr]);k++)
		{
			if((ps->college == Choix_College) || (k == (nbrlist[scr] - 1)))
			{
				posl[k][0] = x;
				posl[k][1] = x + lon;
				posl[k][2] = y;
				posl[k][3] = y + recl;
			}
			else
			{
				posl[k][0] = 0;
				posl[k][1] = 0;
				posl[k][2] = 0;
				posl[k][3] = 0;
				ps++;
				continue;
			}

			bar(x,y,x + lon,y + recl);
			if(k < (nbrlist[scr] - 1))
			{
				buf[0]=scrutin[scr].nom_s;
				buf[1]= 0x00;
				strcat(buf,ps->numlist_l);
				if(strcmp(Arrond,"005") == 0 )
					buf[3]='N';
				else
					buf[3]=ps->college;
				buf[4]=0x00;
				strcat(buf,".bmp");
				if(buf[1] == ' ')
					buf[1] = '0';
				settextjustify( CENTER_TEXT, CENTER_TEXT );
				outtextxy( x + 13, y + recl/2, ps->numlist_l);
				outtextxy( x + 14, y + recl/2, ps->numlist_l);
				if(access(buf,0) == 0)
					bmp(buf, x + 29,y +((recl- 26)/2),0);
				else
				{
				 settextjustify( LEFT_TEXT, CENTER_TEXT );
				 outtextxy( x + 28, y + recl/2, ps->nom_l);
				 outtextxy( x + 29, y + recl/2, ps->nom_l);
				}
			}
			else
			{
			  settextjustify( LEFT_TEXT, CENTER_TEXT );
			  outtextxy(x + 28, y + recl/2, Message_point[46]);
			  outtextxy(x + 29, y + recl/2, Message_point[46]);
			}
			j++;
			y+=(espl/2) + recl;
			ps++;
		}
	}

	/******************* BAS , RETOUR VERS AUTRES, DEBUT*************************/
	if (swCLG == 2)
	{
		setfillstyle(SOLID_FILL, WHITE);
		settextstyle( SMALL_FONT, HORIZ_DIR, 5);
		bararrondi(110,448,530,478,WHITE);
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		outtextxy(320,460, Message_point[73]);
	}
	if (swRLg == 2)
	{
		setfillstyle(SOLID_FILL, WHITE);
		settextstyle( SMALL_FONT, HORIZ_DIR, 5);
		bararrondi(110,448,530,478,WHITE);
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		outtextxy(320,460, Message_point[86]);
	}
	/******************* BAS , RETOUR VERS AUTRES, FIN  *************************/

	#if (LPMNGLIB >=1)
	if (swLPcursor)
		lpPosxy(639, 479);
	#endif
	while(klavier)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor)
			lpCursorOn();
		#endif
		if(point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if(testposl())
			return(0);
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif

	msg_error = 13;
	ptfunct = fin;
	return(0);
}
/***************************  POINTAGE CRAYON *****************************/
point()
{
	rc = 0;
	rin.x.ax = 5;
	rin.x.bx = 0;
	int86(0x33,&rin,&rout);
	if (rout.x.bx)
	{
		rin.h.ah = 4;
		int86(0x10,&rin,&rout);
		if(rout.h.ah)
		{
			cx = rout.x.bx;
			cy = rout.x.cx;
			rc = 1;
		}
		McrBg();
	}
	klavier = inportb(0x308) & 0x10;
	return(rc);
}
/******************** TEST POSITION CRAYON / LISTES ***********************/
testposl()
{
	if((cx > posl[k - 1][0] + 2) && (cx < posl[k - 1][1] - 2))
	if((cy > posl[k - 1][2] + 2) && (cy < posl[k - 1][3] - 2))
	{
		ptfunct = selectc;
		return(1);
	}
	if(((swRLg == 1) || (swCLG == 1)) &&
	   ((cx > poslBL2[0][0] + 2) && (cx < poslBL2[0][1] - 2) &&
	   (cy > poslBL2[0][2] + 2) && (cy < poslBL2[0][3] - 2))   )
	{
		ptfunct = selectc;
		return(1);
	}


	if((swCLG == 2) && (cy > 438))
	{
		if((cx > 110) && (cx < 530))
		{
			ccol = 0;
			CHOIX = 0;
			ptfunct = les_listes;
			return(1);
		}
	}

	if((swRLg == 2) && (cy > 438))
	{
		if ((cx > 110) && (cx < 530))
		{
			crl = 0;
			Choix_RLg = ' ';
			ptfunct = les_listes;
			return(1);
		}
	}


	for(list = 0;list < k - 1;list++)
	{
		if((cx > posl[list][0] + 2) && (cx < posl[list][1] - 2))
		if ((cy > posl[list][2] + 2) && (cy < posl[list][3] - 2))
		{
			ptfunct = affichc;
			return(1);
		}
	}
return(0);
}
/*********** TEST POSITION CRAYON / CANDIDAT UNIQUE ***********************/
testposc()
{
	if(cy > 452)
	{
		if(dejasel != 0)
		{
			if((cx > 10) && (cx < 310))
			{
				dejasel = 0;
				typesel = 0;
				swpas   = 0;
				swnoir  = 0;
				swconf  = 0;
				zero_select(scr,list,1);
				return(0);
			}
			if((cx > 330) && (cx < 630))
			{
				if((scrutin[scr].nom_s == '4') && (Choix_RLg == 'F'))
					swVLR = 0;
				scr ++;
				ptfunct = boucle;
				return(1);
			}
			return(0);
		}
		else
		{
			if((cx > 110) && (cx < 530))
			{
				ptfunct = les_listes;
				return(1);
			}
			else
			return(0);
		}
	}
/****************************************************************************/
/****************************************************************************/
	else
	{
		if (cfgS[scrutin[scr].TYPE_SCRUTIN].swVM == 0)
		for( i = 0;i <= candid;i++)
		{
			if((cx >= posc[i][0]) && (cx < posc[i][1]))
			if((cy >= posc[i][2]) && (cy <= posc[i][3]))
			{
			switch(typesel)
			{
				/*pas de vote */        case 0:if(i == 0) typesel = 1;
				else
					typesel = (i > te[scr][list]) ? 3:2;

				break;
				/* tete de liste */     case 1:if(memoc[scr][list][i] == 0)
				{
					typesel = (i > te[scr][list]) ? 3:2;
					break;
				}
				else return(0);
				/* un effectif */       case 2:if(memoc[scr][list][i] == 0)
				{
					if (i == 0)  /* tete de liste */
						break;
					if(i < (te[scr][list]) + 1)
					{
						type_erreur = 3;
						message_error(type_erreur);
						return(1);
					}
					else
						typesel = 5;
					break;
				}
				else return(0);
				/* suppleant */         case 3:if(memoc[scr][list][i] == 0)
				{
					if (i == 0)  /* tete de liste */
						break;
					if(i > te[scr][list])
					{
						type_erreur = 4;
						message_error(type_erreur);
						return(1);
					}
					else
						typesel = 5;
					break;
				}
				else return(0);
				/* complet */           default:if(memoc[scr][list][i] == 0)
				{
					if (i == 0)  /* tete de liste */
						break;
					if(i < (te[scr][list]) + 1)
						type_erreur = 3;
					else
						type_erreur = 4;
					message_error(type_erreur);
					return(1);
				}
			}
			++dejasel;
			memor(scr,list,i);
			cercle(i,0);
			return(0);
			}
		}
		else
			for( i = 0;i <= candid;i++)
			{
				if((cx >= posc[i][0]) && (cx < posc[i][1]))
				{
					if((cy >= posc[i][2]) && (cy <= posc[i][3]))
					{
						if(memoc[scr][list][i] == 0) /* S�ection */
						{
							++dejasel;
							memor(scr,list,i);
							cercle(i,0);
						}
						else /* D�S�ection */
						{
							--dejasel;
							memor2(scr,list,i);
							cercle(i,1);
						}
						return(0);
					}
				}
			}
		return(0);
	}
}
/****************************************************************************/
/****************************************************************************/
int message_error(int type_erreur)
{
	char chscrut[100];
	setfillstyle(SOLID_FILL, WHITE);
	bar(0,0,639,479);

	setfillstyle(SOLID_FILL, 7);
	bar(20,100,620,280);
	rectangle(20,100,620,280);
	rectangle(21,101,619,279);

	settextstyle( SMALL_FONT, HORIZ_DIR, 9);
	settextjustify(CENTER_TEXT, TOP_TEXT);
	setcolor (BLACK);
	outtextxy(320,125,Message_point[64]);
	outtextxy(321,125,Message_point[64]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify(CENTER_TEXT, TOP_TEXT);

	dejasel = 0;
	typesel = 0;
	swpas   = 0;
	swnoir  = 0;
	swconf  = 0;

	switch ( type_erreur )
	{
		case 1 :
			outtextxy(320 , 200 ,Message_point[60]  ) ;
			outtextxy(321 , 200 ,Message_point[60]  ) ;
			break ;

		case 2 :
			outtextxy(320 , 200 ,Message_point[61] ) ;
			outtextxy(321 , 200 ,Message_point[61] ) ;
			break ;


		case 3 :
			outtextxy(320 , 200 , Message_point[62] ) ;
			outtextxy(321 , 200 , Message_point[62] );
			break ;

		case 4 :
			outtextxy(320 , 200 , Message_point[63] ) ;
			outtextxy(321 , 200 , Message_point[63] );
			break ;
	}

	settextstyle(SMALL_FONT, HORIZ_DIR, 7);
	settextjustify(CENTER_TEXT,BOTTOM_TEXT);
	outtextxy(320,255,Message_point[66]);
	outtextxy(321,255,Message_point[66]);

	calmos(6);
	setfillstyle(SOLID_FILL,BLACK);
	bar(0,0,639,479);
	zero_select(scr,list,0);
}
/************************ memorisation d'une selection *********************/
/* START CONFIDENTIAL */
memor(x,y,z)
int x,y,z;
{
	memoc[x][y][z] = 1;
	memol[x][y] = 1;
	memos[x] = 1;
}

memor2(x,y,z)
int x,y,z;
{
	int i;

	memoc[x][y][z] = 0;

	memol[x][y] = 0;
	for(i = 0;i <= candid;i++)
	{
		if(memoc[x][y][i] == 1)
		{
			memol[x][y] = 1;
			break;
		}
	}
	if(dejasel == 0)
		swDeselect = 1;

	memos[x] = 0;
	for(i = 0;i < atoi(scrutin[scr].nbrelist_s);i++)
	{
		if(memol[x][i] == 1)
		{
			memos[x] = 1;
			break;
		}
	}
}
/* END CONFIDENTIAL */
/************************ annulation d'une selection ***********************/
zero_select(x,y,sw)
int x,y,sw;
{
/* START CONFIDENTIAL */
	int m;


	memos[x] = 0;
	memol[x][y] = 0;

	for(m = 0; m < 90; m++)
	{
		if( memoc[x][y][m] != 0 )
		{
			memoc[x][y][m] = 0 ;
			if(sw)
				cercle(m,1);
		}
	}
/* END CONFIDENTIAL */
}
/***************************************************************************/
nettmem()
{
	/* START CONFIDENTIAL */
	int x,y,z;

	for(x = 0;x < nbrscrutin;x++)
	{
		for(y = 0;y < 51;y++)
		for(z = 0;z < 90;z++)
		memoc[x][y][z] = 0;
	}
	for(x = 0;x < nbrscrutin;x++)
	{
		for(y = 0;y < 51;y++)
			memol[x][y] = 0;
	}
	for(x = 0;x < nbrscrutin;x++)
		memos[x] = 0;
	/* END CONFIDENTIAL */
}
/**************************** FIN DE VOTE **********************************/
int fin()
{
	char c;
	char szBuff[64];
	int i;
	
	/* Compteur permettant le blocage de la machine à voter si l'on introduit
	 par 3 fois une carte de manière erronée */
	static int lsiCptBadCardRead = 0;
	
	if(msg_error != 2)
		lsiCptBadCardRead = 0;
	switch (msg_error)
	{
		case 0:
			ptfunct = Ecrire;
			return(0);
		case 3:
			setfillstyle(SOLID_FILL, WHITE); /* pb Ecriture */
			bar(0,0,639,479);
			setcolor(BLACK);
			settextstyle( SMALL_FONT,HORIZ_DIR, 8);
			settextjustify(CENTER_TEXT, CENTER_TEXT);

			outtextxy(320,150, Message_point[53]);
			outtextxy(321,150, Message_point[53]);

			outtextxy(320,250, Message_point[27]);
			outtextxy(321,250, Message_point[27]);

			outtextxy(320,350, Message_point[44]);
			outtextxy(321,350, Message_point[44]);
			
			Read_Commande();
			fraude(RG,1);
			point();
			while (klavier) point();
			status = Ejectons();
			if (status != 0)
			{
				msg_error = status;
				ptfunct = fin;
				return(0);
			}
			calmos2(2);
		break;
	case 2: /* Probleme de lecture ou d'ecriture */
		++lsiCptBadCardRead;
	case 4:
		setfillstyle(SOLID_FILL, WHITE);
		bar(0,0,639,479);
		setcolor(BLACK);
		settextstyle( SMALL_FONT,HORIZ_DIR, 8);
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		
		if(lsiCptBadCardRead < 3)
		{
			outtextxy(320,150, Message_point[34]);
			outtextxy(321,150, Message_point[34]);
			outtextxy(320,151, Message_point[34]);

			outtextxy(320,250, Message_point[35]);
			outtextxy(321,250, Message_point[35]);
			outtextxy(320,251, Message_point[35]);
	
			outtextxy(320,350, Message_point[36]);
			outtextxy(321,350, Message_point[36]);
			outtextxy(320,351, Message_point[36]);
		}
		else
		{
			outtextxy(320,200, Message_point[26]);
			outtextxy(321,200, Message_point[26]);
			outtextxy(320,201, Message_point[26]);
	
			outtextxy(320,300, Message_point[44]);
			outtextxy(321,300, Message_point[44]);
			outtextxy(320,301, Message_point[44]);
		}
		
		if(lsiCptBadCardRead > 2)
		{
			lsiCptBadCardRead = 0;
			fraude(RG,1);
			while (klavier) point();
		}
		calmos2(2);
		status = Ejectons();
		if (status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		calmos2(2);
		break;

	case 7:
		setfillstyle(SOLID_FILL, WHITE);
		setcolor(BLACK);
		bar(0,0,639,479);
		settextstyle(SMALL_FONT,HORIZ_DIR,8);
		settextjustify(CENTER_TEXT, CENTER_TEXT);

		outtextxy(320,150, Message_point[34]);
		outtextxy(321,150, Message_point[34]);
		outtextxy(320,151, Message_point[34]);

		outtextxy(320,250, Message_point[35]);
		outtextxy(321,250, Message_point[35]);
		outtextxy(320,251, Message_point[35]);

		outtextxy(320,350, Message_point[36]);
		outtextxy(321,350, Message_point[36]);
		outtextxy(320,351, Message_point[36]);

		calmos(4);
		fraude(GG,0);
		break;
	case 1:
	case 5:
	case 6: /* Probleme technique */
	case 9:
		Lecteur_Init();     /*Attention au retour */
		setfillstyle(SOLID_FILL, WHITE);
		bar(0,0,639,479);
		setcolor(BLACK);
		settextstyle(SMALL_FONT,HORIZ_DIR, 10);
		settextjustify(CENTER_TEXT, TOP_TEXT);
		outtextxy(320,200, Message_point[53]);
		outtextxy(321,200, Message_point[53]);
		outtextxy(320,201, Message_point[53]);

		settextjustify(CENTER_TEXT, BOTTOM_TEXT);
		settextstyle(SMALL_FONT, HORIZ_DIR, 7);
		outtextxy(320,300, Message_point[44]);
		outtextxy(321,300, Message_point[44]);

		fraude(GG,0);
		calmos2(2);
		status = Ejectons();
		if (status != 0)
			{
				msg_error = status;
				ptfunct = fin;
				return(0);
			}
		calmos2(2);
		break;

	case 8:
			calmos2(1);
			ptfunct = recom;
			return(0);

	case 11:fraude(GG,0);   /* Deja vote avec la carte */
		setfillstyle(SOLID_FILL, WHITE);
		bar(0,0,639,479);
		setcolor(BLACK);

		settextstyle(SMALL_FONT,HORIZ_DIR, 8);
		settextjustify(CENTER_TEXT, CENTER_TEXT);

		outtextxy(320, 160, Message_point[32]);
		outtextxy(321, 160, Message_point[32]);
		outtextxy(320, 161, Message_point[32]);

		outtextxy(320, 230, Message_point[33]);
		outtextxy(321, 230, Message_point[33]);
		outtextxy(320, 231, Message_point[33]);

		settextstyle(SMALL_FONT,HORIZ_DIR, 8);

		outtextxy(320, 320, Message_point[90]);
		outtextxy(321, 320, Message_point[90]);
		outtextxy(320, 321, Message_point[90]);

		choix_visu();
		if(visu == 1)
		{
			ptfunct = View;
			return(0);
		}
		bar(0, 0, 639,479);
		setcolor ( BLACK );
		settextstyle( SMALL_FONT, HORIZ_DIR, 9 );
		outtextxy(320,160, Message_point[39]);
		outtextxy(321,160, Message_point[39]);
		outtextxy(320,161, Message_point[39]);

		outtextxy(320,300, Message_point[40]);
		outtextxy(321,300, Message_point[40]);
		outtextxy(320,301, Message_point[40]);

		status = Ejectons();
		if(status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		calmos2(2);
		break;
	case 10:
	case 12:
		setfillstyle(SOLID_FILL, WHITE);/* F R A U D E */
		bar(0,0,639,479);
		setcolor(BLACK);
		settextstyle(SMALL_FONT, HORIZ_DIR,10);
		settextjustify(CENTER_TEXT, BOTTOM_TEXT);
		outtextxy(320, 180, Message_point[28]);
		outtextxy(320, 181, Message_point[28]);

		settextjustify(CENTER_TEXT, TOP_TEXT);
		outtextxy(320, 230, Message_point[29]);
		outtextxy(320, 231, Message_point[29]);
		settextjustify(CENTER_TEXT, BOTTOM_TEXT);
		settextstyle(SMALL_FONT, HORIZ_DIR, 7);

		outtextxy(320,400, Message_point[44]);
		outtextxy(320,401, Message_point[44]);

		fraude(RG,1);
		while (klavier) point();
		status = Ejectons();
		if (status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		break;

	case 13:
		fraude(RG,0);   /* Eject de la carte */
		setfillstyle(SOLID_FILL, WHITE);
		bar(0,0,639,479);
		setcolor(BLACK);
		settextstyle(SMALL_FONT,HORIZ_DIR,10);
		settextjustify(CENTER_TEXT, CENTER_TEXT);

		outtextxy(320, 240, Message_point[37]);
		outtextxy(321, 240, Message_point[37]);
		outtextxy(320, 241, Message_point[37]);

		calmos2(2);
		status = Ejectons();
		if(status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		calmos2(2);
		break;
	case 14:
		fraude(RG,0);   /* Eject de la carte entre trop tot */
		status = Ejectons();
		if(status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		 break;
	case 15:
		break;
	case 16:
		setfillstyle(SOLID_FILL,WHITE); /* Fin Visualisation */
		bar(0, 0, 639,479);
		setcolor( BLACK );
		settextstyle( SMALL_FONT, HORIZ_DIR, 9 );
		outtextxy(320,160, Message_point[39]);
		outtextxy(321,160, Message_point[39]);
		outtextxy(320,161, Message_point[39]);

		outtextxy(320,300, Message_point[40]);
		outtextxy(321,300, Message_point[40]);
		outtextxy(320,301, Message_point[40]);

		status = Ejectons();
		if(status != 0)
		{
			msg_error = 6;
			ptfunct = fin;
			return(0);
		}
		calmos2(2);
		break;
	case 17:
		setfillstyle(SOLID_FILL,WHITE);
		bar(0, 0, 639,479);
		setcolor ( BLACK );
		settextstyle( SMALL_FONT, HORIZ_DIR, 10 );
		outtextxy(320,80, Message_point[38]);
		outtextxy(321,80, Message_point[38]);
		outtextxy(320,81, Message_point[38]);
		settextstyle( SMALL_FONT, HORIZ_DIR, 7 );

		outtextxy(320,150, Message_point[39]);
		outtextxy(321,150, Message_point[39]);
		outtextxy(320,151, Message_point[39]);

		outtextxy(320,200, Message_point[40]);
		outtextxy(321,200, Message_point[40]);
		outtextxy(320,201, Message_point[40]);

		outtextxy(320,250, Message_point[108]);
		outtextxy(321,250, Message_point[108]);
		outtextxy(320,251, Message_point[108]);

		outtextxy(320,300, Message_point[109]);
		outtextxy(321,300, Message_point[109]);
		outtextxy(320,301, Message_point[109]);

		outtextxy(320,350, Message_point[110]);
		outtextxy(321,350, Message_point[110]);
		outtextxy(320,351, Message_point[110]);

		status = Ejectons();
		if(status != 0)
		{
			msg_error = status;
			ptfunct = fin;
			return(0);
		}
		for(i=0; i<3; ++i)
			Eject_Commande();
		
		if(calmos4(8))
		{
			ptfunct = recom_view;
			return(0);
		}
		break;
	}
	ptfunct = recom;
       return(0);
}
/***************************************************************************/
/****************************** RESULTAT DU VOTE **************************/
int result()
{
	int liste;

	debut = _C_VOTE_PLACE;

	/* START CONFIDENTIAL */
	Encode_passage(carte);
	for(scr = 0;scr < nbrscrutin;scr++)
	{
		if (cfgS[scrutin[scr].TYPE_SCRUTIN].swVM == 0)
		{
			effectif = 0;
			suppleant= 0;
			if(memos[scr] == 0)
				liste    = 99;
			else
				if(memos[scr] == 2)
					liste    = 98;
				else
				{
					for(y = 0;y < atoi(scrutin[scr].nbrelist_s);y++)
					{
						if(memol[scr][y] == 1)
						{
							liste = y + 1;
							for(z=0;(z < te[scr][y] + 1);z++)
								if(memoc[scr][y][z] == 1)
									effectif = z + 1;
							for(;z < (te[scr][y] + ts[scr][y] + 1); z++)
								if(memoc[scr][y][z] == 1)
									suppleant = z + 1;
						}
					}
				}
			Encode_Vote(carte,liste,effectif,suppleant);
		}
		else
		{
			if(memos[scr] == 0)
				liste    = 99;
			else
				if(memos[scr] == 2)
					liste    = 98;
				else
				{
					for(y = 0;y < atoi(scrutin[scr].nbrelist_s);y++)
					{
						if(memol[scr][y] == 1)
							liste = y + 1;
					}
				}
			Encode_Vote_Multiple(carte,scr,liste);
		}
	}
	Calcul_Crc(carte,_C_TEST_PLACE);
	Encode_Crc(carte);
	ptfunct = fin;
	return(0);
}
/****************************************************************************/
/****************************************************************************/
int Ecrire()
{
	msg_error = 3;

#if (NOCARTE_CHECK >= 1)
	if (swNOCarte != _NOCARTE) { /* BEGIN WITH Carte */
#endif

#if (LOOPWRITE_CARTE == 0)
	status = Write_Commande();
	if(status != 0)
		msg_error = 3;
	else
		msg_error = 17;
#endif

#if (LOOPWRITE_CARTE >= 1)
	memcpy(carteBAK, carte, (size_t)LCARTEBAK);
	for(nPassWriteCarte = 1; nPassWriteCarte <= 3; nPassWriteCarte++)
	{
		status = Write_Commande();
		if(!status)
		{
			Lect_Buffer[0] = '\0';
			status = Read_Commande();
			if(status == 0)
			{
				if(!strcmp(carteBAK, Lect_Buffer))
				{
					msg_error = 17;
					break;
				}
			}
		}
		else
			msg_error = 3;
	}
#endif
#if (NOCARTE_CHECK >= 1)
	} /* END WITH Carte */
	else /* DEMO */
		msg_error = 17;
#endif

ptfunct = fin;
return(0);
}
/* END CONFIDENTIAL */
/***************************************************************************/
param()
{
#include "lparam1.c"
}
/*****************************************************************************/
param_Candidat()
{
	
	nbr_sup = atoi(scrutin[scr].scrutin_max_supp);
	nbr_max = atoi(scrutin[scr].scrutin_max_cand) - 1;/* tete de liste */
	nbc_col = iRuptureColonne;
	nombre_col  = (nbr_max - nbr_sup) / nbc_col;
	if(((nbr_max - nbr_sup) % nbc_col) > 0) nombre_col++;
	if(nbr_sup > 0)
		nombre_col++;
	if(nombre_col == 5)
		nombre_col--;
	if(nombre_col > 5)
		msg_erreur("Trop de colonnes...");
	if(nbr_sup > (nbc_col - 1))
		msg_erreur("Trop de supplants...");

#include "c_meth4.c"
}
/***************************************************************************/
/******          MENU DE CONFIRMATION DU CHOIX DU BLANC              ******/
/***************************************************************************/
int selectc()
{
	setfillstyle(SOLID_FILL, BLACK);
	bar(0, 30, 639, 479);
	setfillstyle(SOLID_FILL, WHITE);
	bar(20,80,620,300);
	setcolor (BLACK);
	rectangle(38,198,302,292);
	rectangle(39,199,301,291);
	rectangle(338,198,602,292);
	rectangle(339,199,601,291);
	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify ( CENTER_TEXT , TOP_TEXT ) ;
	if(LANGUE == 'F')
	  {
			outtextxy(170,220, Message_point[59]);
			outtextxy(170,260, Message_point[58]);

			outtextxy(470,220, Message_point[57]);
			if(strstr(Message_point[57],"confirmer"))
				    outtextxy(552,220,"confirmer");
			outtextxy(470,260, Message_point[58]);
	  }
	else
	  {
			outtextxy(170,220, Message_point[57]);
			outtextxy(170,260, Message_point[59]);

			outtextxy(470,220, Message_point[57]);
			outtextxy(470,260, Message_point[58]);
			if(strstr(Message_point[58],"bevestigen"))
				    outtextxy(509,260, "bevestigen");

	  }

	settextstyle( SMALL_FONT, HORIZ_DIR, 7);
	outtextxy( 320, 110, Message_point[55]);
	outtextxy( 320, 140, Message_point[56]);

/* << el2007 */
	/* On laisse s'�ouler un petit temps */
	calmos3();
/* el2007 >> */
/*********** TEST DE CONFIRMATION **********/

#if (LPMNGLIB >=1)
	if (swLPcursor)
		lpPosxy(639, 479);
#endif
	while(klavier)
	{
#if (LPMNGLIB >=1)
		if (swLPcursor)
			  lpCursorOn();
#endif
		if(point())
		{
#if (LPMNGLIB >=1)
		  lpCursorOff();
#endif
			if((cy > 200) && (cy < 290))
			{
				if((cx > 340) && (cx <  600))
				{
					if((scrutin[scr].nom_s == '4') && (Choix_RLg == 'F'))
					swVLR = 0;
					scr++;
					ptfunct = boucle;
					return(0);
				}
				else
				 if((cx > 40 ) && (cx < 300))
					     {
						ptfunct = les_listes;
						return(0);
					      }
			}
		 }
	 }
#if (LPMNGLIB >=1)
      lpCursorOff();
#endif

  msg_error = 13;
  ptfunct = fin;
  return(0);
}

/***************************************************************************/
/******          MENU DE CONFIRMATION DU CHOIX DU College            ******/
/***************************************************************************/
Select_Col()
{
	setfillstyle(SOLID_FILL, BLACK);
	bar(0, 30, 639, 479);
	setfillstyle(SOLID_FILL, WHITE);
	bar(100,80,540,300);
	setcolor (BLACK);

	rectangle(108,198,282,292);
	rectangle(109,199,281,291);
	rectangle(358,198,532,292);
	rectangle(359,199,531,291);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify ( CENTER_TEXT , TOP_TEXT ) ;
	if(LANGUE == 'F')
	{
		outtextxy(195,220, Message_point[69]);
		outtextxy(196,220, Message_point[69]);
	
		outtextxy(195,240, Message_point[70]);
		outtextxy(196,240, Message_point[70]);
	
		outtextxy(446,220, Message_point[71]);
		outtextxy(447,220, Message_point[71]);
	
		outtextxy(446,240, Message_point[72]);
		outtextxy(447,240, Message_point[72]);
	}
	else
	{
		outtextxy(195,220, Message_point[71]);
		outtextxy(196,220, Message_point[71]);
	
		outtextxy(195,240, Message_point[72]);
		outtextxy(196,240, Message_point[72]);
	
		outtextxy(446,220, Message_point[69]);
		outtextxy(447,220, Message_point[69]);
	
		outtextxy(446,240, Message_point[70]);
		outtextxy(447,240, Message_point[70]);
	}

	settextstyle( SMALL_FONT, HORIZ_DIR, 7);
	outtextxy( 320, 110, Message_point[68]);
	outtextxy( 320, 111, Message_point[68]);
	settextstyle( SMALL_FONT, HORIZ_DIR, 6);

/*********** TEST DE CONFIRMATION **********/

#if (LPMNGLIB >=1)
	if (swLPcursor) {
	  lpPosxy(639, 479);
	}
#endif
	while(klavier)
	{
#if (LPMNGLIB >=1)
		if (swLPcursor) {
		      lpCursorOn();
		}
#endif
		if(point())
		{
#if (LPMNGLIB >=1)
		  lpCursorOff();
#endif
			if((cy > 200) && (cy < 290))
			{
				if((cx > 360) && (cx <  530))
				{
					if(LANGUE == 'F')
						Choix_College = 'N';
					else
						Choix_College = 'F';
					return(0);
				}
				else
				  if((cx > 110 ) && (cx < 280))
					     {
					if(LANGUE == 'F')
						Choix_College = 'F';
					else
						Choix_College = 'N';
						return(0);
					     }
			}
		 }
	 }
#if (LPMNGLIB >=1)
  lpCursorOff();
#endif

  return(1);
}

/*****************************************************************************/
/******       MENU DE CONFIRMATION DU CHOIX DU REGIME LINGUISTIQUE      ******/
/*****************************************************************************/

int Select_RLg()
{
	setfillstyle(SOLID_FILL, BLACK);
	bar(0, 30, 639, 479);
	setfillstyle(SOLID_FILL, WHITE);
	bar( 50,80,590,300);

	setcolor (BLACK);
	rectangle(108,198,282,292);
	rectangle(109,199,281,291);

	rectangle(358,198,532,292);
	rectangle(359,199,531,291);

	settextjustify ( CENTER_TEXT , TOP_TEXT ) ;
	settextstyle( SMALL_FONT, HORIZ_DIR, 5);
	outtextxy(195,220, Message_point[80]);
	outtextxy(196,220, Message_point[80]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	outtextxy(195,240, Message_point[81]);
	outtextxy(196,240, Message_point[81]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 5);
	outtextxy(446,220, Message_point[82]);
	outtextxy(447,220, Message_point[82]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	outtextxy(446,240, Message_point[83]);
	outtextxy(447,240, Message_point[83]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 7);
	outtextxy( 320, 110, Message_point[79]);
	outtextxy( 320, 111, Message_point[79]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);

	/*********** TEST DE CONFIRMATION **********/

	#if (LPMNGLIB >=1)
	if (swLPcursor) {
		lpPosxy(639, 479);
	}
	#endif
	while(klavier) {
		#if (LPMNGLIB >=1)
		if (swLPcursor) {
			lpCursorOn();
		}
		#endif
		if (point()) {
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if ((cy > 200) && (cy < 290)) {
				if ((cx > 360) && (cx <  530)) {
					Choix_RLg = 'N'; return(0);
				}
				else {
					if ((cx > 110 ) && (cx < 280)) {
						Choix_RLg = 'F';
						return(0);
					}
				}
			}
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif

	return(1);
}
/***************************************************************************/
/******                    MENU DE CHOIX DE LA LANGUE                 ******/
/***************************************************************************/
int choix_langue()
{
	setfillstyle(SOLID_FILL,BLACK);
	bar( 0,0,639,479);

	setcolor(WHITE);
	setfillstyle(SOLID_FILL, WHITE);
	bar(50,80,590,300);
	setcolor (BLACK);

	rectangle(108,198,282,292);
	rectangle(109,199,281,291);

	rectangle(358,198,532,292);
	rectangle(359,199,531,291);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify ( CENTER_TEXT , TOP_TEXT ) ;
	outtextxy(195,240,message_1[45]);
	outtextxy(196,240,message_1[45]);
	outtextxy(445,240,message_2[45]);
	outtextxy(446,240,message_2[45]);
	outtextxy( 320, 110, message_1[52]);
	outtextxy( 321, 110, message_1[52]);

	outtextxy( 320, 140, message_2[52]);
	outtextxy( 321, 140, message_2[52]);

	#if (LPMNGLIB >=1)
	if (swLPcursor)lpPosxy(639, 479);
	#endif

	while(1)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor) lpCursorOn();
		#endif

		if(point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if((cy > 200) && (cy < 290))
			{
				if((cx > 110) && (cx <  280))
				{
					LANGUE = 'F';
					Message_point = message_1;
					return(0);
				}
				else
					if((cx >  360) && (cx < 530))
					{
						LANGUE = 'N';
						Message_point = message_2;
						return(0);
					}
			}
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif
}
/***************************************************************************/
int Confirm_langue()
{
	setfillstyle(SOLID_FILL,BLACK);
	bar( 0,0,639,479);
	setcolor(WHITE);
	setfillstyle(SOLID_FILL, WHITE);
	bar(50,80,590,300);
	setcolor(BLACK);

	rectangle(108,198,282,292);
	rectangle(109,199,281,291);

	rectangle(358,198,532,292);
	rectangle(359,199,531,291);

	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	settextjustify ( CENTER_TEXT , TOP_TEXT ) ;
	outtextxy(195,230,Message_point[75]);
	outtextxy(196,230,Message_point[75]);
	if( LANGUE == 'F')
	{
		outtextxy(195,250,message_2[75]);
		outtextxy(196,250,message_2[75]);
	}
	else
	{
		outtextxy(195,250,message_1[75]);
		outtextxy(196,250,message_1[75]);
	}

	outtextxy(445,230,Message_point[74]);
	outtextxy(446,230,Message_point[74]);
	if( LANGUE == 'F')
	{
		outtextxy(445,250,message_2[74]);
		outtextxy(446,250,message_2[74]);
	}
	else
	{
		outtextxy(445,250,message_1[74]);
		outtextxy(446,250,message_1[74]);
	}
	outtextxy( 320, 120, Message_point[65]);
	outtextxy( 321, 120, Message_point[65]);

	outtextxy( 320, 140, Message_point[76]);
	outtextxy( 321, 140, Message_point[76]);


	#if (LPMNGLIB >=1)
	if (swLPcursor) {
		lpPosxy(639, 479);
	}
	#endif
	while(1)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor) {
			lpCursorOn();
		}
		#endif
		if (point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if((cy > 200) && (cy < 290))
			{
				if((cx > 110) && (cx <  280))
					return(0);
				else
					if((cx >  360) && (cx < 530))
						return(1);
			}
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif
}
/********************** GESTION DES TEMOINS LUMINEUX *********************/
fraude(char val,int BELL) /*   OFF; RR rouge; GG VERT ; RG ROUGE + VERT */
{

	outportb(0x308,val);
	if(BELL)
		for(i = 0;i < 5;i++)
			putch(0x07);
}
/***************************************************************************/
calmos(int tmp)
{
	unsigned long far *timer = ( unsigned long * )0x46C;
	temptimes = *timer;         /* Valeur du temps */
	while ((*timer - temptimes <= (unsigned long) tmp * 19) && (!point()) && klavier);
}
/***************************************************************************/
calmos2(int tmp)
{
	unsigned long far *timer = ( unsigned long * )0x46C;
	temptimes = *timer;         /* Valeur du temps */
	while ((*timer - temptimes <= (unsigned long) tmp * 19));
}
/* << el2007 */
/***************************************************************************/
calmos3(void)
{
	unsigned long far *timer = ( unsigned long * )0x46C;
	temptimes = *timer;         /* Valeur du temps */
	while ((*timer - temptimes <= (unsigned long) 8));
}
/***************************************************************************/
calmos4(int tmp)
{
	unsigned long far *timer = ( unsigned long * )0x46C;
	temptimes = *timer;         /* Valeur du temps */
	while ((*timer - temptimes <= (unsigned long) tmp * 19))
	{
		if(!Wait_Card())
			return(1);
	}
	
	return(0);
}
/* el2007 >> */

/***************************************************************************/
bararrondi ( int x1 , int y1 , int x2 , int y2 , int color )
{
	int rayon , cx , cy ;

	rayon = ( y2 - y1 ) / 2;
	cy = y1 + rayon ;
	setcolor( color ) ;
	pieslice( x1+rayon , cy , 90 , 270 , rayon ) ;
	pieslice( x2-rayon , cy , 270 , 360 , rayon ) ;
	pieslice( x2-rayon , cy , 0 , 90 , rayon ) ;
	bar( x1+rayon , y1 , x2-rayon, y2 ) ;

	setcolor( BLACK ) ;
	arc( x1+rayon , cy , 90 , 270 , rayon ) ;
	arc( x2-rayon , cy , 270 , 360 , rayon ) ;
	arc( x2-rayon , cy , 0 , 90 , rayon ) ;
	line( x1+rayon , y1 , x2-rayon , y1 ) ;
	line( x1+rayon , y2 , x2-rayon , y2 ) ;
}
/***************************************************************************/
int Demande_Floppy()
{

	int test;

	/* << el2007 Floppy.be sur disquette de d�arrage */
	char bFloppyOnBoot = 0;
	/*
	setfillstyle(SOLID_FILL,WHITE); / Set to solid fill in color   /
	bar( 0,0,639,479);      / Draw backgnd /
	setcolor( BLACK);


	settextstyle(SMALL_FONT, HORIZ_DIR,12);
	settextjustify(CENTER_TEXT, CENTER_TEXT);

	if(code_langue > '3')
	{
	outtextxy(320, 160, message_1[95]);
	outtextxy(321, 160, message_1[95]);
	outtextxy(320, 320, message_2[95]);
	outtextxy(321, 320, message_2[95]);
	}
	else
	{
	outtextxy(320, 240, message_1[95]);
	outtextxy(321, 240, message_1[95]);
	}

	test = 0;
	*/

	/* Floppy.be se trouve-t-il sur la m�e disquette ? */
	test = access("a:Floppy.be", 0);

	if(test != 0) /* Floppy.be demander une autre disquette */
	{
		setfillstyle(SOLID_FILL,WHITE); /* Set to solid fill in color   */
		bar( 0,0,639,479);      /* Draw backgnd */
		setcolor( BLACK);


		settextstyle(SMALL_FONT, HORIZ_DIR,12);
		settextjustify(CENTER_TEXT, CENTER_TEXT);

		if(code_langue > '3')
		{
			outtextxy(320, 160, message_1[95]);
			outtextxy(321, 160, message_1[95]);
			outtextxy(320, 320, message_2[95]);
			outtextxy(321, 320, message_2[95]);
		}
		else
		{
			outtextxy(320, 240, message_1[95]);
			outtextxy(321, 240, message_1[95]);
		}

		test = 0;
	}
	else
	bFloppyOnBoot = 1;
	/* el2007 >> */
	while(test < 1)
	{
		test =access("a:Floppy.be", 0);
		if(test == 0)
		{
			test = Initialise_Rijndael();
			Initialise_RijndaelMini();
		}
	}
	/* << el2007 */
	if(!bFloppyOnBoot)
	{
		/* el2007 >> */
		setfillstyle(SOLID_FILL,WHITE); /* Set to solid fill in color   */
		bar( 0,0,639,479);      /* Draw backgnd */
		setcolor( BLACK);

		settextstyle(SMALL_FONT, HORIZ_DIR,12);
		settextjustify(CENTER_TEXT, CENTER_TEXT);

		if(code_langue > '3')
		{
			outtextxy(320, 160, message_1[96]);
			outtextxy(321, 160, message_1[96]);
			outtextxy(320, 320, message_2[96]);
			outtextxy(321, 320, message_2[96]);
		}
		else
		{
			outtextxy(320, 240, message_1[96]);
			outtextxy(321, 240, message_1[96]);
		}

		/* << el2007 */
	}
	/* el2007 >> */
	test = 0;
	while(test < 1)
	{
		test = access("a:Machvots.tbl", 0);
		if (test == 0)
		test++;
	}
}
/***************************************************************************/
