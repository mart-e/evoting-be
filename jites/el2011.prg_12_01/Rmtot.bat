REM del c:\el2011.prg\obj\*.obj
set clipper=
set pathbin=clipper5

set lib=c:\%pathbin%\lib;c:\%pathbin%\tools\lib
set include=c:\%pathbin%\include;c:\%pathbin%\tools\include
set obj=c:\%pathbin%\obj;c:\%pathbin%\tools\obj

call c:\clipper5\bin\rmake Totali.rmk
PAUSE
