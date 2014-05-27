#include "fullSpec.h"

/* <encryptAndMac> transforms the plaintext information <plainData> into ciphertext and computes the corresponding MAC value. */
/* <return code>   an integer, negative values refer to an error code, positive values specify the length of the authenticated ciphertext. */
/* <authenticatedCiphertext> refers to the encrypted plaintext and the MAC value. */
/*                           <authenticatedCiphertext> must be able to accomodate at least <dataLen>+<blockLength>+<blockLength/2> bytes. */
/* <cipherKey> 	   refers to the 16-byte encryption key. */
/* <iv> 	   refers to the 16-byte initialization vector for the encryption. */
/* <macKey> 	   refers to the 16-byte MAC-key. */
/* <plainData> 	   refers to plaintext information that must be protected. */
/* <plainDataLen>  specifies the length of the plaintext information. */
int
encryptAndMac (unsigned char *authenticatedCiphertext,
	       unsigned char *cipherKey, unsigned char *iv,
	       unsigned char *macKey, unsigned char *data,
	       unsigned long dataLen)
{
  int ret = ret_unknown;
  keyInstance encKeyInst;	/* state of the encryption engine */
  cipherInstance encCipherInst;	/* specifies the encryption mode */
  int cipherDataLen;
  ret = makeKeyBinary (&encKeyInst, DIR_ENCRYPT, cipherKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = cipherInitBinary (&encCipherInst, iv);
  if (ret < 0)
    {
      goto error;
    }
  ret =
    encryptData (authenticatedCiphertext, &encCipherInst, &encKeyInst, data,
		 dataLen);
  cipherDataLen = ret;
  if (ret < 0)
    {
      printf ("encryption routine returns error code <%i>", ret);
      goto error;
    }
  ret =
    computeMac (authenticatedCiphertext + cipherDataLen, macResultLen,
		authenticatedCiphertext, cipherDataLen, macKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = cipherDataLen + macResultLen;
error:memset (&encCipherInst, 0, sizeof (encCipherInst));
  memset (&encKeyInst, 0, sizeof (encKeyInst));
  return ret;
}


/* <validateMacAndDecrypt> validates the MAC on the encrypted information, and decrypts the information if possible. */
/* <return code>   an integer specifying the length of the plaintext if this was produced, negative values refer to an error code. */
/* <plaintext> refers to the decrypted information if this was produced. */
/*                           <plaintext> must be able to accomodate at most <authenticatedCiphertext> bytes. */
/* <cipherKey> 	   refers to the 16-byte decryption key. */
/* <iv> 	   refers to the 16-byte initialization vector for the decryption. */
/* <macKey> 	   refers to the 16-byte MAC-key. */
/* <authenticatedCiphertext>  	   refers to the authenticated ciphertext. */
/* <authenticatedCiphertextLength> specifies the number of bytes used for the authenticated ciphertext. */
int
validateMacAndDecrypt (unsigned char *plaintext, unsigned char *cipherKey,
		       unsigned char *iv, unsigned char *macKey,
		       unsigned char *authenticatedCiphertext,
		       unsigned long authenticatedCiphertextLength)
{
  int ret = ret_unknown;
  keyInstance decKeyInst;	/* state of the decryption engine */
  cipherInstance decCipherInst;	/* specifies the decryption mode */
  int cipherDataLen;
  ret =
    validateMac (authenticatedCiphertext + authenticatedCiphertextLength -
		 macResultLen, macResultLen, authenticatedCiphertext,
		 authenticatedCiphertextLength - macResultLen, macKey);
  if (ret != ret_ok)
    {
      ret = ret_invalid_mac;
      goto error;
    }
  ret = makeKeyBinary (&decKeyInst, DIR_DECRYPT, cipherKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = cipherInitBinary (&decCipherInst, iv);
  if (ret < 0)
    {
      goto error;
    }
  cipherDataLen =
    decryptData (plaintext, &decCipherInst, &decKeyInst,
		 authenticatedCiphertext,
		 authenticatedCiphertextLength - macResultLen);
  if (cipherDataLen < 0)
    {
      printf ("decryption routine returns error code <%i>", cipherDataLen);
      goto error;
    }
  ret = cipherDataLen;
error:memset (&decCipherInst, 0, sizeof (decCipherInst));
  memset (&decKeyInst, 0, sizeof (decKeyInst));
  return ret;
}
