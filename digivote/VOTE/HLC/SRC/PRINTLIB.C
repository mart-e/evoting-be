/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="printlib.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.3 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:05 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>printlib.c</fileName>
  <creationDate>02/05/2005</creationDate>
  <rights></rights>
  <author>Jurgen Rousseau</author>
  <description>
    <fr>Fonctions DOS</fr>
    <nl>DOS functies</nl></description>
  <history></history>
  <comment>Functionality       : Printer handling...</comment>
</comments>

<header>*/

/************************************************************/
/*                                                          */
/*  INCLUDE FILES                                           */
/*                                                          */
/************************************************************/
/*<includes>*/
#include <stdlib.h>
#include <stdio.h>
#include <bios.h>
#include <dos.h>

#include "hlc.h"
#include "genlib.h"
/*</includes>*/

/************************************************************/
/*                                                          */
/*      CONSTANT DEFINITIONS                                */
/*                                                          */
/************************************************************/
#define LPT1         0

#define TIME_OUT     0x01
#define IO_ERROR     0x08
#define SELECTED     0x10
#define OUT_OF_PAPER 0x20
#define ACKNOWLEDGE  0x40
#define NOT_BUSY     0x80

#define OPSYS        -1
#define FAST         register

#define LINELENGTH   132

#define PageBreak    '\012'

/************************************************************/
/*                                                          */
/*  GLOBAL VARIABLES FOR THIS FILE ONLY                     */
/*                                                          */
/************************************************************/
static char gcLibName[] = "PrintLib";
long  offset,
      flength,
      init_r,
      init_n;
FILE *printer,
     *streamin,
     *streamout;
/*</header>

<body>*/

/*<function id="C_PRTYPE">
<heading>
<description>
  <fr>Détection du type d'imprimante </fr>
  <nl>Verwijderd een bestand</nl></description>
<parameters>
  <parameter>
    <datatype>int</datatype>
      <name>argc</name>
      <description>
        <fr>Nombre de paramètres</fr>
        <nl>Aantal parameters</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[1]</name>
        <description>
          <fr>pas utilisé</fr>
          <nl>ongebruikt</nl></description></parameter>
</parameters>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_prtype                          */
/*                                                          */
/*  Functionality       : Determine type of used printer    */
/*                                                          */
/************************************************************/

c_prtype ( )
{
    InitFunction( gcLibName, "c_prtype", "printer", TRUE);
    if( giStatus == -1) return( 0);

    switch( _bios_printer( _PRINTER_INIT, LPT1, 'x'))
    {
        case SELECTED+NOT_BUSY: /* HP LaserJet 1100 */
            giStatus = 1;
            break;

        case SELECTED         : /* Bull CompuPrint  */
            giStatus = 0;
            break;

        default               :
            giStatus = -1;
            break;
    }
    updSharedVar();
    return(0);
}
/*</code></function>

<function id="C_PRSTAT">
<heading>
<description>
  <fr>Test de l'imprimante avec écriture de l' état</fr>
  <nl>Test van de printer met zetten van status</nl></description>
<parameters></parameters>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_prstat                          */
/*                                                          */
/*  Functionality       : Determine status of printer       */
/*                                                          */
/************************************************************/

c_prstat ( argc, argv)
    int argc;
    char *argv[];
{
    char reset[1];

    InitFunction( gcLibName, "c_prstat", "stat", TRUE);
    if( giStatus == -1) return( 0);

    giStatus = _bios_printer( _PRINTER_STATUS, LPT1, 'x');
    if( giStatus == 144)
        if( ( printer = fopen( "prn", "wb")) == NULL)
            giStatus = 128;
        else
        {
            reset[0] = 0x00;
            if( fwrite( &reset[0], 1, sizeof(reset), printer) < sizeof( reset))
                giStatus = 128;
            fclose( printer);
        }
    updSharedVar();
    return(0);
}
/*</code></function>

<function id="C_PRINT">
<heading>
<description>
  <fr>Impression controlée d'un fichier sur la porte LPT1</fr>
  <nl>Gecontroleerd printen van een bestand naar de LPT1-poort</nl></description>
<parameters>
  <parameter>
    <datatype>int</datatype>
      <name>argc</name>
      <description>
        <fr>Nombre de paramètres</fr>
        <nl>Aantal parameters</nl></description></parameter>
  <parameter>
    <datatype>char*</datatype>
      <name>argv[1]</name>
        <description>
          <fr>le fichier à imprimer</fr>
          <nl>het af te drukken bestand</nl></description></parameter>
</parameters>
</heading>
<code>*/
/************************************************************/
/*                                                          */
/*  Functionname        : c_print                           */
/*                                                          */
/*  Functionality       : Print file contents to printer    */
/*                                                          */
/************************************************************/

c_print ( argc, argv)
    int   argc;
    char *argv[];
{
    unsigned      status;
    char          abyte;
    struct find_t ffblk;

    InitFunction( gcLibName, "c_print", "stat", TRUE);
    if( giStatus == -1) return( 0);

    giStatus = 0;
    if( ! _dos_findfirst( argv[1],
                          _A_NORMAL | _A_HIDDEN | _A_RDONLY,
                          &ffblk))
        flength = ffblk.size;

    if( ( streamin = fopen( argv[1], "rt")) == NULL) {
        giStatus = -1;
        goto error;
    }
    if( prordi( "offset", 0, &offset, 0)) {
        strcpy( gcMessage, "Could not read the shared data 'offset'.");
        goto error;
    }
    if( offset == -1) {
        if( FixPrint( argv[1])) {
            if( ! _dos_findfirst( argv[1],
                                  _A_NORMAL | _A_HIDDEN | _A_RDONLY,
                                  &ffblk))
                flength = ffblk.size;

            if( ( streamin = fopen( argv[1], "rt")) == NULL) {
                giStatus = -6;
                goto error;
            }
            offset = 0;
        }
        else {
            goto error;
        }
    }
    else {
        if( offset > flength){
            giStatus = -2;
            goto error;
        }
    }
    init_r = offset ? 1
                    : 0;
    init_n = init_r;
    fseek( streamin, offset, SEEK_SET);
    while( !feof( streamin) && !giStatus){
        abyte  = getc( streamin);
        status = _bios_printer( _PRINTER_WRITE, LPT1, abyte);
        if( status & TIME_OUT)      giStatus += 1;
        if( status & IO_ERROR)      giStatus += 2;
        if( status & OUT_OF_PAPER)  giStatus += 4;

        if( !giStatus){
            if(      abyte == '\r') init_r = 1;
            else if( abyte == '\n') init_n = 1;
        }
    }
    if( giStatus) {
        if( init_r && init_n) {
            offset = ftell( streamin);
            if( status & 0x01)
                offset--;
        }
        else
            offset = 0;
        if( prowti( "flength", 0, flength, 0))
            sprintf( gcMessage, "Could not write the shared data 'flength'.");
        if( prowti( "offset",  0, offset,  0))
            sprintf( gcMessage, "Could not write the shared data 'offset'.");
    }
    if( !giStatus) remove( argv[1]);

error:
    if( strlen( gcMessage) > 0) {
        write2log();
        promsgd( gcMessage);
    }
    if( streamin) fclose( streamin);
    updSharedVar();
    return( 0);
}
/*</code></function>

<function id="FixLine">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

FixLine (
   char Contents[LINELENGTH]
)
{
   int x, y;

   for( x = 0 ; x < strlen(Contents) ; x++){
       if( x > 0)
           y = x - 1;
       else
           y = x;

       if( Contents[y] != '\r'   &&
           Contents[y] != '\012' &&
           Contents[x] == '\n'     )
           putc( '\r', streamout);
       putc( Contents[x], streamout);
   }
}
/*</code></function>

<function id="FixPrint">
<heading>
<description/>
<parameters/>
</heading>
<code>*/

FixPrint (
   char OrigFile[80]
)
{
    char Buffer     [LINELENGTH],
         LastBuffer [LINELENGTH],
         Prev1Buffer[LINELENGTH],
         Prev2Buffer[LINELENGTH],
         Prev3Buffer[LINELENGTH],
         tmpDrive   [_MAX_DRIVE],
         tmpDir     [_MAX_DIR  ],
         tmpFname   [_MAX_FNAME],
         TempFile   [    80    ];
    int  Printing = 0, i = 0, j;

    _splitpath( OrigFile, tmpDrive, tmpDir, tmpFname, NULL);
    _makepath(  TempFile, tmpDrive, tmpDir, tmpFname, ".$$$");

    if( ( streamout = fopen( TempFile, "wt")) == NULL){
        giStatus = -3;
        return( 0);
    }
    while( !feof( streamin)){
        switch( i)
        {
            case 0 :                            /* Before reading line 1 */
                break;

            case 1 :                            /* Before reading line 2 */
                strcpy( Prev1Buffer, Buffer);
                break;

            case 2 :                            /* Before reading line 3 */
                strcpy( Prev2Buffer, Prev1Buffer);
                strcpy( Prev1Buffer, Buffer);
                break;

            case 3 :                            /* Before reading line 4 */
                strcpy( Prev3Buffer, Prev2Buffer);
                strcpy( Prev2Buffer, Prev1Buffer);
                strcpy( Prev1Buffer, Buffer);
                break;

            default:                            /* Printing is enabled now */
                Printing = 1;
                strcpy( LastBuffer,  Prev3Buffer);
                strcpy( Prev3Buffer, Prev2Buffer);
                strcpy( Prev2Buffer, Prev1Buffer);
                strcpy( Prev1Buffer, Buffer);
                break;
        }
        fgets( Buffer, sizeof(Buffer), streamin);
        i++;
        if( Printing)
            FixLine( LastBuffer);
    }
    fclose( streamin);
    if( !strncmp( PageBreak, Buffer, 1) ||
        !strncmp( "PG",      Buffer, 2))
        if( !strncmp( PageBreak, Prev1Buffer, 1) ||
            !strncmp( "PG",      Prev1Buffer, 2))
            if( !strncmp( PageBreak, Prev2Buffer, 1) ||
                !strncmp( "PG",      Prev2Buffer, 2))
                if( !strncmp( PageBreak, Prev3Buffer, 1) ||
                    !strncmp( "PG",      Prev3Buffer, 2))
                    {}
                else
                    FixLine( Prev3Buffer);
            else{
                FixLine( Prev3Buffer);
                FixLine( Prev2Buffer);
            }
        else{
            FixLine( Prev3Buffer);
            FixLine( Prev2Buffer);
            FixLine( Prev1Buffer);
        }
    else{
        FixLine( Prev3Buffer);
        FixLine( Prev2Buffer);
        FixLine( Prev1Buffer);
        FixLine( Buffer);
    }
    putc( PageBreak, streamout);
    fclose( streamout);
    if( remove( OrigFile)){
        return( giStatus = -4);
    }
    if( rename( TempFile, OrigFile)){
        return( giStatus = -5);
    }
    return( giStatus = 1);
}
/*</code></function>

</body>
</source>
<!-- eof -->*/
