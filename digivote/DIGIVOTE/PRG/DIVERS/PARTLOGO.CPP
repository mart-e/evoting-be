/////////////////////////////////////////////////////////////////////////
// PARTLOGO
// ========
//
// => Used in PRP (Progress)
//
// Visualise bitmaps and rasterdata
// Import bitmaps
// Export rasterdata
//
/////////////////////////////////////////////////////////////////////////

#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <dir.h>
#include <io.h>
#include <string.h>
#include <fstream.h>
#include <sys\stat.h>

#include "..\divers\logging.h"

#include "..\gen\genlogo.h"
#include "..\gen\genvideo.h"

/////////////////////////////////////////////////////////////////////////

// ----------------
// CANCEL
#define C_ESC    27
#define C_F4    318
// ACCEPT
#define C_ENTER  13
#define C_F1    315
// NAVIGATE
#define C_LEFT  331
#define C_RIGHT 333
#define C_TAB     9
// ----------------

#define nbrFiles 3
#define nbrMsgs  7

typedef enum ioTypes {
   BMP,
   TXT,
   DAT
};

char PartLogoFile[] = "?:\\PARTLOGO.DAT",
    ResultFile[]   = "?:\\PARTLOGO.RES";

char RasterFile[MAXPATH];

char *ioFileName[nbrFiles],
    *Messages[nbrMsgs];

int maxX = 0,
   maxY = 0,
   txtY = 0,
     leftX,
      topY,
    rightX,
   bottomY,
   GraphicsMode = FALSE;

ofstream Result;

/////////////////////////////////////////////////////////////////////////

//#pragma warn +par
char Get_RAMdrive( void)
{
   char Drive;
   struct find_t Control_Block;
   char PathName[6];
   int Found = FALSE;

   Found = FALSE;
   for( Drive='Z' ; Drive>='C' ; Drive--)
   {
      sprintf( PathName, "%c:*.*", Drive);
      if( _dos_findfirst( PathName, _A_VOLID, &Control_Block)==0)
         if( strcmp( Control_Block.name, "MS-RAMDR.IVE")==0)
         {
            Found = TRUE;
            break;
         }
   }
   if( !Found)
      Drive = 'A' + getdisk();
   return( Drive);
}

/////////////////////////////////////////////////////////////////////////

void SwitchToGraphicsMode( void)
{
   int  i, x, y;
   char aChar[2], tmpChar[2];

   if( !GraphicsMode) {
     Setup_Video();
     GraphicsMode = TRUE;
     maxX = getmaxx();
     maxY = getmaxy();
     SetBackgroundColor( BLACK);
     if( WRITE == WHITE) WRITE = LIGHTGRAY;
     if( ERASE == WHITE) ERASE = LIGHTGRAY;
     txtY = 0;
     setcolor( WRITE);
     /* Define textstyle for header information */
     settextstyle( TRIPLEX_FONT, HORIZ_DIR, 0);
     setusercharsize( 2, 3, 2, 3);
   }
}

/////////////////////////////////////////////////////////////////////////

void SwitchToTextMode( void)
{
   if( GraphicsMode) closegraph();
}

/////////////////////////////////////////////////////////////////////////

int ReadInputFile( void)
{
   char    lineBuffer[255];
   char   *Token, typeToken;
   ioTypes ioIndex;

   ifstream InputFile( PartLogoFile);
   while( InputFile) {
     InputFile.getline( lineBuffer, sizeof( lineBuffer));
     if( strlen( lineBuffer) == 0) {
       break;
     }
     Token = strtok( lineBuffer, "\t");
     switch( typeToken = Token[0]) {
       case 'A' : // File Definition Parameters
         Token = strtok( NULL, "\t");
         if( !Token) {
            Result.open( ResultFile);
            Result << "E\t-3\tMissing Token for A-record ! (IO type)";
            Result.close();
            return( -3);
         }
         if(      strcmp( Token, "BMP") == 0) {
            ioIndex = BMP;
         }
         else if( strcmp( Token, "TXT") == 0) {
            ioIndex = TXT;
         }
         else if( strcmp( Token, "DAT") == 0) {
            ioIndex = DAT;
         }
         else {
            Result.open( ResultFile);
            Result << "E\t-4\t" << Token << "\tToken unknown for A-record";
            Result.close();
            return( -4);
         }
         Token = strtok( NULL, "\t");
         if( !Token) {
            Result.open( ResultFile);
            Result << "E\t-3\tMissing Token for A-record ! (filename)";
            Result.close();
            return( -3);
         }
         ioFileName[ioIndex] = ( char *) malloc( strlen( Token) + 1);
         strcpy( ioFileName[ioIndex], Token);
         Token = strtok( NULL, "\t");
         if( Token) {
            Result.open( ResultFile);
            Result << "E\t-3\tToo many Tokens for A-record !";
            Result.close();
            return( -3);
         }
         break;

       case 'B' : // " accept for "
       case 'C' : // " List nø "
       case 'D' : // List nø
       case 'E' : // " List name "
       case 'F' : // List name
       case 'G' : // " Accept "
       case 'H' : // " Cancel "
         Token = strtok( NULL, "\t");
         if( !Token) {
            Result.open( ResultFile);
            Result << "E\t-5\tMissing Token for " << typeToken << "-record !";
            Result.close();
            return( -5);
         }
         Messages[typeToken - 'B'] = ( char *) malloc( strlen( Token) + 1);
         strcpy( Messages[typeToken - 'B'], Token);
         Token = strtok( NULL, "\t");
         if( Token) {
            Result.open( ResultFile);
            Result << "E\t-5\tToo many Tokens for " << typeToken << "-record !";
            Result.close();
            return( -5);
         }
         break;

       default  : // everything else is NOT conform
         Result.open( ResultFile);
         Result << "E\t-2\tInputfile not conform";
         Result.close();
         return( -2);
     }
   }
   InputFile.close();
   return( 0);
}

/////////////////////////////////////////////////////////////////////////

int AcceptChoice( int midX_Accept,
              int midX_Cancel,
              int topY,
              char *msg_Item1,
              char *msg_Item0)
{
   int   Answer   = FALSE,
       Choosen  = FALSE,
       ValidKey = TRUE,
       padding  = 7,
       acceptW  = ( strlen( msg_Item1) + 5) * textwidth( "A"),
       cancelW  = ( strlen( msg_Item0) + 5) * textwidth( "A");
   int   KeyPressed;
   char *Item;

   if( strlen( msg_Item1) > strlen( msg_Item0))
     Item = ( char *) malloc( strlen( msg_Item1) + 5);
   else
     Item = ( char *) malloc( strlen( msg_Item0) + 5);

   if( WRITE == LIGHTGRAY) WRITE = WHITE;
   settextjustify( CENTER_TEXT, TOP_TEXT);
   while( !Choosen) {
     if( ValidKey) {
       setfillstyle( SOLID_FILL, Answer?WRITE:ERASE);
       bar( midX_Accept - ( acceptW / 2), topY - 1,
           midX_Accept + ( acceptW / 2), topY + textheight( "A") + padding);
       setcolor( Answer?ERASE:WRITE);
       if( Answer) sprintf( Item, "¯ %s ®", msg_Item1);
       outtextxy( midX_Accept, topY, Answer?Item:msg_Item1);

       setfillstyle( SOLID_FILL, !Answer?WRITE:ERASE);
       bar( midX_Cancel - ( cancelW / 2), topY - 1,
           midX_Cancel + ( cancelW / 2), topY + textheight( "A") + padding);
       setcolor( !Answer?ERASE:WRITE);
       if( !Answer) sprintf( Item, "¯ %s ®", msg_Item0);
       outtextxy( midX_Cancel, topY, !Answer?Item:msg_Item0);
     }
     ValidKey = TRUE;
     if( !( KeyPressed = getch()))
       KeyPressed = getch() | 0x100;
     switch( KeyPressed) {
       case C_ESC  :
       case C_F4   :
         Answer   = FALSE;
         Choosen  = TRUE;
         break;

       case C_F1   :
       case C_ENTER:
         Choosen  = TRUE;
         break;

       case C_LEFT :
       case C_RIGHT:
       case C_TAB  :
         Answer   = !Answer;
         break;

       default     :
         ValidKey = FALSE;
         break;
     }
   }
   settextjustify( LEFT_TEXT, TOP_TEXT);
   return( Answer);
}

/////////////////////////////////////////////////////////////////////////

int AskConfirmation( int BitmapProvided)
{
   int lines = BitmapProvided?10:7,
      txtH  = textheight( "A"),
      txtW  = textwidth( "A"),
      boxH  = lines * txtH,
      boxL  = 5, //( maxX / 2) + 5,
      boxW  = maxX - 5 - boxL,
      boxT  = txtY + ( ( maxY - txtY) / 2) - ( boxH / 2);

   setcolor( WRITE);
   setlinestyle( SOLID_LINE, 1, 2);
   rectangle( boxL - 2,        boxT + ( txtH / 2) - 2,
           boxL + boxW + 2, boxT + boxH - ( txtH / 2) + 2);
   rectangle( boxL,        boxT + ( txtH / 2),
           boxL + boxW, boxT + boxH - ( txtH / 2));
   boxL += ( txtW / 1.5);
   boxW -= txtW;
   boxT += ( 1.3 * txtH);
   if( BitmapProvided) {
     outtextxy( boxL, boxT, "LOGO");
     setcolor( WHITE);
     outtextxy( boxL + ( 5 * txtW), boxT, ioFileName[BMP]);
     setcolor( WRITE);
     boxT += ( 1.5 * txtH);
     outtextxy( boxL, boxT, Messages[0]?Messages[0]:"accept for");
     boxT += ( 1.5 * txtH);
   }
   outtextxy( boxL, boxT, Messages[1]?Messages[1]:"List nø");
   setcolor( WHITE);
   outtextxy( boxL + ( Messages[1]
                  ? strlen( Messages[1]) + 1
                  : 8) * txtW, boxT, Messages[2]?Messages[2]
                                        :"(see selected)");
   setcolor( WRITE);
   boxT += ( 1.5 * txtH);
   outtextxy( boxL, boxT, Messages[3]?Messages[3]:"List name");
   setcolor( WHITE);
   outtextxy( boxL + ( Messages[3]
                  ? strlen( Messages[3])
                  : 10) * txtW, boxT, Messages[4]?Messages[4]
                                        :"(see selected)");
   setcolor( WRITE);
   boxT += ( 1.5 * txtH);
   return( AcceptChoice( boxL + ( boxW * 0.25),
                   boxL + ( boxW * 0.75),
                   boxT,
                   Messages[5]?Messages[5]:"Accept",
                   Messages[6]?Messages[6]:"Cancel"));
}

/////////////////////////////////////////////////////////////////////////

int ExportBitmapToText( int  leftX, int    topY,
                  int rightX, int bottomY)
{
   unsigned char ByteRead;
   FILE *fHandle;
   char *RasterData;
   long BytesWritten, BytesRead;
   char tmpDrive[_MAX_DRIVE];
   char tmpDir  [_MAX_DIR  ];
   char tmpFile [_MAX_FNAME];
   char tmpExt  [_MAX_EXT  ];

   RasterData = ( char *) malloc( strlen( ioFileName[TXT]) + 1);
   strcpy( RasterData, ioFileName[TXT]);
   _splitpath(RasterData, tmpDrive, tmpDir, tmpFile, tmpExt);
   tmpFile[0] = '~';
   _makepath(RasterData, tmpDrive, tmpDir, tmpFile, tmpExt);
   BytesWritten = RasterDataFromScreenToFile( NULL, RasterData,
                                   leftX, topY,
                                   rightX, bottomY,
                                   FALSE);
   if( BytesWritten < 0) {
     remove( RasterData);
     return( BytesWritten);
   }
   // Read RasterFile ...
   if( ( fHandle = fopen( RasterData, "rb")) == NULL) {
     remove( RasterData);
     return( -1);
   }
   // Create text version of RasterFile ...
   ofstream TextFile( ioFileName[TXT]);
   TextFile << "A\t" << ioFileName[BMP]           << "\n"
         << "B\t" << InfoHeader.Width.value_l  << "\n"
         << "C\t" << InfoHeader.Height.value_l << "\n"
         << "D\t" << BytesWritten              << "\n";
   BytesRead = 0;
   ByteRead  = getc( fHandle);
   while( !feof( fHandle)) {
     BytesRead++;
     TextFile << "E\t" << ( int) ByteRead << "\n";
     ByteRead = getc( fHandle);
   }
   //////////////////////////////////////////////////////////////////////
   TextFile.close();
   fclose( fHandle);
   remove( RasterData);
   return( 0);
}

/////////////////////////////////////////////////////////////////////////

int ExportTextFileToRasterFile( char *InputFile,
                        int  *Width,
                        int  *Height)
{
   unsigned char Byte2Write;
   FILE *fHandle = NULL;
   char  lineBuffer[255],
      *Token;

   ifstream TextFile( InputFile);
   while( !TextFile.eof()) {
     TextFile.getline( lineBuffer, sizeof( lineBuffer));
     Token = strtok( lineBuffer, "\t");
     switch( Token[0]) {
       case 'A' : // destination file
         // Close RasterFile if one has been opened before !
         if( fHandle) fclose( fHandle);
         Token = strtok( NULL, "\t");
         if( !Token) {
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-3\tMissing Token for A-record !";
            Result.close();
            return( -3);
         }
         strcpy( RasterFile, Token);
         // Create RasterFile ...
         if( ( fHandle = fopen( RasterFile, "wb")) == NULL) {
            Result.open( ResultFile);
            Result << "E\t-1\tError creating raster data file "
                 << RasterFile << " (" << Token << ") !";
            Result.close();
            return( -1);
         }
         Token = strtok( NULL, "\t");
         if( Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-4\tToo many Tokens for A-record !";
            Result.close();
            return( -4);
         }
         break;

       case 'B' : // width
         if( !fHandle) {
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-2\tNo file to write to ! (B-record)";
            Result.close();
            return( -2);
         }
         Token = strtok( NULL, "\t");
         if( !Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-5\tMissing Token for B-record !";
            Result.close();
            return( -5);
         }
         *Width = atoi( Token);
         Token = strtok( NULL, "\t");
         if( Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-6\tToo many Tokens for B-record !";
            Result.close();
            return( -6);
         }
         break;

       case 'C' : // height
         if( !fHandle) {
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-2\tNo file to write to ! (C-record)";
            Result.close();
            return( -2);
         }
         Token = strtok( NULL, "\t");
         if( !Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-7\tMissing Token for C-record !";
            Result.close();
            return( -7);
         }
         *Height = atoi( Token);
         Token  = strtok( NULL, "\t");
         if( Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-8\tToo many Tokens for C-record !";
            Result.close();
            return( -8);
         }
         break;

       case 'D' : // rasterdata
         if( !fHandle) {
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-2\tNo file to write to ! (D-record)";
            Result.close();
            return( -2);
         }
         Token = strtok( NULL, "\t");
         if( !Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-9\tMissing Token for D-record !";
            Result.close();
            return( -9);
         }
         Byte2Write = atoi( Token);
         if( putc( Byte2Write, fHandle) != Byte2Write) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-10\tError writing to raster data file"
                 << " (" << RasterFile << ") !";
            Result.close();
            return( -10);
         }
         Token = strtok( NULL, "\t");
         if( Token) {
            fclose( fHandle);
            TextFile.close();
            Result.open( ResultFile);
            Result << "E\t-11\tToo many Tokens for D-record !";
            Result.close();
            return( -11);
         }
         break;

       default  :
         break;
     }
   }
   if( fHandle) fclose( fHandle);
   TextFile.close();
   return( 0);
}

/////////////////////////////////////////////////////////////////////////

int VisualizeLogo( int BitmapProvided,
               int LogoWidth,
               int LogoHeight)
{
   int borderBMP;

   borderBMP = ( maxX / 4) / 8;
   leftX     = ( maxX / 4) + borderBMP;
   topY      = txtY + borderBMP;
   rightX    = ( ( maxX / 4) * 3 ) - borderBMP;
   bottomY   = ( maxY / 2) - borderBMP;

   setfillstyle( SOLID_FILL, LIGHTGRAY);
   bar( leftX, topY, rightX, bottomY );
   setfillstyle( SOLID_FILL, BLACK);
   bar( leftX + 3, topY + 3, rightX - 3, bottomY - 3);
   setfillstyle( SOLID_FILL, WHITE);
   bar( leftX + 4, topY + 4, rightX - 4, bottomY - 4);
   if( BitmapProvided) {
     RasterDataFromBitmapFile( leftX + ( ( rightX - leftX) / 2)
                             - ( LogoWidth / 2),
                        topY  + ( ( bottomY - topY) / 2)
                             + ( LogoHeight / 2));
   }
   else {
     RasterDataFromFile( leftX + ( ( rightX - leftX) / 2)
                        - ( LogoWidth / 2),
                    topY  + ( ( bottomY - topY) / 2)
                        - ( LogoHeight / 2),
                    LogoWidth, LogoHeight, 0, RasterFile);
   }
   txtY = maxY / 2;
   return( AskConfirmation( BitmapProvided));
}

/////////////////////////////////////////////////////////////////////////

#pragma startup logOnStartup
#pragma exit    logOnExit
void main( void)
{
   int statusBMP,
      ConfirmationAnswer,
      exportStatus;
   int i,
      Width,
      Height;

   PartLogoFile[0] = Get_RAMdrive();
   ResultFile[0]   = PartLogoFile[0];
   //////////////////////////////////////////////////////////////////////
   if( access( PartLogoFile, 0x00)) {
     Result.open( ResultFile);
     Result << "E\t-1\t" << PartLogoFile;
     Result.close();
     exit( 1);
   }
   //////////////////////////////////////////////////////////////////////
   if( ReadInputFile() != 0) {
     exit( 2);
   }
   //////////////////////////////////////////////////////////////////////
   // If BMP and TXT filenames are provided ...
   // -> visualize BMP for approval ...
   // -> if BMP meets technical requirements,
   //    ask to accept BMP (… la Progress)
   // -> if BMP is accepted, write TXT file for import in Progress
   // -------------------------------------------------------------------
   if(       ioFileName[BMP] &&
          ioFileName[TXT] &&
         !ioFileName[DAT]   ) {
     if( ( statusBMP = LoadBitmapFile( ioFileName[BMP])) == 0) {
       if( ( statusBMP = BitmapStatus()) == 0) {
         SwitchToGraphicsMode();
         ConfirmationAnswer = VisualizeLogo( TRUE,
                                    InfoHeader.Width.value_l,
                                    InfoHeader.Height.value_l);
         if( ConfirmationAnswer == TRUE) {
            exportStatus = ExportBitmapToText(
                         leftX + ( ( rightX - leftX) / 2)
                              - ( InfoHeader.Width.value_l  / 2) + 1,
                         topY  + ( ( bottomY - topY) / 2)
                              - ( InfoHeader.Height.value_l / 2) + 1,
                         leftX + ( ( rightX - leftX) / 2)
                              - ( InfoHeader.Width.value_l  / 2)
                              + InfoHeader.Width.value_l,
                         topY  + ( ( bottomY - topY) / 2)
                              - ( InfoHeader.Height.value_l / 2)
                              + InfoHeader.Height.value_l
                       );
            if( exportStatus == 0) {
              Result.open( ResultFile);
              Result << "A\t" << ConfirmationAnswer;
              Result.close();
            }
            else {
              Result.open( ResultFile);
              Result << "A\t" << exportStatus;
              Result.close();
            }
         }
         else {
            Result.open( ResultFile);
            Result << "A\t" << ConfirmationAnswer;
            Result.close();
         }
         SwitchToTextMode();
       }
       else {
         Result.open( ResultFile);
         Result << "E\t" << statusBMP;
         Result.close();
       }
     }
     else {
       Result.open( ResultFile);
       Result << "E\t-" << statusBMP;
       Result.close();
     }
   }
   //////////////////////////////////////////////////////////////////////
   // If only the TXT filename is provided ...
   // -> visualize BMP (raster data) ...
   // -> Possibility to remove LOGO ...
   // -------------------------------------------------------------------
   else if( !ioFileName[BMP] &&
          ioFileName[TXT] &&
         !ioFileName[DAT]   ) {
     if( ExportTextFileToRasterFile( ioFileName[TXT],
                             &Width,
                             &Height) == 0) {
       SwitchToGraphicsMode();
       Result.open( ResultFile);
       Result << "A\t" << VisualizeLogo( FALSE,   Width,   Height);
       Result.close();
       SwitchToTextMode();
     }
   }
   //////////////////////////////////////////////////////////////////////
   // If only the DAT filename is provided ...
   // -> make (all) raster data file(s) needed for MAV ...
   // -------------------------------------------------------------------
   else if( !ioFileName[BMP] &&
         !ioFileName[TXT] &&
          ioFileName[DAT]   ) {
     if( ExportTextFileToRasterFile( ioFileName[DAT],
                             &Width,
                             &Height) == 0) {
       Result.open( ResultFile);
       Result << "A\t1";
       Result.close();
     }
   }
}

