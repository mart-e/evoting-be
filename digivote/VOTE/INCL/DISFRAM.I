/* Filename        : DISFRAM.I                  */

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES                                                  */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR messages AS C FORMAT "x(65)" EXTENT 5 NO-UNDO.

/************************************************************************/
/*                                                                      */
/*    SHARED FORM DESCRIPTIONS                                          */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED FRAME disframe.

FORM
                              SKIP(1)
    messages[1] AT 2 SPACE(1) SKIP
    messages[2] AT 2 SPACE(1) SKIP
    messages[3] AT 2 SPACE(1) SKIP
    messages[4] AT 2 SPACE(1) SKIP(1)
    messages[5] AT 2 SPACE(1) SKIP(1)
    WITH FRAME disframe
        NO-LABELS
        ROW 8
        CENTERED
        OVERLAY.

/* eof */