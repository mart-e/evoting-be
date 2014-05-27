#include "fullSpec.h"

/* <makeKeyBinary> transforms the binary representation of a 16-byte encryption/decryption key into the form required by the Rijndael algorithm. */
/* <return code>   an integer, zero means ok, negative means an error occurred. */
/* <keyInst> 	   refers to the state of the encryption engine: encryption/decryption */
/* <direction> 	   specifies whether the engine will encrypt or decrypt */
/* <cipherKey> 	   refers to the 16-byte key value */
int
makeKeyBinary (keyInstance * keyInst, BYTE direction,
	       unsigned char *cipherKey)
{
  int ret = ret_unknown;
  if (keyInst == NULL) {
    ret = BAD_KEY_INSTANCE;
    goto error;
  }
  if ((direction == DIR_ENCRYPT) || (direction == DIR_DECRYPT)) {
    keyInst->direction = direction;
  }
  else {
    ret = BAD_KEY_DIR;
    goto error;
  }
  keyInst->keyLen = 128;

  /* initialize key schedule: */
  if (direction == DIR_ENCRYPT) {
    keyInst->Nr =
      rijndaelKeySetupEnc (keyInst->rk, (u8 *) cipherKey, keyInst->keyLen);
  }
  else {
    keyInst->Nr =
      rijndaelKeySetupDec (keyInst->rk, (u8 *) cipherKey, keyInst->keyLen);
  }
  ret = rijndaelKeySetupEnc (keyInst->ek, (u8 *) cipherKey, keyInst->keyLen);
  if (ret < 0) {
    goto error;
  }
  ret = ret_ok;
error:
  return ret;
}

/* <cipherInitBinary> initializes the state of the encryption/decryption engine */
/* <return code>      an integer, zero means ok, negative means an error occurred. */
/* <cipherInst>       refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <IV> 	      refers to an array of 16 bytes containing the initialization vector for CBC-mode */
int cipherInitBinary (cipherInstance * cipherInst, unsigned char *IV)
{
  int ret = ret_unknown;
  cipherInst->mode = MODE_CBC;
  if (IV != NULL) {
    int i;
    for (i = 0; i < MAX_IV_SIZE; i++) {
      cipherInst->IV[i] = (u8) * IV++;
    }
  }
  else {
    memcpy (cipherInst->IV, 0, MAX_IV_SIZE);
  }
  ret = ret_ok;
error:
  return ret;
}

/* <encryptBlock> encrypts a multiple of 16 bytes. */
/* <return code>  an integer, zero means ok, negative means an error occurred. */
/* <result>  	  refers to the array of bytes which contains the ciphertext. */
/*                it is at least <plainLen> bytes large. */
/* <cipherInst>   refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <keyInst>      refers to the encryption key, which is initialized via makeKeyBinary. */
/* <plaintext>    refers to the array of byte containing the information that must be encrypted. */
/* <plainLen>     refers to the number of bytes in the <plaintext>. */
/*                <plainLen> must be a multiple of 16. */
int
encryptBlock (unsigned char *result, cipherInstance * cipherInst,
	      keyInstance * keyInst, unsigned char *plaintext,
	      unsigned long plainLen)
{
  return blockEncrypt (cipherInst, keyInst, plaintext,
		       ( int) (nofBitsInByte * plainLen), result);
}

/* <decryptBlock> decrypts a multiple of 16 bytes. */
/* <return code>  an integer, zero means ok, negative means an error occurred. */
/* <result>	  refers to the array of bytes which contains the decrypted information. */
/*           	  it is at least <cipherLen> bytes large. */
/* <cipherInst>   refers to the mode of decryption, which is initialized via cipherInitBinary. */
/* <keyInst>	  refers to the decryption key, which is initialized via makeKeyBinary. */
/* <ciphertext>   refers to the array of byte containing the information that must be decrypted. */
/* <cipherLen>	  refers to the number of bytes in the <ciphertext>. */
/*           	  <cipherLen> must be a multiple of 16. */
int
decryptBlock (unsigned char *result, cipherInstance * cipherInst,
	      keyInstance * keyInst, unsigned char *ciphertext,
	      unsigned long cipherLen)
{
  return blockDecrypt (cipherInst, keyInst, ciphertext,
		       ( int) (nofBitsInByte * cipherLen), result);
}
