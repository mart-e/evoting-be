/* Filename : INS_CAND.G */

{chklevel.i 1}

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES  VIA INCLUDE FILES                               */
/*                                                                      */
/************************************************************************/
{mencolor.i}          /* De variabelen voor het kleuren van de schermen */
{vargener.i}          /* Variabelen voor het PRP hoofdmenu PMGENEME     */
{selvar.i}            /* De variabelen voor de selection record         */

/************************************************************************/
/*                                                                      */
/*    LOCAL VARIABLES                                                   */
/*                                                                      */
/************************************************************************/
DEF VAR endnum    AS I FORMAT "99"                                NO-UNDO.
DEF VAR selnum    AS I FORMAT "99"                                NO-UNDO.

/************************************************************************/
/*                                                                      */
/*    EXTERNAL PROCEDURES                                               */
/*                                                                      */
/*    selecwr   Wegschrijven selektie record                            */
/*    crcand    Verschillende kandidaten/opvolgers toevoegen            */
/*    addcand   Toevoegen van een kandidaat/opvolger                    */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/*                                                                      */
/*    START PROCEDURE                                                   */
/*                                                                      */
/************************************************************************/
{call.i &prg="selecwr"}    /* Wegschrijven selectie record 'selection'  */

FIND FIRST setup NO-LOCK NO-ERROR.
FIND LAST candidate WHERE candidate.disk-id = setup.disk-id
                      AND candidate.s-id    = verkdat
                      AND candidate.e-id    = verknum
                      AND candidate.p-id    = partnum
                      AND candidate.c-type  = candtyp NO-ERROR.
IF AVAILABLE candidate
THEN DO :
    ASSIGN
    endnum = candidate.c-id
    selnum = candnum.
    IF endnum = 99
    THEN DO :
        {call.i &prg="dispmes" &param=",'pmlcange,ermes' + STRING(candtyp + 1,'99'),0"}
        HIDE MESSAGE NO-PAUSE.
    END.
    ELSE DO :
        {&append_cand}
        trans-block:
        DO TRANSACTION :
            REPEAT ON ENDKEY UNDO trans-block, LEAVE :
                IF {&compare}
                THEN DO :
                    FIND candidate WHERE candidate.disk-id = setup.disk-id
                                     AND candidate.s-id    = verkdat
                                     AND candidate.e-id    = verknum
                                     AND candidate.p-id    = partnum
                                     AND candidate.c-type  = candtyp
                                     AND candidate.c-id    = endnum.
                    ASSIGN
                        candidate.c-id = candidate.c-id + 1
                        candidate.MAC  = "REFRESH"
                        endnum         = endnum - 1.
                    {call.i &prg="sec_cand"}
                END.
                ELSE DO :
                    {&set_candnum}
                    {call.i &prg="addcand"}  /* Toevoegen kandidaat/opvolger */
                    IF menleav = 0
                    THEN UNDO trans-block, LEAVE.
                    ELSE DO :
                        menleav = 0.
                        LEAVE.
                    END.
                END.
            END.
        END.
        {&end_append}
    END.
END.
ASSIGN
candnum = 0
menleav = 1.

{return.i}
/* eof */