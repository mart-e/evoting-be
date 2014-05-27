/* ******************************************************************** */
/* Filename            : MK_MAC.I                                       */
/* -------------------------------------------------------------------- */

IF NOT AVAIL {1}
THEN DO:
    call ADDTOLOG PROGRAM-NAME(1) "Unable to protect unavailable {1}-record !" 2.
    {call.i &prg="MyMess" &param=", 'Unable to protect unavailable {1}-record !'"}
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
            {_MK_MAC.I {1}}
        END.
    END.
END.

/* ******************************************************************** */
