/* Filename            : CANDIDAT.G                                     */

{chklevel.i 1}

/************************************************************************/
/*                                                                      */
/*        SHARED VARIABLES  VIA INCLUDE FILES                           */
/*                                                                      */
/************************************************************************/
{mencolor.i}          /* De variabelen voor het kleuren van de schermen */
{vargener.i}          /* Variabelen voor het PRP hoofdmenu PMGENEME     */
{selvar.i}            /* De variabelen voor de selection record         */

/************************************************************************/
/*                                                                      */
/*        LOCAL VARIABLES                                               */
/*                                                                      */
/************************************************************************/
DEF VAR candnr     AS I FORMAT "99"             NO-UNDO.
DEF VAR candnam1   AS C FORMAT "x(22)"          NO-UNDO.
DEF VAR candnam2   AS C FORMAT "x(22)"          NO-UNDO.

DEF VAR mes        AS C FORMAT "x(20)" EXTENT 3 NO-UNDO.
DEF VAR men_tit    AS C FORMAT "x(40)"          NO-UNDO.
DEF VAR wz-trigger AS I                         NO-UNDO.
DEF VAR wz-limit   AS L                         NO-UNDO.
DEF VAR keypres    AS I                         NO-UNDO.
DEF VAR wz-index   AS I INIT 1                  NO-UNDO.
DEF VAR i          AS I                         NO-UNDO.
DEF VAR maxcs      AS I INIT 0                  NO-UNDO.

/************************************************************************/
/*                                                                      */
/*        LOCAL FORM DESCRIPTIONS                                       */
/*                                                                      */
/************************************************************************/
FORM
           candnr                AT 5 
           candnam1              AT 14 
           candnam2              AT 38 SPACE(2)
    HEADER mes[1] FORMAT "x(10)" AT 2 
           mes[2]                AT 14 
           mes[3]                AT 38
    WITH FRAME pmlcanin
        ROW 5
        NO-LABELS
        OVERLAY
        CENTERED
        {&extra_frame}
        TITLE men_tit.

/************************************************************************/
/*                                                                      */
/*        START PROCEDURE                                               */
/*                                                                      */
/************************************************************************/
{&pre_start}

{call.i &prg="mesfil" 
        &param=",'pmlcanin,tit' + STRING(candtyp + 1,'99'),40,OUTPUT men_tit"}
REPEAT i = 1 TO 3:
    {call.i &prg="mesfil" 
            &param=",'pmlcanin,mes' + STRING(i,'99'),20,OUTPUT mes[i]"}
END.

{&extra_init}

FIND FIRST election WHERE election.s-id = verkdat
                      AND election.e-id = verknum
                  NO-LOCK NO-ERROR.
maxcs = 0.
REPEAT i = 1 TO 3:
    IF (election.maxcan[i] + election.maxsup[i]) > maxcs
    THEN maxcs = election.maxcan[i] + election.maxsup[i].
END.

ASSIGN
wz-trigger = (IF election.supps      THEN 64    ELSE 66  )
wz-limit   = (IF maxcs <= wz-trigger THEN FALSE ELSE TRUE).

FIND FIRST setup NO-LOCK NO-ERROR.
{&fill_list}

{&before_repeat}
REPEAT:
    ASSIGN
    menleav   = 0
    keypres   = 0
    candnam1  = ""
    candnam2  = ""
    {&init_repeat}.

    DISPLAY candnr WITH FRAME pmlcanin.
    PAUSE 0 NO-MESSAGE.

    {call.i &prg="discolme" &param=",'general,fkey{&fkey_nbr}',0"}

    {&fetch_candidate}
    
    UPDATE candnam1 candnam2   
        WITH FRAME pmlcanin
        EDITING:
            IF keypres = 1
            THEN DO:
                {call.i &prg="discolme" &param=",'general,fkey04',0"}
            END.
            READKEY.
            IF LASTKEY <> KEYCODE("""")
            THEN DO:
                APPLY LASTKEY.
                keypres = keypres + 1.
            END.
            ELSE BELL.
        END.
    IF candnam1 = ? THEN candnam1 = "?".
    IF candnam2 = ? THEN candnam2 = "?".

    IF candnam1 <> ""
    THEN DO :
        menleav = 1.
        FIND FIRST candidate WHERE candidate.disk-id = setup.disk-id
                               AND candidate.s-id    = verkdat
                               AND candidate.e-id    = verknum
                               AND candidate.p-id    = partnum
                               AND candidate.c-type  = candtyp
                               AND candidate.c-id    = candnr 
                          NO-ERROR.
        IF AVAILABLE candidate
        THEN ASSIGN
             candidate.c_name1 = candnam1
             candidate.c_name2 = candnam2
             candidate.MAC     = "REFRESH".
        ELSE DO :
/*@source sec=1*/
            CREATE candidate.

            /* encrypt an initialised totals counter */
            call CRYPENC PRPTOT fill("0", AESKEYLEN).
            IF crypStat <> 0
            THEN DO:
                /* ToDo : appropriate message and stop/quit */
            END.

            ASSIGN
            candidate.disk-id  = setup.disk-id
            candidate.s-id     = verkdat
            candidate.e-id     = verknum
            candidate.p-id     = partnum
            candidate.c-type   = candtyp
            candidate.c-id     = candnr
            candidate.c_name1  = SUBSTR(candnam1,1,(IF wz-limit THEN 18 ELSE 22))
            candidate.c_name2  = SUBSTR(candnam2,1,(IF wz-limit THEN 18 ELSE 22))
            candidate.vote     = result3
            candidate.tmp_vote = result3
            candidate.MAC      = "REFRESH".
/*@source sec=0*/

            FIND FIRST party WHERE party.disk-id = setup.disk-id
                               AND party.e-id    = candidate.e-id
                               AND party.p-id    = candidate.p-id
                               AND party.p-id   <> 0
                          NO-ERROR.
            IF AVAILABLE party 
            THEN DO:
                wz-index = (IF election.et-id = 1
                            THEN (IF      SUBSTR(party.taalgroep[1],1,1) = "N" THEN 1
                                  ELSE IF SUBSTR(party.taalgroep[1],1,1) = "F" THEN 2
                                  ELSE                                              3)
                            ELSE 1).
                ASSIGN
                candidate.coll-id = party.coll-id
                candidate.MAC     = "REFRESH"
                
                party.num_can     = party.num_can + 1 - candidate.c-type
                party.num_sup     = party.num_sup +     candidate.c-type
                party.MAC         = "REFRESH".
                
                FIND FIRST election WHERE election.e-id = party.e-id
                                  NO-LOCK NO-ERROR.
                IF (    candidate.c-type = 0
                    AND party.num_can    > election.maxcan[wz-index])
                OR (    candidate.c-type = 1
                    AND party.num_sup    > election.maxsup[wz-index])
                THEN DO:
                    {call.i &prg="dispmes" 
                            &param=",'digivote,' + (IF candidate.c-type = 0 
                                                    THEN 'can' 
                                                    ELSE 'sup') + 'max',0"}
                    UNDO,LEAVE.
                END.
            END.
        END.
    END.
    {&leave_repeat}
END. /* REPEAT */
{&after_repeat}

{call.i &prg="sec_cand"}
{call.i &prg="sec_part"}

HIDE FRAME pmlcanin NO-PAUSE.

{return.i}
/* eof */
