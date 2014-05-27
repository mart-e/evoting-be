no_tot_1 = STRING(  YEAR(   TODAY), "9999")
         + STRING( MONTH(   TODAY),   "99")
         + STRING(   DAY(   TODAY),   "99").
no_tot_2 = STRING(  YEAR( verkdat), "9999")
         + STRING( MONTH( verkdat),   "99")
         + STRING(   DAY( verkdat),   "99").
call ADDTOLOG PROGRAM-NAME(1) VALUE(no_tot_1 + " < " + no_tot_2 + " ?") 0.
IF no_tot_1 < no_tot_2
THEN DO:
    call ADDTOLOG PROGRAM-NAME(1) "TOTALISATION not allowed before election day !" 0.
    {call.i &prg="mesfil" &param=",'digivote,no_tot_1',0,OUTPUT no_tot_1"}
    {call.i &prg="mesfil" &param=",'digivote,no_tot_2',0,OUTPUT no_tot_2"}
    SUBSTR( no_tot_1, INDEX( no_tot_1, "@@TODAY"), 7) = STRING( TODAY, "99/99/9999").
    SUBSTR( no_tot_2, INDEX( no_tot_2, "@@E-DAY"), 7) = STRING( verkdat, "99/99/9999").
    {call.i &prg="disfram" &param=",',' + no_tot_1
                                   ,',' + no_tot_2
                                   ,'digivote,no_tot_3'
                                   ,''
                                   ,TRUE"}
    {return.i}
END.
