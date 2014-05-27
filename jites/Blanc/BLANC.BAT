del .\obj\*.obj
del .\obj\*.exe
c:\tc\bin\tcc -ml -G -c -Ic:\tc\INCLUDE -G -Z -O blanc.c
\tc\BIN\tlink /X \tc\LIB\c0l blanc,blanc, ,\tc\LIB\cl \tc\LIB\graphics \tc\LIB\emu \tc\LIB\mathl lpmng.lib 
pause
