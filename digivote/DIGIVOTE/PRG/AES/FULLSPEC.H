#ifndef __FULL_H__
#define __FULL_H__
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "..\aes\api-fst.h"

#define ret_unknown -99		/* negative return code indicates an error */
#define ret_invalid_key_length -98
#define ret_invalid_data_length -97
#define ret_invalid_output_length -96
#define ret_malloc_problem -95
#define ret_ok 0		/* zero indicates successful execution */
#define nofBitsInByte 8		/* the number of bits in a single byte */

#define max(a,b)((a<b)?b:a)
#define min(a,b)((a<b)?a:b)

/* <makeKeyBinary> transforms the binary representation of a 16-byte encryption/decryption key into the form required by the Rijndael algorithm. */
/* <return code>   an integer, zero means ok, negative means an error occurred. */
/* <keyInst>       refers to the state of the encryption engine: encryption/decryption */
/* <direction>     specifies whether the engine will encrypt or decrypt */
/* <cipherKey>     refers to the 16-byte key value */
int makeKeyBinary (keyInstance * keyInst, BYTE direction,
		   unsigned char *cipherKey);

/* <cipherInitBinary> initializes the state of the encryption/decryption engine */
/* <return code>      an integer, zero means ok, negative means an error occurred. */
/* <cipherInst>       refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <IV>               refers to an array of 16 bytes containing the initialization vector for CBC-mode */
int cipherInitBinary (cipherInstance * cipherInst, unsigned char *IV);

/* <encryptBlock> encrypts a multiple of 16 bytes. */
/* <return code>  an integer, zero means ok, negative means an error occurred. */
/* <result>       refers to the array of bytes which contains the ciphertext. */
/*                it is at least <plainLen> bytes large. */
/* <cipherInst>   refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <keyInst>      refers to the encryption key, which is initialized via makeKeyBinary. */
/* <plaintext>    refers to the array of byte containing the information that must be encrypted. */
/* <plainLen>     refers to the number of bytes in the <plaintext>. */
/*                <plainLen> must be a multiple of 16. */
int encryptBlock (unsigned char *result, cipherInstance * cipherInst,
		  keyInstance * keyInst, unsigned char *plaintext,
		  unsigned long plainLen);

/* <decryptBlock> decrypts a multiple of 16 bytes. */
/* <return code>  an integer, zero means ok, negative means an error occurred. */
/* <result>	  refers to the array of bytes which contains the decrypted information. */
/*           	  it is at least <cipherLen> bytes large. */
/* <cipherInst>   refers to the mode of decryption, which is initialized via cipherInitBinary. */
/* <keyInst>	  refers to the decryption key, which is initialized via makeKeyBinary. */
/* <ciphertext>   refers to the array of byte containing the information that must be decrypted. */
/* <cipherLen>	  refers to the number of bytes in the <ciphertext>. */
/*           	  <cipherLen> must be a multiple of 16. */
int decryptBlock (unsigned char *result, cipherInstance * cipherInst,
		  keyInstance * keyInst, unsigned char *ciphertext,
		  unsigned long cipherLen);

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
int computeIteratedHash (unsigned char *result, int resLen,
			 unsigned char *data, unsigned int dataLen,
			 int nofIterations);

/* <computeHash> is a multi-purpose hash function. */
/* <result>      refers to the array of bytes which is at least <resLen> bytes large. */
/*               <result> contains the hash value computed on <data>. */
/* <resLen>      specifies the number of bytes needed in the hash value. */
/*               <resLen> must be positive and smaller than or equal to 16. */
/* <data>        refers to the array of byte containing the information that must be hashed. */
/* <dataLen>     refers to the number of bytes in the <data> array. */
/*               <dataLen> must be a multiple of 16. */
int computeHash (unsigned char *result, int resLen, unsigned char *data,
		 unsigned int dataLen);

/* <computeKeyGeneratorHash> is a hash function designed to produce master keys and session keys. */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>  	 refers to the array of bytes which is at least <resLen> bytes large. */
/*               <result> contains the hash value computed on <data>. */
/* <resLen>      specifies the number of bytes needed in the hash value. */
/*               <resLen> must be positive and smaller than or equal to 16. */
/* <data>        refers to the array of byte containing the information that must be hashed. */
/* <dataLen>     refers to the number of bytes in the <data> array. */
/*               <dataLen> must be a multiple of 16. */
int computeKeyGeneratorHash (unsigned char *result, int resLen,
			     unsigned char *data, unsigned int dataLen);

/* <computeMac>  computes a Message Authentication Code */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>      refers to the array of bytes which is at least <resLen> bytes large. */
/*               <result> contains the mac value computed on <data> using <key>. */
/* <resLen>      specifies the number of bytes needed in the mac value. */
/*               <resLen> must be positive and smaller than or equal to 8. */
/* <data>        refers to the array of byte containing the information that must be maced. */
/* <dataLen>     refers to the number of bytes in the <data> array. */
/*               <dataLen> must be a multiple of 16. */
/* <key>         refers to the secret mac key. */
int computeMac (unsigned char *result, int resLen, unsigned char *data,
		unsigned int dataLen, unsigned char *key);

#endif
