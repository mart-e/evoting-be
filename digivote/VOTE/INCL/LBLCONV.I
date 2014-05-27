/* Filename        : LBLCONV.I                  */
    /**LBLCONV.I**********************************************BEGIN**/
    /* Replace 'standard' labels with 'fixed' values                */
    /****************************************************************/

    LBLCONV{1}:
    REPEAT WHILE INDEX(wz-lijn,"@@") > 0:
        ASSIGN
        wz-convto   = ""
        wz-startpos = INDEX(wz-lijn,"@@")
        wz-label    = SUBSTR(wz-lijn,wz-startpos + 2,2).

        IF      LOOKUP(wz-label,"00,21,22,90,91") > 0
        THEN DO:
            wz-convto =
                IF      wz-label = "00"
                THEN ""
                ELSE IF wz-label = "21"
                THEN setup.sys-type +
                     setup.org-type +
                     setup.areaname +
                     STRING(setup.orginator,"999")
                ELSE IF wz-label = "22"
                THEN STRING(session.s-id,"99/99/9999")
                ELSE IF wz-label = "90"
                THEN STRING(TODAY,"99/99/9999")
                ELSE IF wz-label = "91"
                THEN STRING(TIME,"HH:MM")
                ELSE "".
        END.
        ELSE IF wz-label = "51" /* Description exemplaar ‚‚n  */
        THEN DO:
            IF taalkode <> wz-doc-lang
            THEN DO:
               ASSIGN
                 wz-taalkode = taalkode
                 taalkode    = wz-doc-lang.
            END.
            ELSE wz-taalkode = -1.
            {call.i &prg="mesfil" &param=",'layout,exempl1',0,OUTPUT wz-convto"}
            IF wz-taalkode <> -1
            THEN taalkode = wz-taalkode.
        END.
        ELSE IF wz-label = "52" /* Description exemplaar twee */
        THEN DO:
            IF taalkode <> wz-doc-lang
            THEN DO:
               ASSIGN
                 wz-taalkode = taalkode
                 taalkode    = wz-doc-lang.
            END.
            ELSE wz-taalkode = -1.
            {call.i &prg="mesfil" &param=",'layout,exempl2',0,OUTPUT wz-convto"}
            IF wz-taalkode <> -1
            THEN taalkode = wz-taalkode.
        END.
        ELSE wz-convto = "N/A".

        /* Convert @@label to string                               */
        /* if the label is unknown it will be replaced with blancs */
        SUBSTR(wz-lijn,wz-startpos,4) = wz-convto.
    END. /* REPEAT while index - LBLCONV */

    /**LBLCONV.I************************************************END**/

