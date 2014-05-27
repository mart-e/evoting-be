/* Filename : PARTNAME.I */
IF partnaam = ? THEN partnaam = "?".
ASSIGN
wz-tel-chars     = 0
wz-tel-non-chars = 0.
REPEAT wz-cur-pos = 1 TO LENGTH(partnaam):
    IF (CAPS(SUBSTR(partnaam,wz-cur-pos,1)) >= "A" AND
        CAPS(SUBSTR(partnaam,wz-cur-pos,1)) <= "Z"    ) OR
       (     SUBSTR(partnaam,wz-cur-pos,1)  >= "0" AND
             SUBSTR(partnaam,wz-cur-pos,1)  <= "9"    )
    THEN wz-tel-chars     = wz-tel-chars     + 1.
    ELSE wz-tel-non-chars = wz-tel-non-chars + 1.
END.
verder = IF wz-tel-chars     > 12 OR
            wz-tel-non-chars > 13
         THEN 0
         ELSE 1.
IF verder = 0
THEN DO:
    {call.i &prg="disfram" &param=",'pmlprtin,err_name','','','',TRUE"}
END.
/* eof */