/* Filename            : PMGENEKA.I                                     */

/************************************************************************/
/*                                                                      */
/*        LOCAL VARIABLES                                               */
/*                                                                      */
/************************************************************************/
DEF VAR titel-ka   AS C FORMAT "X(78)" NO-UNDO.
DEF VAR tit-mes    AS C FORMAT "X(20)" NO-UNDO.

/************************************************************************/

FIND FIRST setup NO-LOCK NO-ERROR.
{call.i &prg="mesfil" &param=",'pmgeneka,titmes' + setup.sys-type,0,OUTPUT tit-mes"}
titel-ka = "E-VOTING" + " : " + STRING(verkdat,"99/99/9999") 
         + " - " + TRIM(tit-mes) + " " 
         + setup.sys-type 
         + setup.org-type
         + setup.areaname 
         + STRING(setup.orginator,"999").

/************************************************************************/
/*                                                                      */
/*        LOCAL FORMS                                                   */
/*                                                                      */
/************************************************************************/
FORM
    verkben partnum AT 51 partnam AT 54 " " SKIP(18)
    WITH FRAME pmgeneka NO-LABELS ROW 1
    TITLE titel-ka OVERLAY CENTERED.
