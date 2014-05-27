/***************************************************************************/
/* JITES                                                                   */
/* prog AL-KHANSA JAMAL                                                    */
/* LANGAGE TC                                                              */
/*                                                                         */
/*                                VARIABLES                                */
/* Hcadre_candid = hauteur du cadre candidat                               */
/* Interl_candid = Interligne entre les cadres candidats                   */
/* Hsaisie_candid= hauteur de la zone de saisie dans cadre candidat        */
/* Lcadre_candid  = longueur du cadre candidat                             */
/* Interc_candid  = Intercolonne entre les cadres candidats                */
/* x_dep          = valeur de la coordonn‚e x de la  1 ere colonne         */
/* x_ent_scr      = valeur de x de l'entete du scrutin                     */
/* x_numl         = valeur de x du numero de la liste                      */
/* x_noml         = valeur de x du nom de la liste                         */
/* nbc_col        = nombre de candidats par colonne                        */
/***************************************************************************/
/* << 10.02 */
/* << 10.02d */
/*#define CDT_NUM_WIDTH   24 / 0=NONE  !0=Numéro de candidat visible et = 'largeur du champs numéro' */
#define CDT_NUM_WIDTH   28 /* 0=NONE  !0=Numéro de candidat visible et = 'largeur du champs numéro' */
/* 10.02d >> */
/* 10.02 >> */
#define BAL_LISTE_INCOMPLETE    0  /* 1=Yes  0=No                            */

#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <bios.h>
#include <stdio.h>
#include <string.h>
#include <graphics.h>
#include <string.h>
#include "PCX.C"
#include "BMP.C"
union REGS rin,rout;
int affichc();
/* << DIGI */
int affichd();
char JITES = 0;
char DIGIVOTE = 0;
/* DIGI >> */
/*************************************************************************/
FILE *pf;
int i,j,x,y;
int x_dep,x_ent_scr,x_numl,x_noml,nbc_col;
int swpas = 0;
int swnoir = 0;
int swconf = 0;
int neffectif,nsuppleant;
int Hcadre_candid ,Interl_candid, Hsaisie_candid,Lsaisie_candid,Lcadre_candid;
int LAsaisie_candid, HAsaisie_candid;
int SWlisteincomplete = 0;
int Interc_candid ;
int scr = 0,list = 0,candid = 0;
int posc[120][4];
int cx,cy,rc,k,k1,w,temps = 1,font_nr,j,x;
int dejasel = 0, typesel = 0, suppleant,effectif;
int unic = 0; /* switch de test d'unicit‚ de vote dans les listes */
int Debut_Affichage;
char  c,*scrnom,*filename;
int nbre_vote = 0;
int nbr_max,nbr_sup;
int nbr_maxEFF = 0, nbr_maxSUP = 0;
int nombre_col,nparcol;
char f_name[50], Sname[50];

char Arrond[3+1] = "   ";
char nom_s = ' ';
int affTL_code = 1, affColMax = 0;

char  nom_s, LANGUE,Choix_College;
/* << el2007 */
/*char LISTE[5],bufnom[10];*/
char LISTE[5],bufnom[16];
char swPcxOnly;
/* el2007 >> */
/****************************************************************************/
#include "garnir.c"
#include "aff_cand.c"
#include "jamal.c"
/****************************************************************************/
/* BEGIN NOTUSED
bararrondi ( int x1 , int y1 , int x2 , int y2 , int color )
{
	int rayon , cx , cy ;

	rayon = ( y2 - y1 ) / 2;
	cy = y1 + rayon ;
	setcolor ( color ) ;
	pieslice ( x1+rayon , cy , 90 , 270 , rayon ) ;
	pieslice ( x2-rayon , cy , 270 , 360 , rayon ) ;
	pieslice ( x2-rayon , cy , 0 , 90 , rayon ) ;
	bar ( x1+rayon , y1 , x2-rayon, y2 ) ;

	setcolor( BLACK ) ;
	arc ( x1+rayon , cy , 90 , 270 , rayon ) ;
	arc ( x2-rayon , cy , 270 , 360 , rayon ) ;
	arc ( x2-rayon , cy , 0 , 90 , rayon ) ;
	line ( x1+rayon , y1 , x2-rayon , y1 ) ;
	line ( x1+rayon , y2 , x2-rayon , y2 ) ;

}
  END NOT USED */
/***************************************************************************/

/**************************** CORPS DU PROGRAMME ***************************/
main(argc,argv)
int argc;
char **argv;
{
	int  ind_arg = 1;

	swPcxOnly = 0;

	if(argc != 2) {
		if(argc != 3)
		{
			fprintf(stderr, "simul: one parameter expected.\n");
			return(1);
		}
		else
			swPcxOnly = 1;
	}

/* << DIGI */
	/* Jites / Digivote */
	if(strstr(argv[0], "SIMUL2.EXE"))
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

	registerbgidriver (EGAVGA_driver);    /* Option pour linker avec la  */
	registerbgifont (small_font);         /* librairies GRAPHICS         */
	registerbgifont (sansserif_font);
	Initialize();
	Initpcx();

	strcpy(f_name,argv[ind_arg]);
	Garnir_cand();
	param();
	/**/        list = 1;
/* << DIGI */
	if(JITES == 1)
		affichc();
	else
		affichd();
/* DIGI >> */
	closegraph();

	return(0);
}

/***************************************************************************/
param()
{
/* param_Candidat() define New method (1, 2, 3, 4) */
#define  PARAM_CANDIDAT_METHOD   4
	int nombre_col_bis;

	nombre_col  = (nbr_max - nbr_sup) / nbc_col;
	if(((nbr_max - nbr_sup) % nbc_col) > 0) nombre_col++;
	if(nbr_sup > 0)
		nombre_col++;
	if(nombre_col == 5)
		nombre_col--;
#include "c_meth4.c"
}
/***************************************************************************/
