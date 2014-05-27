#include <stdio.h>
#include <stdlib.h>
#include "garnir.h"
#include <io.h>
#include <fcntl.h>

int fd;
FILE *fps;
long depl,NPIRATE;
struct liste_t *ps, *adrl;
struct cand_t *fc, *adrc;

/***************************** GARNIR TABLE SCRUTIN **************************/
void Garnir_param()
{
	switch(scrutin[scr].TYPE_SCRUTIN)
	 {
		case 1:
			iRuptureColonne = 14;
			break;
		case 2:
			iRuptureColonne = 14;
			break;
		case 3:
			iRuptureColonne = 22;
			break;
		case 4: 
			iRuptureColonne = 14;
			if(strcmp(Arrond,"005") == 0) 
				  iRuptureColonne = 22;
			if(strcmp(Arrond,"002") == 0)
				  iRuptureColonne = 23;
			break;
		case 7:
			iRuptureColonne = 14;
			break;
	 }

}
/***************************** GARNIR TABLE SCRUTIN **************************/
void Garnir_Ind()          /* Garnir les messages */
{
	char bufs[32];

	fps = fopen("machvots.Tbl","r");
	if(fps == NULL)
		msg_erreur("MACHVOTS FILE ERROR");
	fread(bufs,16,1,fps);
	code_langue = bufs[15];
	fclose(fps);
	test_langue(code_langue);
}
/*****************************************************************************/
void Garnir_scrutin()
{
	char bufs[512],c;
	int i,j;
	int SW_EURO = 0,SW_STR = 0;

/*	printf("%ld ---",farcoreleft());*/
	memoc= farcalloc(sizeof(*memoc),5);

	/* START CONFIDENTIAL */
	/*  ANTI PIRATE */

	for(NPIRATE = 0;NPIRATE < 100000;NPIRATE++);


	#if (MEMORY_CHECK >= 1)
	if(Debut_Mcbc() != 7)
		msg_erreur("Incorrect Config");
	#endif

	i = 0;
	nbrscrutin = 0;

	fps = fopen("machvots.tbl","r");
	if(fps == NULL || NPIRATE < 99000)
		msg_erreur("MACHVOTS FILE ERROR");
	fpl = fopen("machvotl.tbl","r");
	if(fpl == NULL || NPIRATE < 99000)
		msg_erreur("MACHVOTL FILE ERROR");
	/* END CONFIDENTIAL */

	fread(bufs,16,1,fps);
	strncpy(jeton,bufs,15);
	jeton[15] = '\0';

	strncpy(Arrond,bufs,3);
	Arrond[3] = '\0';
	strncpy(ElecDate,&jeton[9],2);
	strcat(ElecDate,"/");
	strncat(ElecDate,&jeton[11],2);
	strcat(ElecDate,"/20");
	strncat(ElecDate,&jeton[13],2);

	Calcul_Jeton(jeton,jetonmini);
	fread(bufs,8,1,fps);
	depl = 0;
	C_VOTE_MAX_BYTE = 0;
	while (!feof(fps))
	{
		scrutin[i].code_s = bufs[0];
		strncpy(scrutin[i].scrutin_max_cand,bufs + 1,2);
		scrutin[i].scrutin_max_cand[2] = '\0';
		strncpy(scrutin[i].scrutin_max_supp,bufs + 3,2);
		scrutin[i].scrutin_max_supp[2] = '\0';
		scrutin[i].nom_s = bufs[5];
		scrutin[i].TYPE_SCRUTIN = (int)((int)bufs[5] - (int)'0');
		C_VOTE_MAX_BYTE += (cfgS[scrutin[i].TYPE_SCRUTIN].nmxB +2);

		if((scrutin[i].TYPE_SCRUTIN == 1)||(scrutin[i].TYPE_SCRUTIN == 6))
			SW_EURO = 1;
		if((scrutin[i].TYPE_SCRUTIN == 2)||(scrutin[i].TYPE_SCRUTIN == 3))
			SW_STR = 1;
		strncpy(scrutin[i].nbrelist_s,bufs + 6,2);
		scrutin[i].nbrelist_s[2] = '\0';
		nbrlist[i] = atoi(scrutin[i].nbrelist_s);
		nbrscrutin++;
		Garnir_liste(atoi(scrutin[i].nbrelist_s),i);
		i++;
		fread(bufs,8,1,fps);
	}
	fclose(fps);
	fclose(fpl);

	if((i > 1) && (SW_EURO == 1))  /*  Plusieures Scrutins  */
		GLOBAL_EURO = 1;
	if((i > 2) && (SW_STR == 1))             /* L‚gislatives + autres */
		GLOBAL_STR = 1;
}
/************************** GARNIR TABLE LISTE *******************************/
Garnir_liste(int nbi, int i)
{
	int  iS;
	char bufl[50];
	int j;
	char buffer[10];

	adrl = ps = (struct liste_t *) malloc(sizeof(struct liste_t)* nbi);
	scrutin[i].ptlist = ps;
	for(j=0;j < nbi;j++)
	{
		fgets(bufl,36,fpl);
		ltoa(depl,buffer,10);
		strncpy(ps->adresse_cand,buffer,6);
		ps->adresse_cand[6] = '\0';
		ps->code_l = bufl[0];
		ps->college = bufl[1];
		strncpy(ps->numlist_l,bufl + 2,2);
		if( ps->numlist_l[0] == '0')
			ps->numlist_l[0] = ' ';
		ps->numlist_l[2] = '\0';
		strncpy(ps->nom_l,bufl + 4,25);
		ps->nom_l[25] = '\0';
		for(iS = 24;((ps->nom_l[iS] == ' ') && (iS >= 0)); iS--)
			ps->nom_l[iS] = '\0';
		strncpy(ps->nbreff_l,bufl + 29,3);
		ps->nbreff_l[3] = '\0';
		strncpy(ps->nbrsup_l,bufl + 32,2);
		ps->nbrsup_l[2] = '\0';
		effectif = atol(ps->nbreff_l); /* nb d'effectifs */
		suppleant = atol(ps->nbrsup_l); /* nb de suppleants */
		te[i][j] = effectif;
		ts[i][j] = suppleant;
		/* << el2007 */
		/*depl += (effectif + suppleant) * 81;*/
		if(j < nbi-1)
			depl += (effectif + suppleant) * 81;
		/* el2007 >> */
		ps++;
	}
	ps = adrl;
	test_langue(code_langue);
}
/*************************  GARNIR TABLE CANDIDATS ***************************/
/*****************************************************************************/
Garnir_cand()
{
	char bufc[100],caisse[35];
	char vide[26];
	int kp;
	char *p;
	
	memset(vide,' ',26);
	free(adrc);

	ps = scrutin[scr].ptlist;
	for (candid = 0; candid < list;candid++)ps++;
	deplacement_file_cand = atol(ps->adresse_cand);

	kp = te[scr][list] + ts[scr][list];

	fpc = fopen("machvotc.tbl","r");
	if(fpc == NULL)
	msg_erreur("MACHVOTC FILE ERROR");
	fseek(fpc,deplacement_file_cand,0);
	adrc = fc = (struct cand_t *) malloc(sizeof(struct cand_t) * kp);
	if(adrc == NULL)
	msg_erreur("MALLOC RAM ERROR");

	for(k=0; k < kp; k++)
	{
		fgets(bufc,81,fpc);
		fc->code_c = bufc[0];
		strncpy(fc->numlist_c,bufc + 1,2);
		fc->numlist_c[2] = '\0';
		caisse[25] = '\0';
		strncpy(caisse,vide,25);
		strncpy(caisse,bufc + 3,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strcpy(fc->nom_c,caisse);
		strncpy(caisse,vide,25);
		strncpy(caisse,bufc + 3 + 25,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strcpy(fc->prenom_c,caisse);

		strncpy(caisse,vide,25);
		strncpy(caisse,bufc + 3 + 50,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strcpy(fc->prenom2_c,caisse);
		fc->E_S = bufc[78];
		fc++;
	}
	fc = adrc;
	fclose(fpc);
}
/***************************** TESTE DE LA LANGUE ****************************/
test_langue(char langue)
{
	switch(langue)
	{
	case '1' : garnir_message("messagef",message_1);
			break;

	case '2' : garnir_message("messagen",message_1);
			break;

	case '3' : garnir_message("messagea",message_1);
			break;

	case '4' :  garnir_message("messagef",message_1);
			 garnir_message("messagen",message_2);
			 break;

	case '5' :  garnir_message("messagef",message_1);
			 garnir_message("messagea",message_2);
			 break;

	case '6' :  garnir_message("messagen",message_1);
			 garnir_message("messagea",message_2);
			 break;

	default  :  msg_erreur("Unknow language");
	}
}
/****************************** GARNIR LES MESSAGES **************************/
garnir_message(char *fichier,char **table)
{
	char bufm[70];

	fpm = fopen(fichier,"r");
	if(fpm == NULL)
		msg_erreur("MESSAGE FILE ERROR");
	i = 0;
	while((fgets(bufm,61,fpm)!= NULL))
	{
		j = strlen(bufm);
		if(j < 61)
			bufm[j-1] = 0x00;
		else
			bufm[61] = 0x00;
		strcpy((char *)table[i],bufm);
		i++;
	}
	fclose(fpm);
}
/****************************** GESTION DES ERREURS **************************/
msg_erreur(char *p)
{
	Lcd_Init();
	calmos2(3);
	while(1)
	{
		printf("%s \n", p);
		Lcd_Affiche(0,0,p);
		calmos2(1);
	}
}
/*****************************************************************************/
verif_terminal()
{
	char tab_verif1[16];
	char tab_verif2[16];
	unsigned char far *pointy;

	/* START CONFIDENTIAL */

	pointy = (uchar * ) 0xC0000087; /* Ancien Terminal */
	for(i = 0;i < 15;i++)
		tab_verif1[i] = *pointy++;
	tab_verif1[i] = 0x00;

	pointy = (uchar * ) 0xC00000DF; /* Nouveau Terminal */
	for(i = 0;i < 15;i++)
		tab_verif2[i] = *pointy++;
	tab_verif2[i] = 0x00;

	if((strcmp(tab_verif1,"Voting Terminal") != 0) &&
	  (strcmp(tab_verif2,"Voting Terminal") != 0))
		msg_erreur("Voting Terminal ?");

	outportb(0x308,0xf4);

/* END CONFIDENTIAL */
}
/*****************************************************************************/

