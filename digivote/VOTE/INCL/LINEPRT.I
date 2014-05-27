/* Filename        : LINEPRT.I                  */
    /**LINEPRT.I**********************************************BEGIN**/

    IF plus{1} = 0
    THEN wz-line-nbr = layout.line-nbr.
    ELSE DO:
        wz-line-nbr = wz-line-nbr + plus{1}.

        /* Empty lines are NOT stored in 'layout', therefore we have to */
        /* force them while we print !                                  */
        REPEAT WHILE wz-line-nbr < layout.line-nbr:
            wz-line-nbr = wz-line-nbr + 1.

            PUT UNFORMATTED SKIP(1).
        END.
    END.
    PUT UNFORMATTED "     " wz-lijn SKIP.

    /**LINEPRT.I************************************************END**/

