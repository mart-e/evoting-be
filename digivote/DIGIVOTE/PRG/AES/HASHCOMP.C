#include "..\aes\fullSpec.h"

/* <computeIteratedHash> is a generic hash function with a parameterized security level. */
/* <return code>   an integer, zero means ok, negative means an error occurred. */
/* <result>        refers to the array of bytes which is at least <resLen> bytes large. */
/*                 <result> contains the hash value computed on <data>. */
/* <resLen>        specifies the number of bytes needed in the hash value. */
/*                 <resLen> must be positive and smaller than or equal to 16. */
/* <data>          refers to the array of byte containing the information that must be hashed. */
/* <dataLen>       refers to the number of bytes in the <data> array. */
/*                 <dataLen> must be a multiple of 16. */
/* <nofIterations> specifies the number of times the internal encryption is iterated. */
int
computeIteratedHash (unsigned char *result, int resLen, unsigned char *data,
		     unsigned int dataLen, int nofIterations)
{
  int ret = ret_unknown;
  unsigned int i;
  unsigned int j;
  unsigned char state[MAX_IV_SIZE] =
    { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98,
    0x76, 0x54, 0x32, 0x10
  };
  unsigned char nullBlock[MAX_IV_SIZE];
  unsigned char initialState[MAX_IV_SIZE];
  unsigned char lastBlock[MAX_IV_SIZE];
  unsigned char *currentBlock = data;
  int nofBlocks = dataLen / MAX_IV_SIZE;
  keyInstance keyInst;		/* state of the encryption engine */
  cipherInstance cipherInst;	/* specifies the encryption mode */

  if ((resLen < 0) || (resLen > MAX_IV_SIZE)) {
    ret = ret_invalid_output_length;
    goto error;
  }
  if (dataLen % MAX_IV_SIZE != 0) {
    ret = ret_invalid_data_length;
    goto error;
  }
  cipherInst.mode = MODE_ECB;
  memset (nullBlock, 0, sizeof (nullBlock));
  for (i = 0; i < nofBlocks; i++) {
    memcpy (initialState, state, sizeof (state));
    ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, currentBlock);
    if (ret < 0) {
      goto error;
    }
    for (j = 0; j < nofIterations; j++) {
      ret =
	blockEncrypt (&cipherInst, &keyInst, state,
		      nofBitsInByte * sizeof (state), state);
      if (ret < 0) {
	goto error;
      }
    }
    for (j = 0; j < MAX_IV_SIZE; j++) {
      state[j] ^= currentBlock[j] ^ initialState[j];
    }
    currentBlock += MAX_IV_SIZE;
  }
/* append last 16-byte block, which consists of 0x55||0x00||0x55||0x55||..||0x55||0x55||nofBlocks */
/* nofBlocks is specified in big endian format, i.e., left most byte is the most significant byte */
  memset (lastBlock, 0x55, sizeof (lastBlock));
  lastBlock[1] = 0;
  i = MAX_IV_SIZE;
  j = nofBlocks;
  do {
    lastBlock[--i] = j % 256;
    j /= 256;
  } while (j > 0);
  memcpy (initialState, state, sizeof (state));
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, lastBlock);
  if (ret < 0) {
    goto error;
  }
  for (j = 0; j < nofIterations; j++) {
    ret =
      blockEncrypt (&cipherInst, &keyInst, state,
		    nofBitsInByte * sizeof (state), state);
    if (ret < 0) {
      goto error;
    }
  }
  for (j = 0; j < MAX_IV_SIZE; j++) {
    state[j] ^= lastBlock[j] ^ initialState[j];
  }
  memcpy (result, state, resLen);
  ret = ret_ok;
error:
  memset (state, 0, sizeof (state));
  memset (initialState, 0, sizeof (initialState));
  memset (lastBlock, 0, sizeof (lastBlock));
  memset (&keyInst, 0, sizeof (keyInst));
  memset (&cipherInst, 0, sizeof (cipherInst));
  return ret;
}

/* <computeHash> is a multi-purpose hash function. */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>  	 refers to the array of bytes which is at least <resLen> bytes large. */
/*           	 <result> contains the hash value computed on <data>. */
/* <resLen>  	 specifies the number of bytes needed in the hash value. */
/*           	 <resLen> must be positive and smaller than or equal to 16. */
/* <data>    	 refers to the array of byte containing the information that must be hashed. */
/* <dataLen> 	 refers to the number of bytes in the <data> array. */
/*           	 <dataLen> must be a multiple of 16. */
int
computeHash (unsigned char *result, int resLen, unsigned char *data,
	     unsigned int dataLen)
{
  return computeIteratedHash (result, resLen, data, dataLen, 1);
}
