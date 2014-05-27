/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="quoter.c">
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.2 $</revision>
  <archiveTimeStamp>$Date: 2005/09/12 12:27:05 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>quoter.c</fileName>
  <creationDate></creationDate>
  <rights></rights>
  <author></author>
  <description>
    <fr>Copie d' un fichier d' entrée vers un fichier de sortie en plaçant des guillemets au début et à la fin de chaque ligne et en protégeant les guillemets</fr>
    <nl>Kopie van een input-bestand naar een output-bestand. Aan het begin en het einde van elke worden aanhalingstekens gezet. De tussenliggende aanhalingstekens worden beschermd.</nl>
  </description>
  <history></history>
</comments>
<header>*/

/*<includes>*/
#include <stdio.h>
#include "hlc.h"
/*</includes>*/

#define OPSYS -1
#define FAST  register

/* GLOBAL VARIABLES */
char Message[78];
long fileok;
char delim   = 0;        /* field delimiter character   */
FILE *streamin  = stdin,    /* file pointer of input file  */
     *streamout = stdout;   /* file pointer of output file */
/*</header>

<body>*/

/*<function id="QUOT">
<heading>
<description>
  <fr>Copie d' un fichier d' entrée vers un fichier de sortie en plaçant des guillemets au début et à la fin de chaque ligne et en protégeant les guillemets</fr>
  <nl>Kopie van een input-bestand naar een output-bestand. Aan het begin en het einde van elke worden aanhalingstekens gezet. De tussenliggende aanhalingstekens worden beschermd.</nl></description>
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
          <fr>pointeur vers les paramètres</fr>
          <nl>pointer naar de parameters</nl></description></parameter>
</parameters>
</heading>
<code>*/
quot ( argc, argv)
    int   argc;
    char *argv[];
{
    fileok = 1;
    streamin   = stdin;
    streamout  = stdout;
    if( !parseargs( argv))
    {
        if( strlen( Message) > 0)
            promsgd( Message);
        fileok = 0;
    }
    else
        proclines();

    if( streamout != stdout)
        fclose( streamout);

    if( streamin  != stdin )
        fclose( streamin );

    if( prowti( "fileok", 0, fileok, 0))
        promsgd( "Could not write the shared data 'fileok'.");

    return( 0);
}
/*</code></function>

<function id="proclines">
<heading>
<description></description>
<parameters></parameters>
</heading>
<code>*/
/*********************************************************/
/** proclines - process the input file line by line   **/
/*********************************************************/
proclines ()
{
FAST  int   c;    /* current input character */
FAST  int   leading = 1;   /* flag if leading or trailing delimiter */

    while( ( c = getc( streamin)) != EOF )
    {
        /* if start of a line, output a leading quote */
        if ( leading)
        {
           leading = 0;
           putc( '"', streamout);
        }

        /* if newline seen, set leading quote switch and output
           trailing quote
           turn each quote character into two quote characters */
        switch( c)
        {
            case '\n':
                leading = 1;

            case '"' :
                putc( '"', streamout);
        }

        putc(c, streamout); /* pass character through */
    }
    if (!leading)
    {
        putc( '"',  streamout);
        putc( '\n', streamout);
    }
}
/*</code></function>

<function id="parseargs">
<heading>
<description></description>
<parameters></parameters>
</heading>
<code>*/
/*****************************************************************/
/** parseargs - parse the command line arguments and sets       **/
/**     global variables with inputs                            **/
/**     returns 1 on success, 0 on error.                       **/
/*****************************************************************/
parseargs (argv)
FAST  char  *argv[];
{
    Message[0] = '\0';
    while( *++argv)
    {
        if( **argv == '-')
        {
            sprintf( Message, "QOUT: Unknown option: -%s\n", argv[0]);
            return( 0);
        }
        else
        {
            if (streamin != stdin)       /* already have an input file  */
                if( streamout != stdout) /* already have an output file */
                {
                    sprintf( Message, "QUOT: 1 input and 1 output file expected !");
                    return( 0);
                }
                else
                {
                    if ( (streamout = fopen(*argv, "wt")) == (FILE *) NULL)
                    {
                        sprintf( Message, "QUOT: Unable to write to %s !", *argv);
                        return( 0);
                    }
                }
            else
                if ( (streamin = fopen(*argv, "r")) == (FILE *) NULL)
                {
                    sprintf( Message, "QUOT: Unable to read from %s !", *argv);
                    return( 0);
                }
        }
    }
    if( streamin  == stdin ||
        streamout == stdout  )
    {
        sprintf( Message, "QUOT: minimum 1 input and 1 output file expected !");
        return( 0);
    }
    return( 1);
}
/*</code></function>

<function id="QUOTD">
<heading>
<description>
  <fr>Copie d' un fichier d' entrée vers un fichier de sortie en plaçant des guillemets autour de chaque champ indiqué par un délimiteur.</fr>
  <nl>Kopie van een input-bestand naar een output-bestand. Velden, aangegeven door een begrenzingsteken, worden tussen aanhalingstekens geplaatst.</nl></description>
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
          <fr>pointeur vers les paramètres</fr>
          <nl>pointer naar de parameters</nl></description></parameter>
</parameters>
</heading>
<code>*/
/*****************************************************************/
/**                                                             **/
/**                            QUOTD                            **/
/**                                                             **/
/*****************************************************************/

quotd ( argc, argv)
   int    argc;
    char *argv[];
{
    fileok = 1;
    streamin  = stdin;
    streamout = stdout;
    if( !parsedargs( argv))
    {
        if( strlen( Message) > 0)
            promsgd( Message);
        fileok = 0;
    }
    else
        procdelims();

    if( streamout != stdout)
        fclose( streamout);

    if( streamin  != stdin )
        fclose( streamin );

    if( prowti( "fileok", 0, fileok, 0))
        promsgd( "Could not write the shared data 'fileok'.");

    return( 0);
}
/*</code></function>

<function id="procdelims">
<heading>
<description></description>
<parameters></parameters>
</heading>
<code>*/
/*****************************************************************/
/** procdelims - process the input file using the delimiter     **/
/*****************************************************************/
procdelims ()
{
FAST     int    c;   /* current character */

SCAN: /* scan for delimiter */
    for(;;)
        switch( c = getc( streamin))
        {
            case '"' :
                goto QTOK;

            case EOF :
                return 0;

            case ' ' :
            case '\t':
            case '\n':
                putc( c, streamout);
                continue;

            default:
                if( c == delim)
                    goto DELIM;
                goto NQTOK;
        }

QTOK: /* found token already quoted */
    for(;;)
    {
        putc( c, streamout);
        switch (c = getc( streamin))
        {
            case EOF:
            case '"':
                putc( '"', streamout);
                goto SCAN;
        }
    }

DELIM:   /* found a delimiter */
    for(;;)
    {
        putc( ' ', streamout);
        switch( c = getc( streamin))
        {
            case '"' :
                goto QTOK;

            case '\n':
            case EOF :
                putc( '\n', streamout);
                goto SCAN;

            default  :
                if( c == delim)
                    putc( '-', streamout);
                else
                    goto NQTOK;
        }
    }

NQTOK:   /* found a token not already quoted */
    putc( '"', streamout);
    for(;;)
    {
        putc( c, streamout);
        switch( c = getc( streamin))
        {
            case '\n':
            case EOF :
                putc( '"',  streamout);
                putc( '\n', streamout);
                goto SCAN;

            case '"' :
                putc( '"',  streamout);
                continue;

            default  :
                if( c == delim)
                {
                    putc( '"', streamout);
                    goto DELIM;
                }
        }
    }
}
/*</code></function>

<function id="parsedargs">
<heading>
<description></description>
<parameters></parameters>
</heading>
<code>*/
/*****************************************************************/
/** parsedargs - parse the command line arguments and sets       **/
/**     global variables with inputs                            **/
/**     returns 1 on success, 0 on error.                       **/
/*****************************************************************/
parsedargs (argv)
FAST  char  *argv[];
{
    while( *++argv)
    {
        if( **argv == '-')
        {
            switch (argv[0][1])
            {
                case 'D':
                case 'd':
                    if( *++argv       == 0 ||
                        strlen(*argv) != 1   )
                    {
                        if( *argv         != 0   &&
                            strlen(*argv) == 3   &&
                            **argv        == '"'   )
                        {
                            delim = argv[0][1];
                            break;
                        }
                        sprintf( Message,
      "The -d option should be followed by a space and a single character !");
                        return( 0);
                    }
                    delim = **argv;
                    break;

                default :
                    sprintf( Message, "Unknown option: -%c !", argv[0][1]);
                    return( 0);
            }
        }
        else
        {
            if (streamin != stdin)       /* already have an input file  */
                if( streamout != stdout) /* already have an output file */
                {
                    sprintf( Message, "QUOTD: 1 input and 1 output file expected !");
                    return( 0);
                }
                else
                {
                    if ( (streamout = fopen(*argv, "wt")) == (FILE *) NULL)
                    {
                        sprintf( Message, "QUOTD: Unable to write to %s !", *argv);
                        return( 0);
                    }
                }
            else
                if ( (streamin = fopen(*argv, "r")) == (FILE *) NULL)
                {
                    sprintf( Message, "QUOTD: Unable to read from %s !", *argv);
                    return( 0);
                }
        }
    }
    if( streamin  == stdin ||
        streamout == stdout  )
    {
        sprintf( Message, "QUOT: minimum 1 input and 1 output file expected !");
        return( 0);
    }
    if( delim == 0)
    {
        sprintf( Message, "Incompatible option: -d !");
        return( 0);
    }
    return( 1);
}
/*</code>
</function>
</body>
</source>*/
