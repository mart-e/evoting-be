#include <stdio.h>
#include <stdlib.h>
#include "garnir.h"
int nbrscrutin = 0;
int nbrlist[10];
int nbrcand[120];
char Msg1[90]="",Msg2[90]="",Msg3[90]="",Msg4[90]="",Msg5[90]="";
int te = 0,ts = 0;
struct cand_t *fc = NULL, *adrc = NULL;
FILE *fpc = NULL;

/* << LAH */
extern char JITES;
extern char DIGIVOTE;
/* LAH >> */
/***********************  GARNIR TABLE CANDIDATS ***************************/
int Garnir_cand()
{
	int  iS;
	char bufc[100],caisse[100],str_nbre_max[4];
	char str_nbre_col[2+1], str_nbre_max_SUP[2+1];
	int k,i,j;

	fpc = fopen(f_name,"r");
	if (fpc == (FILE *)NULL) {
		return(11);
	}
	fgets(bufc,81,fpc);

	strncpy(Msg2,bufc+20,2);
	if(Msg2[0] == '0')
	Msg2[0] =' ';
	Msg2[2] = 0x00;
	strncpy(Msg3,bufc+22,25);
	Msg3[25] = 0x00;
	for (iS = 24; ((Msg3[iS] == ' ') && (iS >= 0)); iS--) Msg3[iS] = '\0';
	strncpy(str_nbre_max,bufc + 47,2);
	str_nbre_max[3] = 0x00;
	nbr_max = atoi(str_nbre_max);
	strncpy(str_nbre_max_SUP,bufc + 49,2);
	str_nbre_max_SUP[3] = 0x00;
	nbr_sup = atoi(str_nbre_max_SUP);
	strncpy(str_nbre_col,bufc + 51,2);
	str_nbre_col[2] = '\0';
	iRuptureColonne = nbc_col = atoi(str_nbre_col);
	strncpy(Arrond,bufc + 51,3);
	Arrond[3] = '\0';

	strncpy(LISTE, bufc +62, 3);
	LISTE[3]  = '\0';

	bufnom[0]  = bufc[59];
	bufnom[1]  = bufc[63];
	bufnom[2]  = bufc[64];
	bufnom[3]  = bufc[65];
	bufnom[4]  = bufc[66];
	bufnom[5]  = bufc[67];
	bufnom[6]  = bufc[68];
	bufnom[7]  = bufc[69];
	bufnom[8]  = '.';
	bufnom[9]  = bufc[60];
	bufnom[10] = bufc[61];
	if(JITES == 1)
		bufnom[11] = 'c';
	else
		bufnom[11] = 'd';
	bufnom[12] = 0x00;

	fgets(bufc,81,fpc);
	strncpy(Msg5,bufc,80);
	for(i = 75;((Msg5[i] == ' ') || (Msg5[i] == 0x00)) && (i > 0);i--)
		Msg5[i+1] = 0x00;

	fgets(bufc,81,fpc);

	strncpy(Msg1,bufc,60);
	for (iS = 59; ((Msg1[iS] == ' ') && (iS >= 0)); iS--) Msg1[iS] = '\0';

	fgets(bufc,81,fpc);

	strncpy(Msg4,bufc,35);
	for(i = 34;((Msg4[i] == ' ') || (Msg4[i] == 0x00)) && (i > 0);i--)
		Msg4[i+1] = 0x00;

	adrc = fc = (struct cand_t *) malloc(sizeof(struct cand_t) * 100);
	while(fgets(bufc,81,fpc) != NULL)
	{
		if (nom_s == ' ')  nom_s = bufc[0];

		memset(caisse,' ',25);
		strncpy(caisse,bufc + 3,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strncpy(fc->nom_c,caisse,25);
		fc->nom_c[25] = 0x00;
		memset(caisse,' ',25);
		strncpy(caisse,bufc + 3 + 25,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strncpy(fc->prenom_c,caisse,25);
		fc->prenom_c[25] = 0x00;

		memset(caisse,' ',25);
		strncpy(caisse,bufc + 3 + 50,25);
		j = 24;
		while(caisse[j] == ' ' && j >= 0)
			caisse[j--] = 0x00;
		strncpy(fc->prenom2_c,caisse,25);
		fc->prenom2_c[25] = 0x00;

		fc->E_S = bufc[78];
		if (bufc[78] == 'E')
			neffectif += 1;
		else
			nsuppleant+= 1;
		fc++;
	}
	te = neffectif;
	ts = nsuppleant;
	fc = adrc;
	if (fpc != (FILE *)NULL) { fclose(fpc); fpc = (FILE *)NULL; }

	return(0);
}
/*****************************************************************************/
