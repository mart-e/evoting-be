/* Filename : STARTDIG.I */

FIND FIRST session NO-ERROR.
IF AVAILABLE session
THEN DO:
    IF session.corrupt
    THEN DO:
        call ADDTOLOG PROGRAM-NAME(1) "Database in corrupted mode !" 1.
        call ADDTOLOG PROGRAM-NAME(1) "Use last known good backup to restore ! (DigRest)" 1.
        {call.i &prg="MyMess" &param=", 'Database not reliable ! Use last backup ! (cfr. logfile)'"}
        call C_DEL VALUE(ramdrive + "VOTE\TRACE.LOG\RESUME").
        {quit.i &corrupt=TRUE}
    END.
END.
ELSE DO:
    {call.i &prg="MyMess" &param=", 'Disk not properly configured ! No session found ! (cfr. logfile)'"}
    {quit.i &corrupt=TRUE}
END.
ASSIGN
debugmode       = INT(session.debug)
session.corrupt = TRUE
session.MAC     = "REFRESH".
{call.i &prg="sec_sess"}

/* eof */