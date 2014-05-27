/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="aes.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   AES.cpp  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Mar 19 2003 10:20:32  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/AES/AES.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:01 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>AES.CPP</fileName>
  <creationDate>Sept 2001</creationDate>
  <rights></rights>
  <author>AVN</author>
  <description>
    <fr>Interface entre gencryp.cpp et l'API AES.</fr>
    <nl>Interface tussen gencryp.cpp en AES API.</nl></description>
  <history></history>
  <comment>This module forms a wrapper around the original AES functions as provided by the KUL.
  These customized functions rely on the optimized ANSI C code for the Rijndael cipher that has
  been placed in the public domain.</comment>
</comments>*/
//===============================================================================
#include "..\aes\fullSpec.h"
#include "..\aes\encdec.c"
#include "..\aes\hashComp.c"
#include "..\aes\macComp.c"
#include "..\aes\mastrGen.c"
