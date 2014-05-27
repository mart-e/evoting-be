  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'jites.ch'

/*
        ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
        บ                    Logiciel  J I T E S                     บ
        บ                                                            บ
        บ  Traitement du jour de l'lection                          บ
        ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

FUNCTION jtElect
  PARAMETER pcCommande

  LOCAL lnRetDirChge, lcMsgTmp, lcRep_O_N, llElectExiste,lcNoMp,;
        lcCleElec, lcNewMotP, lcCtrlMotP, lnChoix, lcDrive, llMsgErr, lcTransfert

  PRIVATE ccXcopy, laTabElec :={}, lcRun

  SSETVERIFY(_VERIFY_ON)


  * ouverture du fichier des messages
  _use("jt_msg",_PATH_JITES + '\' ,_SELECT_MESSAGES)
  _use("elect",_PATH_JITES + '\' ,_SELECT_LES_ELECTIONS, _EXCLUSIVE)
  @ 4, _centre(0, 80, len(_RechMsg('JT_ELECT004'))) SAY _RechMsg('JT_ELECT004') COLOR _COLOR_MENU

  lnRetDirChge = DIRCHANGE(_PATH_JITES)

  DO CASE
    CASE DISKFREE(_UNITE_C) - _CAPACITE_DISK_MINIMUM < 0
      _msgerr('JT_CREAT019') // Impossible d'excuter le programme, Capacit insuffisante

    CASE lnRetDirChge = _NO_DISK_ERR
          IF .NOT. EMPTY(lcNoMp:=jtacces(_TRUE,'1','1'))  // Traitement de saisie du mot passe

            IF pcCommande == _MENU_RESULTAT 
               DateHeur()   // Confirmation de la date de l'heure
            ENDIF
            IF ChoixElect() # K_ESC // Choix de l'lection
              _AffMsg(24,01,_RechMsg("JT_ELECT100"),AM_ALIGN_GA,AM_EFF_OUI,;
                      _COLOR_BLINK+_COLOR_MENU)   // Initialisation en cours...

              llLoginOK := _TRUE
              IF pcCommande == _MENU_RESULTAT 
                 IF FILE(_PATH_ELECTION+"\Data.ARC")
                         lcRun := "Pkxarc -r "+_PATH_ELECTION+"\data.arc >nil"
                         RUN &lcRun
                         lcRun := "Erase "+_PATH_ELECTION+"\data.arc >nil"
                         RUN &lcRun
                 ENDIF
                 llLoginOK := _TRUE

                 * INITIALISER LA VARIABLE ccOperElec qui n'a pas pu l'tre dans
                 * ChoixElect() car param tait encrypt
                 * Ouverture du fichier des paramtres
                 _use( "pa" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_PARAMETRES , _EXCLUSIVE, 'PARAM')
                 ccOperElec := PARAM->p_OperEle
                 ccNoPc = PARAM->p_NoPc
                 param->(DBCLOSEAREA())
              ENDIF

              IF llLoginOK
                VerifNtx(_INDEX_ALL_FILE)

                lnRetDirChge = DIRCHANGE(_PATH_ELECTION)

                * Ouverture du fichier des paramtres
                _use( "pa" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_PARAMETRES , _EXCLUSIVE, 'PARAM')
                * Ouverture du fichier bureaux de votes
                _use( "bv" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_BUREAUX_VOTES , _EXCLUSIVE, 'BURVOTES')
                * Ouverture du fichier des CANDIDATS de l'lection
                _use( "cd" + right(ccElecActive,6),_PATH_ELECTION + '\' , _SELECT_ELECT_CANDIDATS , _EXCLUSIVE, 'CANDIDATS')
                * Ouverture du fichier des LISTES de l'lection
                _use( "LI" + right(ccElecActive,6),_PATH_ELECTION + '\' , _SELECT_ELECT_LISTES    , _EXCLUSIVE, 'LISTES')
                * Ouverture du fichier des scrutins de l'lection
                _use( "sc" + right(ccElecActive,6),_PATH_ELECTION + '\' , _SELECT_ELECT_SCRUTINS , _EXCLUSIVE, 'SCRUTINS')
                * Ouverture du fichier rcupration des votes
                _use( "rv" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_RECUP_VOTES , _EXCLUSIVE, 'RECUPVOTES')
                @ 24,00 SAY SPACE(80) COLOR _COLOR_MENU
                DO CASE
                  CASE lnRetDirChge = _NO_DISK_ERR
                    TotMenu()   // dans JT_RECUP.PRG
                  CASE DISKFREE(_UNITE_C) - _CAPACITE_DISK_MINIMUM < 0
                   _msgerr('JT_CREAT007') // Capacit insuffisante
                  CASE lnRetDirChge = _ER_ACCES_DENIED
                    _msgerr('JT_CREAT008') // Acces denied
                  CASE lnRetDirChge = _ER_PATH_NOT_FOUND
                    _msgerr('JT_CREAT006') // Rpertoire non trouv
                ENDCASE
              ENDIF
            ENDIF
          ENDIF
  ENDCASE
RETURN(NIL)
