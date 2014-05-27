/**************************************************************************/
/*************   S E L E C T I O N  D U  C O D E  M A S T E R    **********/
/**************************************************************************/

int nrclav,nbcol;
int posv[10][4];

/* << el2007 */
void Get_Jeton_IdCreation(char _jeton[16], char _idCreation[12])
{
	FILE *fp;
	char bufs[16];
	
	idCreation[0] = 0x00;


	/* _jeton */
	fp = fopen("machvots.tbl","r");
	if(fp == NULL)
		msg_erreur("machvots.tbl ERROR");
	fread(bufs,16,1,fp);
	strncpy(_jeton,bufs,16);
	_jeton[15] = '\0';
	fclose(fp);

	/* _idcreation */
	fp = fopen("a:1.dsk","r");
	if(fp == NULL)
	{
		fp = fopen("a:2.dsk","r");
		if(fp == NULL)
		{
			fp = fopen("a:3.dsk","r");
			if(fp == NULL)
			{
				fp = fopen("a:4.dsk","r");
				if(fp == NULL)
				{
					msg_erreur("?.dsk ERROR");
				}
			}
		}
	}
		
	fread(bufs,11,1,fp);
	strncpy(_idCreation,bufs,11);
	_idCreation[11] = '\0';
	strcpy(idCreation, _idCreation);
	fclose(fp);
}

/* el2007 >> */

int Aff_Clavier()
{
/* << el2007 */
 char cJeton[16], cIdCreation[12];
 char bufc[10];
/* el2007 >> */
 setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
 bar( 0,0,639,479);      /* Draw backgnd */
 setcolor( WHITE);
  
 settextstyle(SMALL_FONT, HORIZ_DIR,12);
 settextjustify(CENTER_TEXT, CENTER_TEXT);
 
 if(code_langue > '3')
   {
      outtextxy(160, 50, message_1[89]);
      outtextxy(161, 50, message_1[89]);
      outtextxy(480, 50, message_2[89]);
      outtextxy(481, 50, message_2[89]);
   }
  else
   {
      outtextxy(320, 50, message_1[89]);
      outtextxy(321, 50, message_1[89]);
   }
 
 settextstyle(SMALL_FONT, HORIZ_DIR, 6);
 setfillstyle(SOLID_FILL,7);

/* << el2007 */
 /* Identifiants de génération de disquette + bureau (arrondissement ...) */
 settextjustify(LEFT_TEXT,CENTER_TEXT);
 settextstyle( SMALL_FONT, HORIZ_DIR, 4);
 setcolor(WHITE);
 Get_Jeton_IdCreation(cJeton, cIdCreation);
 outtextxy(40, 430, cJeton);
 outtextxy(40, 440, cIdCreation);
/* el2007 >> */
 settextjustify(CENTER_TEXT,CENTER_TEXT);
 settextstyle( SMALL_FONT, HORIZ_DIR, 4);
 setcolor(WHITE);
#if (LPMNGLIB >=1)
  if (swLPcursor) 
	lpPosxy(639, 479);
#endif
#if (LPMNGLIB >=1)
		if (swLPcursor) 
			  lpCursorOn();
#endif


     for(y = 120,k = 0,j = 0 ; j < 4; y+= 75,j++)
       for(nbcol = 0, nrclav = 0,x = 210; (nbcol < 3) && (k < 11); k++ ,x+= 75,nbcol++ )
	{
	  posv[k][0]= x;
	 if(k == 10)
		  posv[k][1]= x + 140;
	 else
		  posv[k][1]= x + 65;
	  posv[k][2]= y ;
	  posv[k][3]= y + 65;
	  setfillstyle(SOLID_FILL,WHITE);        
	  bar(x,y,posv[k][1],posv[k][3]);
	  setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
	  settextjustify(CENTER_TEXT,CENTER_TEXT);
	  settextstyle(SMALL_FONT, HORIZ_DIR, 6);
	  if(k== 9)
	       strcpy(bufc,"0");
	  else
	       itoa(k+1,bufc,10);
	  setcolor(BLACK);
	  if(k < 10)
	      {  
		outtextxy(posv[k][0] + 10,posv[k][2] + 10,bufc);
		outtextxy(posv[k][0] + 11,posv[k][2] + 10,bufc);
	      }
	  else
	      {  
		if(code_langue > '3')
		{
		  outtextxy(posv[k][0] + 64,posv[k][2] + 16, message_1[94]);
		  outtextxy(posv[k][0] + 64,posv[k][2] + 48, message_2[94]);
		}
	       else
		 outtextxy(posv[k][0] + 64,posv[k][2] + 32, message_1[94]);
	     }  
	}

strcpy(fullPassword,"1111111111"); 
while(!Verifie_Password())
		PosClav();
#if (LPMNGLIB >=1)
		if (swLPcursor) 
			  lpCursorOff();
#endif

return(1);
}
/***************************************************************************/
/***************************************************************************/
PosClav()
{
 char bfs[2];       
 
 settextjustify(LEFT_TEXT,LEFT_TEXT);
 settextstyle(SMALL_FONT, HORIZ_DIR, 7);
 setcolor( WHITE);
 
 setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
 bar( 0,95,639,120);      /* Draw backgnd */
 strcpy(fullPassword,""); 

 for(x = 0;x < 10;)
	{
	   if( point())
	     {
	       for(i = 0; i < 11;i++)
		 {
		    if((cx > posv[i][0] + 2) && (cx < posv[i][1] - 2))
		       if ((cy > posv[i][2] + 2) && (cy < posv[i][3] - 2))
			 {
			  if(i == 10)
			    {
			      setfillstyle(SOLID_FILL,BLACK); /* Set to solid fill in color   */
			      bar( 0,95,639,120);      /* Draw backgnd */
			      strcpy(fullPassword,"");
			      x = 0;
			    }
			else
			   {
			     if(i == 9)
				     itoa(0,bfs,10);
			     else
				     itoa(i + 1,bfs,10);
			     bfs[1] = 0x00;
			     strcat(fullPassword,bfs);
			     x++;
			   }
		      }
		  outtextxy(265,110, fullPassword);
		}
	     }
	}

return(1);
}
/***************************************************************************/

