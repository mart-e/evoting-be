#include "fullSpec.h"

/* <encryptBlock> encrypts a 16-byte block. */
/* <return code>  an integer, zero means ok, negative refers to an error. */
/* <result>        refers to the array of bytes which will contain the ciphertext. */
/*                this array must at least be able to accomodate 16 bytes. */
/* <cipherInst>   refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <keyInst>      refers to the encryption key, which is initialized via makeKeyBinary. */
/* <plaintext>    refers to the array of byte containing 16 plaintext bytes. */
int
encryptBlock (unsigned char *result, cipherInstance * cipherInst,
	      keyInstance * keyInst, unsigned char *plaintext)
{
  return blockEncrypt (cipherInst, keyInst, plaintext, nofBitsInByte * 16,
		       result);
}

/* <computeMac>  computes a Message Authentication Code which complies with ISO/IEC 9797-1, algorithm 2, padding method 2. */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>       refers to the array of bytes which is at least <resLen> bytes large. */
/*                <result> contains the mac value computed on <data> using <key>. */
/* <resLen>       specifies the number of bytes needed in the mac value. */
/*                <resLen> must be positive and smaller than or equal to 8. */
/* <data>         refers to the array of byte containing the information that must be maced. */
/* <dataLen>      refers to the number of bytes in the <data> array. */
/*                <dataLen> can be any positive integer value. */
/* <key>          refers to the secret mac key. */
int
computeMac (unsigned char *result, int resLen, unsigned char *data,
	    unsigned int dataLen, unsigned char *key)
{
  int ret = ret_unknown;
  unsigned int i;
  unsigned int j;
  unsigned char state[blockLength] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned char lastblock[blockLength] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned char secondKey[blockLength];
  int nofDataBytesInLastBlock = dataLen % blockLength;
  int nofBlocks = dataLen / blockLength;
  keyInstance keyInst;          /* state of the encryption engine */
  cipherInstance cipherInst;    /* specifies the encryption mode */
  if ((resLen < 0) || (resLen > (macResultLen)))
    {
      ret = ret_invalid_o_length;
      goto error;
    }
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, key);
  if (ret < 0)
    {
      goto error;
    }
  cipherInst.mode = MODE_ECB;
  for (i = 0; i < nofBlocks; i++)
    {
      for (j = 0; j < blockLength; j++)
	{
	  state[j] ^= (*data++);
	}
      ret = encryptBlock (state, &cipherInst, &keyInst, state);
      if (ret < 0)
	{
	  goto error;
	}
    }

  /* last block includes padding bytes */
  memcpy (lastblock, data, nofDataBytesInLastBlock);
  lastblock[nofDataBytesInLastBlock] = 0x80;
  for (j = 0; j < blockLength; j++)
    {
      state[j] ^= lastblock[j];
    }
  ret = encryptBlock (state, &cipherInst, &keyInst, state);
  if (ret < 0)
    {
      goto error;
    }

  /* final encryption uses a different key. */
  ret = computeHash (secondKey, blockLength, key, blockLength);
  if (ret < 0)
    {
      goto error;
    }
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, secondKey);
  if (ret < 0)
    {
      goto error;
    }
  ret = encryptBlock (state, &cipherInst, &keyInst, state);
  if (ret < 0)
    {
      goto error;
    }
  memcpy (result, state, resLen);
  result[resLen] = 0x00;
  ret = ret_ok;
error:memset (state, 0, sizeof (state));
  memset (lastblock, 0, sizeof (lastblock));
  memset (secondKey, 0, sizeof (secondKey));
  memset (&keyInst, 0, sizeof (keyInst));
  memset (&cipherInst, 0, sizeof (cipherInst));
  return ret;
}

/* <validateMac>  checks whether the <referenceMAC> corresponds to the give <data> and <key>. */
/* <return code>  an integer, zero means ok, other values refer to an error. */
/* <reference>    refers to the reference MAC value. */
/* <referenceLen> specifies the length of the reference MAC value. */
/* <data>         refers to the array of byte containing the information that must be validated. */
/* <dataLen>      refers to the number of bytes in the <data> array. */
/* <key>          refers to the secret mac key. */
int
validateMac (unsigned char *reference, int referenceLen, unsigned char *data,
	     unsigned int dataLen, unsigned char *key)
{
  int ret = ret_unknown;
  unsigned char computedMAC[blockLength];
  ret = computeMac (computedMAC, referenceLen, data, dataLen, key);
  if (ret < 0)
    {
      goto error;
    }
  ret = memcmp (computedMAC, reference, referenceLen);
error:memset (computedMAC, 0, sizeof (computedMAC));
  return ret;
}
