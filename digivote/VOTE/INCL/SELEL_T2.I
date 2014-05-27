/* Filename            : SELEL_T2.I                                     */
/*
     Check whether all elections of the same totlevel have been
     selected. If not, warn the user, but allow to continue ...
*/
menleav = 1.
FOR EACH election WHERE election.s-id     = verkdat
                    AND election.e-pr     = FALSE
                    AND election.totlevel = wz-totlevel
                NO-LOCK:
    FIND FIRST party OF election WHERE party.p-id <> 0 NO-LOCK NO-ERROR.
    IF AVAILABLE party
    THEN DO:
        {call.i &prg="disfram" &param=",'totsells,mes01','totsells,mes02','',
                                        ',' + election.long-name[taalkode] + ' '
                                            + election.coll_name[taalkode],
                                        TRUE"}
        {call.i &prg="confirm" &param=",'totselyn,mes01',
                                        'totselyn,mes02',
                                        '',
                                        'general,yes','general,no'"}
        IF menleav = 0 THEN LEAVE.
        call ADDTOLOG PROGRAM-NAME(1) "User accepted that following election has NOT been selected :" 0.
        call ADDTOLOG PROGRAM-NAME(1) VALUE(election.long-name[taalkode] + ' ' +
                                            election.coll_name[taalkode]) 0.
    END.            
END.                    
IF menleav = 0 THEN NEXT.
/* eof */