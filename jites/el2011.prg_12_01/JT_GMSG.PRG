#include "set.ch"
#include "inkey.ch"
#include "setcurs.ch"
#include "jites.ch"
#include "box.ch"

**********************************************************************
* FONCTION _affecr
* Affichage d'un ‚cran se trouvant dans le fichier msg
**********************************************************************
FUNCTION _affecr
PARAMETERS pcNumero,;     && cl‚ pour l'‚cran
           plFaireOmbre   && mettre une ombre

LOCAL lnOldSelect:=SELECT()

plFaireOmbre:=IIF(plFaireOmbre==NIL,_FALSE,_TRUE)

SELECT jt_msg
SEEK pcNumero
IF .NOT. EOF()
*   KEYBOARD CHR(K_ESC)
   KEYBOARD CHR(K_CTRL_END)   && pas de sauvegarde puisse que mode ‚dition seulement
   MEMOEDIT(msg_ecran&ccLangue,msg_ligdeb,msg_coldeb,msg_ligfin,msg_colfin,.F.,"",81)
   KEYBOARD ""
ENDIF

IF plFaireOmbre
   shadow(msg_ligdeb+1,msg_colfin+1,msg_ligfin,msg_colfin+2)
   shadow(msg_ligfin+1,msg_coldeb+2,msg_ligfin+1,msg_colfin+2)
ENDIF

SELECT(lnOldSelect)
RETURN(NIL)


**********************************************************************
* FONCTION _rechmsg
* Recherche d'un message
**********************************************************************
FUNCTION _rechmsg
PARAMETERS pcNumero,;    && num‚ro du msg
           pnLg,;        && lg du msg … prendre
           pnAPartirDe   && … partir du NiŠme caractŠre

LOCAL lcReturn:=""

IF pnAPartirDe == NIL
   pnAPartirDe := 1
ENDIF

IF jt_msg->(DBSEEK(pcNumero))
   lcReturn := IIF(pnLg==NIL,TRIM(jt_msg->msg_msg&ccLangue);
                   ,SUBSTR(jt_msg->msg_msg&ccLangue,pnAPartirDe,pnLg))
ENDIF

RETURN(lcReturn)

**********************************************************************
* FONCTION _affmsg
* Affichage d'un message
*
* PARAMETRES ==> pnNumLig : num‚ro de ligne
*                pnNumCol : num‚ro de colonne
*                pcStrAff : string … afficher
*                pcAligne : alignement G = gauche                 AM_ALIGN_GA
*                                      D = droite                 AM_ALIGN_DR
*                                      C = centr‚                 AM_ALIGN_CE
*                                      M = milieu ABSOLU          AM_ALIGN_MI
*                plEffLig : effacer la ligne avant affichage
*                            O = toute la ligne                    AM_EFF_OUI
*                            N = pas d'effacement                  AM_EFF_NON
*                            P = … partir de pnNumCol jusqu'en 79  AM_EFF_PARTIEL
*                            C = de pnNumCol … 79-pnNumCol         AM_EFF_CENTRE
*                pcCouleur : couleur du msg
* remarque : . si p_numcol <> 0 pour p_aligne = D
*                 => p_numcol est soustrait au r‚sultat du calcul d'align.
*            . si p_numcol <> 0 pour p_aligne = C
*                 => p_numcol est ajout‚ au r‚sultat du calcul du centrage
*            . si p_numcol <> 0 pour p_aligne = M
*                 => p_numcol pas pris en compte pour le calcul du centrage,
*                    sert pour l'effacement
**********************************************************************
FUNCTION _affmsg
PARAMETERS pnNumLig,;
           pnNumCol,;
           pcStrAff,;
           pcAligne,;
           plEffLig,;
           pcCouleur

LOCAL lcOldCoul:=SETCOLOR()

IF pcAligne ==  NIL .OR. pcAligne == ""
   pcAligne = AM_ALIGN_GA
ENDIF

IF pcCouleur <> NIL
   SETCOLOR(pcCouleur)
ENDIF

IF plEffLig <> NIL
   IF plEffLig == AM_EFF_OUI
      @ pnNumLig,00 SAY SPACE(80)
   ELSEIF plEffLig == AM_EFF_PARTIEL
      @ pnNumLig,pnNumCol SAY SPACE(80-pnNumCol)
   ELSEIF plEffLig == AM_EFF_CENTRE
      @ pnNumLig,pnNumCol SAY SPACE(80-(2*pnNumCol))
   ENDIF
ENDIF

IF pcAligne == AM_ALIGN_GA
   @ pnNumLig,pnNumCol SAY pcStrAff
ELSEIF pcAligne == AM_ALIGN_DR
   @ pnNumLig,80-LEN(pcStrAff)-pnNumCol SAY pcStrAff
ELSEIF pcAligne == AM_ALIGN_CE
   @ pnNumLig,ROUND((80-LEN(pcStrAff))/2+.49,0)+pnNumCol SAY pcStrAff
ELSEIF pcAligne == AM_ALIGN_MI
   @ pnNumLig,(80-LEN(pcStrAff))/2 SAY pcStrAff
ENDIF

SETCOLOR(lcOldCoul)
RETURN(NIL)


******************************************************
* FONCTION _msgerr
* Permet d'afficher un message d'erreur
*
******************************************************
FUNCTION _msgerr
PARAMETERS pcNumero,;    && num‚ro du msg
           pnLigDeb ,;   && nø de ligne d‚but
           pnColDeb ,;   && nø de colonne d‚but
           pnLigFin ,;   && nø de ligne fin
           pnColFin ,;   && nø de colonne fin
           pcCadre  ,;   && chaine pour dessiner le cadre et son int‚rieur
           pcColorIn,;   && Couleur de l'‚criture
           pcColorOut    && Couleur du cadre

  LOCAL lcOldCoul := SETCOLOR() ;;
        lcMsg001  := TRIM(_rechmsg('JT_GMSG001'))

  PRIVATE mcMessage, lnTouche

  IF pcCadre == NIL
    pcCadre := B_DOUBLE_SINGLE // "ÉÍ»º¼ÍÈº "
  ENDIF

  IF pcColorIn == NIL
    pcColorIn := _COLOR_ERROR
  ENDIF

  IF pcColorOut == NIL
    pcColorOut := _COLOR_ERROR
  ENDIF

  IF jt_msg->(DBSEEK(pcNumero))
    IF pnLigDeb == NIL
      pnLigDeb := jt_msg->msg_LigDeb
      pnColDeb := jt_msg->msg_ColDeb
      pnLigFin := jt_msg->msg_LigFin
      pnColFin := jt_msg->msg_ColFin
    ENDIF
    @ pnLigDeb, pnColDeb, pnLigFin, pnColFin BOX pcCadre COLOR pcColorIn
    @ pnLigDeb+1, pnColDeb+1 SAY PADC(lcMsg001 +' ' + SUBSTR(pcNumero,4),(pnColFin-pnColdeb)-1) COLOR pcColorIn
    SETCOLOR(pcColorIn)
    KEYBOARD CHR(K_CTRL_END)   && pas de sauvegarde puisse que mode ‚dition seulement
    mcMessage := jt_msg->msg_ecran&ccLangue
    MEMOEDIT(mcMessage ,pnLigDeb+2, pnColDeb+1, pnLigFin-1, pnColFin-1, _FALSE,"",81)
    KEYBOARD ""
    lnTouche = INKEY(0)
    IF lnTouche = K_ALT_M
      mcMessage := MEMOEDIT(mcMessage ,pnLigDeb+2, pnColDeb+1, pnLigFin-1, pnColFin-1, _TRUE,"",81)
      DBSELECTAREA("jt_msg")
      of_reclock(0)
      REPLACE msg_ecran&ccLangue WITH mcMessage
      UNLOCK
    ENDIF
  ELSE
    IF jt_msg->(DBSEEK('JT_GEN001'))  // Message qui indique que le message d'erreur n'est pas trouv‚
      @ jt_msg->msg_LigDeb, jt_msg->msg_ColDeb, jt_msg->msg_LigFin, jt_msg->msg_ColFin BOX pcCadre COLOR pcColorIn
      @ jt_msg->msg_LigDeb+1, jt_msg->msg_ColDeb+1 SAY PADC(lcMsg001 +' ' + pcNumero,(jt_msg->msg_ColFin-jt_msg->msg_Coldeb)-1) COLOR pcColorIn
      SETCOLOR(pcColorIn)
      KEYBOARD CHR(K_CTRL_END)   && pas de sauvegarde puisse que mode ‚dition seulement
      mcMessage := jt_msg->msg_ecran&ccLangue
      MEMOEDIT(mcMessage, jt_msg->msg_LigDeb+2, jt_msg->msg_ColDeb+1, jt_msg->msg_LigFin-1, jt_msg->msg_ColFin-1, _FALSE,"",81)
      KEYBOARD ""
      lnTouche = INKEY(0)
      IF lnTouche = K_ALT_M
        mcMessage := MEMOEDIT(mcMessage, jt_msg->msg_LigDeb+2, jt_msg->msg_ColDeb+1, jt_msg->msg_LigFin-1, jt_msg->msg_ColFin-1, _TRUE,"",81)
        DBSELECTAREA("jt_msg")
        of_reclock(0)
        REPLACE msg_ecran&ccLangue WITH mcMessage
        UNLOCK
      ENDIF
    ENDIF
  ENDIF

  SETCOLOR(lcOldCoul)

RETURN(NIL)

FUNCTION _Message
  PARAMETERS pcNumero,;    && num‚ro du msg
             plArret ,;    && Une touche pour continuer
             pcString      && String affich‚e si pas de nø msg

  PRIVATE lcOldColor ,leWinBuff ,llReturn

  lcOldColor := SETCOLOR()
  leWinBuff  := SAVESCREEN(23,0,24,79)

  SETCOLOR(_COLOR_MENU)
  @ 23,01 CLEAR TO 24,78
  Beep()
  IF EMPTY(pcNumero)
    _AffMsg(23, 0, pcString, AM_ALIGN_CE)
  ELSE
    _AffMsg(23, 0, _RechMsg(pcNumero) , AM_ALIGN_CE)
  ENDIF
  IF plArret
    _AffMsg(24,00, _RechMsg('JT_GMSG002') , AM_ALIGN_CE) // Appuyez sur une touche pour continuer
    INKEY(0)
  ENDIF

  SETCOLOR(lcOldColor)
  RESTSCREEN(23,0,24,79,leWinBuff)

RETURN(NIL)

******************************************************
* FONCTION _GetRep
* Input d'une r‚ponse utilisateur
*
******************************************************
FUNCTION _GetRep
  PARAMETER PcNumero, PlBox
  LOCAL lcRep     := _CHAINE_VIDE    ,;
        lnRep     := 0               ,;
        lcOldCoul := SETCOLOR()      ,;
        lcValid   := _CHAINE_VIDE    ,;
        lnColDeb  := 0               ,;
        lnColFin  := 0               ,;
        lcMsg     := _CHAINE_VIDE    ,;
        lcSaveBox

  plBox := IIF( plBox == NIL, _TRUE, plBox)

  IF jt_msg->(DBSEEK(pcNumero))
    lcRep     := RIGHT(jt_msg->msg_rep&ccLangue,1)
    lcValid   := TRIM(LEFT(jt_msg->msg_rep&ccLangue,LEN(jt_msg->msg_rep&ccLangue)-1))
    lcMsg     := TRIM(jt_msg->msg_msg&ccLangue)
    lnColDeb  := INT( (80 - (LEN(lcMsg)+4)) / 2)
    lnColFin  := lnColdeb + LEN(lcMsg) + 5
    SETCOLOR(_COLOR_BLANC_BLEU)
    IF plBox
      lcSaveBox := SAVESCREEN(jt_msg->msg_LigDeb, lnColDeb, jt_msg->msg_LigFin, lnColFin)
      @ jt_msg->msg_LigDeb, lnColDeb, jt_msg->msg_LigFin, lnColFin BOX SPACE(9) COLOR _COLOR_MENU
      @ jt_msg->msg_LigDeb+1, lnColDeb+2, jt_msg->msg_LigFin-1, lnColFin-2 BOX SPACE(9)
      @ jt_msg->msg_LigDeb+1, lnColDeb+2 SAY lcMsg GET lcRep PICTURE '!' VALID (lcRep $ lcValid)
    ELSE
      @ jt_msg->msg_LigDeb, jt_msg->msg_ColDeb SAY lcMsg GET lcRep PICTURE '!' VALID (lcRep $ lcValid)
    ENDIF
    READ
    IF LASTKEY() # K_ESC
      lnRep := AT( lcRep , lcValid)
    ENDIF
    IF plBox
      RESTSCREEN(jt_msg->msg_LigDeb, lnColDeb, jt_msg->msg_LigFin, lnColFin, lcSaveBox)
    ENDIF
    SETCOLOR(lcOldCoul)
  ENDIF
RETURN lnRep
******************************************************
* FONCTION _SayMsg
* Affiche un message
*
******************************************************
FUNCTION _SayMsg
  PARAMETER pcNumero, plSaveScreen, peEcran

  LOCAL lcOldCoul := SETCOLOR()
  PRIVATE leEcran := ''

  plSaveScreen := IIF(plSaveScreen == NIL, _FALSE, _TRUE)


  IF jt_msg->(DBSEEK(pcNumero))
    IF plSaveScreen
      leEcran := SAVESCREEN(jt_msg->msg_LigDeb, jt_msg->msg_ColDeb, jt_msg->msg_LigFin, jt_msg->msg_ColFin )
    ENDIF
    IF peEcran == NIL
      SETCOLOR(_COLOR_LIG1)
      @ jt_msg->msg_LigDeb, jt_msg->msg_ColDeb, jt_msg->msg_LigFin, jt_msg->msg_ColFin BOX SPACE(9)  //  COLOR _COLOR_MENU
      KEYBOARD CHR(K_CTRL_END)   && pas de sauvegarde puisse que mode ‚dition seulement
      MEMOEDIT(jt_msg->msg_ecran&ccLangue, jt_msg->msg_LigDeb+1, jt_msg->msg_ColDeb+1, jt_msg->msg_LigFin-1, jt_msg->msg_ColFin-1, _FALSE,"",81)
      KEYBOARD ""
    ELSE
      RESTSCREEN(jt_msg->msg_LigDeb, jt_msg->msg_ColDeb, jt_msg->msg_LigFin, jt_msg->msg_ColFin, peEcran)
    ENDIF
    SETCOLOR(lcOldCoul)
  ENDIF
RETURN(leEcran)

*++++++++++++++++
* Centre un titre
*++++++++++++++++
FUNCTION _centre
  PARAMETER pn_coldeb, pn_colfin, pn_lentit
RETURN pn_coldeb + INT((pn_colfin-pn_coldeb - pn_lentit-1) / 2)

******************************************************
* FONCTION _box
* Permet de dessiner une BOX.
* On dessine progressivement la BOX en passant par
* d'autres qui lui sont centr‚es.
******************************************************
FUNCTION _box
PARAMETERS pnLigDeb,;   && nø de ligne d‚but
           pnColDeb,;   && nø de colonne d‚but
           pnLigFin,;   && nø de ligne fin
           pnColFin,;   && nø de colonne fin
           pcCadre,;    && chaine pour dessiner le cadre et son int‚rieur
           pcEffet,;    && "L" effet sur la lig, "C" effet sur la col, " " sur les 2
           pnTempo      && dur‚e de la boucle de tempo pour voir l'effet

LOCAL lnI:=1,lnNbrBoucle,lnCentre,lnPas,lnJ

IF pcCadre == NIL
   pcCadre := "ÉÍ»º¼ÍÈº "
ENDIF

IF pcEffet == NIL
   pcEffet := " "
ENDIF

IF pnTempo == NIL
   pnTempo := cnTempoBox
ENDIF


* nbr de boucle … effectuer
lnNbrBoucle := ROUND((pnLigFin-pnLigDeb+1)/2+0.49,0)
* nø de la colonne "centre" de la fenˆtre
lnCentre := pnColDeb + ((pnColFin-pnColDeb)/2)
* pas d'incr‚mentation de la ligne
lnPas := ((pnColFin-pnColDeb)/2) / lnNbrBoucle

DO WHILE lnI <= lnNbrBoucle
   ln_lig_deb := pnLigDeb+lnNbrBoucle - lnI

   * dessiner le cadre
   IF pcEffet == " "   && effet sur la ligne et la colonne
      @ ln_lig_deb,;
      lnCentre-lnPas*lnI,;
      MIN(ln_lig_deb+2*lnI-1,pnLigFin),;
      lnCentre+lnPas*lnI BOX pcCadre
   ELSEIF pcEffet == BOX_EFFET_LIG   && effet sur la ligne
      @ ln_lig_deb,;
      pnColDeb,;
      MIN(ln_lig_deb+2*lnI-1,pnLigFin),;
      pnColFin BOX pcCadre
   ELSEIF pcEffet == BOX_EFFET_COL   && effet sur la colonne
      @ pnLigDeb,;
      lnCentre-lnPas*lnI,;
      pnLigFin,;
      lnCentre+lnPas*lnI BOX pcCadre
   ELSEIF pcEffet == BOX_EFFET_BAS   && effet en gardant les coordon‚es ligdeb, coldeb et colfin FIGEES
      @ pnLigDeb,;
      pnColDeb,;
      MIN(pnLigDeb+(lnI*2),pnLigFin),;
      pnColFin BOX pcCadre
   ENDIF

   lnI++

   * boucle de temporisation
   FOR lnJ = 1 TO IIF(pcEffet==BOX_EFFET_BAS,pnTempo*1.7,pnTempo)
   NEXT lnJ

ENDDO

* mettre ombre
IF pnLigFin<>24 .AND. pnColFin<78
   shadow(pnLigDeb+1,pnColFin+1,pnLigFin,pnColFin+2)
   shadow(pnLigFin+1,pnColDeb+2,pnLigFin+1,pnColFin+2)
ENDIF
RETURN(NIL)


******************************************************
* FONCTION _box2
* Permet de dessiner une BOX et son ombre
******************************************************
FUNCTION _box2(pnLigDeb,;   && nø de ligne d‚but
               pnColDeb,;   && nø de colonne d‚but
               pnLigFin,;   && nø de ligne fin
               pnColFin,;   && nø de colonne fin
               pcCadre)     && chaine pour dessiner le cadre et son int‚rieur

IF pcCadre == NIL
   pcCadre := "ÉÍ»º¼ÍÈº "
ENDIF

@ pnLigDeb,pnColDeb,pnLigFin,pnColFin BOX pcCadre
IF pnLigFin<>24 .AND. pnColFin<78
   shadow(pnLigDeb+1,pnColFin+1,pnLigFin,pnColFin+2)
   shadow(pnLigFin+1,pnColDeb+2,pnLigFin+1,pnColFin+2)
ENDIF
RETURN(NIL)

*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
*³ Dessine une ombre ³
*ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
  FUNCTION shadow (nUp,nLeft,nDown,nRight)
   RESTSCREEN(nUp,nLeft,nDown,nRight,;
       TRANSFORM(SAVESCREEN(nUp,nLeft,nDown,nRight),;
       REPLICATE("X",(nRight-nLeft+1)*(nDown-nUp+1))))
  RETURN NIL

******************************************************
* FONCTION _beep
* sonnette
******************************************************
FUNCTION _beep
TONE(100,1)
RETURN(NIL)

*************************************************
* FONCTION _waitkey
* Attendre qu'on appuie sur une touche
* RETURN : Touche enfonc‚e en CHR
*************************************************
FUNCTION _waitkey(pcStr)   && str des touches permises

LOCAL lcChoix := ""

IF pcStr == NIL
   SET(_SET_CONSOLE,_OFF)
   WAIT
   SET(_SET_CONSOLE,_ON)
ELSE
   DO WHILE AT(lcChoix,pcStr+CHR(K_ESC)) == 0
      SET(_SET_CONSOLE,_OFF)
      WAIT
      SET(_SET_CONSOLE,_ON)
      lcChoix := UPPER(CHR(LASTKEY()))
   ENDDO
ENDIF
RETURN(lcChoix)


*************************************************
* FONCTION _caltempo
* d‚terminer le temps pour _box
*************************************************
FUNCTION _caltempo

LOCAL lnI:=0,lnDebSec:=SECONDS()

DO WHILE lnDebSec+1 > SECONDS()
   lnI++
ENDDO
RETURN(20000/lnI)



*************************************************
* FONCTION _gesPrompt
* gestion d'un prompt
*************************************************
FUNCTION _gesPrompt(pcCode,;         && Code dans MSG.DBF
                    pnLigDeb,;       && Ligne D‚but des Prompts
                    pnColPrompt,;    && Colonne D‚but des Prompts
                    plRestoreScr,;   && TRUE si restore screen
                    pnChoixDeb,;     && nø de choix actif
                    pnLg1Prompt,;    && Lg d'un Prompt
                    pnNbrChoix)      && Nbr de Prompts

LOCAL lnI,lnChoix,lnOldCursor:=SETCURSOR(),lcOldCoul:=SETCOLOR(_COLOR_TRAV1),;
      lcSavEcr,lcSavLig24:=SAVESCREEN(24,0,24,79),;
      lnOldSelect:=SELECT(),lnMaxLg,pcCodeOld

pcCodeOld := pcCode
plRestoreScr := IIF(plRestoreScr==NIL,_TRUE,plRestoreScr)
lnChoix := IIF(pnChoixDeb==NIL,1,pnChoixDeb)

IF pnNbrChoix == NIL .OR. pnLg1Prompt == NIL
   pnNbrChoix := lnMaxLg := 0
   SELECT jt_msg
   DBSEEK(pcCode,_ON)
   DO WHILE LEFT(msg_numero,LEN(pcCode)) == pcCode .AND. !EOF()
      lnMaxLg := MAX(lnMaxLg,LEN(TRIM(msg_msg&ccLangue)))
      pnNbrChoix++   && compteur de choix
      SKIP
   ENDDO
   SELECT(lnOldSelect)
   pnLg1Prompt := IIF(pnLg1Prompt==NIL,lnMaxLg+1,pnLg1Prompt)
ENDIF

pnColPrompt := IIF(pnColPrompt==NIL,ROUND((80-pnLg1Prompt-4)/2+.49,0),pnColPrompt)

lcSavEcr:=SAVESCREEN(pnLigDeb,pnColPrompt,pnLigDeb+pnNbrChoix+2;
                     ,pnColPrompt+pnLg1Prompt+3)

@ pnLigDeb,pnColPrompt TO pnLigDeb+pnNbrChoix+1,pnColPrompt+pnLg1Prompt+1
DISPBEGIN()
_affmsg(24,0,_RechMsg("JT_CHOIX"),AM_ALIGN_CE,AM_EFF_OUI,_COLOR_MENU)
FOR lnI = 1 TO pnNbrChoix

* Modif 8/2/99 AP
    IF pnNbrChoix >= 10 .AND. lnI <  10
                pcCodeOld := pcCode
                pcCode := pcCode + "0"
    ENDIF
* Fin Modif 8/2/99 AP

    @ pnLigDeb+lnI,pnColPrompt+1 PROMPT _rechmsg(pcCode+LTRIM(STR(lnI)),pnLg1Prompt)

* Modif 8/2/99 AP
    pcCode := pcCodeOld
* Fin Modif 8/2/99 AP
NEXT lnI
DISPEND()
MENU TO lnChoix
SETCURSOR(lnOldCursor)
SETCOLOR(lcOldCoul)
IF plRestoreScr
   RESTSCREEN(pnLigDeb,pnColPrompt,pnLigDeb+pnNbrChoix+2;
              ,pnColPrompt+pnLg1Prompt+3,lcSavEcr)
ENDIF
RESTSCREEN(24,0,24,79,lcSavLig24)
RETURN(lnChoix)

