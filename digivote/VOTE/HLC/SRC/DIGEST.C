/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="digest.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.5 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:04 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>digest.c</fileName>
  <creationDate>13/03/2000</creationDate>

  <rights></rights>
  <author>JRU</author>

  <description>
    <fr>Rend un fichier Hidden et/ou Read-Only</fr>
    <nl>Maak bestand Hidden en/of Read-Only</nl></description>
  <history></history>
  <comment>
    Parameter 1 : bestandsnaam
    Parameter 2 : te bekomen attrib
        - 0 : Visible + Writable
        - 1 : Read-Only
        - 2 : Hidden
        - 3 : Read-Only + Hidden
</comment>
</comments>

<header>*/

/*<includes>*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>

#include "hlc.h"
#include "genlib.h"

#define STREAMBUFFLEN 1024   /* this program uses STREAMDIGEST */
#include "ddcustom.h"        /* customise for digest */
/*</includes>*/

/*</header>
<body>

<function id="digest">
<heading>
  <description>
    <en></en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment/>
  <author>avn</author>
</heading>
<code>*/
int
digest ( argc, argv)
   int  argc;
   char *argv[];
/*----------------------------------------------------------------*/
{
   unsigned char digest[NDIG];
   unsigned char hexdigest[NDIG * 2];
   char aDigit[2];
   FILE *infile;
   int i, j;

   /* ---------------------------------------- */
   /* initialize global variables for 'digest' */
   /* ---------------------------------------- */
   strcpy( gcModuleName, "digest");
   gcFunctionName[0] = '\0'; // don't mention FunctionName -> customized messages !
   giStatus          = 0;
   giError           = 0;
   /* ---------------------------------------- */

   strcpy( hexdigest, "-1");
   if( prowtc( "hex-digest", 0, hexdigest, 0) != 0) {
      promsgd( "Could not write the shared data in 'hex-digest'.");
      strcpy( gcMessage, "Could not write the shared data in 'hex-digest'.");
      write2log();
   }
   memset( hexdigest, '\0', sizeof( hexdigest));
   if( argc == 2) {
      infile = fopen( argv[1], "rb");
      if( infile != NULL) {
         memset( hexdigest, '\0', sizeof( hexdigest));
         memset(    digest, '\0', sizeof(    digest));
         STREAMDIGEST( infile, digest);
         memset( aDigit, '\0', sizeof( aDigit));
         for(i = 0, j = 0 ; i < NDIG ; i++) {
            sprintf( aDigit, "%02x", digest[i]);
            hexdigest[j++] = aDigit[0];
            hexdigest[j++] = aDigit[1];
         }
         hexdigest[NDIG * 2] = '\0';
      }
      else {
         strcpy( hexdigest, "***FILE NOT FOUND***");
         giStatus = -1;
         giError  = 2;
      }
   }
   else {
      strcpy( hexdigest, "***INCORRECT CALL***");
      giStatus = -2;
      giError  = 2;
  }

   if( !infile)
      fclose( infile);

   if( giStatus) {
      strcpy( gcMessage, hexdigest);
      write2log();
   }
   else {
      if( prowtc( "hex-digest", 0, hexdigest, 0) != 0) {
         promsgd( "Could not write the shared data in 'hex-digest'.");
         strcpy( gcMessage, "Could not write the shared data in 'hex-digest'.");
         write2log();
      }
   }

   return( 0);
}
/*</code>
</function>
</body>
</source>*/
