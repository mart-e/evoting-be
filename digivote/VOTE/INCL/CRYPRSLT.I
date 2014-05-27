/*@function sec=1*/
/* Filename        : cryprslt.i                 */

/************************************************************************/
/*                                                                      */
/*        LOCAL VARIABLES                                               */
/*                                                                      */
/************************************************************************/

   DEF VAR Ok    AS L NO-UNDO.
   DEF VAR tmp_i AS I NO-UNDO.
   
/************************************************************************/
/*                                                                      */
/*        SHARED VARIABLES                                              */
/*                                                                      */
/************************************************************************/
   DEF {1} SHARED VAR hex-digest   AS C FORMAT "x(40)"            NO-UNDO.

/* return value of CRYPENC.C */
   DEF {1} SHARED VAR result3      AS C FORMAT "X(32)"            NO-UNDO.
/* return value of CRYPDEC.C */
   DEF {1} SHARED VAR result4      AS C FORMAT "X(16)"            NO-UNDO.

/* :WARNING: avn 20020111 -> database name MUST be specified in LIKE
   otherwise when two elect db's are connected as in COPYLIS.P a compilation
   error is issued because there are two instances of the file and this
   upsets the compiler */

/* return value of C_KEYGEN.C */
   DEF {1} SHARED VAR rndKey       LIKE elect.meta.aesKey         NO-UNDO.

/* keyboard buffer while entering passwords with GetPasw.i */
   DEF {1} SHARED VAR cPassword    LIKE elect.meta.aesKey         NO-UNDO.

/* holds master/sessionkey used to encrypt data in current database */
   DEF {1} SHARED VAR dbmasterk    LIKE elect.meta.aesKey  
                                        INIT "-1"                 NO-UNDO.
   DEF {1} SHARED VAR dbsessionk   LIKE elect.meta.aesKey         
                                        INIT "-1"                 NO-UNDO.

   DEF {1} SHARED VAR masterk      LIKE elect.meta.aesKey         NO-UNDO.
   DEF {1} SHARED VAR sessionk     LIKE elect.meta.aesKey         NO-UNDO.

/* AES keys are 16 bytes long */
   DEF {1} SHARED VAR AESKEYLEN    AS I INIT 16                   NO-UNDO.
/* MAC's are 8 bytes long */
   DEF {1} SHARED VAR MACLEN       AS I INIT 8                    NO-UNDO.
   DEF {1} SHARED VAR hexMAC       AS C FORMAT "X(24)"            NO-UNDO.
   DEF {1} SHARED VAR formatMAC    AS C FORMAT "X(24)"            NO-UNDO.

/* Encyption key values */
   DEF {1} SHARED VAR URNEXE       AS C INIT   "0"                NO-UNDO.
   DEF {1} SHARED VAR MAVEXE       AS C INIT   "1"                NO-UNDO.
   DEF {1} SHARED VAR GENDAT       AS C INIT   "2"                NO-UNDO.
   DEF {1} SHARED VAR URNVOT       AS C INIT   "3"                NO-UNDO.
   DEF {1} SHARED VAR URNTOT       AS C INIT   "4"                NO-UNDO.
   DEF {1} SHARED VAR MAVVOT       AS C INIT   "5"                NO-UNDO.
   DEF {1} SHARED VAR PRPTOT       AS C INIT   "6"                NO-UNDO.
   DEF {1} SHARED VAR URNCHK       AS C INIT   "7"                NO-UNDO.
   DEF {1} SHARED VAR RECEXE       AS C INIT   "8"                NO-UNDO.
   DEF {1} SHARED VAR PRPEXP       AS C INIT   "9"                NO-UNDO.
   DEF {1} SHARED VAR RESMAC       AS C INIT  "10"                NO-UNDO.

/* holds return code set by all Aes-encryption routines */
   DEF {1} SHARED VAR crypStat     AS I                           NO-UNDO.

/* holds material for key generation in rnd.p */
   DEF {1} SHARED VAR iKeyCtr      AS I INIT 0                    NO-UNDO.
   DEF {1} SHARED VAR cUserName    AS C                           NO-UNDO.
   DEF {1} SHARED VAR cRandChars   AS C                           NO-UNDO.
   DEF {1} SHARED VAR cRandDigits  AS C                           NO-UNDO.

/* eof */