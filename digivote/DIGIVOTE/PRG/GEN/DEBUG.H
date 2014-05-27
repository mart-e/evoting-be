/*
============================================================================

   File        : debug.h
   Creation    : Sept 2001

   Author      : AVN

   Description : debug.c header file

===============================================================================
*/

#ifndef _DEBUG_H
   #define _DEBUG_H                   /* Avoid multiple includes of this file */

/* <includes/> */

/* <declarations> <!-- declarations do NOT allocate memory --> */
/* <defines/> */

/* <typedefs/> */

/* <prototypes> <!-- these functions are public --> */
   int Dbug( void);
   int DbugHex( unsigned char* bytes, int len);
   /* </prototypes> */

/* <variables> */
   extern char cLogBuf[ 256];
   /* </variables> */
   /* </declarations> <!-- declarations do NOT allocate memory --> */

#endif  /* #ifndef _DEBUG_H */
/* <!-- eof Debug.h --> */
