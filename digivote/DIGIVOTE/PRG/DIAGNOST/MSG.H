//=======================================================================

#ifndef MSG_H
#define MSG_H

#include "..\diagnost\general.h"
#include "..\diagnost\messages.h"

#define noIndex -100
#define forceLF -255

extern int CentreX;

extern char MessageDirectory[];

extern void SetNormalText();

extern BOOLEAN WriteToFile;
extern char MessageDirectory[];     // Already filled by MSG.CPP
extern char fnMessage[];		    // To be filled by main application
extern char TraceLogDirectory[];    // Already filled by MSG.CPP
extern char fnTraceLog[];		    // To be filled by main application
extern unsigned int TitleBottom;	// Filled in by DrawTitle
extern SYSTEM _system;

#ifdef DIAGNOST

typedef struct
{
   int mnemonic;
   char *C_Code;
   char *S_Code;
   char *M_Code;
   char *text;
} MSG;

extern int htoi( char hex);
extern char *coded_version( void);
extern char *decoded_version( char *coded_version);

extern BOOLEAN kbconnected( void);
extern void GetKey();

extern char *Get_Code( int Mnemo);
extern char *Get_Msg( int Mnemo);
extern char **Get_Msgs( int Mnemo);
extern int Get_NbrMsgs( int Mnemo);
extern char *Get_MsgIdx( int Mnemo, int Index);
#else
#if !defined( CMOS) && !defined( TEST)
extern char *PrinterStatus[];
#endif // CMOS
#ifdef TEST
extern char*lstPrinterStatus[];
#endif
#endif

extern BOOLEAN Load_Messages( char *Language);
extern char *SelectedLanguage( void);

//Tables of Messages
//extern unsigned int DosNrErrorMsg;
//extern unsigned int CardReaderNrErrorMsg;
//extern unsigned int CardReaderNrCommands;

// Prints testing symbol (After heading)
extern void PrintTestingSymbol( unsigned int Y);
extern void EraseTestingSymbol( unsigned int Y);

extern BOOLEAN HandleDosErrors( void);

extern BOOLEAN IsDiagnost( void);
extern BOOLEAN IsDiagnostic( void);
extern BOOLEAN IsWriteProtected( void);
extern BOOLEAN IsDiskInserted( char *Drive );

extern void Write_Checked( FILE *FileID, char *Text);

extern BOOLEAN ResultLogHeader( int Heading, unsigned int Y, char MachineID[]);
extern void WriteInFile( int Heading, int Mnemonic, char Message[],
						 BOOLEAN AddMessageCode);
// Prints test result (After heading)
extern BOOLEAN PrintResult( int Heading, unsigned int Y, int Mnemonic, char Message[]);
extern BOOLEAN PrintTestResult( int Heading, unsigned int Y, int Mnemonic);
extern void EraseTestResult( unsigned int Y);
// Prints a dos error as test result (After heading)
extern BOOLEAN PrintDosError( int Heading, unsigned int Y);
// Prints a card reader error as test result (After heading)
extern BOOLEAN PrintCardReaderError( int Heading, unsigned int Y, char Response[]);
extern void Log_CardReader_Trace( void);
// Prints the heading
extern void PrintHeading( int Heading, unsigned int Y, BOOLEAN Bold);
// Draws a message in the middle of the screen
extern void DrawMessage( char Message[], unsigned int Size);
// Draws a button
extern void DrawButton( unsigned int X, unsigned int Y,
						unsigned int ButtonWidth, unsigned int ButtonHeight,
						char Message[], unsigned int Size, BOOLEAN HighLighted);
extern void EraseButton( unsigned int X, unsigned int Y,
						 unsigned int ButtonWidth, unsigned int ButtonHeight);
extern int SelectItem( char *Instructions, int Items,
					   unsigned int Y);
// Erases the complete screen
extern void EraseScreen();
// Draws an instruction at the bottom of the screen (above timescale)
extern void DrawAttention( char Instruction[]);
extern void DrawInstruction( char Message[]);
extern void EraseInstruction( char Message[]);
extern void EditInstruction( unsigned char *Instruction,
							 unsigned char *String,
							 short StartPosition,
							 short StopPosition,
							 BOOLEAN Overwrite,
							 BOOLEAN EscapeAllowed,
							 BOOLEAN NumericPad,
							 int (*Func)( char *string ));
// Draws a remark (Big instruction) at the bottom of the screen.
extern void DrawRemark( char Remark[]);
extern void EraseRemark( char Remark[]);
// Draws a title in a box at the top of the screen.
extern void DrawTitle( char Title[]);
extern BOOLEAN DrawPrinterTest( int Heading, unsigned int Y);
extern BOOLEAN DrawMemoryTest( int Heading, unsigned int Y);
// extern void TestsEndedAndSucceeded(BOOLEAN X25Result);
extern void TestsEndedAndSucceeded( void);
extern void ResetFiles( void);
extern int LightpenUsed( unsigned int*, unsigned int* );
extern void LogBIOSCheckpoints( void );
#endif
extern void KeepTrackOfTime( BOOLEAN InitializeTimer);
extern void FinalizeTrackOfTime( void);
extern BOOLEAN CheckOffTime(  BOOLEAN CheckOnly);
