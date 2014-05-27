#ifndef Setup_H
#define Setup_H

#ifdef DEBUG
#include <fstream.h>
#endif //DEBUG

#include "..\DIAGNOST\general.h"

// ============================= variables =============================
extern int		     _index;
extern BIOS_TEST     bios;
extern BIOS_VERSION  version;
extern BOOLEAN       BiosInfo;
extern unsigned char *AWARD_addr;
extern unsigned char *AWARD_flag;

typedef struct dataSerialNo{
	char Descr[80];
	char Value[80];
};

extern dataSerialNo SerialNo_D1561[];
extern dataSerialNo SerialNo_D1711[];
enum sn
{
	PcIdent, // PcIdentnumber
	PN,      // ProductName
	CSN,     // CustomSerialNo
	ChAT     // ChassisAssetTag
};

extern BOOLEAN snTested;
extern BOOLEAN snFailed;
extern int     snReturn;

//=============================================================================
//= D E B U G = D E B U G = D E B U G = D E B U G = D E B U G = D E B U G =====
//=============================================================================
#ifdef DEBUG
extern ofstream dFile;
extern char     dMsg[255];
extern char     dName[10];
extern BOOLEAN  ToFile;
#endif // DEBUG
//=============================================================================
//= D E B U G = D E B U G = D E B U G = D E B U G = D E B U G = D E B U G =====
//=============================================================================

// ============================= functions =============================

extern unsigned char GetByteFromCMOS( unsigned char Address);
extern void          SetByteFromCMOS( unsigned char Address, unsigned char Data);

#ifdef CMOS
extern unsigned char Calculate_Mod97( unsigned long ID);
#endif

extern BOOLEAN       forcedMachineID( char machine_name[9]);

extern void setDrvLetter( void);
#ifdef CMOS
extern BOOLEAN       GetMachineName( char machine_name[9],
									 BOOLEAN CMOSvalue);
#else  // !CMOS
extern void          GetMachineName( char machine_name[9]);
#endif // CMOS

extern void          WriteMachineName( char machine_name[9]);

extern void          GetBiosDate( char bios_date[9]);

extern unsigned      GetBaseMemorySize( void);
extern unsigned      GetExtMemorySize( void);
extern void          SetKeyboardCheck(BOOLEAN On);
extern void          SetConfig(BOOLEAN On);
extern BOOLEAN       IsShadowOn( void);
extern void          SetShadowOff();
extern BIOS_TEST     Bios_Test( void);
extern char          *RetrieveBiosVersion( BOOLEAN DetailInfo);

extern void          RebootMachine( void);
extern BOOLEAN       ResetMachineName( BIOS_TEST bios);
extern BOOLEAN       Check_Configuration();
extern unsigned char Determine_System( unsigned long ID);

#ifdef CMOS
extern BOOLEAN       Bureautica_Present( void);
extern BOOLEAN       Save_ID_To_File( void);
extern BOOLEAN       Restore_ID_From_File( void);
extern BOOLEAN       Save_RTC_To_File( char *Filename);
extern BOOLEAN       Restore_RTC_From_File( char *Filename);
#endif //CMOS

#ifdef YELLOW_DISK
extern BOOLEAN 		 Check_Yellow_Configuration( void);
#endif // YELLOW_DISK

#ifdef RTC_LOAD
extern BOOLEAN 		 Bureautica_Present( void);
extern BOOLEAN 		 Save_ID_To_File( void);
extern BOOLEAN 		 Restore_ID_From_File( void);
extern BOOLEAN 		 Restore_RTC_From_File( char *Filename);
#endif // RTC_LOAD

#ifndef RTC_LOAD
#ifndef CMOS
#ifndef TEST
extern void 		 VerifyIfDiagnosticDiskette( BOOLEAN Diagnostic);
#endif // RTC_LOAD
#endif // CMOS
#endif //TEST

#endif
