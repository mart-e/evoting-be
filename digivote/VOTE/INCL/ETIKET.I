/* Filename            : ETIKET.I                                      */

/* Shared variabels voor ETIKET.P */

/* 1. Laser HP , 0 = Bull CompuPrint */
DEF {1} SHARED VAR wz-prt           AS I NO-UNDO.

/* Number of label cols */
DEF {1} SHARED VAR wz-labcols       AS I NO-UNDO.
/* Number of label rows on a page */
DEF {1} SHARED VAR wz-labrows       AS I NO-UNDO.
/* Number of skips on a new page */
DEF {1} SHARED VAR wz-skipafterpage AS I NO-UNDO.

/* Print at colum... 12 cpi */
DEF {1} SHARED VAR wz-atnorm        AS I EXTENT 3 NO-UNDO.

/* Print at colum... 17 cpi */
DEF {1} SHARED VAR wz-atsmal        AS I EXTENT 3 NO-UNDO.

/* Max. char. per label [1] 12cpi, [2] 17 cpi*/
DEF {1} SHARED VAR wz-maxcpl        AS I EXTENT 2 NO-UNDO.
/* Max. rows per label */
DEF {1} SHARED VAR wz-maxrpl        AS I NO-UNDO.

/* List with the position of the field on the label*/
DEF {1} SHARED VAR wz-fieldlist     AS C NO-UNDO.
/* Content Fields to be printed */
DEF {1} SHARED VAR wz-field         AS C EXTENT 10 NO-UNDO.
/* Current etiketcolum on row */
DEF {1} SHARED VAR wz-col           AS I NO-UNDO.
/* Current etiketrow on page */
DEF {1} SHARED VAR wz-row           AS I NO-UNDO.

DEF {1} SHARED WORKFILE wfrow NO-UNDO
    FIELD nr    AS I
    FIELD txt   AS C    EXTENT 3.

/* eof */