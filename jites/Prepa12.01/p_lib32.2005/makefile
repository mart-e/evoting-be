#
#
# Liste des dépendences des libraires de Wim dans \P_LIBxx
#
#

#.path.apl=$(PATHAPL)

#MACRO1=@echo $.=$**
MACRO1=@echo $.


#
# 1er niveau de dépendences
#
# Ne dépend que des DLL de Centura et/ou de Windows
#
PATHLIB1=c:\p_lib32


atom.$(EXTOBJCTD):	kernel32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

cstwim.$(EXTOBJCTD):	vt.$(EXTOBJCTD) cstructl.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

dlldph1.$(EXTOBJCTD):	dlldph32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

paques.$(EXTOBJCTD):	dlldph32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

rtf4.$(EXTOBJCTD):
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

sdklib.$(EXTOBJCTD):	kernel32.rll advapi32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

shell.$(EXTOBJCTD):	shell32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

steutl.$(EXTOBJCTD):	steutl32.rll
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@




#
# 2ème niveau de dépendences
#
# Dépend des DLL de Centura et/ou de Windows et
# des fichiers du niveau précédent
#

cstste.$(EXTOBJCTD):	cstwim.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

erreur.$(EXTOBJCTD):	cstwim.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

mailslots.$(EXTOBJCTD):	sdklib.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@


salscript2.$(EXTOBJCTD): stream.$(EXTOBJCTD)



#
# 3ème niveau de dépendences
#
# Dépend des DLL de Centura et/ou de Windows et
# des fichiers des niveaux précédents
#

baseclas.$(EXTOBJCTD):	cstste.$(EXTOBJCTD) qcktabs.$(EXTOBJCTD) ttmngr.$(EXTOBJCTD) erreur.$(EXTOBJCTD) \
		mpr.rll user32.rll kernel32.rll \
		gdi32.rll steutl.$(EXTOBJCTD) shell.$(EXTOBJCTD) mailslots.$(EXTOBJCTD) \
		salextension.$(EXTOBJCTD) excel.$(EXTOBJCTD) rtf4.$(EXTOBJCTD) \
		WinSDKCst.$(EXTOBJCTD)  WinSDKFct.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@


registry.$(EXTOBJCTD):	cstste.$(EXTOBJCTD) shell32.rll registry2.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@


palscript2.$(EXTOBJCTD): vt.$(EXTOBJCTD) winsdkfct.$(EXTOBJCTD) stream.$(EXTOBJCTD)
#	salfmt.$(EXTOBJCTD) salscript2.$(EXTOBJCTD)


#
# 4ème niveau de dépendences
#
# Dépend des DLL de Centura et/ou de Windows et
# des fichiers des niveaux précédents
#

excel.$(EXTOBJCTD):	steutl.$(EXTOBJCTD) dlldph1.$(EXTOBJCTD) sdklib.$(EXTOBJCTD) registry.$(EXTOBJCTD) cstructl.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

steclas.$(EXTOBJCTD):	baseclas.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@

winword.$(EXTOBJCTD):	registry.$(EXTOBJCTD) shell.$(EXTOBJCTD)
#	$(COMPCTD) $(PATHLIB1)\$&.apl $@


#
#
#


cleanp_lib:
	del *.obj
	del rel\*.$(EXTOBJCTD)

allp_lib: atom.$(EXTOBJCTD) cstwim.$(EXTOBJCTD) dlldph1.$(EXTOBJCTD) paques.$(EXTOBJCTD) \
	  rtf4.$(EXTOBJCTD) sdklib.$(EXTOBJCTD) shell.$(EXTOBJCTD) steutl.$(EXTOBJCTD)\
	  cstste.$(EXTOBJCTD) erreur.$(EXTOBJCTD) mailslots.$(EXTOBJCTD) \
	  baseclas.$(EXTOBJCTD) registry.$(EXTOBJCTD) excel.$(EXTOBJCTD)\
	  steclas.$(EXTOBJCTD) winword.$(EXTOBJCTD)





