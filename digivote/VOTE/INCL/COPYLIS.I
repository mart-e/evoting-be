/***************************************************************************/
/*  Filename : COPYLIS.I                                                   */
/***************************************************************************/

/***************************************************************************/
/*                                                                         */
/*  SHARED VARIABLES                                                       */
/*                                                                         */
/***************************************************************************/
DEF {1} SHARED VAR wz-lab     AS C EXTENT 6                NO-UNDO.
DEF {1} SHARED VAR wz-lscp    AS L EXTENT 4 FORMAT "*/ "   NO-UNDO.
DEF {1} SHARED VAR wz-lst     AS C EXTENT 4 FORMAT "x(50)" NO-UNDO.
DEF {1} SHARED VAR wz-lstok   AS L EXTENT 4 FORMAT "OK/  " NO-UNDO.
DEF {1} SHARED VAR wz-tit     AS C                         NO-UNDO.

/***************************************************************************/
/*                                                                         */
/*  SHARED FRAMES                                                          */
/*                                                                         */
/***************************************************************************/
DEF {1} SHARED FRAME loading.

/***************************************************************************/
/*                                                                         */
/*  LOCAL FORMS                                                            */
/*                                                                         */
/***************************************************************************/
FORM
                                                    SKIP(1)
    wz-lab[1]  FORMAT "x(64)"                       SKIP(1)
    wz-lab[2]  FORMAT "x(64)"                       SKIP(1)
    wz-lab[3]  FORMAT "x(15)"
    wz-lscp[1] COLON 17 wz-lst[1] wz-lstok[1] AT 70
    wz-lscp[2] COLON 17 wz-lst[2] wz-lstok[2] AT 70
    wz-lscp[3] COLON 17 wz-lst[3] wz-lstok[3] AT 70
    wz-lscp[4] COLON 17 wz-lst[4] wz-lstok[4] AT 70 SKIP(1)
    wz-lab[4]  FORMAT "x(64)"                       SKIP
    wz-lab[5]  FORMAT "x(64)"                       SKIP
    wz-lab[6]  FORMAT "x(64)"                       SKIP(1)
    WITH FRAME loading
        CENTERED
        NO-LABELS
        ROW 5
        OVERLAY
        TITLE wz-tit.

