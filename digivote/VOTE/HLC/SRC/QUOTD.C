#include <stdio.h>
#include "hlc.h"

#define OPSYS -1
#define FAST  register

/* GLOBAL VARAIBLES */
char Message[78];
long fileok;
char delim   = 0;        /* field delimiter character   */
FILE *pfdin  = stdin,    /* file pointer of input file  */
     *pfdout = stdout;   /* file pointer of output file */

quotd( argc, argv)
	int	 argc;
    char *argv[];
{
    fileok = 1;
    pfdin  = stdin;
    pfdout = stdout;
    if( !parsedargs( argv))
    {
        if( strlen( Message) > 0)
            promsgd( Message);
        fileok = 0;
    }
    else
        procdelims();

    if( pfdout != stdout)
        fclose( pfdout);

    if( pfdin  != stdin )
        fclose( pfdin );

    if( prowti( "fileok", 0, fileok, 0))
        promsgd( "Could not write the shared data 'fileok'.");

    return( 0);
}

/*****************************************************************/
/** procdelims - process the input file using the delimiter     **/
/*****************************************************************/
procdelims ()
{
FAST		int	 c;	/* current character */

SCAN:	/* scan for delimiter */
    for(;;)
        switch( c = getc( pfdin))
        {
            case '"' :
                goto QTOK;
    
            case EOF :
                return 0;
    
            case ' ' :
            case '\t':
            case '\n':
                putc( c, pfdout);
                continue;
    
            default:
                if( c == delim)
                    goto DELIM;
                goto NQTOK;
        }

QTOK:	/* found token already quoted */
    for(;;)
    {
        putc( c, pfdout);
        switch (c = getc( pfdin))
        {
            case EOF:
            case '"':
                putc( '"', pfdout);
                goto SCAN;
        }
    }

DELIM:	/* found a delimiter */
    for(;;)
    {
        putc( ' ', pfdout);
        switch( c = getc( pfdin))
        {
            case '"' :
                goto QTOK;

            case '\n':
            case EOF :
                putc( '\n', pfdout);
                goto SCAN;

            default  :
                if( c == delim)
                    putc( '-', pfdout);
                else
                    goto NQTOK;
        }
    }

NQTOK:	/* found a token not already quoted */
    putc( '"', pfdout);
    for(;;)
    {
        putc( c, pfdout);
        switch( c = getc( pfdin))
        {
            case '\n':
            case EOF :
                putc( '"',  pfdout);
                putc( '\n', pfdout);
                goto SCAN;

            case '"' :
                putc( '"',  pfdout);
                continue;

            default  :
                if( c == delim)
                {
                    putc( '"', pfdout);
                    goto DELIM;
                }
        }
    }
}

/*****************************************************************/
/** parsedargs - parse the command line arguments and sets       **/
/**     global variables with inputs                            **/
/**     returns 1 on success, 0 on error.                       **/
/*****************************************************************/
parsedargs (argv)
FAST	char	*argv[];
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
            if (pfdin != stdin)       /* already have an input file  */
                if( pfdout != stdout) /* already have an output file */
                {
                    sprintf( Message, "QUOTD: 1 input and 1 output file expected !");
                    return( 0);
                }
                else
                {
                    if ( (pfdout = fopen(*argv, "wt")) == (FILE *) NULL)
                    {
                        sprintf( Message, "QUOTD: Unable to write to %s !", *argv);
                        return( 0);
                    }
                }
            else
                if ( (pfdin = fopen(*argv, "r")) == (FILE *) NULL)
                {
                    sprintf( Message, "QUOTD: Unable to read from %s !", *argv);
                    return( 0);
                }
        }
    }
    if( pfdin  == stdin ||
        pfdout == stdout  )
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
