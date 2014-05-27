PROJ = URNE
PROJFILE = URNE.MAK
DEBUG = 0

LIBDES = .
LIBCOM = .

PWBRMAKE  = $(DRIVE)\BIN\pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = $(DRIVE)\BIN\nmake
CC  = $(DRIVE)\BIN\cl
CFLAGS_G  = /AL /W2 /Ox /BATCH
CFLAGS_D  = /qc /Gi$(PROJ).mdt /Zr /Zi /Od
CFLAGS_R  = /Od
ASM  = $(DRIVE)\BIN\masm
AFLAGS_G  = /Mx /T
AFLAGS_D  = /Zi
LINKER  = $(DRIVE)\BINB\link
ILINK  = $(DRIVE)\BINB\ilink
LRF  = echo > NUL
BIND  = $(DRIVE)\BINB\bind
RC  = $(DRIVE)\BINB\rc
IMPLIB  = $(DRIVE)\BINB\implib
LFLAGS_G  = /ST:4192  /BATCH
LFLAGS_D  = /CO /INC /FAR /PACKC
LFLAGS_R  = /EXE /FAR /PACKC
MAPFILE_D  = NUL
MAPFILE_R  = NUL
LLIBS_G  = $(LIBCOM)\lcom

OBJS  = EPROM.obj ERREUR.obj LCD.obj PC-URNE.obj URNE.obj\
	VARIABLE.obj VOTE.obj IVBIT.obj LECTEUR.obj ZEIT.OBJ MCR.obj\
	UI.obj Utt.obj UQMC.obj KLAVIER.obj URNBE.obj
SBRS  = EPROM.sbr ERREUR.sbr LCD.sbr PC-URNE.sbr URNE.sbr \
	VARIABLE.sbr VOTE.sbr IVBIT.obj LECTEUR.sbr ZEIT.OBJ MCR.obj\
	UI.obj UTT.obj UQMC.obj KLAVIER.obj URNBE.obj

all: $(PROJ).exe

.SUFFIXES: .c .sbr .obj

EPROM.obj : EPROM.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\sys\types.h\
	$(DRIVE)\INCLUDE\sys\stat.h $(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\dos.h\
	$(DRIVE)\INCLUDE\memory.h eprom.h

EPROM.sbr : EPROM.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\sys\types.h\
	$(DRIVE)\INCLUDE\sys\stat.h $(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\dos.h\
	$(DRIVE)\INCLUDE\memory.h eprom.h

ERREUR.obj : ERREUR.C defs.h $(DRIVE)\INCLUDE\time.h\
	$(DRIVE)\INCLUDE\bios.h lcd.h erreur.h

ERREUR.sbr : ERREUR.C defs.h $(DRIVE)\INCLUDE\time.h\
	$(DRIVE)\INCLUDE\bios.h lcd.h erreur.h

LCD.obj : LCD.C defs.h $(DRIVE)\INCLUDE\conio.h\
	$(DRIVE)\INCLUDE\stdio.h lcd.h

LCD.sbr : LCD.C defs.h $(DRIVE)\INCLUDE\conio.h\
	$(DRIVE)\INCLUDE\stdio.h lcd.h

PC-URNE.obj : PC-URNE.C defs.h $(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\dos.h \
	$(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h \
	$(DRIVE)\INCLUDE\memory.h $(DRIVE)\INCLUDE\string.h lcd.h eprom.h \
	variable.h erreur.h pc-urne.h cfgscrut.h cfgscrut.ext

PC-URNE.sbr : PC-URNE.C defs.h $(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\dos.h \
	$(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h \
	$(DRIVE)\INCLUDE\memory.h $(DRIVE)\INCLUDE\string.h lcd.h eprom.h \
	variable.h erreur.h pc-urne.h cfgscrut.h cfgscrut.ext

URNE.obj : URNE.C defs.h $(DRIVE)\INCLUDE\ctype.h\
	$(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h\
	$(DRIVE)\INCLUDE\sys\types.h $(DRIVE)\INCLUDE\sys\stat.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\string.h\
	$(DRIVE)\INCLUDE\time.h lcd.h eprom.h\
	variable.h erreur.h vote.h pc-urne.h urne.h

URNE.sbr : URNE.C defs.h $(DRIVE)\INCLUDE\ctype.h\
	$(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h\
	$(DRIVE)\INCLUDE\sys\types.h $(DRIVE)\INCLUDE\sys\stat.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\string.h\
	$(DRIVE)\INCLUDE\time.h lcd.h eprom.h\
	variable.h erreur.h vote.h pc-urne.h urne.h

VARIABLE.obj : VARIABLE.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\stdlib.h eprom.h variable.h

VARIABLE.sbr : VARIABLE.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\stdlib.h eprom.h variable.h

VOTE.obj : VOTE.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\string.h variable.h vote.h\
	cfgscrut.h cfgscrut.ext

VOTE.sbr : VOTE.C defs.h $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\stdlib.h $(DRIVE)\INCLUDE\string.h variable.h vote.h\
	cfgscrut.h cfgscrut.ext

IVBIT.obj : IVBIT.C $(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h\
	$(DRIVE)\INCLUDE\sys\types.h $(DRIVE)\INCLUDE\sys\stat.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h\
	$(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\math.h\
	$(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\string.h

LECTEUR.obj : LECTEUR.C defs.h $(DRIVE)\INCLUDE\dos.h\
	$(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\string.h\
	com.inc variable.h vote.h lcd.h lecteur.h

LECTEUR.sbr : LECTEUR.C defs.h $(DRIVE)\INCLUDE\dos.h\
	$(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\string.h\
	com.inc variable.h vote.h lcd.h lecteur.h
ZEIT.obj : ZEIT.C MCR.h $(DRIVE)\INCLUDE\conio.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\string.h $(DRIVE)\INCLUDE\dos.h\
	mcr.h
ZEIT.sbr : ZEIT.C MCR.h $(DRIVE)\INCLUDE\conio.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\string.h $(DRIVE)\INCLUDE\dos.h\
	mcr.h
MCR.obj : MCR.C \
	$(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\string.h\
	dzdef.h uqmc.h ui.h mcr.h utt.h
MCR.sbr : MCR.C \
	$(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\conio.h $(DRIVE)\INCLUDE\string.h\
	dzdef.h uqmc.h ui.h mcr.h utt.h

UI.obj : UI.C $(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\string.h $(DRIVE)\INCLUDE\conio.h\
	ui.h
UI.sbr : UI.C $(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\string.h $(DRIVE)\INCLUDE\conio.h\
	ui.h

UTT.obj : UTT.C $(DRIVE)\INCLUDE\dos.h utt.h
UTT.sbr : UTT.C $(DRIVE)\INCLUDE\dos.h utt.h 


UQMC.obj : UQMC.C $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\string.h uqmc.h 

UQMC.sbr : UQMC.C $(DRIVE)\INCLUDE\stdio.h\
	$(DRIVE)\INCLUDE\string.h uqmc.h 

URNBE.obj : URNBE.C $(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h\
	$(DRIVE)\INCLUDE\sys\types.h $(DRIVE)\INCLUDE\sys\stat.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h\
	$(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\math.h\
	$(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\string.h urnbe.h

URNBE.sbr : URNBE.C $(DRIVE)\INCLUDE\fcntl.h $(DRIVE)\INCLUDE\io.h\
	$(DRIVE)\INCLUDE\sys\types.h $(DRIVE)\INCLUDE\sys\stat.h\
	$(DRIVE)\INCLUDE\stdio.h $(DRIVE)\INCLUDE\stdlib.h\
	$(DRIVE)\INCLUDE\io.h $(DRIVE)\INCLUDE\math.h\
	$(DRIVE)\INCLUDE\dos.h $(DRIVE)\INCLUDE\string.h urnbe.h

$(PROJ).bsc : $(SBRS)
	$(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(OBJS)
!IF $(DEBUG)
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
	$(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
!IF $(DEBUG)
	$(ILINK) -a -e "$(LINKER) @$(PROJ).lrf" $@
!ELSE
	$(LINKER) @$(PROJ).lrf
!ENDIF


.c.sbr :
!IF $(DEBUG)
	$(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
	$(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.c.obj :
!IF $(DEBUG)
	$(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
	$(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF


run: $(PROJ).exe
	$(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
	CV $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)

