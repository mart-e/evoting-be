/* Filename            : CHKLEVEL.I                                     */

/*@source sec=1*/
/************************************************************************/
/* Functionality       : Prevents programs from begin started from the  */
/*                       DOS-prompt using 'pro elect -p ??????'.        */
/*                       Only programs meant to start from DOS-prompt   */
/*                       should have a 'parameter' (of no importance)   */
/*                       in the INCLUDE-statement.                      */
/************************************************************************/
{cryprslt.i}  /* almost every procedure needs this for securing records */
{hlcvars.i}   /* almost every procedure needs this for ramdrive usage   */
{logging.i}   /* almost every procedure needs this for debugging info   */

DEF INPUT-OUTPUT PARAM prgSTAT AS I INIT 0{1}.

ON F2 BELL.

RUN traceprg(prgSTAT, ">>>").

/* ***********************************************
   This makes no sence anymore !!!
   Parameters are NOT allowed at highest level !!!
   ***********************************************
IF "{2}" = "" AND PROGRAM-NAME(2) = ?
THEN DO:
    call ADDTOLOG PROGRAM-NAME(1) VALUE("Permission denied : " + PROGRAM-NAME(1)) 1.
    MESSAGE "No such file or directory (-1)".
    STATUS DEFAULT "Procedure complete. Press space bar to continue.".
    PAUSE NO-MESSAGE.
    {quit.i &corrupt=FALSE}
END.
*/
/*@source sec=0*/
