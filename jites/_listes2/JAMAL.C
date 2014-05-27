int    GraphDriver;		/* The Graphics device driver		*/
int    GraphMode = 2;		/* The Graphics mode value		*/
char *GetMode(void);
/******************** INITIALISATION DU DRIVER GRAPHIQUE *******************/
Initialize()
{
 int xasp, yasp;			/* Used to read the aspect ratio*/

 GraphDriver = 9;
 initgraph( &GraphDriver, &GraphMode, "" );
}
/***************************************************************************/
