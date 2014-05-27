  #include "inkey.ch"
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include "jites.ch"

/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё Password encoding routine                         Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
  FUNCTION jtacces
    PARAMETER plChgMotP ,;   // With the possibility to change the password yes or no
              pcNoMP    ,;   // Password number entered
              pcNoMPSav      // Password number modified

    PRIVATE llMotPOK  :=.NOT. _MOT_PASSE_OK ,;  // Result of the password entering
            lcMotPass := _CHAINE_VIDE       ,;
            leEcran   := ''                 ,;
            lcNoMPEnc
    pcNoMP    := '1'
    pcNoMPSav := '1'
    KSETNUM(_TRUE)   // Keypad in NumLock

    SET CURSOR OFF
    DO WHILE _TRUE
      lcMotPass := Get_MotP(_NUMERIQUE + CHR(K_ESC) + CHR(K_BS),10, 25,10)
      IF EMPTY(lcMotPass)
        EXIT
      ENDIF

      IF TypeResul=0
          lcNoMPEnc:="X"
          llMotpOk := _TRUE
          EXIT
      ELSE
         RETURN(lcMotPass)
         EXIT
      ENDIF
    ENDDO
    SET CURSOR ON

 RETURN IIF( llMotPOK, lcNoMPEnc, '')


/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё Conversion MOT PASSE alphab┌tique en num┌rique    Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
  FUNCTION Conv_Num
    PARAMETER pcMotPAlpha
    LOCAL ln_i, lcMotPNum
    lcMotPNum := _CHAINE_VIDE
    FOR ln_i = 1 to LEN(pcMotPAlpha)
      lcMotPNum := lcMotPNum + CHR(48 + MOD(ASC(SUBSTR(pcMotPAlpha, ln_i, 1)),10))
    NEXT
  RETURN lcMotPNum

/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё Saisie du MOT de PASSE                            Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
  FUNCTION Get_MotP        // Input of a password
    PARAMETER pcCarPermis, pnNoLig, pnNoCol, pnLongMotp
    LOCAL lcMotPass  := _CHAINE_VIDE   ,;
          lnLgGetMP  := 0              ,;
          lnGet1Car  := ASC(SPACE(1))  ,;
          leEcran                      ,;
          leZon1Ecr                    ,;
          leZon2Ecr                    ,;
          leZon3Ecr                    ,;
          llContinue := _TRUE          ,;
          lcPW2                        ,;
          lbClefSurPctot
    PRIVATE gcDisk

  leZon3Ecr := SAVESCREEN(PnNoLig , PnNoCol ,PnNoLig+5 , PnNoCol+30)

  DO WHILE _TRUE
    lcMotPass  := _CHAINE_VIDE  
    lnLgGetMP  := 0             
    lnGet1Car  := ASC(SPACE(1)) 
    pnLongMotP := IIF(pnLongMotP == NIL , 5, pnLongMotP)
    leZon1Ecr := SAVESCREEN(PnNoLig , PnNoCol ,PnNoLig+5 , PnNoCol+30)
    @  PnNoLig+1 , PnNoCol ,PnNoLig+5 , PnNoCol+30 BOX REPLICATE(CHR(176),9) COLOR _COLOR_OMBRE
    SETCOLOR(_COLOR_MENU)
    SETCOLOR(_COLOR_BLANC_BLEU)
    @  PnNoLig+0 ,PnNoCol+0, PnNoLig+4, PnNoCol+28 BOX B_SINGLE + ' '
    IF TypeResul=3  // Recuperation of a ballot box floppy with a manual password
            @  PnNoLig+1 ,PnNoCol+2  SAY _rechmsg('JT_RECUP152')   // "Polling office password"
    ELSE    
            @  PnNoLig+1 ,PnNoCol+2  SAY _rechmsg('JT_ACCES001')   // "Enter your password"
    ENDIF
    @  PnNoLig+3 ,PnNoCol+14-(pnLongMotP/2) SAY REPLICATE(".", pnLongMotP)
    @  PnNoLig+3 ,PnNoCol+14-(pnLongMotP/2) SAY "ш"
    leEcran := ''

    DO WHILE lnLgGetMP < pnLongMotP .AND. lnGet1Car # K_ESC
      lnGet1Car := ASC(SPACE(1))
      DO WHILE AT( CHR(lnGet1Car), pcCarPermis) = 0 .AND. lnGet1Car # K_ALT_F1
        KSETNUM(_TRUE)   // Keypad in NumLock
        lnGet1Car := INKEY(0)
        IF .NOT. EMPTY(leEcran)
          _SayMsg('JT_ACCES008', _FALSE, leEcran) // Show previous screen
          leEcran := ''
        ENDIF
            IF AT( CHR(lnGet1Car), pcCarPermis) = 0
              * Message : The password may only contain numbers
              _beep()
              leEcran := _SayMsg('JT_ACCES008', _TRUE) // Enter only numbers
            ENDIF
            lnGet1Car := IIF( lnGet1Car<0 , 0, lnGet1Car)
      ENDDO

      DO CASE
        CASE lnGet1Car == K_BS
          IF lnLgGetMP > 0
            lnLgGetMP := lnLgGetMP - 1
            lcMotPass := LEFT(lcMotPass, LEN(lcMotPass) - 1 )
            @ PnNoLig+3, PnNoCol + 14-(pnLongMotP/2) + lnLgGetMP + 1 SAY "."
          ENDIF
        CASE lnGet1Car == K_ESC
          lcMotPass  := _CHAINE_VIDE
          lnLgGetMP  := 0
          IF TypeResul==3
            pnLongMotp := 0
          ENDIF
        OTHERWISE
          lnLgGetMP := lnLgGetMP + 1
          lcMotPass := lcMotPass + CHR(lnGet1Car)
          @ PnNoLig+3 , PnNoCol+ 14-(pnLongMotP/2) + lnLgGetMP - 1 SAY CHR(lnGet1Car)
      ENDCASE
      @ PnNoLig+3 , PnNoCol + 14-(pnLongMotP/2) + lnLgGetMP SAY "ш"
    ENDDO
    @ PnNoLig + 3 , PnNoCol + 14-(pnLongMotP/2) + lnLgGetMP SAY " "
    
    * Password saving in the PUBLIC PASSWORD variable
    PassWord := lcMotPass

    IF  ( pcCommande==_MENU_RESULTAT .AND.  LASTKEY()#K_ESC ;
      .AND.  (TypeResul = 0 .OR. TypeResul = 3 ) .OR. pnLongMotp  = 10 )

        PassWord2 := PassWord
           lcMotPassD:=LEFT(Password,8)
           lcMotPassF:=RIGHT(Password,2)
           lnMotPassD:=VAL(lcMotPassD)
           lnMotPassF:=VAL(lcMotPassF)
           llContinue = _TRUE
           IF lnMotPassF != 99 - MOD(lnMotPassD,97) 
            leEcran := _SayMsg('JT_ACCES007', _TRUE) // Wrong PASSWORD
            A := INKEY(0)
            _SayMsg('JT_ACCES007', _FALSE, leEcran) // Show previous screen
            loop
           ENDIF
        IF TypeResul=3 .OR. pcCommande != _MENU_RESULTAT 
            EXIT
        ENDIF

      IF llContinue
       IF .NOT. FILE("a:\floppy.be")
        lbClefSurPctot = _FALSE
        gcDisk := _RechMsg('JT_RECUP183') // Insert the TOT PC floppy in the floppy drive
        llContinue  := _FALSE
        IF InsDisk() # K_ESC
             DO WHILE ((.NOT. DISKREADY("a:")) .OR. llContinue = _FALSE ).AND. LASTKEY() # K_ESC
               IF FILE("a:\floppy.be")
                 FILECOPY ('a:\floppy.be','c:\floppy.be')
                 llContinue := _TRUE
               ELSE
                 _Message('JT_ACCES015', _TRUE) // This floppy does not contain any keys
                 InsDisk() // Insert a floppy in the floppy drive
               ENDIF
             ENDDO
        ELSE
            llContinue = _FALSE
        ENDIF
       ELSE
         FILECOPY ('a:\floppy.be','c:\floppy.be')
         llContinue = _TRUE
         lbClefSurPctot = _TRUE
       ENDIF
      ENDIF

        IF llContinue
           IF .NOT. lbClefSurPctot
              gcDisk := _RechMsg('JT_RECUP083') // Insert the TOT PC floppy in the floppy drive
              IF InsDisk() # K_ESC
               DO WHILE (.NOT. DISKREADY("a:")) .AND. LASTKEY() # K_ESC
                 InsDisk() // Insert the floppy in the floppy drive
               ENDDO
              ENDIF
           ENDIF
        ENDIF

        IF llContinue .AND. LASTKEY() # K_ESC
            lcPW2=_FALSE
            lcPW2:=VerifPW()

            IF lcPW2=_TRUE
                 EXIT
            ENDIF

            leZon2Ecr := SAVESCREEN(18,0,20,80)
            RESTSCREEN(18,0,20,80,leZon2Ecr)
        ELSE
            llContinue = _FALSE
            @ 23,1 SAY SPACE(78) COLOR _COLOR_MENU
            @ 24,1 SAY SPACE(78) COLOR _COLOR_MENU
        ENDIF
    ELSE
            EXIT
    ENDIF

    RESTSCREEN(PnNoLig, PnNoCol, PnNoLig+5, PnNoCol+30, leZon1Ecr)
  ENDDO

  RESTSCREEN(PnNoLig, PnNoCol, PnNoLig+5, PnNoCol+30, leZon3Ecr)

  RETURN lcMotPass


/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё Affichage et modification de la date et l'HEURE   Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
  FUNCTION DateHeur
    PRIVATE leEcran
    * Confirmation de la date de l'heure
    leEcran := SAVESCREEN(10 , 20, 15, 62)
    @  11 , 22, 15, 62  BOX REPLICATE(CHR(176),9) COLOR _COLOR_OMBRE
    SETCOLOR(_COLOR_MENU)
    SETCOLOR(_COLOR_BLANC_BLEU)
    @  10 , 20, 14, 60  BOX B_SINGLE +' '
    @ 11, 22 say _rechmsg('JT_ACCES009') + ' ' + DTOC(ccDateJour)
    @ 13, 22 say _rechmsg('JT_ACCES010') + ' ' + ccHeureJour

    IF _GetRep('JT_ACCES011') == _REPONSE_OUI  // Voulez-vous modifier le DATE ou l'HEURE  ?
      @ 11, 22 say _rechmsg('JT_ACCES009') GET ccDateJour                     VALID( YEAR(ccDateJour) >= 2000 .AND. YEAR(ccDateJour) <= 2020 )
      @ 13, 22 say _rechmsg('JT_ACCES010') GET ccHeureJour PICTURE '99:99:99' VALID( TIMEVALID(ccHeureJour))
      READ
      IF LASTKEY() # K_ESC
        SETDATE(ccDateJour, _TRUE)
        SETTIME(ccHeureJour,_TRUE)
      ENDIF
    ENDIF
    RESTSCREEN(10 , 20, 15, 62, leEcran)
  RETURN

***
/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё Choix de l'┌lection                               Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
  FUNCTION ChoixElect
     PARAMETER plAvecQuestion

    LOCAL ldDate, lnLigDeb, lnLigFin, lnChoix, lnReponse, lcListType
    PRIVATE lnRetour := 0

    plAvecQuestion := IIF( plAvecQuestion==NIL, _TRUE, plAvecQuestion)

    * Choice of an election
    DBSELECTAR('ELECT')
    ccElecActive := ''
    ccElecOld    := ''
    ccTypeOld    := ''
    lcListType   := ''
    GO TOP

    DO WHILE (.NOT. EOF())

      IF .NOT. EMPTY(elec_datec)
        IF .NOT. EMPTY(elec_dir)
          ccElecActive := RIGHT(elec_dir, 6 )
          ccTypeElec   := LEFT(elec_dir,2)
          IF DIRCHANGE(_PATH_ELECTION) == _NO_DISK_ERR
            ldDate := RIGHT(elec_dir,2)+'/'+SUBSTR(elec_dir,5,2)+'/'+SUBSTR(elec_dir,3,2)
            lcListType:= lcListType + left(elec_dir,2)

            * ELECTION DBF STRUCTURE CHECK
            of_ctrldbf( _PATH_ELECTION + '\pa' + RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\param'   + '.dbf' )

            * Opening of the parameters file
            _use( "pa" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_PARAMETRES , _EXCLUSIVE, 'PARAM')

            DBSELECTAREA('ELECT')
            ccElecOld := ccElecActive
            ccTypeOld := ccTypeElec
            param->(DBCLOSEAREA())
          ELSE
            ccElecActive := ccElecOld
            ccTypeElec   := ccTypeOld
          ENDIF
        ENDIF
      ENDIF
      DBSELECTAREA('ELECT')
      SKIP 1
    ENDDO
    IF .NOT. EMPTY(ccElecActive)
      cdDateElec   := CTOD(RIGHT(ccElecActive,2)+'/'+SUBSTR(ccElecActive,3,2)+'/'+LEFT(ccElecActive,2))
      ccDateElec  := RIGHT(ccElecActive,2)+'/'+SUBSTR(ccElecActive,3,2)+'/'+'20'+LEFT(ccElecActive,2)

      IF LEN(laTabElec) > 1 // There is more than 1 active election
      ELSE
        @ 10, 01, 22, 78 BOX REPLICATE(CHR(176),9) COLOR _COLOR_FOND
        * Confirmation of the election date
        SETCOLOR(_COLOR_OMBRE)
        @ 11, 22, 13, 62  BOX REPLICATE(CHR(176),9)
        SETCOLOR(_COLOR_MENU)
        SETCOLOR(_COLOR_BLANC_BLEU)
        @ 10, 20, 12, 60  BOX B_SINGLE +' '
        @ 11, 22 say _rechmsg('JT_ENCOD002') + ' ' + ccDateElec

        ccTypeElec   := SUBSTR(lcListType, +1,2)
        * Opening of the parameters file
        _use( "pa" + ccElecActive , _PATH_ELECTION + '\' , _SELECT_ELECT_PARAMETRES , _EXCLUSIVE, 'PARAM')
        USE
      ENDIF
    ELSE
      lnRetour := K_ESC
    ENDIF
    IF lnRetour # K_ESC
       DBSELECTAREA('elect')
       LOCATE ALL FOR ccElecActive == RIGHT(elec_dir, 6 )
       IF FOUND()
* re-indexation
          IF SUBSTR(elec_vers,4,1) == "*"
             clPlantage := _TRUE
             IF (pcCommande # _MENU_RESULTAT) 
                UTReindex()
             ENDIF
          ELSE
             clPlantage := _FALSE
             of_reclock(0)
             REPLACE elec_vers WITH STUFF(elec_vers,4,1,"*")
             UNLOCK
             DBCOMMITALL()
          ENDIF
       ENDIF
    ENDIF

  RETURN lnRetour

/*зддддддддддддддддддддддддддддддддддддддддддддддддддд©
  Ё V┌rification du mot de passe                      Ё
  юддддддддддддддддддддддддддддддддддддддддддддддддддды*/
FUNCTION VerifPW
    PRIVATE lcCRC   , leZoneEcran, ;
            lcDel   ,;
            lcPW := _TRUE

    _AffMsg(23,1,_RechMsg('JT_RECUP201') ,AM_ALIGN_CE,AM_EFF_CENTRE,_COLOR_BLINK+_COLOR_MENU) // Patientez ...
    @24,0 clear to 24,80

             DELETE FILE 'A:\log.err'
             lcCRC:='c:\jites\rijndael r 1 ' + passWord + ' a: a: c:'
             RUN &lcCRC
             @ 23,1 SAY SPACE(78) COLOR _COLOR_MENU

             IF FILE('a:log.err')
               Beep()
               Beep()
               leZoneEcran := SAVESCREEN(18,0,20,80)
               @18,4 clear to 20,70
               _AffMsg(18,5,_RechMsg('JT_RECUP115')) // Lecture du jeton impossible CRC
               IF ccLangue=_NEERLANDAIS
                 @19,4 SAY "of"
               ELSE
                 @19,4 SAY "ou"
               ENDIF
               _AffMsg(20,5,_RechMsg('JT_UTIL701')+IIF(ccLangue=_NEERLANDAIS,' niet goed !',' incorrect !'))  // Mauvais PW
               lcPW:=_FALSE

               lcDel:='DEL a:log.err'
               RUN &lcDel

               Inkey(0)
               RESTSCREEN(18,0,20,80,leZoneEcran)

            * ENDIF
           ENDIF

RETURN(lcPW)
