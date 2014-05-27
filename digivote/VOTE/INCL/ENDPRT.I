/* Filename            : ENDPRT.I                                       */

/* Force automatic printing of last (incomplete) page */
IF      printer = 1                /* HP LaserJet 6L  */
THEN PUT CONTROL CHR(27) + "&l0H". /* Eject Page      */

OUTPUT CLOSE.

IF Ok2Print = TRUE
THEN DO:
    {call.i &prg="endprt"} 
END.
ELSE CALL C_DEL VALUE("\VOTE\SPOOL\PRINT.JOB").
