/* ******************************************************************** */
/* Filename            : CHK_MAC.I                                      */
/* -------------------------------------------------------------------- */

IF NOT AVAIL {1}
THEN DO:
    call ADDTOLOG PROGRAM-NAME(1) "Unable to check unavailable {1}-record !" 2.
    {call.i &prg="MyMess" &param=", 'Unable to check unavailable {1}-record !'"}
END.
ELSE DO:
    FIND FIRST _file WHERE _file-name = "{1}" NO-LOCK NO-ERROR.
    IF NOT AVAIL _file
    THEN call ADDTOLOG PROGRAM-NAME(1) VALUE("_file-name '{1}' not found in _file !") 2.
    ELSE DO:
        FIND FIRST _field OF _file WHERE _field-name = "MAC" NO-LOCK NO-ERROR.
        IF NOT AVAIL _field
        THEN call ADDTOLOG PROGRAM-NAME(1) VALUE("_field-name 'MAC' not found in _file '{1}' !") 2.
        ELSE DO:
            OUTPUT TO VALUE(ramdrive + "SECURE.CHK").
            EXPORT {1} EXCEPT MAC.
            OUTPUT CLOSE.
            crypStat = 0.
            CALL CRYPMAC PRPTOT
                 VALUE(ramdrive + "SECURE.CHK")
                 "NULL".
            IF PROGRAM-NAME(4) <> "digtools"
            THEN CALL C_DEL VALUE(ramdrive + "SECURE.CHK").
            IF crypStat <> 0
            THEN DO:
                call ADDTOLOG PROGRAM-NAME(1) VALUE("Unable to check MAC crypStat = " + STRING(crypStat) + " ! ({1})") 2.
                {call.i &prg="MyMess" &param=", 'Unable to check MAC, crypStat = ' + STRING(crypStat) + ' ! ({1})'"}
                IF stop_it = "quit" 
                THEN DO:
                    call ADDTOLOG PROGRAM-NAME(1) "Program will stop here !" 2.
                    {call.i &prg="MyMess" &param=", 'Program will stop here !'"}
                    QUIT. /* Don't use {quit.i &corrupt=FALSE} */
                END.
            END.
            IF {1}.MAC <> formatMAC
            THEN DO:
                call ADDTOLOG PROGRAM-NAME(1) "{1}-record has changed unexpectedly !" 2.
                {call.i &prg="MyMess" &param=", '{1}-record has changed unexpectedly !'"}
                IF stop_it = "quit"
                THEN DO:
                    call ADDTOLOG PROGRAM-NAME(1) "Program will stop here !" 2.
                    {call.i &prg="MyMess" &param=", 'Program will stop here !'"}
                    QUIT. /* Don't use {quit.i &corrupt=FALSE} */
                END.
                ELSE DO:
                    call ADDTOLOG PROGRAM-NAME(1) VALUE("Calculated : " + formatMAC) 1.
                    call ADDTOLOG PROGRAM-NAME(1) VALUE("Expected   : " + {1}.MAC) 1.
                    LEAVE.
                END.
            END.
        END.
    END.
END.

/* ******************************************************************** */
