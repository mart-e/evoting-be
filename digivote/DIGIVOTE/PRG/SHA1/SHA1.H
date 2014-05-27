/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="sha1.h">
<pvcs><!-- these tags are filled out by version control during check in -->
<workFile>$Workfile:   sha1.h  $</workFile>
<revision>$Revision: 1.2 $</revision>
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
#ifndef __sha1_h__					// avoids multiple inclusions of this file
	#define __sha1_h__

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
#define NDIG 20

#define STRINGDIGEST SHA1_string
#define STREAMDIGEST SHA1_stream
#define MULTIDIGEST SHA1_multi


//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
void SHA1_stream(
	FILE*,
	unsigned char [NDIG]
	);
void SHA1_multi(
	unsigned char *,
	int,
	unsigned char [NDIG]
	);
void SHA1_string(
	unsigned char *,
	int,
	unsigned char [NDIG]
	);
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---


#endif // __sha1_h__
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->