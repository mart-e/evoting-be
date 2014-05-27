/* Filename        : MLBLCONV.I                 */
    /**MLBLCONV.I*********************************************BEGIN**/

    REPEAT WHILE INDEX(wz-lijn,"@[") > 0 :
        ASSIGN
        wz-startpos = INDEX(wz-lijn,"@[")
        wz-label    = SUBSTR(wz-lijn,wz-startpos + 2)
        wz-label    = SUBSTR(wz-label,1,INDEX(wz-label,"]") - 1)
        SUBSTR(wz-lijn,wz-startpos,LENGTH(wz-label) + 3) =
                      ENTRY(wz-exvlg,wz-label).
    END. /* Multi label convert */

    /**MLBLCONV.I***********************************************END**/
