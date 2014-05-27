  #include "inkey.ch"
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include "jites.ch"

  PARAMETERS pcCommande, pcCandidats, pcDrive
/*
        ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ»
        º                    Logiciel  J I T E S                     º
        º                                                            º
        º                Just In Time Electoral System               º
        ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼
*/
  SET(_SET_SCOREBOARD,_OFF)
  SET(_SET_DATEFORMAT,"dd/mm/yyyy")
  SET century on
  SET(_SET_EXACT,_OFF)
  SET(_SET_DELETED,_ON)
  SET(_SET_EXCLUSIVE,_ON)
  SET(_SET_CONFIRM,_ON)
  SETCANCEL(.F.)
  
  PUBLIC  ccElecActive   := ''               ,;  // Election active Ex : 920229
          cdDateElec     := CTOD('  /  /  ') ,;  // Election active Ex : 29/02/92
          ccDateElec     := ''               ,;  // Election active en caractŠre 
          ccUniteActive  := ''               ,;  // Drive actif
          ccTypeElec     := 'EL'             ,;  // # 'EL' communale
          ccDateJour     := DATE()           ,;  // date systŠme
          ccHeureJour    := TIME()           ,;  // heure systŠme
          ccLangue       := _FRANCAIS        ,;  // Code langue
          ccLanguErr     := _FRANCAIS        ,;  // Code langue
          ccOsver        := OSVER()          ,;  // Version du DOS
          cddate                             ,;  // Valeur fichier PARAM.DBF
          ccArron                            ,;  // Libell‚ arrondissement
          ccNumArr                           ,;  // Num‚ro arrondissement
          ccArrPath                          ,;  // R‚pertoire arrondissement
          cnArrEff2, cnArrSup2               ,;  // Nb Effectifs,suppl. scrutin 2
          cnArrEff3, cnArrSup3               ,;  // Nb Effectifs,suppl. scrutin 3 COLLEGE FRANCAIS
          cnArrEff3F,cnArrSup3F              ,;  // Nb Effectifs,suppl. scrutin 3 COLLEGE NERLANDAIS
          ccDistrict                         ,;  // Intitul‚ district
          ccCodeDist     := ''               ,;  // Code district
          ccCodeCant     := ''               ,;  // Code Canton
          ccCodeComm     := ''               ,;  // Code Commune
          ccCommun       := ''               ,;  // Intitul‚ commune
          ccNumCan                           ,;  // Num‚ro commune
          ccBureau                           ,;  // Nø bureau
          ccResponsable                      ,;  // Pr‚sident du bureau de d‚pouillement
          ccRegLing                          ,;  // R‚gime linguistique
          cnNbrBur                           ,;  // Nombre de bureaux de votes
          ccStatut                           ,;  // Statut global ‚lection
          ccEtat                             ,;  //
          ccTransfert                        ,;  // Variable des transfert pour qd on utilise errorlevel()
          ccDriveActif   := _UNITE_A + ':'   ,;  // drive actif
          ccRep          := ''               ,;  // Nom du r‚pertoire suppl‚mentaire ou se trouve les fichiers
          ccOperElec     := ''               ,;  // Code op‚ration ‚lectorale
          ccVList        := ''               ,;  // Valide liste
          ccVCant        := ''               ,;  // Valide le canton
          clPlantage     := _FALSE           ,;  // Valide le canton
          Cryp_Resul                         ,;  // Cryptage des r‚sultats enregistr‚s sur disquette (True si oui; False si non)
          PassWord       := '0000000000'     ,;  // Mot de passe g‚n‚rale permettant de l'avoir toujours en m‚moire
          PassWord2      := '0000000000'     ,;  // Mot de passe g‚n‚rale permettant de l'avoir toujours en m‚moire
          PassWordPc     := '0000000000'     ,;  // Mot de passe g‚n‚rale permettant de l'avoir toujours en m‚moire
          TypeResul      := 0                ,;  // Type d'impression de r‚sulatats
          gnBurMin       := 0                ,;  // nø du bureau mininum pour la d‚charge
          gnBurMax       := 0                ,;  // nø du bureau maximum pour la d‚charge
          PassWordElect  := '2703'          ,;  // Cl‚ interne de l'‚lection  (5377)
          ccNoPc         := ''               ,;     // nø du pc de totalisation
          _IMPR_10CPI        ,;
          _IMPR_12CPI        ,;
          _IMPR_INIT         ,;
          _IMPR_TYPE         ,;
          D_IMPR_10CPI       ,; // * Imprimamtes Digivotes
          D_IMPR_12CPI       ,;            
          D_IMPR_INIT        ,;
          D_IMPR_TYPE        ,;
          J_IMPR_10CPI       ,; // * Imprimantes Jites
          J_IMPR_12CPI       ,;             
          J_IMPR_INIT        ,;                 
          J_IMPR_TYPE          // 
* 5377 -> 2701
*(2,4,5,8) -> (1,4,6,7)
*
* << 2010 à faire
*  2701 -> 2703
*  (1,4,6,7) -> (1,3,6,8)
* 2010 >>
  Cryp_Resul  := _TRUE
*  Cryp_Resul  := _FALSE
  cnTempoBox    := _caltempo()

  PRIVATE ccOldMode   := GETMODE()

  ccUniteActive := IIF(pcDrive == NIL, _UNITE_C, pcDrive)

* CGA80()   // mode CGA 80 colonnes

  cgCEE          := _FALSE               // Election avec ‚lecteurs de la CEE
  cgbBEL         := _FALSE               // Election avec ‚lecteurs Belge … l'‚tenger
  cgbSup         := _TRUE                // Election avec Suppl‚ants

  // V‚rification qu'il n'y a pas eu d'interruption du programme
  * Ouverture du fichier des ‚lections
  _use("elect",_PATH_JITES + '\' ,_SELECT_LES_ELECTIONS, _EXCLUSIVE)
  LOCATE ALL FOR SUBSTR(elec_vers,4,1) == "*"
  IF FOUND()
    * Ouverture et reindex du fichier des messages
    _use("jt_msg",_PATH_JITES + '\' ,_SELECT_MESSAGES,_EXCLUSIVE)
    lcFile :=  _PATH_JITES+"\jt_msg.ntx"
    INDEX ON msg_numero TO &lcFile
    USE
  ENDIF
  DBSELECTAREA( 'ELECT' )
  USE
  IF pcCandidat # 'CANDIDATS' .AND. pcCandidat # 'LISTES'
     * Ouverture du fichier de l'arrondissement
     _use( "jtarrond" ,_PATH_JITES + '\' , _SELECT_ARRONDISSEMENT , _EXCLUSIVE, 'ARROND')
     DBGOTOP()
     ccNumArr  := ARROND->aron_code
     ccArron   := ARROND->aron_nom
     ccArrPath := ARROND->aron_path
     cnArrEff2 := ARROND->aron_eff2
     cnArrSup2 := ARROND->aron_sup2
     cnArrSup3 := ARROND->aron_sup3
     cnArrSup3F:= ARROND->aron_sup3F
     cnArrEff3 := ARROND->aron_eff3
     cnArrEff3F:= ARROND->aron_eff3F

     _IMPR_10CPI    :=''
     _IMPR_12CPI    :=''
     _IMPR_INIT     :=''
     _IMPR_TYPE     :='        '
     D_IMPR_10CPI := CHR(27)+'&k0S'
     D_IMPR_12CPI := CHR(27)+'&k4S'
     D_IMPR_INIT  := ''
     D_IMPR_TYPE  := 'Mode HP'

     J_IMPR_10CPI := CHR(27)+'P'
     J_IMPR_12CPI := CHR(27)+'M'             
     J_IMPR_INIT  := CHR(27)+'@'
     J_IMPR_TYPE  := 'Mode FX'


     DO CASE
       CASE ccNumArr == _ARROND_BRUXELLES
         IF !ChLangue()
           CLOSE DATABASE
           DIRCHANGE(_PATH_JITES)
           @ 0, 0 clear
           QUIT
         ENDIF
       CASE ccNumArr == _ARROND_HASSELT
          ccLangue  := _NEERLANDAIS
          ccLanguErr:= _NEERLANDAIS
       OTHER
          ccLangue  := _FRANCAIS
          ccLanguErr := _FRANCAIS
     ENDCASE
     EcrBase()
  ELSE
    IF FILE('jt_mem.mem')
      SvTransfert := ccTransfert
      RESTORE FROM jt_mem.mem ADDITIVE
      ccTransfert := SvTransfert
    ENDIF
  ENDIF

  _IMPR_10CPI := J_IMPR_10CPI
  _IMPR_12CPI := J_IMPR_12CPI
  _IMPR_INIT := J_IMPR_INIT
  _IMPR_TYPE := J_IMPR_TYPE

  IF  ccLangue  == _NEERLANDAIS
     _IMPR_TYPE :=  SUBSTR(_IMPR_TYPE,6,2) + ' MODUS' 
  ENDIF

  IF .NOT. (NIL == pcCommande)
    IF (pcCommande $ _LISTE_COMMANDE .AND. LEN(pcCommande) = 1) 
      pcCommande = UPPER(pcCommande)
      IF pcCommande = _MENU_RESULTAT   // V‚rifications avant traitement
        jtElect(_MENU_RESULTAT)
  
        IF !EMPTY(ccElecActive)
           * Sortie normal de JITES => flag REINDEX … faux
           * Ouverture du fichier des ‚lections
           _use("elect",_PATH_JITES + '\' ,_SELECT_LES_ELECTIONS, _EXCLUSIVE)
           LOCATE ALL FOR ccElecActive == RIGHT(elec_dir, 6 )
           IF FOUND()
             of_reclock(0)
             REPLACE elec_vers  WITH _VERSION_JITES
             UNLOCK
           ENDIF

           * Effacer les DBF
           IF pcCommande == _MENU_RESULTAT 
              CLOSE DATABASES
           ENDIF

        ENDIF

      ENDIF
    ELSE
      * Code commande trop long ou incompatible
      ? 'Debug : Code commande trop longue ou incompatible'
      a = inkey(0)
    ENDIF
  ELSE
    * Manque le paramŠtre du choix de la proc‚dure
    ? 'Debug : Manque le paramŠtre du choix de la proc‚dure'
    a = inkey(0)
  ENDIF
  CLOSE DATABASE
  DIRCHANGE(_PATH_JITES)
  @ 0, 0 clear
RETURN

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ECRAN DE BASE ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION EcrBase
  SETCOLOR(_COLOR_OMBRE)
  CLEAR SCREEN
  @ 01, 01, 22, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
 _affmsg(00, 00, "                                                  TOT "+ ccNoPc + "  V "+_VERSION_JITES , AM_ALIGN_CE, AM_EFF_OUI, _COLOR_LIG1)
  @ 01,70 SAY _IMPR_TYPE
  @ 23, 01 SAY SPACE(78) COLOR _COLOR_MENU
  @ 24, 01 SAY SPACE(78) COLOR _COLOR_MENU
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ LIGNE STATUT ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION LigStatut
  PARAMETER pcInfo
  LOCAL cLig, cLig1, cNomCant
  * Affiche la ligne 01
  * Si Info $ C --> affiche le canton
  * Si Info $ D --> affiche le District
  * Si Info $ M --> affiche la commune
  IF pcInfo == NIL
    pcInfo := 'C'
  ENDIF
  SETCOLOR(_COLOR_LIG1)
  cLig := ''
  cLig1:= ''
  IF 'C' $ pcInfo  // canton
    IF ccLangue == _NEERLANDAIS .AND. ccNumArr == _ARROND_BRUXELLES
      cLig := ' ³ ' + _RechMsg('JT_JITES002')+' '
      IF LEFT(ccBureau,11) == 'Saint-Josse'
        cLig += 'Sint-Joost-ten-Node' // Canton :
      ELSE
        cLig += TRIM(ccBureau) // Canton :
      ENDIF
    ELSE
      cLig := ' ³ ' + _RechMsg('JT_JITES002')+' '+TRIM(ccBureau) // Canton :
    ENDIF
  ENDIF
  IF 'D' $ pcInfo  // District
    cLig := ' ³ ' + 'District : '+TRIM(ccDistrict)
  ENDIF
  IF 'M' $ pcInfo  // commune
    IF 'C' $ pcInfo
      cLig1 := ' ³ ' + _RechMsg('JT_JITES003')+' '+TRIM(ccCommun)   // Commune
    ELSE
      cLig := ' ³ ' + _RechMsg('JT_JITES003')+' '+TRIM(ccCommun)   // Commune
    ENDIF
  ENDIF
  ccDateElec  := RIGHT(ccElecActive,2)+'/'+SUBSTR(ccElecActive,3,2)+'/'+'20'+LEFT(ccElecActive,2)

* @ 01,01 SAY left(_RechMsg('JT_JITES001')+' '+ccDateElec + space(50),78)
  @ 00,01 SAY left(_RechMsg('JT_JITES001')+' '+ccDateElec + space(25)+'TOT '+ ccNoPc +space(4)+"V "+_VERSION_JITES+ space(10),78)
  @ 01,70 SAY _IMPR_TYPE
  IF !EMPTY( cLig1 )
    @ 02,01 SAY left( SPACE(34) + cLig1+space(50),78)
  ENDIF

RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ AFFICHE TITRE ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION AffTitre
  PARAMETERS pc_chaine, pnNoLig

  pnNoLig := IIF(pnNoLig == NIL , 3, pnNoLig)

  SETCOLOR(_COLOR_OMBRE)
  @ pnNolig+1 ,08 ,pnNoLig+3, 8+LEN(pc_chaine)+5 BOX REPLICATE(CHR(176),9)
  SETCOLOR(_COLOR_TRAV1)
  @ pnNolig   ,06 ,pnNoLig+2, 6+LEN(pc_chaine)+5 BOX B_DOUBLE + ' '
  @ pnNoliG+1,09 SAY pc_chaine
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ JAUGE INIT ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION JaugeInit
  PARAMETERS nLigne
  SETCOLOR(_COLOR_OMBRE)
  @ nLigne+1,17,nLigne+6,67 BOX REPLICATE(CHR(176),9)
  SETCOLOR(_COLOR_TRAV1)
  @ nLigne,15,nLigne+5,65 BOX B_SINGLE + ' '
  @ nLigne+3,20 SAY REPLICATE(CHR(176),40)
  @ nLigne+4,20 SAY "0"
  @ nLigne+4,59 SAY "100 %"
  @ nLigne+4,42 SAY "%"
RETURN (NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ JAUGE EVOLUTION ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION Jauge
  PARAMETERS nLigne,nCur,nFin
  PRIVATE nCol,nPourcent
  nPourcent=(100/nFin)*nCur
  nCol=20+(40/100)*nPourcent
  @ nLigne+3,nCol SAY CHR(219)
  @ nLigne+4,38 SAY nPourcent PICT "99"
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ IMPRESSION D'UN MEMO ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ImpMemo

  PARAMETER paTabTxt, pnLong, pcChpMemo, pcImprime, paTabInst, pnLgPage, plImpGras

  PRIVATE lnCurElem, lnNbLig, lnNbLgImp, ln_i, lcImpr

  * pcImprime --> 1 Charge
  *           --> 2 Imprime
  *           --> 3 Charge et imprime

  IF pcImprime $ '13'
    IF .not. file(_REP_JITES+'\tmp.dbf')
      DO xf_CreatFic
      INDEX on left(ligne_txt,5) TO (_REP_JITES+'\tmp')
    else
      SELECT(0)
      of_netuse(_REP_JITES+'\tmp' ,.T., 0, 'TEXTE')
      INDEX on left(ligne_txt,5) TO (_REP_JITES+'\tmp')
    ENDIF
    ZAP

    // Chargement du m‚mo dans la dbf
    SET PRINTER TO (_REP_JITES+'\tmp')
    SET DEVICE TO PRINTER
    SET PRINTER ON
    SET CONSOLE OFF
    ? jt_msg->msg_ecran&ccLangue
    SET PRINTER OFF
    SET PRINTER TO
    SET DEVICE TO SCREEN
    SET CONSOLE ON
    APPEND FROM (_REP_JITES+'\TMP.prn') sdf
  ELSE
    SELECT(0)
    of_netuse(_REP_JITES+'\tmp' ,.T., 0, 'TEXTE')
    set index to (_REP_JITES+'\tmp')
  ENDIF
  IF pcImprime $ '23'
    lnNbLgImp = pnLgPage
    Set Device To Print
    SET PRINT ON
    SET CONSOLE OFF
    FOR lnCurElem = 1 TO LEN(paTabTxt)
      Set Device To Screen
      lcImpr  = Subtitu(paTabTxt[lnCurElem])
      Set Device To Print
      lnNbLgImp = lnNbLgImp - 1
      @ PROW()+1,01 SAY lcImpr
      IF lnNbLgImp <= 3 .and. pnLgPage # 0
        FOR ln_i = 1 TO lnNbLgImp
          @ PROW()+1, 01 say ' '
        NEXT
        lnNbLgImp = pnLgPage
      ENDIF
    NEXT
    IF pnLgPage # 0 .AND. LEN( paTabTxt) # 0
      Set Device To Print
      EJECT
*     FOR ln_i = 1 TO lnNbLgImp
*       @ PROW()+1, 01 say ' '
*     NEXT
    ENDIF
    Set Device To Screen
    SET PRINT OFF
    SET CONSOLE ON
  ENDIF

  TEXTE->(DBCLOSEAREA())

RETURN .t.

FUNCTION Subtitu
  PARAMETER pc_TxtImp

  PRIVATE tc_TxtImp, ind_Deb, ind_Fin, tc_Chaine, tr, Ch_Rempl, tn_LgCh,;
          tn_LongC, tc_Instruc, ln_i , tn_Deplac, tn_Deb, tn_Fin, tc_Indice

  tc_Indice := ''
  tc_TxtImp := pc_TxtImp
  DO WHILE AT("Û",tc_TxtImp) # 0
    tc_TxtImp := STUFF(tc_TxtImp, AT("Û",tc_TxtImp) , 1, _IMPR_GRAS_ON)
  ENDDO
  DO WHILE AT("Ü",tc_TxtImp) # 0
    tc_TxtImp := STUFF(tc_TxtImp, AT("Ü",tc_TxtImp) , 1, _IMPR_GRAS_OFF)
  ENDDO
  DO WHILE AT("Ý",tc_TxtImp) # 0
    tc_TxtImp := STUFF(tc_TxtImp, AT("Ý",tc_TxtImp) , 1, _IMPR_SOUL_ON)
  ENDDO
  DO WHILE AT("Þ",tc_TxtImp) # 0
    tc_TxtImp := STUFF(tc_TxtImp, AT("Þ",tc_TxtImp) , 1, _IMPR_SOUL_OFF)
  ENDDO
  DO WHILE AT("[",tc_TxtImp) # 0
    Ind_Deb := AT("[", tc_TxtImp)
    Ind_Fin := AT("]", tc_TxtImp)
    IF Ind_Fin < Ind_Deb
      Ind_Fin := Ind_Deb
    ENDIF
    tc_Chaine  := SUBSTR(tc_TxtImp, Ind_Deb+1, (Ind_Fin-Ind_Deb)-1)
    tc_Instruc := paTabInst[val(tc_Chaine)]
    Ch_Rempl   := &tc_instruc
    * REMPLACEMENT
    tn_LongC   := (Ind_Fin - Ind_Deb) + 1
    tn_LgCh    := LEN(ch_Rempl)
    IF LEN(Ch_Rempl) < tn_LongC
      Ch_Rempl := Ch_Rempl + SPACE( tn_LongC - tn_LgCh)
      tn_LgCh  := LEN(Ch_Rempl)
    ENDIF
    tc_Indice  := tc_Indice + IIF(.not. empty(Ch_Rempl), str(Ind_Deb, 3) + str(len(Ch_Rempl),3) ,'')
    tc_TxtImp  := stuff(tc_TxtImp,Ind_Deb, tn_LgCh , Ch_Rempl)
  ENDDO
  if plImpGras
    tn_Deplac := 0
    FOR ln_i=1 TO len(tc_Indice) STEP 6
      tn_Deb    := val(substr(tc_Indice, ln_i, 3))
      tc_TxtImp := stuff(tc_TxtImp,tn_Deb+tn_Deplac, 0, _IMPR_GRAS_ON)
      tn_Deplac := tn_Deplac + 2
      tn_Fin    := tn_Deb + val(substr(tc_Indice, ln_i+3, 3)) + tn_Deplac
      tc_TxtImp := stuff(tc_TxtImp,tn_Fin, 0, _IMPR_GRAS_OFF)
      tn_Deplac := tn_Deplac + 2
    NEXT
  ENDIF
RETURN TRIM(tc_TxtImp)

/*ÚÄÄÄÄÄÄÄÄ CONTROLE DE STRUCTURE D'UNE DBF ÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION of_ctrldbf

PARAMETER ;
          pc_dbf,;        && DBF qui contient les donn‚es
          pc_dbfref       && DBF de r‚f‚rence

PRIVATE ln_nbchp1, ln_nbchp2, lc_dbfref, lc_dbf, tl_modstru, tc_select, tc_tmp,;
        tc_tmp1, tc_filtmp, tn_increm

*      CONTROLE DU NOMBRE DE CHAMPS OU DE L'EXISTANCE DE LA DBF
tl_modstru = .f.
IF .NOT. FILE(pc_dbf)
  * CAS OU LA DBF N'EXISTE PAS
  SELECT(0)
  of_netuse(pc_dbfref, .t., 0, 'dbfref')
  COPY STRUCTURE TO &pc_dbf
  USE
ELSE
  * CAS OU LE NOMBRE DE CHAMPS NE CORRESPOND PAS
  SELECT(0)
  tc_select = LTRIM(str(select()))
  of_netuse(pc_dbfref, .t., 0, 'stru_ref')
  ln_nbchp1 = FCOUNT()
  ln_size1  = RECSIZE()
  SELECT(0)
  of_netuse(pc_dbf, .t., 0, 'stru_data')
  ln_nbchp2 = FCOUNT()
  ln_size2  = RECSIZE()
  USE
  IF ln_nbchp1# ln_nbchp2 .OR. ln_size1 # ln_size2
    tl_modstru = .t.
    tn_increm = 1
    DO WHILE .t.
      tc_FILTMP = stuff(pc_dbf, IIF(rat('\',pc_dbf)#0,rat('\',pc_dbf)+1,1), 2, '#'+str(tn_increm,1))
      if file(tc_filtmp)
        tn_increm = tn_increm + 1
      ELSE
        EXIT
      ENDIF
    ENDDO
    SELECT stru_ref
    COPY STRUCTURE TO &tc_FILTMP
    of_netuse(TC_FILtmp, .t., 0, 'temp')
    APPEND FROM &pc_dbf
    USE
    DELETE FILE &pc_dbf
    RENAME &tc_FILtmp TO &pc_dbf
    tc_tmp1 = left(tc_FILtmp,rat('.',tc_FILtmp))+ 'dbt'
    IF file(tc_tmp1)
      tc_tmp = left(pc_dbf,rat('.',pc_dbf))+ 'dbt'
      DELETE FILE &tc_tmp
      RENAME &tc_tmp1 TO &tc_tmp
    ENDIF
  ENDIF
  SELECT &tc_select
  USE
ENDIF
RETURN tl_modstru
*
FUNCTION ChLangue
  PRIVATE nChLang, llContinue
  SETCOLOR(_COLOR_OMBRE)
  CLEAR SCREEN
  @ 01, 01, 24, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
*  _affmsg(00, 00, "J u s t   I n   T i m e   E l e c t o r a l   S y s t e m   "+"V "+_VERSION_JITES , AM_ALIGN_CE, AM_EFF_OUI, _COLOR_LIG1)
  @ 5, 10 , 19, 70 BOX B_SINGLE + ' '
  @ 8, 25 SAY 'Veuillez choisir votre langue'
  @10, 27 SAY 'Gelieve uw taal te kiezen'
  SETCOLOR( _COLOR_LIG1 )
  @ 14, 11 CLEAR TO 18, 26
  @ 14, 54 CLEAR TO 18, 69
  nChLang := 0
  DO WHILE nChLang == 0 .AND. LASTKEY() # K_ESC
    SETCOLOR(_COLOR_TRAV1)

   @ 16, 57 PROMPT 'Nederlands'
   @ 16, 15 PROMPT 'Fran‡ais'

*    @ 16, 57 PROMPT 'Fran‡ais'
*    @ 16, 15 PROMPT 'Nederlands'
    MENU TO nChLang
  ENDDO
  IF LASTKEY() # K_ESC .AND. nChLang # 0
    IF (nChLang == 1 .AND. LASTKEY() == 13) .OR. (nChLang == 2 .AND. LASTKEY() == 102)
      ccLangue  := _FRANCAIS
      ccLanguErr:= _FRANCAIS
    ELSE
      ccLangue  := _NEERLANDAIS
      ccLanguErr:= _NEERLANDAIS
    ENDIF
    llContinue := _TRUE
  ELSE
    llContinue := _FALSE
  ENDIF
  SETCOLOR(_COLOR_TRAV1)
RETURN llContinue
*---------------------------------------------------------------------
Procedure xf_CreatFic   && cree le fichier … partir du .Txt
*---------------------------------------------------------------------
PARAMETERS PC_FilTxt

  select(0)
  create tmp1
  of_addrec(0)
  replace field_name  with 'LIGNE_TXT' ,;
         field_type  with 'C'         ,;
         field_len   with 254         ,;
         field_dec   with 0
  create (_REP_JITES+'\tmp') from tmp1
  use
  erase tmp1.dbf
  select(0)
  of_netuse( _REP_JITES+'\tmp' , .t., 0, 'TEXTE')
RETURN
*
*


* *********************************************
* Fonction pour menus verticaux d'une base de donn‚es
* ***************************************************

FUNCTION MenuBase

Parameters a,b,ASG,init,Lig,Long

Local Fen1,Fen2,Place,Nb,Total,BP,MaxL,BL,Lettre,Debut

Set cursor off

If Lig > Long
     Lig=Long
Endif

MaxL=Len(ASG[1])
For Nb=2 to Long
        If Len(ASG[Nb])>MaxL
                MaxL:=Len(ASG[Nb])
        Endif
Next
If MaxL>75
        MaxL=75
Endif
For Nb=1 to Long
        If Len(ASG[Nb])<MaxL
                BL=Replicate(chr(32),MaxL-Len(ASG[Nb]))
                AFILL(ASG,ASG[Nb]+BL,Nb,1)
        Endif
        If Len(ASG[Nb])>75
                AFILL(ASG,left(ASG[Nb],75),Nb,1)
        Endif
Next

Total=Lig

If type("a")="U"
        a=0
Endif
If type("b")="U"
        b=0
Endif
If type("init")="U"
        init=1
Endif

If type("ASG")="U"
        Return(0)
Endif

MCHOIX=0
NInkey:=0
Place=Init
@a,b,a+Total+1,b+1+MaxL BOX B_SINGLE + space(1)

BP=Place
If Place+Total-1>Long
        BP=Place-Long+Total
        Fen1=Long-Total+1
        Fen2=Long
else
        BP=1
        Fen1=Place
        Fen2=Place+Total-1
Endif
Debut=1
For Nb=Fen1 to Fen2
        @a+Debut,b+1 say ASG[Nb]
        Debut=Debut+1
next
@a+BP,b+1 say ASG[Place] color _NOIR+"/"+_BLANC

Do While NInkey!=K_RETURN
        NInkey:=INKEY(0)
        @a+BP,b+1 say ASG[Place] color _BLANC+"/"+_NOIR
        If NInkey=K_DOWN
                BP=BP+1
                Place=Place+1
                If Place>Long
                        Place=1
                        Debut=1
                        For Nb=1 to Total
                                @a+Debut,b+1 say ASG[Nb]
                                Debut=Debut+1
                        Next
                        BP=1
                Endif
                If BP>Total
                        Debut=1
                        For Nb=Place-Total+1 to Place
                                @a+Debut,b+1 say ASG[Nb]
                                Debut=Debut+1
                        Next
                        BP=Total
                Endif
        Endif
        If NInkey=K_UP 
                Place=Place-1
                BP=BP-1
                If Place=0
                        Place=Long
                        Debut=1
                        For Nb=Long-Total+1 to Place
                                @a+Debut,b+1 say ASG[Nb]
                                Debut=Debut+1
                        Next
                        BP=Lig
                Endif
                If BP=0
                        Debut=1
                        For Nb=Place to Place+Total-1
                                @a+Debut,b+1 say ASG[Nb]
                                Debut=Debut+1
                        Next
                        BP=1
                Endif

        Endif
        If  NInkey=K_PGDN
                If BP=1
                        Place=Place+Total
                else
                        Place=Place+Total-Lig+1
                endif
                BP=1
                Fen1=Place
                Fen2=Place+Total-1
                If Place>Long-Lig
                        Place=Long
                        BP=Total
                        Fen1=Long-Total+1
                        Fen2=Long
                Endif
                Debut=1
                For Nb=Fen1 to Fen2
                        @a+Debut,b+1 say ASG[Nb]
                        Debut=Debut+1
                Next
        Endif
        If  NInkey=K_PGUP 
                If BP=1
                        Place=Place-Total
                else
                        Place=Place-Lig
                endif
                BP=1
                If Place<=0
                        Place=1
                        BP=1
                Endif
                Debut=1
                For Nb=Place to Place+Total-1
                        @a+Debut,b+1 say ASG[Nb]
                        Debut=Debut+1
                Next
        Endif
        If NInkey=K_HOME
                BP=1
                Place=1
                For Nb=1 to Total
                        @a+Nb,b+1 say ASG[Nb]
                next
        Endif
        If NInkey=K_END
                BP=Total
                Place=Long
                Debut=1
                For Nb=Place-Total+1 to Place
                        @a+Debut,b+1 say ASG[Nb]
                        Debut=Debut+1
                Next
        Endif
        If (NInkey>=97 .AND. NInkey<=122) .OR. ;
            (NInkey>=65 .AND. NInkey<=90) 
                Lettre=upper(Chr(NInkey))
                For Nb=1 to Long
                        If Upper(Left(ASG[Nb],1))=Lettre
                                Place=Nb
                                BP=1
                                Debut=1
                                If Place+Total-1>Long
                                        BP=Place-Long+Total
                                        Fen1=Long-Total+1
                                        Fen2=Long
                                else
                                        BP=1
                                        Fen1=Place
                                        Fen2=Place+Total-1
                                Endif
                                For IR=Fen1 to Fen2
                                        @a+Debut,b+1 say ASG[IR]
                                        Debut=Debut+1
                                Next
                                NB=Long
                        Endif
                Next
        Endif
        If NInkey=K_ESC
                MCHOIX=0
                Return(0)
        Endif
        @a+BP,b+1 say ASG[Place] color _NOIR+"/"+_BLANC
Enddo
MCHOIX=Place

@a,b clear to a+Total+1,b+2+MaxL

Set cursor on
RETURN

* jt_divers

*************************************************
* FUNCTION GDTestPres                           *
* Presence testing of URNE.TBL or MACHVOTS.TBL  *
*************************************************
FUNCTION GDTestPres(pcDrive,;
                    pnNumLec,;
                    plSuite)
LOCAL lnReturn:=0,lcBuffer,llConti,;
      lcElecAct:=RIGHT(ccElecActive,2)+SUBSTR(ccElecActive,3,2)+LEFT(ccElecActive,2)
plSuite := IIF(plSuite==NIL,_TRUE,plSuite)
llConti := _TRUE
DO WHILE FILE(pcDrive+"\urne.ind") .AND. lnReturn # K_ESC .AND. llConti
   lcBuffer := FILESTR(pcDrive+"\urne.ind")
   IF SUBSTR(lcBuffer,10,6) == lcElecAct
      lnReturn := MsgGenDsk("JT_GENDK143",IIF(plSuite,"SQ","Q"),STR(pnNumLec,1),_TRUE)   // Already a BALLOT BOX floppy in the drive
      IF !plSuite
         lnReturn := K_ESC
      ENDIF
   ELSE
      llConti := _FALSE
   ENDIF
ENDDO
llConti := _TRUE
DO WHILE FILE(pcDrive+"\newelect.txt") .AND. lnReturn # K_ESC .AND. llConti
   lcBuffer := FILESTR(pcDrive+"\totali.zip")
   IF SUBSTR(lcBuffer,9,6) == lcElecAct
      lnReturn := MsgGenDsk("JT_GENTO014",IIF(plSuite,"SQ","Q"),STR(pnNumLec,1),_TRUE)   // Already a TOTALISATION PC floppy in the drive
      IF !plSuite
         lnReturn := K_ESC
      ENDIF
   ELSE
      llConti := _FALSE
   ENDIF
ENDDO
RETURN(lnReturn)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Fonction affichage d'un message + wait            ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION MsgGenDsk
  PARAMETER pcNumero,;
            pcTchePermis,;
            pcMsgSuite,;
            plSavLig

  PRIVATE lnReturn:=0,cMsg,lcSavLig

  IF LEN(pcNumero) > 11
    cMsg := pcNumero
  ELSE
    cMsg := _RechMsg(pcNumero)
  ENDIF
  IF ccLAngue= _NEERLANDAIS
    IF pcTchePermis ='Q'
       pcTchePermis='E'
    ENDIF
    IF pcTchePermis ='S'
       pcTchePermis='V'
    ENDIF

    pcTchePermis := IIF(pcTchePermis==NIL,"VE",pcTchePermis)
  ELSE
    pcTchePermis := IIF(pcTchePermis==NIL,"SQ",pcTchePermis)
  ENDIf
  pcMsgSuite := IIF(pcMsgSuite==NIL,"",pcMsgSuite)
  plSavLig := IIF(plSavLig==NIL,_FALSE,plSavLig)

  IF plSavLig
     lcSavLig := SAVESCREEN(23,0,24,79)
  ENDIF
    _AffMsg(23,00,SPACE(80) )
    _AffMsg(24,00,SPACE(80) )


  _AffMsg(23,01, cMsg +SPACE(1)+pcMsgSuite, AM_ALIGN_CE, AM_EFF_CENTRE, _COLOR_MENU)
  _AffMsg(24,01, " "+_RechMsg('JT_GENDK032'), AM_ALIGN_GA, AM_EFF_CENTRE, _COLOR_MENU) // Appuyez sur une touche pour continuer
  IF pcTchePermis $ 'QE'
    _AffMsg(24,01, _RechMsg('JT_GENDK040')+" ", AM_ALIGN_DR, AM_EFF_NON, _COLOR_MENU) // [Q]uitter
  ELSE
    IF pcTchePermis $ 'SV'
            _AffMsg(24,01, _RechMsg('JT_GENDK047')+" ", AM_ALIGN_DR, AM_EFF_NON, _COLOR_MENU) // [S]uite
    ELSE
            _AffMsg(24,01, _RechMsg('JT_GENDK033')+" ", AM_ALIGN_DR, AM_EFF_NON, _COLOR_MENU) // [S]uite [Q]uitter
    ENDIF
  ENDIF

  CLEAR TYPEAHEAD   && vider le contenu du buffer

  DO WHILE  !( UPPER(CHR((lnReturn := INKEY(0)))) $ pcTchePermis )
  ENDDO
  IF ccLangue = _NEERLANDAIS
    IF UPPER(CHR(lnReturn)) == "E"
       lnReturn := K_ESC
    ENDIF
  ELSE
    IF UPPER(CHR(lnReturn)) == "Q"
       lnReturn := K_ESC
    ENDIF
  ENDIF
  IF plSavLig
     RESTSCREEN(23,0,24,79,lcSavLig)
  ENDIF

RETURN lnReturn

*************************************************
* FONCTION UTReindex                            *
* Re-indexation of all election directories     *
*************************************************
FUNCTION UTReindex
LOCAL lcSavRep:=ccRep,lcSavLig:=SAVESCREEN(23,0,24,79)
PRIVATE lcFile
 _AffMsg(23,01,"Herindexatie is bezig ...")
  VerifNtx(_INDEX_ALL_FILE)

  DBCLOSEALL()
  * opening of the messages file
  _use("jt_msg",_PATH_JITES + '\' ,_SELECT_MESSAGES,_EXCLUSIVE)
  lcFile :=  _PATH_JITES+"\jt_msg.ntx"
  INDEX ON msg_numero TO &lcFile

  * Opening of the elections file
  _use("elect",_PATH_JITES + '\' ,_SELECT_LES_ELECTIONS, _EXCLUSIVE)
  lcFile :=  _PATH_JITES+"\elect.ntx"
  INDEX ON right(elec_dir,6)+IIF(left(elec_dir,2)=='EL','ZZ',left(elec_dir,2)) TO &lcFile

  ccRep := lcSavRep
  RESTSCREEN(23,0,24,79,lcSavLig)
RETURN(NIL)



*********************************
* PROCEDURE beep                *
*       Produce a beep          *                    
*********************************

PROCEDURE beep
TONE(760,3)
RETURN

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³       Tester l'impression                         ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION TestImp
  PARAMETER plConfirm   && .T. si demande confirmation

  PRIVATE leOldCoul, llReturn

  STORE SETCOLOR() TO leOldCoul
  SETCOLOR(_COLOR_MENU)

  plConfirm := IIF( plConfirm == NIL, _TRUE, plConfirm)

  PRIVATE ll_ok
  IF plConfirm
    DO beep
    _AffMsg(24,1, _RechMsg('JT_LISTE017'), AM_ALIGN_CE, AM_EFF_CENTRE) // "Confirmez l'impression par [<Ù] ou [Esc] pour l'annuler"
    INKEY(0)
  ELSE
    KEYBOARD CHR(K_RETURN)
    INKEY(0)
  ENDIF
  IF LASTKEY() == K_RETURN
    STORE _TRUE TO ll_ok
    DO WHILE .NOT. ISPRINTER() .AND. ll_ok
      DO beep
      _AffMsg(23,1, _RechMsg('JT_LISTE018'), AM_ALIGN_CE, AM_EFF_CENTRE) // "V‚rifier l'imprimante"
      _AffMsg(24,1, _RechMsg('JT_LISTE019'), AM_ALIGN_GA, AM_EFF_CENTRE) // " Appuyez sur une touche pour continuer"
      _AffMsg(24,2, _RechMsg('JT_LISTE020'), AM_ALIGN_DR)  // "[Esc] pour annuler"
      INKEY(0)
      IF LASTKEY() = K_ESC
         STORE _FALSE TO ll_ok
      ENDIF
    ENDDO
    IF ll_ok

      _AffMsg(24,1, _RechMsg('JT_LISTE021'), AM_ALIGN_CE, AM_EFF_CENTRE)  // "...  IMPRESSION EN COURS  ..."

      llReturn := _TRUE
    ELSE
      llReturn := _FALSE
    ENDIF
  ELSE
    llReturn := _FALSE
  ENDIF
  SETCOLOR(leOldCoul)
RETURN llReturn




