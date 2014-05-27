  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'jites.ch'

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Affiche CORPS de l'‚cran des bureaux de votes     ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION CorpsRecup
  SETCOLOR(_COLOR_OMBRE)
  @ 09,05,21,77 BOX REPLICATE(CHR(176),9)
  SETCOLOR(_COLOR_TRAV1)
  @ 08,03,20,75 BOX B_SINGLE + ' '
  @ 09,07 SAY _RechMsg('JT_RECUP109') // '    Bureaux de votes
  @ 10,03 TO 10,74
  @ 10,07 TO 19,07
  @ 10,07 SAY "Â"
  @ 20,07 SAY "Á"
  @ 09,38 TO 19,38
  @ 08,38 SAY "Â"
  @ 10,03 SAY "Ã"
  @ 10,38 SAY "Å"
  @ 10,75 SAY "´"
  @ 20,38 SAY "Á"
  SETCOLOR(_COLOR_MENU)
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ Affiche une ligne des bureaux de votes            ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION AffRecup
  PARAMETERS pc_color,pn_lig
  LOCAL lnBur
  IF pc_color="R"
    SETCOLOR(_COLOR_LIG1)
  ELSE
    SETCOLOR(_COLOR_TRAV1)
  ENDIF
  lnBur := IIF(ccNumArr=='002' .AND. PARAM->p_BurCommu == _BUR_TOTAL_COMMUNAL,;
               b_noburcom,b_numero)
  IF ccLangue = _FRANCAIS
    @ pn_lig,04 SAY STR(lnBur,3)+"³"+b_bureau+"³" + b_statut
  ELSE
    IF AT(_STAT_BUR_LECTURE,b_statut) # 0
      @ pn_lig,04 SAY STR(lnBur,3)+"³"+b_bureau+"³" + 'Lezing  ' + SUBSTR(b_statut,9,2)
    ELSE
      @ pn_lig,04 SAY STR(lnBur,3)+"³"+b_bureau+"³" + 'Scr.Disk' + SUBSTR(b_statut,9,2)
    ENDIF
  ENDIF
  SETCOLOR(_COLOR_TRAV1)
RETURN(NIL)


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ R‚cup‚ration d'une disquette provenant de l'urne  ³
  ³        (table.urn)                                ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION RecDisq
  PARAMETER pcOrigine

  PRIVATE lnNbLecture, lnNbEcriture , lnTotBur, ln_Record, lcFileInd

    DBSELECTAREA('BURVOTES')
    ln_Record := RECNO()
    GO TOP
    lnNbLecture := 0
    lnNbEcriture:= 0
    lnTotbur    := 0
    DO WHILE .NOT. EOF()
      DO CASE
        CASE AT(_STAT_BUR_LECTURE, b_statut) # 0
          lnNbLecture++
        CASE AT(_STAT_BUR_ECRITURE, b_statut) # 0
          lnNbEcriture++
      ENDCASE
      lnTotbur++
      DBSKIP()
    ENDDO

    GO ln_Record

    IF lnTotBur # (lnNbLecture + LnNbEcriture)
      _Message('JT_RECUP011', _TRUE) // toutes les disquettes ne sont pas ‚crites
    ELSE

      IF AT(_STAT_BUR_LECTURE,b_statut) # 0   && bureau en cours de traitement d‚j… lu
        _Message('JT_RECUP010', _TRUE) // d‚j… statut lecture
      ELSE
        IF lnNbLecture == 0  // Remise … z‚ro si premiŠre r‚cup‚ration
          DBSELECTAREA('RECUPVOTES')

          ZAP   && <<<<< ===== A T T E N T I O N

          lcFileInd := _PATH_ELECTION + '\rv'+ RIGHT(ccElecActive,6)

          INDEX ON STRZERO(rv_numbur) + rv_codescr + rv_college + rv_numlist ;
                + rv_eff_sup + rv_numcand TO &lcFileInd
          COMMIT
          DBSELECTAR('BURVOTES')
        ENDIF

        Recup(pcOrigine)   // R‚cup‚ration proprement-dite de la disquette

        ToOnTotalise() // Totalisation Si N bureaux r‚cup‚r‚s

      ENDIF

    ENDIF

RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ R‚cup‚ration du fichier des votes provenant de    ³
  ³ l'urne (table.urn)                                ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION Recup
  PARAMETER pcOrigine

  PRIVATE lnHandle,lnHdleBak,lcBuffer,lcDate,lnNbrVote,lnNbrNul,lcTime,lnNoDisk,gcDisk
  PRIVATE lcSavLig,llContiRecup,lcRep,lcOldCoul,llContinue:=_TRUE,lcbv,mpbv
  PRIVATE lcCrypt
  Public ccDir
  lcCrypt=PassWord + PassWordElect

  DBSELECTAREA('RECUPVOTES')
  GO TOP

  gcDisk := "'Urne' du bureau"+STR(BURVOTES->b_numero,3)+" "

  IF pcOrigine == _RECUPERATION
     IF InsDisk() # K_ESC
        DO WHILE (.NOT. DISKREADY(ccDriveActif)) .AND. LASTKEY() # K_ESC
           InsDisk() // Ins‚rez une disquette ds le lecteur
        ENDDO
        DO WHILE .NOT. DISKREADYW(ccDriveActif) .AND. LASTKEY() # K_ESC
          lnRetMsg := MsgGenDsk( 'JT_GENDK028') //  Erreur: disquette prot‚g‚e en ‚criture
        ENDDO

     ENDIF
     llContinue := (LASTKEY() # K_ESC)
  ENDIF
 IF !FILE('a:table.urn') .OR. !FILE('a:be.3')
     _Message('JT_RECUP151', _TRUE) // Cette Urne n'est pas cl“tur‚e
     llContinue:=K_ESC
     RETURN
 ENDIF

  IF llContinue

* Code H verif disquette v‚rification des fichiers de l'urne.
     
      Public MCHOIX,CScreen
      Declare ChoixMP:={}
      MChoix:=0
      Cscreen:=SAVESCREEN(15,29,21,60)
      @15,29 clear to 21,50
      @16,30 SAY _RechMsg('JT_GENDK123')

      IF ccLangue = _NEERLANDAIS
           AADD(ChoixMP," Automatisch ")
           AADD(ChoixMP,"   Manueel   ")
      ELSE
           AADD(ChoixMP," Automatique ")
           AADD(ChoixMP,"   Manuel    ")
      ENDIF
      MenuBase(17,35,ChoixMP,1,2,2)
      RESTSCREEN(15,29,21,60,CScreen)
      IF MCHOIX # 0
          IF FILE(ccDriveActif+"Log.err")
                  ccCRC= 'Del '+ccDriveActif+'log.err >nul'
                  RUN &ccCRC
          ENDIF

          Private lcPWPcTot

          lcPWPcTot := PassWord
          DO CASE
              CASE MCHOIX=1
                    lnHandle := FOPEN(ccDriveActif+"urne.ind")
                    lcBuffer := SPACE(17)
                    FREAD(lnHandle,@lcBuffer,17)
                    lcDate := STRTRAN(DTOC(PARAM->p_dateelec),"/","")
                    lcDate := substr(lcDate,1,4)+substr(lcDate,7,2)
                    IF LEFT(lcBuffer,15) <> PARAM->p_numarron+PARAM->p_numcan;
                              +STRZERO(BURVOTES->b_numero,3)+lcDate
        
                            * MAUVAIS JETON
                            _Message('JT_RECUP014', _TRUE) // Cette disquette ne correspond pas au bureau (jeton incorrect)
                            llContinue := K_ESC
                            RETURN
                    ELSE
                            lcbuffer:=substr(lcbuffer,7,3)
                            lcbv:=val(lcbuffer)
                            DBSELECTAREA('BURVOTES')
                            DBGOTOP()
                            LOCATE FOR B_NUMERO = lcBv
                            IF lcBV = B_NUMERO
                                    PassWord := lcPWPcTot          

                                    mpBv:=CRYPT(B_CODEBAK,lcCrypt)
                                    gsClefEnc = CRYPT(B_Clef2,lcCrypt)
                                    gsClefMac = CRYPT(B_Clef1,lcCrypt)

                                    ccDir="cd c:\Jites >nil"
                                    RUN &ccDir
                                    IF FILE('c:\jites\temp\table.urn')
                                            ccDir="deldir c:\Jites\temp >nil"
                                            RUN &ccDir
                                    ENDIF
                                    ccDir="md TEMP >nil"
                                    RUN &ccDir
                                    ccDir="cd c:\Jites\Temp >nil"
                                    RUN &ccDir

                                    ccDir="Copy a:Table.urn c:\jites\temp >nil"
                                    RUN &ccDir
                                    ccDir="Copy a:BE.3 c:\jites\temp >nil"
                                    RUN &ccDir
                                    ccDir="cd c:\Jites >nil"
                                    RUN &ccDir

                                    ccDir="c:\Jites\temp"
                                    ccCrc ='c:\jites\rijn ' + ccDir +' '+ ccDir + ' ' + gsClefMac +' '+ gsClefEnc
                                    DBSELECTAREA('RECUPVOTES')
                                    DBGOTOP()
                            ELSE
                                    * MAUVAIS JETON
                                    _Message('JT_RECUP014', _TRUE) // Cette disquette ne correspond pas au bureau (jeton incorrect)
                                    llContinue := K_ESC
                                    RETURN
                            ENDIF
                    ENDIF

              CASE MCHOIX=2
                Private lcNoMP
                TypeResul := 3
                lcNoMp:=jtacces(_TRUE)
                PassWord := lcPWPcTot
                TypeResul := 0
                IF !EMPTY(lcNoMP)
                  lnHandle := FOPEN(ccDriveActif+"urne.ind")
                  lcBuffer := SPACE(17)
                  FREAD(lnHandle,@lcBuffer,17)
                  lcDate := STRTRAN(DTOC(PARAM->p_dateelec),"/","")
                  lcDate := substr(lcDate,1,4)+substr(lcDate,7,2)
                  IF LEFT(lcBuffer,15) <> PARAM->p_numarron+PARAM->p_numcan;
                              +STRZERO(BURVOTES->b_numero,3)+lcDate
        
                            * MAUVAIS JETON
                            _Message('JT_RECUP014', _TRUE) // Cette disquette ne correspond pas au bureau (jeton incorrect)
                            llContinue := K_ESC
                            RETURN
                  ELSE
                            lcbuffer:=substr(lcbuffer,7,3)
                            lcbv:=val(lcbuffer)
                            DBSELECTAREA('BURVOTES')
                            DBGOTOP()
                            LOCATE FOR B_NUMERO = lcBv
                            IF lcBV = B_NUMERO
                                    mpBv:=CRYPT(B_CODEBAK, lcCrypt)
* << 2010 à faire
*  2701 -> 2703
*  (1,4,6,7) -> (1,3,6,8)
* 2010 >>
*                                   IF VAL(SUBSTR(mpBv,1,1)+SUBSTR(mpBv,4,1)+SUBSTR(mpBv,6,1)+SUBSTR(mpBv,7,1)) <> VAL(SUBSTR(lcNoMp,1,1)+SUBSTR(lcNoMp,4,1)+SUBSTR(lcNoMp,6,1)+SUBSTR(lcNoMp,7,1))
                                    IF VAL(SUBSTR(mpBv,1,1)+SUBSTR(mpBv,3,1)+SUBSTR(mpBv,6,1)+SUBSTR(mpBv,8,1)) <> VAL(SUBSTR(lcNoMp,1,1)+SUBSTR(lcNoMp,3,1)+SUBSTR(lcNoMp,6,1)+SUBSTR(lcNoMp,8,1))
                                          _Message('JT_ELECT012', _TRUE) // Disquette incompatible
                                          llContinue := K_ESC
 

                                          RETURN
                                    ENDIF
                            ENDIF
                  ENDIF
                ENDIF

                IF !EMPTY(lcNoMP)
                                    ccDir="cd c:\Jites >nil"
                                    RUN &ccDir
                                    IF FILE('c:\jites\temp\table.urn')
                                            ccDir="deldir c:\Jites\temp >nil"
                                            RUN &ccDir
                                    ENDIF
                                    ccDir="md TEMP >nil"
                                    RUN &ccDir
                                    ccDir="cd c:\Jites\Temp >nil"
                                    RUN &ccDir
                                    ccDir="Copy a:Table.urn c:\jites\temp >nil"
                                    RUN &ccDir
                                    ccDir="Copy a:BE.3 c:\jites\temp >nil"
                                    RUN &ccDir
                                    ccDir="cd c:\Jites >nil"
                                    RUN &ccDir

                                    ccDir="c:\Jites\temp"

                                    gcDisk := _RechMsg('JT_RECUP183') // Ins‚rer la disquette clef
                                    llContinue  := _FALSE
                                    DO WHILE ((.NOT. DISKREADY("a:")) .OR. llContinue = _FALSE ).AND. LASTKEY() # K_ESC
                                       IF FILE("a:\floppy.be")
                                          FILECOPY ('a:\floppy.be','c:\floppy.be')
                                          llContinue := _TRUE
                                       ELSE
                                         _Message('JT_ACCES015', _TRUE) // Cette Disquette ne contient pas de clefs
                                         InsDisk() // Ins‚rez une disquette ds le lecteur
                                       ENDIF
                                    ENDDO


                                    ccCrc ='c:\jites\rijndael r 3 ' + lcNoMP + ' '  + ccDir +' '+ ccDir + ' c:'
                                    mpBv = lcNoMP
                ELSE
                        llContinue := K_ESC
                        RETURN
                ENDIF

          ENDCASE
          PassWord := lcPWPcTot
          PassWord2 := lcPWPcTot
      ELSE
           llContinue := K_ESC
           RETURN
      ENDIF



      RUN &ccCRC
      IF FILE(ccDir +'/log.err')
         @23,70 SAY "Er1: log"
         _Message('JT_RECUP115', _TRUE) // Lecture du jeton impossible CRC
         lnRetMsg := K_ESC
         llContiRecup := _FALSE
      ELSE
         llContiRecup := _TRUE
      ENDIF

    IF llContiRecup
        * ouvrir le fichier r‚sultats
        lnHandle := FOPEN(ccDir+"\table.urn")
        IF FERROR() = 0
           llContiRecup := _TRUE
           IF !FILE(ccDriveActif+"1.dsk")
              lcSavLig := SAVESCREEN(23,0,24,79)
              lcOldCoul := SETCOLOR(_COLOR_MENU)
              SETCOLOR(_COLOR_MENU)
              @ 23,01 CLEAR TO 24,78
              lcRep := SPACE(1)
              _AffMsg(23,0,_RechMsg("JT_RECUP070"),AM_ALIGN_CE)  // DISQUETTE COPIE !!!
              IF _GetRep( 'JT_RECUP071') == _REPONSE_OUI
                 llContiRecup := _TRUE
              ELSE
                 llContiRecup := _FALSE
              ENDIF
              SETCOLOR(lcOldCoul)
              RESTSCREEN(23,0,24,79,lcSavLig)
           ENDIF

           IF llContiRecup

              lcBuffer := SPACE(17)
              * lecture NøArr+NøCanton+NøBur+date ‚lection (le jeton)
              IF FREAD(lnHandle,@lcBuffer,17) <> 17
                * problŠme lecture
                @23,70 SAY "Er2: lec U"
                _Message('JT_RECUP115', _TRUE) // Lecture du jeton impossible

              ELSE
                 * v‚rifier le Jeton
                  lcDate := STRTRAN(DTOC(PARAM->p_dateelec),"/","")
                  lcDate := substr(lcDate,1,4)+substr(lcDate,7,2)
                 IF LEFT(lcBuffer,15) <> PARAM->p_numarron+PARAM->p_numcan;
                                         +STRZERO(BURVOTES->b_numero,3)+lcDate
                    * MAUVAIS JETON
                    _Message('JT_RECUP014', _TRUE) // Cette disquette ne correspond pas au bureau (jeton incorrect)
                 ELSE
                    _AffMsg(23,01,_RechMsg('JT_RECUP021'), AM_ALIGN_CE, AM_EFF_CENTRE) // R‚cup‚ration des votes exprim‚s en cours...


                    DBSELECTAR('RECUPVOTES')
                    * supprimer les anciens
                    SEEK STRZERO(BURVOTES->b_numero,3)
                    DO WHILE rv_numbur==BURVOTES->b_numero .AND. (.NOT. EOF())
                      of_reclock(0)
                      DELETE
                      UNLOCK
                      SKIP
                    ENDDO

                    lnNbrVotant := 0
                    lnNbrVotn   := 0
                    lnNbrVote   := 0
                    lnNbrVots   := 0

                    IF Recup2(@lnNbrVotant,@lnNbrVotn,@lnNbrVote,@lnNbrVots,mpBv)
                       lnNbrNuln := 0
                       lnNbrNule := 0
                       lnNbrNuls := 0

                       lcTime  := TIME()

                       Decharge(lnNbrVotn,lnNbrVote,lnNbrVots,;
                               @lnNbrNuln,@lnNbrNule,@lnNbrNuls,lcTime, _FALSE)

                       DBSELECTAR('BURVOTES')
                       of_reclock(0)
                       REPLACE b_nbr_votn WITH lnNbrVotn
                       REPLACE b_nbr_vote WITH lnNbrVote
                       REPLACE b_nbr_vots WITH lnNbrVots

                       REPLACE b_nbr_nuln WITH lnNbrNuln
                       REPLACE b_nbr_nule WITH lnNbrNule
                       REPLACE b_nbr_nuls WITH lnNbrNuls
                       lnNoDisk := IF(AT(_STAT_BUR_LECTURE,b_statut) # 0, VAL(SUBSTR(b_statut,9,2))+1,1)
                        REPLACE b_statut WITH _STAT_BUR_LECTURE+" " + STR(lnNoDisk,2,0) + SPACE(6) + DTOC(DATE()) + "  " + lcTime
                       COMMIT
                       UNLOCK
                    ENDIF

                 ENDIF
              ENDIF

           ENDIF

        ELSE   && problŠme d'ouverture du fichier
          _Message('JT_RECUP015', _TRUE) // Il n'y a pas de vote sur cette disquette
        ENDIF

        FCLOSE(lnHandle)
   ENDIF
  ENDIF

  DBSELECTAR('RECUPVOTES')
  COMMIT

  DBSELECTAR('BURVOTES')
   ccDir="cd c:\Jites >nil"
   RUN &ccDir
   ccDir="deldir c:\Jites\temp >nil"
   RUN &ccDir

RETURN(NIL)


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ R‚cup‚ration du fichier des votes provenant de    ³
  ³ l'urne (table.urn)                                ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION Recup2

  PARAMETER pnNbVotant,pnNbVotn,pnNbVote,pnNbVots,mpBV

  PRIVATE lnLgBuf,lcBuffer,ln_ind,lnNbrVote,lnLg1Vote
  PRIVATE lcCodeScru,lnI,lc_lesvote,ll_return,nListe,nTotal,lcWinbuff
  PRIVATE laCodeScru:={},llReturn:=_TRUE
  PRIVATE nNbScrutin,lnTotCarteUrne,laTotCarteParScrutin:={},sCleElection, sClePcTot
  PRIVATE lnTotChiffre:=0 , lnTotListe:=0, lnTotEff:=0, lnTotSup:=0, lnTotTL:=0
  PRIVATE bErreurRecup :=_FALSE, I, nTotFichier := 0

  PRIVATE lcCrypt
  lcCrypt=PassWord + PassWordElect

  * Chargement de la table des codes scrutins
  DBSELECTAREA('SCRUTINS')
  DBGOTOP()
  DO WHILE !EOF()
     IF !EMPTY(s_nomf)
         AADD(laCodeScru,s_code)
     ENDIF
     DBSKIP()
  ENDDO

  lnLg1Vote := 4

  * compter le nombre de listes (TOUS SCRUTINS CONFONDUS)
  DBSELECTAREA('LISTES')
  DBGOTOP()
  nTotal := 0
  DO WHILE !EOF()
     nTotal := nTotal + 1
     DBSKIP()
  ENDDO

  lcWinbuff := SAVESCREEN(10,00,16,79)
  DBGOTOP()
  lcCodeScru := LISTES->l_codescru
  nNbScrutin := 0
  nListe := 0
  DO WHILE !EOF() .AND. llReturn
    nListe := nListe + 1
    nNbScrutin := nNbScrutin +1
    lnTotCarteUrne = 0
    lnLgBuf := (LISTES->l_nbreffec+LISTES->l_nbrsuple + 4)*lnLg1Vote // A.C. 14/03/1995
    lcBuffer := SPACE(lnLgBuf)
    _AffMsg(24,1, ' '+ _RechMsg('JT_RECUP028') + ' '+LISTES->l_nom, AM_ALIGN_GA, AM_EFF_CENTRE) // ' liste '

    * lire les r‚sultats d'une liste
    IF FREAD(lnHandle,@lcBuffer,lnLgBuf) <> lnLgBuf 
       * erreur lecture
       _Message('', _TRUE, _RechMsg('JT_RECUP022')+' '+LISTES->l_codescru+' '+LISTES->l_nom) // Erreur lecture liste
       llReturn := _FALSE
    ELSE
       IF FILE("c:\jites\temp\table.urn")

             bErreurRecup = _FALSE
             FOR I=0 to lnLgBuf step lnLg1Vote
                lnTotChiffre = lnTotChiffre + VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                IF I = 0
                    lnTotListe = VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                    lnTotCarteUrne = lnTotCarteUrne + lnTotListe
                ELSEIF I = 4
                    lnTotSup = VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                ELSEIF I = 8
                    lnTotTL = VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                ELSEIF I = 12
                    lnTotEff = VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                ELSE
                    IF lnTotListe-lnTotTL < VAL(SUBSTR(lcBuffer,I+1,lnLg1Vote))
                      * Erreur dans les voix de pr‚f‚rence
                      bErreurRecup = _TRUE
                      lsErreur = 'CE:'+STRZERO(lnTotListe,6) + " - 'TL:" + STRZERO(lnTotTL,6) + " < CAND:";
                                 + SUBSTR(lcBuffer,I+1,lnLg1Vote)
                      @ 16,22 SAY lsErreur
                      @ 17,22 SAY nNbScrutin
                      @ 18,22 SAY LISTES->l_nom
                      I = lnLgBuf
                    ENDIF
                ENDIF
             NEXT I
             IF lnTotListe < lnTotSup + lnTotTL + lnTotEff
                  * Erreur dans les sous cat‚gories
                  bErreurRecup = _TRUE
                  lsErreur = 'CE:'+STRZERO(LISTES->l_nbr_v_ex,6) + " < 'SUP:" + STRZERO(LISTES->l_nbr_v_tl,6) + " + TL:";
                        +STRZERO(LISTES->l_nbr_v_ef,6) + " + EFF:" + STRZERO(LISTES->l_nbr_v_sp,6)
                  @ 16,22 SAY lsErreur
             ENDIF

             IF bErreurRecup
                  _Message('JT_RECUP196', _TRUE) // ProblŠme dans le r‚sultats - Erreur 27 - Niveau1
                  Return( _FALSE )
             ENDIF
             DBSELECTAREA('RECUPVOTES')

             * traitement du nbr de votes exprim‚s pour la liste
             * MAJ du fichier RECUPVOT
             of_addrec(0)
             REPLACE rv_numbur  WITH BURVOTES->b_numero
             REPLACE rv_codescr WITH LISTES->l_codescru
             REPLACE rv_college WITH LISTES->l_college
             REPLACE rv_numlist WITH LISTES->l_numero
             REPLACE rv_numcand WITH "000"
             REPLACE rv_eff_sup WITH "A"
             lcNbrVote =  LEFT(lcBuffer,lnLg1Vote)+SPACE(16)
             if Cryp_Resul  = _TRUE
               REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"000"+lcCrypt)
             else
               REPLACE rv_nbrvote WITH lcNbrVote
             endif  
             UNLOCK

             * traitement du nbr de votes exprim‚s pour les suppl‚ants seuls
             * MAJ du fichier RECUPVOT
             of_addrec(0)
             REPLACE rv_numbur  WITH BURVOTES->b_numero
             REPLACE rv_codescr WITH LISTES->l_codescru
             REPLACE rv_college WITH LISTES->l_college
             REPLACE rv_numlist WITH LISTES->l_numero
             REPLACE rv_numcand WITH "000"
             REPLACE rv_eff_sup WITH "S"
             lcNbrVote =  SUBSTR(lcBuffer,(lnLg1Vote)+1,lnLg1Vote)+SPACE(16)
             if Cryp_Resul  = _TRUE
               REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"000"+lcCrypt)
             else
               REPLACE rv_nbrvote WITH lcNbrVote
             endif  
              UNLOCK

             * traitement du candidat tˆte de liste
             * MAJ du fichier RECUPVOT
             of_addrec(0)
             REPLACE rv_numbur  WITH BURVOTES->b_numero
             REPLACE rv_codescr WITH LISTES->l_codescru
             REPLACE rv_college WITH LISTES->l_college
             REPLACE rv_numlist WITH LISTES->l_numero
             REPLACE rv_numcand WITH "000"
             REPLACE rv_eff_sup WITH "E"
             lcNbrVote =  SUBSTR(lcBuffer,(2*lnLg1Vote)+1,lnLg1Vote)+SPACE(16)
             if Cryp_Resul  = _TRUE
               REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"000"+lcCrypt)
             else
               REPLACE rv_nbrvote WITH lcNbrVote
             endif  
             UNLOCK

             * traitement du nbr de votes exprim‚s pour les effectifs seuls
             * MAJ du fichier RECUPVOT
             of_addrec(0)
             REPLACE rv_numbur  WITH BURVOTES->b_numero
             REPLACE rv_codescr WITH LISTES->l_codescru
             REPLACE rv_college WITH LISTES->l_college
             REPLACE rv_numlist WITH LISTES->l_numero
             REPLACE rv_numcand WITH "000"
             REPLACE rv_eff_sup WITH "F"
             lcNbrVote =  SUBSTR(lcBuffer,(3*lnLg1Vote)+1,lnLg1Vote)+SPACE(16)
             if Cryp_Resul  = _TRUE
               REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"000"+lcCrypt)
             else
               REPLACE rv_nbrvote WITH lcNbrVote
             endif  
 
             UNLOCK

             * traitement des effectifs
             lnI   := 1
             lnInd := (4*lnLg1Vote)+1   // 3-->4 A.C. 14/03/1995
             DO WHILE lnInd < (LISTES->l_nbreffec*lnLg1Vote)+(4*lnLg1Vote) // 3-->4 A.C. 14/03/1995
                * MAJ du fichier RECUPVOT
                of_addrec(0)
                REPLACE rv_numbur  WITH BURVOTES->b_numero
                REPLACE rv_codescr WITH LISTES->l_codescru
                REPLACE rv_college WITH LISTES->l_college
                REPLACE rv_numlist WITH LISTES->l_numero
                REPLACE rv_numcand WITH STRZERO(lnI,3)
                REPLACE rv_eff_sup WITH _CODE_EFFECTIF
                lcNbrVote := SUBSTR(lcBuffer,lnInd,lnLg1Vote)+SPACE(16)
                if Cryp_Resul  = _TRUE
                  REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,STRZERO(lnI,3)+lcCrypt)
                else
                  REPLACE rv_nbrvote WITH lcNbrVote
                endif  
 
                UNLOCK

                lnInd += lnLg1Vote
                lnI++

             ENDDO

             * traitement des suppl‚ants
             lnI := 1
             DO WHILE lnInd < lnLgBuf
                * MAJ du fichier RECUPVOT
                of_addrec(0)
                REPLACE rv_numbur  WITH BURVOTES->b_numero
                REPLACE rv_codescr WITH LISTES->l_codescru
                REPLACE rv_college WITH LISTES->l_college
                REPLACE rv_numlist WITH LISTES->l_numero
                REPLACE rv_numcand WITH STRZERO(lnI,3)
                REPLACE rv_eff_sup WITH _CODE_SUPPLEANT
                lcNbrVote := SUBSTR(lcBuffer,lnInd,lnLg1Vote)+SPACE(16)
                if Cryp_Resul  = _TRUE
                  REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,STRZERO(lnI,3)+lcCrypt)
                else
                  REPLACE rv_nbrvote WITH lcNbrVote
                endif  
 
                UNLOCK

                lnInd += lnLg1Vote
                lnI++
             ENDDO
       ELSE
             llReturn := _FALSE
             pnNbVotant := 0
       ENDIF
    ENDIF

    DBSELECTAREA('LISTES')
    SKIP

    *
    * changement de scrutin
    *
    IF llReturn .AND. (LISTES->l_CodeScru<>lcCodeScru .OR. EOF())
       * lecture des votes blancs
       lcBuffer := SPACE(lnLg1Vote*4)
       IF FREAD(lnHandle,@lcBuffer,lnLg1Vote*4) <> lnLg1Vote*4 
          * erreur lecture
          _Message('', _TRUE, _RechMsg('JT_RECUP023')+' '+LISTES->l_codescru) // Erreur lecture nombre de votes blanc du scrutin
          llReturn := _FALSE
       ELSE
          IF FILE("c:\jites\temp\table.urn")

                DBSELECTAR('RECUPVOTES')
                * MAJ du fichier RECUPVOT (VOTE BLANC ELECTEUR)
                of_addrec(0)
                REPLACE rv_numbur  WITH BURVOTES->b_numero
                REPLACE rv_codescr WITH lcCodeScru
                REPLACE rv_college WITH "N"
                REPLACE rv_numlist WITH "XX"
                REPLACE rv_numcand WITH "001"
                REPLACE rv_eff_sup WITH _CODE_EFFECTIF
                lcNbrVote =  SUBSTR(lcBuffer,(2*lnLg1Vote)+1,lnLg1Vote)+SPACE(16)
                if Cryp_Resul  = _TRUE
                  REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"001"+lcCrypt)
                else
                  REPLACE rv_nbrvote WITH lcNbrVote
                endif  

                UNLOCK


                lnTotCarteUrne = lnTotCarteUrne + VAL(SUBSTR(lcBuffer,(2*lnLg1Vote)+1,lnLg1Vote))

                * MAJ du fichier RECUPVOT (VOTE BLANC pas vot‚)
                of_addrec(0)
                REPLACE rv_numbur  WITH BURVOTES->b_numero
                REPLACE rv_codescr WITH lcCodeScru
                REPLACE rv_college WITH "N"
                REPLACE rv_numlist WITH "XX"
                REPLACE rv_numcand WITH "002"
                REPLACE rv_eff_sup WITH _CODE_EFFECTIF
                lcNbrVote =  RIGHT(lcBuffer,lnLg1Vote)+SPACE(16)
                if Cryp_Resul  = _TRUE
                  REPLACE rv_nbrvote WITH CRYPT(lcNbrVote,"002"+lcCrypt)
                else
                  REPLACE rv_nbrvote WITH lcNbrVote
                endif  

                UNLOCK
                lnTotCarteUrne = lnTotCarteUrne + VAL(RIGHT(lcBuffer,lnLg1Vote))

                DBSELECTAR('LISTES')
          ELSE
                llReturn := _FALSE
                pnNbVotant := 0
          ENDIF
          lcCodeScru := LISTES->l_codescru
       ENDIF
    ENDIF
    AADD(laTotCarteParScrutin , lnTotCarteUrne)
  ENDDO     && boucle LISTES

  IF llReturn

      lcBuffer := SPACE(lnLg1Vote*7+2)
      IF FREAD(lnHandle,@lcBuffer,lnLg1Vote*7+2) <> lnLg1Vote*7+2

        * erreur lecture
        pnNbVotant := 0
        pnNbVotn   := 0
        pnNbVote   := 0
        pnNbVots   := 0

        nTotFichier:= 0
        _Message('JT_RECUP016', _TRUE) // Erreur lecture du nombre TOTAL de votes exprim‚es
        llReturn := _FALSE
       ELSE
        IF FILE("c:\jites\temp\table.urn")
           sCleElection= RIGHT(lcbuffer,6)

           nTotFichier= VAL(substr(lcbuffer,(4*lnLg1Vote)+1,lnLg1Vote*2))
           pnNbVotant = VAL(substr(lcbuffer,(3*lnLg1Vote)+1,lnLg1Vote))
           pnNbVots   = VAL(substr(lcbuffer,(2*lnLg1Vote)+1,lnLg1Vote))
           pnNbVote   = VAL(substr(lcbuffer,(1*lnLg1Vote)+1,lnLg1Vote))
           pnNbVotn   = VAL(substr(lcbuffer,1,lnLg1Vote))
        ELSE
           llReturn := _FALSE
           pnNbVotant := 0
           pnNbVotn   := 0
           pnNbVote   := 0
           pnNbVots   := 0
           nTotFichier:= 0
        ENDIF
       ENDIF

  ENDIF

  IF pnNbVotant == 0
      _Message('JT_RECUP200', _TRUE) // ProblŠme lecture des r‚sultats, ESSAYEZ A NOUVEAU
      llReturn := _FALSE
  ELSE
    IF pnNbVotant <> pnNbVotn + pnNbVote+pnNbVots
          _Message('JT_RECUP197', _TRUE) // ProblŠme dans le r‚sultats - Erreur 27 - Niveau2
          Return( _FALSE )
    ENDIF
    IF lnTotChiffre <> nTotFichier
          _Message('JT_RECUP198', _TRUE) // ProblŠme dans le r‚sultats - Erreur 27 - Niveau3
          Return( _FALSE )
    ENDIF
    FOR I = 1 to nNbScrutin step 1
       IF pnNbVotant < laTotCarteParScrutin[I]
          _Message('JT_RECUP199', _TRUE) // Nb carte dans urne < nbr de voix pour un scrutin
          Return( _FALSE )
       ENDIF
    NEXT I
* << 2010 à faire
*  2701 -> 2703
*  (1,4,6,7) -> (1,3,6,8)
* 2010 >>
*   sClePcTot = SUBSTR(mpBV,1,1)+SUBSTR(mpBV,4,1)+SUBSTR(mpBV,6,1)+SUBSTR(mpBV,7,1)
    sClePcTot = SUBSTR(mpBV,1,1)+SUBSTR(mpBV,3,1)+SUBSTR(mpBV,6,1)+SUBSTR(mpBV,8,1)

        IF VAL(Alltrim(sCleElection)) <> VAL(PassWordElect) + VAL(sClePcTot)
          _Message('JT_RECUP204', _TRUE) // La cle interne de l'‚lection est incorrecte
          Return( _FALSE )
    ENDIF
  ENDIF

  RESTSCREEN(10,00,16,79,lcWinbuff)

RETURN(llReturn)

*************************************************
* d‚charge
*************************************************
FUNCTION Decharge
  PARAMETER pn_nbrvotn,pn_nbrvote,pn_nbrvots,;
            pn_nbrnuln,pn_nbrnule,pn_nbrnuls,pc_time,pl_impdech,plImp
  LOCAL lcCodeDesScrut:="",lnOldSelect:=SELECT(),lcCodeTxt:="",;
        lcOldLangue:=ccLangue,lcLangImp,lnRecBur
  PRIVATE lc_ecran,lc_oldcoul,lc_rep,ln_i,lnBurDeb,lnBurFin
  PRIVATE taVarImpr[30]
  gbCEE :=_FALSE
  gbBEL :=_FALSE
  plImp := IIF(plImp==NIL,_TRUE,plImp)

  @ 23,01 CLEAR TO 24,78

  lc_ecran=SAVESCREEN(8,01,23,78)

  STORE SETCOLOR() TO lc_oldcoul

  SETCOLOR(_COLOR_ATT)

  @ 8, 01, 22, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
  @ 9,5,22,75 BOX B_DOUBLE + ' '
  @ 9,7  SAY _RechMsg('JT_RECUP002') // ¶  R E C E P I S S E  Ç
  @ 10,7 SAY _RechMsg('JT_RECUP003') + " " + DTOC(PARAM->p_dateelec) // Election du
  @ 11,7 SAY _RechMsg('JT_RECUP004') + " " + BURVOTES->b_presiden // Pr‚sident de bureau
  @ 12,7 SAY _RechMsg('JT_RECUP005')   // Nombre de votes enregistr‚s
* << 2011
*  @ 13,27 SAY _RechMsg('JT_RECUP300')   // -pour ‚lecteurs belges
*  @ 13,70 SAY pn_nbrvotn PICT "9999"
*  @ 14,27 SAY _RechMsg('JT_RECUP301')   // -pour ‚lecteurs de l'union europ.
*  @ 14,70 SAY pn_nbrvote PICT "9999"
*  IF ccOperElec = _TYPE_OPER_4ELEC
*      @ 15,27 SAY _RechMsg('JT_RECUP302')   // -pour ‚lecteurs belges a l'etranger
*    @ 15,70 SAY pn_nbrvots PICT "9999"
*  ENDIF
  @ 12,50 SAY pn_nbrvotn PICT "9999"
* 2011 >>
  IF .NOT. pl_impdech
    SET CURSOR ON
    @ 16,7 SAY _RechMsg('JT_RECUP006') // Encodez le nombre de bulletins nuls :
* << 2011
*    @ 17,27 SAY _RechMsg('JT_RECUP300') // - pour ‚lecteurs belges
*    @ 18,27 SAY _RechMsg('JT_RECUP301') // - pour ‚lecteurs de l'union europ.
*
*    IF ccOperElec = _TYPE_OPER_4ELEC
*        @ 19,27 SAY _RechMsg('JT_RECUP302') // - pour ‚lecteurs belges a l'etranger
*    ENDIF
* 2011 >>>

    STORE SPACE(1) TO lc_rep
    DO WHILE !(lc_rep$"OJ")
* << 2011
*     @ 17,70 GET pn_nbrnuln PICT "9999" VALID ValNbrNul(pn_nbrnuln,pn_nbrvotn)
*     @ 18,70 GET pn_nbrnule PICT "9999" VALID ValNbrNul(pn_nbrnule,pn_nbrvote)
*     IF ccOperElec = _TYPE_OPER_4ELEC
*         @ 19,70 GET pn_nbrnuls PICT "9999" VALID ValNbrNul(pn_nbrnuls,pn_nbrvots)
*     ELSE
*         pn_nbrnuls := 0
*     ENDIF
    @ 16,50 GET pn_nbrnuln PICT "9999" VALID ValNbrNul(pn_nbrnuln,pn_nbrvotn)
    pn_nbrnuls := 0
    pn_nbrnule := 0
* 2011 >>

      READ
      IF LASTKEY() # K_ESC
        STORE SPACE(1) TO lc_rep
        @ 20,25 SAY _RechMsg("JT_RECUP033") GET lc_rep PICT "!";
                 VALID(AT(lc_rep,"JON")<>0)
        READ
      ENDIF
    ENDDO
    SET CURSOR OFF
  ELSE
    @ 16,7  SAY _RechMsg('JT_RECUP006') // Encodez le nombre de bulletins nuls :
* << 2011
*    @ 17,27 SAY _RechMsg('JT_RECUP300') // - pour les ‚lecteurs belges
*    @ 17,70 SAY pn_nbrnuln PICT "9999"
*    @ 18,27 SAY _RechMsg('JT_RECUP301') // -pour ‚lecteurs de l'union europ
*    @ 18,70 SAY pn_nbrnule PICT "9999"
*    IF ccOperElec = _TYPE_OPER_4ELEC
*       @ 19,27 SAY _RechMsg('JT_RECUP302') // - pour les ‚lecteurs belges a l'etranger
*       @ 19,70 SAY pn_nbrnuls PICT "9999"
*    ENDIF
     @ 16,50 SAY pn_nbrnuln PICT "9999"
* 2011 >>
  ENDIF

  IF plImp
     @ 21,25 SAY SPACE(40)
     @ 21,12 SAY _rechmsg("JT_RECUP114")
     SET CURSOR ON
     @ 21,COL()+1 GET gnNbrDecharge PICT "9"
     READ
     SET CURSOR OFF
     SETCOLOR(_COLOR_MENU)

     IF LASTKEY() # K_ESC .AND. gnNbrDecharge # 0
        lcLangImp := LangImp()   // S‚lectionner la langue utilis‚e pour l'impression
     ENDIF

     _affmsg(23,0,_RechMsg('JT_RECUP007'), AM_ALIGN_CE) // Impression de la d‚charge

     * recherche de Tous les codes scrutins pour cette ‚lection
      SELECT SCRUTINS
      DBGOTOP()
      DO WHILE !EOF()
        IF (!EMPTY(s_nomf))
          lcCodeDesScrut += s_code
        ENDIF
        DBSKIP()
      ENDDO
      SELECT(lnOldSelect)

     * recherche le num‚ro du 1er et du dernier bureau
      lnBurDeb := gnBurMin
      lnBurFin := gnBurMax
      IF LASTKEY() # 27 .AND. gnNbrDecharge # 0 .AND. testimp()
          SET CENTURY ON
          ccLangue := lcLangImp
          lcCodeTxt := "JT_RECUPDE2"
          taVarImpr[01] := 'PARAM->p_arron'
          taVarImpr[02] := 'PARAM->p_burdep'
          taVarImpr[03] := 'DTOC(PARAM->p_dateelec)'
          taVarImpr[04] := 'STR(PARAM->p_nbrbur,3)'
          taVarImpr[05] := 'STR(lnBurDeb,3)'
          taVarImpr[06] := 'PADC(trim(BURVOTES->b_presiden),LEN(BURVOTES->b_presiden))'
          taVarImpr[07] := 'STR(BURVOTES->b_numero)'
          taVarImpr[09] := 'pc_time'
          taVarImpr[10] := 'STR(pn_nbrvotn,7)'
          taVarImpr[11] := 'STR(pn_nbrnuln+pn_nbrvotn,7)'
          taVarImpr[12] := 'PADC(TRIM(PARAM->p_reponsab),LEN(PARAM->p_reponsab))'
          taVarImpr[13] := "PADC(trim(IIF(ccNumArr=='002' .AND. ccLangue=='N', RechCantN(),PARAM->p_burdep)),LEN(PARAM->p_burdep))"
          taVarImpr[14] := "'"+_IMPR_12CPI+"'"
          taVarImpr[15] := "'"+_IMPR_10CPI+"'"
          taVarImpr[16] := "STR(BURVOTES->b_nopctot,3)"
          taVarImpr[17] := "TRIM(IIF(ccNumArr=='002' .AND. ccLangue=='N', RechCantN(),PARAM->p_burdep))"
          taVarImpr[18] := 'STR(lnBurFin,3)'
          IF jt_msg->(DBSEEK(lcCodeTxt))
            SET DEVICE TO PRINT
            @ PROW(), PCOL() SAY _IMPR_INIT
            lmMsgEcran := jt_Msg->msg_ecran&ccLangue
            DECLARE taMemo[1] // MLCOUNT(lmMsgEcran, 80)]
            ImpMemo(@taMemo, 80, @lmMsgEcran , '1', @taVarImpr, _NB_LIG_PAGE, _TRUE )
            taLgImp := {}
            ChgTab('{001}')
            FOR ln_i = 1 TO gnNbrDecharge
              ImpMemo(@taLgImp, 80, @lmMsgEcran , '2', @taVarImpr, _NB_LIG_PAGE, _TRUE )
            NEXT ln_i
          ENDIF
          SET CENTURY ON
     ENDIF
  ENDIF

  SETCOLOR(lc_oldcoul)
  SELECT(lnOldSelect)
  RESTSCREEN(8,01,23,78,lc_ecran)

  ccLangue := lcOldLangue   // Restaurer le code langue
RETURN(NIL)


FUNCTION InsDisk
  PRIVATE lnReturn:=0,lcOldCoul:=SETCOLOR(_COLOR_MENU)

  _AffMsg(23,01, _RechMsg('JT_RECUP024')+' '+ gcDisk +' '+ _RechMsg('JT_RECUP025'),;
          AM_ALIGN_CE, AM_EFF_CENTRE) // Ins‚rez la disquette "+gcDisk+" dans le lecteur
  _AffMsg(24,01, _RechMsg('JT_RECUP026'), AM_ALIGN_GA, AM_EFF_CENTRE) // Appuyez sur une touche pour continuer
  _AffMsg(24,01, _RechMsg('JT_RECUP027'), AM_ALIGN_DR) // [ESC] Quitter

  SET CONSOLE OFF
  WAIT
  SET CONSOLE ON
  lnReturn := LASTKEY()
  SETCOLOR(lcOldCoul)
RETURN(lnReturn)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            Menu de totalisation                   ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION TotMenu
  LOCAL lnNbrBNul
  PRIVATE lnNbLu:=0, lnChoix := 0, lcChoixPermis := "", lcOldScreen, lcOldCoul
  PRIVATE lnNbLgBur:=0, lnNbTot:=0
  PRIVATE nNbPcTot := 0 , lTraite
  PRIVATE gnNbrDecharge:=2

   IF FILE("C:\Jites\PC.ind")
      lnHandle := FOPEN("C:\jites\PC.ind")
      lcBuffer := SPACE(1)
      IF FREAD(lnHandle,@lcBuffer,1) == 1
        lsBurMin :=SPACE(3)
        FREAD(lnHandle,@lsBurMin,3)
        gnBurMin := VAL(lsBurMin)
        lsBurMax :=SPACE(3)  
        FREAD(lnHandle,@lsBurMax,3)
        gnBurMax := VAL(lsBurMax)
      ELSE
        MsgGenDsk("JT_UTIL236") // ProblŠme lecture du fichier PC.ind
      ENDIF
      FCLOSE(lnHandle)
   ELSE
     MsgGenDsk("JT_UTIL235") // ProblŠme ouverture du fichier PC.ind
   ENDIF

   _use( "jtcanton" ,_PATH_JITES + '\' , _SELECT_CANTONS        , _EXCLUSIVE, 'LISTE_CANT')
  LOCATE FOR can_typrec=='CANT' .AND. can_code == PARAM->p_numcan .AND. CAN_ARROND == PARAM->p_numarron
  nNbPcTot := IIF( CAN_nburto==0, 1, CAN_nburto)

  EcrBase()

  LigStatut() // Ligne 01 STATUT

  IF FILE( _PATH_JITES + '\TOTALISA.INT' )
    FILEDELETE( _PATH_JITES+'\TOTALISA.INT' )
    jtTotPc( _FALSE )
  ENDIF

  cNumPc := PARAM->p_NoPc
  IF LEN(ALLTRIM( PARAM->p_NoPclut )) >= 1 .AND. nNbPcTot > 1  .AND. cNumPc  $ PARAM->p_NoPclut
    * PC de totalisation (multi PC)
    ToAffPc( nNbPcTot, @lnNbLu,@lnNbLgBur,@lnNbTot )
  ELSE
    * PC de totalisation des bureaux
    ToAffMenu(@lnNbLu,@lnNbLgBur,@lnNbTot)
  ENDIF

  lcChoixPermis := ToPtsMenu(PARAM->p_nbrbur, lnNbLu)
  DO WHILE lnChoix # K_ESC
    lnChoix := 0
    DO WHILE lnChoix # K_ESC .AND. lnChoix # K_F10 .AND. AT(UPPER(CHR(lnChoix)) , lcChoixPermis) == 0
      lnChoix := INKEY(0)
      IF lnChoix == K_ESC
         lnChoix := 0
      ENDIF
    ENDDO
    lcOldScreen := SAVESCREEN(0,0,24,79)
    DO CASE
      CASE lnChoix == K_F10  // CHOIX IMPRIMANTE
        Public MCHOIX,CScreen
        Declare ChoixIMP:={}
        MChoix:=0
        CScreen:=SAVESCREEN(02,00,21,80)
        @15,29 clear to 21,50
        IF ccLangue = _NEERLANDAIS
           @16,30 SAY "Druckmodus     "
           AADD(ChoixIMP,"FX Modus")
           AADD(ChoixIMP,"HP Modus")
        ELSE
           @16,30 SAY "Mode d'impression"
           AADD(ChoixIMP," Mode FX ")
           AADD(ChoixIMP," Mode HP ")
        ENDIF
        MenuBase(17,35,ChoixIMP,1,2,2)
        IF MCHOIX # 0
          IF MCHOIX == 2
                _IMPR_10CPI := D_IMPR_10CPI
                _IMPR_12CPI := D_IMPR_12CPI
                _IMPR_INIT := D_IMPR_INIT
                _IMPR_TYPE := D_IMPR_TYPE
          ELSEIF MCHOIX == 1
                _IMPR_10CPI := J_IMPR_10CPI
                _IMPR_12CPI := J_IMPR_12CPI
                _IMPR_INIT := J_IMPR_INIT
                _IMPR_TYPE := J_IMPR_TYPE
          ENDIF
          IF  ccLangue  == _NEERLANDAIS
               _IMPR_TYPE :=  SUBSTR(_IMPR_TYPE,6,2) + ' MODUS' 
          ENDIF
          SETCOLOR(_COLOR_LIG1)
          @ 01,70 SAY _IMPR_TYPE
          SETCOLOR(_COLOR_MENU)
        ENDIF  
        RESTSCREEN(02,00,21,80,CScreen)

      CASE UPPER(CHR(lnChoix)) == 'L'  // LECTURE D'UNE DISQUETTE
        ToRecDisq()
        ToAffMenu(@lnNbLu,@lnNbLgBur,@lnNbTot)
        lcChoixPermis := ToPtsMenu(PARAM->p_nbrbur, lnNbLu)
        IF !('L' $ lcChoixPermis) .AND. nNbPcTot > 1
          ToAffPc( nNbPcTot, @lnNbLu,@lnNbLgBur,@lnNbTot )
          lcChoixPermis := ToPtsMenu(PARAM->p_nbrbur, lnNbLu)
        ENDIF
      CASE UPPER(CHR(lnChoix)) $ 'DV'  // IMPRESSION DECHARGE
        @ (lnNbLgBur*2)+5, 1, 22, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
        CorpsRecup()
        _AffMsg(23,01, _RechMsg('JT_RECUP078' + ' ' + _CARACT_CROCHETS ) , AM_ALIGN_CE,;
               AM_EFF_CENTRE, _COLOR_MENU) // 'S‚lectionnez le bureau dont vous d‚sirez imprimer la d‚charge'
        _AffMsg(24,01, _RechMsg('JT_RECUP113') , AM_ALIGN_CE,;
                AM_EFF_CENTRE, _COLOR_MENU) // 'Modifier le nombre de bulletins nuls'
        DBSELECTAREA('BURVOTES')
        GO TOP
        Liste(11, 04, 19, 73, 'AffRecup',,,,,"[27][13][77][87][109][119]")
        IF LASTKEY() # K_ESC
          IF AT(_STAT_BUR_LECTURE, BURVOTES->b_statut) # 0
            IF LASTKEY() == K_RETURN
               lcOldCoul := SETCOLOR(_COLOR_MENU)
               Decharge(BURVOTES->b_nbr_votn,BURVOTES->b_nbr_vote,BURVOTES->b_nbr_vots,;
                        BURVOTES->b_nbr_nuln,BURVOTES->b_nbr_nule,BURVOTES->b_nbr_nuls,;
                        SUBSTR(BURVOTES->b_statut,29,8) ,_TRUE)
               SETCOLOR(lcOldCoul)
            ELSE   && M,m,W,w
               * modifier le nombre de bulletins nuls
               lcOldCoul := SETCOLOR(_COLOR_MENU)
               lnNbrNuln := BURVOTES->b_nbr_nuln
               lnNbrNule := BURVOTES->b_nbr_nule
               lnNbrNuls := BURVOTES->b_nbr_nuls
               Decharge(BURVOTES->b_nbr_votn,BURVOTES->b_nbr_vote,BURVOTES->b_nbr_vots,;
                        @lnNbrNuln,@lnNbrNule,@lnNbrNuls,;
                        SUBSTR(BURVOTES->b_statut,29,8) ,_FALSE,_FALSE)
               IF LASTKEY() # K_ESC
                  of_reclock(0)
                  REPLACE b_nbr_nuln WITH lnNbrNuln
                  REPLACE b_nbr_nule WITH lnNbrNule
                  REPLACE b_nbr_nuls WITH lnNbrNuls
                  UNLOCK
                  COMMIT

               ENDIF
               SETCOLOR(lcOldCoul)
            ENDIF
          ELSE
             _AffMsg(23,01, _RechMsg('JT_RECUP110') , AM_ALIGN_CE,;
                 AM_EFF_OUI   , _COLOR_MENU) // Impression impossible, le bureau n' a pas encore ‚t‚ r‚cup‚r‚.
             _AffMsg(24,01, _RechMsg('JT_RECUP026'), AM_ALIGN_CE, AM_EFF_OUI) // Appuyez sur une touche pour continuer
             TONE(700,1)
             TONE(800,1)
             TONE(1000,2)
             SET CONSOLE OFF
             WAIT
             SET CONSOLE ON
          ENDIF
        ENDIF
      CASE UPPER(CHR(lnChoix)) == 'I'  // IMPRESSION INTERMEDIAIRE
        Public xxLangue
        TypeResul := 1
        lcLangImp := LangImp()
        ImpResPar()
        ccLangue:=xxLangue
        TypeResul := 0
        Password := PassWord2
      CASE UPPER(CHR(lnChoix)) $ 'CS'  // CLOTURE / SLUITING
        lcOldCoul := SETCOLOR(_COLOR_MENU)
        IF _GetRep('JT_RECUP072') == _REPONSE_OUI
          lnChoix := K_ESC // Sortie du programme de totalisation
          DBSELECTAREA('PARAM')
          of_reclock(0)
          REPLACE p_clodepou WITH _TRUE
          UNLOCK
          COMMIT

        ENDIF
        SETCOLOR(lcOldCoul)
      CASE UPPER(CHR(lnChoix)) == 'R'  // Impression des r‚sultats
        TypeResul := 2
        jtverif()
        lcOldCoul := SETCOLOR(_COLOR_MENU)
        IF pcCommande == _MENU_RESULTAT
             jtResul()
        ENDIF
        SETCOLOR(lcOldCoul)
        TypeResul := 0
        Password := PassWord2


      CASE UPPER(CHR(lnChoix)) == 'E'  // R(E)cup‚ration r‚sultat
        lcOldCoul := SETCOLOR(_COLOR_MENU)
        RecResPc()
        SETCOLOR(lcOldCoul)

        ToAffPc( nNbPcTot, @lnNbLu,@lnNbLgBur,@lnNbTot )
        lcChoixPermis := ToPtsMenu(PARAM->p_nbrbur, lnNbLu)
      CASE UPPER(CHR(lnChoix)) == 'T'  // (T)ransfert r‚sultat
        lcOldCoul := SETCOLOR(_COLOR_MENU)
        TransRes()
        SETCOLOR(lcOldCoul)
      CASE UPPER(CHR(lnChoix)) == 'B'  // (B)ackup
        lcOldCoul := SETCOLOR(_COLOR_MENU)
        IF ( lnRetMsg := MsgGenDsk('JT_GENDK031')) # K_ESC // Ins‚rez une disquette FORMATEE dans le lecteur
          DO WHILE .NOT. DISKREADY(ccDriveActif) .AND. lnRetMsg # K_ESC
             lnRetMsg := MsgGenDsk('JT_GENDK031') // Ins‚rez une disquette FORMATEE dans le lecteur
          ENDDO
          IF lnRetMsg # K_ESC
            DO WHILE .NOT. DISKREADYW(ccDriveActif) .AND. lnRetMsg # K_ESC
               lnRetMsg := MsgGenDsk( 'JT_GENDK028') //  Erreur: disquette prot‚g‚e en ‚criture
            ENDDO
          ENDIF
          IF lnRetMsg # K_ESC
            IF  FILE("a:backup.bat")
              Cscreen:=SAVESCREEN(0,0,24,79)
              lcRun := "a:backup  c:\jites\*.* a: /s "
              RUN &lcRun
              RESTSCREEN(0,0,24,79,CScreen)
            ELSE
               * No backup floppy
               lnRetMsg := MsgGenDsk( 'JT_ELECT012') //  Bad floppy
            ENDIF
          ENDIF
          SETCOLOR(lcOldCoul)
        ENDIF
     ENDCASE
    IF UPPER(CHR(lnChoix)) <> 'L' .AND. UPPER(CHR(lnChoix)) <> 'E' .AND. lnChoix <> K_F10
       RESTSCREEN(0,0,24,79,lcOldScreen)
    ENDIF
  ENDDO

RETURN(NIL)
/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            Affiche le menu de totalisation        ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ToAffMenu
  PARAMETERS pnNbLu,;      && nbr de bureaux lus
             pnNbLgBur,;   && nbr de lignes occup‚es par les bureaux
             pnNbTot       && nbr de bureaux totalis‚s
  LOCAL lnInd:=1
  PRIVATE lnNoLig, lnNoCol, lnOldSelect:=SELECT(), lnNbFoi

  DBSELECTAREA('PARAM')
  SETCOLOR(_COLOR_MENU)
  pnNbLgBur := ROUND((burvotes->(RECCOUNT()) / 15)+.49,0)
  @ 2, 1 , (pnNbLgBur*2)+3, 78 BOX B_SINGLE + ' '
  @ 2, 3 say _RechMsg( 'JT_RECUP079') // 'Bureaux'

  pnNbLu := 0
  pnNbTot := 0
  DBSELECTAREA('BURVOTES')
  GO TOP
  DO WHILE .NOT. eof()
    lnNbFoi := ROUND( (lnInd /15)+.49,0)
    lnNoLig := 2 + (lnNbFoi * 2)
    lnNoCol := 1 + ((lnInd - ((lnNbFoi-1) * 15)) * 4)
    @ lnNoLig-1,lnNoCol SAY b_numero PICTURE '999' COLOR _COLOR_MENU
    IF left(b_statut,7) == _STAT_BUR_LECTURE
      @ lnNoLig, lnNoCol+1 SAY 'OK'
      pnNbLu++
      pnNbTot += IIF(b_numtot<>0,1,0)
    ELSE
      @ lnNoLig, lnNoCol+1 SAY '  '
    ENDIF
    DBSKIP()
    lnInd++
  ENDDO

  @ (pnNbLgBur*2)+5, 10 say _RechMsg('JT_RECUP080') +' ' + TRANS(PARAM->p_nbrbur,'999') // 'Nombre total de bureaux     : '
  @ (pnNbLgBur*2)+7, 10 say _RechMsg('JT_RECUP081') +' ' + TRANS(pnNbLu         ,'999') // 'Nombre de bureaux r‚colt‚s  : '
  @ (pnNbLgBur*2)+9, 10 say _RechMsg('JT_RECUP082') +' ' + TRANS(pnNbTot        ,'999') // 'Nombre de bureaux totalis‚s : '

  @ 23,01 SAY SPACE(78) COLOR _COLOR_MENU
  @ 24,01 SAY SPACE(78) COLOR _COLOR_MENU
  SELECT(lnOldSelect)
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            Affiche les Pc  de totalisation        ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ToAffPc
  PARAMETERS pnNbPcTot, ;
             pnNbLu,;      && nbr de bureaux lus
             pnNbLgBur,;   && nbr de lignes occup‚es par les bureaux
             pnNbTot       && nbr de bureaux totalis‚s

  PRIVATE lnNoLig, lnNoCol, lnOldSelect:=SELECT(), lnNbFoi, lnI

  EcrBase()

  LigStatut() // Ligne 01 STATUT

  DBSELECTAREA('PARAM')
  SETCOLOR(_COLOR_MENU)
  @ 2, 1 , 5, 78 BOX B_SINGLE + ' '
  @ 2, 3 say _RechMsg( 'JT_RECUP083' ) // 'Pc Totalisation'

  FOR lnI := 1 TO pnNbPcTot
    @ 3, (lnI*3)+4 SAY lnI PICTURE '99' COLOR _COLOR_MENU
    IF STR(lnI,1) $ ALLTRIM(PARAM->p_noPcLut)
      @ 4, (lnI*3)+4 SAY 'OK' COLOR _COLOR_MENU
    ENDIF
  NEXT

  pnNbLu := 0
  pnNbTot := 0
  DBSELECTAREA('BURVOTES')
  GO TOP
  DO WHILE .NOT. eof()
    IF left(b_statut,7) == _STAT_BUR_LECTURE
      pnNbLu++
      pnNbTot += IIF(b_numtot<>0,1,0)
    ENDIF
    DBSKIP()
  ENDDO

  @ 23,01 SAY SPACE(78) COLOR _COLOR_MENU
  @ 24,01 SAY SPACE(78) COLOR _COLOR_MENU
  SELECT(lnOldSelect)
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            Point du menu de totalisation          ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ToPtsMenu
  PARAMETER pnNbTotBur, pnNbBurLu
  PRIVATE lcReturn:=""

  IF pnNbTotBur == pnNbBurLu
    IF !(PARAM->p_NoPc $ PARAM->p_NoPcLut)
      REPLACE  PARAM->p_NoPcLut WITH PARAM->p_NoPc
      DBCOMMITALL()
    ENDIF
  ENDIF

  @ 17, 1, 22, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
  IF pnNbTotBur # pnNbBurLu
    _BOX2(17, 3, 21, 19)
    @ 18, 4 say       _RechMsg( 'JT_RECUP084')      // "(L)ecture d'une"
    @ 19, 4 say PADC( _RechMsg( 'JT_RECUP085') ,15) // "   disquette   "
    lcReturn += "L"
  ELSE
    IF nNbPcTot # 1 .AND. LEN(ALLTRIM(PARAM->p_NoPcLut)) < nNbPcTot
      * backup
      _BOX2(11, 3, 15, 19)
      @ 13, 4 say PADC( _RechMsg( 'JT_RECUP203' ) ,15) // "   (B)ackup    "
      * cloture
      _BOX2(11,58, 15, 74)
      @ 12,59 say PADC( _RechMsg( 'JT_RECUP100') , 15 ) // " (C)l“ture de"
      @ 13,59 say PADC( _RechMsg( 'JT_RECUP101') , 15 ) // " totalisation"
      * r‚cup‚ration
      _BOX2(17, 3, 21, 19)
      @ 18, 4 say PADC( _RechMsg( 'JT_RECUP086' ) ,15) // "R(E)cup‚raion "
      @ 19, 4 say PADC( _RechMsg( 'JT_RECUP087' ) ,15) // "    des        "
      @ 20, 4 say PADC( _RechMsg( 'JT_RECUP088' ) ,15) // "  R‚sultats    "
      lcReturn += "EBCS"
    ELSE
      _BOX2(17, 3, 21, 19)
      @ 19, 4 say PADC( _RechMsg( 'JT_RECUP203' ) ,15) // "   (B)ackup    "
      lcReturn += "B"
    ENDIF
  ENDIF
  IF pnNbBurLu # 0
    _BOX2(17,22, 21, 35)
    @ 18,23 say PADC( _RechMsg( 'JT_RECUP089' ) ,12) // " (D)‚charge "
    @ 19,23 say PADC( _RechMsg( 'JT_RECUP090' ) ,12) // "     au     "
    @ 20,23 say PADC( _RechMsg( 'JT_RECUP091' ) ,12) // "  Pr‚sident "
    lcReturn += "DV"
  ENDIF
  IF (pnNbBurLu >= _TOT_PART_TOUT_LES;
    .AND. pnNbTotBur >= _TOT_PART_TOUT_LES+_TOT_PART_MIN_REST;
    .AND. pnNbBurLu # pnNbTotBur) .OR. (LEN(ALLTRIM(PARAM->p_NoPcLut)) > 0 .AND. LEN(ALLTRIM(PARAM->p_NoPcLut)) < nNbPcTot ) // A. C. Test .OR. .T.
    _BOX2(17,38, 21, 55)
    @ 18,39 say PADC( _RechMsg( 'JT_RECUP092' ) , 15 ) // "  (I)mpression"
    @ 19,39 say PADC( _RechMsg( 'JT_RECUP093' ) , 15 ) // "    r‚sultats "
    @ 20,39 say PADC( _RechMsg( 'JT_RECUP094' ) , 15 ) // " interm‚diaires"
    lcReturn += "I"
  ENDIF
  IF pnNbBurLu == pnNbTotBur .AND. LEN(ALLTRIM(PARAM->p_NoPcLut)) == nNbPcTot
    _BOX2(17,38, 21, 55)
    @ 18,39 say PADC( _RechMsg( 'JT_RECUP095' ) , 15 ) // "  Impression "
    @ 19,39 say PADC( _RechMsg( 'JT_RECUP096' ) , 15 ) // "     des     "
    @ 20,39 say PADC( _RechMsg( 'JT_RECUP097' ) , 15 ) // " (R)‚sultats "
    lcReturn += "R"
  ENDIF
  IF LEN(ALLTRIM(PARAM->p_NoPcLut)) == 1 .AND. pnNbBurLu == pnNbTotBur .AND. nNbPcTot > 1
    _BOX2(17,58, 21, 74)
    @ 18,59 say PADC( _RechMsg( 'JT_RECUP098') , 15 ) // " (T)ransfert "
    @ 19,59 say PADC( _RechMsg( 'JT_RECUP096') , 15 ) // "     des     "
    @ 20,59 say PADC( _RechMsg( 'JT_RECUP099') , 15 ) // " R‚sultats   "
      lcReturn += "T"

  ENDIF
  IF pnNbBurLu == pnNbTotBur .AND. LEN(ALLTRIM(PARAM->p_NoPcLut)) == nNbPcTot
    _BOX2(17,58, 21, 74)
    @ 18,59 say PADC( _RechMsg( 'JT_RECUP100') , 15 ) // " (C)l“ture de"
    @ 19,59 say PADC( _RechMsg( 'JT_RECUP101') , 15 ) // " totalisation"
    lcReturn += "CS"
  ENDIF

RETURN(lcReturn)


/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            R‚cup‚ration disquette Urne            ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION ToRecDisq

  LOCAL lnPosiBur
  PRIVATE lnHandle,lcBuffer,lnBurTrait,lnNbFoi,lnNoCol,lnNoLig
  PRIVATE llContinue := _TRUE


  gcDisk := _RechMsg( 'JT_RECUP105') + ' ' // "Urne "

  IF InsDisk() # K_ESC
        DO WHILE (.NOT. DISKREADY(ccDriveActif)) .AND. LASTKEY() # K_ESC
           InsDisk() // Ins‚rez une disquette ds le lecteur
        ENDDO
   ENDIF
   llContinue := (LASTKEY() # K_ESC)
   IF llContinue

      * ouvrir le fichier r‚sultats
      lnHandle := FOPEN(ccDriveActif+"Urne.ind")
      IF FERROR() = 0

         lcBuffer := SPACE(17)
         * lecture NøArr+NøCanton+NøBur+date ‚lection (le jeton)
         IF FREAD(lnHandle,@lcBuffer,17) <> 17
            * problŠme lecture
            @23,70 SAY "Er3: lec I"
            _Message('JT_RECUP115', _TRUE) // Lecture du jeton impossible

         ELSE
            * isoler le nø de bureau
            lnBurTrait := VAL(SUBSTR(lcBuffer,7,3))
            DBSELECTAREA("BURVOTES")
            DBGOTOP()
            lnPosiBur := 1
            DO WHILE b_numero <> lnBurTrait .AND. !EOF()
               lnPosiBur++
               DBSKIP()
            ENDDO
            IF b_numero == lnBurTrait
               lnNbFoi := ROUND( (lnPosiBur /15)+.49,0)
               lnNoLig := 2 + (lnNbFoi * 2)
               lnNoCol := 1 + ((lnPosiBur - ((lnNbFoi-1) * 15)) * 4)
               @ lnNoLig-1, lnNoCol SAY b_numero PICTURE '999' COLOR _COLOR_BLINK+_COLOR_MENU
               @ lnNoLig, lnNoCol+1 SAY " "+_FLECHE_H COLOR _COLOR_BLINK+_COLOR_MENU

               FCLOSE(lnHandle)

               RecDisq(_MENU_RESULTAT)

            ELSE
              _Message(, _TRUE, _RechMsg('JT_RECUP106')+' ' +STR(lnBurTrait,3)+' '+_RechMsg('JT_RECUP107')) // Ce bureau du jeton impossible
            ENDIF
         ENDIF
      ELSE   && problŠme d'ouverture du fichier
        _Message('JT_RECUP015', _TRUE) // Il n'y a pas de vote sur cette disquette
      ENDIF

      FCLOSE(lnHandle)

   ENDIF

RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³     R‚cup‚ration des r‚sultats Pc Total. > 1      ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION RecResPc
  LOCAL nDiskOk, cNumPc, cAppend, nNbBur
  PRIVATE llContinue := _TRUE, gcDisk

  gcDisk := _RechMsg('JT_RECUP111') // 'TRANSFERT DES RESULTATS '
  nDiskOk := 0
  IF InsDisk() # K_ESC
     DO WHILE (.NOT. DISKREADY(ccDriveActif)) .AND. LASTKEY() # K_ESC
       InsDisk() // Ins‚rez une disquette ds le lecteur
     ENDDO
  ENDIF
  llContinue := (LASTKEY() # K_ESC)
  IF llContinue
    IF FILE( ccDriveActif+'nopctot.dat') .AND. FILE( ccDriveActif+'TRANS.ZIP')
      cNumPc := LEFT(MEMOREAD( ccDriveActif+'nopctot.dat'),1)
      IF cNumPc $ PARAM->p_NoPcLut
        nDiskOk := 2
      ENDIF
    ELSE
      nDiskOk := 1
    ENDIF
    IF nDiskOk == 0
       @ 23,01 CLEAR TO 24,78
       _AffMsg(24,01,_RechMsg('JT_GENDK045')) // R‚cup‚ration en cours...
      // r‚cup‚ration de la disquette
      // - Reprise des bureaux
      // - R‚cup‚ration de rv?????? et totalisation
      DBSELECTAREA( 'BURVOTES' )

* md c:\temprec
      ccMd='md c:\temprec > '+_PATH_JITES +'\jt_run'
      RUN &ccMd

* cd c:\temprec
      ccCd='cd c:\temprec > '+_PATH_JITES +'\jt_run'
      RUN &ccCd

* pkunzip a:\trans.zip
      ccUnzip= 'c:\jites\pkxarc -rg'+ PassWordPc+PassWordElect+' a:\trans.zip  > '+_PATH_JITES +'\jt_run'
      RUN &ccUnzip
* cd ..
     ccCd='cd '+ccDriveActif + ' > '+_PATH_JITES +'\jt_run'
     RUN &ccCd 
      cAppend := 'c:\temprec\' +'bv'+ ccElecActive
      APPEND FROM &cAppend
      nNbBur :=0
      DBGOTOP()
      DO WHILE !EOF()
        nNbBur++
        DBSKIP()
      ENDDO
      DBSELECTAREA( 'RECUPVOTES' )

* cd c:\temprec
      cAppend := 'c:\temprec\' +'rv'+ ccElecActive
      APPEND FROM &cAppend
      @ 23,01 SAY SPACE(78) COLOR _COLOR_MENU
      @ 24,01 SAY SPACE(78) COLOR _COLOR_MENU
      DBSELECTAREA( 'PARAM' )
      REPLACE PARAM->p_NoPcLut WITH ALLTRIM(PARAM->p_NoPcLut)+ cNumPc
      REPLACE PARAM->p_nbrbur  WITH nNbBur
      jtTotPc()
      DBCOMMITALL()

* cd ..
      ccCd='cd..  > '+_PATH_JITES +'\jt_run'
      RUN &ccCd
* deldir c:\temprec
      ccDeldir='deldir c:\temprec  > '+_PATH_JITES +'\jt_run'
      RUN &ccDeldir

      MsgGenDsk("JT_GENDK046",'S') // R‚cup‚ration termin‚.
    ELSE
      DO CASE
        CASE nDiskOK == 1 // Disquette incompatible
                    Beep()
                    Beep()
                    @9,9 to 11,len(_RechMsg('JT_ELECT012'))+10
                    _AffMsg(10,10,_RechMsg('JT_ELECT012')) 
                    _AffMsg(13,8,_RechMsg('JT_CREAT018'))
                    InKey(0)

        CASE nDiskOK == 2 // Disquette dŠj… r‚cup‚r‚e
                    Beep()
                    Beep()
                    @9,9 to 11,len(_RechMsg('JT_ERROR003'))+10
                    _AffMsg(10,10,_RechMsg('JT_ERROR003')) 
                    _AffMsg(13,8,_RechMsg('JT_CREAT018'))
                    InKey(0)

      ENDCASE
    ENDIF
  ENDIF
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³     Transfert des r‚sultats Pc Total. > 1         ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/
FUNCTION TransRes
  LOCAL llContinue, nRecnoBv, nRecnoRv
  PRIVATE gcDisk,Size_Zip,Size_Free
  gcDisk := _RechMsg('JT_RECUP111') // 'TRANSFERT DES RESULTATS '

  IF InsDisk() # K_ESC
     DO WHILE (.NOT. DISKREADY(ccDriveActif)) .AND. LASTKEY() # K_ESC
       InsDisk() // Ins‚rez une disquette ds le lecteur
     ENDDO
     llContinue := (LASTKEY() # K_ESC)
     IF llContinue
       DO WHILE (.NOT. DISKREADY(ccDriveActif)) .AND. LASTKEY() # K_ESC
         InsDisk() // Ins‚rez une disquette ds le lecteur
       ENDDO
       llContinue := (LASTKEY() # K_ESC)
       IF llContinue
         @ 23,01 CLEAR TO 24,78
         _AffMsg(24,01,_RechMsg('JT_GENDK043')) // Transfert en cours...
         STRFILE( PARAM->p_nopc+ CHR(13)+chr(10), ccDriveActif+'nopctot.dat' )

         DBSELECTAREA( 'BURVOTES' )
         nRecnoBv := RECNO()
         USE

         DBSELECTAREA( 'RECUPVOTES' )
         nRecnoRv := RECNO()
         USE
         llFerror := _FALSE
         ccZip := 'c:\jites\PKARC ag'+PassWordPc+PassWordElect+' c:\JITES\trans.zip '+ _PATH_ELECTION + '\?v*.DBF  > '+_PATH_JITES +'\jt_run'

         RUN &ccZip
         Size_Zip  := FILESIZE("c:\jites\trans.zip")
         Size_Free := DISKFREE("A")

         IF Size_Zip >= Size_Free
                    Beep()
                    Beep()
                    @9,9 to 11,len(_RechMsg('JT_ERROR002'))+10
                    _AffMsg(10,10,_RechMsg('JT_ERROR002')) // Pas assez de place sur la disquette
                    _AffMsg(13,8,_RechMsg('JT_CREAT018'))
                    InKey(0)

         ELSE
                  ccZip := 'c:\jites\PKARC ag'+PassWordPc+PassWordElect+' a:\trans.zip '+ _PATH_ELECTION + '\?v*.DBF  > '+_PATH_JITES +'\jt_run'
                  RUN &ccZip
                  IF ERRORLEVEL() # 0 .OR. DOSERROR() # 0
                    _msgerr('JT_CREAT009') // Le run n'a pas fonctionn‚
                     llFerror := _TRUE
                  ENDIF
         ENDIF
                    
         * Ouverture du fichier bureaux de votes
         _use( "bv" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_BUREAUX_VOTES , _EXCLUSIVE, 'BURVOTES')
         IF nRecnoBv # 0
           DBGOTO( nRecnoBv )
         ENDIF
         * Ouverture du fichier r‚cup‚ration des votes
         _use( "rv" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_RECUP_VOTES , _EXCLUSIVE, 'RECUPVOTES')
         IF nRecnoRv # 0
           DBGOTO( nRecnoRv )
         ENDIF
         DBSELECTAREA( 'BURVOTES' )
         IF Size_Zip < Size_Free
                 MsgGenDsk("JT_GENDK044",'S') // Transfert termin‚.
         ENDIF
       ENDIF
     ENDIF
  ENDIF
RETURN(NIL)

/*ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³            Test si totalisation ???               ³
  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ*/

FUNCTION ToOnTotalise

  PRIVATE lnNbBur       := 0 ,; // Nb de bureaux total
          lnNbBurTot    := 0 ,; // nb bureaux totalis‚s
          lnNumMaxTrait := 0 ,; // Nb de groupes d‚j… totalis‚s
          laNoLecture   := {},; // Nb de bureaux en Lecture
          lnI                ,;
          lnRecBurVotes

  DBSELECTAREA('BURVOTES')
  lnRecBurVotes := RECNO()
  GO TOP
  DO WHILE .NOT. EOF()
    lnNbBur++
    IF b_numtot # 0
      lnNbBurTot++
      lnNumMaxTrait := MAX(lnNumMaxTrait, b_numtot)
    ELSE
      IF AT(_STAT_BUR_LECTURE, b_statut) # 0
        AADD(laNoLecture, b_numero)
      ENDIF
    ENDIF
    DBSKIP()
  ENDDO

  IF ( LEN(laNoLecture) == _TOT_PART_TOUT_LES .AND. (lnNbBur - (lnNbBurTot+_TOT_PART_TOUT_LES)) >= _TOT_PART_MIN_REST );
     .OR. ( (LEN(laNoLecture) +lnNbBurTot) == lnNbBur)

    _AffMsg(23,1," "+_RechMsg('JT_RECUP108') + ' ' +REPLICATE(".",LEN(laNoLecture)),AM_ALIGN_GA,AM_EFF_CENTRE,_COLOR_MENU) // Totalisation en cours "
    @ 24,01 SAY SPACE(78) COLOR _COLOR_MENU

    FOR lnI:= 1 TO LEN(laNoLecture)
      @ 23,23+lnI SAY CHR(4) COLOR _COLOR_BLINK+_COLOR_MENU
      jtBurTot(@lnNbBurTot, lnNbBur, laNoLecture[lnI])
      DBSEEK(laNoLecture[lnI])
      of_reclock(0)
      REPLACE b_numtot with lnNumMaxTrait+1
      UNLOCK
      COMMIT
      @ 23,23+lnI SAY CHR(4) COLOR _COLOR_MENU
    NEXT lnI

  ENDIF

  DBGOTO(lnRecBurVotes)

RETURN(NIL)
FUNCTION RechCantN
  PARAMETERS pcType,;
             pcCommun
  LOCAL cNomCanton,lnOldSelect:=SELECT()
  pcType := IIF(pcType==NIL,"CANT",pcType)
  pcCommun := IIF(pcCommun==NIL,BURVOTES->b_commun,pcCommun)
  _use( "jtcanton" ,_PATH_JITES + '\' , _SELECT_CANTONS        , _EXCLUSIVE, 'LISTE_CANT')
  DBSELECTAREA( _SELECT_CANTONS )
  LOCATE ALL FOR CAN_TYPREC == 'CANT' .AND. CAN_ARROND == ccNumArr .AND. CAN_CODE ==PARAM->p_numcan
  cNomCanton := can_nomn
  SELECT(lnOldSelect)
RETURN cNomCanton


FUNCTION RechCantF
  PARAMETERS pcType,;
             pcCommun
  LOCAL cNomCanton,lnOldSelect:=SELECT()
  pcType := IIF(pcType==NIL,"CANT",pcType)
  pcCommun := IIF(pcCommun==NIL,BURVOTES->b_commun,pcCommun)
  _use( "jtcanton" ,_PATH_JITES + '\' , _SELECT_CANTONS        , _EXCLUSIVE, 'LISTE_CANT')
  DBSELECTAREA( _SELECT_CANTONS )
  LOCATE ALL FOR CAN_TYPREC == 'CANT' .AND. CAN_ARROND == ccNumArr .AND. CAN_CODE ==PARAM->p_numcan
  cNomCanton := can_nomf
  SELECT(lnOldSelect)
RETURN cNomCanton

FUNCTION ValNbrNul(pnNbrNul,;     && nbr de votes nuls … valide
                   pnNbrVotant)   && nbr de votants
LOCAL llReturn:=_TRUE
IF pnNbrNul > 2500-pnNbrVotant
   lcSavLig := SAVESCREEN(16,0,16,79)
   _beep()
   @ 16,12 SAY _RechMsg('JT_RECUP112') // Le nombre de votes nuls est trop ‚lev‚.
   INKEY(0)
   RESTSCREEN(16,0,16,79,lcSavLig)
   llReturn := _FALSE
ENDIF
RETURN(llReturn)

FUNCTION LangImp(pcMsg)
LOCAL lcLgImp,lnChoixLg
pcMsg := IIF(pcMsg==NIL,"JT_LANGUE",pcMsg)
IF ccNumArr == _ARROND_BRUXELLES
   * Choix de la langue
   lnChoixLg := _gesPrompt(pcMsg,17,,_TRUE)
   IF lnChoixLg == 1   // Fran‡ais
      lcLgImp := _FRANCAIS
   ELSE
      lcLgImp := _NEERLANDAIS
   ENDIF
ELSE
   lcLgImp := ccLangue
ENDIF
RETURN(lcLgImp)
