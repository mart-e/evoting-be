#include "fullSpec.h"

/* <cipherCFBInitBinary> initializes the state of the encryption/decryption engine */
/* <return code>      an integer, zero means ok, negative means an error occurred. */
/* <cipherInst>       refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <IV>               refers to an array of 16 bytes containing the initialization vector for CFB-mode */
int
cipherCFBInitBinary (cipherInstance * cipherInst, unsigned char *IV)
{
  cipherInst->mode = MODE_CFB1;
  if (IV != NULL)
    {
      int i;
      for (i = 0; i < MAX_IV_SIZE; i++)
	{
	  cipherInst->IV[i] = (u8) * IV++;
	}
    }

  else
    {
      memcpy (cipherInst->IV, 0, MAX_IV_SIZE);
    }
  return ret_ok;
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
int
encryptKeys (BYTE * dataForFloppy, BYTE * cipherKey, BYTE * iv,
	     BYTE * macKey, BYTE * encKey)
{
  int ret = ret_unknown;
  keyInstance encKeyInst;       /* state of the encryption engine */
  cipherInstance encCipherInst; /* specifies the encryption mode */
  BYTE plainData[2 * cipherKeyLength];
  BYTE cipherData[2 * cipherKeyLength];
  memcpy (plainData, macKey, cipherKeyLength);
  memcpy (plainData + cipherKeyLength, encKey, cipherKeyLength);
  ret = makeKeyBinary (&encKeyInst, DIR_ENCRYPT, cipherKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = cipherCFBInitBinary (&encCipherInst, iv);
  if (ret < 0)
    {
      goto error;
    }
  ret =
    blockEncrypt (&encCipherInst, &encKeyInst, plainData,
		  nofBitsInByte * sizeof (plainData), cipherData);
  if (ret < 0)
    {
      goto error;
    }
  if (ret != nofBitsInByte * 2 * cipherKeyLength)
    {
      ret = ret_invalid_e_datalength;
      goto error;
    }
  memcpy (dataForFloppy, iv, blockLength);
  memcpy (dataForFloppy + blockLength, cipherData, sizeof (cipherData));
  ret = blockLength + sizeof (cipherData);
error:memset (&encCipherInst, 0, sizeof (encCipherInst));
  memset (&encKeyInst, 0, sizeof (encKeyInst));
  memset (cipherData, 0, sizeof (cipherData));
  memset (plainData, 0, sizeof (plainData));
  return ret;
}


/* <decryptKeys> decrypts the data retrieved from floppy <dataFromFloppy>. */
/* <return code> an integer, negative values refer to an error code, zero means that no problem was detected.. */
/*               note that zero does not indicate that the decrypted keys are valid: no redundancy is added to avoid exhaustive key searches. */
/* <macKey>      refers to the 16-byte MAC-key that has been decrypted. */
/* <encKey>      refers to the 16-byte encryption key that has been decrypted. */
/* <dataFromFloppy> refers to an array of char that contains the information that was stored on a floppy disk. */
/*                 <dataFromFloppy> must consist of <3*<blockLength>> bytes. */
/* <cipherKey>     refers to the 16-byte encryption key with which the <dataFromFloppy> will be decrypted. */
int
decryptKeys (BYTE * decryptedMacKey, BYTE * decryptedEncKey,
	     BYTE * dataFromFloppy, BYTE * cipherKey)
{
  int ret = ret_unknown;
  keyInstance decKeyInst;       /* state of the decryption engine */
  cipherInstance decCipherInst; /* specifies the decryption mode */
  int decryptedDataLen;
  BYTE decryptedData[2 * cipherKeyLength];
  ret = makeKeyBinary (&decKeyInst, DIR_DECRYPT, cipherKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = cipherCFBInitBinary (&decCipherInst, dataFromFloppy);
  if (ret < 0)
    {
      goto error;
    }
  decryptedDataLen =
    blockDecrypt (&decCipherInst, &decKeyInst, dataFromFloppy + blockLength,
		  nofBitsInByte * 2 * cipherKeyLength, decryptedData);
  if (decryptedDataLen < 0)
    {
      goto error;
    }
  if (decryptedDataLen != nofBitsInByte * 2 * cipherKeyLength)
    {
      ret = ret_invalid_d_datalength;
      goto error;
    }
  memcpy (decryptedMacKey, decryptedData, cipherKeyLength);
  memcpy (decryptedEncKey, decryptedData + cipherKeyLength, cipherKeyLength);
  ret = ret_ok;
error:memset (&decCipherInst, 0, sizeof (decCipherInst));
  memset (&decKeyInst, 0, sizeof (decKeyInst));
  memset (decryptedData, 0, sizeof (decryptedData));
  return ret;
}
