/* Filename            : SELVAR.I                                       */

/************************************************************************/
/*                                                                      */
/*        SHARED VARS                                                   */
/*                                                                      */
/************************************************************************/

/******************************************************/
/*      verkdat   : De verkiezings datum              */
/*      taalkode  : De taalkode voor de schermen      */
/*      verknum   : Het verkiezingsvolgnummer         */
/*      verkben   : De benaming van de verkiezing     */
/*      partnum   : Het partijnummer                  */
/*      partnam   : De partijnaam.                    */
/*      candtyp   : De kandidaat type aanduiding      */
/*      candnum   : Het kandidaatnummer               */
/******************************************************/

DEF {1} SHARED VAR verkdat  LIKE elect.selection.s-id             NO-UNDO.
DEF {1} SHARED VAR taalkode LIKE elect.selection.lang-pc          NO-UNDO.
DEF {1} SHARED VAR verknum  LIKE elect.selection.e-id             NO-UNDO.
DEF {1} SHARED VAR verkcol  LIKE elect.selection.coll-id          NO-UNDO.
DEF {1} SHARED VAR verkben  AS C FORMAT "x(48)"                   NO-UNDO.
DEF {1} SHARED VAR partnum  LIKE elect.selection.p-id FORMAT ">>" NO-UNDO.
DEF {1} SHARED VAR partnam  LIKE elect.selection.party_name       NO-UNDO.
DEF {1} SHARED VAR partlan  AS C FORMAT "x(20)"                   NO-UNDO.
DEF {1} SHARED VAR candtyp  LIKE elect.selection.c-type           NO-UNDO.
DEF {1} SHARED VAR candnum  LIKE elect.selection.c-id             NO-UNDO.
DEF {1} SHARED VAR elecid   LIKE elect.election.et-id             NO-UNDO.
DEF {1} SHARED VAR maxpar   AS I                                  NO-UNDO.

DEF {1} SHARED VAR diskmade LIKE elect.session.disk_made          NO-UNDO.
DEF {1} SHARED VAR diskread LIKE elect.session.disk_read          NO-UNDO.

DEF {1} SHARED VAR colme    AS C FORMAT "x(79)"                   NO-UNDO.
DEF {1} SHARED VAR verkmax  AS I FORMAT "99" INIT 99              NO-UNDO.
