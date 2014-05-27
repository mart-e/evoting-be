#include "fullSpec.h"

/* <extendPassword> transforms a a randomly generated password consisting of a */
/*                   zero-terminated password string of <passwordLength-1> */
/*                   characters into a zero-terminated password string of */
/*                   <fullPasswordLength-1>. */
/* <return code>     an integer, zero means ok, negative values refer to an error code. */
/* <fullPassword>    refers to a zero-terminated string that consists of the randomly generated password and its integrity protection. */
/* <password>        refers to the randomly generated password. */
int
extendPassword (char *fullPassword, char *password)
{
  long passwordValue = 0;
  int i;
  int extension;
  memset (fullPassword, 0, fullPasswordLength);
  if (strlen (password) != passwordLength - 1)
    {
      return ret_invalid_password_length;
    }
  for (i = 0; i < passwordLength - 1; i++)
    {
      passwordValue = 10 * passwordValue + (password[i] - '0');
      fullPassword[i] = password[i];
    }
  extension = integrityOffset - passwordValue % integrityModulus;
  for (i = 0; i < integrityModulusLength; i++)
    {
      fullPassword[passwordLength - i] = extension % 10 + '0';
      extension = extension / 10;
    }
  return ret_ok;
}


/* <validatePassword> determines whether an integrity-protected fullPassword is valid. */
/* <return code>      zero refers to a valid fullPassword, other values refer to an invalid fullPassword. */
/* <fullPassword>         refers to the zero-terminated string with the integrity-protected fullPassword of <fullPassword-1> characters. */
int
validatePassword (char *fullPassword)
{
  long fullPasswordValue = 0;
  int i;
  int extension = 0;
  int reference = 0;
  if (strlen (fullPassword) != fullPasswordLength - 1)
    {
      return ret_invalid_fullPassword_length;
    }
  for (i = 0; i < fullPasswordLength - 1 - integrityModulusLength; i++)
    {
      fullPasswordValue = 10 * fullPasswordValue + (fullPassword[i] - '0');
    }
  for (i = 0; i < integrityModulusLength; i++)
    {
      extension = 10 * extension +
	(fullPassword[fullPasswordLength - 1 - integrityModulusLength + i] -
	 '0');
    }
  reference = integrityOffset - (fullPasswordValue % integrityModulus);
  return (extension != reference);
}


/* <computeKeyFromPassword> computes a the hash value using the zero-terminated integrity-protected <fullPassword>. */
/*                          this method validates the integrity-protected fullPassword before computing the key. */
/* <return code>            zero means ok, all other values refer to an error. */
/* <cipherKey>                    refers to the array of byte that must be able to accomodate 16 bytes. */
/* <fullPassword>               refers to the zero-terminated integrity-protected fullPassword. */
int
computeKeyFromPassword (BYTE * cipherKey, char *fullPassword)
{
  BYTE expandedKey[cipherKeyLength];
  int i;
  if (validatePassword (fullPassword) != ret_ok)
    {
      return ret_invalid_fullPassword_length;
    }
  memset (expandedKey, 0, sizeof (expandedKey));
  for (i = 0; i < fullPasswordLength; i++)
    {
      expandedKey[i] = fullPassword[i] - '0';
    }
  return computeHash (cipherKey, 16, expandedKey, sizeof (expandedKey));
}
