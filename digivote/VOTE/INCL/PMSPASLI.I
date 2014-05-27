/* Filename            : PMSPASLI.I                                     */

/************************************************************************/
/*                                                                      */
/*        SHARED VARIABLES                                              */
/*                                                                      */
/************************************************************************/

DEFINE {1} SHARED VARIABLE men_tit  AS CHARACTER FORMAT "x(50)".
DEFINE {1} SHARED VARIABLE men_mes1 AS CHARACTER FORMAT "x(5)".
DEFINE {1} SHARED VARIABLE men_mes2 AS CHARACTER FORMAT "x(7)".
DEFINE {1} SHARED VARIABLE men_mes3 AS CHARACTER FORMAT "x(30)".
DEFINE {1} SHARED VARIABLE maxdown  AS INTEGER   FORMAT "9".
DEFINE {1} SHARED VARIABLE rec      AS RECID EXTENT 4.
DEFINE {1} SHARED VARIABLE recpnt   AS INTEGER.

/************************************************************************/
/*                                                                      */
/*        SHARED FORM DESCRIPTIONS                                      */
/*                                                                      */
/************************************************************************/

DEFINE {1} SHARED FRAME pmspasli.

FORM
    paswdest.sys-type AT 7   SPACE(0)
    paswdest.org-type        SPACE(0)
    paswdest.pasw-area       SPACE(0)
    paswdest.pasw-id SKIP
    HEADER men_mes1 AT 2 men_mes2 AT 7 men_mes3 AT 29 SPACE(2)
    WITH FRAME pmspasli ROW 4 NO-LABELS
    TITLE men_tit OVERLAY CENTERED maxdown DOWN.
