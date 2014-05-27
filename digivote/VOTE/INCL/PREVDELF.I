/* Filename : PREVDELF.I */

CALL C_DEL VALUE(ramdrive + "PREVIEW.DAT").
call C_DIR VALUE(ramdrive + "PL??????.DAT") 
           VALUE(ramdrive + "LOGOLIST.DIR").
INPUT FROM VALUE(ramdrive + "LOGOLIST.DIR") NO-ECHO.
REPEAT.
    SET wz-file.
    call C_DEL VALUE(ramdrive + wz-file).
END.
INPUT CLOSE.
call C_DEL VALUE(ramdrive + "LOGOLIST.DIR").
CALL C_DEL VALUE(wz-resfile).

/* eof */