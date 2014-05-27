#include "fullSpec.h"

/* <computeIteratedHash> is a generic hash function with a parameterized security level. */
/* <return code>         an integer, zero means ok, negative means an error occurred. */
/* <result>              refers to the array of bytes which is at least <resLen> bytes large. */
/*                       <result> contains the hash value computed on <data>. */
/* <resLen>              specifies the number of bytes needed in the hash value. */
/*                       <resLen> must be positive and smaller than or equal to 16. */
/* <data>                refers to the array of byte containing the information that must be hashed. */
/* <dataLen>             refers to the number of bytes in the <data> array. */
/* <nofIterations>       specifies the number of times the internal encryption is iterated. */
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
  unsigned char initialState[MAX_IV_SIZE];      /* temporary storage */
  unsigned char *currentBlock = data;   /* points to the 16-byte block that is processed */
  int nofBlocks = dataLen / MAX_IV_SIZE;        /* number of full 16-byte blocks */
  int nofRemainingBytes = dataLen - MAX_IV_SIZE * nofBlocks;    /* number of data bytes for the paddedBlock */
  unsigned char paddedBlock[MAX_IV_SIZE];       /* block containing the last data bytes, padded with zero-bytes to fill the one but last block */
  unsigned char lastBlock[MAX_IV_SIZE]; /* last block formatted as specified in the standard */
  keyInstance keyInst;          /* state of the encryption engine */
  cipherInstance cipherInst;    /* specifies the encryption mode */
  if ((resLen < 0) || (resLen > MAX_IV_SIZE))
    {
      ret = ret_invalid_o_length;
      goto error;
    }
  cipherInst.mode = MODE_ECB;
  for (i = 0; i < nofBlocks; i++)
    {
      memcpy (initialState, state, sizeof (state));
      ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, currentBlock);
      if (ret < 0)
	{
	  goto error;
	}
      for (j = 0; j < nofIterations; j++)
	{
	  ret =
	    blockEncrypt (&cipherInst, &keyInst, state,
			  nofBitsInByte * sizeof (state), state);
	  if (ret < 0)
	    {
	      goto error;
	    }
	}
      for (j = 0; j < MAX_IV_SIZE; j++)
	{
	  state[j] ^= currentBlock[j] ^ initialState[j];
	}
      currentBlock += MAX_IV_SIZE;
    }

/* one but last block consists of the databytes padded with zero-bytes to the right */
  if (nofRemainingBytes > 0)
    {
      nofBlocks++;
      memset (paddedBlock, 0, sizeof (paddedBlock));
      memcpy (paddedBlock, currentBlock, nofRemainingBytes);
      memcpy (initialState, state, sizeof (state));
      ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, paddedBlock);
      if (ret < 0)
	{
	  goto error;
	}
      for (j = 0; j < nofIterations; j++)
	{
	  ret =
	    blockEncrypt (&cipherInst, &keyInst, state,
			  nofBitsInByte * sizeof (state), state);
	  if (ret < 0)
	    {
	      goto error;
	    }
	}
      for (j = 0; j < MAX_IV_SIZE; j++)
	{
	  state[j] ^= paddedBlock[j] ^ initialState[j];
	}
    }

/* append last 16-byte block, which consists of 0x55||0x00||0x55||0x55||..||0x55||0x55||nofBlocks */
/* nofBlocks is specified in big endian format, i.e., left most byte is the most significant byte */
  memset (lastBlock, 0x55, sizeof (lastBlock));
  lastBlock[1] = 0;
  i = MAX_IV_SIZE;
  j = nofBlocks;

  do
    {
      lastBlock[--i] = j % 256;
      j /= 256;
    }
  while (j > 0);
  memcpy (initialState, state, sizeof (state));
  ret = makeKeyBinary (&keyInst, DIR_ENCRYPT, lastBlock);
  if (ret < 0)
    {
      goto error;
    }
  for (j = 0; j < nofIterations; j++)
    {
      ret =
	blockEncrypt (&cipherInst, &keyInst, state,
		      nofBitsInByte * sizeof (state), state);
      if (ret < 0)
	{
	  goto error;
	}
    }
  for (j = 0; j < MAX_IV_SIZE; j++)
    {
      state[j] ^= lastBlock[j] ^ initialState[j];
    }
  memcpy (result, state, resLen);
  ret = ret_ok;
error:memset (state, 0, sizeof (state));
  memset (initialState, 0, sizeof (initialState));
  memset (lastBlock, 0, sizeof (lastBlock));
  memset (&keyInst, 0, sizeof (keyInst));
  memset (&cipherInst, 0, sizeof (cipherInst));
  return ret;
}


/* <computeHash> is a multi-purpose hash function.  its security level is specified through the symbol <hashSecurityLevel>. */
/* <return code> an integer, zero means ok, negative values refer to an error. */
/* <result>       refers to the array of bytes which is at least <resLen> bytes large. */
/*                <result> contains the hash value computed on <data>. */
/* <resLen>       specifies the number of bytes needed in the hash value. */
/*               <resLen> must be positive and smaller than or equal to 16. */
/* <data>         refers to the array of byte containing the information that must be hashed. */
/* <dataLen>      refers to the number of bytes in the <data> array. */
int
computeHash (unsigned char *result, int resLen, unsigned char *data,
	     unsigned int dataLen)
{
  return computeIteratedHash (result, resLen, data, dataLen,
			      hashSecurityLevel);
}


/* <computeSimplifiedHash> is a multi-purpose hash function with the lowest number of iterations possible. */
/* <return code>           an integer, zero means ok, negative values refer to an error. */
/* <result>                refers to the array of bytes which is at least <resLen> bytes large. */
/*                         <result> contains the hash value computed on <data>. */
/* <resLen>                 specifies the number of bytes needed in the hash value. */
/*                         <resLen> must be positive and smaller than or equal to 16. */
/* <data>                  refers to the array of byte containing the information that must be hashed. */
/* <dataLen>                refers to the number of bytes in the <data> array. */
int
computeSimplifiedHash (unsigned char *result, int resLen, unsigned char *data,
		       unsigned int dataLen)
{
  return computeIteratedHash (result, resLen, data, dataLen, 1);
}
