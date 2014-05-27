/* Filename            : SELITEM.G                                      */

/************************************************************************/
/*                                                                      */
/*        LOCAL VARIABLES                                               */
/*                                                                      */
/************************************************************************/
DEF VAR first-key    AS RECID                                     NO-UNDO.
DEF VAR sav-key      AS RECID                                     NO-UNDO.
DEF VAR beg-key      AS RECID                                     NO-UNDO.
DEF VAR end-key      AS RECID                                     NO-UNDO.
DEF VAR counter      AS I                                         NO-UNDO.

/************************************************************************/
/*                                                                      */
/*        START PROCEDURE                                               */
/*                                                                      */
/************************************************************************/
ON ESC END-ERROR.

FIND FIRST {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
IF AVAILABLE {&tab_name}
THEN DO:
    ASSIGN
    first-key = RECID({&tab_name})
    down-num  = 0.
    FOR EACH {&tab_name} WHERE {&where} NO-LOCK:
        down-num = down-num + 1.
    END.

    IF down-num > 13 THEN down-num = 13.

    FIND FIRST {&tab_name} WHERE RECID({&tab_name}) = first-key.
    CLEAR FRAME {&fra_name} ALL NO-PAUSE.
    ASSIGN
    key_dis = {&key_dis}
    sav-key = RECID({&tab_name}).
    DISPLAY key_dis WITH FRAME {&fra_name}.
    DOWN WITH FRAME {&fra_name}.
    PAUSE 0 NO-MESSAGE.

    ASSIGN
    counter = 1
    beg-key = sav-key
    end-key = sav-key.

    REPEAT :
        IF counter = down-num
        THEN LEAVE.
        ELSE DO :
            FIND NEXT {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
            IF NOT AVAILABLE {&tab_name}
            THEN LEAVE.
            ELSE DO:
                ASSIGN
                counter = counter + 1
                key_dis = {&key_dis}
                sav-key = RECID({&tab_name}).
                DISPLAY key_dis WITH FRAME {&fra_name}.
                DOWN WITH FRAME {&fra_name}.
                PAUSE 0 NO-MESSAGE.
                end-key = sav-key.
            END.
        END.
    END.
    UP counter WITH FRAME {&fra_name}.
    FIND {&tab_name} WHERE RECID({&tab_name}) = beg-key NO-LOCK NO-ERROR.
    REPEAT :
        menleav = 0.
        {call.i &prg="discolme" &param=",'general,fkey{&fkey}',0"}

        CHOOSE ROW key_dis COLOR VALUE(menu_bg)
            NO-ERROR AUTO-RETURN WITH FRAME {&fra_name}.
        COLOR DISPLAY VALUE(menu_fg) key_dis WITH FRAME {&fra_name}.
        PAUSE 0 NO-MESSAGE.

        IF LASTKEY = KEYCODE("CURSOR-DOWN")
        THEN DO :
            IF end-key <> first-key
            THEN DO :
                FIND {&tab_name} WHERE RECID({&tab_name}) = end-key NO-LOCK NO-ERROR.
                FIND NEXT {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
                IF AVAILABLE {&tab_name}
                THEN DO:
                    ASSIGN
                    key_dis = {&key_dis}
                    sav-key = RECID({&tab_name}).
                    DOWN WITH FRAME {&fra_name}.
                    PAUSE 0 NO-MESSAGE.
                    DISPLAY key_dis WITH FRAME {&fra_name}.
                    PAUSE 0 NO-MESSAGE.
                    end-key = sav-key.
                    FIND {&tab_name} WHERE RECID({&tab_name}) = beg-key NO-LOCK NO-ERROR.
                    FIND NEXT {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
                    ASSIGN
                    sav-key = RECID({&tab_name})
                    beg-key = sav-key.
                END.
            END.
            NEXT.
        END.

        IF LASTKEY = KEYCODE("CURSOR-UP")
        THEN DO :
            IF beg-key <> first-key
            THEN DO :
                FIND {&tab_name} WHERE RECID({&tab_name}) = beg-key NO-LOCK NO-ERROR.
                FIND PREV {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
                IF AVAILABLE {&tab_name}
                THEN DO:
                    ASSIGN
                    key_dis = {&key_dis}
                    sav-key = RECID({&tab_name}).
                    UP WITH FRAME {&fra_name}.
                    DISPLAY key_dis WITH FRAME {&fra_name}.
                    PAUSE 0 NO-MESSAGE.
                    beg-key = sav-key.
                    FIND {&tab_name} WHERE RECID({&tab_name}) = end-key NO-LOCK NO-ERROR.
                    FIND PREV {&tab_name} WHERE {&where} NO-LOCK NO-ERROR.
                    ASSIGN
                    sav-key = RECID({&tab_name})
                    end-key = sav-key.
                END.
            END.
            NEXT.
        END.
/*
        IF LASTKEY = KEYCODE("CURSOR-LEFT")
        THEN DO :
            menupos = {&cursor_left}.
            execopt = 1.
            LEAVE.
        END.

        IF LASTKEY = KEYCODE("CURSOR-RIGHT")
        THEN DO :
            menupos = {&cursor_right}.
            execopt = 1.
            LEAVE.
        END.
*/
        IF LOOKUP(KEYFUNCTION(LASTKEY),"{&action_keys}") > 0
        THEN DO:
            COLOR DISPLAY VALUE(menu_bg) key_dis
                WITH FRAME {&fra_name}.
            PAUSE 0 NO-MESSAGE.
            ASSIGN
            key_dis = FRAME-VALUE
            {&fra_capture}.

            IF 0{&action_flag} > 0 
            THEN DO:
                IF LOOKUP(KEYFUNCTION(LASTKEY),"{&action_keys}") > NUM-ENTRIES("{&action_prgs}")
                THEN DO:
                    {call.i &prg="edit_epc" &param=",'{&action_type}',
                                                      {&action_flag},
                                                     '{&action_prgs}'"}
                END.
                ELSE DO:
                    {call_var.i &prg="ENTRY(LOOKUP(KEYFUNCTION(LASTKEY),'{&action_keys}'),
                                            '{&action_prgs}')"}
                END.
                IF menleav = 1 
                OR menleav = 9
                THEN LEAVE.
                {&extra_action}
                IF menleav < 0
                THEN DO:
                    key_dis = {&key_dis}.
                    DISPLAY key_dis WITH FRAME {&fra_name}.
                    PAUSE 0 NO-MESSAGE.
                END. 
            END.
            {&action_post}
        END.
        {&extra_keys}
    END.
    PAUSE 0 NO-MESSAGE.
END.
{&action_else}

HIDE FRAME {&fra_name} NO-PAUSE.

ON ESC BELL.

/* eof */