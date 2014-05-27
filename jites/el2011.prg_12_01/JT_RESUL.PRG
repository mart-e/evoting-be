  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'jites.ch'

/*
        ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
        º                    Logiciel  J I T E S                     º
        º                                                            º
        º  Impression des r‚sultats                                  º
        ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/

FUNCTION jtResul
  PARAMETER plOfficiel
  LOCAL llDisquette:=_TRUE
  plOfficiel := IIF( plOfficiel==NIL, _TRUE, plOfficiel )
  PRIVATE lnChoix

   EcrBase() // Ecran de base

   AffTitre(_RechMsg('JT_RESUL001')) // IMPRESSION DES RESULTATS

  LigStatut() // Ligne 01 STATUT

  SETCOLOR(_COLOR_MENU)
  lnChoix := 1

    SETCOLOR(_COLOR_OMBRE)
    @ 09,24,IIF(llDisquette,13,12),51 BOX REPLICATE(CHR(176),9)
    SETCOLOR(_COLOR_TRAV1)
    @ 08,22,IIF(llDisquette,12,11),49 BOX B_SINGLE +' '
    SETCOLOR(_COLOR_MENU)
    _AffMsg(23,00,_RechMsg('JT_RESUL002') + " "+ _CARACT_CROCHETS + " . " + _RechMsg('JT_RESUL003'), AM_ALIGN_CE ) // Choisissez … l'aide des touches [] . Confirmez avec [<Ù].
    SETCOLOR(_COLOR_TRAV1)
    DO WHILE lnChoix <> 0
      @ 09,23 PROMPT LEFT(_RechMsg('JT_RESUL004')+SPACE(26),26) // " R‚sultats globaux     "
      @ 10,23 PROMPT LEFT(_RechMsg('JT_RESUL005')+SPACE(26),26) // " R‚sultats par scrutin "
      IF llDisquette
         @ 11,23 PROMPT LEFT(_RechMsg('JT_RESUL107')+SPACE(26),26) // " Disquette             "
      ENDIF
      MENU TO lnChoix
      DO CASE
        CASE lnChoix = 1
          ImpTout()
        CASE lnChoix = 2
          ParScrutin()
        CASE lnChoix == 3
          jtRDisk()
      ENDCASE
    ENDDO

RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ impression de TOUS les r‚sultats                  ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ImpTout
  LOCAL lcLangImp,lcOldLangue:=ccLangue, ccMsgColl1
  PRIVATE ln_i, ln_j, ln_k, leEcran, lc_rep, xs_code, xs_nom, xs_record
  DECLARE xs_code[5], xs_nom[5], xs_record[5],;
            xs_max_eff[5]  ,; // Nbre effectifs max. scrutin
            xs_max_sup[5]     // Nbre suppl‚ants max. scrutin

  leEcran := SAVESCREEN(17,00,24,79)
  SETCOLOR(_COLOR_MENU)
  @ 23,01 CLEAR TO 24,78
  @ 17,06,20,73 BOX B_SINGLE + ' '
  STORE SPACE(1) TO lc_rep
  ccMsgColl1 := _RechMsg('JT_RESUL008') // "ProcŠs-verbal"

  IF (lc_rep :=_GetRep( 'JT_RESUL013', _FALSE )) == _REPONSE_OUI

    lcLangImp := LangImp("JT_LANGUE")
    IF testimp(.F.)

      ccLangue := lcLangImp

      DBSELECTAREA('SCRUTINS')
      go top

      ln_i := 1
      DO WHILE (.NOT. EOF()) .AND. ln_i <=5
        IF (.NOT. EMPTY(s_nomf))
          xs_code[ln_i]    := s_code
          IF ccLangue = _NEERLANDAIS
             xs_nom[ln_i]     := s_nomn
          ELSE
             xs_nom[ln_i]     := s_nomf
          ENDIF
          xs_record[ln_i]  := RECNO()
          xs_max_eff[ln_i] := s_max_eff
          xs_max_sup[ln_i] := s_max_sup
          ln_i := ln_i + 1
        ENDIF
        SKIP
      ENDDO

      FOR ln_j:= 1 to ln_i - 1
        IF (xs_code[ln_J] $ _SCRUTIN_CHAMBRE)
          @ 24,02 SAY ccMsgColl1 + space(10) // "ProcŠs-verbal"
          ImpRec(_FALSE, xs_code[ln_j], xs_max_eff[ln_j]+xs_max_sup[ln_j])
        ENDIF
        ImpTot(_FALSE, xs_record[ln_j])
      NEXT ln_j
    ENDIF
    ccLangue := lcOldLangue   // Restaurer la langue de travail
  ENDIF

  RESTSCREEN(17,00,24,79,leEcran)
  SETCOLOR(_COLOR_TRAV1)
RETURN


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Impression des r‚sultats partiels                 ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ImpResPar
  DECLARE xs_code[5], xs_nom[5],xs_max_eff[5], xs_max_sup[5],;
          lnNbScrut, lc_date, lc_heure, lcLibScrutin, lcCodeScru, lnDebut, ln_i,;
          lnNbVoteTot, lnNbBurRecup, xs_vb[4], lnNbTotNul, lnCumulPourc := 0, lnTotScru,;
          lnVoteTotn,lnVoteTote,lnVoteTots,lnTotNuln,lnTotNule,lnTotNuls
* modif
  PRIVATE lnNbVotTC   ,;
          lnNbBuRC    ,;
          lnNbToNC    ,;
          lnNbTBComm  , lnNbVotes, lnNbNul, lnTotScru

  _AffMsg(23,1,_RechMsg( 'JT_RESUL014') ,AM_ALIGN_CE,AM_EFF_CENTRE) // "IMPRESSION DES RESULTATS PARTIELS"
  IF testimp()
    xxLangue:=ccLangue
    ccLangue:=lcLangImp

    DBSELECTAREA('SCRUTINS')
    GO TOP

    lnNbScrut := 0
* <<  phe2009  5 Scru tin ?
    DO WHILE (.NOT. EOF()) .AND. lnNbScrut <= 5

      IF (.NOT. EMPTY(s_nomf))
        lnNbScrut             := lnNbScrut + 1
        xs_code[lnNbScrut]    := s_code
        IF ccLangue = _NEERLANDAIS
           xs_nom[lnNbScrut]  := TRIM(s_nomn)
        ELSE
           xs_nom[lnNbScrut]  := TRIM(s_nomf)
        ENDIF
        xs_max_eff[lnNbScrut] := s_max_eff
        xs_max_sup[lnNbScrut] := s_max_sup
        xs_vb[lnNbScrut]      := s_nbr_vbe +  s_nbr_vbn
      ENDIF
      SKIP
    ENDDO

    * Compte le nombre de bureaux dŠj… r‚cup‚r‚s
    lnNbVoteTot  := 0
    lnVoteTotn   := 0
    lnVoteTote   := 0
    lnVoteTots   := 0

    lnNbBurRecup := 0
    lnNbTotNul   := 0
    lnTotNuln    := 0
    lnTotNule    := 0
    lnTotNuls    := 0

    lnNbVotTC    := 0  // Nb votes pour les communales
    lnNbBuRC     := 0  // Nb de bureaux les communales r‚cup.
    lnNbToNC     := 0  // Nb votes NUL
    lnNbTBComm   := 0
    DBSELECTAREA('BURVOTES')
    GO TOP
    DO WHILE .NOT. EOF()
      IF b_commun == PARAM->p_burdep .OR. PARAM->p_burcommu == _BUR_TOTAL_COMMUNAL
        lnNbTBComm   += 1
      ENDIF
      IF AT(_STAT_BUR_LECTURE, B_STATUT) # 0 .AND. b_numtot # 0
        lnVoteTotn   += b_nbr_votn + b_nbr_nuln
        lnVoteTote   += b_nbr_vote + b_nbr_nule
        lnVoteTots   += b_nbr_vots + b_nbr_nuls
        lnNbVoteTot  = lnVoteTotn + lnVoteTote + lnVoteTots  
        lnNbBurRecup += 1
        lnTotNuln    += b_nbr_nuln
        lnTotNule    += b_nbr_nule 
        lnTotNuls    += b_nbr_nuls 

        IF b_commun == PARAM->p_burdep .OR. PARAM->p_burcommu == _BUR_TOTAL_COMMUNAL
          lnNbVotTC  += b_nbr_votn + b_nbr_nuln + b_nbr_vote+ b_nbr_nule
          IF b_numtot # 0 // Uniquement les bureaux totalis‚s
            lnNbBuRC   += 1
          ENDIF
          lnNbToNC   += b_nbr_nuln + b_nbr_nule
        ENDIF
      ENDIF
      DBSKIP()
    ENDDO

    IF lnNbVoteTot # 0

      IF testimp(.F.)


        * date et heure d‚but impression
        lc_Date  := DTOC(DATE())
        lc_Heure := TIME()

        SET DEVICE TO PRINT
        SET PRINT ON
        SET CONSOLE OFF
        @ PROW(), PCOL() SAY _IMPR_INIT
        FOR ln_i:=1 to lnNbScrut
          lnTotScru := 0
                DO CASE
                     CASE  xs_code[ln_i]==_SCRUTIN_CHAMBRE
                        lnNbVotes := lnVoteTotn
                        lnNbNul   := lnTotNuln 
                     CASE  xs_code[ln_i]==_SCRUTIN_SENAT
                        lnNbVotes := lnVoteTotn
                        lnNbNul   := lnTotNuln
                ENDCASE
                lnNbVoteTot := lnNbVotes
                lnNbTotNul  := lnNbNul

                TeteResPar(xs_code[ln_i], _FALSE)
                lnCumulPourc := 0
                DBSELECTAREA('LISTES')
                DBSEEK(xs_code[ln_i])
                DO WHILE xs_code[ln_i] == l_codescru .AND. (.NOT. EOF())
                  IF PROW() > 60
                    EJECT
                    TeteResPar(xs_code[ln_i], _TRUE)
                  ENDIF

                  IF cgbSup
                    @ PROW() + 1, 04 say _RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ' ;
                      +trans( l_nbr_v_tl                                   , '9999999') + ' ' ;
                      +trans( l_nbr_v_ef                                   , '9999999') + ' ' ;
                      +trans( l_nbr_v_ex-(l_nbr_v_tl+l_nbr_v_ef+l_nbr_v_sp), '9999999') + ' ' ;
                      +trans( l_nbr_v_sp                                   , '9999999') + ' ' ;
                      +trans( l_nbr_v_ex                                   , '9999999')
                  ELSE
                    @ PROW() + 1, 04 say _RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ' ;
                      +trans( l_nbr_v_tl                                   , '9999999') + ' ' ;
                      +trans( l_nbr_v_ef                                   , '9999999') + ' ' ;
                      +trans( l_nbr_v_ex                                   , '9999999')
                  ENDIF
                    lnTotScru := lnTotScru + l_nbr_v_ex
                  lnCumulPourc += ROUND((l_nbr_v_ex/lnNbVotes)*100,2)
                  SKIP
                ENDDO
                IF  cgbSup
                  @ PROW() + 1, 04 say SPACE(LEN(_RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ')+(4*8) ) + ;
                    'ÄÄÄÄÄÄÄ'
                  @ PROW() + 1, 04 say SPACE(LEN(_RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ')+(4*8) ) ;
                    +trans( lnTotScru                                    , '9999999')
                ELSE
                  @ PROW() + 1, 04 say SPACE(LEN(_RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ')+(2*8) ) + ;
                    'ÄÄÄÄÄÄÄ'
                  @ PROW() + 1, 04 say SPACE(LEN(_RechMsg('JT_RESUL030')+' '+l_numero+' '+l_nom   + ' ')+(2*8) ) ;
                    +trans( lnTotScru                                    , '9999999')
                ENDIF
                lnCumulPourc += ROUND((lnNbNul/lnNbVotes)*100,2)
                @ PROW()+2  , 04 SAY _RechMsg( 'JT_RESUL015' )+SPACE(1);
                  +str(xs_vb[ln_i]+lnNbNul ,6) // +' '+_RechMsg('JT_RESUL121')  // "Nombre de votes blancs et de votes nuls" cartes
                EJECT
        NEXT
        SET DEVICE TO SCREEN
        SET PRINT OFF
        SET CONSOLE ON
      ENDIF
    ENDIF
  ENDIF
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Impression de l'entˆte des r‚sultats partiels     ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION TeteResPar
  PARAMETER pcCodeScru, plSuite
  LOCAL lnNbrBurTot
  PRIVATE  lcLibScrutin, lnDebut, lnLgLib

  lcLibScrutin := SPACE(10)
  DO CASE
    CASE pcCodescru == _SCRUTIN_CHAMBRE
      IF ccNumArr == _ARROND_BRUXELLES
        @ PROW()+1,00 SAY _RechMsg('JT_RESUL106') // "Circonscription BHV "
        @ PROW(),00   SAY REPLICATE('_',LEN(_RechMsg('JT_RESUL106'))+1)
      ELSE
        @ PROW()+1,00 SAY _RechMsg('JT_RESUL149')+' '+TRIM(ccArron) // "Circonscription ‚lectorale de
        @ PROW(),00   SAY REPLICATE('_',LEN(_RechMsg('JT_RESUL149'))+LEN(TRIM(ccArron))+1)
      ENDIF
      @ PROW(),55   SAY _RechMsg('JT_RESUL033') +' '+ lc_date         // "Date  : "
      @ PROW()+1,55 SAY _RechMsg('JT_RESUL034') +' '+ lc_heure        // "Heure : "
      @ PROW()+3,00 SAY _RechMsg('JT_RESUL035') +' '+ IIF(ccNumArr==_ARROND_BRUXELLES .AND. ccLangue==_NEERLANDAIS, RechCantN(),PARAM->p_burdep) // "Canton de "
      @ PROW(),00   SAY REPLICATE('_', LEN(_RechMsg('JT_RESUL035')))
      lcLibScrutin := _RechMsg('JT_RESUL038') // 'La Chambre'
    CASE pcCodescru == _SCRUTIN_SENAT 
      IF ccNumArr == _ARROND_BRUXELLES
        @ PROW()+1,00 SAY _RechMsg('JT_RESUL106') // "Arrondissement "
        @ PROW(),00   SAY REPLICATE('_',LEN(_RechMsg('JT_RESUL106'))+1)
      ELSE
        @ PROW()+1,00 SAY _RechMsg('JT_RESUL108') // "CollŠge ‚lectoral "
        @ PROW(),00   SAY REPLICATE('_',LEN(_RechMsg('JT_RESUL108')))
      ENDIF
      @ PROW(),55   SAY _RechMsg('JT_RESUL033') +' '+ lc_date         // "Date  : "
      @ PROW()+1,55 SAY _RechMsg('JT_RESUL034') +' '+ lc_heure        // "Heure : "
      @ PROW()+3,00 SAY _RechMsg('JT_RESUL035') +' '+ IIF(ccNumArr==_ARROND_BRUXELLES .AND. ccLangue==_NEERLANDAIS, RechCantN(),PARAM->p_burdep) // "Canton de "
      @ PROW(),00   SAY REPLICATE('_', LEN(_RechMsg('JT_RESUL035')))
      lcLibScrutin := _RechMsg('JT_RESUL039') // 'Le S‚nat'
  ENDCASE
  lcLibScrutin = lcLibScrutin  + ' Tot. ' + ccNoPc
  lnDebut := INT( (80-(LEN(lcLibScrutin)+4))/2 )
  @ PROW()+2,lnDebut SAY "Ú"+REPLICATE("Ä",LEN(lcLibScrutin)+2)+"¿"
  @ PROW()+1,lnDebut SAY "³ "+lcLibScrutin+" ³"+IIF(plSuite, _RechMsg('JT_RESUL044'), '') // ' (Suite)'
  @ PROW()+1,lnDebut SAY "À"+REPLICATE("Ä",LEN(lcLibScrutin)+2)+"Ù"

  IF LEN(ALLTRIM(PARAM->p_NoPcLut)) > 1
    lsBurTot := str(gnBurMax,3)
  ELSE
    lsBurTot := str(PARAM->p_nbrbur,3)
  ENDIF

  @ PROW()+2, 04 SAY _RechMsg('JT_RESUL045') +' '+ str(lnNbBurRecup,3) + ' '+_RechMsg('JT_RESUL046')+' '+lsBurTot+' '+_RechMsg('JT_RESUL047') +' '+ltrim(trans(lnNbVoteTot,'999999'))+' '+_RechMsg('JT_RESUL121')

*                      'R‚sultats Partiels '                                   ' bureaux sur '                                        ' soit '                                                            ' cartes'
  @ PROW()+1, 00 SAY '  '
  IF ccLangue == _FRANCAIS
     @ PROW()+1, 04 SAY PADR( _RechMsg('JT_RESUL122'),34) +'   Sc1     Sc2     Sc3     Sc4    '+_RechMsg('JT_RESUL123') // Total
  ELSE
     @ PROW()+1, 04 SAY PADR( _RechMsg('JT_RESUL122'),34) +'   Oc1     Oc2     Oc3     Oc4    '+_RechMsg('JT_RESUL123') // Total
  ENDIF
  @ PROW()+1, 04 SAY       'ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ ÄÄÄÄÄÄÄ'
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ menu du listing … imprimer                        ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ParScrutin
  LOCAL lcLangImp,lcOldLangue:=ccLangue , ccMsgColl1
  PRIVATE xs_code, xs_nom, xs_record , ln_i,lnChoiScru,ln_j,leEcran1,leEcran2, lnLgMax
  DECLARE xs_code[5], xs_nom[5],xs_record[5],xs_max_eff[5], xs_max_sup[5]

  ccMsgColl1 := _RechMsg('JT_RESUL008') // "ProcŠs-verbal"

  DBSELECTAREA('SCRUTINS')
  GO TOP

  lnNbScrut := 0
  DO WHILE (.NOT. EOF()) .AND. lnNbScrut <= 5
        lnNbScrut             := lnNbScrut + 1
        xs_code[lnNbScrut]    := s_code
        IF ccLangue= _NEERLANDAIS
          xs_nom[lnNbScrut]     := TRIM(s_nomn)
        ELSE
          xs_nom[lnNbScrut]     := TRIM(s_nomf)
        ENDIf
        xs_max_eff[lnNbScrut] := s_max_eff
        xs_max_sup[lnNbScrut] := s_max_sup
        xs_record[lnNbScrut]  := RECNO()
        SKIP
  ENDDO

* lnLgMax := LEN(_rechmsg( 'JT_CANDI102')) + 5
  lnLgMax := LEN(_rechmsg( 'JT_CANDI106')) + 5

  leEcran1 := SAVESCREEN(11,25,16,25+lnLgMax)

  SETCOLOR(_COLOR_OMBRE)
  @ 12,27,13+lnNbScrut,25+lnLgMax BOX REPLICATE(CHR(176),9)
  SETCOLOR(_COLOR_TRAV1)
  @ 11,25,12+lnNbScrut,(25+lnLgMax)-2 BOX B_SINGLE + ' '
  SETCOLOR(_COLOR_TRAV1)

  IF lnNbScrut > 0
    lnChoiScru = 1
    DO WHILE lnChoiScru <> 0
      FOR ln_j=1 TO lnNbScrut
          DO CASE
            CASE xs_code[ln_j] == _SCRUTIN_CHAMBRE
              @ 11+ln_j,26 PROMPT " "+PADR(_rechmsg('JT_CANDI102'),lnLgMax-5)+" "
            CASE xs_code[ln_j] == _SCRUTIN_SENAT
              @ 11+ln_j,26 PROMPT " "+PADR(_rechmsg('JT_CANDI103'),lnLgMax-5)+" "
          ENDCASE
      NEXT
      MENU TO lnChoiscru

      IF lnChoiScru<>0
        leEcran2 := SAVESCREEN(12+lnChoiScru,28,18+lnChoiScru,76)
        SETCOLOR(_COLOR_OMBRE)
        @ 13+lnChoiScru,30,18+lnChoiScru,76 BOX REPLICATE(CHR(176),9)
        SETCOLOR(_COLOR_TRAV1)
        @ 12+lnChoiScru,28,17+lnChoiScru,74 BOX B_SINGLE + ' '
        SETCOLOR(_COLOR_TRAV1)
        lnChoiImp := 1
        DO WHILE lnChoiImp <> 0
          SETCOLOR(_COLOR_MENU)
          _AffMsg(23,00,_RechMsg('JT_RESUL002')+" "+_CARACT_CROCHETS+' . '+_RechMsg('JT_RESUL003'), AM_ALIGN_CE)
          @ 24,01 SAY SPACE(78)
          SETCOLOR(_COLOR_TRAV1)
          @ 13+lnChoiScru,29 PROMPT LEFT(_RechMsg('JT_RESUL009')+SPACE(45),45) //  R‚sultats globaux
          @ 14+lnChoiScru,29 PROMPT LEFT(_RechMsg('JT_RESUL010')+SPACE(45),45) //  ProcŠs-verbal de recensement des votes
          @ 15+lnChoiScru,29 PROMPT LEFT(_RechMsg('JT_RESUL011')+SPACE(45),45) //  Tableau de recensement des votes
          @ 16+lnChoiScru,29 PROMPT LEFT(_RechMsg('JT_RESUL012')+SPACE(45),45) //  Tableau r‚capitulatif par liste
          MENU TO lnChoiImp
          IF lnChoiImp # 0
            lcLangImp := LangImp("JT_LANGUE")
            SETCOLOR(_COLOR_MENU)
            @ 23,01 SAY SPACE(78)
            IF testimp()
              ccLangue := lcLangImp
              DO CASE
                CASE lnChoiImp == 1
                    SETCOLOR(_COLOR_MENU)
                    * impression du PV
                    @ 24,02 SAY ccMsgColl1 // ProcŠs-verbal
                    ImpRec(_FALSE, xs_code[lnChoiScru], xs_max_eff[lnChoiScru]+xs_max_sup[lnChoiScru])
                    * impression du tableau
                    ImpTot(_FALSE  ,xs_record[lnChoiScru] )
                CASE lnChoiImp == 2 
                    ImpRec(_FALSE,xs_code[lnChoiScru], xs_max_eff[lnChoiScru]+xs_max_sup[lnChoiScru])
                CASE lnChoiImp == 3
                    ImpTot(_FALSE  ,xs_record[lnChoiScru] )
                CASE lnChoiImp = 4
                    Private gn_page
                    gn_page:=0
                    ImpRecapi(xs_record[lnChoiScru])
              ENDCASE
              ccLangue := lcOldLangue   // Restaurer la langue de travail
            ENDIF
          ENDIF
        ENDDO
        SETCOLOR(_COLOR_MENU)
        _AffMsg(23,00, _RechMsg('JT_RESUL002')+' ' + _CARACT_CROCHETS + " . "+ _RechMsg('JT_RESUL003'), AM_ALIGN_CE)
        SETCOLOR(_COLOR_TRAV1)
        RESTSCREEN(12+lnChoiScru,28,18+lnChoiScru,76,leEcran2)
      ENDIF

    ENDDO
  ENDIF

  RESTSCREEN(11,25,16,25+lnLgMax,leEcran1)
RETURN(NIL)


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ impression R‚sultat par liste                     ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ImpTot
  PARAMETER pl_confirme,;   && .T. s'il faut une confirmation d'impression
            pn_recordscru   && nø du record du scrutin  0=les traiter TOUS
* modif 26/01/99
  PRIVATE leEcran,lc_rep,;
          gn_nbr_vote,gn_votn,gn_vote,gn_vots,;
          gn_nbr_bnul,gn_nuln,gn_nule,gn_nuls,;
          la_tab,ln_nbrbur,ln_i,lc_strliste
* fin modif 26/01/99
  PRIVATE ln_lg_scru,lc_date,lc_heure,gn_page,la_v_list,la_v_nomi,la_v_vote, la_v_tl, la_v_sp, la_v_ef, ln_nbrliste
  PRIVATE ll_continu, gnNbVComm,gnNbVComm, lnNbBuComm

  ll_Continu := _FALSE
  IF pl_confirme
    leEcran := SAVESCREEN(17,00,24,79)
    SETCOLOR(_COLOR_MENU)
    @ 23,01 CLEAR TO 24,78
    @ 17,06,21,73 BOX B_SINGLE + ' '
    STORE SPACE(1) TO lc_rep
    IF (lc_rep := _GetRep( 'JT_RESUL013', _FALSE )) == _REPONSE_OUI   // Confirmez l'impression des r‚sultats de la totalisation
      IF testimp(.F.)
        ll_Continu := _TRUE
      ENDIF
    ENDIF
  ELSE
    SETCOLOR(_COLOR_MENU)
    ll_Continu := _TRUE
  ENDIF

  IF ll_Continu
    * compter le nbr de bulletins enregistr‚s + nbr bulletins nuls

    STORE 0 TO gn_nbr_vote,gn_votn,gn_vote,gn_vots
    STORE 0 TO gn_nbr_bnul,gn_nuln,gn_nule,gn_nuls
    STORE 0 TO gnNbVComm,gnNbBComm

    DBSELECTAREA('BURVOTES')
    GO TOP
    ln_NbrBur  := 0
    lnNbBuComm := 0
    DO WHILE .NOT. EOF()
      gn_Nbr_Vote += BURVOTES->b_nbr_votn + BURVOTES->b_nbr_vote +BURVOTES->b_nbr_vots
      gn_Votn     += BURVOTES->b_nbr_votn
      gn_Vote     += BURVOTES->b_nbr_vote
      gn_Vots     += BURVOTES->b_nbr_vots

      gn_Nbr_Bnul += BURVOTES->b_nbr_nuln + BURVOTES->b_nbr_nule + BURVOTES->b_nbr_nuls
      gn_nuln     += BURVOTES->b_nbr_nuln
      gn_Nule     += BURVOTES->b_nbr_nule
      gn_Nuls     += BURVOTES->b_nbr_nuls

      ln_NbrBur   += 1
      IF b_commun == PARAM->p_burdep .OR. PARAM->p_burcommu == _BUR_TOTAL_COMMUNAL
        gnNbVComm += BURVOTES->b_nbr_votn
        gnNbBComm += BURVOTES->b_nbr_nuln
        lnNbBuComm+= 1
      ENDIF
      SKIP
    ENDDO

    DBSELECTAREA('SCRUTINS')
    GO TOP
 
    * date et heure d‚but impression
    lc_Date  := DTOC(DATE())
    lc_Heure := TIME()
    * tableau pour DOCUMENT 2
   
    DECLARE la_v_list[200]   // 1->100 pour collŠge F et 101->199 pour collŠge N
    DECLARE la_v_nomi[200]   // 1->100 pour collŠge F et 101->199 pour collŠge N
    DECLARE la_v_vote[200]   // 1->100 pour collŠge F et 101->199 pour collŠge N
    DECLARE la_v_tl[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N
    DECLARE la_v_sp[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N
    DECLARE la_v_ef[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N

    gn_page := 0   && nø de page
    IF pn_recordscru == 0
      STORE _TRUE TO cc_int_imp    && interruption impression
      DO WHILE (.NOT. EOF()) .AND. cc_int_imp
        IF (.NOT. EMPTY(SCRUTINS->s_nomf)) .and. (.NOT. (SCRUTINS->s_code==_SCRUTIN_PROVINCIAL .and. PARAM->p_burcommu == _BUR_TOTAL_COMMUNAL ))
          IF (.NOT. (SCRUTINS->s_code==_SCRUTIN_COMMUNAL .and. PARAM->p_burcommu # _BUR_TOTAL_COMMUNAL))
            ImpScru(1)
          ENDIF
        ENDIF
        SKIP
      ENDDO

    ELSE   && traitement d'UN scrutin
      GO pn_recordscru
      ImpScru(2)
    ENDIF

  ENDIF

  IF pl_Confirme
    RESTSCREEN(17,00,24,79,leEcran)
    SETCOLOR(_COLOR_TRAV1)
  ENDIF

RETURN(NIL)


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ impression par scrutin                            ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ImpScru
  PARAMETER pn_typeaff   && type d'affichage
  LOCAL lnIndListe,llTraite,lnNbEff, lnNbSup
  PRIVATE gnVFr:=gnVNdl:=0
  SET DEVICE TO SCREEN
  IF pn_TypeAff == 1
     IF ccLangue = _NEERLANDAIS
       @ 19,12 SAY _RechMsg('JT_RESUL007') + ' ' +trim(SCRUTINS->s_nomn)+space(10)  // Traitement du scrutin
     ELSE
       @ 19,12 SAY _RechMsg('JT_RESUL007') + ' ' +trim(SCRUTINS->s_nomf)+space(10)  // Traitement du scrutin
     ENDIF
  ENDIF
  IF ccNumArr == _ARROND_BRUXELLES .AND. SCRUTINS->s_code == _SCRUTIN_SENAT
    DBSELECTAR('LISTES')
    SEEK SCRUTINS->s_code
    DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. (.NOT. EOF())
      DBSELECTAR('CANDIDATS')
      SEEK LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF
      DO WHILE CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste+CANDIDATS->c_eff_sup;
               == LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF
        SKIP
      ENDDO

      DBSELECTAR('LISTES')

      IF LISTES->l_college == 'F'
        gnVFr += LISTES->l_nbr_v_ex                       
      ELSE
        gnVNdl += LISTES->l_nbr_v_ex                      
      ENDIF
      SKIP
    ENDDO
  ENDIF

  * initialiser les tableaux pour DOCUMENT 2
  AFILL(la_v_list,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N
  AFILL(la_v_nomi,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N
  AFILL(la_v_vote,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N
  AFILL(la_v_tl  ,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N
  AFILL(la_v_sp  ,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N
  AFILL(la_v_ef  ,0)   // 1->50 pour collŠge F et 51->99 pour collŠge N

  * impression des r‚sultats des listes du scrutin
  DBSELECTAR('LISTES')
  SEEK SCRUTINS->s_code
  STORE _TRUE TO cc_int_imp,ll_imptot
  STORE 0 TO ln_NbrListe
  STORE "" TO lc_strliste
  DO WHILE LISTES->l_codescru==SCRUTINS->s_code .AND. (.NOT. EOF()) ;
     .AND. cc_int_imp 
    IF ccNumArr # _ARROND_BRUXELLES
       llTraite := _TRUE
    ELSE
       IF SCRUTINS->s_code $  _SCRUTIN_CHAMBRE + _SCRUTIN_REGIONAL + _SCRUTIN_BXL_REGION_FLA
         llTraite := _TRUE
       ELSE
         llTraite := (LISTES->l_college == ccLangue)
       ENDIF
    ENDIF
    IF llTraite
       SET DEVICE TO SCREEN
       IF pn_TypeAff == 1
         @ 20,12 SAY _RechMsg( 'JT_RESUL019')+ ' '+LISTES->l_nom  // "Traitement de la liste "
       ELSE
         @ 24,02 SAY _RechMsg( 'JT_RESUL020')+' ' +LISTES->l_nom // "Liste "
       ENDIF

       IF SCRUTINS->s_code == _SCRUTIN_SENAT .AND. ccNumArr == _ARROND_BRUXELLES .AND. LISTES->l_college == _COLLEGE_FRANCOPHONE
         lnNbEff := cnArrEff3F
         lnNbSup := cnArrSup3F
       ELSE
         lnNbEff := SCRUTINS->s_max_eff
         lnNbSup := SCRUTINS->s_max_sup
       ENDIF

        IF ccLAngue= _NEERLANDAIS
             ImpReList(SCRUTINS->s_code, SCRUTINS->s_nomn, lc_Date, lc_Heure,;
                 LISTES->l_codescru+LISTES->l_college+LISTES->l_numero, ll_imptot,;
                 LISTES->l_nbreffec ,LISTES->l_nbrsuple ,LISTES->l_nom)
        ELSE
             ImpReList(SCRUTINS->s_code, SCRUTINS->s_nomf, lc_Date, lc_Heure,;
                 LISTES->l_codescru+LISTES->l_college+LISTES->l_numero, ll_imptot,;
                 LISTES->l_nbreffec ,LISTES->l_nbrsuple ,LISTES->l_nom)
        ENDIF
       ll_imptot   := _FALSE
       ln_nbrliste := ln_nbrliste + 1
       lnIndListe := STRZERO( VAL(LISTES->l_numero)+IIF(LISTES->l_college==_COLLEGE_FRANCOPHONE,0,100), 3 )
       
       lc_strliste := lc_strliste + lnIndListe
    ENDIF
    DBSELECTAR('LISTES')
    SKIP
  ENDDO

  * imprimer le document 2
  SET DEVICE TO PRINT
  @ PROW(), PCOL() SAY _IMPR_INIT + _IMPR_12CPI
  IF ccLangue = _NEERLANDAIS 
     ImpDoc2(SCRUTINS->s_code, SCRUTINS->s_nomn, lc_date, lc_heure, ln_nbrliste, lc_strliste)
  ELSE
     ImpDoc2(SCRUTINS->s_code, SCRUTINS->s_nomf, lc_date, lc_heure, ln_nbrliste, lc_strliste)
  ENDIF
  SET DEVICE TO SCREEN

  DBSELECTAR('SCRUTINS')
RETURN(NIL)


*************************************************
* impression de l'en-tˆte document 1  (liste)
*************************************************
FUNCTION ImpTete3
  PARAMETER pc_codescru,;  && code du scrutin
            pc_nomscru,;   && nom du scrutin
            pc_date,;      && date d‚but impression
            pc_heure       && heure d‚but impression

  PRIVATE ln_lg_scru, lcLibScrutin

  lcLibScrutin := space(10)

  gn_page = gn_page + 1

  DO CASE
    CASE pc_codescru == _SCRUTIN_CHAMBRE
      IF gn_page == 1
          IF ccNumArr == _ARROND_BRUXELLES
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL305') // Annexe 1
          ELSE
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL305') // Annexe 1 
          ENDIF
        IF ccNumArr == _ARROND_BRUXELLES
          @ PROW()+1,00 SAY _RechMsg('JT_RESUL106')  // Election Bruxelles-Hal-Vilvoorde
        ELSE
          @ PROW()+1,00 SAY _RechMsg('JT_RESUL032')+" "+ TRIM(ccArron) // "Circonscription ‚lectorale
        ENDIF
        @ PROW()  ,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

        @ PROW()+1,00 SAY _RechMsg('JT_RESUL085')+" "+TRIM(IIF(ccNumArr==_ARROND_BRUXELLES .AND. ccLangue==_NEERLANDAIS, RechCantN(),PARAM->p_burdep)) // "Canton de :
        @ PROW()+2,67 SAY _RechMsg('JT_RESUL033') +' '+ lc_date         // "Date  : "
        @ PROW()+1,67 SAY _RechMsg('JT_RESUL034') +' '+ lc_heure        // "Heure : "
        @ PROW()+1,03 SAY _RechMsg('JT_RESUL086') // Bureau principal de canton
        @ PROW()+1,09 SAY 'ÄÄÄÄÄÄÄÄ'
        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL111') ,92) // election de la chambre
        SET CENTURY ON
*       @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL088')+' '+DTOC(PARAM->p_dateelec) ,92) // election de la chambre
        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL088')+' '+_RechMsg('JT_DATE') ,92) // election de la chambre

        SET CENTURY ON

        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL053'), 92) // "TABLEAU DE RECENSEMENT DES VOTES"
        @ PROW()  ,00 SAY PADC(REPLICATE('_',18) ,92)
      ELSE
          IF ccNumArr == _ARROND_BRUXELLES
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL305') // Annexe 1
          ELSE
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL305') // Annexe 1
          ENDIF

        @ PROW()+1,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

      ENDIF
      lcLibScrutin := _RechMsg('JT_RESUL041') // 'region

    CASE pc_codescru == _SCRUTIN_SENAT
      IF gn_page == 1
        IF ccNumArr == _ARROND_BRUXELLES
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // Annexe 2
            @ PROW()+1,00 SAY _RechMsg('JT_RESUL108') // CollŠge ‚lectoral fran‡ais
        ELSE
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // Annexe 2
        ENDIF
        IF ccNumArr == _ARROND_BRUXELLES
          @ PROW()+1,00 SAY _RechMsg('JT_RESUL106')  // Election Bruxelles-Hal-Vilvoorde
        ELSE
          @ PROW()+1,00 SAY _RechMsg('JT_RESUL108') // "CollŠge ‚lectoral "
        ENDIF
        @ PROW()  ,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

        @ PROW()+1,00 SAY _RechMsg('JT_RESUL085')+" "+TRIM(IIF(ccNumArr==_ARROND_BRUXELLES .AND. ccLangue==_NEERLANDAIS, RechCantN(),PARAM->p_burdep)) // "Canton de :

        @ PROW()+2,67 SAY _RechMsg('JT_RESUL033') +' '+ lc_date         // "Date  : "
        @ PROW()+1,67 SAY _RechMsg('JT_RESUL034') +' '+ lc_heure        // "Heure : "
        @ PROW()+1,03 SAY _RechMsg('JT_RESUL086') // Bureau principal de canton

        @ PROW()+1,09 SAY 'ÄÄÄÄÄÄÄÄ'
        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL113') ,92) // election senat 
        SET CENTURY ON
*       @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL088')+' '+DTOC(PARAM->p_dateelec) ,92) // election de la chambre
        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL088')+' '+_RechMsg('JT_DATE') ,92) // election de la chambre
        SET CENTURY ON
        @ PROW()+2,00 SAY PADC(_RechMsg('JT_RESUL053'), 92) // "TABLEAU DE RECENSEMENT DES VOTES"
        @ PROW()  ,00 SAY PADC(REPLICATE('_',22) ,92)
      ELSE
          IF ccNumArr == _ARROND_BRUXELLES
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // Annexe 2
          ELSE
            @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // Annexe 2
          ENDIF
        @ PROW()+1,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

      ENDIF
      lcLibScrutin := _RechMsg('JT_RESUL039') // 'Le S‚nat
    ENDCASE
    ln_lg_scru = LEN(TRIM(lcLibScrutin))
RETURN(NIL)

*************************************************
* impression ligne de la page 1
*************************************************
FUNCTION ImpLig1
  PARAMETER ln_numlig,;   && num‚ro de ligne
            lc_str        && string … imprimer
  @ ln_numlig,15 SAY "³"
  @ ln_numlig,16 SAY lc_str
  @ ln_numlig,72 SAY "³"
RETURN(NIL)


*************************************************
* impression des r‚sultats d'une liste
*************************************************
FUNCTION ImpReList
  PARAMETER pc_codescru,;  && code du scrutin
            pc_nomscru,;   && nom du scrutin
            pc_date,;      && date d‚but impression
            pc_heure,;     && heure d‚but impression
            pc_codeliste,; && code de la liste
            pl_imptot,;    && .T. pour imprimer les totaux (pour 1Šre liste)
            pn_nbreff,;    && nbr d'effectifs ( MAX  4.26 -> exact)
            pn_nbrsup,;    && nbr de suppl‚ants ( MAX  4.26 -> exact )
            pc_nomliste    && sigle

   LOCAL lnIndTab,lcLib,nLigne

   SET DEVICE TO PRINT
   @ PROW(), PCOL() SAY _IMPR_INIT + _IMPR_12CPI

   ImpTete3(pc_codescru, pc_nomscru, pc_date, pc_heure)

   implig2(PROW()+4, SPACE(42)+_RechMsg('JT_RESUL030') +' '+LISTES->l_numero+' '+pc_nomliste) // "Liste "
   @ PROW(),43 SAY "________"
   implig2(PROW()+1,"")

*    // avec suppl‚ants
   lcLib := _RechMsg('JT_RESUL124') // " A. chiffre ‚lectoral
   implig2(PROW()+1,LEFT(lcLib,4)+_IMPR_SOUL_ON+SUBSTR(lcLib,5)+_IMPR_SOUL_OFF )
   implig2(PROW()+1,"")
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL125'),70)+STR(LISTES->l_nbr_v_tl)) // "1) Dans  la case de tˆte : "
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL126'),70) )
   implig2(PROW()+1,"")
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL127'),70) )
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL128'),70)+STR(LISTES->l_nbr_v_ef)) // Effectifs seuls
   implig2(PROW()+1,"")
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL129'),70) )
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL130'),70) )
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL133'),70)+STR(LISTES->l_nbr_v_ex-(LISTES->l_nbr_v_tl+LISTES->l_nbr_v_ef+LISTES->l_nbr_v_sp),6))
   implig2(PROW()+1,"")
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL131'),70) )
   implig2(PROW()+1,SPACE(4)+PADR(_RechMsg('JT_RESUL132'),70)+ STR(LISTES->l_nbr_v_sp)) // Suppl‚ants seuls
   implig2(PROW()+1,"")
   implig2(PROW()+1,SPACE(72)+REPLICATE("Ä",9))
   implig2(PROW()+1,PADR(SPACE(37)+_RechMsg('JT_RESUL134'),73) +STR(LISTES->l_nbr_v_ex,7) + ' (1)' ) // "Total"
   implig2(PROW()+1,SPACE(72)+REPLICATE("Í",9))
   implig2(PROW()+1,"")
   lnIndTab := VAL(LISTES->l_numero)+IIF(LISTES->l_college==_COLLEGE_FRANCOPHONE,0,100)
   la_v_list[lnIndTab] += LISTES->l_nbr_v_tl+LISTES->l_nbr_v_sp

   la_v_vote[lnIndTab] += LISTES->l_nbr_v_ex
   la_v_tl[lnIndTab]   += LISTES->l_nbr_v_tl 
   la_v_sp[lnIndTab]   += LISTES->l_nbr_v_sp
   la_v_ef[lnIndTab]   += LISTES->l_nbr_v_ef 

   DBSELECTAR('CANDIDATS')

    * impression des effectifs
    ImpCand(pc_codeliste, _CODE_EFFECTIF ,pn_nbreff, pn_nbreff+pn_nbrsup,pc_nomliste)
    * imprimer les suppl‚ants
    ImpCand(pc_codeliste, _CODE_SUPPLEANT ,pn_nbrsup, pn_nbreff+pn_nbrsup,pc_nomliste)
    IF PROW() >= 60
      nLigne=1
    ELSE
      nLigne=0
    ENDIF

    IF ccLangue == _FRANCAIS
       @ 60+nLigne,2 SAY "(1) Le chiffre ‚lectoral d'une liste correspond au total des cartes magn‚tiques"
       @ 61+nLigne,2 SAY "comportant un vote valable en tˆte de cette liste ou un  vote valable en faveur"
       @ 62+nLigne,2 SAY "d'un ou de plusieurs candidats (titulaires et/ou suppl‚ants) de cette liste."

    ELSE
       @ 60+nLigne,5 SAY " (1) Het stemcijfer van een lijst  is het totaal  van de magneetkaarten voor een"
       @ 61+nLigne,5 SAY " lijst met  een  geldige stem bovenaan  die lijst  of met een  geldige stem voor"
       @ 62+nLigne,5 SAY " ‚‚n of meer kandidaten (titularissen en/of opvolgers) van die lijst."
    ENDIF
  EJECT
  SET DEVICE TO SCREEN
RETURN(NIL)


*************************************************
* impression des r‚sultats des candidats d'une
* liste   avec suppl‚ants
*************************************************


FUNCTION ImpCand
  PARAMETER pc_codelist,;   && code de la liste
            pc_code_e_s,;   && code Effectif Suppl‚ant
            pn_nbrcandi,;   && nbr de candidats MAX … traiter
            pn_NbTot
  LOCAL lnIndTab,lcLib
  PRIVATE ln_totliste,ll_flag,ln_i
  IF PROW() >= 60
    EJECT
    gn_page = gn_page + 1
    @ PROW()+2,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

    implig2(PROW()+4,SPACE(35)+_RechMsg('JT_RESUL030')+' '+LISTES->l_numero+' '+pc_nomliste+' '+_RechMsg('JT_RESUL044')) // "Liste "   ' (Suite)'
    @ PROW(),36 SAY "________"
    implig2(PROW()+1,"")
  ENDIF

  implig2(PROW()+1,"")
  IF (PROW()+3 ) >= 60
    EJECT
    gn_page = gn_page + 1
    @ PROW()+2,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"
    implig2(PROW()+2,"")
  ENDIF

  IF pc_code_e_s == _CODE_EFFECTIF
    lcLib := _RechMsg('JT_RESUL064') // " B. VOTES NOMINATIFS POUR LES CANDIDATS TITULAIRES"
    implig2(PROW()+1,LEFT(lcLib,4)+_IMPR_SOUL_ON+SUBSTR(lcLib,5)+_IMPR_SOUL_OFF) // " B. VOTES NOMINATIFS POUR LES CANDIDATS-TITULAIRES"
  ELSE
    lcLib := _RechMSg('JT_RESUL065') // " C. VOTES NOMINATIFS POUR LES SUPPLEANTS"
    implig2(PROW()+1,LEFT(lcLib,4)+_IMPR_SOUL_ON+SUBSTR(lcLib,5)+_IMPR_SOUL_OFF) // " C. VOTES NOMINATIFS POUR LES CANDIDATS-SUPPLEANTS"
  ENDIF
  implig2(PROW()+1,"")
  ln_totliste = 0   && nbr de votes pour effectifs
  SEEK pc_codelist + pc_code_e_s
  FOR ln_i = 1 TO pn_nbrcandi
    IF CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste;
       +CANDIDATS->c_eff_sup = pc_codelist+pc_code_e_s .AND. .NOT. EOF()
      STORE .T. TO ll_flag
    ELSE
      STORE .F. TO ll_flag
    ENDIF
*   IF PROW() >= 60
    IF PROW() >= 61
      EJECT
      gn_page = gn_page + 1
      @ PROW()+2,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"

      implig2(PROW()+4,SPACE(35)+_RechMsg('JT_RESUL030')+' '+LISTES->l_numero+' '+pc_nomliste+' '+_RechMsg('JT_RESUL044')) // "Liste "   ' (Suite)'
      @ PROW(),36 SAY "________"
      implig2(PROW()+1,"")
    ENDIF

*    implig2(PROW()+1,SPACE(5)+STR(ln_i,2)+". ";
*           +IIF(ll_flag,IIF(pn_NbTot>_NBCANDI_MODE3X15,;
*            left(TRIM(CANDIDATS->c_nom1)+' '+trim(CANDIDATS->c_nom2)+' '+trim(CANDIDATS->c_nom3)+space(63),63),;
*            left(trim(CANDIDATS->c_nom1)+' '+trim(CANDIDATS->c_nom2)+space(63),63))+' '+STR(CANDIDATS->c_nbr_v_ca,7),""))


     implig2(PROW()+1,SPACE(5)+STR(ln_i,2)+". ";
           +IIF(ll_flag,;
            left(trim(CANDIDATS->c_nom1)+' '+trim(CANDIDATS->c_nom2)+space(63),63)+' '+STR(CANDIDATS->c_nbr_v_ca,7);
            ,""))


    IF ll_flag
      ln_totliste = ln_totliste + CANDIDATS->c_nbr_v_ca
      SKIP
    ENDIF
  NEXT ln_i

  IF !(pc_CodeScru $ _SCRUTIN_CHAMBRE + _SCRUTIN_SENAT )
    implig2(PROW()+1,SPACE(72)+REPLICATE("Í",9))
    implig2(PROW()+1,SPACE(67)+_RechMsg('JT_RESUL062')+STR(ln_totliste,7))  // "Total"
    implig2(PROW()+1,"")
  ENDIF

  IF pc_code_e_s == _CODE_EFFECTIF
   lnIndTab := VAL(LISTES->l_numero) + IIF(LISTES->l_College==_COLLEGE_FRANCOPHONE,0,100)
    la_v_nomi[lnIndTab] += ln_totliste
  ENDIF

RETURN(NIL)

*************************************************
* impression ligne de la page r‚sultat liste
*************************************************
FUNCTION ImpLig2
  PARAMETER ln_numlig,;   && num‚ro de ligne
            lc_str        && string … imprimer
  @ ln_numlig,1 SAY lc_str
RETURN(NIL)


*************************************************
* impression du document 2
*************************************************
FUNCTION ImpDoc2
  PARAMETER pc_codescru,;  && code du scrutin
            pc_nomscru,;   && nom du scrutin
            pc_date,;      && date d‚but impression
            pc_heure,;     && heure d‚but impression
            pn_nbrlist,;   && nbr de liste … traiter
            pc_strlist     && string qui contient les nø des listes

  PRIVATE ln_i,ln_tot,ln_val,ln_emis, lnVoteTot , lnNulTot

* << phe2009 vérifier la boucle !!!!!!
  lnTotBxl := 0
  IF pc_codescru=_SCRUTIN_BXL_REGION_FLA
    FOR ln_i = 1 TO 200
      lnTotBxl += la_v_vote[ln_i]                  
    NEXT ln_i
  ELSE
    FOR ln_i = IIF(ccLangue==_FRANCAIS,1,101) TO IIF(ccLangue==_FRANCAIS,100,200)
      lnTotBxl += la_v_vote[ln_i]                  
    NEXT ln_i
  ENDIF
*  
  ImpTete2(pc_codescru, pc_nomscru, pc_date, pc_heure)
  lnVoteTot := gn_votn
  lnNulTot  := gn_nuln
   
  IF ccNumArr == _ARROND_BRUXELLES .AND. pc_CodeScru $ _SCRUTIN_SENAT 
     @ PROW()+3,27 SAY _RechMsg('JT_RESUL136') // "- Cartes valables  :"
     @ PROW(),PCOL()+1 SAY lnTotBxl PICT "9999999"
     @ PROW()+2,0 SAY ''
  ELSE 
     @ PROW()+3,15 SAY _RechMsg('JT_RESUL137') // "- Cartes enregistr‚es :"
     @ PROW(),PCOL()+1 SAY gn_votn PICT "9999999"
     @ PROW()+1,15 SAY _RechMsg('JT_RESUL138') // "- Cartes avec des votes valables      :"
     @ PROW(),PCOL()+1 SAY gn_votn ;
                -SCRUTINS->s_nbr_vbe - SCRUTINS->s_nbr_vbn PICT "9999999"
     @ PROW()+1,15 SAY _RechMsg('JT_RESUL139') // "- Cartes avec des votes blanc         :"
     @ PROW(),PCOL()+1 SAY SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn PICT "9999999"

     @ PROW()+1,15 SAY _RechMsg('JT_RESUL140') // "- Cartes avec des votes d‚clar‚s nuls : "
     @ PROW(),PCOL()+1 SAY gn_nuln PICT "9999999"
     @ PROW()+3,00 SAY ""   // Passer 3 lignes
  ENDIF

  ln_tot = 0
  Private lContinuePrint

  FOR ln_i = 1 TO pn_nbrlist
    IF PROW()+18 > 62  // A.C. 24/03/95
      EJECT
      ImpTete2(pc_codescru, pc_nomscru, pc_date, pc_heure)
    ENDIF
    ImpLigD2(SUBSTR(pc_strlist,(ln_i*3)-2,3))
  
  NEXT ln_i

  @ PROW()+2,1 SAY PADR(SPACE(40)+_RechMsg('JT_RESUL068'),66) // "Total g‚n‚ral"
  @ PROW(),PCOL()+2 SAY ln_tot PICT "9999999"
  @ PROW()+1,3 SAY PADR(SPACE(25)+_RechMsg('JT_RESUL135'),66)
  @ PROW()+0,68 SAY REPLICATE("Í",8)

  ln_val  := gn_votn  - SCRUTINS->s_nbr_vbe - SCRUTINS->s_nbr_vbn
  ln_emis := gn_votn  + gn_nuln 
  ln_nul  := gn_nuln
  
  IF PROW()+10 > 65
      EJECT
      ImpTete2(pc_codescru, pc_nomscru, pc_date, pc_heure)
  ENDIF

  IF ccLangue == _NEERLANDAIS
          IF ccNumArr = _ARROND_BRUXELLES .AND. pc_CodeScru $ _SCRUTIN_SENAT 
            @ PROW()+3,02 SAY '     Het bureau stelt vast dat het gezamenlijk totaal der stemcijfers juist'
            @ PROW()+1,02 SAY 'gelijk is aan het aantal geregistreerde kaarten met geldige stemmen.'
          ELSE
            @ PROW()+3,02 SAY '  Het bureau stelt vast dat het  gezamenlijk  totaal der stemcijfers juist gelijk'
            @ PROW()+1,02 SAY 'is aan het aantal geregistreerde kaarten met geldige stemmen, hetwelk bedraagt '+STR(ln_emis-ln_nul-SCRUTINS->s_nbr_vbe-SCRUTINS->s_nbr_vbn,7) 
            @ PROW()+1,02 SAY '('+ STR(ln_emis-ln_nul,7)+' geregistreerde kaarten in de bussen min'+ STR(SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,7)+' geregistreerde kaarten met'
            @ PROW()+1,02 SAY 'blanco stemmen).'
          ENDIF
  ELSE
      @ PROW()+3,02 SAY "        Le bureau constate que le total g‚n‚ral des chiffres ‚lectoraux donne un"
      IF ccNumArr = _ARROND_BRUXELLES .AND. pc_CodeScru $  _SCRUTIN_SENAT
         @ PROW()+1,02 SAY "nombre ‚gal … celui des cartes enregistr‚es avec des votes valables."
      ELSE
        IF ccNumArr = _ARROND_BRUXELLES
          @ PROW()+1,02 SAY "nombre ‚gal … celui des cartes enregistr‚es avec des votes valables qui s'‚lŠve …"
          @ PROW()+1,02 SAY STR(ln_emis-ln_nul-SCRUTINS->s_nbr_vbe-SCRUTINS->s_nbr_vbn,7)+"  ("+ STR(ln_emis-ln_nul,7)+" cartes trouv‚es dans les urnes moins "+STR(SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,7) + " cartes"
          @ PROW()+1,02 SAY "avec des votes blancs)."
        ELSE
          @ PROW()+1,02 SAY "nombre ‚gal … celui des cartes enregistr‚es avec des votes valables s'‚lŠvant …"
          @ PROW()+1,02 SAY STR(ln_emis-ln_nul-SCRUTINS->s_nbr_vbe-SCRUTINS->s_nbr_vbn,7)+"  ("+ STR(ln_emis-ln_nul,7)+" cartes enregistr‚es dans les urnes moins "+STR(SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,7) + " cartes"
          @ PROW()+1,02 SAY "enregistr‚es avec des votes blancs)."
        ENDIF
      ENDIF
  ENDIF                                                                                                 
  @ PROW()+3,15 SAY _RechMsg('JT_RESUL100') // "Fait … ........................, le ..................."
  @ PROW()+2,03 SAY _RechMsg('JT_RESUL101') // "Le secr‚taire,        Les assesseurs,          Le pr‚sident,"

  EJECT

  IF ccNumArr == _ARROND_BRUXELLES .AND. pc_CodeScru == _SCRUTIN_SENAT
    gnVoteFr := 0
    gnVoteNe := 0
    DBSELECTAR('LISTES')
    SEEK SCRUTINS->s_code
    DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. (.NOT. EOF())

      IF LISTES->l_college == 'F'
        gnVoteFr := gnVoteFr + LISTES->l_nbr_v_ex                       
      ELSE
        gnVoteNe := gnVoteNe + LISTES->l_nbr_v_ex                       
      ENDIF

      SKIP
    ENDDO

    PRIVATE taVarImpr[15]


    AFILL(taVarImpr, '')
 
    IF pc_CodeScru == _SCRUTIN_SENAT
       lcCodeTxt := "JT_RESULCO2"
       taVarImpr[01] := 'STR(gnVotefr,7)'
       taVarImpr[02] := 'STR(gnVoteNe,7)'
       taVarImpr[03] := 'STR(SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,7)'
       taVarImpr[04] := 'STR(gn_nbr_bnul,7)'
       taVarImpr[05] := 'STR(gn_nbr_vote+gn_nbr_bnul,7)'
       taVarImpr[06] := "'"+_IMPR_12CPI+"'"
       taVarImpr[07] := "'"+_IMPR_10CPI+"'"
       taVarImpr[08] := "IIF(ccLangue=='F','COLLEGE ELECTORAL FRANCAIS','NEDERLANDS KIESCOLLEGE')"
       taVarImpr[09] := "IIF(ccLangue=='F','Saint-Josse-ten-Noode','Sint-Joost-ten-Node')"
       taVarImpr[10] := 'DTOC(PARAM->p_dateelec)'
    ENDIF
    SET DEVICE TO PRINT
    SET PRINT ON
    SET CONSOLE OFF
    @ PROW(), PCOL() SAY _IMPR_INIT
    IF jt_msg->(DBSEEK(lcCodeTxt))
      lmMsgEcran := jt_Msg->msg_ecran&ccLangue
      DECLARE taMemo[1]  // MLCOUNT(lmMsgEcran, 80)]
      taLgImp := {}
      ImpMemo(@taMemo, 80, @lmMsgEcran , '1', @taVarImpr, _NB_LIG_PAGE, _TRUE )
      ChgTab('{001}')
      ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE ) // Haut page
    ENDIF
  ENDIF
RETURN(NIL)



*************************************************
* impression de l'en-tˆte document 2
*************************************************
FUNCTION ImpTete2
  PARAMETER pc_codescru,;  && code du scrutin
            pc_nomscru,;   && nom du scrutin
            pc_date,;      && date d‚but impression
            pc_heure       && heure d‚but impression

  gn_page = gn_page + 1
  DO CASE
     CASE pc_codescru == _SCRUTIN_CHAMBRE
      IF ccNumArr == _ARROND_BRUXELLES
          @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // annexe 2
      ELSE
          @ PROW()+1,53 SAY _RechMsg('JT_RESUL307') // annexe 2
      ENDIF
      @ PROW()+1,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"
      @ PROW()+2, 0 SAY PADC(_RechMsg('JT_RESUL089') , 92) // TABLEAU RECAPITULATIF
      @ PROW(),00   SAY PADC(REPLICATE('_',LEN(_RechMsg('JT_RESUL089'))),92)
      @ PROW()+1,01   SAY '   '

    CASE pc_codescru == _SCRUTIN_SENAT
      IF ccNumArr == _ARROND_BRUXELLES
          @ PROW()+1,53 SAY _RechMsg('JT_RESUL309') // annexe 3
      ELSE
          @ PROW()+1,53 SAY _RechMsg('JT_RESUL309') // annexe 3
      ENDIF
      @ PROW()+1,72 SAY _RechMsg('JT_RESUL052') +STR(gn_page,3) // "Page"
      @ PROW()+2, 0 SAY PADC(_RechMsg('JT_RESUL089') , 92) // TABLEAU RECAPITULATIF
      @ PROW(),00   SAY PADC(REPLICATE('_',LEN(_RechMsg('JT_RESUL089'))),92)
      @ PROW()+1,01   SAY '   '
 
  ENDCASE
RETURN(NIL)


*************************************************
* impression ligne corps du document 2
*************************************************
FUNCTION ImpLigD2
  PARAMETER pc_numlist   && nø de liste
  LOCAL lcNumListe:=STRZERO( IIF(VAL(pc_numlist)>100,;
                                 VAL(pc_numlist)-100,VAL(pc_numlist)) ,2 )
  LOCAL cNoMsg, nSavRecno, nSavSelect := SELECT()

    DBSELECTAREA('LISTES')
    nSavRecNo := LISTES->(RECNO())
    IF VAL(pc_numlist) < 101 
      LOCATE ALL FOR pc_codescru==L_CODESCRU .AND. L_NUMERO == lcNumListe
    ELSE
      LOCATE ALL FOR pc_codescru==L_CODESCRU .AND. L_NUMERO == lcNumListe .AND. L_COLLEGE == _COLLEGE_NEERLANDOPHONE
    ENDIF
    lcNumListe = lcNumListe +' '+LISTES->l_nom
    LISTES->(DBGOTO( nSavRecno ))
    @ PROW()+2,03 SAY _RechMsg('JT_RESUL090')+' '+lcNumListe+_RechMsg('JT_RESUL091') // La Liste ... obtient :
    @ PROW()+1,03 SAY REPLICATE("-",LEN(_RechMsg("JT_RESUL090"))+LEN(_RechMsg("JT_RESUL091"))+3)
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL125') , 68)
    @ PROW(),PCOL()   SAY la_v_tl[VAL(pc_numlist)] PICT "9999999"
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL126') , 66)
    @ PROW()+2,1 SAY PADR(_RechMsg('JT_RESUL127') , 66)
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL128') , 68)
    @ PROW(),PCOL()   SAY la_v_ef[VAL(pc_numlist)] PICT "9999999"
    @ PROW()+2,1 SAY PADR(_RechMsg('JT_RESUL129') , 66)
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL130') , 66)
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL133') , 68)
    @ PROW(),PCOL()   SAY la_v_vote[VAL(pc_numlist)]-(la_v_tl[VAL(pc_numlist)]+la_v_ef[VAL(pc_numlist)]+la_v_sp[VAL(pc_numlist)])    PICT "9999999"
    @ PROW()+2,1 SAY PADR(_RechMsg('JT_RESUL131') , 66)
    @ PROW()+1,1 SAY PADR(_RechMsg('JT_RESUL132') , 68)
    @ PROW(),PCOL()   SAY la_v_sp[VAL(pc_numlist)] PICT "9999999"
    @ PROW()+1,68 SAY REPLICATE("Ä",8)
    @ PROW()+1,3 SAY PADR(SPACE(31)+_RechMsg('JT_RESUL134'),66) // "Total"
    @ PROW(),PCOL() SAY la_v_vote[VAL(pc_numlist)] PICT "9999999"
    @ PROW()+1,68 SAY REPLICATE("Í",8)
    ln_tot = ln_tot + la_v_vote[VAL(pc_numlist)]
RETURN(NIL)


*************************************************
* impression du r‚capitulatif par liste
*************************************************
FUNCTION ImpRecapi
  PARAMETER pn_recordscru   && nø de record du scrutin
  LOCAL lnIndTab,lnIndListe,llTraite
  PRIVATE gn_nbr_vote,gn_votn,gn_vote,gn_vots,;
          gn_nbr_bnul,gn_nuln,gn_nule,gn_nuls,;
          la_v_list,la_v_nomi,la_v_tl, la_v_sp, la_v_eff,;
          ln_nbrliste,lc_strliste

  SETCOLOR(_COLOR_MENU)
  @ 24,26 SAY '  '+_RechMsg( 'JT_RESUL022' ) + '  ' // '...  CALCUL EN COURS  ... ' 
  DBSELECTAREA('BURVOTES')
  GO TOP

  STORE 0 TO gn_nbr_vote,gn_votn,gn_vote,gn_vots
  STORE 0 TO gn_nbr_bnul,gn_nuln,gn_nule,gn_nuls
  STORE 0 TO gnNbVComm,gnNbBComm
  DO WHILE .NOT. EOF()
      gn_Nbr_Vote += BURVOTES->b_nbr_votn + BURVOTES->b_nbr_vote +BURVOTES->b_nbr_vots
      gn_Votn     += BURVOTES->b_nbr_votn
      gn_Vote     += BURVOTES->b_nbr_vote
      gn_Vots     += BURVOTES->b_nbr_vots

      gn_Nbr_Bnul += BURVOTES->b_nbr_nuln + BURVOTES->b_nbr_nule + BURVOTES->b_nbr_nuls
      gn_Nbr_Bnul += BURVOTES->b_nbr_nuln
      gn_Nbr_Bnul += BURVOTES->b_nbr_nule
      gn_Nbr_Bnul += BURVOTES->b_nbr_nuls

      gn_nuln     += BURVOTES->b_nbr_nuln
      gn_Nule     += BURVOTES->b_nbr_nule
      gn_Nuls     += BURVOTES->b_nbr_nuls
    SKIP
  ENDDO

  DBSELECTAREA('PARAM')
  GO TOP

  DBSELECTAREA('CANDIDATS')
  GO TOP

  DBSELECTAREA('LISTES')
  GO TOP

  DBSELECTAREA('SCRUTINS')
  GO pn_recordscru

  * tableau pour DOCUMENT 2
  
  DECLARE la_v_list[200]   // 1->100 pour collŠge F et 101->1199 pour collŠge N
  DECLARE la_v_nomi[200]   // 1->100 pour collŠge F et 101->199 pour collŠge N
  DECLARE la_v_vote[200]   // 1->100 pour collŠge F et 101->199 pour collŠge N
  DECLARE la_v_tl[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N
  DECLARE la_v_sp[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N
  DECLARE la_v_ef[200]     // 1->100 pour collŠge F et 101->199 pour collŠge N

  AFILL(la_v_list,0)
  AFILL(la_v_nomi,0)
  AFILL(la_v_vote,0)
  AFILL(la_v_tl  ,0)       
  AFILL(la_v_sp  ,0)       
  AFILL(la_v_ef  ,0)      

  * impression des r‚sultats des listes du scrutin
  DBSELECTAR('LISTES')
  SEEK SCRUTINS->s_code
  STORE 0 TO ln_nbrliste
  STORE "" TO lc_strliste
  DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. (.NOT. EOF())
    IF ccNumArr # _ARROND_BRUXELLES
       llTraite := _TRUE
    ELSE
       IF SCRUTINS->s_code $ _SCRUTIN_CHAMBRE + _SCRUTIN_REGIONAL  + _SCRUTIN_BXL_REGION_FLA
          llTraite := _TRUE
       ELSE
          llTraite := (LISTES->l_college == ccLangue)
       ENDIF
    ENDIF
    IF llTraite
       DBSELECTAR('CANDIDATS')
       SEEK LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF
       DO WHILE CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste+CANDIDATS->c_eff_sup;
                   == LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF
            lnIndTab := VAL(LISTES->l_numero)+IIF(LISTES->l_college==_COLLEGE_FRANCOPHONE,0,100)
            la_v_nomi[lnIndTab] += CANDIDATS->c_nbr_v_ca
            SKIP
        ENDDO

       DBSELECTAR('LISTES')
       ln_nbrliste := ln_nbrliste + 1
       lnIndListe := STRZERO( VAL(LISTES->l_numero)+IIF(LISTES->l_college==_COLLEGE_FRANCOPHONE,0,100), 3 )
       lc_strliste := lc_strliste + lnIndListe

       lnIndTab := VAL(LISTES->l_numero)+IIF(LISTES->l_college==_COLLEGE_FRANCOPHONE,0,100)
       
       la_v_list[lnIndTab] += ( LISTES->l_nbr_v_tl + LISTES->l_nbr_v_sp )
       la_v_vote[lnIndTab] += LISTES->l_nbr_v_ex
       la_v_tl[lnIndTab]   += LISTES->l_nbr_v_tl  
       la_v_sp[lnIndTab]   += LISTES->l_nbr_v_sp  
       la_v_ef[lnIndTab]   += LISTES->l_nbr_v_ef  
    ENDIF

    SKIP
  ENDDO

  * imprimer le document 2
  @ 24,26 SAY _RechMsg("JT_RESUL023") // ...  IMPRESSION EN COURS  ..."
  SET DEVICE TO PRINT
  @ PROW(), PCOL() SAY _IMPR_INIT + _IMPR_12CPI
  IF ccLangue =_NEERLANDAIS
     ImpDoc2(SCRUTINS->s_code, SCRUTINS->s_nomn, DTOC(DATE()), TIME(), ln_nbrliste, lc_strliste)
  ELSE
     ImpDoc2(SCRUTINS->s_code, SCRUTINS->s_nomf, DTOC(DATE()), TIME(), ln_nbrliste, lc_strliste)
  ENDIF
  SET DEVICE TO SCREEN

* CLOSE DATABASES
RETURN(NIL)


*************************************************
* ‚cran choix du listing des r‚sultats … imprimer
*************************************************
FUNCTION Ecran_Imp
  AffTitre(_RechMsg('JT_RESUL001')) // IMPRESSION DES RESULTATS

   LigStatut() // Ligne 01 STATUT
  SETCOLOR(_COLOR_OMBRE)
  @ 11,25,14,58 BOX REPLICATE(CHR(176),9)
  SETCOLOR(_COLOR_TRAV1)
  @ 10,23,13,56 BOX B_SINGLE + ' '
  SETCOLOR(_COLOR_MENU)
  _AffMsg(23,00,_RechMsg('JT_RESUL002') + " "+ _CARACT_CROCHETS + " . " + _RechMsg('JT_RESUL003'), AM_ALIGN_CE ) // Choisissez … l'aide des touches [] . Confirmez avec [<Ù].
RETURN(NIL)

********

FUNCTION ResultBur
    PRIVATE lc_scrutin,lc_liste,ll_flag,lc_rep,lc_screen, lcEcran

    lcEcran := SAVESCREEN(17,09,21,69)
    SETCOLOR(_COLOR_MENU)
    @ 23,01 CLEAR TO 24,78
    @ 17,09,21,69 BOX B_SINGLE + ' '
    STORE SPACE(1) TO lc_rep

    IF (lc_rep := _GetRep( 'JT_RESUL024', _FALSE)) == _REPONSE_OUI
      IF testimp(_FALSE)
        SET EXACT OFF
        SET DEVICE TO PRINT
        @ PROW(), PCOL() SAY _IMPR_INIT
        DBSELECTAREA('BURVOTES')
        GO TOP
        lc_scrutin := ""
        lc_liste   := ""
        ll_flag    := _FALSE
        DO WHILE .NOT. EOF()
          SELECT RECUPVOTES
          SEEK STRZERO(BURVOTES->b_numero)
          IF .NOT. EOF()
            @ 1,1 SAY _RechMsg( 'JT_RESUL025') +STR(BURVOTES->b_numero,3)+" "+BURVOTES->b_bureau  // "Bureau: "
            @ 2,1
          ENDIF
          DO WHILE (.NOT. EOF()) .AND. BURVOTES->b_numero==rv_numbur

            DBSELECTAR('SCRUTINS')
            SEEK RECUPVOTES->rv_codescr
            IF ccLangue = _NEERLANDAIS
               IF (.NOT. EOF()) .AND. lc_scrutin # s_nomn
                 @ PROW()+3,1  SAY _RechMsg( 'JT_RESUL026') +s_nomn // "Scrutin: "
                 lc_scrutin := s_nomn
                 ll_flag    := _TRUE
               ENDIF
            ELSE
               IF (.NOT. EOF()) .AND. lc_scrutin # s_nomf
                 @ PROW()+3,1  SAY _RechMsg( 'JT_RESUL026') +s_nomf // "Scrutin: "
                 lc_scrutin := s_nomf
                 ll_flag    := _TRUE
               ENDIF
            ENDIF

            DBSELECTAR('LISTES')
            IF RECUPVOTES->rv_numlist == "XX"
              IF RECUPVOTES->rv_numcand="001"
                @ PROW()+1,10 SAY "Nombre de votes blancs exprim‚s: "+LTRIM(STR(RECUPVOTES->rv_nbrvote))
              ELSEIF RECUPVOTES->rv_numcand="002"
                @ PROW()+1,10 SAY "Nombre de votes blancs non exprim‚s: "+LTRIM(STR(RECUPVOTES->rv_nbrvote))
              ENDIF
            ELSE
              SEEK RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist
              IF (.NOT. EOF() .AND. lc_liste # l_numero) .OR. ll_flag
                @ PROW()+1,1
                @ PROW()+1,10 SAY "Liste: "+l_numero+" "+l_nom
                lc_liste := l_numero
                ll_flag  := _FALSE
              ENDIF
            ENDIF

            DBSELECTAR('CANDIDATS')
            IF RECUPVOTES->rv_numcand == "000"
              IF RECUPVOTES->rv_eff_sup == "A"
                @ PROW()+1,15 SAY "Nombre de votes exprim‚s pour la liste: "+LTRIM(STR(RECUPVOTES->rv_nbrvote))
              ELSEIF RECUPVOTES->rv_eff_sup == _CODE_EFFECTIF
                @ PROW()+1,15 SAY "Nombre de votes exprim‚s pour la tˆte de liste: "+LTRIM(STR(RECUPVOTES->rv_nbrvote))
              ELSEIF RECUPVOTES->rv_eff_sup == _CODE_SUPPLEANT
                @ PROW()+1,15 SAY "Nombre de votes nominatifs pour les suppl‚ants: "+LTRIM(STR(RECUPVOTES->rv_nbrvote))
              ENDIF
            ELSE
              SEEK RECUPVOTES->rv_codescr+RECUPVOTES->rv_college+RECUPVOTES->rv_numlist;
                   +RECUPVOTES->rv_eff_sup+RECUPVOTES->rv_numcand
              IF .NOT. EOF()
                @ PROW()+1,20 SAY PADR(trim(c_nom1)+' '+trim(c_nom2)+' '+trim(c_nom3),50)+": "+LTRIM(STR(RECUPVOTES->rv_nbrvote))+" votes obt."
              ENDIF
            ENDIF

            DBSELECTAR('RECUPVOTES')
            SKIP
          ENDDO
          EJECT
          DBSELECTAR('BURVOTE')
          SKIP
        ENDDO
        SET EXACT ON
        SET DEVICE TO SCREEN
      ENDIF
    ENDIF
    SETCOLOR(_COLOR_TRAV1)
    RESTSCREEN(17,09,21,69, lcEcran)
  RETURN(NIL)

*************************************************
* impression du procŠs-verbal de recensement
*************************************************
FUNCTION ImpRec
PARAMETER pl_confirme,;   && .T. s'il faut une confirmation d'impression
          pc_CodeScrut,;
          pn_NbCandidat

  PRIVATE lc_ecran,lc_rep,la_bur,ln_i,ln_compt,ll_continu

  lc_ecran = SAVESCREEN(17,00,24,79)
  IF pl_confirme
    STORE .F. TO ll_continu
    SETCOLOR(_COLOR_MENU)
    @ 23,01 CLEAR TO 24,78
    @ 17,06,21,73 BOX B_SINGLE + ' '
    STORE SPACE(1) TO lc_rep
    IF (lc_rep := _GetRep( 'JT_RECUP027', _FALSE)) == _REPONSE_OUI 
      IF testimp(.F.)
        STORE .T. TO ll_continu
      ENDIF
    ENDIF
  ELSE
    STORE .T. TO ll_continu
  ENDIF

  IF ll_continu
    DBSELECTAREA('PARAM')
    GO TOP
    DBSELECTAREA('BURVOTES')
    GO TOP

    ProcesVerb(pc_CodeScrut)

  ENDIF

  RESTSCREEN(17,00,24,79,lc_ecran)
  SETCOLOR(_COLOR_TRAV1)

RETURN(NIL)

********

FUNCTION ProcesVerb
  PARAMETER pcCodeScrut
  * recherche de Tous les codes scrutins pour cette ‚lection

  PRIVATE taVarImpr[15], lnNoPage := 3  ,lnNbrVote

  AFILL(taVarImpr, '')
  lcCodeTxt := IIF( ccNumArr # "002", "JT_RESUOPV2", "JT_RESULPV2" )
  taVarImpr[01] := 'PARAM->p_arron'
  taVarImpr[02] := 'PARAM->p_burdep'
  taVarImpr[03] := "IIF(ccNumArr=='002' .AND. ccLangue=='N', RechCantN(),PARAM->p_burdep)"
  taVarImpr[04] := "_RechMsg('JT_DATE')"
  taVarImpr[05] := 'STR(BURVOTES->b_numero,4)'
  taVarImpr[06] := 'SUBSTR(BURVOTES->b_statut,29,8)'
  taVarImpr[07] := 'STR(BURVOTES->b_nbr_votn,6)'
  taVarImpr[08] := "_RechMsg('JT_RESUL148')" // Bruxelles-Hal-Vilvorde
  taVarImpr[09] := "'"+_IMPR_12CPI+"'"
  taVarImpr[10] := "'"+_IMPR_10CPI+"'"
  taVarImpr[11] := "ccArron" // LIEGE ou HUY-WAREMME
  taVarImpr[12] := "TRIM(BURVOTES->b_bureau)" // Nom du canton
  SET DEVICE TO PRINT
  SET PRINT ON
  SET CONSOLE OFF
  SET CENTURY ON
  @ PROW(), PCOL() SAY _IMPR_INIT
  IF jt_msg->(DBSEEK(lcCodeTxt))
    lmMsgEcran := jt_Msg->msg_ecran&ccLangue
    DECLARE taMemo[1]  // MLCOUNT(lmMsgEcran, 80)]
    taLgImp := {}
    ImpMemo(@taMemo, 80, @lmMsgEcran , '1', @taVarImpr, _NB_LIG_PAGE, _TRUE )
    ChgTab('{0A0}')
    ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE ) // Haut page
    taLgImp := {}
    ChgTab('{000}')
    ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE ) // Haut page
    taLgImp := {}
    ChgTab('{001}')
    ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, 0           , _TRUE ) // Haut page
    DBSELECTAREA('BURVOTES')
    GO TOP
    taLgImp := {}
    ChgTab('{002}')
    SET DEVICE TO PRINT
    SET PRINT ON
    SET CONSOLE OFF
    DO WHILE .NOT. EOF()
          ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, 0           , _TRUE ) // Haut page
          SET DEVICE TO PRINT
          SET PRINT ON
          SET CONSOLE OFF
          IF PROW() > ( _NB_LIG_PAGE - 6 )
            lnNoPage := lnNoPage + 1
            taLgImp := {}
            ChgTab('{003}')
            ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, 0           , _TRUE ) // Haut page
            SET DEVICE TO PRINT
            SET PRINT ON
            SET CONSOLE OFF
            EJECT
            @ prow()+2, 0 say space(88)
            taLgImp := {}
            ChgTab('{001}')
            ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, 0           , _TRUE ) // Haut page
            taLgImp := {}
            ChgTab('{002}')
          ENDIF
      DBSELECTAREA('BURVOTES')
      DBSKIP()  
    ENDDO

    taLgImp := {}
    ChgTab('{003}')
    ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, 0           , _TRUE ) // Haut page
    taLgImp := {}
    ChgTab('{004}')
    SET DEVICE TO PRINT
    SET PRINT ON
    SET CONSOLE OFF
    IF (PROW() + LEN(taLgImp)) > ( _NB_LIG_PAGE - 6 )
      EJECT
      lnNoPage := lnNoPage + 1
      @ prow()+2, 0 say space(88)
    ENDIF
    ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE ) // Haut page
    tn_ind :=5
    taLgImp := {}
    ChgTab('{'+STRZERO(tn_Ind,3)+'}')
    DO WHILE LEN(taLgImp)#0
      SET DEVICE TO PRINT
      SET PRINT ON
      SET CONSOLE OFF
      lnNoPage := lnNoPage + 1
      @ prow()+2, 0 say space(88)
      ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE ) // Haut page
      tn_Ind := tn_Ind + 1
      taLgImp := {}
      ChgTab('{'+STRZERO(tn_Ind,3)+'}')
    ENDDO

  ENDIF
  SET CENTURY ON
RETURN(NIL)

****
FUNCTION ChgTab
  PARAMETER pcTypeLg

  LOCAL ln_i, nSavSelect := SELECT()

  SELECT(0)
  of_netuse(_REP_JITES+'\tmp' ,.T., 0, 'TEXTE')
  set index to (_REP_JITES+'\tmp')
  IF DBSEEK( pcTypeLg )
    DO WHILE LEFT(ligne_txt,5) == pcTypeLg .AND. !EOF()
      AADD(taLgImp, TRIM(SUBSTR(ligne_txt,6) ) )
      DBSKIP()
    ENDDO
  ENDIF
  TEXTE->(DBCLOSEAREA())
  DBSELECTAREA( nSavSelect )
RETURN(NIL)
