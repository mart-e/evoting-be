/* Filename        : TMGENEIN.I                 */

/************************************************************************/
/*                                                                      */
/*        SHARED VARS                                                   */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR diskstatus AS C FORMAT "X(48)"                 NO-UNDO.
DEF {1} SHARED VAR allowprt   AS C FORMAT "X(25)"                 NO-UNDO.

/************************************************************************/
/*                                                                      */
/*        SHARED FRAME                                                  */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED FRAME tmgenein.

FORM
    diskstatus allowprt  AT 54
    WITH FRAME tmgenein 
        NO-LABELS 
        NO-BOX 
        ROW 2 
        COLUMN 2 
        OVERLAY.

/* eof */