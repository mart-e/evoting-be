/* Filename            : HLCVARS.I                                      */

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES                                                  */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR printer   AS I NO-UNDO. /* C_PRTYPE                  */
DEF {1} SHARED VAR stat      AS I NO-UNDO. /* C_PRSTAT, C_PRINT         */
DEF {1} SHARED VAR fileok    AS I NO-UNDO. /* C_EXEC, C_TSTDSK, C_TSTFN */
DEF {1} SHARED VAR ramdrive  AS C FORMAT "x(3)"  NO-UNDO.
DEF {1} SHARED VAR temp      AS C FORMAT "x(40)" NO-UNDO.
DEF {1} SHARED VAR memAvail  AS I NO-UNDO.
DEF {1} SHARED VAR diskfree  AS I NO-UNDO.
DEF {1} SHARED VAR filesize  AS I NO-UNDO.
DEF {1} SHARED VAR dsklbl    AS CHAR FORMAT "x(11)" NO-UNDO.

/* eof */