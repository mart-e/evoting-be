/* Filename        : _MK_MAC.I */

OUTPUT TO VALUE(ramdrive + "SECURE.CHK").
EXPORT {1} EXCEPT MAC.
OUTPUT CLOSE.
crypStat = 0.
CALL CRYPMAC PRPTOT
     VALUE(ramdrive + "SECURE.CHK")
     "NULL".
CALL C_DEL VALUE(ramdrive + "SECURE.CHK").
IF crypStat <> 0
THEN DO:
    call ADDTOLOG PROGRAM-NAME(1) VALUE("Unable to calculate MAC crypStat = " + STRING(crypStat) + " ! ({1})") 2.
    {call.i &prg="MyMess" &param=", 'Unable to calculate MAC, crypStat = ' + 
                STRING(crypStat) + ' ! ({1})'"}
    IF stop_it = "quit" 
    THEN DO:
        call ADDTOLOG PROGRAM-NAME(1) "Program will stop here !" 2.
        {call.i &prg="MyMess" &param=", 'Program will stop here !'"}
        RELEASE {1}.
        {quit.i &corrupt=FALSE}
    END.
END.
{1}.MAC = formatMAC.
RELEASE {1}.

/* eof */