/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="hashapi.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   boilerplate.cpp  $</workFile>
  <revision>$Revision: 1.7 $</revision>
<workFileTimeStamp/>
<archive />
<archiveTimeStamp />
<pvcsUser/>
</pvcs>
<comments>
  <fileName>hashapi.cpp</fileName>
  <creationDate>09/2008</creationDate>
  <rights>Steria Benelux</rights>
  <author>avn</author>
  <description>
    <fr>Hash API</fr>
    <nl>Hash API</nl></description>
  <history />
  <comment />
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <conio.h>
#include <io.h>
#include <string.h>

#include "..\gen\gentype.h"

#include "..\sha1\hashapi.h"
#include "..\sha1\sha1.h"

#define _DEBUGMODE      0      // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
char 	gDigestString[NDIG*2 +1] = "";

//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="setPreviousDigestString">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE setPreviousDigestString (
	char* pHexDigest		// in: an hexadecimal digest
	)
// ---------------------------------------------------------------------------
{

	strcpy( gDigestString, pHexDigest);

	return SUCCEEDED;
}
//</code></function>


/*<function id="doHash">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE doHash (
	char* pFileName,			// in: name of the file to digest
	char* pDigestString					// out: a digest string
	)
// ---------------------------------------------------------------------------
{
		FILE*			infile;

		unsigned char 	digest[NDIG];	// holds the binary digest
		char 			aDigit[2 +1];
		int i;

	*pDigestString = '\0';

	infile = fopen( pFileName, "rb");
	if ( infile != NULL)
	{
		memset( digest, '\0', sizeof( digest));

		SHA1_stream( infile, digest);

    	fclose ( infile);

		// convert digest to string
		for ( i = 0; i < NDIG ; i++)
		{
			sprintf( aDigit, "%02X\0", digest[i]);
			strcat( pDigestString, aDigit);
		}

		return SUCCEEDED;
	}
	else
	{
		// file not found
		return ABORTED;
	}

}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="getPreviousDigestString">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
char* getPreviousDigestString ()
// ---------------------------------------------------------------------------
{
	return gDigestString;
}
//</code></function>


/*<function id="computeHash">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE computeHash (
	char* pFileName			// in: name of the file to digest
	)
// ---------------------------------------------------------------------------
{
		char 	aNewDigestString[NDIG*2 +1];

	if ( doHash( pFileName, aNewDigestString) == ABORTED) return ABORTED;
	if ( setPreviousDigestString( aNewDigestString) == ABORTED) return ABORTED;

	return SUCCEEDED;
}
//</code></function>

/*<function id="compareHash">
<heading>
  <description/>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE compareHash (
	char* pFileName		// in: name of the file to compare with previous digest
	)
// ---------------------------------------------------------------------------
{
		char 			aNewDigestString[NDIG*2 +1];

	if ( doHash( pFileName, aNewDigestString) == ABORTED) return ABORTED;
	if ( strcmp( aNewDigestString, getPreviousDigestString())) return ABORTED;
	// set ( and log?)
	if ( setPreviousDigestString( aNewDigestString) == ABORTED) return ABORTED;

	return SUCCEEDED;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
