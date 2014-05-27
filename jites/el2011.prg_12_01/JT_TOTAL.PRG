  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'jites.ch'

/*
        ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
        บ                    Logiciel  J I T E S                     บ
        บ                                                            บ
        บ  Totalisation des votes                                    บ
        ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

FUNCTION jtTotPc
  PARAMETER llTypMsg
  LOCAL lnRecBurVotes
  PRIVATE ll_Flag, nFin, nCur, lcWinbuff,lc_Rep, llMajStat, llTotFait
  PRIVATE lcCrypt
  lcCrypt=PassWord + PassWordElect

  llTypMsg := IIF( llTypMsg == NIL, _TRUE, llTypMsg )

  STRFILE( 'Totalisation en cours ... '+DTOC(DATE())+ ' '+TIME(), _PATH_JITES+'\TOTALISA.int')

  DBCOMMIT()

  DBSELECTAREA('PARAM')
  GO TOP

  SET CURSOR OFF
  IF llTypMsg
    _AffMsg(23,02, _RechMsg('JT_TOTAL002')) // Totalisation en cours. Un instant S.V.P. ...
  ELSE
    _AffMsg(23,02, _RechMsg('JT_TOTAL008')) // Un instant S.V.P. ...
  ENDIF

  DBSELECTAREA('SCRUTINS')
  GO TOP
  REPLACE ALL s_nbr_vbe WITH 0
  REPLACE ALL s_nbr_vbn WITH 0

  DBSELECTAREA('LISTES')
  GO TOP
  REPLACE ALL l_nbr_v_tl WITH 0
  REPLACE ALL l_nbr_v_sp WITH 0
  REPLACE ALL l_nbr_v_ef WITH 0
  REPLACE ALL l_nbr_v_ex WITH 0

  DBSELECTAREA('CANDIDATS')
  GO TOP
  REPLACE ALL c_nbr_v_ca WITH 0

  DBSELECTAREA('RECUPVOTES')
  GO TOP
  nFin := RECCOUNT()
  nCur := 0
  GO TOP
  DO WHILE .NOT. EOF()
    IF rv_numlist=="XX"           && vote blanc
      DBSELECTAR('SCRUTINS')
      IF DBSEEK( RECUPVOTES->rv_codescr)
      lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
        IF RECUPVOTES->rv_numcand == "001"
          REPLACE s_nbr_vbe WITH s_nbr_vbe + lnNbrVote
        ELSEIF RECUPVOTES->rv_numcand == "002"
          REPLACE s_nbr_vbn WITH s_nbr_vbn + lnNbrVote
        ENDIF
      ENDIF
    ELSEIF rv_numcand == "000"      && tete de liste
      DBSELECTAR('LISTES')
      IF DBSEEK(RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist)
        lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
        DO CASE
          CASE RECUPVOTES->rv_eff_sup == "A"
            REPLACE l_nbr_v_ex WITH l_nbr_v_ex + lnNbrVote
          CASE RECUPVOTES->rv_eff_sup == "E"
            REPLACE l_nbr_v_tl WITH l_nbr_v_tl + lnNbrVote
          CASE RECUPVOTES->rv_eff_sup == "S"
            REPLACE l_nbr_v_sp WITH l_nbr_v_sp + lnNbrVote
          CASE RECUPVOTES->rv_eff_sup == "F"
            REPLACE l_nbr_v_ef WITH l_nbr_v_ef + lnNbrVote
        ENDCASE
      ENDIF
    ELSE
      DBSELECTAR('CANDIDATS')
      IF DBSEEK(RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist;
                +RECUPVOTES->rv_eff_sup+RECUPVOTES->rv_numcand)
        lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
        REPLACE c_nbr_v_ca WITH c_nbr_v_ca + lnNbrVote
      ENDIF
    ENDIF
    DBSELECTAR('RECUPVOTES')
    DBSKIP()
  ENDDO
  COMMIT

  IF !llTypMsg // Il y a eu interruption lors de la totalisation

    DBSELECTAREA('BURVOTES')
    lnRecBurVotes := RECNO()
    GO TOP
    DO WHILE .NOT. EOF()
      IF b_numtot == 0
        of_reclock(0)
        REPLACE b_numtot with 1
        UNLOCK
      ENDIF
      DBSKIP()
    ENDDO
    DBGOTO(lnRecBurVotes)
    DBCOMMIT()
    SETCOLOR(_COLOR_MENU)
    _AffMsg(23,02, '', AM_ALIGN_CE, AM_EFF_OUI ) // Efface la ligne 23
  ENDIF

  DELETEFILE( _PATH_JITES + '\totalisa.int' )

RETURN(NIL)
/*ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
  ณ impression des tableaux rcapitulatifs par liste  ณ
  ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู*/
FUNCTION ImpReca

  PRIVATE lc_Rep, lcEcran, ln_i, xs_nom, xs_record, lnJ

  DECLARE xs_nom[5],xs_record[5]

  lcEcran := SAVESCREEN(18,00,24,79)
  SETCOLOR(_COLOR_MENU)
  @ 23,01 CLEAR TO 24,78
  @ 18,06,21,73 BOX B_SINGLE +  ' '
  STORE SPACE(1) TO lc_Rep
  SET CURSOR ON
  @ 19,10 SAY _RechMsg( 'JT_TOTAL007' ) ;
          GET lc_rep PICT "!" VALID(AT(lc_rep,"ON")<>0)  // "Confirmez l'impression des tableaux rcapitulatifs (O/N) :"
  READ
  SET CURSOR OFF
  IF LASTKEY() # K_ESC .AND. lc_Rep == "O"
    IF testimp(_FALSE)
      DBSELECTAREA('SCRUTINS')
      GO TOP

      ln_i := 0
      DO WHILE .NOT. EOF() .AND. ln_i <= 5
        IF .NOT. EMPTY(s_nomf)
          ln_i++
          IF ccLangue = _NEERLANDAIS
             xs_nom[ln_i]    := s_nomn
          ELSE
             xs_nom[ln_i]    := s_nomf
          ENDIF
          xs_record[ln_i] := RECNO()
        ENDIF
        SKIP
      ENDDO

      FOR lnJ := 1 TO ln_i
        @ 20,10 SAY _RechMsg('JT_TOTAL003')+ ' ' +TRIM(xs_nom[lnJ])+SPACE(10) // Traitement du scrutin :
        ImpRecapi(xs_record[lnJ] )
      NEXT lnJ

    ENDIF
  ENDIF

  RESTSCREEN(18,00,24,79,lcEcran)
  SETCOLOR(_COLOR_TRAV1)

RETURN(NIL)


/*ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ
  ณ Totalisation par bureau  ณ
  ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู*/
FUNCTION jtBurTot
  PARAMETERS pnNbDejaTot,;   && nbr de bureaux dj… totaliss
             pnNbTotBur,;    && nbr total de bureaux
             pnNumBurTrait   && n๘ du bureau … traiter

  PRIVATE llReturn:=_FALSE
  PRIVATE lcCrypt
  lcCrypt=PassWord + PassWordElect

  STRFILE( 'Totalisation en cours ... '+DTOC(DATE())+ ' '+TIME(), _PATH_JITES+'\TOTALISA.int')

  DBCOMMIT()

  IF BURVOTES->(DBSEEK(pnNumBurTrait))

     IF pnNbDejaTot == 0   && pas encore de bureaux totaliss => RAZ
        DBSELECTAREA('SCRUTINS')
        REPLACE ALL s_nbr_vbe WITH 0
        REPLACE ALL s_nbr_vbn WITH 0

        DBSELECTAREA('LISTES')
        REPLACE ALL l_nbr_v_tl WITH 0
        REPLACE ALL l_nbr_v_sp WITH 0
        REPLACE ALL l_nbr_v_ef WITH 0
        REPLACE ALL l_nbr_v_ex WITH 0

        DBSELECTAREA('CANDIDATS')
        REPLACE ALL c_nbr_v_ca WITH 0
        COMMIT
     ENDIF

     pnNbDejaTot++

     DBSELECTAREA('RECUPVOTES')
     DBSEEK(STRZERO(pnNumBurTrait,3),_ON)
     DO WHILE rv_numbur == pnNumBurTrait .AND. !EOF()

        IF rv_numlist=="XX"           && vote blanc
           DBSELECTAR('SCRUTINS')
           IF DBSEEK(RECUPVOTES->rv_codescr)
              lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
              IF RECUPVOTES->rv_numcand == "001"
                 REPLACE s_nbr_vbe WITH s_nbr_vbe + lnNbrVote
              ELSEIF RECUPVOTES->rv_numcand == "002"
                 REPLACE s_nbr_vbn WITH s_nbr_vbn + lnNbrVote
              ENDIF
           ENDIF
        ELSEIF rv_numcand == "000"      && tete de liste
           DBSELECTAR('LISTES')
           IF DBSEEK(RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist)
              lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
              DO CASE
                 CASE RECUPVOTES->rv_eff_sup == "A"
                   REPLACE l_nbr_v_ex WITH l_nbr_v_ex + lnNbrVote
                 CASE RECUPVOTES->rv_eff_sup == "E"
                   REPLACE l_nbr_v_tl WITH l_nbr_v_tl + lnNbrVote
                 CASE RECUPVOTES->rv_eff_sup == "S"
                   REPLACE l_nbr_v_sp WITH l_nbr_v_sp + lnNbrVote
                 CASE RECUPVOTES->rv_eff_sup == "F"
                   REPLACE l_nbr_v_ef WITH l_nbr_v_ef + lnNbrVote
              ENDCASE
           ENDIF
        ELSE
           DBSELECTAR('CANDIDATS')
           IF DBSEEK(RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist;
                     +RECUPVOTES->rv_eff_sup+RECUPVOTES->rv_numcand)
              lnNbrVote =VAL(Crypt(RECUPVOTES->rv_nbrvote,RECUPVOTES->rv_numcand+lcCrypt))
              REPLACE c_nbr_v_ca WITH c_nbr_v_ca + lnNbrVote
           ENDIF
        ENDIF
        DBSELECTAR('RECUPVOTES')

        DBSKIP()
     ENDDO

     DBCOMMIT()

     IF pnNbTotBur == pnNbDejaTot + 1
        DBSELECTAREA('PARAM')
        GO TOP

        of_reclock(0)

        REPLACE p_statut WITH "Rsultat"
        ccEtat    := "R"

        UNLOCK
        ccStatut := LEFT(p_statut,8)
        COMMIT
     ENDIF

     llReturn := _TRUE

  ENDIF

  DBSELECTAREA('BURVOTES')

  DELETEFILE( _PATH_JITES + '\totalisa.int' )

RETURN(llReturn)
