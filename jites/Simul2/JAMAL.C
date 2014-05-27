int    GraphDriver;		/* The Graphics device driver		*/
int    GraphMode = 2;           /* The Graphics mode value              */
char *GetMode(void);
/******************** INITIALISATION DU DRIVER GRAPHIQUE *******************/
Initialize()
{
	GraphDriver = 9;
	initgraph( &GraphDriver, &GraphMode, "" );
}
/***************************************************************************/
