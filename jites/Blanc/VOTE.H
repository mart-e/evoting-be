/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE VOTE.C  --  HEADER                             */
/*******************************************************************/
/* START CONFIDENTIAL */  

#define _CARTE_MAX_BYTE        (79)
#define _CARTE_DATA_PLACE      (1)
#define _CARTE_DATA_MAX_BYTE   (75)

#define _C_LISTE_MAX_BYTE     (2)
#define _C_EFFECTIF_MAX_BYTE  (2)
#define _C_SUPPLEANT_MAX_BYTE (2)
#define _C_COMMUNE_MAX_BYTE   (23)


#define _C_JETON_MAX_BYTE     (5)
#define _C_TEST_MAX_BYTE (1)
#define _C_PASSAGE_MAX_BYTE   (1)
#define _C_CRC_MAX_BYTE       (4)
#define _C_VOTE_MAX_BYTE      (6)
#define _C_DATA_MAX_BYTE      (75)

#define  _C_DATA_PLACE        (0)
#define  _C_JETON_PLACE       (_C_DATA_PLACE + 0)
#define  _C_PASSAGE_PLACE     (_C_DATA_PLACE + 5)
#define  _C_CRC_PLACE         (_C_DATA_PLACE + 6)
#define  _C_TEST_PLACE   (_C_DATA_PLACE + 10)
#define  _C_VOTE_PLACE        (_C_DATA_PLACE + 11)

/* END CONFIDENTIAL */  

/***********************************************/

void Recupere_Jeton(char *Carte, char *Jeton);
void Encode_Jeton(char *Carte, char *Jeton);
void Recupere_Vote(char *Carte, int Scrutin,
			  int *Liste, int *Effectif, int *Suppleant);
void Encode_Vote(char *Carte,int Liste, int Effectif, int Suppleant);
void Recupere_Passage(char *Carte, int *Passage);
void Encode_Passage(char *Carte);
void Recupere_TestCarte(char *Carte, int *Test);
void Encode_TestCarte(char *Carte, int Test);
void Valide_Carte(char *Carte, int *Nombre_Lecture);
void Initialise_Carte(char *Carte);
void Recupere_Crc(char *Carte);
void Encode_Crc(char *Carte);
void Calcul_Crc(char *Carte, int pos);
/*******************************************************************/
/*  FIN DU MODULE VOTE.C  --  HEADER                               */
/*******************************************************************/
/*******************************************************************/
