#ifndef GENPRINT_H
#define GENPRINT_H

#define TIME_OUT     0x01
#define IO_ERROR     0x08
#define SELECTED     0x10
#define OUT_OF_PAPER 0x20
#define ACKNOWLEDGE  0x40
#define NOT_BUSY     0x80

#define HP_RESET     "\033\105"             //HP1100
#define HP_SELFTEST  "\033\172"             //HP1100
#define HP_CR_LF     "\033\046\153\063\107" //HP1100

#define IBM_RESET    "\033\100"             //BULL CompuPrint
#define IBM_SELFTEST ""                     //BULL CompuPrint
#define IBM_CR_LF    "\033\012"             //BULL CompuPrint

#define PRINTER_BELL "\007"

enum PRTCMD {WRITE_TO_PRINTER, INITIALIZE_PRINTER, PRINTERSTATUS};
enum PRTPORT {LPT1, LPT2, LPT3};
enum PRTSTAT {_TIME_OUT, _IO_ERROR, _SELECTED,
              _OUT_OF_PAPER, _ACKNOWLEDGE, _NOT_BUSY};

void ShowPrinterStatus( int status);
int RequestPrintStatus( int Port);
int InitializePrinter( int Port);
int WriteToPrinter( int Port, char *Buffer);
#endif