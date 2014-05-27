  #include "inkey.ch"
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include "jites.ch"


FUNCTION liste
PARAMETERS pn_lig_deb,;   && coordonn‚e lig. d‚but
           pn_col_deb,;   &&            col. d‚but
           pn_lig_fin,;   &&            lig. fin
           pn_col_fin,;   &&            col. fin
           pc_aff_lig,;   && proc‚dure affichage ligne
           pu_born_inf,;  && borne inf‚rieure
           pu_born_sup,;  && borne sup‚rieure
           pc_user_fct,;  && fct utilisateur appell‚e pour traiter les key exception
           pc_lig_vide,;  && ligne vide
           pc_key_quit,;  && codes ASCII pour sortir de liste
           pa_param_uf,;  && paramŠtres … envoyer … la fct user
           pc_cle_deb     && cl‚ d‚but dans la s‚lection

PRIVATE ln_return,;       && valeur retourn‚e par la fct
        ln_ret_udf,;      && valeur retourn‚e par la fct user
        ll_softseek,;     && valeur du softseek avant l'entr‚e dans liste
        gn_buffer,;       && buffer pour la recherche partielle
        gc_ecrbuf,;       && ‚cran o— est affich‚ le buffer
        gn_ligactive,;    && nø de ligne active
        gn_der_lig,;      && nø de derniŠre ligne occup‚e
        gu_cleindex,;     && cl‚ de l'index
        gc_crit_inf,;     && Ú str qui contient le test pour ne pas d‚passer
        gc_crit_sup,;     && À la borne inf‚rieure/sup‚rieure
        gb_usr_fct        && code bloc pour ex‚cution de la fct user
***        gb_aff_lig:={|gc_color,gn_numlig|&pc_aff_lig(gc_color,gn_numlig)}

PRIVATE ln_sortie

* mettre … TRUE le softseek
ll_softseek:=SET( _SET_SOFTSEEK , _TRUE)

* analyse et initialisation des paramŠtres
pn_lig_deb := IIF(pn_lig_deb==NIL,1,pn_lig_deb)
pn_col_deb := IIF(pn_col_deb==NIL,0,pn_col_deb)
pn_lig_fin := IIF(pn_lig_fin==NIL,24,pn_lig_fin)
pn_col_fin := IIF(pn_col_fin==NIL,79,pn_col_fin)

* gestion cl‚ de l'index
gu_cleindex := INDEXKEY(INDEXORD())
* sauvegarde la ligne o— se place le gc_buffer
IF TYPE(gu_cleindex) == "C"
   gc_ecrbuf := SAVESCREEN(pn_lig_deb-1,pn_col_deb,pn_lig_deb-1;
                          ,pn_col_deb+LEN(&gu_cleindex)-1)
ENDIF

* d‚finition des critŠres d'appartenance
pu_born_inf := IIF(pu_born_inf==NIL,"",pu_born_inf)
pu_born_sup := IIF(pu_born_sup==NIL,"",pu_born_sup)
gc_crit_inf := IIF(EMPTY(pu_born_inf),"(EOF() .OR. BOF())";
                  ,"(&gu_cleindex" + "<pu_born_inf)")
gc_crit_sup := IIF(EMPTY(pu_born_sup),"(.NOT. EOF() .AND. .NOT. BOF())";
                       ,"(&gu_cleindex" + "<=pu_born_sup)")

* cr‚er et initialiser le tableau des paramŠtres
* le 1er ‚l‚ment contiendra le nø de la derniŠre ligne occup‚e OU -1 si
* pas de record
IF pa_param_uf == NIL
   pa_param_uf := {NIL}
ELSE
   AADD(pa_param_uf,NIL)
   AINS(pa_param_uf,1)
ENDIF
pc_user_fct = IIF(pc_user_fct==NIL,"",pc_user_fct+"(@pa_param_u)")
pc_key_quit := IIF(pc_key_quit==NIL .OR. EMPTY(pc_key_quit),"[27][13]",pc_key_quit)
pc_lig_vide := IIF(pc_lig_vide==NIL,SPACE(pn_col_fin-pn_col_deb+1),pc_lig_vide)


gn_ligactive := pn_lig_deb
gn_der_lig := pn_lig_fin
gc_buffer := ""


* Afficher le 1er ‚cran
IF .NOT. EMPTY(pu_born_inf)
   * Afficher … partir du record appartenant … la s‚lection
   IF pc_cle_deb == NIL
      DO va_debut
      DO aff_page   && afficher 1Šre page
   ELSE
      SEEK pc_cle_deb   && se positionner sur cle_deb
      IF EOF() .OR. .NOT. &gc_crit_sup   && cl‚ sup‚rieure au dernier de la s‚lection
         SKIP -1
      ENDIF
      DO majDerLig
      pa_param_uf[1] := IIF(gn_der_lig>=pn_lig_deb,gn_der_lig,-1)
      DO va1depage
   ENDIF
ELSE
   * afficher … partir du record ACTIF (quand pas de s‚lection)
   DO majDerLig
   pa_param_uf[1] := IIF(gn_der_lig>=pn_lig_deb,gn_der_lig,-1)
   DO va1depage
ENDIF


* gestion de la liste
ln_sortie = 1
DO WHILE ln_sortie <> 0

   _waitkey()

   * gn_derlig>=pn_lig_deb SINON signifie pas de records
   * pa_param_uf[1] est une variable pour l'utilisateur.
   *                Vaut gn_der_lig si records
   *                sinon vaut -1
   pa_param_uf[1] := IIF(gn_der_lig>=pn_lig_deb,gn_der_lig,-1)

   IF LASTKEY()=K_UP .AND. pa_param_uf[1]<>-1
      DO ligne_up
   ELSEIF LASTKEY()=K_DOWN .AND. pa_param_uf[1]<>-1
      DO ligne_dn
   ELSEIF LASTKEY()=K_PGUP .AND. pa_param_uf[1]<>-1
      DO page_up
   ELSEIF LASTKEY()=K_PGDN .AND. pa_param_uf[1]<>-1
      DO page_dn
   ELSEIF LASTKEY()=K_HOME .AND. pa_param_uf[1]<>-1
      DO aff_home
   ELSEIF LASTKEY()=K_END .AND. pa_param_uf[1]<>-1
      DO aff_end
   ELSEIF "["+ALLTRIM(STR(LASTKEY()))+"]" $ pc_key_quit
      ln_sortie = 0
   ELSE
      IF .NOT. EMPTY(pc_user_fct)
         SET(_SET_SOFTSEEK,ll_softseek)

           ln_ret_udf := &pc_user_fct

         SET( _SET_SOFTSEEK, _TRUE)

         DO refresh WITH ln_ret_udf

      ENDIF
   ENDIF

ENDDO
ln_return = 1

SET(_SET_SOFTSEEK,ll_softseek)   && restaurer l'ancienne valeur

RETURN ln_return


*************************************************
* Maj de la variable gn_der_lig
*************************************************
PROCEDURE majDerLig
PRIVATE ln_recnum,ln_compt
ln_recnum = RECNO()
DO va_debut
ln_compt = pn_lig_deb
DO WHILE .NOT. EOF() .AND. &gc_crit_sup .AND. ln_compt<=pn_lig_fin
   SKIP
   ln_compt = ln_compt + 1
ENDDO
gn_der_lig = ln_compt - 1
GO ln_recnum
RETURN


*************************************************
* refresh ‚cran suivant le return de la fct user
*************************************************
PROCEDURE refresh
PARAMETER pn_ret_udf
IF pn_ret_udf = LI_EXIT           && sortir de liste             S/
   ln_sortie = 0
ELSEIF pn_ret_udf = LI_1DEPAGE    && 1er de la page              A/C/G
   DO va1depage
ELSEIF pn_ret_udf = LI_MAJLIG     && refresh ligne active        M/
   DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
ELSEIF pn_ret_udf = LI_REFRESH    && refresh complet             D/E/
   DO totrefresh
ELSEIF pn_ret_udf = LI_RESTPAGE   && afficher reste de la page   Ins
   DO restpage                  
ELSEIF pn_ret_udf = LI_NOTHING    && ne rien faire
ENDIF
RETURN


*************************************************
* affichage d'une page
*************************************************
PROCEDURE aff_page
PARAMETER pl_affreverse   && afficher en inverse
PRIVATE ln_numrec

pl_affreverse = IIF(TYPE("pl_affreverse")="U",.T.,.F.)

gn_ligactive = pn_lig_deb

ln_numrec = RECNO()
DO WHILE .NOT. EOF() .AND. &gc_crit_sup .AND. gn_ligactive<=pn_lig_fin
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   SKIP
   gn_ligactive = gn_ligactive + 1
ENDDO

* re‡oit le dernier nø de ligne occup‚e
gn_der_lig = gn_ligactive - 1

IF gn_der_lig >= pn_lig_deb   && sinon plus rien … afficher
   GO ln_numrec
   gn_ligactive = pn_lig_deb
   IF pl_affreverse
       DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
   ENDIF
ENDIF
RETURN


*************************************************
* faire un SEEK ou un GO TOP
*************************************************
PROCEDURE va_debut
IF EMPTY(pu_born_inf)
   GO TOP
ELSE
   SEEK pu_born_inf
ENDIF
RETURN


*************************************************
* se positionner sur la ligne suivante
*************************************************
PROCEDURE ligne_dn
PRIVATE ln_numrec

DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb

ln_numrec = RECNO()
SKIP
IF EOF() .OR. .NOT. &gc_crit_sup
   GO ln_numrec
ELSE
   IF gn_ligactive = pn_lig_fin
      SCROLL(pn_lig_deb,pn_col_deb,pn_lig_fin,pn_col_fin,1)
   ELSE
      gn_ligactive = gn_ligactive + 1
   ENDIF
ENDIF
DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
RETURN


*************************************************
* se positionner sur la ligne pr‚c‚dente
*************************************************
PROCEDURE ligne_up
PRIVATE ln_numrec

DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb

ln_numrec = RECNO()
SKIP -1
IF BOF() .OR. &gc_crit_inf
   GO ln_numrec
ELSE
   IF gn_ligactive = pn_lig_deb
      SCROLL(pn_lig_deb,pn_col_deb,pn_lig_fin,pn_col_fin,-1)
   ELSE
      gn_ligactive = gn_ligactive - 1
   ENDIF
ENDIF
DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
RETURN


*************************************************
* se positionner sur la page suivante
*************************************************
PROCEDURE page_dn
PRIVATE ln_numrec,ln_oldligactive,ln_newrec,ln_posi1er,ln_last_rec

IF gn_der_lig < pn_lig_fin   && qu'une seule page
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   DO va_debut
   * se positionner sur le dernier record
   SKIP gn_der_lig-pn_lig_deb
   gn_ligactive = gn_der_lig
   DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
ELSE
   ln_numrec = RECNO()
   SKIP pn_lig_fin-pn_lig_deb+1
   IF EOF() .OR. .NOT. &gc_crit_sup
      * compter le nbr de record … partir de l'actif (les derniers du filtre)
      ln_last_rec = aff_last(ln_numrec)
   ELSE
      ln_newrec = RECNO()
      * se positionner sur le 1er de la page suivante
      * car on est sur l'actif de la page suivante
      ln_oldligactive = gn_ligactive
      SKIP -(gn_ligactive-pn_lig_deb)
      ln_posi1er = RECNO()
      SKIP pn_lig_fin-pn_lig_deb+1
      IF EOF() .OR. .NOT. &gc_crit_sup
         * on arrive … la derniŠre page mais on n'a pas assez de records
         * => utiliser des pr‚c‚dents
         ln_last_rec = aff_last(ln_posi1er)
      ELSE
         GO ln_posi1er
         * afficher nouvelle page
         DO aff_page WITH .F.
         * mettre en reverse l'enregistrement actif
         gn_ligactive = ln_oldligactive
         GO ln_newrec
         DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
      ENDIF
   ENDIF
ENDIF

RETURN


*************************************************
* afficher la derniŠre page
*************************************************
FUNCTION aff_last
PARAMETER pn_posi
PRIVATE ln_last_rec
GO pn_posi
DO WHILE .NOT. EOF() .AND. &gc_crit_sup
   SKIP
ENDDO
SKIP -1
ln_last_rec = RECNO()   && MAJ du nø du dernier record
* se mettre sur le record d‚but de page
SKIP -(pn_lig_fin-pn_lig_deb)
DO aff_page WITH .F.   && afficher la page
* mettre en reverse le dernier record
gn_ligactive = pn_lig_fin
GO ln_last_rec
DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
RETURN ln_last_rec


*************************************************
* se positionner sur la page pr‚c‚dente
*************************************************
PROCEDURE page_up
PRIVATE ln_numrec,ln_newrec,ln_oldligactive,ll_debut,ln_rec_deb

ln_numrec = RECNO()
ll_debut = .F.
SKIP -(pn_lig_fin-pn_lig_deb+1)
ln_newrec = RECNO()
IF BOF() .OR. &gc_crit_inf
   * tester si le premier de la liste affich‚ est le v‚ritable 1er
   DO va_debut
   ln_rec_deb = RECNO()
   GO ln_numrec
   SKIP -(gn_ligactive-pn_lig_deb)
   IF RECNO() = ln_rec_deb
      ll_debut = .T.
   ENDIF
ENDIF

IF ll_debut
   GO ln_numrec
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   DO va_debut
   gn_ligactive = pn_lig_deb
   DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
ELSE
   GO ln_newrec
   * se positionner sur le 1er de la page pr‚c‚dente
   * car on est sur l'actif de la page suivante
   SKIP -(gn_ligactive-pn_lig_deb)
   IF BOF() .OR. &gc_crit_inf
      DO va_debut
      DO aff_page
   ELSE
      ln_oldligactive = gn_ligactive
      DO aff_page WITH .F.
      * mettre en reverse l'enregistrement actif
      gn_ligactive = ln_oldligactive
      GO ln_newrec
      DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
   ENDIF
ENDIF
RETURN


*************************************************
* se positionner d‚but du fichier
*************************************************
PROCEDURE aff_home
DO va_debut
DO aff_page
RETURN


*************************************************
* se positionner fin du fichier
*************************************************
PROCEDURE aff_end
PRIVATE ln_last_rec,ln_numrec
IF gn_der_lig < pn_lig_fin   && qu'une seule page
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   DO va_debut
   * se positionner sur le dernier record
   SKIP gn_der_lig-pn_lig_deb
   gn_ligactive = gn_der_lig
   DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
ELSE
   * se positionner sur le dernier record
   IF EMPTY(pu_born_sup)
      GO BOTTOM
   ELSE
      SEEK pu_born_sup
   ENDIF
   ln_numrec = RECNO()
   ln_last_rec = aff_last(ln_numrec)
ENDIF
RETURN


*************************************************
* afficher la page … partir du record actif
*************************************************
PROCEDURE va1depage
PRIVATE ln_numrec,ln_nbrrec
ln_numrec = RECNO()
IF gn_der_lig < pn_lig_fin   && qu'une seule page

   DO aff_va1 WITH ln_numrec

ELSE
   SKIP pn_lig_fin-pn_lig_deb
   IF EOF() .OR. .NOT. &gc_crit_sup
      GO ln_numrec
      ln_nbrrec = 0
      DO WHILE .NOT. EOF() .AND. &gc_crit_sup
         ln_nbrrec = ln_nbrrec + 1
         SKIP
      ENDDO
      * se positionner sur le 1er de la page
      SKIP -(pn_lig_fin-pn_lig_deb+1)
      DO aff_page WITH .F.
      GO ln_numrec
      gn_ligactive = pn_lig_fin-ln_nbrrec+1
      DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
   ELSE
      GO ln_numrec
      DO aff_page
   ENDIF
ENDIF
RETURN


*************************************************
* afficher 1er page pour va1depage
*************************************************
PROCEDURE aff_va1
PARAMETER pn_numrec   && nø de record … rendre actif

PRIVATE ln_posi

ln_posi = pn_lig_deb
DO va_debut
* afficher la page (permet de supprimer l'ancien actif et trouver nouveau)
gn_ligactive = pn_lig_deb
DO WHILE .NOT. EOF() .AND. &gc_crit_sup .AND. gn_ligactive<=pn_lig_fin
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   IF RECNO() = pn_numrec   && sauver le nø de ligne du record … s‚lectionner
      ln_posi = gn_ligactive
   ENDIF
   gn_ligactive = gn_ligactive + 1
   SKIP
ENDDO
gn_der_lig = gn_ligactive - 1
GO pn_numrec
gn_ligactive = ln_posi
IF pa_param_uf[1]<>-1
   DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
ENDIF
RETURN


*************************************************
* faire un refresh de la page
*************************************************
PROCEDURE totrefresh
PRIVATE ln_numrec,ln_posi1er,ln_oldligactive,ln_oldderlig
***SKIP +1
***SKIP -1
ln_oldderlig = gn_der_lig
IF BOF() .OR. &gc_crit_inf
   DO aff_home
ELSE
   ln_numrec = RECNO()
   SKIP -(gn_ligactive-pn_lig_deb)
   IF BOF() .OR. &gc_crit_inf
      DO aff_va1 WITH ln_numrec
   ELSE
      ln_posi1er = RECNO()
      GO ln_numrec
      SKIP (pn_lig_fin-gn_ligactive)
      IF EOF() .OR. .NOT. &gc_crit_sup
         GO ln_numrec
         DO aff_end
      ELSE
         GO ln_posi1er
         ln_oldligactive = gn_ligactive
         DO aff_page WITH .F.
         GO ln_numrec
         gn_ligactive = ln_oldligactive
         DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
      ENDIF
   ENDIF
ENDIF
***IF ln_oldderlig<>gn_der_lig
   FOR ln_i = gn_der_lig+1 TO pn_lig_fin
       @ ln_i,pn_col_deb SAY pc_lig_vide
   NEXT ln_i
   @ gn_ligactive,pn_col_deb SAY ""   && se repositionner sur la ligne active
***ENDIF
RETURN


*************************************************
* faire un refresh de la page … partir de la
* ligne active
*************************************************
PROCEDURE restpage
PRIVATE ln_numrec,ln_oldligactive

ln_numrec = RECNO()
ln_oldligactive = gn_ligactive
DO WHILE .NOT. EOF() .AND. &gc_crit_sup .AND. gn_ligactive<=pn_lig_fin
   DO &pc_aff_lig WITH LI_NORMAL,gn_ligactive,pn_col_deb
   SKIP
   gn_ligactive = gn_ligactive + 1
ENDDO

* re‡oit le dernier nø de ligne occup‚e
gn_der_lig = gn_ligactive - 1

GO ln_numrec
gn_ligactive = ln_oldligactive
DO &pc_aff_lig WITH LI_REVERSE,gn_ligactive,pn_col_deb
RETURN


*************************************************
* FONCTION rechpart
* recherche partielle
* Ne marche que pour les index caractŠres !!!
*************************************************
FUNCTION rechpart

LOCAL ln_numrec:=RECNO(),;
      ln_return:=LI_NOTHING

IF TYPE(gu_cleindex) == "C"
   IF LASTKEY()==K_BS
      gc_buffer := LEFT(gc_buffer,LEN(gc_buffer)-1)
   ELSE
      IF LEN(gc_buffer) < LEN(&gu_cleindex) .AND. LASTKEY() >= 31
         gc_buffer := gc_buffer + UPPER(CHR(LASTKEY()))
      ENDIF
   ENDIF

   DBSEEK(gc_buffer,_ON)
   IF EOF() .OR. LEFT(&gu_cleindex,LEN(gc_buffer)) <> gc_buffer
      _beep()
      GO ln_numrec
      gc_buffer := LEFT(gc_buffer,LEN(gc_buffer)-1)
   ELSE
      IF ln_numrec <> RECNO()   && pas faire un refresh si on est tjs sur le mˆme record
         ln_return := LI_1DEPAGE
      ENDIF
   ENDIF
   RESTSCREEN(pn_lig_deb-1,pn_col_deb,pn_lig_deb-1;
              ,pn_col_deb+LEN(&gu_cleindex)-1,gc_ecrbuf)
   @ pn_lig_deb-1,pn_col_deb SAY gc_buffer COLOR "N/W"
ENDIF

RETURN(ln_return)
