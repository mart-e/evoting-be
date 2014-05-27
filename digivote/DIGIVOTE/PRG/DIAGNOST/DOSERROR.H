//=======================================================================

#ifndef DOSERRORS_H
#define DOSERRORS_H

#define NO_ERROR       -1
#define WRITE_PROTECTED 0
#define DRIVE_NOT_READY 2
#define DATA_ERROR_CRC  4

extern int DosErrorNr;
extern void InstallDosErrorHandler();

#endif
