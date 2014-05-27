#define LISTE_BOULE 1
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
/* Hsaisie_candid= hauteur de la zone de saisie dans cadre candidat        */
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
#define true  1
#define false 0
int candidtmp,nbc_cols,Diam,Condense;
char bufc[10],bufname[50];
int Tampon,Clon;
struct palettetype palette;  /*pour lire les infos de la palette*/

FILE *ptc, *fpc;
union REGS rin,rout;

void affiche_num_cdt(int _iNum)
{
	char c[3];

	/* Separateur */
	setfillstyle(SOLID_FILL,BLACK);
	bar(x+CDT_NUM_WIDTH-1,y,x+CDT_NUM_WIDTH-1,posc[k][3] + HAsaisie_candid);
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

	iNbreDeSiegeAPourvoir = nbr_max - nbr_sup;
	iNbreEffectif = te;
	if(iNbreDeSiegeAPourvoir <= iRuptureColonne)
		iNbreColonne = 1;
	else
	{
	   if(iNbreEffectif <= iRuptureColonne)
		iNbreColonne = 1;
	   else
		iNbreColonne = iNbreEffectif/iRuptureColonne;
	 }

	if((iNbreColonne * iRuptureColonne) <  iNbreEffectif)
			iNbreColonne++;

	if(iNbreColonne == 1)
		(*_nb_cdt_par_colonne) = iNbreEffectif;
	else
	 {
	   if(iNbreColonne < 4)
	     {
		(*_nb_cdt_par_colonne) = iNbreEffectif / iNbreColonne;
		if((*_nb_cdt_par_colonne * iNbreColonne) < iNbreEffectif)
			++(*_nb_cdt_par_colonne);
	     }
	   else
		*_nb_cdt_par_colonne =  iRuptureColonne;
	  }
}

int affichc()
{
int m,status, val,nbccols;
int nombre_col_bis,j;
int iNumCdt;
int nb_cdt_par_colonne;

strcpy(Sname,f_name);
for(j = strlen(Sname)-1; j >= 0; j--)
   {
     if(Sname[j] == '.')
	 {
	   Sname[j] = 0x00;
	   break;
	 }
   }
/**************************   ENTETE CANDIDAT  ****************************/

setfillstyle(SOLID_FILL,BLACK); 
bar( 0,0,639,479);     
setcolor( WHITE);
settextstyle(SMALL_FONT, HORIZ_DIR, 6);

if(LISTE_BOULE)
 {
    setfillstyle(SOLID_FILL,7);
    if(affTL_code == 1)
	     posc[0][0] = 320 - 25;
    else
      {
	if (affTL_code == 2)
		posc[0][0] = 80;
	else
	  {
	   if (affTL_code == 3)
		   posc[0][0] = 72;
	   else
	     {
	      if (affTL_code == 4)
		   posc[0][0] = 52;
	      else
		  if (affTL_code == 5)
			 posc[0][0] = 52;
	     }
	  }
       }
    posc[0][1] = posc[0][0] + 50;
    posc[0][2] = 2;
    posc[0][3] = 27;
    bar(posc[0][0] + 10 ,posc[0][2], posc[0][1] - 10 , posc[0][3]);
    cercle(0);
 }
else
 {
   posc[0][0] = 52;
   posc[0][1] = posc[0][0] + 10;
   posc[0][2] = 2;
   posc[0][3] = 27;
  }
/**************************   ENTETE CANDIDAT  ****************************/

 setfillstyle(SOLID_FILL,WHITE); /* Set to solid fill in color   */
 setcolor(WHITE);
 settextjustify(RIGHT_TEXT,CENTER_TEXT);  /* */
 settextstyle( SMALL_FONT, HORIZ_DIR, 5);
 outtextxy( 630, 13, Msg1);
 outtextxy( 629, 13, Msg1);

 settextstyle( SMALL_FONT, HORIZ_DIR, 6);
 if (affTL_code == 1)
	{
		settextjustify(LEFT_TEXT,CENTER_TEXT);
		outtextxy(  5, 13, Msg2);
		outtextxy(  6, 13, Msg2);
		outtextxy(  5, 14, Msg2);
		outtextxy(  6, 14, Msg2);
		if(access(strcat(Sname,".bmp"),0) == 0)
			bmp(Sname,posc[0][0] -10, 2,0);
		else
		{
			settextjustify(RIGHT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][0] -10, 13, Msg3);
			outtextxy( posc[0][0] -11, 13, Msg3);
			outtextxy( posc[0][0] -10, 14, Msg3);
			outtextxy( posc[0][0] -11, 14, Msg3);
		}
	}
	else
	if (affTL_code == 2)
	{
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, Msg2);
		outtextxy( posc[0][0] -11, 13, Msg2);
		outtextxy( posc[0][0] -10, 14, Msg2);
		outtextxy( posc[0][0] -11, 14, Msg2);

		if(access(strcat(Sname,".bmp"),0) == 0)
			bmp(Sname,posc[0][1] + 10, 2,0);
		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, Msg3);
			outtextxy( posc[0][1] +11, 13, Msg3);
			outtextxy( posc[0][1] +10, 14, Msg3);
			outtextxy( posc[0][1] +11, 14, Msg3);
		}
	}
	else
	    if (affTL_code == 3)
	     {
		if(!LISTE_BOULE)
		{
			setfillstyle(SOLID_FILL,WHITE);
			bar(5,1,210,28);
			setcolor(BLACK);
		}
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, Msg2);
		outtextxy( posc[0][0] -11, 13, Msg2);
		outtextxy( posc[0][0] -10, 14, Msg2);
		outtextxy( posc[0][0] -11, 14, Msg2);

		if(access(strcat(Sname,".bmp"),0)==0)
			bmp(Sname,posc[0][1] + 10, 2,0);
		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, Msg3);
			outtextxy( posc[0][1] +11, 13, Msg3);
			outtextxy( posc[0][1] +10, 14, Msg3);
			outtextxy( posc[0][1] +11, 14, Msg3);
		}
		if(!LISTE_BOULE)
			setcolor(WHITE);
	}
	else if (affTL_code == 4) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, Msg2);
		outtextxy( posc[0][0] -11, 13, Msg2);
		outtextxy( posc[0][0] -10, 14, Msg2);
		outtextxy( posc[0][0] -11, 14, Msg2);

		if(access(strcat(Sname,".bmp"),0)==0)
			bmp(Sname,posc[0][1] + 10 , 2,0);
		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, Msg3);
			outtextxy( posc[0][1] +11, 13, Msg3);
			outtextxy( posc[0][1] +10, 14, Msg3);
			outtextxy( posc[0][1] +11, 14, Msg3);
		}
	}
	else if (affTL_code == 5) {
		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		outtextxy( posc[0][0] -10, 13, Msg2);
		outtextxy( posc[0][0] -11, 13, Msg2);
		outtextxy( posc[0][0] -10, 14, Msg2);
		outtextxy( posc[0][0] -11, 14, Msg2);

		if(access(strcat(Sname,".bmp"),0)==0)
			bmp(Sname,posc[0][1] + 10, 2,0);
		else
		{
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( posc[0][1] +10, 13, Msg3);
			outtextxy( posc[0][1] +11, 13, Msg3);
			outtextxy( posc[0][1] +10, 14, Msg3);
			outtextxy( posc[0][1] +11, 14, Msg3);
		}
	}

	line(0,30,639,30);
	/**************                AFFICHAGE DES CANDIDATS        ***************/

	settextjustify(CENTER_TEXT,CENTER_TEXT);
	settextstyle( SMALL_FONT, HORIZ_DIR, 4);
	setcolor( BLACK);

	candid = te + ts;
	type_ecran(&nb_cdt_par_colonne);
	for(k=1,x = x_dep;(k < te + 1);x+= Lcadre_candid + Interc_candid)
	{
	  for(j = 0,y=Debut_Affichage;(k < te + 1) && j < nb_cdt_par_colonne;y+= Hcadre_candid + Interl_candid,k++,j++)
		{
			posc[k][0]= (x + Lcadre_candid) - LAsaisie_candid;
			posc[k][1]= (x + Lcadre_candid) - 1;
			posc[k][2]=  y + HAsaisie_candid;
			posc[k][3]= (y + Hcadre_candid) - 1 - HAsaisie_candid;

			setfillstyle(SOLID_FILL,WHITE);
			bar(x,y,posc[k][1],posc[k][3] + HAsaisie_candid);
			Cal_affiche();
			affiche_num_cdt(k);
			fc++;
		}
	}

	nbc_cols = ts  + 1;

	settextstyle(SMALL_FONT, HORIZ_DIR, 6);
	setcolor( WHITE);

	if ((ts > 0) && ( te < 70))
	{
		x = 639 -( x_dep + Lcadre_candid);
		y = Debut_Affichage;
		j = 1;
		outtextxy(x + (Lcadre_candid / 2 ) ,y + ((Hcadre_candid/2) - Interl_candid),Msg4);
		outtextxy(x + (Lcadre_candid / 2 ) + 1 ,y + ((Hcadre_candid/2) - Interl_candid),Msg4);
		settextstyle( SMALL_FONT, HORIZ_DIR, 4);
		setcolor( BLACK);
	}
	else
		if((ts > 0) && ( te > 69))
		{
			j = j++;
			x-= Lcadre_candid + Interc_candid;
			outtextxy(x + (Lcadre_candid / 2 ) ,y + ((Hcadre_candid/2) - Interl_candid),Msg4);
			outtextxy(x + (Lcadre_candid / 2 ) + 1 ,y + ((Hcadre_candid/2) - Interl_candid),Msg4);
			settextstyle( SMALL_FONT, HORIZ_DIR, 4);
			setcolor( BLACK);
		}

	for(;(k < candid + 1);x+= Lcadre_candid + Interc_candid,y =  0,j = 0)
		for(iNumCdt=1,  y += ((Hcadre_candid + Interl_candid)) ;(k < candid + 1) ;y+= Hcadre_candid + Interl_candid,k++,j++,iNumCdt++)
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
			affiche_num_cdt(iNumCdt);
			fc++;
		}

	val = true;
	while(val)
	{
		setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
		bar( 0,447,639,479);      /* Draw backgnd */
		setcolor( WHITE);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		settextstyle( SMALL_FONT, HORIZ_DIR, 6);
		outtextxy( 322,470, Msg5);

		if(!swPcxOnly)
			c = getch();
		else
			c = 'F';

		if (c == 'P' ||  c == 'p' || c =='I' || c == 'i')
		{
			rin.h.ah = 2;
			rin.x.dx = 0;
			int86(0x17,&rin,&rout);
			if(rout.h.ah != 0x90)
			{
				setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
				bar( 0,438,639,479);      /* Draw backgnd */
				setcolor( WHITE);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				settextstyle(SMALL_FONT, HORIZ_DIR, 6);
				outtextxy( 320,470, "Imprimante occupe, pressez une touche.");
				c = getch();
			}
			else
			{
				int86(0x05,&rin,&rout);
				ptc = fopen("PRN","w");
				bufc[0]  = 0x0C;
				bufc[1] = 0x00;
				fputs(bufc,ptc);
				fclose(ptc);
				val = false;
			}
		}
		else
		if (c == 'F' ||  c == 'f')
		{
		  pcxDisplayFile(bufnom,0,0,639,479,0);
		  if(swPcxOnly)
			  return;
		}
		else
			val = false;
	}
}

/***************/
int affichd()
/***************/
{
	int c, diam;
	int x1, x2, y1, y2, val;
	int j, nombre_col_bis;
	int nb_cdt_par_colonne;
	strcpy(Sname,f_name);
	for(j = strlen(Sname)-1; j >= 0; j--)
	{
		if(Sname[j] == '.')
		{
			Sname[j] = 0x00;
			break;
		}
	}
	setfillstyle(SOLID_FILL,WHITE);
	bar(0,0,639,479);

	if(1)
	{
		/* S�arateur */
		setfillstyle(SOLID_FILL,BLACK);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		setcolor(BLACK);
		bar(0,42,639,42);

		/* Boule */
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		setcolor(BLACK);
		bar(84,2,128,40);
		x = 84+(128-84)/2;
		y = 2+(40-2)/2;
		diam = 6;

		settextstyle(SMALL_FONT, HORIZ_DIR, 6);
		setcolor(WHITE);
		circle(x ,y, diam);
		setcolor(BLACK);
		setfillstyle(SOLID_FILL, WHITE);
		floodfill(x,y, WHITE);
		settextstyle( SMALL_FONT, HORIZ_DIR, 4);

		/* Id liste */
		settextstyle(SMALL_FONT, HORIZ_DIR, 11);
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		outtextxy( 41, 15, Msg2);

		/* Nom liste */
		if(access(strcat(Sname,".bmp"),0)==0)
			bmp(Sname,156, 10, 0);
		else
		{
			settextstyle(SMALL_FONT, HORIZ_DIR, 7);
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy( 156, y, Msg3);
		}

		settextjustify(RIGHT_TEXT,CENTER_TEXT);
		settextstyle( SMALL_FONT, HORIZ_DIR, 5);
		outtextxy( 630, 13, Msg1);
		outtextxy( 629, 13, Msg1);
	}

	if(1)
	{
		x_dep = 2;
		Lcadre_candid = 199;
		LAsaisie_candid = 0;
		HAsaisie_candid = 0;
		Interc_candid = 19;
		
		if(nbc_col <= 14)
			Hcadre_candid = 28;
		else
			if(nbc_col <= 22)
				Hcadre_candid = 19;
			else
			  {
			       Lcadre_candid = 157;
			       Hcadre_candid = 17;
			       Interc_candid = 2;
			  }

		candid = te + ts;
		type_ecran(&nb_cdt_par_colonne);
		setfillstyle(SOLID_FILL,BLACK);
		settextstyle( SMALL_FONT, HORIZ_DIR, 4);
		for(k=1,x = x_dep;(k < te + 1);x+= Lcadre_candid + Interc_candid)
		{
			for(j = 0,y=45;(k < te + 1) && j < nb_cdt_par_colonne;y+= Hcadre_candid,k++,j++)
			{
				posc[k][0]= (x + Lcadre_candid) - LAsaisie_candid;
				posc[k][1]= (x + Lcadre_candid) - 1;
				posc[k][2]=  y + HAsaisie_candid;
				posc[k][3]= (y + Hcadre_candid) - HAsaisie_candid;
				bar(x,y,posc[k][1],y);
				bar(x,posc[k][3] + HAsaisie_candid,posc[k][1],posc[k][3] + HAsaisie_candid);
				bar(x,y,x,posc[k][3] + HAsaisie_candid);
				bar(posc[k][1],y,posc[k][1],posc[k][3] + HAsaisie_candid);

				Cal_affiche();
				affiche_num_cdt(k);

				fc++;
			}
		}

		/* Suppl�nts */
		
		if((ts > 0) && ( te > 69))
		{
			x-= Lcadre_candid + Interc_candid;
		}
		else
		     {          
			x = 639 - (x_dep+Lcadre_candid);
			y = 45;
		     }
		
		for(j=0;(k <= candid + 1);y+= Hcadre_candid,k++,j++)
		{
			posc[k][0]= (x + Lcadre_candid) - LAsaisie_candid;
			posc[k][1]= (x + Lcadre_candid) - 1;
			posc[k][2]=  y + HAsaisie_candid;
			posc[k][3]= (y + Hcadre_candid) - HAsaisie_candid;
			bar(x,y,posc[k][1],y);
			bar(x,posc[k][3] + HAsaisie_candid,posc[k][1],posc[k][3] + HAsaisie_candid);
			bar(x,y,x,posc[k][3] + HAsaisie_candid);
			bar(posc[k][1],y,posc[k][1],posc[k][3] + HAsaisie_candid);

			if(j)
			{
				Cal_affiche();
				affiche_num_cdt(j);

				fc++;
			}
			else /* Suppl�nt */
			{
				setfillstyle(SOLID_FILL, 7);
				bar(x,y,posc[k][1],posc[k][3] + HAsaisie_candid);
				setfillstyle(SOLID_FILL, BLACK);
				bar(x,y,posc[k][1],y);
				bar(x,posc[k][3] + HAsaisie_candid,posc[k][1],posc[k][3] + HAsaisie_candid);
				bar(x,y,x,posc[k][3] + HAsaisie_candid);
				bar(posc[k][1],y,posc[k][1],posc[k][3] + HAsaisie_candid);

				settextstyle( SMALL_FONT, HORIZ_DIR, 6);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				outtextxy(x+(posc[k][1]-x)/2  +1,(y + Hcadre_candid/2) -2, Msg4);
				outtextxy(x+(posc[k][1]-x)/2  +1,(y + Hcadre_candid/2) -1, Msg4);

				settextstyle( SMALL_FONT, HORIZ_DIR, 4);
			}
		}
	}

	val = true;
	while(val)
	{
		settextjustify(CENTER_TEXT,CENTER_TEXT);
		settextstyle( SMALL_FONT, HORIZ_DIR, 6);
		outtextxy( 322,470, Msg5);

		if(!swPcxOnly)
			c = getch();
		else
			c = 'F';

		if (c == 'P' ||  c == 'p' || c =='I' || c == 'i')
		{
			rin.h.ah = 2;
			rin.x.dx = 0;
			int86(0x17,&rin,&rout);
			if(rout.h.ah != 0x90)
			{
				setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
				bar( 0,438,639,479);      /* Draw backgnd */
				setcolor( WHITE);
				settextjustify(CENTER_TEXT,CENTER_TEXT);
				settextstyle(SMALL_FONT, HORIZ_DIR, 6);
				outtextxy( 320,470, "Imprimante occupe, pressez une touche.");
				c = getch();
			}
			else
			{
				int86(0x05,&rin,&rout);
				ptc = fopen("PRN","w");
				bufc[0]  = 0x0C;
				bufc[1] = 0x00;
				fputs(bufc,ptc);
				fclose(ptc);
				val = false;
			}
		}
		else
		if (c == 'F' ||  c == 'f')
		{
		   pcxDisplayFile(bufnom,0,0,639,479,0);
		   if(swPcxOnly)
			   return;
		}
		else
			val = false;
	}
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
			outtextxy(x+CDT_NUM_WIDTH  +1,(y + Hcadre_candid/3) -2, fc->nom_c);
			settextjustify(CENTER_TEXT,CENTER_TEXT);
			break;
		case 2 :
			settextjustify(LEFT_TEXT,CENTER_TEXT);
			outtextxy(x+CDT_NUM_WIDTH  +1,(y + Hcadre_candid/3) -2, fc->nom_c);
			settextjustify(CENTER_TEXT,CENTER_TEXT);
			outtextxy(x +((CDT_NUM_WIDTH+Lcadre_candid) / 2 ),y + ((Hcadre_candid * 2) / 3) , fc->prenom_c);
			break;
	}
}
/***************************************************************************/
/******          MENU DE CONFIRMATION DU CHOIX DU CANDIDAT + CERCLE   ******/
/***************************************************************************/
cercle(z)
int z;
{
	int xc,yc,diam;
	xc = posc[z][1];

	if(z == 0)
	{
		xc -= (posc[z][1] - posc[z][0] ) / 2;
		yc = 15;
		diam = 11;
		settextstyle(SMALL_FONT, HORIZ_DIR, 6);
		setcolor(8);
		circle(xc ,yc, diam);
		setcolor(BLACK);
		setfillstyle(SOLID_FILL, WHITE);
		floodfill(xc,yc, 8);
		settextstyle( SMALL_FONT, HORIZ_DIR, 4);
	}
}
