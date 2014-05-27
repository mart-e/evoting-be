#define  true  1
#define  false 0
#define  _Version "Version urne 12.01\x0"
/* << 10.02e */

/* << AP 16/04/2010 
__#define MINI_PWD "2701"
__#define MINI_POS "1467" 
*/

#define MINI_PWD "2703"
#define MINI_POS "1368"

/* >> AP 16/04/2010 */

/* 10.02e >> */
typedef int boolean;
/*******************************  EPROM *************************************/

#define _EPROM_TAILLE  127
#define _LCD_REG_BASE      (0x378)
#define _LCD_REG_DONNEE    (_LCD_REG_BASE + 0x0000) /* Adresse du port */
#define _LCD_REG_CONTROLE  (_LCD_REG_BASE + 0x0002) /* Adresse du port */
                                                    /* de controle     */
#define _LCD_TEMPS_ATTENTE (0x30)
/* START CONFIDENTIAL */

/* << 10.07 */
/*#define _E_NB_MAX_VOTE   (2000)*/
#define _E_NB_MAX_VOTE   (1800)
/* 10.07 >> */
#define _E_FERMETURE_MAX_BYTE     (1)
#define _E_OUVERTURE_MAX_BYTE     (1)
#define _E_TABLE_MAX_BYTE         ( 55 *_E_NB_MAX_VOTE)
#define _E_JETON_ETENDU_PLACE (0)
#define _E_FERMETURE_PLACE    (20)
#define _E_OUVERTURE_PLACE    (21)
#define _E_JITES              (400)
#define _E_PAR_PLACE          (500)
#define _E_TABLE_PLACE        (1500)
#define _E_TABLE_VAL_PLACE    (3000)
#define _E_RIEN               (100)

#define _E_VARIABLES   (100)
#define _E_JETON_ETENDU (_E_VARIABLES + _E_JETON_ETENDU_PLACE)
#define _E_FERMETURE  (_E_VARIABLES + _E_FERMETURE_PLACE)
#define _E_OUVERTURE  (_E_VARIABLES + _E_OUVERTURE_PLACE)
#define _E_JETON      (_E_VARIABLES + _E_JETON_PLACE)
#define _E_TABLE_VAL  (_E_PAR_PLACE + _E_TABLE_PLACE)
#define _E_TABLE      (_E_PAR_PLACE + _E_TABLE_PLACE + _E_TABLE_VAL_PLACE +_E_RIEN)


/**************************  MESSAGES D'ERREURS ****************************/

#define _PAS_ERREUR            (0)
#define _W_CONDENSAT           (1)
#define _W_JETON               (2)
#define _W_DEJA_LUE            (3)
#define _W_CARTE_OUBLIEE       (4)
#define _W_MEMOIRE             (5)
#define _W_FICHIER_PAS_TROUVE  (6)
#define _W_CREATION_FICHIER    (7)
#define _W_ECRITURE            (8)
#define _W_LECTURE             (9)
#define _W_MAUVAIS_MOT_PASSE   (10)
#define _W_FLASH_EPROM         (11)

/******************** CHARGEMENT PARAMETRES URNE PC-URNE **********************/

#define _LECT_NOMBRE(valeur, strtmp, buffer, inc, nbc)        \
       for (inc = 0; inc < nbc; strtmp[inc++] = *buffer++);   \
       strtmp[inc] = '\0';                                    \
       valeur = atoi(strtmp);

#define _LECT_CHAINE(valeur, buffer, inc, nbc)                \
       for (inc = 0; inc < nbc; valeur[inc++] = *buffer++);   \
       valeur[inc] = '\0';

#define _CARTE_MAX_BYTE         75
#define _CARTE_DATA_MAX_BYTE    75
#define _JETON_ETENDU_MAX_BYTE  15
#define _JETON_MAX_BYTE          5
#define _CLEF_MAX_BYTE           10

#define _NB_MAX_SCRUTIN  (5)
#define _NB_MAX_LISTE    (51 * _NB_MAX_SCRUTIN)
#define _NB_MAX_CANDIDAT (100)

/************************* LAMPES VALIDEUSE LECTEUR *************************/

#define _OFF_RED        0x08
#define _ON_RED         0x28
#define _OFF_GREEN      0x10
#define _ON_GREEN       0x30
#define _OFF_RED_GREEN  0x18
#define _ON_RED_GREEN   0x38
#define _OFF_NO_LED     0x00
#define _ON_NO_LED      0x20
//#define _OFF_NO_LED     0x00
#define _LED_PORT       0x3E0

/****************************** ETAT DES BATTERY'S **************************/

extern int BATTERY;

/********************** POSITION DU VOTE SUR CARTE  VOTE.C *******************/

#define _C_JETON_MAX_BYTE     (_JETON_MAX_BYTE)
#define _C_PASSAGE_MAX_BYTE   (1)
#define _C_CRC_MAX_BYTE       (4)

#define _C_CLEF_MAX_BYTE      (10)
#define _C_TEST_MAX_BYTE      (1)
#define _C_DATA_MAX_BYTE      (75)
#define  _C_DATA_PLACE        (0)
#define  _C_JETON_PLACE       (_C_DATA_PLACE + 0)
#define  _C_PASSAGE_PLACE     (_C_DATA_PLACE + 5)
#define  _C_CRC_PLACE         (_C_DATA_PLACE + 6)
#define  _C_TEST_PLACE        (_C_DATA_PLACE + 10)
#define  _C_VOTE_PLACE        (_C_DATA_PLACE + 11)


/****************************************************************************
                               VARIABLES EXTERNES
*****************************************************************************/
extern char SUPPORT; /* E = Eprom / R = Ram */
extern int  FEram, FEdsk;
extern int  Total_Erreur_Eprom;
extern char Chemin_R[25], Chemin_D[25];
extern int  C_VOTE_MAX_BYTE;
extern char FERMETURE;
extern char OUVERTURE;
extern char NC,Type_Multi;
extern int  NOMBRE_VAL,NOMBRE_NATIO_VAL,NOMBRE_EURO_VAL,NOMBRE_STR_VAL;
/* << 10.02b */
extern int  /*NOMBRE_VOTES,NOMBRE_NATIO,NOMBRE_EURO,NOMBRE_STR,*/Additionne;
/* 10.02b >> */
extern int  GLOBAL_NATIO,GLOBAL_EURO,GLOBAL_STR;
extern int  DEMANDE_FERMETURE;
extern char JETON_ETENDU[_JETON_ETENDU_MAX_BYTE+1];
extern char JETON[_JETON_MAX_BYTE+1];
extern char CLEF_MAITRE[_CLEF_MAX_BYTE+1];
extern char CLEF_URNE[_CLEF_MAX_BYTE+1];
extern char CLEF_URNE_BIDON[_CLEF_MAX_BYTE+1];
extern char CLEF_SUPER_USER[_CLEF_MAX_BYTE+1];
extern char Statut_Vote, Statut_Val;
extern int  USER;
extern int  POSITION_VOTE;
extern char COLOR;
extern int NOMBRE_SCRUTIN;
extern int SCRUTIN[_NB_MAX_SCRUTIN];
extern int PC_LISTE_BLANC[_NB_MAX_SCRUTIN];
extern int *TABLE_PCC;
extern int FILE_EPROM;      /* Fichier servant a la sauvegarde */
extern int SAUVE_TO_EPROM;  /* Dtermine si sauvegarde directe */

extern int Table[2000], Table_Val[3000];

extern int Cmpt_Gen,Cmpt_Gen_Val;
extern int rang,valrang;
extern char Crc_Calcul[5],Crc_Recup[5];


/* << 10.01 */
/*extern char Message_point[118][21];*/
extern char Message_Point[130][21];
/* 10.01 >>  */
extern int      TYPE_SCRUTIN[5];
extern int      BK_PC;
extern int      Disq_BK;
/* << 10.04 */
/*extern char     Strtmp[75];*/
extern char     szVoteContents[76];
/* 10.04 >> */
extern char     *Buffer_Par;
extern char _Code[];
extern char MULTI_ELEC;
extern unsigned int EPROM_SEG;
extern unsigned int timing;
extern int PORT_1_NEXT_COMMAND;
char GCOLOR;

char fullPassword[11];
unsigned char CMinipassword[12];
extern char Minicodage[5];

/****************************************************************************/
/* END CONFIDENTIAL */




