/*******************************************************************/
/*  DEBUT DU MODULE URNBE.C  --  HEADER                            */
/*******************************************************************/

int Crc_Rijndael(char *mode,char *type,char *src,char *dest);
int Remplit(void);
int Rfloppy(char *msg);
int Rfile(char *fichier);
int Wmac(void);
int CalculMac(void);
int Encrypte(void);
void Calcul_Crc(char *Carte, int pos);
int MoveTbl(void);
extern int Getch(char Color);

extern void Light_LED(char LED);
void Erreur(char *msg);
int Initialise_Rijndael(void);
void Initialise_RijndaelMini(void);
/* << 10.03b */
extern void Read_Ivbit(void);
/* 10.03b >> */

/*******************************************************************/
/*  FIN DU MODULE URNEBE.C  --  HEADER                             */
/*******************************************************************/
