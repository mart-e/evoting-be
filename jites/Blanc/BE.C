#include "fullSpec.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <dir.h>
/* << el2007 */
#define NIVEAU "4"
#define BE     "be.4"
/* el2007 >> */


#define PlainDataLen   14400
#define CipherDataLen  14400

#include "alg-fst.c"
#include "api-fst.c"
#include "encdec.c"
#include "enckeys.c"
#include "encmac.c"
#include "hashcomp.c"
#include "maccomp.c"
#include "psswd.c"
/* << el2007 */
#include "sortbe.c"
/* el2007 >> */

char fullPassword[fullPasswordLength + 1];
int hashResultLen = sizeof (hashResult);
keyInstance encKeyInst;       /* state of the encryption engine */
cipherInstance encCipherInst; /* specifies the encryption mode */
keyInstance decKeyInst;       /* state of the decryption engine */
cipherInstance decCipherInst; /* specifies the decryption mode */
/* << el2007 */
int giProgressionMax = 0;
/* el2007 >> */

Crc_Rijndael()
{
	strcpy(mode,"R");
	/* << el2007 */
	/*strcpy(niveau,"1");*/
	strcpy(niveau,NIVEAU);
	/* el2007 >> */
	ret = Remplit();
	if(!ret)
		return(0);
	/* << el2007 */
	SortFiles();
	/* el2007 >> */
	makeKeyBinary (&decKeyInst, DIR_DECRYPT, decryptedEncKey);
	cipherInitBinary (&decCipherInst, iv);

	ret = CalculMac();
	if(!ret)
		return(0);

	strcpy(source,"A:");
	/* << el2007 */
	/*  strcat(source,"be");*/
	strcat(source,BE);
	/* el2007 >> */
	ret = Rfile(source);
	if(!ret)
		return(0);

	for (i = 0; i < macResultLen;i++)
		if(macResult[i] != wrkspc[i+16])
			return(0);
		else
			return(1);
}
/****************************************************************************/
Rfloppy(char *msg)
{
	int fd;

	strcpy(floppy,"a:");
	strcat(floppy,"Floppy.be");

	if((fd = open( floppy, O_BINARY | O_RDWR)) == -1)
		return(0);
	read(fd,msg,80);
	close(fd);
	return(1);
}
/****************************************************************************/
Rfile(char *fichier)
{
	int fd;
	char Bffer[100];
	int handel_Read;

	if((fd = open( fichier, O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
		return(0);

	handel_Read = read(fd,Bffer,64);
	if(handel_Read == -1)
		return(0);
	close(fd);
	Bffer[handel_Read] = 0x00;

	for ( i = 0 ; i <  handel_Read ; i++)
	{
		if(Bffer[i] < 58)
			Bffer[i] -= 48;
		else
			Bffer[i] -= 55;
	}

	for ( i = 0, j = 0; j < handel_Read / 2; j++)
	{
		wrkspc[j]   = Bffer[i++] << 4;
		wrkspc[j]   = Bffer[i++] | wrkspc[j];
	}
	wrkspc[j] = 0x00;
	return(1);
}
/*****************************************************************************/
/******************************************************************************/
/*       C O N T R O L E   D U   C R C    D E S   F I C H I E R S            */
/*             P R E P A R A T I O N    D U    T A B L E A U                 */
/* << el2007 */
#define ONLY_BE_2 \
( \
	!strnicmp(fileinfo.ff_name,"BE", 2) || \
	!stricmp(fileinfo.ff_name,"URNE.EXE") || \
	!strnicmp(fileinfo.ff_name,"MSG_", 4) || \
	!strnicmp(fileinfo.ff_name,"URNE", 4) || \
	!stricmp(fileinfo.ff_name,"RUNURN.BAT") || \
	!stricmp(fileinfo.ff_name,"TABLE.URN") || \
	strstr(fileinfo.ff_name,"FE_DSK") || \
	!stricmp(fileinfo.ff_name,"LOG.ERR") \
)
/* el2007 >> */

Remplit()
{
struct ffblk fileinfo;

count = 0;
strcpy(source,"a:");
strcat(source,"*.*");

/* << el2007 */
	if(strcmp(niveau,"4") == 0) /* BLANC */
	{
		if(findfirst(source,&fileinfo,0) == -1)
			return(0);

		if(!ONLY_BE_2)
		{
			giProgressionMax += (int)(fileinfo.ff_fsize / 1024) + 1.;
			strcpy(stru_fichier[count].nom,fileinfo.ff_name);
			count++;
		}
		while(findnext(&fileinfo) == 0)
		{
			if(!ONLY_BE_2)
			{
				giProgressionMax += (int)(fileinfo.ff_fsize / 1024) + 1.;
				strcpy(stru_fichier[count].nom,fileinfo.ff_name);
				count++;
			}
		}

		return 1;
	}
/* el2007 >> */
	if(strcmp(niveau,"3") == 0)
	{
		strcpy(stru_fichier[count].nom,"TABLE.URN");
		count++;
	}
	else
	{
		if(findfirst(source,&fileinfo,0) == -1)
			return(0);
		if(strnicmp(fileinfo.ff_name,"BE",2) != 0)
		{
			if(strcmp(niveau,"2") == 0 )
			{
				if((strcmp(fileinfo.ff_name,"TABLE.URN")!= 0)    &&
				   (strcmp(fileinfo.ff_name,"FE_DSK.BK") !=0)   &&
				   (strcmp(fileinfo.ff_name,"FE_DSK") !=0)     &&
				   (strcmp(fileinfo.ff_name,"LOG.ERR") !=0))
				{
					strcpy(stru_fichier[count].nom,fileinfo.ff_name);
					count++;
				}
			}
			else
			{
				if(strcmp(fileinfo.ff_name,"LOG.ERR")!= 0)
				{
					strcpy(stru_fichier[count].nom,fileinfo.ff_name);
					count++;
				}
			}
		}
		while(findnext(&fileinfo) == 0)
		{
			if(strnicmp(fileinfo.ff_name,"BE",2) != 0)
			{
				if(strcmp(niveau,"2") == 0 )
				{
					if((strcmp(fileinfo.ff_name,"TABLE.URN") != 0)   &&
					   (strcmp(fileinfo.ff_name,"FE_DSK.BK") !=0)  &&
					   (strcmp(fileinfo.ff_name,"FE_DSK") !=0)    &&
					   (strcmp(fileinfo.ff_name,"LOG.ERR") !=0))
					{
						strcpy(stru_fichier[count].nom,fileinfo.ff_name);
						count++;
					}
				}
				else
				{
					if(strcmp(fileinfo.ff_name,"LOG.ERR")!= 0)
					{
						strcpy(stru_fichier[count].nom,fileinfo.ff_name);
						count++;
					}
				}
			}
		}
	}
 return(1);
}
/****************************************************************************/
/****************************************************************************/
/* << el2007 */
void Affiche_Progression(int _iActuel)
{
	char szBuff[64];
	static int iLast = -1;
	int iPourc;

	iPourc = (int) ((float)_iActuel / ((float)giProgressionMax) * 100.);

	if(iLast == iPourc || iPourc > 100) /* Pas de rafraichissement */
		return;

	iLast = iPourc;
	setfillstyle(SOLID_FILL, WHITE);
	bar(300,400,600,450);
	setcolor(BLACK);
	settextstyle(SMALL_FONT, HORIZ_DIR,5);
	settextjustify(CENTER_TEXT, CENTER_TEXT);
	sprintf(szBuff, "( %d \% )", iPourc);
	outtextxy(520, 430, szBuff);
	outtextxy(519, 430, szBuff);
}
/* el2007 >> */
CalculMac()
{
	int  handel_Read,fd;
	char plainData [PlainDataLen];
	char Buffer [1050];
	/* << el2007 */
	int iCpt = 0;
	/* el2007 >> */

	j = 0;
	plainData[j] = 0x00;

	for (x = 0; x < count;x++)
	{
		strcpy(source,"a:");
		strcat(source,stru_fichier[x].nom);
		fd = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
		if(fd == -1)
		return(0);
		while((handel_Read = read(fd,Buffer,1024)) != NULL)
		{
			computeMac (macResult, macResultLen, Buffer, handel_Read, decryptedMacKey);
			for (i = 0; i < macResultLen;i++,j++)
				plainData[j] = macResult[i];
			/* << el2007 */
			Affiche_Progression(++iCpt);
			/* el2007 >> */
		}
		plainData[j] = 0x00;
		close(fd);
	}
	computeMac (macResult, macResultLen, plainData, j, decryptedMacKey);
	return(1);
}

/************************************************/
void Calcul_Crc(char *Carte, int pos)
{
	int i,j;
	char Ca[3], Cai[3], Buff[70];

	Ca[0] = 0x80;
	Ca[1] = 0x80;
	Ca[2] = 0x00;

	for(i = 0; i < C_VOTE_MAX_BYTE + _C_TEST_MAX_BYTE;
	    Buff[i] = Carte[pos + i], i++);

	Buff[C_VOTE_MAX_BYTE + _C_TEST_MAX_BYTE] = 0x00;
	computeMac (macResult, macResultLen, Buff, C_VOTE_MAX_BYTE + _C_TEST_MAX_BYTE, decryptedMacKeyMini);

	for(i = 0,j = 4; i < 4;i++,j++)
	{
		Ca[0] = Ca[0] ^ (unsigned char) macResult[i];
		Ca[1] = Ca[1] ^ (unsigned char) macResult[j];
	}
	sprintf(Cai, "%02X",(unsigned char) Ca[0]);
	strcpy(Crc_Calcul,Cai);
	sprintf(Cai, "%02X",(unsigned char) Ca[1]);
	strcat(Crc_Calcul,Cai);
}
/****************************************************************************/
Initialise_Rijndael()
{
	makeKeyBinary (&encKeyInst, DIR_DECRYPT, derivedKey);
	cipherInitBinary (&encCipherInst, iv);
	Rfloppy(dataFromFloppy);
	ret= decryptKeys (decryptedMacKey, decryptedEncKey, dataFromFloppy, derivedKey);
	if (ret < 0)
		return(0);
	else
		return(1);
}
/****************************************************************************/
Initialise_RijndaelMini()
{
	extendPassword(fullPasswordMini,CMinipassword);
	computeKeyFromPassword (derivedKeyMini, fullPasswordMini);
	computeKeyFromPassword (decryptedEncKeyMini, fullPasswordMini);
	computeKeyFromPassword (decryptedMacKeyMini, fullPasswordMini);
}
/****************************************************************************/
Verifie_Password()
{
	ret = validatePassword (fullPassword);
	if (ret != 0)
		return(0);

	ret = computeKeyFromPassword (derivedKey, fullPassword);
	if (ret != 0)
		return(0);
	strcpy(source,"A:");
	/* << el2007 */
	/*  strcat(source,"be");*/
	strcat(source,BE);
	/* el2007 >> */
	ret = Rfile(source);
	if(!ret)
		return(0);

	for (i = 0; i < 16;i++)
		iv[i] = wrkspc[i];
	iv[i] = 0x00;

	return(1);
}
