/* Filename            : PRINTER.I                                      */

DEF {1} VAR Ok2Print       AS L                        NO-UNDO.

DEF {1} VAR prt-lines      AS I                        NO-UNDO.
DEF {1} VAR prt-page       AS L                        NO-UNDO.

DEF {1} VAR PagNbrY        AS L INIT True              NO-UNDO.
DEF {1} VAR PagNbrN        AS L INIT False             NO-UNDO.

DEF {1} VAR PagHeader      AS CHARACTER FORMAT "x(75)" NO-UNDO.
DEF {1} VAR SubHeader      AS CHARACTER FORMAT "x(75)" NO-UNDO.
DEF {1} VAR AllSpaces      LIKE PagHeader              NO-UNDO.

DEF {1} VAR wz-lijn        AS C    FORMAT "x(90)"      NO-UNDO.
DEF     VAR wz-startpos    AS I                        NO-UNDO.
DEF     VAR wz-label       AS C                        NO-UNDO.
DEF     VAR wz-convto      AS C                        NO-UNDO.
DEF     VAR wz-resvpos     AS I                        NO-UNDO.
DEF     VAR wz-exvlg       AS I                        NO-UNDO.
DEF     VAR wz-taalkode    AS I                        NO-UNDO.

/* workzones for table 'usage' */
DEF {1} VAR wz-doc-type    LIKE elect.usage.doc-type   NO-UNDO.
DEF {1} VAR wz-setup-id    LIKE elect.usage.setup-id   NO-UNDO.
DEF {1} VAR wz-et-combi    LIKE elect.usage.et-combi   NO-UNDO.
DEF {1} VAR wz-et-ids      LIKE elect.usage.et-ids     NO-UNDO.
DEF {1} VAR wz-doc-id      LIKE elect.usage.doc-id     NO-UNDO.
DEF {1} VAR wz-doc-lang    LIKE elect.usage.doc-lang   NO-UNDO.

/* workzones for table 'layout' */
DEF {1} VAR wz-page-nbr    LIKE elect.layout.page-nbr  NO-UNDO.
DEF {1} VAR wz-line-nbr    LIKE elect.layout.line-nbr  NO-UNDO.
DEF {1} VAR wz-page-number AS I                        NO-UNDO.

/* used in LINEPRT.I to detect if wz-line-nbr should be incremented or not */
DEF     VAR plus           AS I INIT 1                 NO-UNDO.
DEF     VAR plus0          AS I INIT 0                 NO-UNDO.
DEF     VAR plus1          AS I INIT 1                 NO-UNDO.

/* used in REPLLBL.I to replace a label with length different from 8       */
DEF     VAR repl           AS I INIT 8                 NO-UNDO.
DEF     VAR repl4          AS I INIT 4                 NO-UNDO.

/************************************************************************/
/*                                                                      */
/*   LOCAL FORMS                                                        */
/*                                                                      */
/************************************************************************/
FORM
    " " SKIP " " SKIP(2)
    WITH FRAME nopagprt NO-BOX NO-LABELS NO-UNDERLINE PAGE-TOP.

FORM
    HEADER " " SKIP "PG" AT 1 PAGE-NUMBER FORMAT "999" AT 3
                              PagHeader                AT 6 
                              SubHeader                AT 6 SKIP(2)
    WITH FRAME dopagprt NO-BOX NO-LABELS NO-UNDERLINE PAGE-TOP.

/************************************************************************/
/*                                                                      */
/*   INITIAL FILL ...                                                   */
/*                                                                      */
/************************************************************************/
AllSpaces = FILL(" ",75).
