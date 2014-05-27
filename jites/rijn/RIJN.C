#include "fullSpec.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <dir.h>


#define PlainDataLen   24000
#define CipherDataLen  24000

struct
{
    char nom[14];
}stru_fichier[150];
/* << el2007 */
/* << AP 16/04/2010 
__char Minicodage[] = "2701";
*/

char Minicodage[] = "2703";

/* >> AP 16/04/2010 */

/* el2007 >> */

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
	qsort((void *)stru_fichier, count, 14, sort_function);
}

main(int argc, char *argv[])
{
  point = errmsg;

   strcpy(sourceori,argv[1]);
   strcpy(destineori,argv[2]);
   strcpy(macKey,argv[3]);
   strcpy(encKey,argv[4]);
   strcpy(niveau,"3");
   strcpy(destine,destineori);
   strcat(destine,"\\LOG.ERR");
   unlink(destine);
   Remplit();

   SortFiles();

   /* be_filename */
   strcpy(be_filename, "be.");
   strcat(be_filename, niveau);
   be_filename[4] = 0x00;

   strcpy(source,sourceori);
   strcat(source,"\\");
   strcat(source,be_filename);

   Rfile(source);
   for (i = 0; i < 16;i++)
      iv[i] = wrkspc[i];
   iv[i] = 0x00;

   /* Nouveau IV */
/* << el2007 */
/*
__   iv[0] = Minicodage[0];
__   iv[2] = Minicodage[1];
__   iv[4] = Minicodage[2];
__   iv[6] = Minicodage[3];
*/

/* << AP 16/04/2010 
__   iv[0] = Minicodage[0];
__   iv[3] = Minicodage[1];
__   iv[5] = Minicodage[2];
__   iv[6] = Minicodage[3];
*/

   iv[0] = Minicodage[0];
   iv[2] = Minicodage[1];
   iv[5] = Minicodage[2];
   iv[7] = Minicodage[3];

/* >> AP 16/04/2010 */

/* el2007 >> */
 /***********************/

   Rstr(macKey);
   for (i = 0; i < 16;i++)
      decryptedMacKey[i] = wrkspc[i];
   decryptedMacKey[i] = 0x00;

   Rstr(encKey);
   for (i = 0; i < 16;i++)
      decryptedEncKey[i] = wrkspc[i];
   decryptedEncKey[i] = 0x00;

   makeKeyBinary (&decKeyInst, DIR_DECRYPT, decryptedEncKey);
   cipherInitBinary (&decCipherInst, iv);

   CalculMac(sourceori);

   strcpy(source,sourceori);
   strcat(source,"\\");
   strcat(source,be_filename);
   Rfile(source);


   for (i = 0; i < macResultLen;i++)
    if(macResult[i] != wrkspc[i + 16])
        {
           sprintf(errmsg,"problem CRC.\n");
           Erreur(point);
        }
      Decrypte();
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
  exit(0);
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
}
/*****************************************************************************/
Rstr(char *chaine)
{
  BYTE Bffer[100],C1,C2;
  int i,j,n,Carry;


  strcpy(Bffer,chaine);

  for ( i = 0 ; i <  64 ; i++)
      {
	 if(Bffer[i] < 58)
	      Bffer[i] -= 48;
	  else
	      Bffer[i] -= 55;
      }


  for ( i = 0, j = 0; j < 32 / 2; j++)
      {
           wrkspc[j]   = Bffer[i++] << 4;
           wrkspc[j]   = Bffer[i++] | wrkspc[j];
      }
      wrkspc[j] = 0x00;
}

/******************************************************************************/
/*       C O N T R O L E   D U   C R C    D E S   F I C H I E R S            */
/*             P R E P A R A T I O N    D U    T A B L E A U                 */
Remplit()
{
register int i;
struct ffblk fileinfo;
point = errmsg;
count = 0;
strcpy(source,sourceori);
strcat(source,"\\*.*");

if(strcmp(niveau,"3") == 0)
   {
     strcpy(stru_fichier[count].nom,"TABLE.URN");
     count++;
   }
else
   {
     if(findfirst(source,&fileinfo,0) == -1)
	   {
	     sprintf(errmsg,"Aucun fichier … traiter\n");
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
		   (strcmp(fileinfo.ff_name,"FE_DSK.CRP") !=0)    &&
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

}

/****************************************************************************/
/****************************************************************************/
Decrypte()
{
register int x, n, i, Carry;
int  handel_Read, handel_Write;
BYTE plainData [PlainDataLen];
BYTE cipherData[CipherDataLen];


for (x = 0; x < count;x++)
 {
  if((strcmp(stru_fichier[x].nom,"TABLE.URN") == 0))
   {
     strcpy(source,sourceori);
     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);

     plainData[0] = 0x00;
     cipherData[0] = 0x00;

     handel_Read = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Read == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s\n",source);
	Erreur(point);
       }
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
      if(plainDataLen < 0)
	   {
            sprintf(errmsg,"Erreur de decryptage du fichier %s\n",destine);
	    Erreur(point);
	   }
      Carry = write(handel_Write,plainData,plainDataLen);
      if(Carry != plainDataLen)
	   {
            sprintf(errmsg,"Erreur d'‚criture du fichier %s\n",destine);
	    Erreur(point);
	   }
     close(handel_Write);
   }
 }
}

/****************************************************************************/
CalculMac(char *Path)
{
register int x, n, i, Carry,j;
int  handel_Read;
BYTE plainData [PlainDataLen];
BYTE cipherData[CipherDataLen];
BYTE Buffer [1050];

j = 0;
plainData[j] = 0x00;

for (x = 0; x < count;x++)
  {
     strcpy(source,Path);
     strcat(source,"\\");
     strcat(source,stru_fichier[x].nom);
     handel_Read = open(source, O_RDWR | O_BINARY,0x0100 | 0x0080);
     if(handel_Read == -1)
       {
        sprintf(errmsg,"Erreur d'ouverture du fichier %s -Mac\n",source);
	Erreur(point);
       }
      while((Carry = read(handel_Read,Buffer,1024)) != NULL)
       {
        computeMac (macResult, macResultLen, Buffer, Carry, decryptedMacKey);
        for (i = 0; i < macResultLen;i++,j++)
            plainData[j] = macResult[i];
       }
     plainData[j] = 0x00;
     close(handel_Read);
  }
   computeMac (macResult, macResultLen, plainData, j, decryptedMacKey);
}

