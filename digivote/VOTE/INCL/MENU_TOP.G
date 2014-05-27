/* Filename            : MENU_TOP.G                                     */


VIEW FRAME {&fra_kader}.
PAUSE 0 NO-MESSAGE.
{call.i &prg="putvinfo"}

{&menu_top} = "".
REPEAT {&menu_pos} = {&menu_start} TO NUM-ENTRIES(optlist):
    {call.i &prg="mesfil" 
            &param=",'{&lang_fixed}' + ENTRY({&menu_pos},optlist),13,
                     OUTPUT {&menu_top}[{&menu_pos}]"}
END.

ASSIGN
{&menu_pos}  = {&menu_start}
{&menu_exec} = 0.

REPEAT WITH FRAME {&fra_menu} NO-HIDE:
    {&show_info}

    HIDE MESSAGE.
    DISPLAY {&menu_top}.

    {&action_pre}
    
    IF {&menu_pos} > 0
    THEN DO :
        IF      NUM-ENTRIES(optlist) = 1
        THEN COLOR DISPLAY VALUE(menu_fg) {&menu_top}[{&menu_start} FOR 1].
        ELSE IF NUM-ENTRIES(optlist) = 2
        THEN COLOR DISPLAY VALUE(menu_fg) {&menu_top}[{&menu_start} FOR 2].
        ELSE IF NUM-ENTRIES(optlist) = 3
        THEN COLOR DISPLAY VALUE(menu_fg) {&menu_top}[{&menu_start} FOR 3].
        ELSE IF NUM-ENTRIES(optlist) = 4
        THEN COLOR DISPLAY VALUE(menu_fg) {&menu_top}[{&menu_start} FOR 4].
        ELSE COLOR DISPLAY VALUE(menu_fg) {&menu_top}[{&menu_start} FOR 5].
        
        COLOR DISPLAY VALUE(menu_bg) {&menu_top}[{&menu_pos}].
        NEXT-PROMPT {&menu_top}[{&menu_pos}].
        IF {&menu_exec} = 0
        THEN DO:
            {call.i &prg="discolme" &param=",'general,fkey02',0"}
        END.
    END.
    IF {&menu_exec} = 0
    THEN DO:
        IF      NUM-ENTRIES(optlist) = 1
        THEN CHOOSE FIELD {&menu_top}[{&menu_start} FOR 1]
                 COLOR VALUE(menu_bg)
                 AUTO-RETURN
                 GO-ON ("CURSOR-RIGHT" "CURSOR-LEFT"
                        "TAB"          "BACK-TAB"
                        "F4"           "ESC"        ).
        ELSE IF NUM-ENTRIES(optlist) = 2
        THEN CHOOSE FIELD {&menu_top}[{&menu_start} FOR 2]
                 COLOR VALUE(menu_bg)
                 AUTO-RETURN
                 GO-ON ("CURSOR-RIGHT" "CURSOR-LEFT"
                        "TAB"          "BACK-TAB"
                        "F4"           "ESC"        ).
        ELSE IF NUM-ENTRIES(optlist) = 3
        THEN CHOOSE FIELD {&menu_top}[{&menu_start} FOR 3]
                 COLOR VALUE(menu_bg)
                 AUTO-RETURN
                 GO-ON ("CURSOR-RIGHT" "CURSOR-LEFT"
                        "TAB"          "BACK-TAB"
                        "F4"           "ESC"        ).
        ELSE IF NUM-ENTRIES(optlist) = 4
        THEN CHOOSE FIELD {&menu_top}[{&menu_start} FOR 4]
                 COLOR VALUE(menu_bg)
                 AUTO-RETURN
                 GO-ON ("CURSOR-RIGHT" "CURSOR-LEFT"
                        "TAB"          "BACK-TAB"
                        "F4"           "ESC"        ).
        ELSE CHOOSE FIELD {&menu_top}[{&menu_start} FOR 5]
                 COLOR VALUE(menu_bg)
                 AUTO-RETURN
                 GO-ON ("CURSOR-RIGHT" "CURSOR-LEFT"
                        "TAB"          "BACK-TAB"
                        "F4"           "ESC"        ).
        
        {&menu_pos} = FRAME-INDEX.
        IF KEYFUNCTION(LASTKEY) = "END-ERROR" OR
           KEYLABEL(LASTKEY)    = "ESC"
        THEN DO:
            {call.i &prg="prog_end"}
        END.
        ELSE IF LASTKEY = KEYCODE("TAB") OR
                LASTKEY = KEYCODE("CURSOR-RIGHT")
        THEN REPEAT:
                 IF {&menu_pos} = NUM-ENTRIES(optlist)
                 THEN {&menu_pos} = {&menu_start}.
                 ELSE {&menu_pos} = {&menu_pos} + 1.
                 /* avoid selecting disabled items */
                 IF {&exec_list}[{&menu_pos}] <> ?
                 THEN LEAVE.
             END.
        ELSE IF LASTKEY = KEYCODE("BACK-TAB")     OR
                LASTKEY = KEYCODE("CURSOR-LEFT")  
        THEN REPEAT:
                 IF {&menu_pos} = {&menu_start}
                 THEN {&menu_pos} = NUM-ENTRIES(optlist).
                 ELSE {&menu_pos} = {&menu_pos} - 1.
                 /* avoid selecting disabled items */
                 IF {&exec_list}[{&menu_pos}] <> ?
                 THEN LEAVE.
             END.
        ELSE {&menu_exec} = {&menu_start}.
    END.
    IF {&menu_exec} = {&menu_start}
    THEN DO :
        {&menu_exec} = 0.
        DISPLAY CAPS({&menu_top}[{&menu_pos}]) @ {&menu_top}[{&menu_pos}].
        PAUSE 0 NO-MESSAGE.
        IF SEARCH( {&exec_list}[{&menu_pos}]) <> ? OR
           LIBRARY({&exec_list}[{&menu_pos}]) <> ?
        THEN DO:
            {call_var.i &prg="{&exec_list}[{&menu_pos}]"}
            {&action_post}
        END.
        ELSE DO :
            {call.i &prg="dispmes" &param=",'general,notavail',0"}
            HIDE MESSAGE NO-PAUSE.
            IF {&menu_pos} = {&menu_items}
            THEN LEAVE.
        END.
    END.
END.
HIDE MESSAGE NO-PAUSE.

HIDE FRAME {&fra_menu} NO-PAUSE.
HIDE FRAME {&fra_kader} NO-PAUSE.

/* eof */