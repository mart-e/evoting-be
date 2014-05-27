{
	int iSep;

	x_ent_scr = 400;
	x_numl    =  88;
	x_noml    = 183;
	Condense = 0;

	if (nbc_col == 14)
	{
		Lcadre_candid   = 165 + CDT_NUM_WIDTH;
		iSep            = (639-(Lcadre_candid*3))/4;
		Debut_Affichage =  47;
		x_dep           =  iSep;
		Hcadre_candid   =  25;
		Interl_candid   =   3;
		Interc_candid   = iSep;
		Diam            =   8;
		HAsaisie_candid = Hcadre_candid /4;
		LAsaisie_candid = Lcadre_candid;
		affTL_code      = 3;
		affColMax       = nombre_col;
		return(0);
	}
	else
	{
		if (nbc_col == 22)
		{
			Lcadre_candid   = 165 + CDT_NUM_WIDTH;
			iSep            = (639-(Lcadre_candid*3))/4;
			Debut_Affichage =  33;
			x_dep           =  iSep;
			Hcadre_candid   =  21;
			Interl_candid   =   1;
			Interc_candid   = iSep;
			Diam            =   8;
			HAsaisie_candid = Hcadre_candid /4;
			LAsaisie_candid = Lcadre_candid;
			affTL_code      = 3;
			affColMax       = nombre_col;
			return(0);
		}
	       else
		{
		 if (nbc_col == 23)
		 {
			Lcadre_candid   = 129 + CDT_NUM_WIDTH;
			iSep            = (639-(Lcadre_candid*3))/80;
			Debut_Affichage =  33;
			x_dep           =  2;
			Hcadre_candid   =  17;
			Interl_candid   =   1;
			Interc_candid   = 2;
			Diam            =   8;
			HAsaisie_candid = Hcadre_candid /4;
			LAsaisie_candid = Lcadre_candid;
			affTL_code      = 3;
			affColMax       = nombre_col;
			return(0);
		 }
	       }
	}
}

