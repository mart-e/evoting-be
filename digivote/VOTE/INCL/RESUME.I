/* 
   Filename  : RESUME.I

   Purpose   : Automatically launch programs ...
   
   ATTENTION : Need a LOCAL VAR on_resume  DEF AS L with INIT FALSE
                      LOCAL VAR prglvl     DEF AS I
                      LOCAL VAR resume-prg DEF AS C
   
*/
IF NUM-ENTRIES(resume-list) > 0
THEN DO:
    prglvl = 2.
    DO WHILE PROGRAM-NAME(prglvl + 1) <> ?:
        prglvl = prglvl + 1.
    END.
    IF prglvl < NUM-ENTRIES(resume-list)
    THEN IF ENTRY(prglvl,resume-list) = PROGRAM-NAME(1)
         THEN ASSIGN
              resume-prg = ENTRY(prglvl + 1,resume-list)
              on_resume  = TRUE.
         ELSE on_resume = FALSE.
    ELSE on_resume = FALSE.
END.
ELSE on_resume = FALSE.

IF NOT on_resume
THEN ASSIGN
     resume-list = ""
     prglvl      = 1.

/* eof */