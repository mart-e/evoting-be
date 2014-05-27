/* Filename        : REPLLBL.I                  */
/**REPLLBL.I**********************************************BEGIN**/

    IF wz-resvpos = 0
    THEN DO:
        ASSIGN
        SUBSTR(wz-lijn,wz-startpos,repl{1}) = " "
        SUBSTR(wz-lijn,wz-startpos,1)       = wz-convto.
    END.
    ELSE DO:
        IF LENGTH(wz-convto) <> wz-resvpos
        THEN DO:
            ASSIGN
            wz-convto = IF LENGTH(wz-convto) > wz-resvpos
                        THEN SUBSTR(wz-convto,1,wz-resvpos)
                        ELSE wz-convto + FILL(" ",wz-resvpos -
                                                  LENGTH(wz-convto)).
        END.
        ASSIGN
        SUBSTR(wz-lijn,wz-startpos,repl{1})    = SUBSTR(AllSpaces,1,repl{1})
        SUBSTR(wz-lijn,wz-startpos,wz-resvpos) = wz-convto.
    END.

    /**REPLLBL.I************************************************END**/
