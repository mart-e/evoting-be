#include "set.ch"
#include "inkey.ch"
#include "setcurs.ch"
#include "jites.ch"

FUNCTION _use(pcFile,;        && code du fichier
              pcPathData ,;   && Path du fichier
              pnNumSelect,;   && nø de select o— doit ˆtre ouvert le fichier
              plMode,;        && TRUE = exclusif, FALSE = shared
              pcAlias)        && nom de l'alias
  LOCAL lcFile

  * s‚lection de la select
  SELECT(IIF(pnNumSelect ==NIL, 0 , pnNumSelect))
  plMode     := IIF(plMode    ==NIL, _SHARED        , plMode)
  pcFile     := LOWER(pcFile)
  pcPathData := IIF(pcPathData==NIL, _PATH_JITES+"\", pcPathData)
  pcAlias    := IIF(pcAlias   ==NIL, pcFile         , pcAlias)

* ouvrir le fichier
  of_netuse( pcPathData + pcFile, plMode, 0, pcAlias)

* ouvrir ses index
IF pcFile=="jt_msg"
   IF .NOT. FILE(pcPathData+"jt_msg.ntx" )
     lcFile :=  pcPathData + 'jt_msg'
     of_netuse( pcPathData + pcFile, _TRUE, 0, pcAlias)
     INDEX ON msg_numero TO &lcFile
     of_netuse( pcPathData + pcFile, _FALSE, 0, pcAlias)
   ENDIF
   * fichier des tables
   DBSETINDEX( pcPathData + "jt_msg" )
ELSEIF pcFile=="jites_bak"
   * fichier des mots de passe
ELSEIF pcFile=="elect"
   * fichier des ‚lections
   if .NOT. FILE(pcPathData+"elect.ntx" )
     lcFile :=  pcPathData + 'elect'
     of_netuse( pcPathData + pcFile, _TRUE, 0, pcAlias)
     INDEX ON right(elec_dir,6)+IIF(left(elec_dir,2)=='EL','ZZ',left(elec_dir,2)) TO &lcFile
     of_netuse( pcPathData + pcFile, _FALSE, 0, pcAlias)
   ENDIF
   DBSETINDEX( pcPathData + "elect" )
*  ? '#'+UPPER(indexkey(1))+'#'
*  wait
   if UPPER(indexkey(1)) == 'ELEC_DIR'
     lcFile :=  pcPathData + 'elect'
     of_netuse( pcPathData + pcFile, _TRUE, 0, pcAlias)
     INDEX ON right(elec_dir,6)+IIF(left(elec_dir,2)=='EL','ZZ',left(elec_dir,2)) TO &lcFile
     of_netuse( pcPathData + pcFile, _FALSE, 0, pcAlias)
     DBSETINDEX( pcPathData + "elect" )
   endif
ELSEIF LEFT(pcFile,2) == "bv"
   * fichier bureaux de votes
   DBSETINDEX( pcPathData + pcFile )
ELSEIF LEFT(pcFile,2) == "cd"
   * fichier candidats
   DBSETINDEX( pcPathData + pcFile )
ELSEIF LEFT(pcFile,2) == "li"
   * fichier listes
   DBSETINDEX( pcPathData + pcFile )
ELSEIF LEFT(pcFile,2) == "rv"
   * fichier r‚cup‚ration des votes
   DBSETINDEX( pcPathData + pcFile )
ELSEIF LEFT(pcFile,2) == "sc"
   * fichier des scrutins
   DBSETINDEX( pcPathData + pcFile )
ENDIF

RETURN(NIL)


*************************************************
* FUNCTION _useTmp
* ouvrir les fichiers TMP
* (r‚pertoire de travail mcPathTmp
*************************************************
FUNCTION _useTmp(pcFile,;        && code du fichier
                 pnNumSelect,;   && nø de select o— doit ˆtre ouvert le fichier
                 plMode,;        && TRUE = exclusif, FALSE = shared
                 pcAlias)        && nom de l'alias

* s‚lection de la select
SELECT(IIF(pnNumSelect==NIL,0,pnNumSelect))
plMode:=IIF(plMode==NIL,_SHARED,plMode)
pcFile:=LOWER(pcFile)

* ouvrir le fichier
of_netuse(mcPathTmp+pcFile,plMode,0,IIF(pcAlias==NIL,pcFile,pcAlias))

RETURN(NIL)



FUNCTION VerifNtx
  PARAMETER pcModeIndex      //  R=index inexistant   A=r‚index de tout

  LOCAL lcFile

  * CONTROLE DE STRUCTURE DES DBF DE L'ELECTION
  of_ctrldbf( _PATH_ELECTION + '\pa' + RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\param'   + '.dbf' )
  of_ctrldbf( _PATH_ELECTION + '\bv'+  RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\burvote' + '.dbf' )
  of_ctrldbf( _PATH_ELECTION + '\sc'+  RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\scrutins'+ '.dbf' )
  of_ctrldbf( _PATH_ELECTION + '\li'+  RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\listes'  + '.dbf' )
  of_ctrldbf( _PATH_ELECTION + '\cd'+  RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\candidat'+ '.dbf' )
  of_ctrldbf( _PATH_ELECTION + '\rv'+  RIGHT(ccElecActive,6) + '.dbf'  ,  _PATH_STRU_BAS + '\votes'   + '.dbf' )

  select(0)
  of_netuse( _PATH_ELECTION + '\pa' + RIGHT(ccElecActive,6) , _SHARED, 0, 'PARAM')
  IF EOF()
    of_addrec(0)
    REPLACE p_statut WITH "D‚but"
    UNLOCK
  ENDIF
  cdDate       = p_DateElec
  ccArron      = IIF( EMPTY( ccArron) , p_Arron, ccArron )
  ccDistrict   = p_District
  ccBureau     = p_Burdep
  ccResponsable= p_Reponsab
  cnNbrBur     = p_NbrBur
  ccNumCan     = p_NumCan
  ccStatut     = LEFT(p_Statut,8)
  ccEtat       = LEFT(p_Statut,1)
  USE

  * TEST indexation
  IF pcModeIndex = _INDEX_ALL_FILE   && R‚index tout

     select(0)  // Bureau de votes
     lcFile    := _PATH_ELECTION + '\bv'+ RIGHT(ccElecActive,6)
     of_netuse( lcFile, _EXCLUSIVE, 0, 'BURVOTE')
     INDEX ON B_NUMERO TO &lcFile
     USE

     select(0)  // fichier scrutins
     lcFile    := _PATH_ELECTION + '\sc'+ RIGHT(ccElecActive,6)
     of_netuse( lcFile, _EXCLUSIVE, 0, 'SCRUTIN')
     INDEX ON S_CODE TO &lcFile
     USE

     select(0)  // fichier des listes
     lcFile    := _PATH_ELECTION + '\li'+ RIGHT(ccElecActive,6)
     of_netuse( lcFile, _EXCLUSIVE, 0, 'LISTES')
     INDEX ON L_CODESCRU+L_COLLEGE+L_NUMERO TO &lcFile
     USE

     select(0)  // fichier candidats
     lcFile    := _PATH_ELECTION + '\cd'+ RIGHT(ccElecActive,6)
     of_netuse( lcFile, _EXCLUSIVE, 0, 'CANDIDATS')
     INDEX ON C_CODESCRU+C_COLLEGE+C_NUMLISTE+C_EFF_SUP+C_NUMCANDI TO &lcFile
     USE

     select(0)  // fichier r‚cup‚ration des votes
     lcFile    := _PATH_ELECTION + '\rv'+ RIGHT(ccElecActive,6)
     of_netuse( lcFile, _EXCLUSIVE, 0, 'RECUPVOT')
     pack
     INDEX ON STRZERO(RV_NUMBUR)+RV_CODESCR+RV_COLLEGE+RV_NUMLIST;
              +RV_EFF_SUP+RV_NUMCAND TO &lcFile
     USE

  ELSE   && r‚indexer uniquement les fichiers supprim‚s

     lcFile    := _PATH_ELECTION + '\bv'+ RIGHT(ccElecActive,6)
     IF .NOT. FILE(lcFile+".ntx")
       select(0)  // Bureau de votes
       of_netuse( lcFile, _EXCLUSIVE, 0, 'BURVOTE')
       INDEX ON B_NUMERO TO &lcFile
     ENDIF

     lcFile    := _PATH_ELECTION + '\sc'+ RIGHT(ccElecActive,6)
     IF .NOT. FILE(lcFile+".ntx")
       select(0)  // fichier scrutins
       of_netuse( lcFile, _EXCLUSIVE, 0, 'SCRUTIN')
       INDEX ON S_CODE TO &lcFile
       USE
     ENDIF

     lcFile    := _PATH_ELECTION + '\li'+ RIGHT(ccElecActive,6)
     IF .NOT. FILE(lcFile+".ntx")
       select(0)  // fichier des listes
       of_netuse( lcFile, _EXCLUSIVE, 0, 'LISTES')
       INDEX ON L_CODESCRU+L_COLLEGE+L_NUMERO TO &lcFile
       USE
     ENDIF

     lcFile    := _PATH_ELECTION + '\cd'+ RIGHT(ccElecActive,6)
     IF .NOT. FILE(lcFile+".ntx")
       select(0)  // fichier candidats
       of_netuse( lcFile, _EXCLUSIVE, 0, 'CANDIDATS')
       INDEX ON C_CODESCRU+C_COLLEGE+C_NUMLISTE+C_EFF_SUP+C_NUMCANDI TO &lcFile
       USE
     ENDIF

     lcFile    := _PATH_ELECTION + '\rv'+ RIGHT(ccElecActive,6)
     IF .NOT. FILE(lcFile+".ntx")
       select(0)  // fichier r‚cup‚ration des votes
       of_netuse( lcFile, _EXCLUSIVE, 0, 'RECUPVOT')
       INDEX ON STRZERO(RV_NUMBUR)+RV_CODESCR+RV_COLLEGE+RV_NUMLIST;
                +RV_EFF_SUP+RV_NUMCAND TO &lcFile
       USE
     ENDIF
  ENDIF
RETURN(NIL)
