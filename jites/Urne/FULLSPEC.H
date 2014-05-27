#ifndef __FULL_H__
#define __FULL_H__
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "api-fst.h"

#define hashSecurityLevel       50      /* number of iterations for the hash function */

#define cipherKeyLength         16      /* rijndael/AES uses 16-byte keys */
#define blockLength             16      /* rijndael/AES uses 16-byte blocks */
#define macResultLen            (blockLength/2) /* rijndael-based MAC algorithms produces at most 8 MAC-value bytes */

#define passwordLength          9       /* the length of a zero-byte-terminated password */
#define integrityModulus        97      /* the modulus against which the password integrity is computed */
#define integrityModulusLength  2       /* the number of digits in the modulus */
#define integrityOffset         99      /* the value from which the remainder must be subtracted */
#define fullPasswordLength      (passwordLength+integrityModulusLength) /* the length of the password including integrity protection */

#define ret_unknown -99         /* negative return code indicates an error */
#define ret_invalid_key_length -98
#define ret_invalid_data_length -97
#define ret_invalid_o_length -96
#define ret_malloc_problem -95
#define ret_invalid_fullPassword_length -94
#define ret_invalid_password_length -93
#define ret_invalid_mac -92
#define ret_invalid_e_datalength -91
#define ret_invalid_d_datalength -90

#define ret_ok 0                /* zero indicates successful execution */
#define nofBitsInByte 8         /* the number of bits in a single byte */

/*#define max(a,b)((a<b)?b:a)
#define min(a,b)((a<b)?a:b)*/

/* <extendPassword> transforms a a randomly generated password consisting of a */
/*                   zero-terminated password string of <passwordLength-1> */
/*                   characters into a zero-terminated password string of */
/*                   <fullPasswordLength-1>. */
/* <return code>     an integer, zero means ok, negative refers to an error code. */
/* <fullPassword>    refers to a zero-terminated string that consists of the randomly generated password and its integrity protection. */
/* <password>        refers to the randomly generated password. */
int extendPassword (char *fullPassword, char *password);

/* <validatePassword> determines whether an integrity-protected fullPassword is valid. */
/* <return code>      zero refers to a valid fullPassword, other values refer to an invalid fullPassword. */
/* <fullPassword>         refers to the zero-terminated string with the integrity-protected fullPassword of <fullPassword-1> characters. */
int validatePassword (char *fullPassword);

/* <computeKeyFromPassword> computes a the hash value using the zero-terminated integrity-protected <fullPassword>. */
/*                          this method validates the integrity-protected fullPassword before computing the key. */
/* <return code>            zero means ok, all other values refer to an error. */
/* <key>                    refers to the array of byte that must be able to accomodate 16 bytes. */
/* <fullPassword>               refers to the zero-terminated integrity-protected fullPassword. */
int computeKeyFromPassword (BYTE * key, char *fullPassword);

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

/* <encryptData> encrypts information. */
/* <return code>  an integer, zero means ok, negative means an error occurred. */
/* <result>        refers to the array of bytes which contains the ciphertext. */
/*                this array must at least be able to accomodate <plainLen>+16 bytes. */
/* <cipherInst>   refers to the mode of encryption, which is initialized via cipherInitBinary. */
/* <keyInst>      refers to the encryption key, which is initialized via makeKeyBinary. */
/* <plaintext>    refers to the array of byte containing the information that must be encrypted. */
/* <plainLen>     refers to the number of bytes in the <plaintext>. */
int
encryptData (unsigned char *result, cipherInstance * cipherInst,
	     keyInstance * keyInst, unsigned char *plaintext,
	     unsigned long plainLen);

/* <decryptData> decrypts encrypted information. */
/* <return code>  an integer.  an error has occurred if it is smaller than zero. */
/*                if non-negative, it represents the number of bytes that have been encrypted. */
/* <result>           refers to the array of bytes which contains the decrypted information. */
/*                 it is at least <cipherLen> bytes large. */
/* <cipherInst>   refers to the mode of decryption, which is initialized via cipherInitBinary. */
/* <keyInst>       refers to the decryption key, which is initialized via makeKeyBinary. */
/* <ciphertext>   refers to the array of byte containing the information that must be decrypted. */
/* <cipherLen>     refers to the number of bytes in the <ciphertext>. */
/*                 <cipherLen> must be a multiple of 16. */
int
decryptData (unsigned char *result, cipherInstance * cipherInst,
	     keyInstance * keyInst, unsigned char *ciphertext,
	     unsigned long cipherLen);

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
	     unsigned int dataLen);

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
		       unsigned int dataLen);

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
computeMacFloppy (unsigned char *result, int resLen,  unsigned char *key);

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
	    unsigned int dataLen, unsigned char *key);

/* <validateMac>  checks whether the <reference>-MAC corresponds to the give <data> and <key>. */
/* <return code>  an integer, zero means ok, other values refer to an error. */
/* <reference>    refers to the reference MAC value. */
/* <referenceLen> specifies the length of the reference MAC value. */
/* <data>         refers to the array of byte containing the information that must be validated. */
/* <dataLen>      refers to the number of bytes in the <data> array. */
/* <key>          refers to the secret mac key. */
int
validateMac (unsigned char *reference, int referenceLen, unsigned char *data,
	     unsigned int dataLen, unsigned char *key);

/* <encryptAndMac> transforms the plaintext information <plainData> into ciphertext and computes the corresponding MAC value. */
/* <return code>   an integer, negative values refer to an error code, positive values specify the length of the authenticated ciphertext. */
/* <authenticatedCiphertext> refers to the encrypted plaintext and the MAC value. */
/*                           <authenticatedCiphertext> must be able to accomodate at least <dataLen>+<blockLength>+<blockLength/2> bytes. */
/* <cipherKey>     refers to the 16-byte encryption key. */
/* <iv>            refers to the 16-byte initialization vector for the encryption. */
/* <macKey>        refers to the 16-byte MAC-key. */
/* <plainData>     refers to plaintext information that must be protected. */
/* <plainDataLen>  specifies the length of the plaintext information. */
int
encryptAndMac (unsigned char *authenticatedCiphertext,
	       unsigned char *cipherKey, unsigned char *iv,
	       unsigned char *macKey, unsigned char *data,
	       unsigned long dataLen);

/* <validateMacAndDecrypt> validates the MAC on the encrypted information, and decrypts the information if possible. */
/* <return code>   an integer specifying the length of the plaintext if this was produced, negative values refer to an error code. */
/* <plaintext> refers to the decrypted information if this was produced. */
/*                           <plaintext> must be able to accomodate at most <authenticatedCiphertext> bytes. */
/* <cipherKey>     refers to the 16-byte decryption key. */
/* <iv>            refers to the 16-byte initialization vector for the decryption. */
/* <macKey>        refers to the 16-byte MAC-key. */
/* <authenticatedCiphertext>       refers to the authenticated ciphertext. */
/* <authenticatedCiphertextLength> specifies the number of bytes used for the authenticated ciphertext. */
int
validateMacAndDecrypt (unsigned char *plaintext, unsigned char *cipherKey,
		       unsigned char *iv, unsigned char *macKey,
		       unsigned char *authenticatedCiphertext,
		       unsigned long authenticatedCiphertextLength);

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
int encryptKeys (BYTE * dataForFloppy, BYTE * cipherKey, BYTE * iv,
		 BYTE * macKey, BYTE * encKey);

/* <decryptKeys> decrypts the data retrieved from floppy <dataFromFloppy>. */
/* <return code> an integer, negative values refer to an error code, zero means that no problem was detected.. */
/*               note that zero does not indicate that the decrypted keys are valid: no redundancy is added to avoid exhaustive key searches. */
/* <macKey>      refers to the 16-byte MAC-key that has been decrypted. */
/* <encKey>      refers to the 16-byte encryption key that has been decrypted. */
/* <dataFromFloppy> refers to an array of char that contains the information that was stored on a floppy disk. */
/*                 <dataFromFloppy> must consist of <3*<blockLength>> bytes. */
/* <cipherKey>     refers to the 16-byte encryption key with which the <dataFromFloppy> will be decrypted. */
int decryptKeys (BYTE * decryptedMacKey, BYTE * decryptedEncKey,
		 BYTE * dataFromFloppy, BYTE * cipherKey);

#endif /*  */
