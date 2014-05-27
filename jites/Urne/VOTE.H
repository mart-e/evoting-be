
/*******************************************************************/
/*******************************************************************/
/*  DEBUT DU MODULE VOTE.C  --  HEADER                             */
/*******************************************************************/
/***********************************************/

void Recupere_Jeton(char *Carte, char *Jeton);
void Encode_Jeton(char *Carte, char *Jeton);
void Recupere_Vote(char *Carte);
void Encode_Vote(char *Carte, int Scrutin,
			     int Liste, int Effectif, int Suppleant);
void Valide_Carte(char *Carte, int *Nombre_Lecture);
void Initialise_Carte(char *Carte);
Recupere_stat(char *Carte);
Recupere_stat_Val(char *Carte);
void Recupere_Crc(char *Carte,int Place);
void Encode_Crc(char *Carte);

/*******************************************************************/
/*  FIN DU MODULE VOTE.C  --  HEADER                               */
/*******************************************************************/
