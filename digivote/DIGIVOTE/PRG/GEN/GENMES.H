/*<?xml version="1.0" encoding="ISO-8859-1"?>

<source name="genmes.h"><!-- use lower case for name -->

<pvcs/>
<comments>
  <fileName>genmes.h</fileName>
  <creationDate>01/10/2002</creationDate>

  <rights>Steria Belux</rights>
  <author>AVN</author>

  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _genmes_h                   // avoids multiple inclusions of this file
    #define _genmes_h

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//<system></system>
//<user></user>
#include "..\gen\genmsg.h"
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//                                         <!-- these functions are public -->
//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
char* Get_Text( int Mnemo, int Language);
//</prototypes>


#endif // _genmes_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->