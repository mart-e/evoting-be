/* Filename            : BIJLAGE.I                                       */

/* Afdruk van ondercategorie */
        /* ontcijferen stemmen + totaliseer */
        call CRYPDEC PRPTOT party.vote_top.
        ASSIGN
        wz-catg = 0
        wz-catgtot = 0
        wz-catg[1] = INT(result4).
        call CRYPDEC PRPTOT party.vote_can.
        wz-catg[2] = INT(result4).
        call CRYPDEC PRPTOT party.vote_cs.
        wz-catg[3] = INT(result4).
        call CRYPDEC PRPTOT party.vote_sup.
        wz-catg[4] = INT(result4).

        wz-catgtot = wz-catg[1] + wz-catg[2] + wz-catg[3] + wz-catg[4].


        IF wz-tl = 1 THEN
            PUT UNFORMATTED
                "Ondercategorie 1¯       Aantal kaarten met stemmen bovenaan"   AT 5 SKIP
                "                        de lijst"                              AT 5
                wz-catg[1]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Ondercategorie 2¯       Aantal kaarten met stemmen voor ÇÇn"   AT 5 SKIP
                "                        of meerdere kandidaat-titularissen"    AT 5
                wz-catg[2]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Ondercategorie 3¯       Aantal kaarten met stemmen voor ÇÇn"   AT 5 SKIP
                "                        of meerdere kandidaat-titularissen"    AT 5
                wz-catg[3]  AT 80   FORMAT "z,zzz,zz9"
                SKIP
                "                        en voor ÇÇn of meerdere kandidaat-"    AT 5 SKIP
                "                        opvolgers"                             AT 5 SKIP(1)
                "Ondercategorie 4¯       Aantal kaarten met stemmen voor ÇÇn"   AT 5 SKIP
                "                        of meerdere kandidaat-opvolgers"       AT 5
                wz-catg[4]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "_________" AT 80
                SKIP(1)
                "STEMCIJFER : " AT 50
                wz-catgtot  AT 80   FORMAT "z,zzz,zz9"
                SKIP(2).

        ELSE IF wz-tl = 2 THEN
            PUT UNFORMATTED
                "Sous-catÇgorie 1¯       Nombre de cartes avec des votes"                       AT 5 SKIP
                "                        en tàte de liste"                                      AT 5
                wz-catg[1]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Sous-catÇgorie 2¯       Nombre de cartes avec des votes en faveur d'un ou"     AT 5 SKIP
                "                        plusieurs candidats titulaires"                        AT 5
                wz-catg[2]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Sous-catÇgorie 3¯       Nombre de cartes avec des votes en faveur d'un ou"     AT 5 SKIP
                "                        plusieurs candidats titulaires et d'un ou"             AT 5
                wz-catg[3]  AT 80   FORMAT "z,zzz,zz9"
                SKIP
                "                        plusieurs candidats supplÇants"                        AT 5 SKIP(1)
                "Sous-catÇgorie 4¯       Nombre de cartes avec des votes en faveur d'un ou"     AT 5 SKIP
                "                        plusieurs candidats supplÇants"                        AT 5
                wz-catg[4]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "_________" AT 80
                SKIP(1)
                "CHIFFRE ELECTORAL" AT 50
                wz-catgtot  AT 80   FORMAT "z,zzz,zz9"
                SKIP(2).

        ELSE IF wz-tl = 3 THEN
            PUT UNFORMATTED
                "Unterkategorie 1        Anzahl Karten mit Stimmabgabe im Kopffeld"         AT 5
                wz-catg[1]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Unterkategorie 2        Anzahl Karten mit Stimmabgabe fÅr den"             AT 5 SKIP
                "                        ordentlichen Kandidaten"                           AT 5
                wz-catg[2]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "Unterkategorie 3        Anzahl Karten mit Stimmabgabe fÅr den"             AT 5 SKIP
                "                        ordentlicher Kandidaten und fÅr einen"      AT 5
                wz-catg[3]  AT 80   FORMAT "z,zzz,zz9"
                SKIP
                "                        oder mehrere Ersatzkandidaten"                     AT 5 SKIP(1)
                "Unterkategorie 4        Anzahl Karten mit Stimmabgabe fÅr einen oder"      AT 5 SKIP
                "                        mehrere Ersatzkandidaten"                          AT 5
                wz-catg[4]  AT 80   FORMAT "z,zzz,zz9"
                SKIP(1)
                "_______" AT 80
                SKIP(1)
                "WAHLZIFFER : " AT 50
                wz-catgtot  AT 80   FORMAT "z,zzz,zz9"
                SKIP(2).
