/*@source sec=1*/
/***************************************
 Add 2 additional bytes for length-field
 Add size (align until plural of 16)
 Add 8 additional bytes for MAC-field
***************************************/ 
wz-size-B001 = wz-size-B001
             + 2 
             + wz-size-line + (IF wz-size-line MODULO 16 = 0
                               THEN 0
                               ELSE 16 - ( wz-size-line MODULO 16)
                              )
             + 8.
/*@source sec=0*/          
