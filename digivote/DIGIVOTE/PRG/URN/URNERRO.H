//=======================================================================
/*

   File        : URNERRO.H
   Creation    : 05/02/93

   Author      : B.S.S.

   Description : Typedefs and structures for URN Error handling

*/
//=============================================================================
/*h*/

#ifndef _URNERRO                      // avoid mutiple inclusions of this file
   #define _URNERRO

//======================== I N C L U D E   F I L E S ==========================

#include <setjmp.h>

//============================ T Y P E D E F S ================================

typedef enum { RBP_STARTUP
             , RBP_BEFORE_FORMATTED
             , RBP_AFTER_FORMATTED
             , RBP_BEFORE_IN_BAC
             , RBP_AFTER_IN_BAC
             , RBP_CLOSE_OFF

} ROLLBACK_POINT;               // symbolic names for rollback state indicator


//=================== E X T E R N A L   V A R I A B L E S =====================

extern ROLLBACK_POINT Rollback_Point;

extern jmp_buf JMP_STARTUP;

extern jmp_buf JMP_BEFORE_FORMATTED;
extern jmp_buf JMP_AFTER_FORMATTED;

extern jmp_buf JMP_BEFORE_IN_BAC;
extern jmp_buf JMP_AFTER_IN_BAC;

extern jmp_buf JMP_CLOSE_OFF;


#endif                                              // end of #ifndef _URNERRO

//=== URNERRO.H ===============================================================
