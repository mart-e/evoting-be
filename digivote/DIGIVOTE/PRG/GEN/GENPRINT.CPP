#include <STDIO.H>
#include <CONIO.H>
#include <BIOS.H>
#include <STRING.H>
#include "..\gen\genprint.h"
#include "..\diagnost\general.h"
#include "..\diagnost\msg.h"

void ShowPrinterStatus( int status)
{
    char *Message;

    strcpy( Message, "");

	if( status&TIME_OUT    )
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _TIME_OUT));
	#else
	   strcat( Message, lstPrinterStatus[_TIME_OUT] );
	#endif
	if( status&IO_ERROR    )
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _IO_ERROR));
	#else
	   strcat( Message, lstPrinterStatus[_IO_ERROR] );
	#endif
	if( status&SELECTED    )
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _SELECTED));
	#else
	   strcat( Message, lstPrinterStatus[_SELECTED] );
	#endif
	if( status&OUT_OF_PAPER)
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _OUT_OF_PAPER));
	#else
	   strcat( Message, lstPrinterStatus[_OUT_OF_PAPER] );
	#endif
	if( status&ACKNOWLEDGE )
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _ACKNOWLEDGE));
	#else
	   strcat( Message, lstPrinterStatus[_ACKNOWLEDGE] );
	#endif
	if( status&NOT_BUSY    )
	#ifdef DIAGNOST
	   strcat( Message, Get_MsgIdx( PrinterStatus, _NOT_BUSY));
	#else
	   strcat( Message, lstPrinterStatus[_NOT_BUSY] );
	#endif
	printf( "%s\n", Message);
}

int RequestPrintStatus( int Port)
{
	int status, abyte=0;

    return( biosprint( PRINTERSTATUS, abyte, Port));
}

int InitializePrinter( int Port)
{
    int status, abyte=0;

    return( biosprint( INITIALIZE_PRINTER, abyte, Port));
}

int WriteToPrinter( int Port, char *Buffer)
{
    int i, status=SELECTED+NOT_BUSY;

    // Add NewLine & CarriageReturn at the end of each line to print
    for( i=0 ; i<strlen(Buffer) && status==SELECTED+NOT_BUSY; i++)
        status=biosprint( WRITE_TO_PRINTER, Buffer[i], Port);

    return( status);
}