/*
============================================================================

   File        : debug.c
   Creation    : Sept 2001

   Author      : AVN

   Description : Debugging library

===============================================================================
*/
/* <includes> */
   #include <stdio.h>
   #include <string.h>

   #include "debug.h"
   /*</includes> */

/* <definitions> <!-- these DO allocate memory -->
/* <globalVariables> */

   char cLogBuf[256];                   /* buffer to hold logging/dbg info */
   /* </globalVariables> */
   /* </definitions> */

/* <functions> */
/* <public> */

/*==========================================================================*/
int Dbug()
/*==========================================================================*/
{
      static FILE *stream;
      static int firstCall = 1;

   if ( firstCall)
   {
       if ( ( stream = fopen( "AES.LOG", "at")) != NULL )
       {  /* mode: append + text */
           printf( "\a"); /* ring bell on open log */
       }
       firstCall = 0;
   }

   if ( !strncmp( "close", cLogBuf, 5))
   {
      fprintf( stream, "--- eof ---\n");
      fclose( stream);
      firstCall = 1;
   }
   else
   {
      fprintf( stream, cLogBuf);
      fflush( stream);
   }

   return 0;
}


/*==========================================================================*/
int DbugHex( unsigned char* bytes, int len)
/*==========================================================================*/
{
      int j;

   /* dump len bytes */
   sprintf( cLogBuf, "["); Dbug();
   for ( j=0; j < len; j++)
   {
      sprintf( cLogBuf, "%02X ", bytes[j]); Dbug();
   }
   sprintf( cLogBuf, "]\n"); Dbug();

   return 0;
}

/* </public> */
/* </functions> */
/* eof */