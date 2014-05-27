/* Filename        : PMLPRTGE.I                                         */

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES                                                  */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR key_dis            AS C FORMAT "x(57)" NO-UNDO.
DEF {1} SHARED VAR mes1               AS C FORMAT "x(25)" NO-UNDO.
DEF {1} SHARED VAR mes2               AS C FORMAT "x(20)" NO-UNDO.
DEF {1} SHARED VAR mes3               AS C FORMAT "x(4)"  NO-UNDO.
DEF {1} SHARED VAR down-num           AS I                NO-UNDO.
DEF {1} SHARED VAR men_tit            AS C FORMAT "x(30)" NO-UNDO.

/************************************************************************/
/*                                                                      */
/*    SHARED FRAMES                                                     */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED FRAME pmlprtge.

/************************************************************************/
/*                                                                      */
/*    SHARED/LOCAL FORMS                                                */
/*                                                                      */
/************************************************************************/
FORM
    SKIP
    key_dis AT 2 SPACE(1) SKIP
    HEADER mes1 AT 5 mes2 AT 31 mes3 AT 52
    WITH FRAME pmlprtge SCROLL 1 down-num DOWN ROW 5
    TITLE men_tit NO-LABELS OVERLAY CENTERED.
