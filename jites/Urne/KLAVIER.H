/*******************************************************************/
/*  DEBUT DU MODULE Klavier.C  --  HEADER                          */
/*******************************************************************/
int Kbhit(char color ,int loop);
int Getch(char color);
int Trait_Tc(char tc,char Ordre,int Color);
/* << 10.03d */
  /* << 10.03f */
/*void Ctrl_Relais(char *pzMessage, int iLine, int iCpt);
int Getchar(char color, char *pzMessage, int iLine, int iCpt);*/
void Ctrl_Relais(char *pzMessage,char *pzMsg, int iLine, int iCpt);
int Getchar(char color, char *pzMessage,char *pzMsg, int iLine, int iCpt);
  /* 10.03f >> */
extern void Lcd_Affiche(int Ligne, int Colonne, char *Chaine);
/* 10.03d >> */
/*******************************************************************/
