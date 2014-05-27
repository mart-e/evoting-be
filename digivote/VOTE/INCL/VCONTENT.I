/* Filename            : VCONTENTS.I                                    */

/************************************************************************/
/*                                                                      */
/*        SHARED VARS                                                   */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR vc-disklabel   AS C FORMAT "x(11)"             NO-UNDO.
DEF {1} SHARED VAR vc-creator     AS C                            NO-UNDO.
DEF {1} SHARED VAR vc-originator  AS C                            NO-UNDO.
DEF {1} SHARED VAR vc-destinators AS C                            NO-UNDO.
DEF {1} SHARED VAR vc-e-date      AS C FORMAT "x(10)"             NO-UNDO.

/* eof */