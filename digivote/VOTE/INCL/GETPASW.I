/*@function sec=1*/
/* Filename        : GETPASW.I                 */
/*
==============================================================================

   File      : GetPasw.i

   Project   : E-VOTING

   Function  : Include file for receiving passwords.

   Parameters: {1} is the command (SET, UPDATE),
               {2} is the name of the password variable,
               {3} is the name of the password form.

   Usage     : {GetPasw.i "set" "oldpass" "paswcha"}.

   Remarks   : the variable cPassword that acts as a keyboard buffer has
               been defined in the file cryprslt.i .

               When the user aborts editing this variable becomes a flag, so
               it is possible to check how the edit ended.

==============================================================================
*/

DO:
    ASSIGN
    {2}       = ""
    cPassword = "".
    /* dynamic SET/UPDATE phrase */
    DISPLAY {2} WITH FRAME {3}.
    {1} {2} WITH FRAME {3}
    EDITING:
        READKEY. /* read key pressed */
        /* is key pressed one of the allowed characters for passwords ? */
        IF ( ( KEYCODE(KEYFUNCTION(LASTKEY)) >= KEYCODE("0") AND
               KEYCODE(KEYFUNCTION(LASTKEY)) <= KEYCODE("9")    ) OR
             ( KEYCODE(KEYFUNCTION(LASTKEY)) >= KEYCODE("A") AND
               KEYCODE(KEYFUNCTION(LASTKEY)) <= KEYCODE("Z")    ) OR
             ( KEYCODE(KEYFUNCTION(LASTKEY)) >= KEYCODE("a") AND
               KEYCODE(KEYFUNCTION(LASTKEY)) <= KEYCODE("z")    )   ) 
        THEN DO:
            IF LENGTH(cPassword) < AESKEYLEN 
            THEN DO: /* Build password from input and display "*" in mask. */
                 cPassword = cPassword + CAPS(KEYFUNCTION(LASTKEY)).
                 APPLY KEYCODE("*").
            END.
            ELSE BELL. /* sorry, we do not accept more characters than AESKEYLEN ! */
        END.
        /* is key pressed one of the editing keys we 'customized' ? */
        ELSE IF KEYFUNCTION(LASTKEY) = "BACKSPACE"
        THEN DO:
            IF LENGTH(cPassword) > 0 
            THEN DO: /* remove last character */
                cPassword = SUBSTRING(cPassword, 1, (LENGTH(cPassword) - 1)).
                APPLY LASTKEY.
            END.
            ELSE BELL. /* sorry, nothing more to remove ! */
        END.
        ELSE IF LOOKUP(KEYFUNCTION(LASTKEY), "END-ERROR,ESC,F4") > 0 
        THEN DO: /* user aborts editing process, no password is returned here */
            ASSIGN
            cPassword = "ABORTED"
            {2}       = cPassword.
            APPLY LASTKEY.
        END.
        /* is key pressed one of the 'standard' editing keys we allow */
        ELSE IF LOOKUP(KEYFUNCTION(LASTKEY), "TAB,BACK-TAB,GO,RETURN") > 0 AND
             LENGTH(cPassword) = AESKEYLEN
        THEN APPLY LASTKEY. /* user is leaving edit field               */
        ELSE BELL.          /* sorry, these keys are not allowed here ! */
    END. /* editing: */
END. /* do: */
{2} = cPassword.

/* :WARNING: avn 20020118                            */ 
/* no return here because we're in an include file ! */
/*                        eof                        */
