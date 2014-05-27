#include <stdio.h>
#include "hlc.h"

#define OPSYS -1
#define FAST  register

/* GLOBAL VARIABLES */
char Message[78];
long fileok;     
FILE *pfin  = stdin,    /* file pointer of input file  */
     *pfout = stdout;   /* file pointer of output file */

quot( argc, argv)
    int   argc;
    char *argv[];
{
    fileok = 1;
    pfin   = stdin;
    pfout  = stdout;
    if( !parseargs( argv))
    {
        if( strlen( Message) > 0)
            promsgd( Message);
        fileok = 0;
    }
    else
        proclines();

    if( pfout != stdout)
        fclose( pfout);

    if( pfin  != stdin )
        fclose( pfin );

    if( prowti( "fileok", 0, fileok, 0))
        promsgd( "Could not write the shared data 'fileok'.");

    return( 0);
}


/*********************************************************/
/** proclines - process the input file line by line	**/
/*********************************************************/
proclines ()
{
FAST	int	c;		/* current input character */
FAST	int	leading = 1;	/* flag if leading or trailing delimiter */

    while( ( c = getc( pfin)) != EOF )
    {
        /* if start of a line, output a leading quote */
        if ( leading)
        {
           leading = 0;
           putc( '"', pfout);
        }

        /* if newline seen, set leading quote switch and output
           trailing quote
           turn each quote character into two quote characters */
        switch( c)
        {
            case '\n':
                leading = 1;

            case '"' :
                putc( '"', pfout);
        }

        putc(c, pfout); /* pass character through */
    }
    if (!leading)
    {
        putc( '"',  pfout);
        putc( '\n', pfout);
    }
}

/*****************************************************************/
/** parseargs - parse the command line arguments and sets       **/
/**     global variables with inputs                            **/
/**     returns 1 on success, 0 on error.                       **/
/*****************************************************************/
parseargs (argv)
FAST	char	*argv[];
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
            if (pfin != stdin)       /* already have an input file  */
                if( pfout != stdout) /* already have an output file */
                {
                    sprintf( Message, "QUOT: 1 input and 1 output file expected !");
                    return( 0);
                }
                else
                {
                    if ( (pfout = fopen(*argv, "wt")) == (FILE *) NULL)
                    {
                        sprintf( Message, "QUOT: Unable to write to %s !", *argv);
                        return( 0);
                    }
                }
            else
                if ( (pfin = fopen(*argv, "r")) == (FILE *) NULL)
                {
                    sprintf( Message, "QUOT: Unable to read from %s !", *argv);
                    return( 0);
                }
        }
    }
    if( pfin  == stdin ||
        pfout == stdout  )
    {
        sprintf( Message, "QUOT: minimum 1 input and 1 output file expected !");
        return( 0);
    }
    return( 1);
}
