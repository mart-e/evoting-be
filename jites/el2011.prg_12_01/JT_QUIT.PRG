/*旼컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴커
  � Fonction pour quitter le programme JITES           �
  읕컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴켸
*/
  #include 'inkey.ch'
  #include "box.ch"
  #include "error.ch"
  #include "set.ch"          // Defined by Clipper
  #include 'jites.ch'

  FUNCTION _QuitJt
    PRIVATE ln_rep,lc_winbuff,lc_winbuff2,lc_color
    lc_winbuff=SAVESCREEN(14,23,18,54)
    lc_winbuff2=SAVESCREEN(23,00,24,79)
    lc_color=SETCOLOR()
    SET COLOR TO _COLOR_MENU
    @ 23,01 CLEAR TO 24,78
    SET COLOR TO W+/R
    @ 14,24,18,55 BOX B_DOUBLE + ' '
    @ 15,26 SAY _rechmsg('JT_QUIT001')  // D굎irez-vous quitter JITES ?
    @ 17,26 SAY _rechmsg('JT_QUIT002')  //"     [O]ui        [N]on     "
    ln_rep=0
    DO WHILE ln_rep!=ASC("O") .AND. ln_rep!=ASC("N")
      ln_rep=ASC(UPPER(CHR(INKEY(0))))
    ENDDO
    ln_rep=IF(ln_rep=ASC("O"),0,1)
    SET COLOR TO &lc_color
    RESTSCREEN(14,24,18,55,lc_winbuff)
    RESTSCREEN(23,00,24,79,lc_winbuff2)
  RETURN ln_rep
