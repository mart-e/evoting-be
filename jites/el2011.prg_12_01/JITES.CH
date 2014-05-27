***
* ⁄ƒƒƒƒ Jites.ch
* ≥
*
*/

#command MEM_DECLARE => PRIVATE lxVarMem,lnBoucle               ;;
                        FOR lnBoucle = 1 TO FCOUNT()            ;;
                            lxVarMem := "X"+FIELDNAME(lnBoucle) ;;
                            PRIVATE &lxVarMem                   ;;
                        NEXT lnBoucle

// Type d'unitÇ
#define _UNITE_A             "A"
#define _UNITE_B             "B"
#define _UNITE_C             "C"

// chemin d'accäs
#define _REP_JITES            "\jites"
#define _PATH_JITES           ccUniteActive+':'+_REP_JITES                  // Root JITES
#define _PATH_JITES_DISQUETTE _UNITE_A+':'+_REP_JITES                       // Root JITES
#define _PATH_STRU_BAS        _PATH_JITES+"\stru_bas"                       // Path fichiers de base
#define _PATH_PC_TOT          _PATH_JITES+"\pc_tot"                         // Path fichiers pour PC de totalisation
#define _PATH_ELECTION        _PATH_JITES+"\"+ccTypeElec+ccElecActive+ccRep // Path fichiers d'une Çlection

// type de champ
#define _CARACTER            "C"
#define _NUMERIC             "N"
#define _DATE                "D"
#define _LOGIC               "L"
#define _MEMO                "M"

// ElÇments logiques
#define _TRUE                .t.
#define _FALSE               .f.
#define _VRAI                .t.
#define _FAUX                .f.
#define _EJECT               .t.
#define _NO_EJECT            .f.
#define _OK                  .t.
#define _NOT_OK              .f.
#define _ON                  .t.
#define _OFF                 .f.
#define _VERIFY_ON           .t.
#define _VERIFY_OFF          .F.
#define _SHARED              .f.
#define _EXCLUSIVE           .t.
#define _MOT_PASSE_OK        .t.

// Les couleurs de base
#define _NOIR                "N"
#define _BLEU                "B"
#define _VERT                "G"
#define _TURQUOISE           "BG"
#define _ROUGE               "R"
#define _MAGENTA             "RB"
#define _BRUN                "GR"
#define _GRIS_CLAIR          "W"
#define _GRIS_FONCE          "N+"
#define _BLEU_CLAIR          "B+"
#define _VERT_CLAIR          "G+"
#define _TURQUOISE_CLAIR     "BG+"
#define _ROSE                "R+"
#define _MAGENTA_CLAIR       "RB+"
#define _JAUNE               "GR+"
#define _BLANC               "W+"
#define _INVERSION           "≥"
#define _CACHE               "X"

// Couleurs de JITES
#define _COLOR_LIG1          _NOIR+'/'+_BLANC+','+_NOIR+'/'+_BLANC
#define _COLOR_OMBRE         _BLANC+'/'+_NOIR
#define _COLOR_FOND          _NOIR+'+/'+_BLANC+','+_NOIR+'/'+_BLANC
#define _COLOR_MENU          _BLANC+'+/'+_NOIR+','+_NOIR+'/'+_BLANC
#define _COLOR_ERROR         _BLANC+'/'+_ROUGE+','+_BLANC+'/'+_ROUGE
#define _COLOR_BLANC_BLEU    _BLANC + '/' + _NOIR
#define _COLOR_TRAV1         _BLANC +'/' + _NOIR + ',' + _NOIR+'/' +_BLANC
#define _COLOR_ATT           'W/R,N/W'
#define _COLOR_BLINK         '*'
#define _COLOR_EXCEP         _NOIR+'/'+_TURQUOISE+','+_NOIR+'/'+_BLANC

// Les langues
#define _FRANCAIS            "F"
#define _NEERLANDAIS         "N"
#define _ALLEMAND            "A"

// Les traitements
#define _RECUPERATION        "6"
#define _MENU_RESULTAT       "9"

#define _LISTE_COMMANDE     _MENU_RESULTAT

// Paramätre de la fonction _affmsg()
#define AM_ALIGN_GA    "G"        // Aligner Ö gauche
#define AM_ALIGN_DR    "D"        // Aligner Ö droite
#define AM_ALIGN_CE    "C"        // Centrer
#define AM_ALIGN_MI    "M"        // Milieu absolu
#define AM_EFF_OUI     "O"        // Effacer avant afficher msg
#define AM_EFF_NON     "N"        // Ne pas effacer avant afficher msg
#define AM_EFF_PARTIEL "P"        // Effacer ligne Ö partir de
#define AM_EFF_CENTRE  "C"        // Effacer Reste

// Code retour DIRMAKE()
#define _NO_DISK_ERR         0
#define _ER_FILE_NOT_FOUND   -2
#define _ER_PATH_NOT_FOUND   -3
#define _ER_ACCES_DENIED     -5

// Code retour FRENAME
#define _ECHOUE              -1

// Code FERROR()
#define _FICHIER_INEXISTANT         2
#define _REPERTOIRE_INEXISTANT      3
#define _NB_MAX_FICHIERS_DEPASSE    4
#define _ACCES_IMPOSSIBLE           5
#define _INDICATION_INVALIDE        6
#define _ESPACE_MEMOIRE_INSUFFISANT 8
#define _INDICATION_UNITE_ERRONEE  15
#define _DISQUETTE_PROTEGEE        19
#define _UNITE_NON_PRETE           21
#define _ERREUR_DE_CHECKSUM        23
#define _ERREUR_ECRITURE           29
#define _ERREUR_LECTURE            30
#define _ACCES_MULTIPLE_INTERDIT   32
#define _ACCES_MULT_INTERDIT       33

// CapacitÇ total de la disquette
#define _CAPACITE_DISK_B       1442000
#define _CAPACITE_DISK_MINIMUM 1024000

// Constantes JITES
#define _VERSION_JITES       "12.01"

// Mode de rÇindexation
#define _INDEX_NTX_NOT_EXIST      'R'
#define _INDEX_ALL_FILE           'A'

// Chaine de caractäres
#define _ALPHA_MAJUSCULE          'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
#define _NUMERIQUE                '0123456789'
#define _CHAINE_VIDE              ''
#define _CARACT_CROCHETS          '['+CHR(24)+CHR(25)+']'
#define _FLECHE_H                 CHR(24)

// SELECT des fichiers DS /jites
#define _SELECT_MESSAGES                1
#define _SELECT_MOT_PASSE               2
#define _SELECT_LES_ELECTIONS           3
#define _SELECT_PARAMETRES              4
#define _SELECT_CANTONS                 5
#define _SELECT_ARRONDISSEMENT          6
#define _SELECT_SCRUTINS                7
#define _SELECT_TYPE_ELECTION           8

// SELECT des fichier d'une Çlection
#define _SELECT_ELECT_PARAMETRES       20
#define _SELECT_ELECT_SCRUTINS         21
#define _SELECT_ELECT_BUREAUX_VOTES    22
#define _SELECT_ELECT_CANDIDATS        23
#define _SELECT_ELECT_LISTES           24
#define _SELECT_ELECT_RECUP_VOTES      25


// SELECT pour le transfert de bureaux entre pc de totalisation
#define _SELECT_BUR_TRANS              47

* DEFINE pour la fonction LISTE
#define LI_NORMAL      "N"        // Afficher en mode normal
#define LI_REVERSE     "R"        // Afficher en inverse vidÇo
#define LI_EXIT_ESC    "[27]"     // Key pour sortir de la liste par Esc
#define LI_EXIT_TAB    "[9]"      // Key pour sortir de la liste par TAB
        // Return de la Fct Usr
#define LI_EXIT        0          // Quitter la liste
#define LI_1DEPAGE     1          // Se positionner sur 1er ligne de la page
#define LI_MAJLIG      2          // Refresh de la ligne
#define LI_REFRESH     3          // Refresh TOTAL
#define LI_RESTPAGE    4          // RÇafficher uniquement le reste de la page
#define LI_NOTHING     5          // Ne rien faire

// RÇponses utilisateur
#define _REPONSE_OUI              1
#DEFINE _REPONSE_NON              2

// Les paramätres d'un GET
#define _GET_PICTURE                  1
#define _GET_COLOR                    2
#define _GET_WHEN                     3
#define _GET_RANGE                    4
#define _GET_VALID                    5
#define _GET_MODIF                    _TRUE
#define _GET_NOMODIF                  _FALSE

// Codes propre Ö l'application
#define _CODE_EFFECTIF               'E'
#define _CODE_SUPPLEANT              'S'

#define _IMPR_GRAS_ON             ''
#define _IMPR_GRAS_OFF            ''
#define _IMPR_SOUL_ON             ''
#define _IMPR_SOUL_OFF            ''
#define _NB_LIG_PAGE              66
#define _ASCII_CR_LF              CHR(13)+CHR(10)

// Types d'opÇrations Çlectorales
// << phe2009
#define _TYPE_OPER_REGION             '7'
#define _TYPE_OPER_4ELEC              '8'
// phe2009>>
#define _TYPE_OPER_LEGISLATIVE        '3'

// Scrutins possibles
#define _SCRUTIN_EUROPEEN             '1'
#define _SCRUTIN_CHAMBRE              '2'
#define _SCRUTIN_SENAT                '3'
#define _SCRUTIN_REGIONAL             '4'
#define _SCRUTIN_PROVINCIAL           '5'
#define _SCRUTIN_COMMUNAL             '6'
#define _SCRUTIN_BXL_REGION_FLA       '7'

// Statuts des bureaux de votes
#define _STAT_BUR_LECTURE             'Lecture'
#define _STAT_BUR_ECRITURE            'Ecr.Disk'


// Paramätres de la totalisation partielle
#define _TOT_PART_TOUT_LES      10
#define _TOT_PART_MIN_REST       3

// Mode test
#define _BUR_TOTAL_COMMUNAL      'C'

// Arrondissement
#define _ARROND_LIEGE            '001'
#define _ARROND_BRUXELLES        '002'
#define _ARROND_VERLAINE         '003'
#define _ARROND_VERVIERS         '004'
#define _ARROND_HASSELT          '005'
#define _ARROND_MONS             '006'
#define _ARROND_MARCHE           '007'
#define _ARROND_MOUSCRON         '008'

// Colläge
#define _COLLEGE_FRANCOPHONE     'F'
#define _COLLEGE_NEERLANDOPHONE  'N'

  /* Paramätres de la fonction _box()      */
#define BOX_EFFET_BAS                 "B"
#define BOX_EFFET_COL                 "C"
#define BOX_EFFET_LIG                 "L"

  /* Autres Caractäres  que inkey.ch  */
#define    K_ACTIF                    251


