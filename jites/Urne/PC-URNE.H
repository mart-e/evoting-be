/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE PC-URNE.C  --  HEADER                          */
/*******************************************************************/
/* START CONFIDENTIAL */
struct scrutin_t
{
  int  Nombre_Listes;
  int Votes_Bl_Exp;
  int Votes_Bl_Non_Exp;
  struct liste_t *ptlist;
}scrutin[5];

struct liste_t
{
    int Nombre_Effectifs;
    int Nombre_Suppleants;
    int Votes_Candidats[90];
    int Votes_Effectifs;
    int Votes_Suppleants;
    int Votes_Tete_Liste;
    int Votes_Liste;
};
/* END CONFIDENTIAL */

/***********************************************/

void Initialise_Date(char *Buffer);
void Initialise_Variables(char *Buffer);
int Lit_Fichier_PCC(char *Nom_Fichier);
int Lit_Fichier_ARR(char *Nom_Fichier);
int Initialise_Table_PCC(void);
void Additione_Vote(int Scrutin, int Liste, int Eff, int Sup);
void Additione_Vote_Multiple(int Scrutin, int Liste, int Bl);
int Ecrit_Table_PCC(char *Nom_Fichier);
void Transfert_Table_PCC(char *Nom_Fichier);
int Get_Nombre_Votes(void);
int Choix_Langue(int choix);
int Verifie_E_Vote(void);
/* << 10.02b */
void Init_Listes(void);
/* 1.02b >> */

extern int Saisie_Langue(void);
extern void Battery_Off(void);
extern int Kbhit(char COLOR,int loop);
extern int Getch(char COLOR);
/* << 10.02f */
extern void Light_LED(char LED);
/* 10.02f >> */
/* << 10.03f*/
extern int Getchar(char color, char *pzMessage,char *pzMsg, int iLine, int iCpt);
/* 10.03f >> */
/*******************************************************************/
/*  FIN DU MODULE PC-URNE.C  --  HEADER                            */
/*******************************************************************/
