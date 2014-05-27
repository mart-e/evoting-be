#include "fullSpec.h"

#define nof_generation_hash_iterations 100   /* w in the E-Voting Rijndaelintegratie document version 1.2 */

/* <computeKeyGeneratorHash> is a hash function designed to produce master keys and session keys. */
/* <return code> an integer, zero means ok, negative means an error occurred. */
/* <result>     refers to the array of bytes which is at least <resLen> bytes large. */
/*              <result> contains the hash value computed on <data>. */
/* <resLen>     specifies the number of bytes needed in the hash value. */
/*              <resLen> must be positive and smaller than or equal to 16. */
/* <data>       refers to the array of byte containing the information that must be hashed. */
/* <dataLen>    refers to the number of bytes in the <data> array. */
/*              <dataLen> must be a multiple of 16. */
int
computeKeyGeneratorHash (unsigned char *result, int resLen,
          unsigned char *data, unsigned int dataLen)
{
  return computeIteratedHash (result, resLen, data, dataLen,
               nof_generation_hash_iterations);
}
