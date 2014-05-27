/***********************************************************************
**--------------------------------------------------------------------**
*    Fonction       : Permet de suivre le chaŒnage des blocs de        *
*                     m‚moire allou‚s par DOS                          *
**--------------------------------------------------------------------**/


#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <conio.h>

/* START CONFIDENTIAL */  

/*== Typedef =========================================================*/

typedef unsigned int  WORD;
typedef BYTE          BOOLEAN;
typedef BYTE far      *FB;               /* Pointeur FAR sur un octet */

/*== Constantes ======================================================*/


/*== Structures et unions ============================================*/

struct MCB
      {                              /* D‚crit un bloc MCB en m‚moire */
        BYTE id_code; /* 'M' = il existe un bloc qui suit , 'Z' = Fin */
        WORD psp;                           /* Segment du PSP associ‚ */
        WORD distance;              /* Nombre de paragraphes r‚serv‚s */
      };

typedef struct MCB far *MCBPtr;            /* Pointeur FAR sur un MCB */

/*== Macros ==========================================================*/

#ifdef MK_FP                                   /* MK_FP d‚j… d‚fini ? */
  #undef MK_FP
#endif

#define MK_FP(s, o) ((void far *) (((unsigned long) (s)<<16)|(o)))
int Nombre_exe = 0;

MCBPtr first_mcb( void )
{
  union REGS   regs;      
  struct SREGS sregs; 

 regs.h.ah = 0x52;
 intdosx( &regs, &regs, &sregs );           
 return(*((MCBPtr far *) MK_FP( sregs.es -1, regs.x.bx + 12)));
}
/**********************************************************************/
void trace_mcb( void )
{

 MCBPtr   act_mcb;            
 BOOLEAN  fin;                  
 BYTE     nr_mcb,i;
 FB       lptr;                      
 fin     = FALSE;                    
 act_mcb = first_mcb();     
 do                         
 {
     if( act_mcb->id_code == 'Z')   fin = TRUE;                                     
     if (*((unsigned far *) MK_FP( act_mcb->psp, 0 )) == 0x20cd)
                                                   Nombre_exe ++;  
    act_mcb = (MCBPtr) MK_FP( FP_SEG(act_mcb) + act_mcb->distance + 1, 0 );
 }
  while( !fin);
}

/***********************************************************************
**                           PROGRAMME PRINCIPAL                      **
***********************************************************************/
Debut_Mcbc( void )
{
 trace_mcb();                      
 return(Nombre_exe);
}
/* END CONFIDENTIAL */  
