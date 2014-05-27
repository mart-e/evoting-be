#include "fullSpec.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <dir.h>
/* << el2007 */
#include <process.h>
#include <alloc.h>

/* << AP 16/04/2010 
__#define MINI_PWD "2701"
__#define MINI_POS "1467" 
*/

#define MINI_PWD "2703"
#define MINI_POS "1368"

/* >> AP 16/04/2010 */

/* el2007 >> */

#define IGNORE 0
#define RETRY  1
#define ABORT  2
#define BREAK  3
/* << el2007 */
/*
#define PlainDataLen   24000
#define CipherDataLen  24000
*/
/* el2007 >> */

int handler(int errval,int ax,int bp,int si);
extern void Control_Keyboard(void);
extern void Control_Keyboard_Restore(void);
int Remplit(void);
int Remplit_Niveau1(void);
int Rfile(char *fichier);
extern Erreur(char *msg);
extern Rfloppy(char *msg);
int Encrypte(void);
int Decrypte(void);
int CalculMac(char *Path);
int Wmac(void);

struct
{
    char nom[32];
}stru_fichier[150];

/* << el2007 */
/*char Minicodage[] = "4630";*/
char Minicodage[] = MINI_PWD;
/* el2007 >> */
unsigned long decryptedLen;
unsigned long cipherDataLen,plainDataLen;
BYTE *point;
BYTE destine[60],destineori[60],floppy[60],floppyori[60], errmsg[100];
BYTE source[60],sourceori[60];
BYTE wrkspc[60];
BYTE fullPassword[fullPasswordLength + 1];
BYTE be_filename[5]; /* be.1 - be.2 - be.3 */

int ret,count,x,j;             /* holds the return code of important method calls. */
unsigned long matches;        /* counts the number of equal bytes in the plaintext and the decrypted ciphertext. */
unsigned long i;              /* counter to produce nice output */
keyInstance encKeyInst;       /* state of the encryption engine */
cipherInstance encCipherInst; /* specifies the encryption mode */
keyInstance decKeyInst;       /* state of the decryption engine */
cipherInstance decCipherInst; /* specifies the decryption mode */

BYTE cipherKey[100];
BYTE macKey[100];

BYTE encKey[100];
BYTE iv[100];
BYTE derivedKey[100];
BYTE decryptedMacKey[100];
BYTE decryptedEncKey[100];
BYTE *decrypted = NULL;
BYTE hashResult[100];
BYTE macResult[100];
BYTE *authenticatedEncryptedPlaintext;
BYTE dataForFloppy[50];
BYTE dataFromFloppy[50];
BYTE mode[2];
BYTE niveau[2];

int hashResultLen = sizeof (hashResult);
int dataForFloppyLength;

#include "alg-fst.c"
#include "api-fst.c"
#include "encdec.c"
#include "enckeys.c"
#include "encmac.c"
#include "hashcomp.c"
#include "maccomp.c"
#include "psswd.c"

int sort_function(const void *_a, const void *_b)
{
	return(strcmp(_a, _b));
}

void SortFiles(void)
{
	qsort((void *)stru_fichier, count, 32, sort_function);
}

main(int argc, char *argv[], char **envp)
{
  FILE *fu;

  point = errmsg;

/* << el2007 */
	if(!strcmp(argv[2],"9"))
	{
		/* BE.2 */
		strcpy(argv[2],"2");
		spawnve(P_WAIT, argv[0], argv, envp);
		strcpy(destine,argv[5]);
		strcat(destine,"\\LOG.ERR");
		if(access(destine,0) == 0)
			return 1;
		/* BE.4 */
		strcpy(argv[2],"4");
		spawnve(P_WAIT, argv[0], argv, envp);
		return 1;
	}
/* el2007 >> */

  strcpy(mode,argv[1]);
  strcpy(niveau,argv[2]);
  strcpy(fullPassword,argv[3]);
  strcpy(sourceori,argv[4]);
  strcpy(destineori,argv[5]);
  strcpy(floppyori,argv[6]);

  strcpy(destine,destineori);
  strcat(destine,"\\LOG.ERR");
  unlink(destine);

  /* Gestion du clavier + interrupt disquette */
  if((mode[0] == 'v') ||  (mode[0] == 'V') && strcmp(niveau,"1") == 0)
  {
    harderr(handler);
    /* management of the keyboard */
    Control_Keyboard();
  }

  /* If aborded (diskette removed) -> \\LOG.ERR */
  if((mode[0] == 'r') ||  (mode[0] == 'R') ||
     (mode[0] == 'v') ||  (mode[0] == 'V'))
  {
    if(strcmp(niveau,"1") == 0)
    {
      fu = fopen(destine,"a");
      fclose(fu);
    }
  }

  /* be_filename */
  strcpy(be_filename, "be.");
  strcat(be_filename, niveau);
  be_filename[4] = 0x00;

/* <validatePassword> determines whether an integrity-protected password is valid. */
/* <return code>      zero refers to a valid password, other values refer to an invalid password. */
/* <fullPassword>     refers to the zero-terminated string with the integrity-protected password of <fullPassword-1> characters. */



  ret = validatePassword (fullPassword);
  if (ret != 0)
    {
	  sprintf(errmsg,"password <%s> is *INVALID*\n", fullPassword);
	  Erreur(point);
    }

/* <computeKeyFromPassword> computes a the hash value using the zero-terminated integrity-protected <fullPassword>. */
/*                          this method validates the integrity-protected fullPassword before computing the cipherKey. */
/* <return code>            zero means ok, all other values refer to an error. */
/* <cipherKey>                    refers to the array of byte that must be able to accomodate 16 bytes. */
/* <fullPassword>           refers to the zero-terminated integrity-protected fullPassword. */

  ret = computeKeyFromPassword (derivedKey, fullPassword);
  if (ret != 0)
    {
       sprintf(errmsg,"cipherKey derivation fails with error <%i>\n",ret);
       Erreur(point);
    }
   Remplit();

  SortFiles();

/* <encryptKeys> produces the ciphertext of a <macKey> and an <encKey>, given an initialization vector <iv> and the 16-byte key with which these keys must be encrypted. */
/* <return code> an integer, negative values refer to an error code, positive values specify the length of the output parameter <dataForFloppy>. */
/* <dataForFloppy> refers to an array of char that will contain the information that can be stored on a floppy disk. */
/*                 <dataForFloppy> must be able to accomodate at least <3*<blockLength>> bytes. */
/* <cipherKey>     refers to the 16-byte encryption key with which the two keys (<macKey> and <encKey>) must be encrypted. */
/*                 this key must be different must not be related to <encKey> and <macKey> */
/* <iv>            refers to the 16-byte initialization vector for the encryption. */
/*                 <iv> must be used only once, and should be generated randomly. */
/* <macKey>        refers to the 16-byte MAC-key that must be encrypted. */
/* <encKey>        refers to the 16-byte encryption key that must be encrypted. */

  if((mode[0] == 'w') ||  (mode[0] == 'W'))
    {
         strcpy(floppy,floppyori);
         strcat(floppy,"\\");
         strcat(floppy,"ivmac.be");
         Rfile(floppy);

         for (i = 0; i < 16;i++)
            iv[i] = wrkspc[i];
         iv[i] = 0x00;


         makeKeyBinary (&encKeyInst, DIR_ENCRYPT, derivedKey);
         cipherInitBinary (&encCipherInst, iv);

         Rfloppy(dataFromFloppy);
         ret= decryptKeys (decryptedMacKey, decryptedEncKey, dataFromFloppy, derivedKey);
	 if (ret < 0)
	    {
	       sprintf(errmsg,"problem during decryptKeys: <%i>.\n", ret);
	       Erreur(point);
	    }

          makeKeyBinary (&encKeyInst, DIR_ENCRYPT, decryptedEncKey);
          cipherInitBinary (&encCipherInst, iv);

          Encrypte();
          CalculMac(destineori);
          Wmac();
    }

/* <decryptKeys> decrypts the data retrieved from floppy <dataFromFloppy>. */
/* <return code> an integer, negative values refer to an error code, zero means that no problem was detected.. */
/*               note that zero does not indicate that the decrypted keys are valid: no redundancy is added to avoid exhaustive key searches. */
/* <macKey>      refers to the 16-byte MAC-key that has been decrypted. */
/* <encKey>      refers to the 16-byte encryption key that has been decrypted. */
/* <dataFromFloppy> refers to an array of char that contains the information that was stored on a floppy disk. */
/*                 <dataFromFloppy> must consist of <3*<blockLength>> bytes. */
/* <cipherKey>     refers to the 16-byte encryption key with which the <dataFromFloppy> will be decrypted. */

  else
   {
          strcpy(source,sourceori);
          strcat(source,"\\");
          strcat(source,be_filename);
          Rfile(source);

         for (i = 0; i < 16;i++)
            iv[i] = wrkspc[i];
         iv[i] = 0x00;

        if(strcmp(niveau,"3") == 0)
           {
            /* Nouveau IV */
/* << el2007 */
/*
            iv[0] = Minicodage[0];
            iv[2] = Minicodage[1];
            iv[4] = Minicodage[2];
            iv[6] = Minicodage[3];
*/
            iv[MINI_POS[0]-49] = Minicodage[0];
            iv[MINI_POS[1]-49] = Minicodage[1];
            iv[MINI_POS[2]-49] = Minicodage[2];
            iv[MINI_POS[3]-49] = Minicodage[3];
/* el2007 >> */
           /***********************/
           }


         makeKeyBinary (&encKeyInst, DIR_DECRYPT, derivedKey);
         cipherInitBinary (&encCipherInst, iv);
         Rfloppy(dataFromFloppy);
         ret= decryptKeys (decryptedMacKey, decryptedEncKey, dataFromFloppy, derivedKey);
	 if (ret < 0)
	    {
	       sprintf(errmsg,"problem during decryptKeys: <%i>.\n", ret);
	       Erreur(point);
	    }

          makeKeyBinary (&decKeyInst, DIR_DECRYPT, decryptedEncKey);
          cipherInitBinary (&decCipherInst, iv);

          CalculMac(sourceori);

          strcpy(source,sourceori);
          strcat(source,"\\");
          strcat(source,be_filename);
          Rfile(source);

        for (i = 0; i < macResultLen;i++)
            if(macResult[i] != wrkspc[i+16])
	    {
               sprintf(errmsg,"problem CRC.\n");
	       Erreur(point);
	    }
          Decrypte();
    }

  /* If aborded (diskette removed) -> \\LOG.ERR */
  if((mode[0] == 'r') ||  (mode[0] == 'R') ||
     (mode[0] == 'v') ||  (mode[0] == 'V'))
  {
    if(strcmp(niveau,"1") == 0)
    {
      unlink(destine);
    }
  }

	if((mode[0] == 'v') ||  (mode[0] == 'V'))
		Control_Keyboard_Restore();

	return 1;
}
/****************************************************************************/
/*             F I C H I E R     D E S    E R R E U R S                     */

Erreur(char *msg)
{
  FILE *fps;

  strcpy(destine,destineori);
  strcat(destine,"\\LOG.ERR");
  fps = fopen(destine,"a");
  fputs(msg,fps);
  fclose(fps);

	if((mode[0] == 'v') ||  (mode[0] == 'V'))
		Control_Keyboard_Restore();

  exit(0);

	return 1;
}
/****************************************************************************/
/****************************************************************************/
/*                            F I C H I E R S                               */

Wfloppy(char *msg,int ret)
{
  int fd;

  strcpy(floppy,floppyori);
  strcat(floppy,"\\floppy.be");

  if((fd = open(floppy,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening Floppy.be");
	  Erreur(point);
    }

  write(fd,msg,ret);
  close(fd);
  exit(0);

	return 1;
}

/****************************************************************************/

Rfloppy(char *msg)
{
  int fd;

  strcpy(floppy,floppyori);
  strcat(floppy,"\\Floppy.be");

  if((fd = open( floppy, O_BINARY | O_RDWR)) == -1)
    {
	  sprintf(errmsg,"error opening Floppy.be");
	  Erreur(point);
    }
  read(fd,msg,80);
  close(fd);

	return 1;
}

/****************************************************************************/

Rfile(char *fichier)
{
  int fps;
  BYTE Bffer[100],C1,C2;
  int i,j,n,Carry;


  if((fps = open( fichier, O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening %s",fichier);
	  Erreur(point);
    }

  Carry = read(fps,Bffer,64);
  close(fps);
  Bffer[Carry] = 0x00;

  for ( i = 0 ; i <  Carry ; i++)
      {
	 if(Bffer[i] < 58)
	      Bffer[i] -= 48;
	  else
	      Bffer[i] -= 55;
      }


  for ( i = 0, j = 0; j < Carry / 2; j++)
      {
           wrkspc[j]   = Bffer[i++] << 4;
           wrkspc[j]   = Bffer[i++] | wrkspc[j];
      }
      wrkspc[j] = 0x00;
	return 1;
}

/*****************************************************************************/
Wmac()
{
  int fd;
  int i;
  BYTE Bffer[100];
  BYTE Cai[3];

  strcpy(destine,destineori);
  strcat(destine,be_filename);
  unlink(destine);
  if((fd = open(destine,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening mac.be");
	  Erreur(point);
    }

  Bffer[0] = 0x00;
  for(i = 0; i < 16;i++)
        {
          sprintf(Cai, "%02X",(unsigned char) iv[i]);
          strcat(Bffer,Cai);
        }

  for(i = 0; i < macResultLen;i++)
        {
          sprintf(Cai, "%02X",(unsigned char) macResult[i]);
          strcat(Bffer,Cai);
        }

  write(fd,Bffer,strlen(Bffer));
  close(fd);
	return 1;
}

#pragma warn -par
error_win(char *msg)
{
	return(RETRY);
}

#pragma warn -par
int handler(int errval,int ax,int bp,int si)
{
	hardresume(error_win(""));

	return ABORT;
}

Get_EL_Date(BYTE *_date)
{
	FILE *fu;
	char line[9];
	char pathname[60];

	sprintf(pathname, "%s\\PCT%d.DSK", sourceori, 1);
	fu = fopen(pathname, "r");
	if(fu == NULL)
	{
		sprintf(pathname, "%s\\PCT%d.DSK", sourceori, 2);
		fu = fopen(pathname, "r");
		if(fu == NULL)
		{
			sprintf(pathname, "%s\\PCT%d.DSK", sourceori, 3);
			fu = fopen(pathname, "r");
			if(fu == NULL)
			{
				_date[0] = 0x00;
				return 0;
			}
		}
	}

	fread(line, 8, 1, fu);
	_date[0] = line[2];
	_date[1] = line[3];
	_date[2] = line[4];
	_date[3] = line[5];
	_date[4] = line[6];
	_date[5] = line[7];
	_date[6] = 0x00;
	/*puts(_date);*/

	fclose(fu);
	return 1;
}

Remplit_Niveau1()
{
	register int i;struct ffblk fileinfo;
	BYTE pathname[60], path[60], el_date[7];

	/* Stru_bas */
	strcpy(pathname,sourceori);
	strcpy(path, "Stru_bas\\");
	strcat(pathname, path);
	strcat(pathname,"*.*");

	if(findfirst(pathname,&fileinfo,0) != -1)
	{
		sprintf(stru_fichier[count].nom, "%s%s", path, fileinfo.ff_name);
		count++;
		while(findnext(&fileinfo) == 0)
		{
			sprintf(stru_fichier[count].nom, "%s%s", path, fileinfo.ff_name);
			count++;
		}
	}

	/* EL?????? */
	strcpy(pathname,sourceori);
	strcpy(path,"EL");
	Get_EL_Date(el_date);
	strcat(path,el_date);
	strcat(path,"\\");
	strcat(pathname, path);
	strcat(pathname,"*.*");
	/*puts(pathname);*/

	if(findfirst(pathname,&fileinfo,0) != -1)
	{
		sprintf(stru_fichier[count].nom, "%s%s", path, fileinfo.ff_name);
		count++;
		while(findnext(&fileinfo) == 0)
		{
			sprintf(stru_fichier[count].nom, "%s%s", path, fileinfo.ff_name);
			count++;
		}
	}
	return 1;
}

/****************************************************************************/
/******************************************************************************/
/******************************************************************************/
/*       C O N T R O L E   D U   C R C    D E S   F I C H I E R S            */
/*             P R E P A R A T I O N    D U    T A B L E A U                 */
/* << el2007 */
#define ONLY_BE_4 \
	( \
		!strnicmp(fileinfo.ff_name,"BE", 2) || \
		strstr(fileinfo.ff_name,".BMP") || \
		!stricmp(fileinfo.ff_name,"BLANC.EXE") || \
		!stricmp(fileinfo.ff_name,"DZINE.COM") || \
		(strstr(fileinfo.ff_name,"MACHVOT") && stricmp(fileinfo.ff_name,"MACHVOTS.TBL")) || \
		!strnicmp(fileinfo.ff_name,"MAV", 3) || \
		!strnicmp(fileinfo.ff_name,"MESSAGE", 7) || \
		!stricmp(fileinfo.ff_name,"PARAM.TBL") || \
		(!stricmp(fileinfo.ff_name,"TABLE.URN")) || \
		strstr(fileinfo.ff_name,"FE_DSK") || \
		!stricmp(fileinfo.ff_name,"LOG.ERR") \
	)

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
register int i;struct ffblk fileinfo;
/* << el2007 */
FILE *fu;
/* el2007 >> */
point = errmsg;
count = 0;
strcpy(source,sourceori);
strcat(source,"\\*.*");


/* << el2007 */
	/* URNE */
	if(strcmp(niveau,"2") == 0)
	{
		if(findfirst(source,&fileinfo,0) == -1)
		{
			sprintf(errmsg,"Aucun fichier  traiter\n");
			Erreur(point);
		}
		if(!ONLY_BE_4)
		{
			strcpy(stru_fichier[count].nom,fileinfo.ff_name);
			count++;
		}
		while(findnext(&fileinfo) == 0)
		{
			if(!ONLY_BE_4)
			{
				strcpy(stru_fichier[count].nom,fileinfo.ff_name);
				count++;
			}
		}
		return 1;
	}

	/* BLANC */
	if(strcmp(niveau,"4") == 0)
	{
		if(findfirst(source,&fileinfo,0) == -1)
		{
			sprintf(errmsg,"Aucun fichier  traiter\n");
			Erreur(point);
		}
		if(!ONLY_BE_2)
		{
			strcpy(stru_fichier[count].nom,fileinfo.ff_name);
			count++;
		}
		while(findnext(&fileinfo) == 0)
		{
			if(!ONLY_BE_2)
			{
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
		{
			sprintf(errmsg,"Aucun fichier  traiter\n");
			Erreur(point);
		}
		if(strnicmp(fileinfo.ff_name,"BE",2) != 0)
		{
			if(strcmp(niveau,"2") == 0 )
			{
				if((strcmp(fileinfo.ff_name,"TABLE.URN")!= 0)    &&
					(strcmp(fileinfo.ff_name,"FE_DSK.BK") !=0)   &&
					(strcmp(fileinfo.ff_name,"FE_DSK") !=0)     &&
					(strcmp(fileinfo.ff_name,"FE_DSK.CRP") !=0)     &&
					(strcmp(fileinfo.ff_name,"LOG.ERR") !=0)   &&
		    (strcmp(fileinfo.ff_name,"urne.exe") !=0))
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
						(strcmp(fileinfo.ff_name,"FE_DSK.CRP") !=0)  &&
						(strcmp(fileinfo.ff_name,"LOG.ERR") !=0)   &&
		    (strcmp(fileinfo.ff_name,"urne.exe") !=0))
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

	if(strcmp(niveau,"1") == 0 )
	{
		Remplit_Niveau1();
	}
	return 1;
}
/****************************************************************************/
Encrypte()
{
register int x, n, i, Carry;
int  handel_Read, handel_Write;
/* << el2007 */
/*
BYTE plainData [PlainDataLen];
BYTE cipherData[CipherDataLen];
*/
BYTE *plainData;
BYTE *cipherData;
unsigned long PlainDataLen;
unsigned long CipherDataLen;
/* el2007 >> */

for (x = 0; x < count;x++)
 {
  if((strcmp(stru_fichier[x].nom,"TABLE.URN") == 0))
   {
     strcpy(source,sourceori);
     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);
/* << el2007 */
     /*plainData[0] = 0x00;
     cipherData[0] = 0x00;*/
/* el2007 >> */

     handel_Read = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Read == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
	Erreur(point);
       }
     strcpy(destine,destineori);
     strcat(destine,"\\");
     strcat(destine,stru_fichier[x].nom);
/* << el2007 */
PlainDataLen = CipherDataLen = filelength(handel_Read);
plainData = (BYTE *) malloc ( sizeof(BYTE) * PlainDataLen );
cipherData = (BYTE *) malloc ( sizeof(BYTE) * CipherDataLen );
if(plainData == NULL || cipherData == NULL)
{
sprintf(errmsg,"Probleme d'allocation de memoire\n");
Erreur(point);
}
plainData[0] = 0x00;
cipherData[0] = 0x00;
/* el2007 >> */

     plainDataLen = read(handel_Read,plainData,PlainDataLen);
      if(plainDataLen == -1)
        {
         sprintf(errmsg,"Erreur de lecture du fichier %s\n",source);
         Erreur(point);
        }
     close(handel_Read);
     unlink(destine);
     handel_Write = open(destine,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Write == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",destine);
	Erreur(point);
       }

      cipherDataLen = encryptData (cipherData, &encCipherInst, &encKeyInst, plainData,
		 plainDataLen);
      Carry = write(handel_Write,cipherData,cipherDataLen);
      if(Carry != cipherDataLen)
	   {
            sprintf(errmsg,"Erreur d'criture du fichier %s\n",destine);
	    Erreur(point);
	   }
     close(handel_Write);
/* << el2007 */
free( plainData );
free ( cipherData );
/* el2007 >> */
   }
 }
	return 1;
}
/****************************************************************************/
Decrypte()
{
register int x, n, i, Carry;
int  handel_Read, handel_Write;
/* << el2007 */
/*
BYTE plainData [PlainDataLen];
BYTE cipherData[CipherDataLen];
*/
BYTE *plainData;
BYTE *cipherData;
unsigned long PlainDataLen;
unsigned long CipherDataLen;
/* el2007 >> */

for (x = 0; x < count;x++)
 {
  if((strcmp(stru_fichier[x].nom,"TABLE.URN") == 0))
   {
     strcpy(source,sourceori);
     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);

/* << el2007 */
/*
     plainData[0] = 0x00;
     cipherData[0] = 0x00;
*/
/* el2007 >> */

     handel_Read = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Read == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
	Erreur(point);
       }
/* << el2007 */
PlainDataLen = CipherDataLen = filelength(handel_Read);
plainData = (BYTE *) malloc ( sizeof(BYTE) * PlainDataLen );
cipherData = (BYTE *) malloc ( sizeof(BYTE) * CipherDataLen );
if(plainData == NULL || cipherData == NULL)
{
sprintf(errmsg,"Probleme d'allocation de memoire\n");
Erreur(point);
}
plainData[0] = 0x00;
cipherData[0] = 0x00;
/* el2007 >> */
     strcpy(destine,destineori);
     strcat(destine,"\\");
     strcat(destine,stru_fichier[x].nom);
     cipherDataLen = read(handel_Read,cipherData,CipherDataLen);
      if(cipherDataLen == -1)
        {
         sprintf(errmsg,"Erreur de lecture du fichier %s\n",source);
         Erreur(point);
        }
     close(handel_Read);
     unlink(destine);

     handel_Write = open(destine,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Write == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",destine);
	Erreur(point);
       }
      plainDataLen = decryptData (plainData, &decCipherInst, &decKeyInst, cipherData,
                 cipherDataLen);
      Carry = write(handel_Write,plainData,plainDataLen);
      if(Carry != plainDataLen)
	   {
            sprintf(errmsg,"Erreur d'criture du fichier %s\n",destine);
	    Erreur(point);
	   }
     close(handel_Write);
/* << el2007 */
free( plainData );
free ( cipherData );
/* el2007 >> */
   }
 }
	return 1;
}

/****************************************************************************/
CalculMac(char *Path)
{
register int x, n, i, Carry,j;
int  handel_Read;
/* << el2007 */
/*
BYTE plainData [PlainDataLen];
BYTE cipherData[CipherDataLen];
*/
BYTE *plainData;
unsigned long PlainDataLen;
/* el2007 >> */

BYTE Buffer [1050];
j = 0;
/* << el2007 */
/*plainData[j] = 0x00;*/

PlainDataLen = 1;
plainData = (BYTE *) malloc ( sizeof(BYTE) * PlainDataLen );
if(plainData == NULL)
{
sprintf(errmsg,"Probleme d'allocation de memoire\n");
Erreur(point);
}
/* el2007 >> */

  for (x = 0; x < count;x++)
  {
     strcpy(source,Path);
     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);
     handel_Read = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Read == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
        Erreur(point);
       }
      while((Carry = read(handel_Read,Buffer,1024)) != NULL)
       {
        computeMac (macResult, macResultLen, Buffer, Carry, decryptedMacKey);
/* << el2007 */
PlainDataLen += macResultLen;
plainData = (BYTE *) realloc ( plainData, sizeof(BYTE) * PlainDataLen );
if(plainData == NULL)
{
sprintf(errmsg,"Probleme d'allocation de memoire\n");
Erreur(point);
}
/* el2007 >> */
        for (i = 0; i < macResultLen;i++,j++)
            plainData[j] = macResult[i];
       }
     plainData[j] = 0x00;
     close(handel_Read);
  }
   computeMac (macResult, macResultLen, plainData, j, decryptedMacKey);
/* << el2007 */
free( plainData );
/* el2007 >> */
	return 1;
}

