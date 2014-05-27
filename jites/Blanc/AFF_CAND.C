/***************************************************************************/
/* JITES                                                                   */
/* prog AL-KHANSA JAMAL                                                    */
/* rvis  partir du 10-9-1992 comme maquette bxls                        */
/* LE 6-2-1991                                                             */
/* LANGAGE TC                                                              */
/*                                                                         */
/*                                VARIABLES                                */
/* Hcadre_candid = hauteur du cadre candidat                               */
/* Interl_candid = Interligne entre les cadres candidats                   */
/* Lsaisie_candid = longueur de la zone de saisie candidat                 */
/* Lcadre_candid  = longueur du cadre candidat                             */
/* Interc_candid  = Intercolonne entre les cadres candidats                */
/* x_dep          = valeur de la coordonne x de la  1 ere colonne         */
/* x_ent_scr      = valeur de x de l'entete du scrutin                     */
/* x_numl         = valeur de x du numero de la liste                      */
/* x_noml         = valeur de x du nom de la liste                         */
/* nbc_col        = nombre de candidats par colonne                        */
/***************************************************************************/
/******************   S E L E C T I O N  D U  C A N D I D A T  ************/
/**************************************************************************/
void affiche_num_cdt(int _iNum)
{
	char c[3];

	/* Separateur */
	setfillstyle(SOLID_FILL,BLACK);
	bar(x+CDT_NUM_WIDTH-1,y,x+CDT_NUM_WIDTH-1,posc[k][3] + HAsaisie_candid);

	/* Num�o */
	sprintf(c, "%2d", _iNum);
	settextstyle( SMALL_FONT, HORIZ_DIR, 5);
	settextjustify(CENTER_TEXT,CENTER_TEXT);
	outtextxy(x+(CDT_NUM_WIDTH/2)-1,(y + Hcadre_candid/2) -1, c);
	settextstyle( SMALL_FONT, HORIZ_DIR, 4);
}


void type_ecran(int *_nb_cdt_par_colonne)
{
	int iNbreDeSiegeAPourvoir;
	int iNbreColonne;
	int iNbreEffectif;
	
	iNbreDeSiegeAPourvoir = atoi(scrutin[scr].scrutin_max_cand) - atoi(scrutin[scr].scrutin_max_supp) - 1;
	iNbreEffectif = te[scr][list];

	if(iNbreDeSiegeAPourvoir <= iRuptureColonne)
				iNbreColonne = 1;
	else 
	{
	  if(iNbreEffectif <= iRuptureColonne) /* candidats titulaires <= iNbreDeSiegeAPourvoir */
				iNbreColonne = 1;
	  else /* candidats titulaires >= 15 */
				iNbreColonne = iNbreEffectif/iRuptureColonne;
	}
	if((iNbreColonne * iRuptureColonne) < iNbreEffectif)
				iNbreColonne++;

	/* Nbr cdt par colonne */
	if(iNbreColonne == 1)
		(*_nb_cdt_par_colonne) = iNbreEffectif;
	else 
	{  if(iNbreColonne < 4)
	       {
		(*_nb_cdt_par_colonne) = iNbreEffectif / iNbreColonne;
		if((*_nb_cdt_par_colonne * iNbreColonne)< iNbreEffectif)
		++(*_nb_cdt_par_colonne);
	       }
	    else
		*_nb_cdt_par_colonne=iRuptureColonne;

	}

}
/****************************************************************************/
int affichc()
{
	char buf[30];
	int iNumCdt;
	int nb_cdt_par_colonne;
	swpas = 0;
	swDeselect = 0;
	Garnir_param();
	param_Candidat();
	garnir_cand();
	buf[0]=scrutin[scr].nom_s;
	buf[1]= 0x00;
	strcat(buf,ps->numlist_l);
	if(strcmp(Arrond,"005") == 0 )
		buf[3]='N';
	else
		buf[3]=ps->college;
	buf[4]=0x00;
	strcat(buf,".bmp");
	if(buf[1] == ' ')   buf[1] = '0';

/**************************   ENTETE CANDIDAT  ****************************/

	setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
	bar( 0,0,639,479);      /* Draw backgnd */
	setcolor( WHITE);
	settextstyle(SMALL_FONT, HORIZ_DIR, 6);
	setfillstyle(SOLID_FILL,7);

/**************************   ENTETE CANDIDAT  ****************************/


	if(affTL_code == 1) {
		posc[0][0] = 320 - 25;
	}
	else if (affTL_code == 2) {
		posc[0][0] = 80;
	}
	else if (affTL_code == 3) {
		posc[0][0] = 72;
	}
	else if (affTL_code == 4) {
		posc[0][0] = 52;
	}
	else if (affTL_code == 5) {
		posc[0][0] = 52;
	}
	posc[0][1] = posc[0][0] + 50;
	posc[0][2] = 2;
	posc[0][3] = 27;
	bar(posc[0][0] + 10 ,posc[0][2], posc[0][1] - 10 , posc[0][3]);
	cercle(0,0);
	/**************************   ENTETE CANDIDAT  ****************************/

	setfillstyle(SOLID_FILL,WHITE); /* Set to solid fill in color   */
	setcolor( WHITE);

	settextjustify(RIGHT_TEXT,CENTER_TEXT);
	settextstyle( SMALL_FONT, HORIZ_DIR, 5);

	outtextxy( 630, 13, Message_point[code_scr]);
	outtextxy( 629, 13, Message_point[code_scr]);



	settextstyle( SMALL_FONT, HORIZ_DIR, 6);
	if      (affTL_code == 1) {
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy(  5, 13, ps->numlist_l);
		outtextxy(  6, 13, ps->numlist_l);
		outtextxy(  5, 14, ps->numlist_l);
		outtextxy(  6, 14, ps->numlist_l);

		if(access(buf,0) == 0)
			bmp(buf, posc[0][0] -10,2,0);

		else
		{
			settextjustify(RIGHT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][0] -10, 13, ps->nom_l);
			outtextxy( posc[0][0] -11, 13, ps->nom_l);
			outtextxy( posc[0][0] -10, 14, ps->nom_l);
			outtextxy( posc[0][0] -11, 14, ps->nom_l);
		}
	}
	else if (affTL_code == 2) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, ps->numlist_l);
		outtextxy( posc[0][0] -11, 13, ps->numlist_l);
		outtextxy( posc[0][0] -10, 14, ps->numlist_l);
		outtextxy( posc[0][0] -11, 14, ps->numlist_l);
		if(access(buf,0) == 0)
			bmp(buf, posc[0][1] +10,2,0);
		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, ps->nom_l);
			outtextxy( posc[0][1] +11, 13, ps->nom_l);
			outtextxy( posc[0][1] +10, 14, ps->nom_l);
			outtextxy( posc[0][1] +11, 14, ps->nom_l);
		}
	}
	else if (affTL_code == 3) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, ps->numlist_l);
		outtextxy( posc[0][0] -11, 13, ps->numlist_l);
		outtextxy( posc[0][0] -10, 14, ps->numlist_l);
		outtextxy( posc[0][0] -11, 14, ps->numlist_l);

		if(access(buf,0) == 0)
			bmp(buf, posc[0][1] +10,2,0);
		else
		{
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][1] +10, 13, ps->nom_l);
		outtextxy( posc[0][1] +11, 13, ps->nom_l);
		outtextxy( posc[0][1] +10, 14, ps->nom_l);
		outtextxy( posc[0][1] +11, 14, ps->nom_l);
		}
	}
	else if (affTL_code == 4) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, ps->numlist_l);
		outtextxy( posc[0][0] -11, 13, ps->numlist_l);
		outtextxy( posc[0][0] -10, 14, ps->numlist_l);
		outtextxy( posc[0][0] -11, 14, ps->numlist_l);

		if(access(buf,0) == 0)
			bmp(buf, posc[0][1] +10,2,0);
		else
		{
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][1] +10, 13, ps->nom_l);
		outtextxy( posc[0][1] +11, 13, ps->nom_l);
		outtextxy( posc[0][1] +10, 14, ps->nom_l);
		outtextxy( posc[0][1] +11, 14, ps->nom_l);
		}
	}
	else if (affTL_code == 5) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, ps->numlist_l);
		outtextxy( posc[0][0] -11, 13, ps->numlist_l);
		outtextxy( posc[0][0] -10, 14, ps->numlist_l);
		outtextxy( posc[0][0] -11, 14, ps->numlist_l);

		if(access(buf,0) == 0)
			bmp(buf, posc[0][1] +10,2,0);

		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, ps->nom_l);
			outtextxy( posc[0][1] +11, 13, ps->nom_l);
			outtextxy( posc[0][1] +10, 14, ps->nom_l);
			outtextxy( posc[0][1] +11, 14, ps->nom_l);
		}
	}

	line(0,30,639,30);  /* ligne de sparation. */
/**************                AFFICHAGE DES CANDIDATS        ***************/

	settextjustify(CENTER_TEXT,CENTER_TEXT);
	settextstyle( SMALL_FONT, HORIZ_DIR, 4);
	setcolor( BLACK);

	candid = te[scr][list] + ts[scr][list];
	type_ecran(&nb_cdt_par_colonne);
	for(k=1,x = x_dep;(k < te[scr][list] + 1);x+= Lcadre_candid + Interc_candid)
	{
		for(j = 0,y = Debut_Affichage;(k < te[scr][list] + 1) && j < nb_cdt_par_colonne;y+= Hcadre_candid + Interl_candid,k++,j++)
		{
			posc[k][0]= (x + Lcadre_candid) - LAsaisie_candid;
			posc[k][1]= (x + Lcadre_candid) - 1;
			posc[k][2]=  y + HAsaisie_candid;
			posc[k][3]= (y + Hcadre_candid) - 1 - HAsaisie_candid;

			setfillstyle(SOLID_FILL,WHITE);
			bar(x,y,posc[k][1],posc[k][3] + HAsaisie_candid);

			Cal_affiche();
			cercle(k,0);
			affiche_num_cdt(k);
			fc++;
		}
	}

	settextstyle(SMALL_FONT, HORIZ_DIR, 6);
	setcolor( WHITE);

	if ((ts[scr][list] > 0) && ( te[scr][list] < 70))
	{
		x = 639 -( x_dep + Lcadre_candid);
		y = Debut_Affichage;
		j = 1;
		outtextxy(x + (Lcadre_candid / 2 ) ,y + ((Hcadre_candid/2) - Interl_candid),Message_point[47]);
		outtextxy(x + (Lcadre_candid / 2 ) + 1 ,y + ((Hcadre_candid/2) - Interl_candid),Message_point[47]);
		settextstyle( SMALL_FONT, HORIZ_DIR, 4);
		setcolor( BLACK);
	}
	else
		if((ts[scr][list] > 0) && ( te[scr][list] > 69))
		{
			j = j++;
			x-= Lcadre_candid + Interc_candid;
			outtextxy(x + (Lcadre_candid / 2 ) ,y + ((Hcadre_candid/2) - Interl_candid),Message_point[47]);
			outtextxy(x + (Lcadre_candid / 2 ) + 1 ,y + ((Hcadre_candid/2) - Interl_candid),Message_point[47]);
			settextstyle( SMALL_FONT, HORIZ_DIR, 4);
			setcolor( BLACK);
		}
	for(;(k < candid + 1);x+= Lcadre_candid + Interc_candid,y =  0,j = 0)
		for(iNumCdt=1, y += ((Hcadre_candid + Interl_candid)) ;(k < candid + 1) ;y+= Hcadre_candid + Interl_candid,k++,j++, iNumCdt++)
		{
		if(j == 0)
			y += Debut_Affichage - (Hcadre_candid + Interl_candid);
		posc[k][0]= (x + Lcadre_candid) - LAsaisie_candid;
		posc[k][1]= (x + Lcadre_candid) - 1;
		posc[k][2]=  y + HAsaisie_candid;
		posc[k][3]= (y + Hcadre_candid) - 1 - HAsaisie_candid;

		setfillstyle(SOLID_FILL,WHITE);
		bar(x,y,posc[k][1],posc[k][3] + HAsaisie_candid);
		Cal_affiche();
		cercle(k,0);
		affiche_num_cdt(iNumCdt);
		fc++;
		}
#if (LPMNGLIB >=1)
	if (swLPcursor)  lpPosxy(639, 479);
#endif
	if(visu == 1)   /* Mode visualisation  */
	{
		setfillstyle(SOLID_FILL, WHITE);
		bararrondi(110,452,530,479,WHITE);
		setcolor(BLACK);
		settextjustify(CENTER_TEXT, CENTER_TEXT);
		settextstyle( SMALL_FONT, HORIZ_DIR, 7);
		outtextxy(320,464, Message_point[92]);
		outtextxy(321,464, Message_point[92]);
		if(passe > 0)
			calmos(Visu_tempo);
		else
		{
			while(1)
			{
			#if (LPMNGLIB >=1)
				if (swLPcursor)
					lpCursorOn();
				#endif
				if (point())
				{
					#if (LPMNGLIB >=1)
					lpCursorOff();
					#endif
					if((cy > 450) && (cy < 479))
					{
						if((cx > 108) && (cx <  530))
							return(0);
					}
				}
			}
		}
		return(0);
	}
	while((klavier) || (passe == 0))
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor)  lpCursorOn();
		#endif
		if (point()) {
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if (testposc()) {
				return(0);
			}
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif

	msg_error = 13;
	ptfunct = fin;

	return(0);
}
/***************************************************************************/
/******                       Calcul de position                     ******/
/***************************************************************************/

Cal_affiche()
{
	int nombre/*, diff3*/;


	nombre = 0;
	if(fc->nom_c[0] != 0x00)
	nombre = 1;
	if(fc->prenom_c[0] != 0x00)
	nombre = 2;
	if(Condense == 1)
	{
		(fc->nom_c[18] = 0x00);
		(fc->prenom_c[18] = 0x00);
	}
	switch(nombre)
	{
	case 1 :
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy(x+CDT_NUM_WIDTH +5,(y + Hcadre_candid/3) -2, fc->nom_c);
		break;
	case 2 :
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy(x+CDT_NUM_WIDTH +5,(y + Hcadre_candid/3) -2, fc->nom_c);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		outtextxy(x+((CDT_NUM_WIDTH+Lcadre_candid) / 2 ),y + ((Hcadre_candid * 2) / 3) , fc->prenom_c);
		break;
	}
}
/***************************************************************************/
/******          MENU DE CONFIRMATION DU CHOIX DU CANDIDAT + CERCLE   ******/
/***************************************************************************/
cercle(z,z1)    int z,z1;
{
	int xc,yc,diam;
	int ax1, ax2, ay1, ay2;
	xc = posc[z][1];

	ax1= (posc[z][1] - Lcadre_candid) + 1;
	ax2= ax1 + Lcadre_candid;
	ay1= posc[z][2] - HAsaisie_candid;
	ay2= ay1 + Hcadre_candid;

	if(z == 0)
	{
		xc -= (posc[z][1] - posc[z][0] ) / 2;
		yc = 15;
		diam = 11;
	}
	settextstyle(SMALL_FONT, HORIZ_DIR, 6);
	setcolor(8);
	if(z == 0)
		circle(xc ,yc , diam);
	if(memoc[scr][list][z] == 1)
	{
		if(z == 0)
		{
			setfillstyle(SOLID_FILL, BLACK);
			floodfill(xc ,yc ,8);
		}
		if(z > 0)
			Inverse_Candid(ax1,ax2,ay1,ay2, WHITE, 7);
		if (((swnoir == 0) && (visu == 0)) || swDeselect)
		{
			setfillstyle(SOLID_FILL, BLACK);
			bar(0,447,639,479);
			swnoir = 1;
			if(swconf == 0 || swDeselect)
			{
				setfillstyle(SOLID_FILL,WHITE);
				setcolor(BLACK);
				bararrondi(10,452,310,479,WHITE);
				bararrondi(330,452,630,479,WHITE);
				settextjustify(CENTER_TEXT, CENTER_TEXT);
				settextstyle( SMALL_FONT, HORIZ_DIR, 5);
				outtextxy(160,462, Message_point[49]);
				outtextxy(480,462, Message_point[50]);
				if(strstr(Message_point[50],"bevestigen"))
					outtextxy(573,462, "bevestigen"); 
				else if(strstr(Message_point[50],"confirmer"))
					outtextxy(501,462, "confirmer");
				swconf = 1;
				swpas = 1;
				swDeselect = 0;
			}
		}
	}
	else
	{
		if (((swpas == 0) && (visu == 0)) || swDeselect)
		{
			setfillstyle(SOLID_FILL, BLACK);
			bar(0,447,639,479);
			setfillstyle(SOLID_FILL, WHITE);

			bararrondi(110,452,530,479,WHITE);
			setcolor(BLACK);
			settextjustify(CENTER_TEXT, CENTER_TEXT);
			settextstyle( SMALL_FONT, HORIZ_DIR, 5);
			outtextxy(320,464, Message_point[48]);
			swpas = 1;
		}
		if(z == 0)
		{
			setfillstyle(SOLID_FILL, WHITE);
			floodfill(xc,yc, 8);
		}
		if((z > 0) && ((z1 > 0) || swDeselect))
			Inverse_Candid(ax1,ax2,ay1,ay2, 7, WHITE);
	}
	setcolor(BLACK);
	setfillstyle(SOLID_FILL, WHITE);
	settextstyle(SMALL_FONT, HORIZ_DIR, 4);
}

