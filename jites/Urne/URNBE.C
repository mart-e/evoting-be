#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "defs.h"
#include "urnbe.h"
#include "fullspec.h"
#include "alg-fst.c"
#include "api-fst.c"
#include "encdec.c"
#include "enckeys.c"
#include "encmac.c"
#include "hashcomp.c"
#include "maccomp.c"
#include "psswd.c"
#include "lcd.h"

/* << 10.02b */
#define ONLY_BE_4 \
	( \
		!strnicmp(find.name,"BE", 2) || \
		strstr(find.name,".BMP") || \
		!stricmp(find.name,"BLANC.EXE") || \
		!stricmp(find.name,"DZINE.COM") || \
		(strstr(find.name,"MACHVOT") && stricmp(find.name,"MACHVOTS.TBL")) || \
		!strnicmp(find.name,"MAV", 3) || \
		!strnicmp(find.name,"MESSAGE", 7) || \
		!stricmp(find.name,"PARAM.TBL") || \
		(!stricmp(find.name,"TABLE.URN")) || \
		strstr(find.name,"FE_DSK") || \
		!stricmp(find.name,"LOG.ERR") \
	)
/* 10.02b >> */

char type[2],mode[2],sourceori[50],destineori[50],source[50],destine[50];
unsigned long cipherDataLen,plainDataLen;
unsigned char floppy[60];
unsigned char wrkspc[60];
int ret,count;             /* holds the     return code of important method calls. */
unsigned long matches;        /* counts the number of equal bytes in the plaintext and the decrypted ciphertext. */
unsigned char iv[100];
unsigned char ivtmp[100];
unsigned char derivedKey[100];
unsigned char decryptedMacKey[100];
unsigned char decryptedEncKey[100];
unsigned char *decrypted = NULL;
unsigned char hashResult[100];
unsigned char macResult[100];
unsigned char *authenticatedEncryptedPlaintext;
unsigned char dataFromFloppy[50];
unsigned char derivedKeyMini[100];
unsigned char decryptedMacKeyMini[100];
unsigned char decryptedEncKeyMini[100];
unsigned char fullPasswordMini[12];

struct
{
    char nom[14];
}stru_fichier[100];

#define PlainDataLen   24000
#define CipherDataLen  24000
char *pointer, errmsg[50];
unsigned char plainData [PlainDataLen];
unsigned char cipherData[CipherDataLen];

int hashResultLen = sizeof (hashResult);
keyInstance encKeyInst;       /* state of the encryption engine */
cipherInstance encCipherInst; /* specifies the encryption mode */
keyInstance decKeyInst;       /* state of the decryption engine */
cipherInstance decCipherInst; /* specifies the decryption mode */
struct dosdate_t ddate;
struct dostime_t dtime;
struct find_t find;

/*****************************************************************************/
/*       C O N T R O L E   D U   C R C    D E S   F I C H I E R S
/*****************************************************************************/
/* << 10.01 */
int sort_function(const void *_a, const void *_b)
{
	return(strcmp(_a, _b));
}
/******************************************************************************/
void SortFiles(void)
{
	qsort((void *)stru_fichier, count, 14, sort_function);
}
/* 10.01 >> */
/******************************************************************************/
int Crc_Rijndael(char *Mode,char *Type,char *src,char *dest)
{
 int i;
 unsigned char strtmp[20];

  pointer = errmsg;
  strcpy(type,Type);
  strcpy(mode,Mode);
  strcpy(sourceori,src);
  strcpy(destineori,dest);

  ret = Remplit();
  if(ret==-1)
	return(ret);

  /* << 10.01 */
  if(strcmp(type,"2") == 0)
  {
	  SortFiles();
  }
  /* 10.01 >> */

  if((mode[0] == 'w') || (mode[0] == 'W'))
    {

	    /* Nouveau IV */
/* << 10.02e */
/*
	    iv[0] = Minicodage[0];
	    iv[2] = Minicodage[1];
	    iv[4] = Minicodage[2];
	    iv[6] = Minicodage[3];
*/
/* << 10.03b */
	    Read_Ivbit();
/* 10.03b >> */
	    iv[MINI_POS[0]-49] = Minicodage[0];
	    iv[MINI_POS[1]-49] = Minicodage[1];
	    iv[MINI_POS[2]-49] = Minicodage[2];
	    iv[MINI_POS[3]-49] = Minicodage[3];
/* 10.02e >> */
	   /***********************/
	  makeKeyBinary (&encKeyInst, DIR_ENCRYPT, decryptedEncKey);
	  cipherInitBinary (&encCipherInst, iv);
	  ret = Encrypte();
	  if(ret== -1)
	    return(ret);
	  /* << 10.03g */
	  /*CalculMac();*/
	  ret = CalculMac();
	  if(ret== -1)
	    return(ret);
	  /* 10.03g >> */
	  ret = Wmac();
	  if(ret== -1)
	      return(ret);
    }
  else
    {

	MoveTbl();
	makeKeyBinary (&decKeyInst, DIR_DECRYPT, decryptedEncKey);
	cipherInitBinary (&decCipherInst, iv);
	ret = CalculMac();
	if(ret== -1)
	    return(ret);

	/* << 10.01 */
	/*strcpy(source,sourceori);
	strcat(source,"\\");
	strcat(source,"be");*/
	sprintf(source,"%s\\be.%s",sourceori,Type);
	/* 10.01 >> */
	ret = Rfile(source);
	if(ret== -1)
	    return(ret);
	for (i = 0; i < macResultLen;i++)
	if(macResult[i] != wrkspc[i+16])
	     return(-1);
    }
return(1);
}
/****************************************************************************/
int Rfloppy(char *msg)
{
  int fd;
  int handel_Read;

  pointer = errmsg;
  strcpy(floppy,"a:");
  strcat(floppy,"Floppy.be");

  if((fd = open( floppy, O_BINARY | O_RDWR)) == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",floppy);
	  Erreur(pointer);
	  return(-1);
	 }
  handel_Read =read(fd,msg,80);
  if(handel_Read == -1)
	 {
	  sprintf(errmsg,"Erreur de lecture du fichier %s\n",floppy);
	  Erreur(pointer);
	  /* << 10.03g */
	  close(fd);
	  /* 10.03g >> */
	  return(-1);
	 }
  close(fd);
  return(1);
}
/****************************************************************************/
int Rfile(char *fich)
{
  int fd,i,j;
  unsigned char Bffer[100];
  int handel_Read;

  pointer = errmsg;


  fd = open( fich, O_RDWR | O_BINARY);
  if(fd == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",fich);
	  Erreur(pointer);
	  return(-1);
	 }

  handel_Read = read(fd,Bffer,64);
  if(handel_Read == -1)
	 {
	  sprintf(errmsg,"Erreur de lecture du fichier %s\n",fich);
	  Erreur(pointer);
	  /* << 10.03g */
	  close(fd);
	  /* 10.03g >> */
	  return(-1);
	 }
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
int Remplit()
{
register int i;
pointer = errmsg;

strcpy(source,sourceori);
strcat(source,"\\");
strcat(source,"*.*");
count = 0;

/* << 10.02b */
	if(strcmp(type,"2") == 0) /* URNE */
	{
		if(_dos_findfirst( source, 0x00, &find ))
		{
			sprintf(errmsg,"Aucun fichier  traiter\n");
			Erreur(pointer);
			return(-1);
		}
		if(!ONLY_BE_4)
		{
			strcpy(stru_fichier[count].nom,find.name);
			count++;
		}
		while(!_dos_findnext(&find))
		{
			if(!ONLY_BE_4)
			{
				strcpy(stru_fichier[count].nom,find.name);
				count++;
			}
		}
		return 1;
	}
/* 10.02b >> */
if(strcmp(type,"3") == 0)
   {
     strcpy(stru_fichier[count].nom,"TABLE.URN");
     count++;
   }
else
   {
     if(_dos_findfirst( source, 0x00, &find ) )
	   {
	     sprintf(errmsg,"Aucun fichier  traiter\n");
	     Erreur(pointer);
	     return(-1);
	   }
     if(strnicmp(find.name,"BE",2) != 0)
	  {
	   if(strcmp(type,"2") == 0)
	    {
	      if((strcmp(find.name,"TABLE.URN")!= 0) &&
		  (strcmp(find.name,"FE_DSK.BK") !=0)&&
		   (strcmp(find.name,"FE_DSK") !=0)&&
		    (strcmp(find.name,"LOG.ERR") !=0))
		{
		  strcpy(stru_fichier[count].nom,find.name);
		  count++;
		}
	    }
     else
	   {
	     strcpy(stru_fichier[count].nom,find.name);
	     count++;
	   }
	 }
     while(!_dos_findnext( &find ) )
      {
       if(strnicmp(find.name,"BE",2) != 0)
	 {
	  if(strcmp(type,"2") == 0)
	    {
	     if((strcmp(find.name,"TABLE.URN") != 0) &&
		  (strcmp(find.name,"FE_DSK.BK") !=0)&&
		   (strcmp(find.name,"FE_DSK") !=0)&&
		    (strcmp(find.name,"LOG.ERR") !=0))
		       {
			 strcpy(stru_fichier[count].nom,find.name);
			 count++;
		       }
	    }
	 else
	    {
	      strcpy(stru_fichier[count].nom,find.name);
	      count++;
	    }
	 }
      }

   }
return(1);
}
/****************************************************************************/
int Wmac()
{
  int fd, i;
  int handel_Write;
  unsigned char Bffer[100];
  unsigned char Cai[3];

  pointer = errmsg;

  /* << 10.01 */
  /*strcpy(destine,destineori);
  strcat(destine,"\\be");*/
  sprintf(destine,"%s\\be.%s",destineori,type);
  /* 10.01 >> */
  unlink(destine);

  fd = open( destine, O_BINARY | O_WRONLY | O_CREAT | O_EXCL,
					S_IREAD | S_IWRITE );

  if(fd == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",destine);
	  Erreur(pointer);
	  return(-1);
	 }

  Bffer[0] = 0x00;

  for(i = 0; i < 16;i++)
	{
	  sprintf(Cai, "%02X",(unsigned char) ivtmp[i]);
	  strcat(Bffer,Cai);
	}

  for(i = 0; i < macResultLen;i++)
	{
	  sprintf(Cai, "%02X",(unsigned char) macResult[i]);
	  strcat(Bffer,Cai);
	}

  handel_Write = write(fd,Bffer,strlen(Bffer));
  if(handel_Write != strlen(Bffer))
	 {
	  sprintf(errmsg,"Erreur d'ecriture du fichier %s\n",destine);
	  Erreur(pointer);
	  /* << 10.03g */
	  close(fd);
	  /* 10.03g >> */
	  return(-1);
	 }
  close(fd);
  return(1);
}
/****************************************************************************/
int CalculMac()
{
int  handel_Read,fd,j,x,i;
unsigned char Buffer [1030];

pointer = errmsg;


j = 0;
plainData[j] = 0x00;

for (x = 0; x < count;x++)
  {
     if(strcmp(type,"3") == 0)
       strcpy(source,destineori);
     else
       strcpy(source,sourceori);

     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);

     fd = open(source, O_RDWR | O_BINARY);
     if(fd == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
	  Erreur(pointer);
	  return(-1);
	 }

      while((handel_Read = read(fd,Buffer,1024)) > 0)
       {
	/* Leds */

	if(GCOLOR ==_ON_GREEN)
		  GCOLOR = _ON_RED;
	else
		  GCOLOR = _ON_GREEN;
	Light_LED(GCOLOR);

	computeMac (macResult, macResultLen, Buffer, handel_Read, decryptedMacKey);
	for (i = 0; i < macResultLen;i++,j++)
	    plainData[j] = macResult[i];
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

 for (i = 0; i < C_VOTE_MAX_BYTE + _C_TEST_MAX_BYTE;
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
int Encrypte()
{
int  handel_Read, handel_Write,Carry_Read,Carry_Write;
pointer = errmsg;

     strcpy(source,sourceori);
     strcat(source,"\\");
     strcat(source,"TABLE.URN");
     plainData[0] = 0x00;
     cipherData[0] = 0x00;

     handel_Read = open(source, O_RDWR | O_BINARY);
     if(handel_Read == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
	  Erreur(pointer);
	  return(-1);
	 }

     strcpy(destine,destineori);
     strcat(destine,"\\");
     strcat(destine,"TABLE.URN");

     Carry_Read = read(handel_Read,plainData,PlainDataLen);
      if(Carry_Read == -1)
       {
	 sprintf(errmsg,"Erreur de lecture du fichier %s\n",source);
	 Erreur(pointer);
	  /* << 10.03g */
	 close(handel_Read);
	 /* 10.03g >> */
	 return(-1);
       }
     close(handel_Read);
     unlink(destine);
     handel_Write = open( destine, O_BINARY | O_WRONLY | O_CREAT | O_EXCL,
					S_IREAD | S_IWRITE );
     if(handel_Write == -1)
	 {
	  sprintf(errmsg,"Erreur de cration du fichier %s\n",destine);
	  Erreur(pointer);
	  return(-1);
	 }

     cipherDataLen = encryptData (cipherData, &encCipherInst, &encKeyInst, plainData,
		 Carry_Read);
     Carry_Write = write(handel_Write,cipherData,cipherDataLen);
     if(Carry_Write != cipherDataLen)
	 {
	  sprintf(errmsg,"Erreur d'criture du fichier %s\n",destine);
	  Erreur(pointer);
	  /* << 10.03g */
	  close(handel_Write);
	  /* 10.03g >> */
	  return(-1);
	 }
     close(handel_Write);

 return(1);
}
/****************************************************************************/
int MoveTbl()
{
int  handel_Read, handel_Write,Carry_Read, Carry_Write;

pointer = errmsg;

     plainData[0] = 0x00;
     cipherData[0] = 0x00;

     handel_Read = open("a:urne.tbl", O_RDWR | O_BINARY);
     if(handel_Read == -1)
	 {
	  sprintf(errmsg,"Erreur d'ouverture du fichier %s\n","Urne.tbl");
	  Erreur(pointer);
	  return(-1);
	 }

     Carry_Read = read(handel_Read,plainData,1024);
     if(Carry_Read == -1)
       {
	 sprintf(errmsg,"Erreur de lecture du fichier %s\n","Urne.tbl");
	 Erreur(pointer);
	 /* << 10.03g */
	  close(handel_Read);
	  /* 10.03g >> */
	 return(-1);
       }
     close(handel_Read);
     unlink("c:urne.tbl");
     handel_Write = open( "c:urne.tbl", O_BINARY | O_WRONLY | O_CREAT | O_EXCL,
					S_IREAD | S_IWRITE );
     if(handel_Write == -1)
	 {
	  sprintf(errmsg,"Erreur de cration du fichier %s\n","Urne.tbl");
	  Erreur(pointer);
	  return(-1);
	 }

     Carry_Write = write(handel_Write,plainData,Carry_Read);
     if(Carry_Write != Carry_Read)
	 {
	  sprintf(errmsg,"Erreur d'criture du fichier %s\n","Urne.tbl");
	  Erreur(pointer);
	  /* << 10.03g */
	  close(handel_Write);
	  /* 10.03g >> */
	  return(-1);
	 }
     close(handel_Write);
 return(1);
}
/****************************************************************************/

int Initialise_Rijndael()
{
   int i;

   pointer = errmsg;
   makeKeyBinary (&encKeyInst, DIR_DECRYPT, derivedKey);
   cipherInitBinary (&encCipherInst, iv);
   Rfloppy(dataFromFloppy);
   ret= decryptKeys (decryptedMacKey, decryptedEncKey, dataFromFloppy, derivedKey);
   if (ret < 0)
      {
	sprintf(errmsg,"Erreur de decryptage\n");
	Erreur(pointer);
	return(-1);
      }
   else
	    return(1);
}
/****************************************************************************/
void Initialise_RijndaelMini()
{
   extendPassword(fullPasswordMini,CMinipassword);
   computeKeyFromPassword (derivedKeyMini, fullPasswordMini);
   computeKeyFromPassword ( decryptedEncKeyMini, fullPasswordMini);
   computeKeyFromPassword (decryptedMacKeyMini, fullPasswordMini);
}
/****************************************************************************/
int Verifie_Password()
{
  int i;

  ret = validatePassword (fullPassword);
  if (ret != 0)
	  return(0);

  ret = computeKeyFromPassword (derivedKey, fullPassword);
  if (ret != 0)
	  return(0);

   strcpy(source,"a:");
   strcat(source,"\\");
   /* << 10.01 */
   //strcat(source,"be");
   strcat(source,"be.2");
   /* 10.01 >> */
   ret = Rfile(source);
   if(ret == -1)
	    return(ret);

   for (i = 0; i < 16;i++)
      {
	iv[i] = wrkspc[i];
	ivtmp[i] = wrkspc[i];
      }
   iv[i] = 0x00;
   ivtmp[i] = 0x00;

  return(1);
}
/****************************************************************************/
/*             F I C H I E R     D E S    E R R E U R S                     */
void Erreur(char *msg)
{
  FILE *fp;

  strcpy(destine,destineori);
  strcat(destine,"\\LOG.ERR");
  fp = fopen(destine,"a");
  fputs(msg,fp);
  fclose(fp);
}
/****************************************************************************/
/****************************************************************************/
Initialise_Date(char *Buffer)
/*
    Analyse la table provenant du PC de controle, initialise la Date
    Rend void
*/
{
  char strtmp[4];
  char dyy[3],dmm[3],djj[3];
  int yy,jj,mm;


  djj[0] = Buffer[9];
  djj[1] = Buffer[10];
  djj[2] = 0x00;
  dmm[0] = Buffer[11];
  dmm[1] = Buffer[12];
  dmm[2] = 0x00;
  dyy[0] = Buffer[13];
  dyy[1] = Buffer[14];
  dyy[2] = 0x00;
  sscanf(djj,"%d",&jj);
  sscanf(dmm,"%d",&mm);
  sscanf(dyy,"%d",&yy);
  if(yy < 80)
	 yy +=2000;
  else
	 yy +=1900;
  ddate.year = yy;
  ddate.month = mm;
  ddate.day = jj;
  _dos_setdate( &ddate );
  dtime.hour = 8;
  dtime.minute =0;
  dtime.second = 0;
 _dos_settime( &dtime );

}

