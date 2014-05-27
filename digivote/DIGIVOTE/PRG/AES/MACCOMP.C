#include "..\aes\fullSpec.h"

/* <computeMac>  computes a Message Authentication Code which complies with ISO/IEC 9797-1, algorithm 2, padding method 2. */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>   	  refers to the array of bytes which is at least <resLen> bytes large. */
/*            	  <result> contains the mac value computed on <data> using <key>. */
/* <resLen>   	  specifies the number of bytes needed in the mac value. */
/*           	  <resLen> must be positive and smaller than or equal to 8. */
/* <data>    	  refers to the array of byte containing the information that must be maced. */
/* <dataLen> 	  refers to the number of bytes in the <data> array. */
/*           	  <dataLen> can be any positive integer value. */
/* <key>     	  refers to the secret mac key. */
int
computeMac (unsigned char *result, int resLen, unsigned char *data,
	    unsigned int dataLen, unsigned char *key)
{
  int ret = ret_unknown;
  unsigned int i;
  unsigned int j;
  unsigned char state[MAX_IV_SIZE] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned char lastblock[MAX_IV_SIZE] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  unsigned char secondKey[MAX_IV_SIZE];
  int nofDataBytesInLastBlock = dataLen % MAX_IV_SIZE;
  int nofBlocks = dataLen / MAX_IV_SIZE;
  keyInstance keyInst;		/* state of the encryption engine */
  cipherInstance cipherInst;	/* specifies the encryption mode */

  if ((resLen < 0) || (resLen > (MAX_IV_SIZE / 2))) {
    ret = ret_invalid_output_length;
    goto error;
  }
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, key);
  if (ret < 0) {
    goto error;
  }
  cipherInst.mode = MODE_ECB;
  for (i = 0; i < nofBlocks; i++) {
    for (j = 0; j < MAX_IV_SIZE; j++) {
      state[j] ^= (*data++);
    }
    ret = encryptBlock (state, &cipherInst, &keyInst, state, sizeof (state));
    if (ret < 0) {
      goto error;
    }
  }
  /* last block includes padding bytes */
  memcpy (lastblock, data, nofDataBytesInLastBlock);
  lastblock[nofDataBytesInLastBlock] = 0x80;
  for (j = 0; j < MAX_IV_SIZE; j++) {
    state[j] ^= lastblock[j];
  }
  ret = encryptBlock (state, &cipherInst, &keyInst, state, sizeof (state));
  if (ret < 0) {
    goto error;
  }
  /* final encryption uses a different key. */
  ret = computeHash (secondKey, MAX_IV_SIZE, key, MAX_IV_SIZE);
  if (ret < 0) {
    goto error;
  }
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, secondKey);
  if (ret < 0) {
    goto error;
  }
  ret = encryptBlock (state, &cipherInst, &keyInst, state, sizeof (state));
  if (ret < 0) {
    goto error;
  }
  memcpy (result, state, resLen);
  ret = ret_ok;
error:
  memset (state, 0, sizeof (state));
  memset (lastblock, 0, sizeof (lastblock));
  memset (secondKey, 0, sizeof (secondKey));
  memset (&keyInst, 0, sizeof (keyInst));
  memset (&cipherInst, 0, sizeof (cipherInst));
  return ret;
}
