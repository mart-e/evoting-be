/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="digimenu.cpp"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.6 $</revision>
  <archiveTimeStamp>$Date: 2008/05/13 13:26:20 $</archiveTimeStamp>
  <pvcsUser>$Author: jru $</pvcsUser>
</pvcs>
<comments>
  <fileName>digimenu.cpp</fileName>
  <creationDate/>
  <rights/>
  <author/>
  <description>
    <fr>menu PRP/TOT</fr>
    <nl>PRP/TOT menu</nl>
  </description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

//------------------------------------------------------ PRIVATE FUNCTIONS ---
#include <io.h>
#include <dir.h>
#include <dos.h>
#include <alloc.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <sys\stat.h>

#include "logging.h"
//#include "..\digest\ddige.h"

// Dir and Filenames
////////////////////
#define _CFG_file   "MENU.CFG"
#define _INI_file   "MENU.INI"

#define RootDir     "\\VOTE"
#define RunDir      "\\VOTE\\RUN"
#define TempDir     "\\VOTE\\TEMP"
#define SaveDir     "\\VOTE\\SAVE"
#define SpoolDir    "\\VOTE\\SPOOL"
#define TraceLogDir "\\VOTE\\TRACE.LOG"
#define BatDir      "\\VOTE\\FILES\\BAT"
#define ResultsDir  "\\VOTE\\RESULTS"

#define ShellBat    "\\VOTE\\TEMP\\SHELL.BAT"
#define dBLogFile   "\\VOTE\\ELECT.LG"
#define UpdateLog   "\\VOTE\\UPDATE.LOG"
#define LibBat      "\\VOTE\\RUN\\LIB.BAT"

// TRAP DOS ERRORS
//////////////////
#define IGNORE  0
#define RETRY   1
#define ABORT   2

// KEY DEFINITIONS
//////////////////
#define C_ESC        27
#define C_F3        317
#define C_F4        318
#define C_SHIFT_F4  343
#define C_F5        319
#define C_F7        321
#define C_F10       324
#define C_DOWN      336
#define C_UP        328
#define C_PGDN      337
#define C_PGUP      329
#define C_HOME      327
#define C_END       335
#define C_LEFT      331
#define C_RIGHT     333
#define C_SPACEBAR   32
#define C_ENTER      13

// ACCESS MODES
///////////////
#define READ_WRITE 06
#define READ_ONLY  04
#define EXISTS     00

/***************************************************************************/
/*                                                                         */
/*                   Global declaration of variables                       */
/*                                                                         */
/***************************************************************************/
typedef enum BOOLEAN       { FALSE,          TRUE           };
typedef enum COPY_MOVE     { COPY_FILE,      MOVE_FILE      };
typedef enum DATE_TIME     { NEW_DATETIME,   KEEP_DATETIME  };
typedef enum MUST_EXIST    { IF_FILE_EXISTS, FILE_MUST_EXIST};
typedef enum ATTRIB_HIDDEN { VISIBLE,        HIDDEN         };

char *DOScmd, *LogMessage;
char *RAMdrive;

char Buffer[80];

BOOLEAN JumpToFunction    = FALSE;
BOOLEAN Cancelled         = FALSE;
BOOLEAN ForcedExit        = FALSE;
BOOLEAN Notify_DOS_Errors = TRUE;
int DOS_Error = -1;

time_t TimerTime;

/* define the error messages for trapping disk problems */
static char *err_msg[] = {
    "WRITE PROTECT",
    "UNKNOWN UNIT",
    "DRIVE NOT READY",
    "UNKNOWN COMMAND",
	"DATA ERROR (CRC)",
	"BAD REQUEST",
    "SEEK ERROR",
    "UNKNOWN MEDIA TYPE",
    "SECTOR NOT FOUND",
    "PRINTER OUT OF PAPER",
    "WRITE FAULT",
    "READ FAULT",
    "GENERAL FAILURE",
    "RESERVED",
    "RESERVED",
	"INVALID DISK CHANGE"
};

// SCREEN INFORMATION
/////////////////////
typedef enum MODEtype { RESET, NORMAL, HIGHLIGHT,
                               REVERSE, HIGHREVERSE } MODE;

typedef enum STYLEtype { SINGLE, DOUBLE, MIXED1, MIXED2 } STYLE;
static char ulc[] = { 218, 201, 213, 214 };
static char urc[] = { 191, 187, 184, 183 };
static char llc[] = { 192, 200, 212, 211 };
static char lrc[] = { 217, 188, 190, 189 };
static char hl[]  = { 196, 205, 205, 196 };
static char vl[]  = { 179, 186, 179, 186 };

// SCREEN & BIOS INFORMATION
////////////////////////////
#define NbrBUTTOMS 9
static struct BUTTOMtype {
   int X;                           // Position on ButtomBar
   char ShortCut[10];               // ShortCut Key(s)
   BOOLEAN FirstChar;               // Is first char sensitive or not ?
   char *Comment;                   // Text to display on ButtomBar
} BUTTOM[ NbrBUTTOMS ] =
{
  {  3, { 0x12, '\0'               }, FALSE, "Move Selection"             },
  { 22, { 'E','N','T','E','R','\0' }, FALSE, "Choose"                     },
  { 52, { 'F','5','\0'             }, FALSE, "Menu Refresh"               },
  { 70, { 'F','1','0','\0'         }, FALSE, "Exit"                       },
  {  3, { '\0'                     }, FALSE, "DOS Mode ... Executing ..." },
  {  3, { 0x12, 0x1D, '\0'         }, FALSE, "Move Position"              },
  { 21, { '\0'                     }, TRUE,  "Error"                      },
  { 27, { '\0'                     }, TRUE,  "Not OK"                     },
  { 70, { 'E','S','C','\0'         }, TRUE,  "Back"                       }
};
/* 0x12 : Up and Down arrow    */
/* 0x1D : Left and Right arrow */

#define MAINmenu   0
#define DOSmenu    1
#define LOGmenu    2
#define dBLOGmenu  3
static char *StatusBar[] = {
  { "111100000" },           // Main   Menu
  { "000010001" },           // DOS    Menu
  { "000001111" },           // LOG    Menu
  { "000001001" }            // dBLOG  Menu
};

// SUPPLEMENTARY MENU INFORMATION
/////////////////////////////////
typedef struct MenuChoice *ChoiceList;

struct MenuChoice {
   char *Text;                 // Text shown on screen
   char HotKey;                // HotKey to start menu option
   char *DOSCommand;           // Command to execute in DOS-window
   char *Parameters;           // Command parameters (if present)
   char *DescNL;               // Short Description in Dutch
   char *DescFR;               // Short Description in French
   char *DescGE;               // Short Description in German

   ChoiceList Next;            // Link to the next Choice
   ChoiceList Prev;            // Link to the prev Choice
};

ChoiceList FirstChoice = NULL;
int NbrOfItems = 0;

typedef struct sLineInfo *LineInfo;
struct sLineInfo {
    long offset;                 // Position of Line in File
    BOOLEAN _NOK;
    BOOLEAN _ERR;

    LineInfo Next;               // Link to the next Line in File
    LineInfo Prev;               // Link to the prev Line in File

	LineInfo Next_NOK;           // Link to the next Line in File with <NOK>
    LineInfo Prev_NOK;           // Link to the prev Line in File with <NOK>

    LineInfo Next_ERR;           // Link to the next Line in File with <ERR>
    LineInfo Prev_ERR;           // Link to the prev Line in File with <ERR>
};
LineInfo FirstLine = NULL,
         LastLine  = NULL;

// PROTOTYPING OF LOCAL FUNCTIONS
/////////////////////////////////
void ClearScreen( void);
void DrawBox( int TX, int TY, int BX, int BY, STYLE Style);
void SetMODE( MODE mode);
void WriteOnScreen( int X, int Y, char *Text, MODE mode);
void WriteCentered( int Y, char *Text, MODE mode);
void WriteLeftAssigned( int Y, char *Text, MODE mode);
void WriteRightAssigned( int Y, char *Text, MODE mode);
void HideCursor( void);
void PlaySound();
int GetKey( BOOLEAN Hide);
int Wait_For_SPACEBAR_or_ESC( void);
char *Alloc_String( char *String, int Length);
BOOLEAN Make_Hidden( char *Filename);

// TRAP DOS ERRORS / HARDWARE ERRORS
////////////////////////////////////
int error_win( char *msg)
{
    int retval;

    if( !Notify_DOS_Errors)
        retval = IGNORE;
    else
    {
        cprintf( msg);
        // prompt for user to press a key to abort, retry, ignore
        while( TRUE)
        {
            switch( ( retval = GetKey( FALSE)))
            {
                case 'r':
                case 'R':
                    retval = RETRY;
                    break;

                case 'i':
                case 'I':
                    retval = IGNORE;
                    break;
            }
            if( retval == RETRY ||
                retval == IGNORE)
                break;
        }
        cprintf( "\r\n");
    }
    return( retval);
}

// pragma warn -par reduces warnings which occur
// due to the non use of the parameters errval,
// bp and si to the handler.
#pragma warn -par
int handler(int errval,int ax,int bp,int si)
{
    static char msg[80];
    unsigned di;
    int drive;
    int errorno;

    di= _DI;
	/* if this is not a disk error then it was */
    /* another device having trouble           */
    if (ax < 0)
    {
       /* report the error */
       error_win( "Device error - Aborting ...");
       /* and return to the program directly requesting abort */
       hardretn( ABORT);
    }
    /* otherwise it was a disk error */
    drive     = ax & 0x00FF;
    errorno   = di & 0x00FF;
    DOS_Error = errorno;
    /* report which error it was */
    sprintf(msg, "\r\nERROR: %s on drive %c\r\n[R]etry, [I]gnore: ",
            err_msg[ errorno ], 'A' + drive);
    // return to the program via dos interrupt 0x23 with abort, retry,
    // or ignore as input by the user.
	hardresume( error_win( msg));
    return( ABORT);
}

/***************************************************************************/
/*                                                                         */
/*                   General Screen/Menu functions                         */
/*                                                                         */
/***************************************************************************/
void HideCursor( void)
{
    int i;
    union REGS Reg;

    i=0;
    Reg.h.bh = (unsigned char)i;
    i=24;
    Reg.h.dh = (unsigned char)i;
    i=80;
    Reg.h.dl = (unsigned char)i;
    Reg.h.ah = 2;
    int86(0x10, &Reg, &Reg);
}

int GetKey( BOOLEAN Hide)
{
    int c;

    if( Hide) HideCursor();
    if( !(c = getch()))
        c = getch() | 0x100;
    return( c);
}

void PlaySound()
{
    sound( 100);
    delay( 100);
    nosound();
}

int Wait_For_SPACEBAR_or_ESC( void)
{
    int KeyPressed;

	cprintf( "\r\nPress SPACEBAR to continue ...");
    for( KeyPressed = 0 ;
         KeyPressed != C_SPACEBAR &&
         KeyPressed != C_ESC ;       )
    {
         switch( KeyPressed = GetKey( FALSE))
         {
             case C_SPACEBAR:
             case C_ESC     :
                 break;

             default        :
                 PlaySound();
                 break;
         }
    }
    return( KeyPressed);
}

char *Alloc_String( char *String, int Length)
{
    int StringLength, i;

    if( strlen( String) == 0 && Length == 0)
    {
        cprintf( "ERROR: Alloc_String() : Nor STRING or LENGTH entered !");
        Wait_For_SPACEBAR_or_ESC();
        cprintf( "\r\n");
        return( NULL);
    }
    StringLength = strlen( String);
    if( StringLength == 0)
        StringLength = Length;

    char *Destination = new char [ StringLength + 1];

    if( strlen( String) > 0)
        return( strcpy( Destination, String));
	else
		return( Destination);
}


// GENERAL FUNCTIONS/PROCEDURES
///////////////////////////////

/***************************************************************************/
/*                                                                         */
/*                   Get/Extract Version and BuildLevel                    */
/*                                                                         */
/***************************************************************************/
int Get_BuildVersion( char *Version)
{
	char *Extract;
	int i, j;

	Extract = Alloc_String( Version, 0);
	for( i = 0, j = 0 ; i < strlen( Extract) ; i++)
	{
		if( Extract[i] == '.') j += 1;
		if( j == 2)
		{
			Extract[i] = '\0';
			break;
		}
	}
	return( atoi( Extract));
}

int Get_MajorVersion( char *Version)
{
	char *Extract;
	int i, j;

	Extract = Alloc_String( Version, 0);
	for( i = 0, j = 0 ; i < strlen( Extract) ; i++)
	{
		if( Extract[i] == '.') j += 1;
		if( j == 1)
		{
			Extract[i] = '\0';
			break;
		}
	}
	return( atoi( Extract));
}

int Get_MinorVersion( char *Version)
{
	char *Extract;
	int i, j, k;

	Extract = Alloc_String( Version, 0);
	for( i = 0, j = 0, k = 0 ; i < strlen( Extract) ; i++)
	{
		if( Extract[i] == '.')
		{
			j += 1;
			if( j == 1) k = i + 1;
		}
		if( j == 2)
		{
			Extract[i] = '\0';
			break;
		}
	}
	Extract = &Extract[k];
	return( atoi( Extract));
}

int Get_BuildLevel( char *Version)
{
	char *Extract;
	int i, j;

	Extract = Alloc_String( Version, 0);
	for( i = 0, j = 0 ; i < strlen( Extract) ; i++)
	{
		if( Extract[i] == '.') j += 1;
		if( j == 2)
		{
			Extract = &Extract[++i];
			break;
		}
	}
	return( atoi( Extract));
}

char *Get_VERSION( BOOLEAN Formatted)
{
//	int i, j, k;
//	BOOLEAN Updated;
//	char *Version;
//	char *BuildLevel;

	ifstream VERSION( "\\VOTE\\MENU.VER");
	if( !VERSION)
		strcpy( Buffer, "?.?");
	else
	{
		VERSION.getline( Buffer, sizeof( Buffer));
		if( !VERSION)
			strcpy( Buffer, "?.?");
		else
		{
			if( Formatted)
				if( Get_BuildLevel( Buffer) == 0)
					sprintf( Buffer, "v%d.%d",
									 Get_MajorVersion( Buffer),
									 Get_MinorVersion( Buffer));
				else
					sprintf( Buffer, "v%d.%d Build %.3d",
									 Get_MajorVersion( Buffer),
									 Get_MinorVersion( Buffer),
									 Get_BuildLevel( Buffer));
		}
	}
	return( (char *)Buffer);
}

/***************************************************************************/
/*                                                                         */
/*                   Get Information from \CONTENTS-file                   */
/*                                                                         */
/***************************************************************************/
char *Get_CONTENTS( char *Drive)
{
	char ContentsFile[12];

	sprintf( ContentsFile, "%s\\CONTENTS", Drive);
	ifstream CONTENTS( ContentsFile);
	if( !CONTENTS)
		strcpy( Buffer, "????????");
	else
	{
		CONTENTS.getline( Buffer, sizeof( Buffer));
		if( !CONTENTS)
			strcpy( Buffer, "????????");
	}
	return( (char *)Buffer);
}

void InitializeScreen( void)
{
   int i;
   char *DateTime;
   struct date currDate;

   DateTime = Alloc_String( "Today : dd-mm-yyyy hh:mm", 0);
   getdate( &currDate);
   sprintf( DateTime, "Today : %.2d/%.2d/%.4d",
					  currDate.da_day,  currDate.da_mon, currDate.da_year);
   ClearScreen();
   SetMODE( REVERSE);
   gotoxy( 1,  1); clreol();
   gotoxy( 1,  2); clreol();
   gotoxy( 1, 25); clreol();
   SetMODE( NORMAL);
   WriteLeftAssigned(1,"Steria Benelux sa/nv", HIGHREVERSE);
   WriteCentered( 1, "E-VOTING - MENU", REVERSE);
   WriteCentered( 2, "-=-=-=-=-=-=-=-", REVERSE);
   WriteRightAssigned( 1, Get_VERSION( TRUE), HIGHREVERSE);
   WriteRightAssigned( 2, DateTime, HIGHREVERSE);
   DrawBox( 1, 3, 80, 24, DOUBLE);
   WriteOnScreen(  3, 4, "On drive C: ", NORMAL);
   WriteOnScreen( 15, 4, Get_CONTENTS( "C:"), HIGHLIGHT);
   WriteOnScreen(  3, 5, "On drive D: ", NORMAL);
   WriteOnScreen( 15, 5, Get_CONTENTS( "D:"), HIGHLIGHT);
}

void ClearScreen( void)
{
   SetMODE( RESET);
   clrscr();
}

void DrawBox( int TX, int TY, int BX, int BY, STYLE Style)
{
   int x, y;

   gotoxy( TX, TY); cout << ulc[Style];
   gotoxy( TX, BY); cout << llc[Style];
   gotoxy( BX, TY); cout << urc[Style];
   gotoxy( BX, BY); cout << lrc[Style];
   for( x=TX+1 ; x < BX ; x++)
   {
      gotoxy( x, TY); cout << hl[Style];
      gotoxy( x, BY); cout << hl[Style];
   }
   for( y=TY+1 ; y < BY ; y++)
   {
      gotoxy( TX, y); cout << vl[Style];
      gotoxy( BX, y); cout << vl[Style];
   }
   gotoxy( TX+1, TY+1);
}

void SetMODE( MODE mode)
{
   switch( mode)
   {
      case RESET      :
      case NORMAL     :
         textcolor( LIGHTGRAY);
         textbackground( BLACK);
         break;

      case HIGHLIGHT  :
         textcolor( WHITE);
         textbackground( BLACK);
         break;

      case REVERSE    :
         textcolor( BLACK);
         textbackground( WHITE);
         break;

      case HIGHREVERSE:
         textcolor( DARKGRAY);
         textbackground( WHITE);
         break;
   }
}

void WriteOnScreen( int X, int Y, char *Text, MODE mode)
{
   SetMODE( mode);
   gotoxy( X, Y);
   cputs( Text);
   SetMODE( RESET);
}

void WriteCentered( int Y, char *Text, MODE mode)
{
   text_info TI;

   gettextinfo( &TI);
   SetMODE( mode);
   gotoxy( ( ( TI.winright-TI.winleft-strlen( Text)+3)/2), Y);
   cputs( Text);
   SetMODE( RESET);
}

void WriteLeftAssigned( int Y, char *Text, MODE mode)
{
   SetMODE( mode);
   gotoxy( 1, Y);
   cputs( Text);
   SetMODE( RESET);
}

void WriteRightAssigned( int Y, char *Text, MODE mode)
{
   text_info TI;

   gettextinfo( &TI);
   SetMODE( mode);
   gotoxy( TI.winright-TI.winleft-strlen( Text)+2, Y);
   cputs( Text);
   SetMODE( RESET);
}

void ButtomBar( char *Activate)
{
   int i, j;
   char *Info;

   Info = new char[10];

   SetMODE( HIGHREVERSE);
   gotoxy( 1, 25); clreol();

   for( i=0 ; i < strlen( Activate) ; i++)
   {
      if( Activate[i] == '1')
      {
         if( BUTTOM[i].ShortCut[0] != '\0')
         {
            gotoxy( BUTTOM[i].X-2, 25);
            SetMODE( HIGHREVERSE);
            cputs( "³");
            SetMODE( REVERSE);
            sprintf( Info, "[%s] ", BUTTOM[i].ShortCut);
            cputs( Info);
         }
         else
         {
            gotoxy( BUTTOM[i].X-1, 25);
            cputs( "³");
         }

         if( BUTTOM[i].FirstChar == TRUE)
         {
            for( j=0 ; j < strlen( BUTTOM[i].Comment) ; j++)
            {
               if( j == 0)
                  SetMODE( REVERSE);
               else
                  SetMODE( HIGHREVERSE);

               sprintf( Info, "%c", BUTTOM[i].Comment[j]);
               cputs( Info);
            }
         }
         else
         {
            SetMODE( HIGHREVERSE);
            cputs( BUTTOM[i].Comment);
         }
         cputs( "³");
      }
   }

   SetMODE( RESET);
}

BOOLEAN Read_MENULST( void)
{
   BOOLEAN Result = TRUE;
   ChoiceList LinkC = NULL, TempC = NULL;
   char Buffer[1000], PrevChar;
   char *MenuLst, *Field;
   int i, j, k, w;

   // Pre-Fill Variables
   MenuLst  = Alloc_String( NULL, 40);
   Field    = Alloc_String( NULL, 100);

   PrevChar = '\0';

   LinkC    = FirstChoice;

   sprintf(MenuLst, "%s\\%s", RootDir, _CFG_file);
   ifstream MENULST( MenuLst);
   if( !MENULST)
      Result = FALSE;
   while( Result == TRUE &&
          !MENULST.eof())
   {
      MENULST.getline( Buffer, sizeof( Buffer));
      if( !MENULST &&
          !MENULST.eof())
         Result = FALSE;
      else
      {
         if( strlen( Buffer) == 0)
             continue;

         Field[0]   = '\0';
         for( i=0, j=0, w=0 ;
              i < strlen( Buffer) && Result == TRUE ;
              i++)
         {
            switch( Buffer[i])
            {
               case '|' :                    // Field Delimiter
				  if( Buffer[i] == PrevChar) // if '||' detected = EOL
                     i = strlen( Buffer);
                  else
                  {
                     w++;
                     j=0;
                     switch( w)
                     {
                        case 1 :
                           TempC = new MenuChoice;
                           TempC->Text       = Alloc_String( Field, 0);
                           TempC->HotKey     = ' ';
                           for( k = 0 ; k < ( strlen( TempC->Text) - 1); k++)
                               if( TempC->Text[k] == '&')
                               {
                                   TempC->HotKey = TempC->Text[k + 1];
                                   k = strlen( TempC->Text);
                               }
                           break;

                        case 2 :
                           TempC->DOSCommand = Alloc_String( Field, 0);
                           break;

                        case 3 :
                           TempC->Parameters = Alloc_String( Field, 0);
                           break;

                        case 4 :
                           TempC->DescNL     = Alloc_String( Field, 0);
                           break;

                        case 5 :
                           TempC->DescFR     = Alloc_String( Field, 0);
                           break;

                        case 6 :
                           TempC->DescGE     = Alloc_String( Field, 0);
                           break;

                        default:
                           i = strlen( Buffer);
                           break;
                     }
                  }
				  Field[0]='\0';
                  break;

               case '\0':
               case '\n':
                  break;

               default  :
                  Field[j++] = Buffer[i];
                  Field[j]   = '\0';
                  break;
            }
            PrevChar = Buffer[i];
         }
         TempC->Next = NULL;
         TempC->Prev = NULL;
         if( FirstChoice == NULL)
             FirstChoice = TempC;
         else
         {
             TempC->Prev       = LinkC;
             TempC->Next       = FirstChoice;
             LinkC->Next       = TempC;
             FirstChoice->Prev = TempC;
         }
         LinkC = TempC;
         NbrOfItems++;
      }
   }

   return( Result);
}

void Draw_MenuChoice( int X, int Y, char *MenuChoice, BOOLEAN Selected)
{
    int i;
    BOOLEAN HotKey;

    HotKey = FALSE;
    gotoxy( X, Y);
    for( i = 0 ; i < strlen( MenuChoice) ; i++)
        if( MenuChoice[i] == '&')
            HotKey = TRUE;
        else
        {
			if( Selected)
                if( HotKey)
                {
                    SetMODE( HIGHLIGHT);
                    HotKey = FALSE;
                }
                else
                    SetMODE( REVERSE);
            else
                if( HotKey)
                {
                    SetMODE( HIGHLIGHT);
                    HotKey = FALSE;
                }
                else
                    SetMODE( NORMAL);
            putch( MenuChoice[i]);
        }
}

void Show_MENU( int X, int Y)
{
    ChoiceList LinkC = NULL;

    DrawBox( X - 1, Y - 1,
             X + strlen( FirstChoice->Text) - 1, Y + NbrOfItems,
             SINGLE);
    for( LinkC = FirstChoice        ;
         LinkC->Next != FirstChoice ;
         LinkC = LinkC->Next         )
    {
        Draw_MenuChoice( X, Y++, LinkC->Text, FALSE);
    }
    Draw_MenuChoice( X, Y++, LinkC->Text, FALSE);
}

void Show_Description( ChoiceList Item)
{
    window( 2, 21, 79, 23);
    SetMODE( REVERSE);
    clrscr();
    WriteOnScreen( 2, 1, "N:", REVERSE);
    WriteOnScreen( 5, 1, Item->DescNL, HIGHREVERSE);
    WriteOnScreen( 2, 2, "F:", REVERSE);
    WriteOnScreen( 5, 2, Item->DescFR, HIGHREVERSE);
	WriteOnScreen( 2, 3, "D:", REVERSE);
    WriteOnScreen( 5, 3, Item->DescGE, HIGHREVERSE);
    SetMODE( NORMAL);
    window( 1, 1, 80, 25);
}

void Refresh_MENU( int X, int Y)
{
    InitializeScreen();
    Show_MENU( X, Y);
    ButtomBar( StatusBar[MAINmenu]);
}

void Show_FreeMem( void)
{
    char *FreeMem;

    FreeMem = Alloc_String( "Free Memory : 999999Kb", 0);
	sprintf( FreeMem, "Free Memory : %.0fKb", (float)( farcoreleft() / 1024));
    WriteLeftAssigned( 2, FreeMem, HIGHREVERSE);
}

// E-VOTING FUNCTIONS
/////////////////////

void Create_LOG_Window( char *Title, int menu)
{
    ButtomBar( StatusBar[menu]);
    window( 1, 3, 80, 24);
    clrscr();
    DrawBox( 1, 1, 80, 22, DOUBLE);
    window( 2, 4, 79, 23);
    SetMODE( REVERSE);
    gotoxy( 1, 1); clreol();
    WriteCentered( 1, Title, REVERSE);
    window( 2, 5, 79, 23);
}

void Create_DOS_Window( void)
{
    ButtomBar( StatusBar[DOSmenu]);
    window( 1, 3, 80, 24);
    clrscr();
    DrawBox( 1, 1, 80, 22, DOUBLE);
    window( 2, 4, 79, 23);
}

void Destroy_DOS_Window( void)
{
    window( 1, 1, 80, 25);
}

void Reset_Timer( void)
{
    TimerTime = time( NULL);
}

void Elapsed_Time( void)
{
    time_t NewTime;
    double Elapsed, ElapsedMin, ElapsedSec;

    NewTime = time( NULL);

    Elapsed = difftime( NewTime, TimerTime);

    ElapsedSec = (double)( (long)Elapsed % 60);
    ElapsedMin = (double)( ( Elapsed - ElapsedSec) / 60);
    sprintf( LogMessage, "Elapsed Time = %.0f min %.0f sec",
                         ElapsedMin, ElapsedSec);
    Add_To_Logging( EMPTY, LogMessage);
    cprintf( "\r\n%s\r\n", LogMessage);
}

/***************************************************************************/
/*                                                                         */
/*                   General File and Disk Handling                        */
/*                                                                         */
/***************************************************************************/
int Get_RAMdrive( void)
{
    int Drive;
    struct find_t Control_Block;
    char *PathName;

    for( Drive = 'Z' ; Drive >= 'C' ; Drive--)
    {
        sprintf( PathName, "%c:*.*", Drive);
        if( !_dos_findfirst( PathName, _A_VOLID, &Control_Block))
            if( !strcmp( Control_Block.name, "MS-RAMDR.IVE"))
                return( Drive);
    }
    return( 0);
}

#pragma warn +par
BOOLEAN File_Exists( char *Drive, char *Path, char *Name)
{
    char *File_To_Check;
    char FileDrive     [ _MAX_DRIVE ];
    char FilePath      [ _MAX_DIR   ];
    char FileName      [ _MAX_FNAME ];
    char FileExtention [ _MAX_EXT   ];

    File_To_Check = Alloc_String( NULL, _MAX_PATH);

	_splitpath( Name, FileDrive, FilePath, FileName, FileExtention);
    if( FileDrive == NULL || strlen( FileDrive) == 0)
        strcpy( FileDrive, Drive);
    if( FilePath  == NULL || strlen( FilePath)  == 0)
        strcpy( FilePath,  Path );

    _makepath( File_To_Check, FileDrive, FilePath, FileName, FileExtention);

    return( !access( File_To_Check, EXISTS)?TRUE
                                           :FALSE);
}

#pragma warn +par
BOOLEAN File_ReadOnly( char *Filename)
{
    return( !access( Filename, READ_ONLY)?TRUE
                                         :FALSE);
}

#pragma warn +par
BOOLEAN Make_ReadWrite( char *Filename)
{
    return( File_ReadOnly( Filename)
            ?( !chmod( Filename, S_IWRITE|S_IREAD)?TRUE
                                                  :FALSE)
            :TRUE);
}

#pragma warn +par
BOOLEAN Make_Hidden( char *Filename)
{
    BOOLEAN Result = TRUE;
    struct find_t ffblk;
    unsigned attrib;

    if( !_dos_findfirst( Filename, _A_NORMAL|_A_RDONLY|_A_HIDDEN, &ffblk))
    {
        attrib = ffblk.attrib;
        if( attrib | _A_HIDDEN)
        {
            attrib |= _A_HIDDEN;
            Result = !_dos_setfileattr( Filename, attrib)?TRUE
                                                         :FALSE;
        }
    }
	else
        Result = FALSE;

    return( Result);
}

#pragma warn +par
long Copy_File( char *Source, char *Destination, BOOLEAN Keep_DateTime)
{
    BOOLEAN Result = TRUE;
    long FileSize, SizeCopied;
    unsigned OriginalDate, OriginalTime, OriginalAttrib;
    struct find_t ffblk;
    FILE *stream;

    if( !File_Exists( NULL, NULL, Source))
        return( -1);

    if( !_dos_findfirst( Source, _A_NORMAL|_A_HIDDEN|_A_RDONLY, &ffblk))
    {
        FileSize       = ffblk.size;
        OriginalDate   = ffblk.wr_date;
        OriginalTime   = ffblk.wr_time;
        OriginalAttrib = (unsigned) ffblk.attrib;
    }
    else
        return( -1);

    ifstream FROMfile( Source,      ios::binary);
    ofstream TOfile  ( Destination, ios::binary);
    TOfile << FROMfile.rdbuf();
    TOfile.seekp( 0, ios::end);
    SizeCopied = TOfile.tellp();
    if( FileSize != SizeCopied)
        Result = FALSE;

    if( Result)
        Result = !_dos_setfileattr( Destination, OriginalAttrib)?TRUE
                                                                :FALSE;

    if( Result && Keep_DateTime)
    {
        if( ( stream = fopen( Destination, "rb")) == NULL)
            Result = FALSE;
        else
		{
            if( _dos_setftime( fileno( stream), OriginalDate,
                                                OriginalTime) != 0)
                Result = FALSE;

            fclose( stream);
        }
    }

    if( Result)
        return( SizeCopied);
    else
        return( -1);
}

#pragma warn +par
BOOLEAN Remove_File( char *Drive, char *Path, char *Name)
{
    BOOLEAN Result = TRUE;
    char *File_To_Delete;
    char FileDrive     [ _MAX_DRIVE ];
    char FilePath      [ _MAX_DIR   ];
    char FileName      [ _MAX_FNAME ];
    char FileExtention [ _MAX_EXT   ];

    File_To_Delete = Alloc_String( NULL, _MAX_PATH);

    _splitpath( Name, FileDrive, FilePath, FileName, FileExtention);
    if( FileDrive == NULL || strlen( FileDrive) == 0)
        strcpy( FileDrive, Drive);
    if( FilePath  == NULL || strlen( FilePath)  == 0)
        strcpy( FilePath,  Path );
    _makepath( File_To_Delete, FileDrive, FilePath, FileName, FileExtention);

    if( !File_Exists( NULL, NULL, File_To_Delete))
        Result = TRUE;
    else
    {
        if( File_ReadOnly( File_To_Delete))
            Result = Make_ReadWrite( File_To_Delete);

        if( Result)
            Result = !remove( File_To_Delete)?TRUE
                                             :FALSE;
    }
	return( Result);
}

BOOLEAN Copy_Move_File( char *FromDrive, char *FromPath, char *FromFile,
                        char *ToDrive,   char *ToPath,   char *ToFile,
                        COPY_MOVE     Move_File,
                        DATE_TIME     Keep_DateTime,
                        MUST_EXIST    Must_Exist,
                        ATTRIB_HIDDEN Hidden)
{
    char *Source, *Destination;
    char FileName      [ _MAX_FNAME ];
    char FileExtention [ _MAX_EXT   ];
    BOOLEAN Result = TRUE;

    Source      = Alloc_String( NULL, _MAX_PATH);
    Destination = Alloc_String( NULL, _MAX_PATH);

    // Composite Source
    _splitpath( FromFile, NULL, NULL, FileName, FileExtention);
    _makepath( Source, FromDrive, FromPath, FileName, FileExtention);
    // Composite Destination
    _splitpath( ToFile, NULL, NULL, FileName, FileExtention);
    _makepath( Destination, ToDrive, ToPath, FileName, FileExtention);

    if( File_Exists( FromDrive, FromPath, FromFile))
    {
        if( Copy_File( Source, Destination,
                       (BOOLEAN)Keep_DateTime) == -1)
            Result = FALSE;

        if( (BOOLEAN)Move_File)
            Result = Remove_File( FromDrive, FromPath, FromFile);

        if( (BOOLEAN)Hidden)
            Result = Make_Hidden( Destination);
    }
    else
        Result = (BOOLEAN)!Must_Exist;

    if( !Result)
    {
        sprintf( LogMessage, "Error Copying %s to %s !",
                             Source, Destination);
        Add_To_Logging( ERR, LogMessage);
	}

    return( Result);
}

#pragma warn +par
BOOLEAN Format_Disk( char *Drive, char *Label, BOOLEAN Quick,
                                               BOOLEAN System)
{
    char *parLabel, *Format_inp;
    BOOLEAN Result;
    int RetCode;

    // Pre-Fill Variable
    parLabel   = Alloc_String( NULL, 20);
    Format_inp = Alloc_String( NULL, 40);

    sprintf( parLabel, "/V:%s", Label);
    sprintf( Format_inp, "%s\\FORMAT.INP", TempDir);

    /* :WARNING: avn 20020212 -> format fails without notice
       in case the TempDir does not exist... */

    /* prepare a textfile containing input for the format command */
    ofstream FORMAT_INP( Format_inp);
    FORMAT_INP << "Y\r\n"
               << "N\r\n";
    FORMAT_INP.close();

    Result = TRUE;
    /* take input from file in stead of stdin (console), otherwise the
       format command will prompt the user */
    if( freopen( Format_inp, "rt", stdin)  == NULL)
        Result = FALSE;
    /* disable stdout and stderr */
    if( freopen("NUL",       "wt", stdout) == NULL)
        Result = FALSE;
    if( freopen("NUL",       "wt", stderr) == NULL)
        Result = FALSE;

    if( Result)
    {
        if( Quick)
            RetCode = spawnlp( P_WAIT, "FORMAT.COM", "FORMAT.COM",
                                       Drive, parLabel, "/Q", "/U", NULL);
		else
            RetCode = spawnlp( P_WAIT, "FORMAT.COM", "FORMAT.COM",
                                       Drive, parLabel, "/U", NULL);
        if( RetCode == 0 && System)
            RetCode = spawnlp( P_WAIT, "SYS.COM", "SYS.COM",
                                       "C:\\", Drive, NULL);
        if( RetCode != 0)
        {
            sprintf( LogMessage, "FORMAT/SYS:RetCode=[%d] errno=[%d]",
                                 RetCode, errno);
            Add_To_Logging( EMPTY, LogMessage);
            switch( errno)
            {
                case E2BIG  :
                    sprintf( LogMessage, "FORMAT/SYS:Arg list too long");
                    break;

                case EINVAL :
                    sprintf( LogMessage, "FORMAT/SYS:Invalid argument");
                    break;

                case ENOENT :
                    sprintf( LogMessage, "FORMAT/SYS:Path or file name not found");
                    break;

                case ENOEXEC:
                    sprintf( LogMessage, "FORMAT/SYS:Exec format error");
                    break;

                case ENOMEM :
                    sprintf( LogMessage, "FORMAT/SYS:Not enough core");
                    break;

                default     :
                    sprintf( LogMessage, "FORMAT/SYS:Unknown error [%d]", errno);
                    break;
            }
            Result = FALSE;
            Add_To_Logging( ERR, LogMessage);
        }
    }

    if( freopen("CON", "rt", stdin)  == NULL)
        Result = FALSE;
    if( freopen("CON", "wt", stdout) == NULL)
		Result = FALSE;
    if( freopen("CON", "wt", stderr) == NULL)
        Result = FALSE;

    Remove_File( NULL, NULL, Format_inp);

    if( !Result)
    {
        sprintf( LogMessage, "Error Formatting %s !", Drive);
        Add_To_Logging( ERR, LogMessage);
    }

    return( Result);
}

BOOLEAN Optimize_Database( char *From)
{
    BOOLEAN Result = TRUE;
    int RetCode;

    cprintf( "Optimizing database ... Please wait ... ");

    if( freopen( "NUL", "rt", stdin)  == NULL)
        Result = FALSE;
    if( freopen( "NUL", "wt", stdout) == NULL)
        Result = FALSE;
    if( freopen( "NUL", "wt", stderr) == NULL)
        Result = FALSE;

    sprintf( DOScmd, "%s%s\\ELECT", From, RootDir);
    if( Result)
    {
        RetCode = spawnlp( P_WAIT, "_PROUTIL", "_PROUTIL",
                                   DOScmd, "-C", "TRUNCATE", "BI", NULL);
        if( RetCode != 0)
        {
            sprintf( LogMessage, "_PROUTIL:RetCode=[%d] errno=[%d]",
                                 RetCode, errno);
            Add_To_Logging( EMPTY, LogMessage);
            switch( errno)
            {
                case E2BIG  :
                    sprintf( LogMessage, "_PROUTIL:Arg list too long");
                    break;

				case EINVAL :
                    sprintf( LogMessage, "_PROUTIL:Invalid argument");
                    break;

                case ENOENT :
                    sprintf( LogMessage, "_PROUTIL:Path or file name not found");
                    break;

                case ENOEXEC:
                    sprintf( LogMessage, "_PROUTIL:Exec format error");
                    break;

                case ENOMEM :
                    sprintf( LogMessage, "_PROUTIL:Not enough core");
                    break;

                default     :
                    sprintf( LogMessage, "_PROUTIL:Unknown error [%d]", errno);
                    break;
            }
            Result = FALSE;
            Add_To_Logging( ERR, LogMessage);
        }
    }

    if( Result)
    {
        RetCode = spawnlp( P_WAIT, "PROLOG", "PROLOG", DOScmd, NULL);
        switch( RetCode)
        {
            case 0 : // Everything OK
                break;

            case 1 : // LOG-file is small enough
                sprintf( LogMessage, "PROLOG:File is too small to truncate !", DOScmd);
                Add_To_Logging( EMPTY, LogMessage);
                break;

            default:
                sprintf( LogMessage, "PROLOG:RetCode=[%d] errno=[%d]",
                                     RetCode, errno);
                Add_To_Logging( EMPTY, LogMessage);
                switch( errno)
                {
                    case E2BIG  :
						sprintf( LogMessage, "PROLOG:Arg list too long");
                        break;

                    case EINVAL :
                        sprintf( LogMessage, "PROLOG:Invalid argument");
                        break;

                    case ENOENT :
                        sprintf( LogMessage, "PROLOG:Path or file name not found");
                        break;

                    case ENOEXEC:
                        sprintf( LogMessage, "PROLOG:Exec format error");
                        break;

                    case ENOMEM :
                        sprintf( LogMessage, "PROLOG:Not enough core");
                        break;

                    default     :
                        sprintf( LogMessage, "PROLOG:Unknown error [%d]", errno);
                        break;
                }
                Result = FALSE;
                Add_To_Logging( ERR, LogMessage);
                break;
        }
    }

    if( freopen("CON", "rt", stdin)  == NULL)
        Result = FALSE;
    if( freopen("CON", "wt", stdout) == NULL)
        Result = FALSE;
    if( freopen("CON", "wt", stderr) == NULL)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    cprintf( "OK\r\n");
    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                  Create Shell Script to be run in DOS                   */
/*                                                                         */
/***************************************************************************/
BOOLEAN Create_ShellScript( char *Tag,
                            char *FromDrive,
                            char *ToDrive,
                            char *ShellName)
{
    BOOLEAN Result = TRUE, tagFound = FALSE;
    char tagBuffer[80];
    long i;

    cprintf( "Generating Shell Script ... Please wait ... ");

    ofstream SHELL;
    ifstream INIfile("\\VOTE\\MENU.INI");
    if( !INIfile)
    {
		Result = FALSE;
        cprintf( "Not OK\r\nError creating %s !\r\n", ShellBat);
        Wait_For_SPACEBAR_or_ESC();
    }
    else
    {
        while( Result && !INIfile.eof())
        {
            INIfile.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
				if( !tagFound)
				{
				    if( !strcmp( Buffer, Tag))
				    {
 				        tagFound = TRUE;
 				        if( ShellName == NULL)
                            SHELL.open( ShellBat);
                        else
                            SHELL.open( ShellName);
                        if( !SHELL)
                        {
                            Result = FALSE;
                            cprintf( "Not OK\r\nError creating %s !\r\n", ShellBat);
                            Wait_For_SPACEBAR_or_ESC();
                        }
					}
			    }
			    else
			    {
					if( !strcmp( Buffer, "<EOF>"))
					{
						tagFound = FALSE;
						SHELL.close();
				    }
				    else
				    {
						for( i = 0 ; i < strlen( Buffer) ; i++)
						{
							if( Buffer[i    ] == '#' &&
							    Buffer[i + 1] == '[' &&
							    Buffer[i + 4] == ']'   )
							{
								strcpy( tagBuffer, &Buffer[i + 2]);
								tagBuffer[2] = '\0';
								switch( atoi( tagBuffer))
								{
									case 0 : SHELL << RAMdrive;  break;
									case 1 : SHELL << FromDrive; break;
									case 2 : SHELL << ToDrive;   break;
									case 3 : SHELL << RootDir;   break;
									case 10: SHELL << LogFile;   break;
									case 11: SHELL << UpdateLog; break;
									case 20: SHELL << LibBat;    break;
									default:
									    Result = FALSE;
									    Buffer[i + 5] = '\0';
                                        cprintf( "Not OK\r\nUnknown TAG %s !\r\n",
                                                 &Buffer[i]);
                                        Wait_For_SPACEBAR_or_ESC();
								}
								i += 4;
							}
							else
							    SHELL << Buffer[i];
						}
					    SHELL << "\n";
					}
			    }
            }
        }
    }
    if( !Result)
        cprintf( "Not ");
    cprintf( "OK");

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                DIGBACK                                  */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN DigBack( char *Parameters)
{
    char *FromContents, *FromDrive, *ToDrive,
         *ArchiveFile, *ArchiveList;
    BOOLEAN Result = TRUE;
    int KeyPressed, RetCode;
    unsigned attrib;
    struct find_t ffblk;

    // Pre-Fill Variables
    FromDrive    = Alloc_String( "?:",  0);
    ToDrive      = Alloc_String( "?:",  0);
    ArchiveFile  = Alloc_String( NULL, 80);
    ArchiveList  = Alloc_String( NULL, 80);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 15)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
    {
        strncpy( FromDrive, &Parameters[ 6], 2);
        strncpy( ToDrive,   &Parameters[13], 2);
    }
    cprintf( "OK\r\n");

    if( Result)
    {
        cprintf( "DigBack from %s to %s\r\n", FromDrive, ToDrive);
        DOS_Error = -1;
        FromContents = Alloc_String( Get_CONTENTS( FromDrive), 0);
        if( DOS_Error != -1)
        {
            Result = FALSE;
            cprintf( "Errors or No SOURCE present in %s !\r\n(%s)\r\n",
                     FromDrive, err_msg[DOS_Error]);
        }
        else
        {
            SetMODE( NORMAL);    cprintf( "\r\nTake a Backup from ");
            SetMODE( HIGHLIGHT); cprintf( "%s\r\n", FromContents);
            SetMODE( NORMAL);
        }
    }

    if( Result)
    {
        SetMODE( HIGHLIGHT);
        cprintf( "\r\nPut an EMPTY diskette in %s ...", ToDrive);
        cprintf( "\r\nALL DATA ON DISKETTE WILL BE DESTROYED !!!\r\n");
        SetMODE( NORMAL);
        cprintf( "\r\nPress [S] to start DigBack ...");
        for( KeyPressed = 0 ;
             KeyPressed != 'S' &&
             KeyPressed != 's' &&
             KeyPressed != C_ESC ; )
        {
            switch( KeyPressed = GetKey( FALSE))
            {
                case 'S'  :
                case 's'  :
                case C_ESC:
                    break;

                default   :
                    PlaySound();
                    break;
            }
        }

        if( KeyPressed == C_ESC)
            Cancelled = TRUE;
        else
        {
            sprintf( LogMessage, "DigBack started ... from %s to %s",
                                 FromContents, ToDrive);
            Add_To_Logging( EMPTY, LogMessage);
            Reset_Timer();
        }

        gotoxy( 1, wherey());     clreol();
        gotoxy( 1, wherey() - 2); clreol();
        gotoxy( 1, wherey() - 1); clreol();
    }

    if( Result && !Cancelled)
    {
        cprintf( "Formatting %s ... Please wait ... ", ToDrive);
        if( !Format_Disk( ToDrive, "DIGBACKÿDB", TRUE, FALSE))
            if( !Format_Disk( ToDrive, "DIGBACKÿDB", FALSE, FALSE))
            {
                cprintf( "Not ");
                Result = FALSE;
            }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Copying busy ... Please wait ... ");
            Result = Copy_Move_File( FromDrive, "\\", "CONTENTS",
                                     ToDrive,   "\\", "CONTENTS",
                     COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, HIDDEN);
        if( Result)
            Result = Copy_Move_File( FromDrive, RootDir, "MENU.VER",
                                     ToDrive,   "\\", "VERSION.DBK",
                     COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, HIDDEN);
        if( Result)
            Result = Copy_Move_File( FromDrive, "\\", "PANACHE",
                                     ToDrive,   "\\", "PANACHE",
                     COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, HIDDEN);
        if( Result)
            Result = Copy_Move_File( FromDrive, "\\", "KANTONS",
                                     ToDrive,   "\\", "KANTONS",
                     COPY_FILE, KEEP_DATETIME, IF_FILE_EXISTS, VISIBLE);
        if( !Result)
            cprintf( "Not ");
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
        Result = Optimize_Database( FromDrive);

    if( Result && !Cancelled)
    {
        cprintf( "Backing up database ... Please wait ... ");

        if( freopen( "NUL", "rt", stdin)  == NULL)
            Result = FALSE;
        if( freopen( "NUL", "wt", stdout) == NULL)
            Result = FALSE;
        if( freopen( "NUL", "wt", stderr) == NULL)
            Result = FALSE;

        sprintf( ArchiveFile, "%s%s\\ELECT.EXE", FromDrive, TempDir);
        sprintf( ArchiveList, "%s%s\\ELECT.??",  FromDrive, RootDir);

        RetCode = spawnlp( P_WAIT, "ARJ", "ARJ",
                                   "A", "-A", "-E", "-JE", "-Y", "-GDigiMenu",
                                   ArchiveFile, ArchiveList, NULL);

        if( freopen("CON", "rt", stdin)  == NULL)
            Result = FALSE;
        if( freopen("CON", "wt", stdout) == NULL)
            Result = FALSE;
        if( freopen("CON", "wt", stderr) == NULL)
            Result = FALSE;

        if( RetCode != 0)
        {
            sprintf( LogMessage, "ARJ:RetCode=[%d] errno=[%d]",
                                 RetCode, errno);
            Add_To_Logging( EMPTY, LogMessage);
            switch( errno)
            {
                case E2BIG  :
                    sprintf( LogMessage, "ARJ:Arg list too long");
                    break;

                case EINVAL :
                    sprintf( LogMessage, "ARJ:Invalid argument");
                    break;

                case ENOENT :
                    sprintf( LogMessage, "ARJ:Path or file name not found");
                    break;

                case ENOEXEC:
                    sprintf( LogMessage, "ARJ:Exec format error");
                    break;

                case ENOMEM :
                    sprintf( LogMessage, "ARJ:Not enough core");
                    break;

                default     :
                    sprintf( LogMessage, "ARJ:Unknown error [%d]", errno);
                    break;
            }
            Add_To_Logging( ERR, LogMessage);
            Add_To_Logging( ERR, "Error Backing up database !");
            cprintf( "Not ");
            Result = FALSE;
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Backing up to %s ... Please wait ... ", ToDrive);
        Result = Copy_Move_File( FromDrive, TempDir, "ELECT.EXE",
                                 ToDrive,   "\\",    "ELECT.DBK",
                 MOVE_FILE, KEEP_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( !Result)
            cprintf( "Not ");
        cprintf( "OK\r\n");
    }

    if( !Cancelled)
        Add_To_Logging( Result?OK:NOK, "DigBack ended !");

    if( Result && !Cancelled)
    {
        Elapsed_Time();
        Wait_For_SPACEBAR_or_ESC();
    }
    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                DIGREST                                  */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN DigRest( char *Parameters)
{
    char *FromDrive, *ToDrive;
    char *ToFile;
    char *QueryDisk, *ToContents, *FromContents;
    char *DigRestVersion, *currVersion, *restVersion, *BuildInf;
    BOOLEAN Result        = TRUE,
            SaveResult    = FALSE,
            BackupDisk    = TRUE,
            BackupStarted = FALSE,
            ForceRelease  = FALSE;
    int KeyPressed;
    unsigned attrib;
    struct find_t ffblk;

    // Pre-Fill Variables
    FromDrive      = Alloc_String( "?:", 0);
    ToDrive        = Alloc_String( "?:", 0);
    ToFile         = Alloc_String( NULL, 80);
    QueryDisk      = Alloc_String( "?:*.*", 0);
    DigRestVersion = Alloc_String( "?:\\VERSION.DBK", 0);
    currVersion    = Alloc_String( NULL, 20);
    restVersion    = Alloc_String( NULL, 20);
    BuildInf       = Alloc_String( "?:\\BUILD.INF", 0);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 15)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
    {
        strncpy( FromDrive, &Parameters[ 6], 2);
        strncpy( ToDrive,   &Parameters[13], 2);
        DigRestVersion[0] = FromDrive[0];
        BuildInf[0]       = RAMdrive[0];
    }
    cprintf( "OK\r\n");

    if( Result)
    {
        cprintf( "DigRest from %s to %s\r\n", FromDrive, ToDrive);
        SetMODE( HIGHLIGHT);
        cprintf( "\r\nPut a BACKUP diskette in %s ...\r\n", FromDrive);
        SetMODE( NORMAL);
        cprintf( "\r\nPress [S] to start DigRest ...");
        for( KeyPressed = 0 ;
             KeyPressed != 'S' &&
             KeyPressed != 's' &&
             KeyPressed != C_ESC ; )
        {
            switch( KeyPressed = GetKey( FALSE))
            {
                case 'S'  :
                case 's'  :
                case C_ESC:
                    break;

                default   :
                    PlaySound();
                    break;
            }
        }

        if( KeyPressed == C_ESC)
            Cancelled = TRUE;

        gotoxy( 1, wherey());     clreol();
        gotoxy( 1, wherey() - 2); clreol();
    }

    if( Result && !Cancelled)
    {
        BackupDisk = TRUE;
        cprintf( "Checking diskette in %s ... Please wait ... ", FromDrive);
        sprintf( QueryDisk, "%s*.*", FromDrive);
        if( !_dos_findfirst( QueryDisk, _A_VOLID, &ffblk))
        {
            if( strcmp( ffblk.name, "DIGBACKÿ.DB"))
                BackupDisk = FALSE;
            if( BackupDisk)
                BackupDisk = File_Exists( FromDrive, "\\", "CONTENTS" );
            if( BackupDisk)
                BackupDisk = File_Exists( FromDrive, "\\", "VERSION.DBK" );
            if( BackupDisk)
                BackupDisk = File_Exists( FromDrive, "\\", "PANACHE" );
            if( BackupDisk)
                BackupDisk = File_Exists( FromDrive, "\\", "ELECT.DBK");
        }
        else
            BackupDisk = FALSE;

        if( !BackupDisk)
        {
            cprintf( "Not OK\r\n");
            SetMODE( HIGHLIGHT);
            sprintf( LogMessage, "This is NOT a BACKUP diskette !");
            Add_To_Logging( ERR, LogMessage);
            cprintf( "\r\n%s\r\n\r\n", LogMessage);
            SetMODE( NORMAL);
        }
        else
        {
            cprintf( "OK\r\n");
        }

        Result = BackupDisk;
    }

    // Check if version of DigRest corresponds to version of current database
    if( Result && !Cancelled)
    {
        cprintf( "Checking version ... Please wait ... ");
        // Fetching current database version...
        sprintf( currVersion, "%s", Get_VERSION( FALSE));
        // Fetching DigBack version...
        ifstream VERSION( DigRestVersion);
        if( !VERSION)
            strcpy( Buffer, "?.?");
        else
        {
            VERSION.getline( Buffer, sizeof( Buffer));
            if( !VERSION) strcpy( restVersion, "?.?");
        }
        VERSION.close();
        sprintf( restVersion, "%s", Buffer);
        if( !strcmp( currVersion, "?.?") || !strcmp( restVersion, "?.?"))
        {
            cprintf( "Not OK\r\n");
            Add_To_Logging( ERR, "DigRest not possible : unknown version detected !");
            sprintf( LogMessage, "DigRest Version : %s", restVersion);
            Add_To_Logging( ERR, LogMessage);
            sprintf( LogMessage, "Current Version : %s", currVersion);
            Add_To_Logging( ERR, LogMessage);
            Result = FALSE;
        }
        else
        {
            if( Get_MajorVersion( restVersion) != Get_MajorVersion( currVersion) ||
                Get_MinorVersion( restVersion) != Get_MinorVersion( currVersion) ||
                Get_BuildLevel(   restVersion) >  Get_BuildLevel(   currVersion)   )
            {
                cprintf( "Not OK\r\n");
                Add_To_Logging( ERR, "DigRest not possible : incompatible versions detected !");
                sprintf( LogMessage, "DigRest Version : %s", restVersion);
                Add_To_Logging( ERR, LogMessage);
                sprintf( LogMessage, "Current Version : %s", currVersion);
                Add_To_Logging( ERR, LogMessage);
                Result = FALSE;
            }
            else
            {
                cprintf( "OK\r\n");
                gotoxy( 1, wherey());
                clreol();
                if( Get_BuildLevel( restVersion) < Get_BuildLevel( currVersion))
                    ForceRelease = TRUE;
            }
        }
    }

    if( Result && !Cancelled)
    {
        SetMODE( NORMAL);    cprintf( "Restoring from ");
        FromContents = Alloc_String( Get_CONTENTS( FromDrive), 0);
        SetMODE( HIGHLIGHT); cprintf( "%s", FromContents);
        ToContents   = Alloc_String( Get_CONTENTS( ToDrive),   0);
        if( strlen( ToContents) > 0)
        {
            SetMODE( NORMAL);    cprintf( "\r\n          to   ");
            SetMODE( HIGHLIGHT); cprintf( "%s", ToContents);
            cprintf( "\r\n\r\n               !!! OVERWRITING !!!");
        }
        SetMODE( NORMAL);
        cprintf( "\r\n\r\nPress [S] to start DigRest ...");
        for( KeyPressed = 0 ;
             KeyPressed != 'S' &&
             KeyPressed != 's' &&
             KeyPressed != C_ESC ; )
        {
            switch( KeyPressed = GetKey( FALSE))
            {
                case 'S'  :
                case 's'  :
                case C_ESC:
                    break;

                default   :
                    PlaySound();
                    break;
            }
        }

        if( KeyPressed == C_ESC)
            Cancelled = TRUE;
        else
        {
            sprintf( LogMessage, "DigRest started ... from %s to %s",
                                 FromContents, ToContents);
            Add_To_Logging( EMPTY, LogMessage);
            Reset_Timer();
            BackupStarted = TRUE;
        }

        gotoxy( 1, wherey()); clreol();
        if( strlen( ToContents) > 0)
        {
            gotoxy( 1, wherey() - 2);
            clreol();
        }
    }

    if( Result && !Cancelled)
    {
        cprintf( "Saving OLD database ... Please wait ... ");
        // Making SAVE-directory (to be sure it exists !)
        sprintf( ToFile, "%s%s", ToDrive, SaveDir);
        mkdir( ToFile);
        if( Result)
            Result = Copy_Move_File( ToDrive, RootDir, "ELECT.DB",
                                     ToDrive, SaveDir, "ELECT.DB",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
            Result = Copy_Move_File( ToDrive, RootDir, "ELECT.BI",
                                     ToDrive, SaveDir, "ELECT.BI",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
            Result = Copy_Move_File( ToDrive, RootDir, "ELECT.LG",
                                     ToDrive, SaveDir, "ELECT.LG",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
            Result = Copy_Move_File( ToDrive, RootDir, "ELECT.CF",
                                     ToDrive, SaveDir, "ELECT.CF",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
            Result = Copy_Move_File( ToDrive, RootDir, "ELECT.SG",
                                     ToDrive, SaveDir, "ELECT.SG",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
            Result = Copy_Move_File( ToDrive, "\\",    "CONTENTS",
                                     ToDrive, SaveDir, "CONTENTS",
                     MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, HIDDEN);
        if( Result)
            Result = Copy_Move_File( ToDrive, "\\",    "PANACHE",
                                     ToDrive, SaveDir, "PANACHE",
                     MOVE_FILE, NEW_DATETIME, IF_FILE_EXISTS, HIDDEN);
        if( Result)
            Result = Copy_Move_File( ToDrive, "\\",    "KANTONS",
                                     ToDrive, SaveDir, "KANTONS",
                     MOVE_FILE, NEW_DATETIME, IF_FILE_EXISTS, VISIBLE);
        if( !Result)
            cprintf( "Not ");
        cprintf( "OK\r\n");
        SaveResult = Result;
    }

    if( Result && !Cancelled)
    {
        cprintf( "Restore busy ... Please wait ... ");
        // Copying Database Archive File
        Result = Copy_Move_File( FromDrive, "\\",    "ELECT.DBK",
                                 ToDrive,   RootDir, "ELECT.EXE",
                 COPY_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
        if( Result)
        {
            ofstream SHELL( ShellBat);
            if( !SHELL)
            {
                Result = FALSE;
                cprintf( "Not OK\r\nError creating %s !\r\n", ShellBat);
                Wait_For_SPACEBAR_or_ESC();
            }
            else
            {
                SHELL << "@Echo OFF\n"
                      << ToDrive << "\n"
                      << "Cd " << RootDir << "\n"
                      << ToDrive << RootDir << "\\ELECT.EXE -Y -I -GDigiMenu > NUL\n"
                      << "C:\n"
                      << "Cd " << RootDir << "\n";
            }
            SHELL.close();
            if( Result)
            {
                sprintf( DOScmd, "%s > NUL", ShellBat);
                if( system( DOScmd) != 0)
                {
                    Add_To_Logging( ERR, "Error restoring database !");
                    Result = FALSE;
                }
            }
            Remove_File( NULL, NULL, ShellBat);
            if( Result)
            {
                    Result = File_Exists( ToDrive, RootDir, "ELECT.DB");
                if( Result)
                    Result = File_Exists( ToDrive, RootDir, "ELECT.BI");
                if( Result)
                    Result = File_Exists( ToDrive, RootDir, "ELECT.LG");
                if( Result)
                    Result = File_Exists( ToDrive, RootDir, "ELECT.CF");
                if( Result)
                    Result = File_Exists( ToDrive, RootDir, "ELECT.SG");
            }
        }
        Remove_File( ToDrive, RootDir, "ELECT.EXE");
        if( Result)
            Result = Copy_Move_File( FromDrive, "\\", "CONTENTS",
                                     ToDrive,   "\\", "CONTENTS",
                     COPY_FILE, NEW_DATETIME, FILE_MUST_EXIST, HIDDEN);
        if( Result)
            Result = Copy_Move_File( FromDrive, "\\", "PANACHE",
                                     ToDrive,   "\\", "PANACHE",
                     COPY_FILE, NEW_DATETIME, IF_FILE_EXISTS, HIDDEN);
        if( Result)
            Result = Copy_Move_File( FromDrive, "\\", "KANTONS",
                                     ToDrive,   "\\", "KANTONS",
                     COPY_FILE, NEW_DATETIME, IF_FILE_EXISTS, VISIBLE);
        if( !Result)
            cprintf( "Not ");
        cprintf( "OK\r\n");
    }

    if( !Cancelled)
        Add_To_Logging( Result?OK:NOK, "DigRest ended !");
    // If DigRest is NOT successful, restore SAVE if it was successfull
    if( !Result && BackupDisk && BackupStarted)
    {
        if( SaveResult)
        {
            Result = TRUE;
            cprintf( "Going back to previous situation ... Please wait ... ");
            // Deleting 'corrupted' restored files ...
            Remove_File( ToDrive, RootDir, "ELECT.DB");
            Remove_File( ToDrive, RootDir, "ELECT.BI");
            Remove_File( ToDrive, RootDir, "ELECT.LG");
            Remove_File( ToDrive, RootDir, "ELECT.CF");
            Remove_File( ToDrive, RootDir, "ELECT.SG");
            Remove_File( ToDrive, "\\",    "CONTENTS");
            Remove_File( ToDrive, "\\",    "PANACHE" );
            Remove_File( ToDrive, "\\",    "KANTONS" );

            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "ELECT.DB",
                                         ToDrive, RootDir, "ELECT.DB",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "ELECT.BI",
                                         ToDrive, RootDir, "ELECT.BI",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "ELECT.LG",
                                         ToDrive, RootDir, "ELECT.LG",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "ELECT.CF",
                                         ToDrive, RootDir, "ELECT.CF",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "ELECT.SG",
                                         ToDrive, RootDir, "ELECT.SG",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, VISIBLE);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "CONTENTS",
                                         ToDrive, "\\",    "CONTENTS",
                         MOVE_FILE, NEW_DATETIME, FILE_MUST_EXIST, HIDDEN);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "PANACHE",
                                         ToDrive, "\\",    "PANACHE",
                         MOVE_FILE, NEW_DATETIME, IF_FILE_EXISTS, HIDDEN);
            if( Result)
                Result = Copy_Move_File( ToDrive, SaveDir, "KANTONS",
                                         ToDrive, "\\",    "KANTONS",
                         MOVE_FILE, NEW_DATETIME, IF_FILE_EXISTS, VISIBLE);
            Add_To_Logging( Result?OK:NOK, "Previous situation restored !");
            if( !Result)
                cprintf( "Not ");
            cprintf( "OK\r\n");
        }
        else
        {
            Add_To_Logging( ERR, "No previous situation to restore !");
            cprintf( "Restore of previously saved database NOT possible !\r\n");
        }
    }
    else
    {
        if( ForceRelease)
        {
            ofstream BUILD( BuildInf);
            if( !BUILD)
            {
                cprintf( "Error creating %s !", BuildInf);
                Wait_For_SPACEBAR_or_ESC();
            }
            else
            {
                BUILD << Get_MajorVersion( currVersion) << " "
                      << Get_MinorVersion( currVersion) << " "
                      << Get_BuildLevel(   currVersion);
            }
            BUILD.close();

            Result = Create_ShellScript( "[ForceRelease]", FromDrive, ToDrive, NULL);
            if( Result)
            {
                Elapsed_Time();
                sleep( 2);
            }
        }
    }

    if( Result && !Cancelled)
    {
        Elapsed_Time();
        Wait_For_SPACEBAR_or_ESC();
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                DIGCOPY                                  */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN DigCopy( char *Parameters)
{
    char *FromContents, *ToContents;
    char *DirLst, *FileLst;
    char *FromDrive, *ToDrive;
    char *FromFile, *ToFile;
    char *_TempDir, *_SaveDir, *_SpoolDir, *_TraceLogDir, *_BatDir, *_ResultsDir;
    long FileSize, FileSizeCopied;
    long TotalSize, SizeCopied;
    long TotalFiles, FilesCopied;
    float ProcentCopied;
    struct find_t ffblk;
    BOOLEAN Result = TRUE, Keep_DateTime;
    int currY, KeyPressed;

    // Pre-Fill Variables
    FromDrive    = Alloc_String( "?:", 0);
    ToDrive      = Alloc_String( "?:", 0);
    FromFile     = Alloc_String( NULL, 80);
    ToFile       = Alloc_String( NULL, 80);
    _TempDir     = Alloc_String( NULL, 40);
    _SaveDir     = Alloc_String( NULL, 40);
    _SpoolDir    = Alloc_String( NULL, 40);
    _TraceLogDir = Alloc_String( NULL, 40);
    _BatDir      = Alloc_String( NULL, 40);
    _ResultsDir  = Alloc_String( NULL, 40);
    DirLst       = Alloc_String( NULL, 40);
    FileLst      = Alloc_String( NULL, 40);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 15)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
    {
        strncpy( FromDrive, &Parameters[ 6], 2);
        strncpy( ToDrive,   &Parameters[13], 2);

        sprintf( _TempDir,     "%s%s", FromDrive, TempDir    );
        sprintf( _SaveDir,     "%s%s", FromDrive, SaveDir    );
        sprintf( _SpoolDir,    "%s%s", FromDrive, SpoolDir   );
        sprintf( _TraceLogDir, "%s%s", FromDrive, TraceLogDir);
        sprintf( _BatDir,      "%s%s", FromDrive, BatDir     );
        sprintf( _ResultsDir,  "%s%s", FromDrive, ResultsDir );
    }
    cprintf( "OK\r\n");

	if( Result)
    {
        cprintf( "DigCopy from %s to %s\r\n\r\n", FromDrive, ToDrive);
        DOS_Error = -1;
        FromContents = Alloc_String( Get_CONTENTS( FromDrive), 0);
        if( DOS_Error != -1)
        {
            Result = FALSE;
            cprintf( "Errors or No SOURCE present in %s !\r\n(%s)\r\n",
                     FromDrive, err_msg[DOS_Error]);
        }
        else
        {
            SetMODE( NORMAL);    cprintf( "Copy from ");
            SetMODE( HIGHLIGHT); cprintf( "%s\r\n", FromContents);
            SetMODE( NORMAL);
        }
	}

    if( Result)
    {
        DOS_Error = -1;
        ToContents = Alloc_String( Get_CONTENTS( ToDrive), 0);
        ToContents[strlen( ToContents)] = '\0';
        if( DOS_Error != -1)
        {
            Result = FALSE;
            cprintf( "Errors or No DESTINATION present in %s !\r\n(%s)\r\n",
                     ToDrive, err_msg[DOS_Error]);
        }
        else
        {
            SetMODE( NORMAL);    cprintf( "Copy to   ");
            SetMODE( HIGHLIGHT); cprintf( "%s\r\n", ToContents);
            if( strcmp( ToContents, "????????"))
            {
                PlaySound();
                cprintf( "\r\nALL DATA ON DRIVE %s WILL BE DESTROYED !!!\r\n",
                         ToDrive);
            }
            SetMODE( NORMAL);
        }
    }

    if( Result)
	{
        cprintf( "\r\nPress [S] to start DigCopy ...");
        for( KeyPressed = 0 ;
             KeyPressed != 'S' &&
             KeyPressed != 's' &&
             KeyPressed != C_ESC ; )
        {
            switch( KeyPressed = GetKey( FALSE))
			{
                case 'S'  :
                case 's'  :
                case C_ESC:
                    break;

                default   :
                    PlaySound();
                    break;
            }
        }

        if( KeyPressed == C_ESC)
            Cancelled = TRUE;
        else
        {
            Add_To_Logging( EMPTY, "DigCopy started ...");
            Reset_Timer();
        }

        gotoxy( 1, wherey());
        clreol();
        if( strcmp( ToContents, "????????"))
        {
            gotoxy( 1, wherey() - 4);
            clreol();
            SetMODE( NORMAL);    cprintf( "Copy to   ");
            SetMODE( HIGHLIGHT); cprintf( "????????");
            SetMODE( NORMAL);
            gotoxy( 1, wherey() + 2);
            clreol();
        }
    }

    if( Result && !Cancelled)
    {
        cprintf( "Formatting %s ... Please wait ... ", ToDrive);
		if( !Format_Disk( ToDrive, "E-VOTING", TRUE, TRUE))
		{
			if( !Format_Disk( ToDrive, "E-VOTING", FALSE, TRUE))
            {
                cprintf( "Not ");
                Result = FALSE;
            }
        }
		cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
        Result = Optimize_Database( FromDrive);

    sprintf( DirLst,  "%s\\DIR.LST",  TempDir);
    sprintf( FileLst, "%s\\FILE.LST", TempDir);

    if( Result && !Cancelled)
    {
        cprintf( "Storing directory structure ... Please wait ... ");
        sprintf( DOScmd, "DIR %s\\ /S /OGN /B /AD  > %s", FromDrive, DirLst);
        if( system( DOScmd) != 0)
        {
            Add_To_Logging( ERR, "Error storing directory structure !");
            cprintf( "Not ");
            Result = FALSE;
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Storing complete file structure ... Please wait ... ");
        sprintf( DOScmd, "DIR %s\\ /S /OGN /B /A-D-S > %s", FromDrive, FileLst);
        if( system( DOScmd) != 0)
        {
            Add_To_Logging( ERR, "Error storing complete file structure !");
            cprintf( "Not ");
            Result = FALSE;
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Creating directory structure on %s ... Please wait ... ",
                 ToDrive);
        ifstream DIR( DirLst);
        while( Result && !DIR.eof())
        {
            DIR.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
				Buffer[0] = ToDrive[0];
                if( mkdir( Buffer) != 0)
                {
                    sprintf( LogMessage, "Error creating directory %s !",
                                         Buffer);
                    Add_To_Logging( ERR, LogMessage);
                    Result = FALSE;
                }
            }
        }
        if( !Result)
            cprintf( "Not ");
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Preparing COPY of files from %s to %s ... Please wait ... ",
                 FromDrive, ToDrive);
        ifstream FILE( FileLst);
        TotalSize  = 0;
        TotalFiles = 0;
        while( Result && FILE && !FILE.eof())
        {
            FILE.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
                if( !strncmp(  Buffer,    _TempDir,      strlen( _TempDir    )) ||
                    !strncmp(  Buffer,    _SaveDir,      strlen( _SaveDir    )) ||
                    !strncmp(  Buffer,    _SpoolDir,     strlen( _SpoolDir   )) ||
                    !strncmp(  Buffer,    _TraceLogDir,  strlen( _TraceLogDir)) ||
                    !strncmp(  Buffer,    _ResultsDir,   strlen( _ResultsDir )) ||
                    !strncmp( &Buffer[3], "COMMAND.COM", 11                   )   )
                    continue;

                if( !_dos_findfirst( Buffer, _A_NORMAL|_A_HIDDEN|_A_RDONLY, &ffblk))
                {
                    TotalSize += ffblk.size;
                    TotalFiles++;
                }
                else
                {
                    sprintf( LogMessage, "Error accessing file %s ", Buffer);
                    Add_To_Logging( ERR, LogMessage);
                    Result = FALSE;
                }
            }
		}
        if( !Result || TotalSize  == 0 ||
                       TotalFiles == 0)
        {
            cprintf( "Not ");
            Result = FALSE;         // Force => if Total??? == 0
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Files to copy : %5lu - Size to copy : %10lu bytes\r\n",
                 TotalFiles, TotalSize);
        ifstream FILE( FileLst);
        currY = wherey();
        ProcentCopied = 0;
        SizeCopied    = 0;
        FilesCopied   = 0;
        while( Result && FILE && !FILE.eof())
        {
            FILE.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
                if( !strncmp(  Buffer,    _TempDir,      strlen( _TempDir    )) ||
                    !strncmp(  Buffer,    _SaveDir,      strlen( _SaveDir    )) ||
                    !strncmp(  Buffer,    _SpoolDir,     strlen( _SpoolDir   )) ||
                    !strncmp(  Buffer,    _TraceLogDir,  strlen( _TraceLogDir)) ||
                    !strncmp(  Buffer,    _ResultsDir,   strlen( _ResultsDir )) ||
                    !strncmp( &Buffer[3], "COMMAND.COM", 11                   )   )
                    continue;

                sprintf( FromFile, "%s",   Buffer);
                sprintf( ToFile  , "%c%s", ToDrive[0], &Buffer[1]);
                gotoxy( 1, currY);
                cprintf( "Files copied  : %5lu - Size copied  : %10lu bytes - %3.0f\%",
                         FilesCopied, SizeCopied, ProcentCopied);

                if( !_dos_findfirst( FromFile, _A_NORMAL|_A_HIDDEN|_A_RDONLY, &ffblk))
                    FileSize = ffblk.size;
                else
                    FileSize = 0;
                gotoxy( 1, currY + 1);
                cprintf( "Current File  : %s (%lu bytes)", ToFile, FileSize); clreol();

                if( strncmp( Buffer, _BatDir,  strlen( _BatDir )) == 0)
                    Keep_DateTime = TRUE;
                else
                    Keep_DateTime = FALSE;

                FileSizeCopied = Copy_File( FromFile, ToFile, Keep_DateTime);
                if( FileSizeCopied == -1)
				{
                    sprintf( LogMessage, "Error copying from %s to %s !",
                                         FromFile, ToFile);
                    Result = FALSE;
                }
                else
                {
                    SizeCopied += FileSizeCopied;
                    FilesCopied++;
                    ProcentCopied = (float)SizeCopied / (float)TotalSize * 100;
                    gotoxy( 1, currY);
                    cprintf( "Files copied  : %5lu - Size copied  : %10lu bytes - %3.0f\%",
                             FilesCopied, SizeCopied, ProcentCopied);
                }
            }
        }
        gotoxy( 1, currY + 1); clreol();
        if( TotalFiles    != FilesCopied ||
            TotalSize     != SizeCopied  ||
            ProcentCopied != 100.0         )
            Result = FALSE;
    }

    if( !Cancelled)
    {
        Remove_File( NULL, NULL, DirLst);
        Remove_File( NULL, NULL, FileLst);
    }

    if( Result && !Cancelled)
    {
        Result = Create_ShellScript( "[DigName]", FromDrive, ToDrive, NULL);
        if( Result)
        {
            Elapsed_Time();
            sleep( 2);
        }
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                DIGNAME                                  */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN DigName( char *Parameters)
{
    BOOLEAN Result = TRUE;
    char *FromDrive;

    // Pre-Fill Variables
    FromDrive = Alloc_String( "?:", 0);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 8)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
        strncpy( FromDrive, &Parameters[6], 2);
    cprintf( "OK\r\n");

    if( Result)
    {
        Add_To_Logging( EMPTY, "DigName started ...");
        Result = Create_ShellScript( "[DigName]", FromDrive, NULL, NULL);
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                E-VOTING                                 */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN Digivote( char *Parameters)
{
    BOOLEAN Result = TRUE;
    char *FromDrive;

    // Pre-Fill Variables
    FromDrive = Alloc_String( "?:", 0);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 8)
        Result = FALSE;
    if( !Result)
        cprintf( "Not ");
    else
		strncpy( FromDrive, &Parameters[6], 2);
    cprintf( "OK\r\n");

    if( Result)
        Result = Optimize_Database( FromDrive);

    if( Result && !Cancelled)
    {
        cprintf( "Copying database ... Please wait ... ");
        Result = Copy_Move_File( FromDrive, RootDir, "ELECT.DB",
                                 RAMdrive,  RootDir, "ELECT.DB",
                 COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, VISIBLE);
    }

    if( Result && !Cancelled)
        Result = Copy_Move_File( FromDrive, RootDir, "ELECT.BI",
                                 RAMdrive,  RootDir, "ELECT.BI",
                 COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, VISIBLE);

    if( Result && !Cancelled)
        Result = Copy_Move_File( FromDrive, RootDir, "ELECT.LG",
                                 RAMdrive,  RootDir, "ELECT.LG",
                 COPY_FILE, KEEP_DATETIME, FILE_MUST_EXIST, VISIBLE);

    if( !Result)
        cprintf( "Not ");
    cprintf( "OK\r\n");

    if( Result)
    {
		Add_To_Logging( EMPTY, "e-Voting started ...");
		Result = Create_ShellScript( "[Digivote]", FromDrive, NULL, NULL);
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                VERSION                                  */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN Version( char *Parameters)
{
    BOOLEAN Result = TRUE;
    char *FromDrive;

    // Pre-Fill Variables
	FromDrive = Alloc_String( "?:", 0);

    cprintf( "Checking parameters ... ");
    if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 8)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
        strncpy( FromDrive, &Parameters[6], 2);
    cprintf( "OK\r\n");

    if( Result)
    {
		Add_To_Logging( EMPTY, "Version started ...");
		Result = Create_ShellScript( "[Version]", FromDrive, NULL, NULL);
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                CONTENTS+                                */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN ContentsPlus( char *Parameters)
{
	char *FromDrive, *ContentsFile, *RaRead;
    int KeyPressed, i;
    char Buffer[80];
    ifstream CONTENTS;
    BOOLEAN Result = TRUE;

    // Pre-Fill Variables
    FromDrive    = Alloc_String( "?:", 0);
    ContentsFile = Alloc_String( "?:\\CONTENTS", 0);
    RaRead       = Alloc_String( "RaRead -d ? -n > NUL", 0);

    cprintf( "Checking parameters ... ");
	if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 8)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
    {
        strncpy( FromDrive, &Parameters[ 6], 2);
        sprintf( ContentsFile, "%s\\CONTENTS", FromDrive);
        sprintf( RaRead, "RaRead -d %c -n > NUL", FromDrive[0]);
	}
    cprintf( "OK\r\n");

    for( KeyPressed = 0 ;
         KeyPressed != C_ESC && Result ; )
    {
        clrscr();
        cprintf( "Insert a VOTING diskette in drive %s ...\r\n", FromDrive);
        cprintf( "Press SPACEBAR to view contents ...");
        switch( ( KeyPressed = GetKey( FALSE)))
        {
            case C_ESC     :
				break;

            case C_SPACEBAR:
                cprintf( "\r\n\r\n");

                DOS_Error = -1;
                CONTENTS.open( ContentsFile);
                if( !CONTENTS)
                {
					SetMODE( HIGHLIGHT);
                    if( DOS_Error == -1)
						cprintf( "This is not a DIGIVOTE diskette !\r\n");
                    else
                        cprintf( "Error or No diskette present (%s) !\r\n",
                                 err_msg[DOS_Error]);
                }
                else
                {
					CONTENTS.close();
				    cprintf( "Checking diskette ... Please wait ... ");
					system( RaRead);
					cprintf( "Done\r\n");
					CONTENTS.open( ContentsFile);
					SetMODE( HIGHLIGHT);
                    for( i = 0 ; !CONTENTS.eof() ; i++)
                    {
                        CONTENTS.getline( Buffer, sizeof( Buffer));
                        cprintf( "%s\r\n", Buffer);
                        if( i % 15 == 0 && i > 0)
                        {
							SetMODE( NORMAL);
							KeyPressed = Wait_For_SPACEBAR_or_ESC();
							gotoxy( 1, wherey());
							clreol;
							gotoxy( 1, wherey() - 1);
							SetMODE( HIGHLIGHT);
						}
                    }
                }
                CONTENTS.close();
                SetMODE( NORMAL);
                KeyPressed = Wait_For_SPACEBAR_or_ESC();
				break;

            default:
                PlaySound();
                break;
        }
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                CONTENTS                                 */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN Contents( char *Parameters)
{
	char *FromDrive, *ContentsFile;
    int KeyPressed, i;
    char Buffer[80];
    ifstream CONTENTS;
    BOOLEAN Result = TRUE;

    // Pre-Fill Variables
    FromDrive    = Alloc_String( "?:", 0);
    ContentsFile = Alloc_String( "?:\\CONTENTS", 0);

    cprintf( "Checking parameters ... ");
	if( Result && !strcmp( Parameters, "N/A"))
        Result = FALSE;
    if( Result && strlen( Parameters) != 8)
        Result = FALSE;

    if( !Result)
        cprintf( "Not ");
    else
    {
        strncpy( FromDrive, &Parameters[ 6], 2);
        sprintf( ContentsFile, "%s\\CONTENTS", FromDrive);
	}
    cprintf( "OK\r\n");

    for( KeyPressed = 0 ;
         KeyPressed != C_ESC && Result ; )
    {
        clrscr();
        cprintf( "Insert a VOTING diskette in drive %s ...\r\n", FromDrive);
        cprintf( "Press SPACEBAR to view contents ...");
        switch( ( KeyPressed = GetKey( FALSE)))
        {
            case C_ESC     :
				break;

            case C_SPACEBAR:
                cprintf( "\r\n\r\n");
                SetMODE( HIGHLIGHT);
                DOS_Error = -1;
                CONTENTS.open( ContentsFile);
                if( !CONTENTS)
                {
                    if( DOS_Error == -1)
						cprintf( "This is not a DIGIVOTE diskette !\r\n");
                    else
                        cprintf( "Error or No diskette present (%s) !\r\n",
                                 err_msg[DOS_Error]);
                }
                else
                {
                    for( i = 0 ; !CONTENTS.eof() ; i++)
                    {
                        CONTENTS.getline( Buffer, sizeof( Buffer));
                        cprintf( "%s\r\n", Buffer);
                        if( i % 15 == 0 && i > 0)
                        {
							SetMODE( NORMAL);
							KeyPressed = Wait_For_SPACEBAR_or_ESC();
							gotoxy( 1, wherey());
							clreol;
							gotoxy( 1, wherey() - 1);
							SetMODE( HIGHLIGHT);
						}
                    }
                }
                CONTENTS.close();
                SetMODE( NORMAL);
                KeyPressed = Wait_For_SPACEBAR_or_ESC();
				break;

            default:
                PlaySound();
                break;
        }
    }

    return( Result);
}

/***************************************************************************/
/*                                                                         */
/*                                CLEAN-UP                                 */
/*                                                                         */
/***************************************************************************/
#pragma warn +par
BOOLEAN CleanUp( char *Parameters)
{
	char *FromDrive, *ContentsFile;
	int KeyPressed;
	char Buffer[80];
	ifstream CONTENTS;
	BOOLEAN Result       = TRUE,
			DigivoteDisk = FALSE;

	// Pre-Fill Variables
	FromDrive    = Alloc_String( "?:", 0);
	ContentsFile = Alloc_String( "?:\\CONTENTS", 0);

	cprintf( "Checking parameters ... ");
	if( Result && !strcmp( Parameters, "N/A"))
		Result = FALSE;
	if( Result && strlen( Parameters) != 8)
		Result = FALSE;

	if( !Result)
		cprintf( "Not ");
	else
	{
		strncpy( FromDrive, &Parameters[ 6], 2);
		sprintf( ContentsFile, "%s\\CONTENTS", FromDrive);
	}
	cprintf( "OK\r\n");

	for( KeyPressed = 0 ;
		 KeyPressed != C_ESC && Result ; )
	{
		clrscr();
		cprintf( "Insert a DIGIVOTE disk in drive %s ...\r\n", FromDrive);
		cprintf( "Press SPACEBAR to view contents ...");
		switch( ( KeyPressed = GetKey( FALSE)))
		{
			case C_ESC     :
				break;

			case C_SPACEBAR:
				cprintf( "\r\n\r\n");
				SetMODE( HIGHLIGHT);
				DOS_Error = -1;
				CONTENTS.open( ContentsFile);
				if( !CONTENTS)
				{
					DigivoteDisk = FALSE;
					if( DOS_Error == -1)
						cprintf( "This is not a DIGIVOTE disk !\r\n");
					else
						cprintf( "Error or No disk present (%s) !\r\n",
								 err_msg[DOS_Error]);
				}
				else
				{
					DigivoteDisk = TRUE;
					for( ; !CONTENTS.eof() ; )
					{
						CONTENTS.getline( Buffer, sizeof( Buffer));
						cprintf( "%s\r\n", Buffer);
					}
				}
				CONTENTS.close();
				SetMODE( NORMAL);
				if( DigivoteDisk)
				{
					cprintf( "\r\nCleanup DIGIVOTE disk ?\r\n");
					cprintf( "Press [Y] to clean, any other key to keep disk ...");
					switch( ( KeyPressed = GetKey( FALSE)))
					{
						case 'Y' :
						case 'y' :
							cprintf( "Cleaning in progress ... ");
							if( !Format_Disk( FromDrive, "DIGIVOTE",
											  TRUE, FALSE))
								cprintf( "Not ");
							cprintf( "OK !");
							break;
					}
				}
				KeyPressed = Wait_For_SPACEBAR_or_ESC();
				break;

			default:
				PlaySound();
				break;
		}
	}

	return( Result);
}

void Visualise_Contents( int Y, char *Contents, sLineInfo *LI,
						 int Part, BOOLEAN Extended)
{
	#define LINELENGTH 78
    BOOLEAN Too_Long       = FALSE,
            MiddleOfString = FALSE;
    MODE    CurrMODE       = NORMAL;
    int     i, MaxLen, FromPos;
    char    DateTime      [20],
            Status        [ 6],
            Description   [80],
            ContinueAfter [ 3],
            ContinueBefore[ 3];

	for( i = 0 ; i < strlen( Contents) ; i++)
		if( Contents[i] == '\r')
			Contents[i] = '\0';

	if( Extended)
	{
		strncpy( DateTime,  Contents,     19); DateTime[20] = '\0';
		strncpy( Status,   &Contents[19],  5); Status  [ 6] = '\0';
        MaxLen  = 53;
        FromPos = 25;
    }
    else
    {
        MaxLen  = 77;
        FromPos =  1;
    }

    if( strlen( Contents) > LINELENGTH)
    {
        if( Part)
            MaxLen -= 2;
        if( ( LINELENGTH + ( Part * 10)) > strlen( Contents))
            MiddleOfString = FALSE;
        else
            MiddleOfString = TRUE;
        Too_Long = TRUE;
    }
    else
        Part = 0;

	for( i = 0                                                    ;
		 i < MaxLen &&
         ( ( FromPos - 1) + i + ( Part * 10)) < strlen( Contents) ;
		 i++                                                       )
        Description[i] = Contents[( FromPos - 1) + i + ( Part * 10)];

    for( ; i < MaxLen ; i++)
        Description[i] = ' ';
    Description[i] = '\0';

    if( Extended)
    {
		if( LI->_ERR)
            CurrMODE = HIGHLIGHT;

        WriteOnScreen(  1, Y, DateTime, CurrMODE);
        WriteOnScreen( 20, Y, Status,   LI->_NOK ? REVERSE : CurrMODE);
    }
    WriteOnScreen( FromPos + ( Part ? 2 : 0), Y, Description, CurrMODE);
    if( Too_Long)
    {
        if( Part)
        {
            sprintf( ContinueBefore, "%c%c%c", 0x11, 0xC4, '\0');
            WriteOnScreen( FromPos, Y, ContinueBefore, REVERSE);
        }
        if( !Part || MiddleOfString)
        {
            sprintf( ContinueAfter,  "%c%c%c", 0xC4, 0x10, '\0');
            WriteOnScreen( MaxLen + ( Extended ? ( Part ? 25 : 23)
                                               : ( Part ?  2 :  0)), Y,
                           ContinueAfter,  REVERSE);
        }
	}
}

void Visualise_DATAFILE( char *Input_DataFile, BOOLEAN Extended)
{
    #define MAX_LINES 19
    LineInfo TempLine      = NULL,
             LinkLine      = NULL,
             Link_NOK      = NULL,
             Link_ERR      = NULL,
             FirstOnScreen = NULL,
             LastOnScreen  = NULL;
    unsigned char Buffer [250];
	int i, j, KeyPressed, Part = 0;
    BOOLEAN Result = TRUE;

    ifstream DATAFILE( Input_DataFile, ios::binary);
    // Store all offsets in double linked pointer list ...
    TempLine           = new sLineInfo;
    TempLine->offset   = DATAFILE.tellg();
    TempLine->_NOK     = FALSE;
    TempLine->_ERR     = FALSE;
    TempLine->Next     = NULL;
    TempLine->Prev     = NULL;
    TempLine->Next_NOK = NULL;
    TempLine->Prev_NOK = NULL;
    TempLine->Next_ERR = NULL;
    TempLine->Prev_ERR = NULL;
    FirstLine          = TempLine;
    LastLine           = TempLine;
    DATAFILE.getline( Buffer, sizeof( Buffer));
    if( !DATAFILE || strlen( Buffer) == 0)
        Result = FALSE;
    while( !DATAFILE.eof() && Result)
    {
        TempLine           = new sLineInfo;
        TempLine->offset   = DATAFILE.tellg();
        TempLine->_NOK     = FALSE;
        TempLine->_ERR     = FALSE;
        TempLine->Next     = NULL;
        TempLine->Prev     = LastLine;
        TempLine->Next_NOK = NULL;
        TempLine->Prev_NOK = Link_NOK;
        TempLine->Next_ERR = NULL;
        TempLine->Prev_ERR = Link_ERR;

        LastLine->Next     = TempLine;
        LastLine           = TempLine;
        DATAFILE.getline( Buffer, sizeof( Buffer));
        if( !DATAFILE || strlen( Buffer) == 0)
            Result = FALSE;
        else
            if( Extended)
            {
                if     ( !strncmp( &Buffer[19], "<NOK>", 5))
                {
                    LastLine->_NOK = TRUE;
                    Link_NOK = LastLine;
				}
                else if( !strncmp( &Buffer[19], "<ERR>", 5))
                {
                    LastLine->_ERR = TRUE;
                    Link_ERR = LastLine;
                }
            }
    }
    LastLine->offset = -1;
    LastLine         = LastLine->Prev;
    if( Extended)
        for( LinkLine  = LastLine,      Link_NOK = NULL,
                                        Link_ERR = NULL ;
             LinkLine != NULL                           ;
             LinkLine  = LinkLine->Prev                  )
        {
            LinkLine->Next_NOK = Link_NOK;
            LinkLine->Next_ERR = Link_ERR;
            if( LinkLine->Next->Prev_NOK->offset != LinkLine->Prev_NOK->offset)
                Link_NOK = LinkLine;
            if( LinkLine->Next->Prev_ERR->offset != LinkLine->Prev_ERR->offset)
                Link_ERR = LinkLine;
        }
    // Prepare screen for visualisation ...
    DATAFILE.clear();
    FirstOnScreen = FirstLine;
    LastOnScreen  = FirstLine;
    for( i = 1                                                  ;
         i < MAX_LINES + 1 && LastOnScreen != LastLine->Next    ;
         i++,                 LastOnScreen  = LastOnScreen->Next )
    {
        DATAFILE.seekg( LastOnScreen->offset);
        DATAFILE.getline( Buffer, sizeof( Buffer));
        Visualise_Contents( i, Buffer, LastOnScreen,
                            Part, Extended);
    }
    if( LastOnScreen == NULL)
        LastOnScreen = LastLine;
    else
        LastOnScreen = LastOnScreen->Prev;
    // Scrolling is enabled ...
    for( KeyPressed = 0      ;
         KeyPressed != C_ESC ; )
    {
        HideCursor();
		if( !(KeyPressed = getch()))
            KeyPressed = getch() | 0x100;
        switch( KeyPressed)
        {
            case 'E'     :
            case 'e'     :
                if( FirstOnScreen->Next_ERR == NULL     ||
                    LastOnScreen            == LastLine ||
                    !Extended                             )
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    clrscr();
                    DATAFILE.clear();
                    DATAFILE.seekg( 0, ios::beg);
                    for( i = 1,               FirstOnScreen = FirstOnScreen->Next_ERR,
                                              LastOnScreen  = FirstOnScreen           ;
                         i < MAX_LINES + 1 && LastOnScreen != LastLine->Next          ;
                         i++,                 LastOnScreen  = LastOnScreen->Next       )
                    {
                        DATAFILE.seekg( LastOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( i, Buffer, LastOnScreen,
                                            Part, Extended);
                    }
                    if( LastOnScreen == NULL)
                        LastOnScreen = LastLine;
                    else
                        LastOnScreen = LastOnScreen->Prev;

                    for( ; i < MAX_LINES + 1         &&
                           FirstOnScreen != FirstLine   ; i++ )
                    {
                        gotoxy( 1, 1); insline();
                        FirstOnScreen = FirstOnScreen->Prev;
                        DATAFILE.seekg( FirstOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( 1, Buffer, FirstOnScreen,
                                            Part, Extended);
                    }
                }
				break;

            case 'N'     :
            case 'n'     :
                if( FirstOnScreen->Next_NOK == NULL     ||
                    LastOnScreen            == LastLine ||
                    !Extended                             )
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    clrscr();
                    DATAFILE.clear();
                    DATAFILE.seekg( 0, ios::beg);
                    for( i = 1,               FirstOnScreen = FirstOnScreen->Next_NOK,
                                              LastOnScreen  = FirstOnScreen           ;
                         i < MAX_LINES + 1 && LastOnScreen != LastLine->Next          ;
                         i++,                 LastOnScreen  = LastOnScreen->Next       )
                    {
                        DATAFILE.seekg( LastOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( i, Buffer, LastOnScreen,
                                            Part, Extended);
                    }
                    if( LastOnScreen == NULL)
                        LastOnScreen = LastLine;
                    else
                        LastOnScreen = LastOnScreen->Prev;

                    for( ; i < MAX_LINES + 1         &&
                           FirstOnScreen != FirstLine   ; i++ )
                    {
                        gotoxy( 1, 1); insline();
                        FirstOnScreen = FirstOnScreen->Prev;
                        DATAFILE.seekg( FirstOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( 1, Buffer, FirstOnScreen,
                                            Part, Extended);
                    }
                }
                break;

			case C_LEFT  :
                if( Part == 0)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    Part--;
                    for( i = 1,               LinkLine  = FirstOnScreen ;
                         i < MAX_LINES + 1 && LinkLine != LastLine->Next;
                         i++,                 LinkLine  = LinkLine->Next )
                    {
                        DATAFILE.seekg( LinkLine->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( i, Buffer, LinkLine,
                                            Part, Extended);
                    }
                }
                break;

            case C_RIGHT :
                if( Part == 10)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    Part++;
                    for( i = 1,               LinkLine  = FirstOnScreen ;
                         i < MAX_LINES + 1 && LinkLine != LastLine->Next;
                         i++,                 LinkLine  = LinkLine->Next )
                    {
                        DATAFILE.seekg( LinkLine->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( i, Buffer, LinkLine,
                                            Part, Extended);
                    }
                }
                break;

            case C_HOME  :
				if( FirstOnScreen == FirstLine)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    clrscr();
                    DATAFILE.clear();
                    DATAFILE.seekg( 0, ios::beg);
                    for( i = 1,               FirstOnScreen = FirstLine,
                                              LastOnScreen  = FirstLine         ;
                         i < MAX_LINES + 1 && LastOnScreen != LastLine->Next    ;
                         i++,                 LastOnScreen  = LastOnScreen->Next )
                    {
                        DATAFILE.seekg( LastOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( i, Buffer, LastOnScreen,
                                            Part, Extended);
                    }
                    if( i == MAX_LINES + 1)
                        if( LastOnScreen == NULL)
                            LastOnScreen = LastLine;
                        else
                            LastOnScreen = LastOnScreen->Prev;
                }
                break;

			case C_PGUP  :
            case C_UP    :
                if( FirstOnScreen == FirstLine)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    for( i = ( KeyPressed == C_PGUP ? 1 : MAX_LINES)     ;
                         i < MAX_LINES + 1 && FirstOnScreen != FirstLine ;
                         i++                                   )
                    {
                        gotoxy( 1, 1); insline();
                        FirstOnScreen = FirstOnScreen->Prev;
						LastOnScreen  = LastOnScreen->Prev;
                        DATAFILE.seekg( FirstOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( 1, Buffer, FirstOnScreen,
                                            Part, Extended);
                    }
				}
                break;

            case C_END :
                if( LastOnScreen == LastLine)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
                }
                else
                {
                    clrscr();
                    for( i = 1,               FirstOnScreen  = LastLine,
                                              LastOnScreen   = LastLine           ;
                         i < MAX_LINES + 1 && FirstOnScreen != FirstLine          ;
                         i++,                 FirstOnScreen  = FirstOnScreen->Prev )
                    {
                        gotoxy( 1, 1); insline();
                        DATAFILE.seekg( FirstOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( 1, Buffer, FirstOnScreen,
                                            Part, Extended);
                    }
                    if( i == MAX_LINES + 1)
                        if( FirstOnScreen == NULL)
                            FirstOnScreen = FirstLine;
                        else
                            FirstOnScreen = FirstOnScreen->Next;
                }
                break;

            case C_PGDN:
            case C_DOWN:
                if( LastOnScreen == LastLine)
                {
                    sound( 100);
                    delay( 100);
                    nosound();
				}
                else
                {
                    for( i = ( KeyPressed == C_PGDN ? 1 : MAX_LINES)   ;
                         i < MAX_LINES + 1 && LastOnScreen != LastLine ;
                         i++                                            )
					{
                        gotoxy( 1, 1); delline();
                        FirstOnScreen = FirstOnScreen->Next;
                        LastOnScreen  = LastOnScreen->Next;
                        DATAFILE.seekg( LastOnScreen->offset);
                        DATAFILE.getline( Buffer, sizeof( Buffer));
                        Visualise_Contents( MAX_LINES, Buffer, LastOnScreen,
                                            Part, Extended);
                    }
                }
                break;

            case C_ESC :
                break;

            default    :
                sound( 100);
                delay( 100);
                nosound();
                break;
        }
    }
    DATAFILE.close();
}

BOOLEAN Create_Etallon( void)
{
    char *FromFile, *ToFile, *FileLst;
    long FileSize, FileSizeCopied;
    long TotalSize, SizeCopied;
    long TotalFiles, FilesCopied;
    float ProcentCopied;
    struct find_t ffblk;
    int currY, KeyPressed;
    BOOLEAN Result = TRUE;

    char FileName      [ _MAX_FNAME ];
    char FileExtention [ _MAX_EXT   ];

	// Pre-Fill Variables
    FromFile = Alloc_String( NULL, 80);
    ToFile   = Alloc_String( NULL, 80);
    FileLst  = Alloc_String( NULL, 40);

    SetMODE( REVERSE); clreol();
	WriteCentered( 1, " 'Etalon' Creation\r\n", REVERSE);

    SetMODE( HIGHLIGHT);
    cprintf( "\r\nPut an EMPTY diskette in A: ...");
    cprintf( "\r\nALL DATA ON DISKETTE WILL BE DESTROYED !!!\r\n");
    SetMODE( NORMAL);
    cprintf( "\r\nPress [S] to start creation of 'Etalon' ...");
    for( KeyPressed = 0 ;
         KeyPressed != 'S' &&
         KeyPressed != 's' &&
         KeyPressed != C_ESC ; )
    {
        switch( KeyPressed = GetKey( FALSE))
        {
            case 'S'  :
            case 's'  :
            case C_ESC:
                break;

            default   :
                PlaySound();
                break;
        }
    }

    if( KeyPressed == C_ESC)
        Cancelled = TRUE;
    else
    {
        sprintf( LogMessage, "Etalon started ...");
        Add_To_Logging( EMPTY, LogMessage);
        Reset_Timer();
    }

    gotoxy( 1, wherey());     clreol();
    gotoxy( 1, wherey() - 2); clreol();
    gotoxy( 1, wherey() - 1); clreol();

    if( Result && !Cancelled)
	{
        cprintf( "Formatting A: ... Please wait ... ");
        if( !Format_Disk( "A:", "MASTERÿU", TRUE, TRUE))
            if( !Format_Disk( "A:", "MASTERÿU", FALSE, TRUE))
            {
                cprintf( "Not ");
				Result = FALSE;
            }
        cprintf( "OK\r\n");
    }

    sprintf( FileLst, "%s\\FILE.LST", TempDir);

    if( Result && !Cancelled)
    {
        cprintf( "Storing file structure for 'Etalon' ... Please wait ... ");
        sprintf( DOScmd, "DIR \\VOTE\\FILES\\BAT /OGN /B /S > %s", FileLst);
        if( system( DOScmd) != 0)
        {
            Add_To_Logging( ERR, "Error storing file structure for 'Etalon' !");
            cprintf( "Not ");
            Result = FALSE;
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Preparing COPY of 'Etalon'-files A: ... Please wait ... ");
        ifstream FILE( FileLst);
        TotalSize  = 0;
        TotalFiles = 0;
        while( Result && FILE && !FILE.eof())
        {
            FILE.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
                if( !_dos_findfirst( Buffer, _A_NORMAL|_A_HIDDEN|_A_RDONLY, &ffblk))
                {
                    TotalSize += ffblk.size;
                    TotalFiles++;
                }
                else
                {
                    sprintf( LogMessage, "Error accessing file %s ", Buffer);
					Add_To_Logging( ERR, LogMessage);
                    Result = FALSE;
                }
            }
        }
        FILE.close();
		if( !Result || TotalSize  == 0 ||
                       TotalFiles == 0)
        {
            cprintf( "Not ");
            Result = FALSE;         // Force => if Total??? == 0
        }
        cprintf( "OK\r\n");
    }

    if( Result && !Cancelled)
    {
        cprintf( "Files to copy : %5lu - Size to copy : %10lu bytes\r\n",
                 TotalFiles, TotalSize);
        ifstream FILE( FileLst);
        currY = wherey();
        ProcentCopied = 0;
        SizeCopied    = 0;
        FilesCopied   = 0;
        while( Result && FILE && !FILE.eof())
        {
            FILE.getline( Buffer, sizeof( Buffer));
            if( strlen( Buffer) > 0)
            {
                sprintf( FromFile, "%s",   Buffer);
                _splitpath( Buffer, NULL, NULL, FileName, FileExtention);
                sprintf( ToFile  , "A:\\%s%s", FileName, FileExtention);
                if( ToFile[ strlen( ToFile) - 1] == '.')
                    ToFile[ strlen( ToFile) - 1] = '\0';

                gotoxy( 1, currY);
                cprintf( "Files copied  : %5lu - Size copied  : %10lu bytes - %3.0f\%",
                         FilesCopied, SizeCopied, ProcentCopied);

                if( !_dos_findfirst( FromFile, _A_NORMAL|_A_HIDDEN|_A_RDONLY, &ffblk))
                    FileSize = ffblk.size;
                else
                    FileSize = 0;
                gotoxy( 1, currY + 1);
                cprintf( "Current File  : %s (%lu bytes)", ToFile, FileSize); clreol();

                FileSizeCopied = Copy_File( FromFile, ToFile, TRUE);
                if( FileSizeCopied == -1)
                {
                    sprintf( LogMessage, "Error copying from %s to %s !",
										 FromFile, ToFile);
                    Result = FALSE;
                }
                else
                {
                    SizeCopied += FileSizeCopied;
					FilesCopied++;
                    ProcentCopied = (float)SizeCopied / (float)TotalSize * 100;
                    gotoxy( 1, currY);
                    cprintf( "Files copied  : %5lu - Size copied  : %10lu bytes - %3.0f\%",
                             FilesCopied, SizeCopied, ProcentCopied);
                }
            }
        }
        gotoxy( 1, currY + 1); clreol();
        if( TotalFiles    != FilesCopied ||
            TotalSize     != SizeCopied  ||
            ProcentCopied != 100.0         )
            Result = FALSE;
    }

//  if( !Cancelled)
//      Remove_File( NULL, NULL, FileLst);

    if( !Cancelled)
        Add_To_Logging( Result?OK:NOK, "Etalon ended !");

    if( Result && !Cancelled)
    {
        Elapsed_Time();
        Wait_For_SPACEBAR_or_ESC();
    }
    return( Result);
}

// MAIN PROGRAM
///////////////
#pragma startup logOnStartup
#pragma exit    logOnExit
void main( int argc, char **argv)
{
    int InitX, InitY;
    int KeyPressed, Y, HotY;
    BOOLEAN Result = TRUE, HotKeyPressed;
    ChoiceList MenuItem, HotItem;

    textmode( BW80);

    // Pre-Fill Variables
    // ===> Global Variables
	DOScmd     = Alloc_String( NULL, 100);
	LogMessage = Alloc_String( NULL, 500);
    RAMdrive   = Alloc_String( "?:", 0);
    sprintf( RAMdrive, "%c:", Get_RAMdrive());

	Notify_DOS_Errors = FALSE;
	harderr( handler);

	Remove_File( NULL, NULL, ShellBat);

	if( !Read_MENULST())
	{
		sprintf( LogMessage, "Problems reading %s\\%s !",
		                     RootDir, _CFG_file);
		Add_To_Logging( ERR, LogMessage);
		cout << "\n" << LogMessage << "\n";
		exit( 1);
	}
	InitX = 40 - ( strlen( FirstChoice->Text) / 2);
	InitY = 13 - ( NbrOfItems / 2);
	Refresh_MENU( InitX, InitY);

	Add_To_Logging( EMPTY, "Menu started ...");

	ForcedExit    = FALSE;
	HotKeyPressed = FALSE;

	MenuItem   = FirstChoice;
	Y          = InitY;
	KeyPressed = 0;
	if( argc == 2)
	{
		if( !strncmp( argv[1], "-F:", 3) &&
             strlen ( argv[1]) > 3         )
        {
            while( strcmp( MenuItem->DOSCommand, &argv[1][3]) &&
                   MenuItem->Next != FirstChoice          )
            {
                MenuItem = MenuItem->Next;
                Y++;
            }
            if( !strcmp(MenuItem->DOSCommand, &argv[1][3]))
            {
				JumpToFunction = TRUE;
                KeyPressed     = C_ENTER;
            }
            else
            {
                MenuItem       = FirstChoice;
				Y              = InitY;
                KeyPressed     = 0;
            }
        }
    }
    for( ; KeyPressed != C_F10 ; )
	{
		Show_FreeMem();
		Draw_MenuChoice( InitX, Y, MenuItem->Text, TRUE);
		Show_Description( MenuItem);
		if( !JumpToFunction)
			if( HotKeyPressed)
			{
				KeyPressed    = C_ENTER;
				HotKeyPressed = FALSE;
			}
			else
				KeyPressed    = GetKey( TRUE);

		switch( KeyPressed)
		{
			case C_UP:
				if( MenuItem->Prev == NULL)
					PlaySound();
				else
				{
					Draw_MenuChoice( InitX, Y, MenuItem->Text, FALSE);
					if( MenuItem == FirstChoice)
					{
						MenuItem = FirstChoice->Prev;
						Y = InitY + NbrOfItems - 1;
                    }
                    else
                        for( MenuItem = MenuItem->Prev, Y--            ;
                             !strncmp( MenuItem->DOSCommand, "N/A", 3) ;
                             MenuItem = MenuItem->Prev, Y--             );
                }
                break;

			case C_DOWN:
                if( MenuItem->Next == NULL)
                    PlaySound();
                else
                {
                    Draw_MenuChoice( InitX, Y, MenuItem->Text, FALSE);
					if( MenuItem->Next == FirstChoice)
                    {
                        MenuItem = FirstChoice;
                        Y = InitY;
                    }
                    else
                        for( MenuItem = MenuItem->Next, Y++            ;
                             !strncmp( MenuItem->DOSCommand, "N/A", 3) ;
                             MenuItem = MenuItem->Next, Y++             );
                }
                break;

            case C_ENTER:
                Create_DOS_Window();
				Cancelled = FALSE;
				if( !strncmp( MenuItem->DOSCommand, "fct_", 4))
				{
					if     ( !strcmp( MenuItem->DOSCommand,
									  "fct_DigBack"))
						Result = DigBack( MenuItem->Parameters);
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_DigRest"))
						Result = DigRest( MenuItem->Parameters);
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_DigCopy"))
					{
						Result = DigCopy( MenuItem->Parameters);
						if( Result && !Cancelled)
						{
							KeyPressed = C_F10;
							ForcedExit = TRUE;
						}
					}
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_DigName"))
					{
						Result = DigName( MenuItem->Parameters);
						if( Result)
						{
							KeyPressed = C_F10;
							ForcedExit = TRUE;
						}
					}
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_Digivote"))
					{
						Result = Digivote( MenuItem->Parameters);
						if( Result)
						{
							KeyPressed = C_F10;
							ForcedExit = TRUE;
						}
					}
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_Version"))
					{
						Result = Version( MenuItem->Parameters);
						if( Result)
						{
							KeyPressed = C_F10;
							ForcedExit = TRUE;
						}
					}
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_ContentsPlus"))
						Result = ContentsPlus( MenuItem->Parameters);
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_Contents"))
						Result = Contents( MenuItem->Parameters);
					else if( !strcmp( MenuItem->DOSCommand,
									  "fct_CleanUp"))
						Result = CleanUp( MenuItem->Parameters);
					else
					{
						cprintf( "Error : Function Unknown !\r\n");
						cprintf( "        %s\r\n", MenuItem->DOSCommand);
						Wait_For_SPACEBAR_or_ESC();
					}
					if( !Result)
                    {
                        cprintf( "... Error detected ... %s NOT possible !\r\n",
                                 &MenuItem->DOSCommand[4]);
                        Wait_For_SPACEBAR_or_ESC();
					}
                }
                else
                {
                    ofstream SHELL( ShellBat);
                    if( !SHELL)
                    {
                        cprintf( "Error creating %s !", ShellBat);
                        Wait_For_SPACEBAR_or_ESC();
                    }
                                        else
                    {
                        SHELL << "@Echo OFF\n"
                              << "Call " << MenuItem->DOSCommand << "\n"
                              << "Cd " << RootDir << "\n"
                              << "MENU.BAT\n";
                        KeyPressed = C_F10;
                        ForcedExit = TRUE;
                    }
                    SHELL.close();
                }
                if( !ForcedExit)
                {
					JumpToFunction = FALSE;
                    Destroy_DOS_Window();
                    Refresh_MENU( InitX, InitY);
                }
                break;

            case C_F3 :
				Create_LOG_Window( "Database Logging", dBLOGmenu);
                                Visualise_DATAFILE( dBLogFile, FALSE);
                Destroy_DOS_Window();
                Refresh_MENU( InitX, InitY);
                break;

            case C_F4 :
                Create_LOG_Window( "Digivote Logging", LOGmenu);
                Visualise_DATAFILE( LogFile, TRUE);
                Destroy_DOS_Window();
                Refresh_MENU( InitX, InitY);
                break;

			case C_SHIFT_F4 :
				Create_LOG_Window( "Update Logging", LOGmenu);
				Visualise_DATAFILE( UpdateLog, FALSE);
				Destroy_DOS_Window();
				Refresh_MENU( InitX, InitY);
				break;

			case C_F5 :
                Refresh_MENU( InitX, InitY);
                break;

            case C_F7 :
                if( !strncmp( Get_CONTENTS( "C:"), "MAMA", 4))
                {
                    Create_DOS_Window();
                    if( !Create_Etallon())
                    {
                        cprintf( "... Error detected ... 'Etalon' NOT possible !\r\n");
                        Wait_For_SPACEBAR_or_ESC();
                    }
                    Destroy_DOS_Window();
                    Refresh_MENU( InitX, InitY);
                }
                else
                    PlaySound();
                break;

            case C_F10:
                KeyPressed = C_F10;
                break;

            default   :
				HotKeyPressed = FALSE;
                if( ( KeyPressed > 64 && KeyPressed <  92) ||
                    ( KeyPressed > 96 && KeyPressed < 123)   )
                {
                    if( KeyPressed > 96 && KeyPressed < 123)
                        KeyPressed -= 32;
                    for( HotItem = FirstChoice, HotY = InitY             ;
                         HotItem-> Next != FirstChoice && !HotKeyPressed ;
                         HotItem = HotItem->Next, HotY++                  )
                        if(   KeyPressed       == HotItem->HotKey ||
                            ( KeyPressed + 32) == HotItem->HotKey   )
                            HotKeyPressed = TRUE;

                    if(   KeyPressed       == HotItem->HotKey ||
                        ( KeyPressed + 32) == HotItem->HotKey   )
                    {
                        HotKeyPressed = TRUE;
                        HotItem       = HotItem->Next;
                        HotY++;
                    }
                    if( HotKeyPressed)
                    {
                        Draw_MenuChoice( InitX, Y, MenuItem->Text, FALSE);
                        Y        = --HotY;
                        MenuItem = HotItem->Prev;
                    }
                    else
                        PlaySound();
                }
                else
                    PlaySound();
                break;
         }
    }
    ClearScreen();

    if( ForcedExit)
        Add_To_Logging( EMPTY, "Suspended to DOS ...");
    else
    {
        Add_To_Logging( EMPTY, "Menu stopped !");
        cout << "\nHave a nice day !\n\n";
    }
    textmode( LASTMODE);
}
//=============================================================== EOF BODY ===
/*</body>
</source>
<!-- eof -->*/
