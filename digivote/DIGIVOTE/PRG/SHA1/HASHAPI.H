/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="hashapi.h">
<pvcs><!-- these tags are filled out by version control during check in -->
<workFile>$Workfile:   hashapi.h  $</workFile>
<revision>$Revision: 1.3 $</revision>
<workFileTimeStamp/>
<archive />
<archiveTimeStamp />
<pvcsUser/>
</pvcs>
<comments>
  <description>
    <fr>Fichier d'en-tête</fr>
    <nl>Header bestand</nl></description>
  <comment />
</comments>*/
//<header>
//============================================================= BOF HEADER ===
#ifndef __hashapi_h__				// avoids multiple inclusions of this file
	#define __hashapi_h__

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---

//--------------------------------------------------------------- TYPEDEFS ---

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
char* getPreviousDigestString (
	);
RET_CODE computeHash (
	char* pFileName				// in: name of the file to digest
	);
RET_CODE compareHash (
	char* pFileName				// in: name of the file to compare with previous digest
	);

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---


#endif // __hashapi_h__
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->