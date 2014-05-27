//=======================================================================

#ifndef RESPONSA_H
	#define RESPONSA_H

#include "..\DIAGNOST\General.h";

extern unsigned char ZIPCode[5];
extern unsigned char Community[50];

extern void Get_Date( void);
extern void Get_Time( void);
extern BOOLEAN Acceptable_YEAR( void);
extern char *formatted_DateTime( char *separator);
extern BOOLEAN Set_Date( void);
extern BOOLEAN Set_Time( void);

extern BOOLEAN EditYear( int Heading, unsigned int Y);
extern BOOLEAN EditMonth( int Heading, unsigned int Y);
extern BOOLEAN EditDay( int Heading, unsigned int Y);
extern BOOLEAN EditHour( int Heading, unsigned int Y);
extern BOOLEAN EditMinutes( int Heading, unsigned int Y);

#ifndef RTC_LOAD
extern BOOLEAN EditPID( int Heading, unsigned int Y);

extern BOOLEAN EditOfficeNbr( int Heading, unsigned int Y);
extern BOOLEAN EditName( int Heading, unsigned int Y);
extern BOOLEAN EditStreet( int Heading, unsigned int Y);
extern BOOLEAN EditHouseNr( int Heading, unsigned int Y);
extern BOOLEAN EditZIPCode( int Heading, unsigned int Y);
extern BOOLEAN EditCommunity( int Heading, unsigned int Y);
extern BOOLEAN EditTelephoneNr( int Heading, unsigned int Y);
extern BOOLEAN EditExtension( int Heading, unsigned int Y);
extern BOOLEAN EditName( int Heading, unsigned int Y);
extern BOOLEAN CheckZIPCode( char *zip_to_check);
extern BOOLEAN Correct_ResponsableFile( void);
extern BOOLEAN Accept( int Heading, unsigned int Y);
#endif // RTC_LOAD

#endif // RESPONSA_H

//=======================================================================
