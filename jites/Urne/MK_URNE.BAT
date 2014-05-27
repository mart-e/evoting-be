del *.obj
del *.exe
SET LIB=c:\c600\LIB
SET INCLUDE=c:\c600\INCLUDE
c:\c600\BIN\nmake  DRIVE=c:\c600 urne.mak
copy *.obj .\obj\
del *.obj
pause
