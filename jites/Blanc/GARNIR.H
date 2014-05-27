struct scrutin_t
{
    char code_s;
    char scrutin_max_cand[3];
    char scrutin_max_supp[3];
    char nom_s;
    int  TYPE_SCRUTIN;
    char nbrelist_s[3];
    struct liste_t *ptlist;
}scrutin[10];

struct liste_t
{
    char code_l;
    char college;
    char numlist_l[3];
    char nom_l[26];
    char nbreff_l[4];
    char nbrsup_l[3];
    char adresse_cand[7];
};

struct cand_t
{
    char code_c;
    char numlist_c[3];
    char nom_c[26];
    char prenom_c[26];
    char prenom2_c[26];
    char E_S;
};
