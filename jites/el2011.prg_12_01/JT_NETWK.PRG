*********************** FONCTIONS RESEAUX (d‚but) **************************
*-----------------------------------------------------------------------
FUNCTION of_netuse    &&  ****** Ouverture d'une dbf *********
*-----------------------------------------------------------------------
  PARAMETER ;
              pc_nomdbf,; //  nom de la dbf … ouvrir
              pl_modexc,; //  .t. mode exclusif / .f. mode partag‚
              pn_attend,; //  essai d'ouverture en seconde (0 = infini)
              pc_alias        && nom de l'ALIAS

  PRIVATE ;
              tl_infini,;
              tn_attend,;
              tc_alias, ;
              te_e1

  te_e1 = SAVESCREEN(0,0 , 1,79)

  tc_alias = IIF( PCOUNT() < 4 , pc_nomdbf, pc_alias)
  tl_infini = ( pn_attend = 0 )
  tn_attend = pn_attend       

  DO WHILE (tl_infini .OR. tn_attend > 0)

    IF pl_modexc
      USE &pc_nomdbf EXCLUSIVE ALIAS &tc_alias
    ELSE
      USE &pc_nomdbf ALIAS &tc_alias
    ENDIF

    IF .NOT. NETERR()
      RESTSCREEN(0, 0, 1, 79, te_e1)
      RETURN (.t.)
    ENDIF

    @ 0 ,0 say '*** Fichier occup‚ ['+pc_nomdbf+'] ***'

    INKEY(1) && attend une seconde
    tn_attend = tn_attend - 1

  ENDDO
  RESTSCREEN(0, 0, 1, 79, te_e1)
RETURN (.f.)

*-----------------------------------------------------------------------
FUNCTION of_reclock    && ********** blocage d'un record ************
*-----------------------------------------------------------------------
  PARAMETER ;
              pn_attend       && essai de blocage en seconde (0 = infini )

  PRIVATE  ;
              tl_infini,;
              tn_attend,;
              te_e1

  IF RLOCK()
    RETURN (.t.)    && record bloqu‚
  ENDIF

  te_e1 = SAVESCREEN(0,0 , 1,79)
  @ 0,0 say '*** Fiche occup‚e ['+trim(dbf(select()))+' rec='+TRANSFORM(recno(),'999999')+'] ***'

  tl_infini = ( pn_attend = 0 )
  tn_attend = pn_attend

  DO WHILE (tl_infini .OR. tn_attend > 0)

    INKEY(.5) && attend 1/2 seconde
    tn_attend = tn_attend - .5

    IF RLOCK()
      RESTSCREEN(0, 0, 1, 79, te_e1)
      RETURN (.t.)   && record bloqu‚
    ENDIF

  ENDDO
  RESTSCREEN(0, 0, 1, 79, te_e1)
RETURN (.f.)

*-----------------------------------------------------------------------
FUNCTION of_addrec    && ********** APPEND BLANK RESEAU ************
*-----------------------------------------------------------------------
  PARAMETER ;
              pn_attend       && essai APPEND BLANK en seconde (0 = infini )

  PRIVATE  ;
              tl_infini,;
              tn_attend,;
              te_e1

  APPEND BLANK
  IF .NOT. NETERR()
    RETURN (.t.)  && ok APPEND BLANK pass‚
  ENDIF

  te_e1 = SAVESCREEN(0,0 , 1,79)
  @ 0,0 say '*** Ajout occup‚ ['+trim(dbf(select()))+' rec='+TRANSFORM(reccount()+1,'999999')+'] ***'

  tl_infini = ( pn_attend = 0 )
  tn_attend = pn_attend

  DO WHILE (tl_infini .OR. tn_attend > 0)

    APPEND BLANK
    IF .NOT. NETERR()
      RESTSCREEN(0, 0, 1, 79, te_e1)
      RETURN (.t.)  && ok APPEND BLANK pass‚
    ENDIF

    INKEY(.5)
    tn_attend = tn_attend - .5

  ENDDO

  RESTSCREEN(0, 0, 1, 79, te_e1)
RETURN (.f.)
*********************** FONCTIONS RESEAUX (fin  ) **************************

