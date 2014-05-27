/*******************************************************************/
/*******************************************************************/
/* DEBUT DU MODULE ERREUR.C -- HEADER                              */
/*******************************************************************/

void Boucle_Erreur(void);
void Change_Disque(char *Message);
void Wait_Time(int Sec);
extern void Battery_Off(void);
int Kbhit(char COLOR,int loop);
int Getch(char COLOR);
int error_win(int numerror);
/* START CONFIDENTIAL */
int handler(unsigned errval,unsigned di,unsigned far *devhdr);
void _hardresume(int retval);
void _hardretn(int result);
/* END CONFIDENTIAL */

/*******************************************************************/
/* FIN DU MODULE ERREUR.C -- HEADER                                */
/*******************************************************************/
