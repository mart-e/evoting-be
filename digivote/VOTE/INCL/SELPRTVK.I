/* Filename            : SELPRTVK.I                                     */

/************************************************************************/
/*                                  */
/*    SHARED VARIABLES                      */
/*                                  */
/************************************************************************/

/***********************************************************/
/* Gebruikte variabelen voor user interface van prp        */
/*                                                         */
/*   selexecopt : Uitvoeren van de optie in hoofdmenu,     */
/*                zonder een CHOOSE te doen.               */
/*   selmenleav : De toets F4 is gebruikt in een submenu.  */
/***********************************************************/

DEFINE {1} SHARED VARIABLE selexecopt AS INTEGER  INITIAL ?.
DEFINE {1} SHARED VARIABLE selmenleav AS INTEGER  INITIAL ?.
