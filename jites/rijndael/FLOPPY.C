#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <dos.h>
#include <dir.h>

#include "fullSpec.h"
#include "alg-fst.c"
#include "api-fst.c"
#include "encdec.c"
#include "enckeys.c"
#include "encmac.c"
#include "hashcomp.c"
#include "maccomp.c"
#include "psswd.c"
#define plainDataLen (10 * 1024)

unsigned long cipherDataLen;
unsigned long decryptedLen;
float nofSeconds = 10;
char *point;
BYTE path[60],pathori[60],chemin[60],cheminori[60], errmsg[100];
BYTE wrkspc[60];
BYTE fullPassword[fullPasswordLength + 1];

int ret;                      /* holds the return code of important method calls. */
unsigned long matches;        /* counts the number of equal bytes in the plaintext and the decrypted ciphertext. */
unsigned long i;              /* counter to produce nice output */
keyInstance encKeyInst;       /* state of the encryption engine */
cipherInstance encCipherInst; /* specifies the encryption mode */
keyInstance decKeyInst;       /* state of the decryption engine */
cipherInstance decCipherInst; /* specifies the decryption mode */

BYTE cipherKey[cipherKeyLength + 1];
BYTE macKey[cipherKeyLength + 1];

BYTE encKey[cipherKeyLength + 1];
BYTE iv[cipherKeyLength + 1];

BYTE *plainData = NULL;
BYTE *cipherData = NULL;
BYTE derivedKey[cipherKeyLength + 1 ];
BYTE decryptedMacKey[cipherKeyLength + 1];
BYTE decryptedEncKey[cipherKeyLength + 1];
BYTE *decrypted = NULL;
BYTE hashResult[blockLength + 1];
BYTE macResult[macResultLen + 1];
BYTE *authenticatedEncryptedPlaintext;
int hashResultLen = sizeof (hashResult);
int dataForFloppyLength;
BYTE dataForFloppy[3 * blockLength + 1];
BYTE dataFromFloppy[3 * blockLength + 1];
BYTE mode[2];


main(int argc, char *argv[])
{
  point = errmsg;

  strcpy(mode,argv[1]);
  strcpy(fullPassword,argv[2]);
  strcpy(pathori,argv[3]);  
  strcpy(cheminori,argv[4]);

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
  strcpy(cipherKey,derivedKey);

  if (ret != 0)
    {
       sprintf(errmsg,"cipherKey derivation fails with error <%i>\n",ret);
       Erreur(point);
    }



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
          Rfile("mac.be");
          strcpy(macKey,wrkspc);

          Rfile("enc.be");
          strcpy(encKey,wrkspc);

          Rfile("ivcle.be");
          strcpy(iv,wrkspc);

          ret = encryptKeys (dataForFloppy, derivedKey, iv, macKey, encKey);
	  if (ret < 0)
	    {
	       sprintf(errmsg,"problem during encryptKeys: <%i>.\n", ret);      
	       Erreur(point);
	    }

	  dataForFloppyLength = ret;
	  Wfloppy(dataForFloppy,ret);
          exit(0);
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
         Rfloppy(dataFromFloppy);
         ret= decryptKeys (decryptedMacKey, decryptedEncKey, dataFromFloppy, derivedKey);

	 if (ret < 0)
	    {   
	       sprintf(errmsg,"problem during decryptKeys: <%i>.\n", ret);
	       Erreur(point);   
	    }   

       Wmac();
       Wenc();
       exit(0);
    }
}
/****************************************************************************/
/*             F I C H I E R     D E S    E R R E U R S                     */

Erreur(char *msg)
{
  FILE *fps;
	
  strcpy(chemin,cheminori);
  strcat(chemin,"\\LOG.ERR");
  fps = fopen(chemin,"a");
  fputs(msg,fps);
  fclose(fps);
  exit(0);
}
/****************************************************************************/
/****************************************************************************/
/*                            F I C H I E R S                               */

Wfloppy(char *msg,int ret)
{
  int fd;
	
  strcpy(chemin,cheminori);
  strcat(chemin,"\\floppy.be");
  
  if((fd = open(chemin,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening Floppy.be");
	  Erreur(point);
    }

  write(fd,msg,ret);
  close(fd);
  exit(0);
}

Rfloppy(char *msg)
{
  int fd;
	
  strcpy(path,pathori);
  strcat(path,"\\Floppy.be");

  if((fd = open( path, O_BINARY | O_RDWR)) == -1)
    {
	  sprintf(errmsg,"error opening Floppy.be");
	  Erreur(point);
    }
  read(fd,msg,80);
  close(fd);

}

/****************************************************************************/

Rfile(char *fichier)
{
  FILE *fps;
  BYTE Bffer[cipherKeyLength * 2],C1,C2;
  int i,j;
	  
  strcpy(path,pathori);
  strcat(path,"\\");
  strcat(path,fichier);

  if ((fps = fopen(path,"r")) == NULL)
    {
	  sprintf(errmsg,"error opening %s",fichier);
	  Erreur(point);
    }
  
  fgets(Bffer,34,fps);
  fclose(fps);

  for ( i = 0 ; i < (cipherKeyLength * 2); i++)
      {
	 if(Bffer[i] < 58)
	      Bffer[i] -= 48;
	  else
	      Bffer[i] -= 55;
      }


  for ( i = 0, j = 0; j < cipherKeyLength ; j++)
      {
		
           wrkspc[j]   = Bffer[i++] << 4;
           wrkspc[j]   = Bffer[i++] | wrkspc[j];
      }
      wrkspc[j] = 0x00;
}


/*****************************************************************************/
Wmac()
{
  int fd;
  int i;
  BYTE Bffer[cipherKeyLength * 3];
  BYTE Cai[3];

  strcpy(chemin,cheminori);
  strcat(chemin,"\\mac.be");
  
  if((fd = open(chemin,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening mac.be");
	  Erreur(point);
    }

  Bffer[0] = 0x00;
  
  for(i = 0; i < cipherKeyLength;i++)
        {
          sprintf(Cai, "%02X",decryptedMacKey[i]);
          strcat(Bffer,Cai);
        }
  write(fd,Bffer,cipherKeyLength * 2);
  close(fd);
}
/*****************************************************************************/

Wenc()
{
  int fd;
  int i;
  BYTE Bffer[cipherKeyLength * 3];
  BYTE Cai[3];
	
  strcpy(chemin,cheminori);
  strcat(chemin,"\\enc.be");
  
  if((fd = open(chemin,O_CREAT | O_RDWR | O_BINARY,0x0100 | 0x0080)) == -1)
    {
	  sprintf(errmsg,"error opening enc.be");
	  Erreur(point);
    }

  Bffer[0] = 0x00;

  for(i = 0; i < cipherKeyLength;i++)
        {
          sprintf(Cai, "%02X",decryptedEncKey[i]);
          strcat(Bffer,Cai);
        }
  write(fd,Bffer,cipherKeyLength * 2);
  close(fd);
}

