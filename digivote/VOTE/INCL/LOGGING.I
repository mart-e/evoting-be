/* Filename            : LOGGING.I                                      */

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES                                                  */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR debugmode   AS I INIT 0                        NO-UNDO.
DEF {1} SHARED VAR resume-list AS C FORMAT "x(78)"                NO-UNDO.
DEF {1} SHARED VAR key-cancel  AS L                               NO-UNDO.

/* eof */