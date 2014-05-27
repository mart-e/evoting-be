//=============================================================================
/*

   File        : GENLANG.H
   Creation    : 01/01/93

   Author      : B.S.S.

   Description : GENLANG.CPP header file

*/
//=============================================================================
/*h*/

#ifndef _GENLANG                      // avoid mutiple inclusions of this file
   #define _GENLANG

//=========================== C O N S T A N T S ===============================

const int N_LANG =  3;
const int NONE   = -1;
const int ASK    = -2;

//=========================== P R O T O T Y P E S =============================

int Working_Language( int Language = ASK);

#ifdef _APPLICATION_URN

void Init_Languages( char * Param_Language);

#endif

#ifdef _APPLICATION_MAV

void Init_Languages( char * Param_Language, char * Param_Adm_Language);

#endif
#endif                                              // end of #ifndef _GENLANG

//=== GENLANG.H ================================================================
