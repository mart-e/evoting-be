/* Filename : QUIT.I */

FIND FIRST elect.session NO-ERROR.

ASSIGN 
elect.session.corrupt = {&corrupt}
elect.session.MAC     = (IF  dbmasterk  <> "-1"
                         AND dbsessionk <> "-1"
                         THEN "REFRESH"
                         ELSE "CORRUPT").
/* Don't secure session-record if AES is not active !! */                         
IF elect.session.MAC = "REFRESH"
THEN DO:
    {call.i &prg="sec_sess"}
END.
{call.i &prg="cfmake"}

RUN traceprg(prgSTAT,"END"). /* DON't use {call.i} ! Use RUN instead ! */
RUN cleanup.                 /* Don't use {call.i} ! Use RUN instead ! */
/*@source sec=1*/
call CRYPSTOP.                         /* stop AES-engine */
/*@source sec=0*/

CALL C_DEL "\VOTE\SPOOL\PRINT.JOB".

/************************************************************************/
/*                                                                      */
/*        ON STATEMENTS VIA INCLUDE FILES                               */
/*                                                                      */
/************************************************************************/
{CURDUORG.I}

HIDE ALL NO-PAUSE.

QUIT.

/* eof */