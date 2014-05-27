/******************************************************************/
/*  DEBUT DU MODULE VOTE.C                                         */
/*******************************************************************/

/* START CONFIDENTIAL */  

#define boolean int
#define true    1
#define false   0
#define then    /**/

#include "vote.h"
char _Code[] = "0EC3ZN678LAB2DFRH1IJK9M5OPQGSTUVWXY4";

/***********************************************/

void Recupere_Jeton(char *Carte, char *Jeton)
{
  int i;

  for (i = 0; i < _C_JETON_MAX_BYTE;
			  Jeton[i] = Carte[_C_JETON_PLACE + i], i++);
  Jeton[_C_JETON_MAX_BYTE] = '\0';
}
 
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
	 Cbuv1[2]  = '\0';
	 Cbuv2[1]  = '\0';
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
/************************************************/
void Encode_Vote(char *Carte,int Liste, int Effectif, int Suppleant)
{
	char strtmp[10];
	int  i;

	sprintf(strtmp,"%2d",Liste);
	for (i = 0; i < _C_LISTE_MAX_BYTE; Carte[debut++] = strtmp[i], i++);
	sprintf(strtmp,"%2d", Effectif);
	for (i = 0; i < _C_EFFECTIF_MAX_BYTE; Carte[debut++] = strtmp[i], i++);
	sprintf(strtmp, "%2d", Suppleant);
	for (i = 0; i < _C_SUPPLEANT_MAX_BYTE; Carte[debut++] = strtmp[i], i++);
} 

/************************************************/
void Encode_Vote_Multiple(char *Carte, int Scrutin, int Liste)
{
	char strtmp[10];
	char strurn[26],c; 
	int i,j,z;

	sprintf(strtmp,"%2d",Liste); /* la liste 0 est le vote blanc */
	for (i = 0; i < _C_LISTE_MAX_BYTE;
				  Carte[debut++] = strtmp[i], i++);

	for(z = 0;z < 90;z++) ch[z].ch0 = 0; /* remise … z‚ro */
	for(z=0;z < 90;z++)
		{
	 if(memoc[Scrutin][Liste - 1][z] == 1)
		 ch[z].ch0 = 1;
		}
	
	for(i = 0, j = 0; (i < cfgS[scrutin[Scrutin].TYPE_SCRUTIN].nmxV);)
	{   
	 c = 0x00;
	 c = (ch[i++].ch0) * 8;
	 c+= (ch[i++].ch0) * 4;
	 c+= (ch[i++].ch0) * 2;
	 c+= (ch[i++].ch0) * 1;
	 z = c;
	 strurn[j++] = codage[z];
	 }     
	 strurn[j] = 0x00;
	 
	 for (i = 0; i < cfgS[scrutin[Scrutin].TYPE_SCRUTIN].nmxB;
				  Carte[debut++] = strurn[i], i++);
	 Carte[debut] = '\0';
} 
/************************************************/
void Encode_Passage(char *Carte)
{
 Carte[_C_PASSAGE_PLACE] = '1';
} 

/***********************************************/
void Encode_Crc(char *Carte)
{
  int i;
  for (i = 0; i < _C_CRC_MAX_BYTE; 
                         Carte[_C_CRC_PLACE + i] = Crc_Calcul[i],i++);
}
/***********************************************/
void Recupere_Crc(char *Carte)
{
  int i;
  for (i = 0; i < _C_CRC_MAX_BYTE;
                         Crc_Recup[i] = Carte[_C_CRC_PLACE + i],i++);
  Crc_Recup[4] = 0x00;
} 
/***********************************************/
void Recupere_Passage(char *Carte, int *Passage)
{
  int i;

  if(Carte[_C_PASSAGE_PLACE] == '0')
          *Passage = 0;
  else
          *Passage = 1;
} 
/***********************************************/
/* END CONFIDENTIAL */  
