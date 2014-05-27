/*******************************************************************/
/*  DEBUT DU MODULE VARIABLE.C  --  HEADER                         */
/*******************************************************************/

void Encode_E_Variable(long Position, char Valeur);
void Recupere_E_Variable(long Position, char *Valeur);
void Encode_E_Var_String(long Position, char *Valeur, int Longueur);
void Recupere_E_Var_String(long Position, char *Valeur, int Longueur);
void Recupere_E_Vote(int Position, char *Valeur, int Longueur);
void Encode_E_Vote(int Position, char *Valeur, int Longueur);
void Valide_E_Val(int Position);
void Recupere_Nb_Cartes(void);
void Recupere_Nb_Validations(void);

boolean Est_Valide_E_Vote(int Position);
boolean Est_Libre_E_Position(int Position);
boolean Est_Libre_E_Position_Val(int Position);
boolean Est_Valide_E_Val(int Position);

/* << 10.02b */
boolean Alloc_Mem(void);
void Free_Mem(void);
void Init_Vote_Counters(void);
void Check_Vote_Counters(void);
/* 10.02b >> */

/* << 10.03c */
void Encrypt_Decrypt(char *pzInputData, char *pzPassword, unsigned int iSize);
void Generate_Password(char *pzPassword, long Position, boolean bIndic);
/* 10.03c >> */

/* << 10.04 */
int Encrypt_Vote(char *pzInputData, char *pzPassword, unsigned int iSize);
/* << 10.04 */


extern void Change_Disque(char *Message);
extern int Getch(char COLOR);

/* << 10.03g */
 extern int Disk_Num(void);
 extern int Check_NumDisk(int iNum);
 /* 10.03g >> */
/*******************************************************************/
/*  FIN DU MODULE VARIABLE.C  --  HEADER                           */
/*******************************************************************/
