nbre_Liste_Int = 0;                                      
colonne = 0;
k = 0;

while(k < (nbrl - 1))
             {
               if(listes[iliste].college == Choix_College)
				      nbre_Liste_Int++; 
	       iliste++;
	       k++; 
	     }
nbre_Liste_Int++; 

nbr_lignes =  nbre_Liste_Int / 3;
nbr_lignes +=((nbre_Liste_Int % 3) > 0) ? 1 : 0;
colonne = 3;
espl = 5;
/* << el2007 */
espw = 2;
/* el2007 >> */
lon = 210;

if (nbr_lignes <= 5)
	 {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
		 h = 60;
		 recl = 60;
	 }
if (nbr_lignes > 5 && nbr_lignes <= 6)
	 {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
		 h = 60;
		 recl = 56;
	 }
	
if (nbr_lignes > 6 && nbr_lignes <= 7)
	 {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
		h = 55;
		recl = 50;
	 }
	
if (nbr_lignes > 7 && nbr_lignes <= 8)
	 {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
		 h = 55;
                 recl = 42;
	 }

if (nbr_lignes > 8 && nbr_lignes <= 9)
         {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
                  h = 55;
                  recl = 36;
	}
if (nbr_lignes > 9 && nbr_lignes <= 10)
	 {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
		 h = 55;
                 recl = 32;
	 }

if (nbr_lignes > 10 && nbr_lignes <= 11)
         {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
                  h = 55;
                  recl = 30;
	}

if (nbr_lignes > 11 && nbr_lignes <= 12)
         {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
                  h = 55;
                 recl = 25;
	}

if (nbr_lignes > 12 && nbr_lignes <= 13)
         {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
                  h = 55;
                  recl = 25;
	}

if (nbr_lignes > 13 && nbr_lignes <= 14)
         {
/* << el2007 */
                 /*settextstyle( SMALL_FONT, HORIZ_DIR, 4);*/
                 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
/* el2007 >> */
                  espl = 7;
                  h = 55;
                  recl = 25;
	}

