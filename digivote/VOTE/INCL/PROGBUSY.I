/***************************************************************************/
/*   Filename : PROGBUSY.I                                                 */
/***************************************************************************/

    wz-sign-pos = wz-sign-pos + 1.
    PUT SCREEN ROW 23 COLUMN wz-sign-pos CHR(wz-sign).
    IF wz-sign-pos = 80 
    THEN ASSIGN
         wz-sign-pos = 0
         wz-sign     = (IF wz-sign = 32
                        THEN        176
                        ELSE         32).
    
/***************************************************************************/
