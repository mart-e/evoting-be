/* Filename        : VARGENER.I                 */

/************************************************************************/
/*                                                                      */
/*    SHARED VARIABLES                                                  */
/*                                                                      */
/************************************************************************/

/***********************************************************/
/* Gebruikte variabelen voor user interface van prp        */
/*                                                         */
/*      men_top : De horizontale menu bar van PMGENEME.    */
/*      menupos : De positie in het hoofdmenu PMGENEME.    */
/*      execopt : Uitvoeren van de optie in hoofdmenu,     */
/*                zonder een CHOOSE te doen.               */
/*      menleav : De toets F4 is gebruikt in een submenu.  */
/***********************************************************/

DEFINE {1} SHARED VARIABLE men_top AS CHARACTER EXTENT 5 FORMAT "X(13)".

DEFINE {1} SHARED VARIABLE menupos AS INTEGER  INITIAL ?.
DEFINE {1} SHARED VARIABLE execopt AS INTEGER  INITIAL ?.
DEFINE {1} SHARED VARIABLE menleav AS INTEGER  INITIAL ?.
