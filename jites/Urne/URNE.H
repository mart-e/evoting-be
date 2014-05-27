/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE PRINCIPAL URNE.C  --  HEADER                   */
/*******************************************************************/
boolean Init_Systeme(void);
boolean Init_Version();

int Verifie_Urne(char *Clef);
int Verifie_Maitre(char *Clef);
int Test_Ep(void);
int Test_Eprom(void);
int Saisie_Langue(void);
int Eprom_Init(void);
int Test_Urne(void);
int Test_Bk(void);
int Test_Bk_Aut(void);
int Test_Contenu_Urne(void);
int Test_Contenu_Urne2(void);
/* << 10.01 */
/*int Fermeture_Urne(char *NomFichier);*/
int Fermeture_Urne(void);
/* 10.01 >> */
int Demande_Action(void);
int Affiche_Result(void);
int Disq_BK;
void Calcul_Jeton(char *Jeton_Etendu, char *Jeton);
void Calcul_Clef_Mac(char *Jeton_Etendu, int *Clef_Id_Mac);
/* << 10.02e */
/*void Demande_Identite(char *Clef);*/
boolean Demande_Identite(char *pzClef, boolean bRequire);
boolean Check_Vote_CRC(void);
/* 10.02e */
void _harderr(void(fptr)());
void Ouverture_Urne(void);
void Battery_Ok(void);
void Battery_On(void);
void Battery_Off(void);
/* << 10.03d*/
void Check_Relais(void);/*char cKeyboard*/
/* 10.03d >> */
/* << 10.02f */
/*void Light_LED(char);*/
/* 10.02f >> */
void Ecriture_force(void);
int Demande_Mac(void);
/* << 10.01 */
int Disk_Num(void);
void Write_Data_Disk(int iNum);
int Language_ID(void);
void Load_Message(int iNum);
void Read_File(char *pzFileName);
int Select_Language_ID(void);
/* 10.01 >> */
/* << 10.03g*/
int Check_NumDisk(int iNum);
/* 10.03g >> */
/* << 10.04 */
int Check_Key(char *key);
/* 10.04 >> */
/* START CONFIDENTIAL */

extern int Crc_Rijndael(char *Type,char *Mode,char *src,char *dest);
extern int Initialise_Rijndael(void);
int Verifie_Password(void);

extern int Kbhit(char COLOR,int loop);
extern int Getch(char Color);
extern int Trait_Tc(int tc,char Ordre,int Color);
extern int Lecteur_Put_Tempo(void);
extern int Choix_Langue(int choix);
extern int Crc(char *type,char *mode,char *src,char *dest, char *passe);
extern void Initialise_RijndaelMini(void);

/* << 10.03d*/
/* << 10.03f*/
/*extern int Getchar(char color, char *pzMessage, int iLine, int iCpt);*/
extern int Getchar(char color, char *pzMessage,char *pzMsg, int iLine, int iCpt);
/* 10.03f >> */
/* 10.03d >> */

/* END CONFIDENTIAL */

/*******************************************************************/
/*  FIN DU MODULE PRINCIPAL URNE.C  --  HEADER                     */
/*******************************************************************/
