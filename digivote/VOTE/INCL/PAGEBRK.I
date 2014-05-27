/* Filename        : PAGEBRK.I                  */
    /**PAGEBRK.I**********************************************BEGIN**/

    IF layout.page-nbr <> wz-page-nbr
    THEN DO:
        IF wz-page-nbr > 0
        THEN DO:
            PAGE.
            ASSIGN wz-line-nbr = 0.
        END.
        ASSIGN wz-page-nbr = layout.page-nbr.
    END.

    /**PAGEBRK.I************************************************END**/

