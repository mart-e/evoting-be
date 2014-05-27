int Do_command(int Command, COM *Port);
int Do_Nothing(COM *Port);
int Fin(COM *Port);

int Port_receive_completed(COM *Port);

int Lecteur_Feed(COM *Port);
int Lecteur_Read(COM *Port);
int Lecteur_Set_Led(COM *Port,int Color);
int Lecteur_Write(COM *Port);
int Lecteur_Eject(COM *Port);
int Lecteur_Get_Position(COM *Port);
int Lecteur_Vide(COM *Port);
int Lecteur_Deblock(COM *Port);

int Verify_Lrc(char *Buffer, char Lrc_Lu);
int Lecteur_Status(COM *Port);
int Lecteur_Status_Feed(COM *Port);
int Lecteur_Status_Write(COM *Port);
int Lecteur_Status_Position(COM *Port);
int Lecteur_Status_Eject(COM *Port);
int Lecteur_Status_Deblock(COM *Port);

void Valide(void);
int Lecteur_Valide(COM *Port);

int Determine_Position(void);
int Enregistre(char *Carte);
int Lecteur_Enregistre(COM *Port);

int Lecteur_Valide_Vote(COM *Port);
int Lecteur_1_Init(void);
int Lecteur_2_Init(void);

void Boucle_Lecteur(void);
int Command_Recom(void);
int Test_Retour(char *Command_Rx);
void Secu_Stat(void);
/*¨<< 10.02f */
void Light_LED(char LED);
int Check_Election(char *pzJeton_B, char *pzJeton_C);
/* 10.02f >> */

extern int Verifie_E_Vote(void);
extern int Tcr_Snd(char *Command);
extern int Tcr_Rx(char *Buffer);
extern int Fin_Rx(char *Buffer);
extern int TcrInit(void);
extern int Kbhit(char COLOR, int loop);
extern int Getch(char COLOR);
extern void McrBg(void);
extern void Battery_On(void);
extern void Battery_Off(void);
/* << 10.02e */
/*extern void Demande_Identite(char *Clef);*/
extern boolean Demande_Identite(char *pzClef, boolean bRequire);
/* 10.02e */
extern int Verifie_Urne(char *Clef);
extern void Calcul_Crc(char *Carte, int pos);
/* << 10.03b */
extern void Maj_IvBit(char);
/* 10.03b >> */
/*******************************************************************/
/*******************************************************************/
