/* START CONFIDENTIAL */  

/***************************************************************************
*
* Functions using BIOS Timer TICK
*
* KC, Apr-92: Creation
*
**********/
#ifndef ulong
#define ulong   unsigned long
#endif

#define UTT_TICK_IN_A_DAY  (1573040L)

ulong utt_get(void);
ulong utt_elapsed( ulong reference_tick);
/* END CONFIDENTIAL */  
