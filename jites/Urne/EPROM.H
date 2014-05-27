/*******************************************************************/
/* DEBUT DU MODULE DE GESTION DU FLASH EPROM -- HEADER             */
/*******************************************************************/
 boolean Eprom_Init(void);
 boolean Eprom_Read(unsigned char *Valeur, long Emplacement);
 boolean Eprom_Writed(long Emplacement);
 boolean Eprom_Flash(void);
 boolean Ram_Writed(long Emplacement);
 boolean Ram_Read(unsigned char *Valeur, long Emplacement);
 boolean Ram_Write(unsigned char Valeur, long Emplacement);
 boolean Dsk_Read(unsigned char *Valeur, long Emplacement);
 boolean Dsk_Write(unsigned char Valeur, long Emplacement);
 boolean Eprom_Write(unsigned char Valeur, long Emplacement);
 extern void Light_LED(char LED);

 int     Ram_Eprom_Load(void);
 int     Ram_Disk_Load(void);
 int     Disk_Ram_Load(void);
 int     Eprom_Ram_Load(void);
 int     Witch_Support(void);
/* << 10.02 */
 boolean Check_EPROM(void);
/* 10.02 >> */
 void Down_Load_Ind(void);
 /* << 10.02h */
 int Compare_Ram_Eprom(void);
 int Compare_Ram_Disk(void);
 extern void Erreur(char *msg);
 /* 10.02h >> */

 extern int Getch(char COLOR);
 extern int Test_Ep(void);
 /* << 10.03g */
 extern int Disk_Num(void);
 extern int Check_NumDisk(int iNum);
 /* 10.03g >> */
extern void Encode_E_Var_String(long Position, char *Valeur, int Longueur);

/*******************************************************************/
/*  FIN DU MODULE DE GESTION DU FLASH EPROM -- HEADER              */
/*******************************************************************/
