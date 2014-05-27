/*    Filename : PREVIEW.I */ 
/*  
      Show PREVIEW of a specific party (list of candidates) 
*/

    DISPLAY verkben partnum partnam WITH FRAME pmgenein.
    PAUSE 0 NO-MESSAGE.
    {call.i &prg="disfram" &param=",'digivote,indwait1','','','',FALSE"}
    {call.i &prg="{&preview_dat}" &param=",ramdrive + 'PREVIEW.DAT'"}
    /*@call=../../digivote/prg/divers/preview.cpp*/
    HIDE FRAME disframe NO-PAUSE.
    HIDE FRAME pmgeneka NO-PAUSE.
    HIDE FRAME pmgenein NO-PAUSE.
    HIDE FRAME pmgeneme NO-PAUSE.
    wz-params = {&prg_params}.
    call ADDTOLOG PROGRAM-NAME(1) VALUE(wz-params) 0.
    
    call C_DEL VALUE(wz-resfile).
    DOS SILENT VALUE(wz-exefile) VALUE(wz-params).
    
    PUT CURSOR OFF.
    PAUSE 0 NO-MESSAGE.
    VIEW FRAME pmgeneka.
    PAUSE 0 NO-MESSAGE.
    DISPLAY verkben partnum partnam WITH FRAME pmgenein.
    PAUSE 0 NO-MESSAGE.
    {call.i &prg="putvinfo"}
    VIEW FRAME pmgeneme.
    PAUSE 0 NO-MESSAGE.
    {&extra}
    call C_TSTFN VALUE(wz-resfile).
    IF fileok = 0
    THEN DO:
        {call.i &prg="interror" &param=",'pmgeneme,opt2', 
                     'internal,errmsg',
                     ''"}
        IF "{&if_init}" = "" 
        THEN DO:
            {call.i &prg="initverk"} /* Initialisatie verkiezing */
            verknum = 1.
        END.
        LEAVE.
    END.

/* eof */