/* Filename            : CFVAR.I                                        */

/************************************************************************/
/*                                                                      */
/*        SHARED VARS                                                   */
/*                                                                      */
/************************************************************************/
DEF {1} SHARED VAR db-version     AS L                            NO-UNDO.

DEF {1} SHARED VAR cf-signature   AS L                            NO-UNDO.
DEF {1} SHARED VAR cf-version     LIKE elect.setup.version        NO-UNDO.
DEF {1} SHARED VAR cf-disk-id     LIKE elect.setup.disk-id        NO-UNDO.
DEF {1} SHARED VAR cf-sys-type    LIKE elect.setup.sys-type       NO-UNDO.
DEF {1} SHARED VAR cf-org-type    LIKE elect.setup.org-type       NO-UNDO.
DEF {1} SHARED VAR cf-areaname    LIKE elect.setup.areaname       NO-UNDO.
DEF {1} SHARED VAR cf-orginator   LIKE elect.setup.orginator      NO-UNDO.
DEF {1} SHARED VAR cf-corrupt     LIKE elect.session.corrupt      NO-UNDO.
DEF {1} SHARED VAR cf-hands_off   LIKE elect.session.hands_off    NO-UNDO.
DEF {1} SHARED VAR cf-disk_read   LIKE elect.session.disk_read    NO-UNDO.
DEF {1} SHARED VAR cf-disk_made   LIKE elect.session.disk_made    NO-UNDO.
DEF {1} SHARED VAR cf-password    LIKE elect.setup.password       NO-UNDO.
DEF {1} SHARED VAR cf-local-types AS L                            NO-UNDO.

/* eof */