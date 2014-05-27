  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'fileio.ch'
  #include 'jites.ch'

/*
        ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
        บ                    Logiciel  J I T E S                     บ
        บ                                                            บ
        บ  Gnration de la disquette                                บ
        ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
*/

FUNCTION jtRDisk
LOCAL lnOldSelect:=SELECT(),lcDrive:="A:",lcFile,lnHandle,lnRetMsg:=0,;
      lcBuffer,lcNumCant,lnVoteEff,lnVoteSup,;
      lnNbrVotn,lnNbrVote,lnNbrVots,lnNbrNuln,lnNbrNule,lnNbrNuls,;
      lnNbrVotT,lnNbrNulT,;
      lcSavLig:=SAVESCREEN(23,0,24,79),lcTypeElec,;
      ccLstScrut, lnNoScrut, ccScrutTraite, gnVoteFr, gnVoteNe, cNomFiRV, cNomFiBV, ccCleRech
PRIVATE lcRun

lnRetMsg := MsgGenDsk("JT_RDISK002",,,_TRUE)   // Insrez une disquette dans le lecteur A
_AffMsg(23,01,_rechmsg("JT_RDISK003"),AM_ALIGN_GA,AM_EFF_OUI,_COLOR_MENU)
IF lnRetMsg # K_ESC .AND. GDTestPres(lcDrive,1,_FALSE) # K_ESC
     IF DISKSPACE(lcDrive) < 1000000
          RESTSCREEN(23,0,24,79,lcSavLig)
          SELECT(lnOldSelect)
          RETURN(NIL)
     ENDIF
     _AffMsg(24,01,_rechmsg("JT_RDISK004"),AM_ALIGN_GA,AM_EFF_OUI,_COLOR_MENU)
     IF FILE("a:\urne.exe") .OR. FILE("a:\totali.zip")
   * BV or TOT floppy
        lnRetMsg := MsgGenDsk( 'JT_ELECT012') //  Bad floppy
        RESTSCREEN(23,0,24,79,lcSavLig)
        SELECT(lnOldSelect)
        RETURN(NIL)
     ENDIF

   @ 24,00 SAY SPACE(80)

   DO CASE
     CASE ccOperElec == _TYPE_OPER_LEGISLATIVE
       ccLstScrut := _SCRUTIN_CHAMBRE + _SCRUTIN_SENAT
       lcLesTypes := "CS"
     OTHERWISE
       ccLstScrut := ''
       lcLesTypes := "?"
   ENDCASE
   PARAM->(DBGOTOP())
   lcFileBur := lcDrive + "\DET" + SUBSTR( 'xLBHVAORU',VAL(RIGHT(ccNumArr,1))+1,1)+ PARAM->p_numcan + 'a' + ".TXT"
   IF FILE(lcFileBur)
        FILEDELETE(lcFileBur)
   ENDIF
   lnHandleDet := FCREATE(lcFileBur,0)
   DBSELECTAREA('BURVOTES')
   DBGOTOP()
   DO WHILE !EOF()
      lnDeclare:= BURVOTES->b_nbr_nuls +BURVOTES->b_nbr_nule +BURVOTES->b_nbr_nuln
      lcBufferDet := STR(BURVOTES->b_numero,3);
                     +SUBSTR(BURVOTES->b_statut,29,8);
                     +STR(BURVOTES->b_nbr_votn,6);
                     +STR(lnDeclare,6)

      FWRITE(lnHandleDet,lcBufferDet+_ASCII_CR_LF)
      DBSKIP()
   ENDDO
   FCLOSE(lnHandleDet)

   FOR lnNoScrut := 1 TO LEN( ccLstScrut)
     ccScrutTraite := SUBSTR( ccLstScrut, lnNoScrut, 1)
     lcTypeElec    := SUBSTR( lcLesTypes, lnNoScrut, 1)

     PARAM->(DBGOTOP())
     lcFile := lcDrive + "\TOT" + SUBSTR( 'xLBHVAORU',VAL(RIGHT(ccNumArr,1))+1,1)+ PARAM->p_numcan + lcTypeElec + ".TXT"
     IF FILE(lcFile)
        FILEDELETE(lcFile)
     ENDIF
     lnHandle := FCREATE(lcFile,0)
     IF lnHandle # -1
        * compter le nbr de votes
        DBSELECTAREA('BURVOTES')
        DBGOTOP()

        lnNbrVotn:=0
        lnNbrVote:=0
        lnNbrVots:=0

        lnNbrNuln:=0
        lnNbrNule:=0
        lnNbrNuls:=0

        DO WHILE !EOF()
           DO CASE
               CASE ccScrutTraite = _SCRUTIN_EUROPEEN
                  lnNbrVote += BURVOTES->b_nbr_votn +BURVOTES->b_nbr_vote
                  lnNbrNule += BURVOTES->b_nbr_nuln +BURVOTES->b_nbr_nule

               CASE ccScrutTraite = _SCRUTIN_REGIONAL
                  lnNbrVote += BURVOTES->b_nbr_votn
                  lnNbrNule += BURVOTES->b_nbr_nuln

               CASE ccScrutTraite = _SCRUTIN_BXL_REGION_FLA 
                  lnNbrVote += BURVOTES->b_nbr_votn
                  lnNbrNule := 0

               OTHERWISE   
                  lnNbrVote += BURVOTES->b_nbr_votn +BURVOTES->b_nbr_vots
                  lnNbrNule += BURVOTES->b_nbr_nuln +BURVOTES->b_nbr_nuls
                  
           ENDCASE
           DBSKIP()
        ENDDO
        lnNbrVotT = lnNbrVote
        lnNbrNulT = lnNbrNule

        DBSELECTAREA("SCRUTINS")
        LOCATE FOR s_code == ccScrutTraite

        lcNumCant := RIGHT(PARAM->p_numcan,2)
        *
        * RECORD TYPE 01 : CANTON
        *

        IF ccNumArr == _ARROND_BRUXELLES .AND. (SCRUTINS->s_code == _SCRUTIN_SENAT .OR. SCRUTINS->s_code == _SCRUTIN_EUROPEEN .OR. SCRUTINS->s_code == _SCRUTIN_REGIONAL)
          gnVoteFr := 0
          gnVoteNe := 0
          * traitement des listes
          DBSELECTAREA("LISTES")
          DBGOTOP()
          DBSEEK(SCRUTINS->s_code)
          DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. !EOF()
            IF LISTES->l_college == 'F'
              gnVoteFr := gnVoteFr + LISTES->l_nbr_v_ex
            ELSE
              gnVoteNe := gnVoteNe + LISTES->l_nbr_v_ex
            ENDIF
            DBSELECTAREA("LISTES")
            DBSKIP()
          ENDDO
          lcBuffer := lcNumCant+PADR(PARAM->p_burdep,36)+lcTypeElec;
                      +STRZERO(lnNbrVotT+lnNbrNulT,6)  ;
                      +STRZERO(lnNbrNulT+SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,6);
                      +STRZERO(gnVotefr+gnVoteNe,6);
                      +STRZERO(gnVotefr,6);
                      +STRZERO(gnVoteNe,6)
        ELSEIF ccNumArr == _ARROND_BRUXELLES .AND. (SCRUTINS->s_code==_SCRUTIN_BXL_REGION_FLA)
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
              DBSELECTAR('LISTES')
              DBSKIP()
            ENDDO
            lcBuffer := lcNumCant+PADR(PARAM->p_burdep,36)+lcTypeElec;
                      +STRZERO(SCRUTINS->s_nbr_vbe+gnVotefr+gnVoteNe,6)  ;
                      +STRZERO(SCRUTINS->s_nbr_vbe,6);
                      +STRZERO(gnVotefr+gnVoteNe,6)
        ELSE
          lcBuffer := lcNumCant+PADR(PARAM->p_burdep,36)+lcTypeElec;
                      +STRZERO(lnNbrVotT+lnNbrNulT,6);
                      +STRZERO(lnNbrNulT+SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn,6);
                      +STRZERO(lnNbrVotT-SCRUTINS->s_nbr_vbe-SCRUTINS->s_nbr_vbn,6)
        ENDIF


        FWRITE(lnHandle,lcBuffer+_ASCII_CR_LF)

        * traitement des listes
        DBSELECTAREA("LISTES")
        DBGOTOP()
        DBSEEK(SCRUTINS->s_code)
        DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. !EOF()
           * compter la somme des votes nominatifs pour les effectifs et les supplants
           lnVoteEff := lnVoteSup := 0
           DBSELECTAR('CANDIDATS')
           DBSEEK(LISTES->l_codescru+LISTES->l_college+LISTES->l_numero,_ON)
           DO WHILE CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste;
              == LISTES->l_codescru+LISTES->l_college+LISTES->l_numero .AND. !EOF()
              IF CANDIDATS->c_eff_sup == _CODE_EFFECTIF
                 lnVoteEff += CANDIDATS->c_nbr_v_ca
              ELSE
                 lnVoteSup += CANDIDATS->c_nbr_v_ca
              ENDIF
              DBSKIP()
           ENDDO
           DBSELECTAREA("LISTES")
           *
           * RECORD TYPE 02 : LISTE
           *
            lcBuffer := lcNumCant+lcTypeElec;
                          +LISTES->l_numero;
                          +STRZERO(LISTES->l_nbr_v_tl,6);
                          +STRZERO(LISTES->l_nbr_v_ef,6);
                          +STRZERO(LISTES->l_nbr_v_sp,6);
                          +STRZERO(LISTES->l_nbr_v_ex-(LISTES->l_nbr_v_tl+LISTES->l_nbr_v_ef+LISTES->l_nbr_v_sp),6);
                          +STRZERO(LISTES->l_nbr_v_ex,6) + LISTES->l_college
            FWRITE(lnHandle,lcBuffer+_ASCII_CR_LF)

           * traitement des candidats
           DBSELECTAR('CANDIDATS')
           DBSEEK(LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF,_ON)
           DO WHILE CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste;
              == LISTES->l_codescru+LISTES->l_college+LISTES->l_numero .AND. !EOF()
              *
              * RECORD TYPE 03 : CANDIDAT
              *
                 lcBuffer := lcNumCant+lcTypeElec;
                             +CANDIDATS->c_numliste;
                             +IIF(CANDIDATS->c_eff_sup==_CODE_EFFECTIF,"1","2");
                             +RIGHT(CANDIDATS->c_numcandi,2);
                             +STRZERO(CANDIDATS->c_nbr_v_ca,6)
              FWRITE(lnHandle,lcBuffer+_ASCII_CR_LF)
              DBSKIP()
           ENDDO
           DBSELECTAREA("LISTES")
           DBSKIP()
        ENDDO
        FCLOSE(lnHandle)
     ENDIF
   NEXT
ENDIF
RESTSCREEN(23,0,24,79,lcSavLig)
SELECT(lnOldSelect)
RETURN(NIL)
**************

FUNCTION jtVerif
LOCAL lnOldSelect:=SELECT(),lnRetMsg:=0,;
      lcNumCant,lnNbrVote,lnNbrVotT,;
      lcSavLig:=SAVESCREEN(23,0,24,79),lcTypeElec,;
      ccLstScrut, lnNoScrut, ccScrutTraite, gnVoteFr, gnVoteNe
PRIVATE lcRun

   DO CASE
     CASE ccOperElec == _TYPE_OPER_LEGISLATIVE
       ccLstScrut := _SCRUTIN_CHAMBRE + _SCRUTIN_SENAT
       lcLesTypes := "CS"
     CASE ccOperElec == _TYPE_OPER_REGION
        IF ccNumArr == _ARROND_BRUXELLES
          ccLstScrut := _SCRUTIN_EUROPEEN + _SCRUTIN_REGIONAL + _SCRUTIN_BXL_REGION_FLA
          lcLesTypes := "ERV"
        ELSE
          ccLstScrut := _SCRUTIN_EUROPEEN + _SCRUTIN_REGIONAL
          lcLesTypes := "ER"
        ENDIF  
     CASE ccOperElec == _TYPE_OPER_4ELEC
       IF ccNumArr == _ARROND_BRUXELLES
          ccLstScrut := _SCRUTIN_EUROPEEN + _SCRUTIN_CHAMBRE + _SCRUTIN_SENAT + _SCRUTIN_REGIONAL + _SCRUTIN_BXL_REGION_FLA
          lcLesTypes := "ECSRV"
       ELSE 
          ccLstScrut := _SCRUTIN_EUROPEEN + _SCRUTIN_CHAMBRE + _SCRUTIN_SENAT + _SCRUTIN_REGIONAL 
          lcLesTypes := "ECSR"
       ENDIF   
   OTHERWISE
       ccLstScrut := ''
       lcLesTypes := "?"
   ENDCASE
 
   FOR lnNoScrut := 1 TO LEN( ccLstScrut)
        ccScrutTraite := SUBSTR( ccLstScrut, lnNoScrut, 1)
        lcTypeElec    := SUBSTR( lcLesTypes, lnNoScrut, 1)

        * compter le nbr de votes
        DBSELECTAREA('BURVOTES')
        DBGOTOP()

        lnNbrVote:=0

        DO WHILE !EOF()
           DO CASE
               CASE ccScrutTraite = _SCRUTIN_EUROPEEN
                  lnNbrVote += BURVOTES->b_nbr_votn +BURVOTES->b_nbr_vote
               CASE ccScrutTraite = _SCRUTIN_REGIONAL
                  lnNbrVote += BURVOTES->b_nbr_votn
               OTHERWISE   
                  lnNbrVote += BURVOTES->b_nbr_votn +BURVOTES->b_nbr_vots
           ENDCASE
           DBSKIP()
        ENDDO

        lnNbrVotT = lnNbrVote
        DBSELECTAREA("SCRUTINS")
        LOCATE FOR s_code == ccScrutTraite
        IF ccNumArr == _ARROND_BRUXELLES .AND. (SCRUTINS->s_code == _SCRUTIN_SENAT .OR. SCRUTINS->s_code == _SCRUTIN_EUROPEEN)
          gnVoteFr := 0
          gnVoteNe := 0
          * traitement des listes
          DBSELECTAREA("LISTES")
          DBGOTOP()
          DBSEEK(SCRUTINS->s_code)
          DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. !EOF()
            IF LISTES->l_college == 'F'
              gnVoteFr := gnVoteFr + LISTES->l_nbr_v_ex
            ELSE
              gnVoteNe := gnVoteNe + LISTES->l_nbr_v_ex
            ENDIF
            DBSELECTAREA("LISTES")
            DBSKIP()
          ENDDO
          lnVoteVal :=  gnVoteFr + gnVoteNe
        ELSEIF SCRUTINS->s_code == _SCRUTIN_BXL_REGION_FLA       
          gnVoteNe := 0
          * traitement des listes
          DBSELECTAREA("LISTES")
          DBGOTOP()
           DBSEEK(_SCRUTIN_BXL_REGION_FLA)
           DO WHILE LISTES->l_codescru == _SCRUTIN_BXL_REGION_FLA .AND. !EOF()
             gnVoteNe := gnVoteNe + LISTES->l_nbr_v_ex
             DBSELECTAREA("LISTES")
             DBSKIP()
           ENDDO
          lnVoteVal := gnVoteNe
        ELSE
          lnVoteVal :=  lnNbrVotT -  (SCRUTINS->s_nbr_vbe+SCRUTINS->s_nbr_vbn)
        ENDIF
        * traitement des listes
        DBSELECTAREA("LISTES")
        DBGOTOP()
        DBSEEK(SCRUTINS->s_code)
        lnSommeListe := 0
        DO WHILE LISTES->l_codescru == SCRUTINS->s_code .AND. !EOF()
           lnSommeListe:=lnSommeListe+LISTES->l_nbr_v_ex
           IF LISTES->l_nbr_v_ex  < LISTES->l_nbr_v_tl + LISTES->l_nbr_v_ef + LISTES->l_nbr_v_sp

              lsErreur = ' L: '+ LISTES->l_numero + SPACE(3)
              lsErreur = lsErreur + 'T'+STRZERO(LISTES->l_nbr_v_ex,6) + " < " + STRZERO(LISTES->l_nbr_v_tl,6) + " + ";
                        +STRZERO(LISTES->l_nbr_v_ef,6) + " + " + STRZERO(LISTES->l_nbr_v_sp,6)
              @ 16,22 SAY lsErreur
              _Message('JT_RESUL219', _TRUE) // Problme de contr“le des  rsultats - Erreur 28 - Niveau 1
           ENDIF
           * traitement des candidats
           DBSELECTAR('CANDIDATS')
           DBSEEK(LISTES->l_codescru+LISTES->l_college+LISTES->l_numero+_CODE_EFFECTIF,_ON)
           DO WHILE CANDIDATS->c_codescru+CANDIDATS->c_college+CANDIDATS->c_numliste;
              == LISTES->l_codescru+LISTES->l_college+LISTES->l_numero .AND. !EOF()
              IF CANDIDATS->c_eff_sup==_CODE_EFFECTIF
                IF CANDIDATS->c_nbr_v_ca > LISTES->l_nbr_v_ex - (LISTES->l_nbr_v_tl + LISTES->l_nbr_v_sp)
                  lsErreur = 'L:'+ LISTES->l_numero + 'C:'+ CANDIDATS->c_numcandi  
                  lsErreur = lsErreur +'E'+ STRZERO(CANDIDATS->c_nbr_v_ca,6) + " > " + STRZERO(LISTES->l_nbr_v_ex,6) + " - ";
                        +STRZERO(LISTES->l_nbr_v_tl,6) + " - " + STRZERO(LISTES->l_nbr_v_sp,6)
                  @ 16,22 SAY lsErreur
                  _Message('JT_RESUL220', _TRUE) // Problme de contr“le des  rsultats - Erreur 28 - Niveau 2
                ENDIF
              ELSE
                IF CANDIDATS->c_nbr_v_ca > LISTES->l_nbr_v_ex - (LISTES->l_nbr_v_tl + LISTES->l_nbr_v_ef)
                  lsErreur = 'L:'+ LISTES->l_numero + 'C:'+ CANDIDATS->c_numcandi  
                  lsErreur = lsErreur +'S'+ STRZERO(CANDIDATS->c_nbr_v_ca,6) + " > " + STRZERO(LISTES->l_nbr_v_ex,6) + " - ";
                        +STRZERO(LISTES->l_nbr_v_tl,6) + " - " + STRZERO(LISTES->l_nbr_v_ef,6)
                  @ 16,22 SAY lsErreur
                  _Message('JT_RESUL220', _TRUE) // Problme de contr“le des  rsultats - Erreur 28 - Niveau 2
                ENDIF
              ENDIF
              DBSKIP()

           ENDDO
           DBSELECTAREA("LISTES")
           DBSKIP()
        ENDDO
        IF lnSommeListe  <> lnVoteVal
           lsErreur = STRZERO(lnSommeListe,6) + " <> " + STRZERO(lnVoteVal,6)
           @ 16,22 SAY SPACE(42)
           @ 16,39 SAY lsErreur + '  Elec. : '+ ccScrutTraite 
           _Message('JT_RESUL221', _TRUE) // Problme de contr“le des  rsultats - Erreur 28 - Niveau 3
        ENDIF
   NEXT
RESTSCREEN(23,0,24,79,lcSavLig)
SELECT(lnOldSelect)
RETURN(NIL)


