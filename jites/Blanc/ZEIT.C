#include <conio.h>
#include <stdio.h>
#include <string.h>
#define  false     0
#define  true      1
#define  time_out  2
/***************************************************************************/
/* START CONFIDENTIAL */
/***************************************************************************/

uchar RxBuffer[100],TxBuffer[100],Buff_Commande[100];
uchar cmd[100];
int compt;

int Lecteur_Init()
{
	return(McrInit(0x300,5,RxBuffer,sizeof(RxBuffer),TxBuffer,sizeof(TxBuffer)));
}
/***************************************************************************/

int Read_Commande()
{
	strcpy(Buff_Commande,_READ);
	strcat(Buff_Commande,_CR);
	status = Tcr(Buff_Commande,1);
	if(status == time_out)
		return(3);
	if(Lect_Buffer[0] == 'x')
		status = Test_Retour(Lect_Buffer[2]);
	else
		status = 0;
	Lect_Buffer[74] = 0x00;

	return(status);
}

/***************************************************************************/
/***************************************************************************/

int Write_Commande()
{
	strcpy(Buff_Commande,_WRITE);
	strcat(Buff_Commande,carte);
	strcat(Buff_Commande,_CR);
	status = Tcr(Buff_Commande,1);
	if(status == time_out)
		return(3);
	status = Test_Retour(Lect_Buffer[2]);
	return(status);
}

/***************************************************************************/
/***************************************************************************/

int Eject_Commande()
{
	strcpy(Buff_Commande,_EJECT_FRONT);
	strcat(Buff_Commande,_CR);
	status = Tcr(Buff_Commande,1);
	if (status == time_out)
		return(4);
	status = Test_Retour(Lect_Buffer[2]);
	return(status);
}

/* << el2007 */
/***************************************************************************/
/***************************************************************************/

int Ejectons()
{
	Eject_Commande();
	if(status == 4)
		Eject_Commande();

	return(status);
}
/* el2007 >> */

/***************************************************************************/
int Status_Commande()
{
	strcpy(Buff_Commande,_STATUS);
	strcat(Buff_Commande,_CR);
	status = Tcr(Buff_Commande,1);
	if(status == time_out)
		return(4);
	status = strcmp(Lect_Buffer,"x80");
	return(status);
}
/***************************************************************************/
int Tempo()
{
	strcpy(Buff_Commande,"t2");
	strcat(Buff_Commande,_CR);
	status = Tcr(Buff_Commande,1);
	return(status);
}
/***************************************************************************/
int Wait_Card()
{
	if((c = McrRx()) != 'a')
		return(true);
	else
		return(false);
}
/***************************************************************************/
/***************************************************************************/

int Tcr(char *Buff_Commande,int etat)
{
	unsigned long far *timer = ( unsigned long * )0x46C;
	int tmp = 3;
	char ok;

	McrBg();
	while((c = McrRx()) != -1);
	if(etat)
	{
		while(*Buff_Commande)
		{
			if(McrTx(*Buff_Commande++) == -1)
				return(time_out);
		}
	}
	status = McrCheckState();
	if(status != 0)
		return(time_out);

	i = 0;
	McrBg();
	temptimes = *timer; /* Valeur du temps */
	ok = '1';
	while((*timer - temptimes <= (unsigned long) tmp * 19) && ok != 0x00 )
	{
		McrBg();
		while((( c = McrRx()) != -1) && (i < 75))
		{
			ok = c = (c == 0x0D) ? 0x00: c;
			Lect_Buffer[i++] = c;
		}
	}
	Lect_Buffer[i] = 0x00;
	return(0);
}
/***************************************************************************/
/***************************************************************************/
/*                   FIN DU MODULE LECTEUR.C                               */
/***************************************************************************/
int Test_Retour(char etat)
{
	switch(etat)
	{
		case '0' :
			status = 0;  /*     ACK                   */
			break;
		case '1' :
			status = 1;  /*     Unknown Commande      */
			break;
		case '2' :
			status = 2;  /*     Reading Error         */
			break;
		case '3' :
			status = 3;  /*     Writing Error         */
			break;
		case '5' :
			status = 5;  /*     Str too long          */
			break;
		case '6' :
		case '7' :
			status = 6;  /*     Card Rejected         */
			break;
		case '8' :
			status = 8;  /*     No Card      ????     */
			break;
		case '9' :
			status = 9;  /*     Transmission Error    */
			break;
		case 'a' :
			status = 10; /*     Abnormal Card Detected*/
			break;
		default  :
			status = 4;
			break;
	}

	return(status);
}
/****************************************************************************/
/****************************************************************************/
/* END CONFIDENTIAL */
