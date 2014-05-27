/*******************************************************************/
#define  _Blanc_Exp   0
#define  _Vote        1
#define  _Pas_De_Vote 2
/*******************************************************************/

View()
{
	unsigned int Position;
	int  Param,VParam;
	char Nul_StrL[3],bufscr[2];
	int  i,Compt,j,tot;

	Nul_StrL[2] = 0x00;
	CHOIX = 0;
	Choix_College = 'F';
	for(Compt = _C_VOTE_PLACE, tot = 0,scr = 0; scr < nbrscrutin; scr++)
	{
		list=  cfgS[scrutin[scr].TYPE_SCRUTIN].nmxB;
		tot +=2;
		tot += list;
	}
	tot += _C_VOTE_PLACE;
	tot--;

	if(((carte[tot]) == 0x00)  || ((carte[tot + 1]) != 0x00))
	{
		msg_error = 12;
		ptfunct = fin;
		return(0);
	}

	for(Compt = _C_VOTE_PLACE, scr = 0; scr < nbrscrutin; scr++)
	{
	code_scr = (((scrutin[scr].TYPE_SCRUTIN) - 1) * 3);
	if(code_scr == 9)
	   {
		if(strcmp(Arrond,"005") == 0 ) code_scr+=1;
		if(strcmp(Arrond,"002") == 0 ) code_scr+=2;
	   }  
	strncpy(Nul_StrL,&carte[Compt] ,2);
	list = atoi(Nul_StrL);
	if((list < 98) && (list > (nbrlist[scr] - 1)))
	{
		msg_error = 12;
		ptfunct = fin;
		return(0);
	}

	ccol = 0;
	crl = 0;
	ps = scrutin[scr].ptlist;

	if((scrutin[scr].nom_s == '1'))
		ClOui = 1;
	else
		ClOui = 0;

	if((strcmp(Arrond,"002")==0) && (scrutin[scr].nom_s== '4'))
	{
		swRLg = 1;
		RLg = LANGUE;
		Choix_RLg = ' ';
		swCLG = 0;
	}
	else
	if((strcmp(Arrond,"002")==0) && ((scrutin[scr].nom_s == '1')
	|| (scrutin[scr].nom_s == '3')))
	{
	swRLg = 0;
	swCLG = 1;
	RLg = ' ';
	Choix_RLg = ' ';
	}
	else
	{
	swRLg = 0;
	RLg = ' ';
	Choix_RLg = ' ';
	swCLG = 0;
	}
	setfillstyle(SOLID_FILL,WHITE);
	bar(0, 0, 639, 479);
	setcolor( BLACK );
	settextstyle( SMALL_FONT, HORIZ_DIR, 8);
	settextjustify(CENTER_TEXT,CENTER_TEXT);
	code_scr = (((scrutin[scr].TYPE_SCRUTIN) - 1) * 3);
	if(code_scr == 9)
	   {
		if(strcmp(Arrond,"005") == 0 ) code_scr+=1;
		if(strcmp(Arrond,"002") == 0 ) code_scr+=2;
	   }  

	if((list == 0) || (list == 98))
		Param = _Pas_De_Vote;
	else
	   {     
		outtextxy( 320, 200, Message_point[code_scr]);
		outtextxy( 321, 200, Message_point[code_scr]);
		calmos(3);  /* Jamal le 19/10/2005 */
	    
		if(list == 99)
			Param = _Blanc_Exp;
		else
		{
			list--;
			ps = scrutin[scr].ptlist;
			ps += list;
			Choix_College = ps->college;
			ps = scrutin[scr].ptlist;
			Param = _Vote;
		}
	     }
	ps = scrutin[scr].ptlist;
	Compt +=2;
	if(Param == _Vote)
	{
		/* START CONFIDENTIAL */
		ps += list;
		for(z = 0,VParam = 0; (z < cfgS[scrutin[scr].TYPE_SCRUTIN].nmxV);Compt++)
		{
			tot = te[scr][list] + ts[scr][list] + 1;
			switch(carte[Compt])
			{
				case '1' :
					if(z > tot)
						VParam+=1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					break;
				case '2' :
					if(z > tot)
						VParam+=1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					break;
				case '3' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					break;
				case '4' :
					if(z > tot)
						VParam+=1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					break;
				case '5' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					break;
				case '6' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					break;
				case '7' :
					if(z > tot)
						VParam+=3;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					break;
				case '8' :
					if(z > tot)
						VParam+=1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					break;
				case '9' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					break;
				case 'A' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					break;
				case 'B' :
					if(z > tot)
						VParam+=3;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					break;
				case 'C' :
					if(z > tot)
						VParam+=2;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					break;
				case 'D' :
					if(z > tot)
						VParam+=3;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 1;
					break;
				case 'E' :
					if(z > tot)
						VParam+=3;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 0;
					break;
				case 'F' :
					if(z > tot)
						VParam+=4;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					memoc[scr][list][z++] = 1;
					break;
				default  :
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
					memoc[scr][list][z++] = 0;
			}
		}
		/* END CONFIDENTIAL */
		if(VParam > 0) /* Liste ne correspond pas */
		{
			msg_error = 12;
			ptfunct = fin;
			return(0);
		}

		affichc();
	}
	else
	{
		Compt += cfgS[scrutin[scr].TYPE_SCRUTIN].nmxB;
	}
	if (Param == _Blanc_Exp)
		view_vote_blanc();
	if((!klavier) && (passe > 0))
	{
		msg_error = 13;
		ptfunct = fin;
		return(0);
		}
	}

	msg_error = 16;
	ptfunct   = fin;
	return(0);
	}

/************************************************************************************/
/************************************************************************************/
view_vote_blanc()
{

/*      calmos(3); */
	setfillstyle(SOLID_FILL, BLACK);
	bar( 0,0,639,479);
	setcolor(WHITE);
	settextstyle( SMALL_FONT, HORIZ_DIR, 7);
	settextjustify( CENTER_TEXT, CENTER_TEXT );
	outtextxy(320, 10, Message_point[code_scr]);
	outtextxy(321, 10, Message_point[code_scr]);
	setfillstyle(SOLID_FILL, WHITE);
	bar(20,80,620,300);
	setcolor (BLACK);
	settextstyle( SMALL_FONT, HORIZ_DIR, 9);
	settextjustify ( CENTER_TEXT , CENTER_TEXT) ;
	outtextxy(320,190, Message_point[55]);
	outtextxy(320,191, Message_point[55]);
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
		#if (LPMNGLIB >=1)
		if (swLPcursor) lpPosxy(639, 479);
		#endif
		while((klavier) || (passe == 0))
		{
			#if (LPMNGLIB >=1)
			if (swLPcursor) lpCursorOn();
			#endif
			if (point())
			{
				#if (LPMNGLIB >=1)
				lpCursorOff();
				#endif
				if((cy > 452) && (cy < 479))
					return(0);
			}
		}
	}
}
/***************************************************************************/
/******                    CHOIX DE VISUALISER                        ******/
/***************************************************************************/
int choix_visu()
{
	setcolor (BLACK);

	rectangle(107,374,283,446);
	rectangle(108,375,282,445);

	rectangle(357,374,533,446);
	rectangle(358,375,532,445);

	settextstyle( SMALL_FONT, HORIZ_DIR, 8);
	settextjustify ( CENTER_TEXT , CENTER_TEXT ) ;

	outtextxy(195,407,Message_point[91]);
	outtextxy(196,407,Message_point[91]);

	outtextxy(445,407,Message_point[74]);
	outtextxy(446,407,Message_point[74]);

	while(1)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor) {
			lpCursorOn();
		}
		#endif
		if (point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if((cy > 375) && (cy < 445))
			{
				if((cx > 108) && (cx <  282))
				{
					visu = 0;
					return(0);
				}
				else
					if((cx >  358) && (cx < 532))
					{
						visu = 1;
						return(0);
					}
			}
		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
#endif
}

/****************************************************************************/
int choix_vote()
{
	setfillstyle(SOLID_FILL, WHITE);
	bar(107,274,293,346);
	bar(347,274,533,346);
	/* bararrondi(110,452,530,479,WHITE); */
	setcolor (BLACK);
	settextstyle( SMALL_FONT, HORIZ_DIR, 8);
	settextjustify ( CENTER_TEXT , CENTER_TEXT ) ;

	outtextxy(200,307,Message_point[124]);
	outtextxy(201,307,Message_point[124]);

	outtextxy(440,307,Message_point[125]);
	outtextxy(441,307,Message_point[125]);

	settextstyle( SMALL_FONT, HORIZ_DIR, 7);
	/*
	outtextxy(320,464, Message_point[126]);
	outtextxy(321,464, Message_point[126]);
	*/
	while(1)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor) {
			lpCursorOn();
		}
		#endif
		if (point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if((cy > 275) && (cy < 345))
			{
				if((cx > 108) && (cx <  292))
					return(1);
				else
					if((cx >  348) && (cx < 532))
						return(2);
			}
			/*       else
			if((cy > 452) && (cy < 479))
			if((cx > 108) && (cx <  530))
			return(3);*/

		}
	}
	#if (LPMNGLIB >=1)
	lpCursorOff();
	#endif
}

Dem_Verif()
{

	setfillstyle(SOLID_FILL, BLACK);
	bar( 0,0,639,479);
	setcolor(WHITE);
	settextstyle( SMALL_FONT, HORIZ_DIR, 12);
	settextjustify( CENTER_TEXT , CENTER_TEXT ) ;

	outtextxy(320,240,Message_point[129]);
	outtextxy(321,240,Message_point[129]);

	setfillstyle(SOLID_FILL, WHITE);
	bararrondi(110,452,530,479,WHITE);

	setcolor(BLACK);

	settextstyle(SMALL_FONT, HORIZ_DIR, 7);
	outtextxy(320,464, Message_point[92]);
	outtextxy(321,464, Message_point[92]);

	while(1)
	{
		#if (LPMNGLIB >=1)
		if (swLPcursor) lpCursorOn();
		#endif
		if (point())
		{
			#if (LPMNGLIB >=1)
			lpCursorOff();
			#endif
			if((cy > 452) && (cy < 479))
			if((cx > 108) && (cx <  530))
			{
				#if (LPMNGLIB >=1)
				lpCursorOff();
				#endif
				return(0);
			}
		}
	}
}
