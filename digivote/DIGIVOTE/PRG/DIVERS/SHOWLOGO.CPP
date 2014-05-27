/////////////////////////////////////////////////////////////////////////
// SHOWLOGO
// ========
//
// Analyze and visualize a Windows Bitmap (BMP)
//
/////////////////////////////////////////////////////////////////////////

#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "..\gen\genlogo.h"
#include "..\gen\genvideo.h"

/////////////////////////////////////////////////////////////////////////

char *errorBitmapFile[] = {
   "Bitmap OK",
   "Error Opening BitMap",
   "Read Failed. (Header)",
   "Read Failed. (InfoHeader)",
   "Read Failed. (ColorTable)",
   "Read Failed. (RasterData)"
};

char *formatWriteError[] = {
   "Error creating %s !",
   "Error writing to %s (Dimensions) !",
   "Error writing to %s (Raster Data) !",
   "Error writing to %s (Last byte of Raster Data) !"
};

char *typeCompression[] = {
	"no compression (BI_RGB)",
	"8bit RLE encoding (BI_RLE8)",
	"4bit RLE encoding (BI_RLE4)"
};

unsigned char *tmpSignature;

int i, sizeofColorTable,
	   GraphicsMode = FALSE,
	   LogoOK       = TRUE;

char lineBuffer[80],
	 tmpText[80];

char *BitmapFile,
	 *RasterFile;

long maxX,
	 maxY;

/////////////////////////////////////////////////////////////////////////

void SwitchToGraphicsMode( void)
{
   if( !GraphicsMode) {
	  GraphicsMode = TRUE;
	  Setup_Video();
	  maxX = getmaxx();
	  maxY = getmaxy();
	  SetBackgroundColor( BLACK);
   }
}

/////////////////////////////////////////////////////////////////////////

void DrawCornerMarks( int Color,
					  int leftX,  int topY,
					  int rightX, int bottomY)
{
   setcolor( Color);
   // Upper Left Corner ...
   line( leftX - 1,  topY - 1,    leftX - 5,  topY - 1   );
   line( leftX - 1,  topY - 1,    leftX - 1,  topY - 5   );
   // Lower Left Corner ...
   line( leftX - 1,  bottomY + 1, leftX - 5,  bottomY + 1);
   line( leftX - 1,  bottomY + 1, leftX - 1,  bottomY + 5);
   // Upper Right Corner ...
   line( rightX + 1, topY - 1,    rightX + 5, topY - 1   );
   line( rightX + 1, topY - 1,    rightX + 1, topY - 5   );
   // Lower Right Corner ...
   line( rightX + 1, bottomY + 1, rightX + 5, bottomY + 1);
   line( rightX + 1, bottomY + 1, rightX + 1, bottomY + 5);
}

/////////////////////////////////////////////////////////////////////////

void WriteRasterDataToFile( char *Dimensions,
							char *RasterFile,
							int  leftX, int    topY,
							int rightX, int bottomY)
{
//   unsigned char Byte2Write;
//   FILE *fHandle;
   long BytesWritten;
//   int i, x, y, BitCount,
//	   ByteWritten = TRUE;

   DrawCornerMarks( LIGHTGRAY, leftX, topY, rightX, bottomY);

   setcolor( ERASE);
   if( LogoOK)
	  outtextxy( 0, maxY - 10,
				 "Press [T] to toggle WHITE/BLACK of LOGO or [Q] to quit...");
   else
	  outtextxy( 0, maxY - 10,
				 "Press [Q] to quit...");
   setcolor( WRITE);
   sprintf( lineBuffer, "Writing raster data to %s ...", RasterFile);
   outtextxy( 0, maxY - 20, lineBuffer);

   if( ( BytesWritten = RasterDataFromScreenToFile( Dimensions,
													RasterFile,
													leftX, topY,
													rightX, bottomY,
													TRUE)           ) < 0) {
	  setcolor( ERASE);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  setcolor( WRITE);
	  sprintf( lineBuffer, formatWriteError[1 - BytesWritten], RasterFile);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  outtextxy( 0, maxY - 10, "Press any key to continue...");
	  flush_keyb();
	  getch();
	  setcolor( ERASE);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  outtextxy( 0, maxY - 10, "Press any key to continue...");
	  setcolor( WRITE);
	  outtextxy( 0, maxY - 10, "Press any key to quit...");
	  return;
   }
   setcolor( ERASE);
   outtextxy( 0, maxY - 20, lineBuffer);
   setcolor( WRITE);
   sprintf( lineBuffer, "Raster data written to %s ! (%ld)",
						RasterFile, BytesWritten);
   outtextxy( 0, maxY - 20, lineBuffer);
   outtextxy( 0, maxY - 10, "Press any key to continue...");
   flush_keyb();
   getch();
   setcolor( ERASE);
   outtextxy( 0, maxY - 20, lineBuffer);
   outtextxy( 0, maxY - 10, "Press any key to continue...");
   setcolor( WRITE);
   if( LogoOK)
	  outtextxy( 0, maxY - 10,
				 "Press [T] to toggle WHITE/BLACK of LOGO or [Q] to quit...");
   else
	  outtextxy( 0, maxY - 10,
				 "Press [Q] to quit...");

   DrawCornerMarks( ERASE, leftX, topY, rightX, bottomY);
}

/////////////////////////////////////////////////////////////////////////

void ShowRasterDataFromFile( int leftX,     int topY,
							 int LogoWidth, int LogoHeight,
							 int SkipBytes,
							 char *RasterFile)
{
   long BytesRead;

   if( SkipBytes == 0) {
	  DrawCornerMarks( LIGHTGRAY, leftX, topY,
								  leftX + LogoWidth  - 1,
								  topY  + LogoHeight - 1);

	  setcolor( ERASE);
	  if( LogoOK)
		 outtextxy( 0, maxY - 10,
					"Press [T] to toggle WHITE/BLACK of LOGO or [Q] to quit...");
	  else
		 outtextxy( 0, maxY - 10,
					"Press [Q] to quit...");
	  setcolor( WRITE);
	  sprintf( lineBuffer, "Reading raster data from %s ...", RasterFile);
	  outtextxy( 0, maxY - 20, lineBuffer);
   }
   if( ( BytesRead = RasterDataFromFile( leftX,     topY,
										 LogoWidth, LogoHeight,
										 SkipBytes,
										 RasterFile)) == -1) {
	  if( SkipBytes == 0) {
		 setcolor( ERASE);
		 outtextxy( 0, maxY - 20, lineBuffer);
		 setcolor( WRITE);
		 sprintf( lineBuffer, "Error reading from %s !", RasterFile);
		 outtextxy( 0, maxY - 20, lineBuffer);
		 outtextxy( 0, maxY - 10, "Press any key to continue...");
		 flush_keyb();
		 getch();
		 setcolor( ERASE);
		 outtextxy( 0, maxY - 20, lineBuffer);
		 outtextxy( 0, maxY - 10, "Press any key to continue...");
		 setcolor( WRITE);
		 outtextxy( 0, maxY - 10, "Press any key to quit...");
	  }
	  else {
		 setcolor( ERASE);
		 sprintf( lineBuffer, "Error reading from %s !", RasterFile);
		 outtextxy( 0, maxY - 20, lineBuffer);
		 outtextxy( 0, maxY - 10, "Press any key to continue...");
	  }
   }
   else if( SkipBytes == 0) {
	  setcolor( ERASE);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  setcolor( WRITE);
	  sprintf( lineBuffer, "Raster data displayed from %s ! (%ld)",
						   RasterFile, BytesRead);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  outtextxy( 0, maxY - 10, "Press any key to continue...");
	  flush_keyb();
	  getch();
	  setcolor( ERASE);
	  outtextxy( 0, maxY - 20, lineBuffer);
	  outtextxy( 0, maxY - 10, "Press any key to continue...");
	  setcolor( WRITE);
	  if( LogoOK)
		 outtextxy( 0, maxY - 10,
					"Press [T] to toggle WHITE/BLACK of LOGO or [Q] to quit...");
	  else
		 outtextxy( 0, maxY - 10,
					"Press [Q] to quit...");

	  DrawCornerMarks( ERASE, leftX, topY,
							  leftX + LogoWidth  - 1,
							  topY  + LogoHeight - 1);
   }
}

/////////////////////////////////////////////////////////////////////////

void ToggleLogoColors( int leftX,  int topY,
					   int rightX, int bottomY)
{
   int x, y;

   for( x  = leftX,    y = topY ;
		x <= rightX && y <= bottomY ; )
   {
	  putpixel( x, y, getpixel( x, y) == WHITE
					  ? BLACK
					  : WHITE);
	  if( ++x > rightX)
	  {
		 x = leftX;
		 y++;
	  }
   }
}

/////////////////////////////////////////////////////////////////////////

void Show_Steria_Logo( void)
{
   int handle;

   //////////////////////////////////////////////////////////////////////
   // Show STERIA logo (Copyright)
   // ============================
   if( ( handle = open( "SHOWLOGO.DAT", O_RDONLY | O_BINARY)) == -1)
   {
	  printf( "Error opening SHOWLOGO.DAT !\n");
	  printf( "\nPress any key to continue ...");
	  flush_keyb();
	  getch();
	  exit( 1);
   }
   if( read( handle, lineBuffer, 4) == -1)
   {
	  close( handle);
	  printf( "Error reading from SHOWLOGO.DAT !\n");
	  printf( "\nPress any key to continue ...");
	  flush_keyb();
	  getch();
	  exit( 1);
   }
   InfoHeader.Width.value_l = atol( lineBuffer);
   if( read( handle, lineBuffer, 4) == -1)
   {
	  close( handle);
	  printf( "Error reading from SHOWLOGO.DAT !\n");
	  printf( "\nPress any key to continue ...");
	  flush_keyb();
	  getch();
	  exit( 1);
   }
   InfoHeader.Height.value_l = atol( lineBuffer);
   close( handle);
   //////////////////////////////////////////////////////////////////////
   SwitchToGraphicsMode();
   cleardevice();
   //////////////////////////////////////////////////////////////////////
   setfillstyle( SOLID_FILL, WHITE);
   bar( 0, 0, maxX, maxY - 11);
   //////////////////////////////////////////////////////////////////////
   char Copyright[] = "Copyright (C) Steria Benelux sa/nv 2003.";
   outtextxy( maxX - ( strlen( Copyright) * 8), maxY - 9, Copyright);
   //////////////////////////////////////////////////////////////////////
   ShowRasterDataFromFile( ( maxX / 2) - ( InfoHeader.Width.value_l  / 2),
						   ( maxY / 2) - ( InfoHeader.Height.value_l / 2),
						   InfoHeader.Width.value_l,
						   InfoHeader.Height.value_l,
						   8, "SHOWLOGO.DAT");
   //////////////////////////////////////////////////////////////////////
   int i,
	   WAIT_SECONDS  = 3,
	   INTERVAL_MSEC = 10;

   for( i = 0 ; i < ( WAIT_SECONDS * 1000) ; i += INTERVAL_MSEC)
   {
	  delay( INTERVAL_MSEC);
	  if( kbhit()) i = ( WAIT_SECONDS * 1000);
   }

   flush_keyb();
   //////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[])
{
   int  BitmapStatus,
		ParamsOK   = TRUE,
		AcceptKeys = TRUE;
   char KeyPressed,
		allowedKey;
   long startX,
		startY;
   long posX = 0,
		posY = 0,
		posCHK,
		posMAX;

   if( argc != 2) {
	  ParamsOK = FALSE;
   }
   else if( strlen( argv[1]) > 8) {
	  ParamsOK = FALSE;
   }
   if( !ParamsOK) {
	  printf( "Syntax : SHOWLOGO.EXE <filename>\n");
	  printf( "------                NO EXTENTION !\n");
	  printf( "                      MAX 8 CHARACTERS !\n\n");
	  printf( "Prerequisites of bitmap :\n");
	  printf( "* Dimensions (WxH in pixels) : %dx%d\n",
				 MAX_LOGO_WIDTH, MAX_LOGO_HEIGHT);
	  printf( "* Bits per pixel............ : %d\n",
				 MAX_LOGO_BITCOUNT);
	  printf( "* Compression............... : %s\n",
				 typeCompression[MAX_LOGO_COMPRESSION]);
	  printf( "* Max colors used........... : %d\n",
				 MAX_LOGO_COLORSUSED);
	  printf( "\nPress any key to continue ...");
	  flush_keyb();
	  getch();
	  printf( "\n");
	  exit( 1);
   }

   //////////////////////////////////////////////////////////////////////
   for( i = 0 ; i < strlen( argv[1]) ; i++)
	 argv[1][i] = toupper( argv[1][i]);

   BitmapFile = ( char *) malloc( strlen( argv[1]) + 5);
   sprintf( BitmapFile, "%s.BMP", argv[1]);
   if( access( BitmapFile, 0x00))
   {
	  printf( "File not found : %s !\n", BitmapFile);
	  printf( "\nPress any key to continue ...");
	  flush_keyb();
	  getch();
	  exit( 1);
   }
   RasterFile = ( char *) malloc( strlen( argv[1]) + 5);
   sprintf( RasterFile, "%s.DAT", argv[1]);

   Show_Steria_Logo();

   SwitchToGraphicsMode();

   SetBackgroundColor( BLACK);

   posCHK = maxX * 0.5;
   posMAX = maxX * 0.75;

   sprintf( lineBuffer, "     [ %s ]", BitmapFile);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   outtextxy( posX, ++posY * lineHeight, "BMP=>");
   if( ( BitmapStatus = LoadBitmapFile( BitmapFile)) != 0) {
	  outtextxy( posX, ++posY * lineHeight,
				 errorBitmapFile[BitmapStatus]);
	  exit( 1);
   }
   // ======
   sprintf( lineBuffer, "     [Header     = %8d]",
                        sizeof( Header));
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "     [InfoHeader = %8d]",
						sizeof( InfoHeader));
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "     [ColorTable = %8d]",
						sizeColorTable);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "     [RasterData = %8d]",
						sizeRasterData);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   tmpSignature = ( char *) malloc( sizeof( Header.Signature));
   strcpy( tmpSignature,
			Header.Signature.value_c);
   tmpSignature[ sizeof( Header.Signature) ] = '\0';

   ////////////////////////////////////////////////////////////////////////
   // HEADER
   // ======
   posY++;
   sprintf( lineBuffer, "Header : %d bytes", sizeof( Header));
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> Signature       (%d bytes) = %s",
		   sizeof( Header.Signature),
		   tmpSignature);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   if( strcmp( tmpSignature, LOGO_SIGNATURE) != 0)
	  outtextxy( posCHK, posY++ * lineHeight, "Not a Windows Bitmap File !");
   else
	  outtextxy( posCHK, posY++ * lineHeight, "OK");
   // ======
   sprintf( lineBuffer, "-> FileSize        (%d bytes) = %d",
		   sizeof( Header.FileSize),
		   Header.FileSize.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> reserved        (%d bytes) = %d",
		   sizeof( Header.reserved),
		   Header.reserved.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> DataOffset      (%d bytes) = %d",
		   sizeof( Header.DataOffset),
		   Header.DataOffset.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);

   ////////////////////////////////////////////////////////////////////////
   // INFOHEADER
   // ==========
   posY++;
   sprintf( lineBuffer, "InfoHeader : %d bytes", sizeof( InfoHeader));
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> Size            (%d bytes) = %d",
		   sizeof( InfoHeader.Size),
		   InfoHeader.Size.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> Width           (%d bytes) = %d",
		   sizeof( InfoHeader.Width),
		   InfoHeader.Width.value_l);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   if( InfoHeader.Width.value_l > MAX_LOGO_WIDTH)
   {
	  sprintf( lineBuffer, "max %3d px !", MAX_LOGO_WIDTH);
	  outtextxy( posCHK, posY * lineHeight, lineBuffer);
	  if( InfoHeader.Width.value_l > maxX)
	  {
		 sprintf( lineBuffer, "> %d", maxX);
		 outtextxy( posMAX, posY * lineHeight, lineBuffer);
	  }
   }
   else
	  outtextxy( posCHK, posY * lineHeight, "OK");
   // ======
   sprintf( lineBuffer, "-> Height          (%d bytes) = %d",
		   sizeof( InfoHeader.Height),
		   InfoHeader.Height.value_l);
   outtextxy( posX, ++posY * lineHeight, lineBuffer);
   if( InfoHeader.Height.value_l > MAX_LOGO_HEIGHT)
   {
	  sprintf( lineBuffer, "max %3d px !", MAX_LOGO_HEIGHT);
	  outtextxy( posCHK, posY * lineHeight, lineBuffer);
	  if( InfoHeader.Height.value_l > maxY)
	  {
		 sprintf( lineBuffer, "> %d", maxY);
		 outtextxy( posMAX, posY * lineHeight, lineBuffer);
	  }
   }
   else
	  outtextxy( posCHK, posY * lineHeight, "OK");
   // ======
   sprintf( lineBuffer, "-> Planes          (%d bytes) = %d",
		   sizeof( InfoHeader.Planes),
		   InfoHeader.Planes.value_l);
   outtextxy( posX, ++posY * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> BitCount        (%d bytes) = %d",
		   sizeof( InfoHeader.BitCount),
		   InfoHeader.BitCount.value_l);
   outtextxy( posX, ++posY * lineHeight, lineBuffer);
   if( InfoHeader.BitCount.value_l > MAX_LOGO_BITCOUNT)
	  sprintf( lineBuffer, "must be monochrome !");
   else
	  sprintf( lineBuffer, "OK");
   outtextxy( posCHK, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> Compression     (%d bytes) = %d",
		   sizeof( InfoHeader.Compression),
		   InfoHeader.Compression.value_l);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   if( InfoHeader.Compression.value_l > MAX_LOGO_COMPRESSION)
	  sprintf( lineBuffer, "no compression allowed !");
   else
	  sprintf( lineBuffer, "OK");
   outtextxy( posCHK, posY++ * lineHeight, "OK");
   // ======
   sprintf( lineBuffer, "-> ImageSize       (%d bytes) = %d",
		   sizeof( InfoHeader.ImageSize),
		   realImageSize.value_l);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   if(   Header.FileSize.value_l
	   - Header.DataOffset.value_l != realImageSize.value_l)
	  sprintf ( lineBuffer, "size mismatch ! (should be %d)",
							InfoHeader.ImageSize.value_l);
   else
	  sprintf( lineBuffer, "OK");
   outtextxy( posCHK, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> XpixelsPerM     (%d bytes) = %d",
		   sizeof( InfoHeader.XpixelsPerM),
		   InfoHeader.XpixelsPerM.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> YpixelsPerM     (%d bytes) = %d",
		   sizeof( InfoHeader.YpixelsPerM),
		   InfoHeader.YpixelsPerM.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> ColorsUsed      (%d bytes) = %d",
		   sizeof( InfoHeader.ColorsUsed),
		   InfoHeader.ColorsUsed.value_l);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   if( InfoHeader.ColorsUsed.value_l > MAX_LOGO_COLORSUSED)
	  sprintf( lineBuffer, "too many colors !");
   else
	  sprintf( lineBuffer, "OK");
   outtextxy( posCHK, posY * lineHeight, lineBuffer);
   if( InfoHeader.ColorsUsed.value_l != sizeofColorTable / sizeof( colorDef))
   {
	  InfoHeader.ColorsUsed.value_l = sizeofColorTable / sizeof( colorDef);
	  sprintf( lineBuffer, "Forced to %d !", InfoHeader.ColorsUsed.value_l);
   }
   else
	  lineBuffer[0] = '\0';
   outtextxy( posMAX, posY++ * lineHeight, lineBuffer);
   // ======
   sprintf( lineBuffer, "-> ColorsImportant (%d bytes) = %d",
		   sizeof( InfoHeader.ColorsImportant),
		   InfoHeader.ColorsImportant.value_l);
   outtextxy( posX, posY++ * lineHeight, lineBuffer);

   ////////////////////////////////////////////////////////////////////////
   // EXTRA INFORMATION
   // =================
   sprintf( lineBuffer, "-> BytesPerLine              = %d", BytesPerLine);
   outtextxy( posX, ++posY * lineHeight, lineBuffer);
   if(   InfoHeader.ImageSize.value_l
	   / InfoHeader.Height.value_l == 0)
	  outtextxy( posCHK, posY * lineHeight, "should be > 0");

   ////////////////////////////////////////////////////////////////////////
   // COLOR TABLE
   // ===========
   if( InfoHeader.ColorsUsed.value_l <= MAX_LOGO_COLORSUSED)
   {
	  posY += 2;
	  outtextxy( posX,   posY * lineHeight, "Defined ColorTable :");
	  outtextxy( posX, ++posY * lineHeight, "ÚÄÄÄÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÂÄÄÄÄÄÄ¿");
	  outtextxy( posX, ++posY * lineHeight, "³ Color ³ Red ³ Green ³ Blue ³");
	  outtextxy( posX, ++posY * lineHeight, "ÃÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÅÄÄÄÄÄÄ´");
	  for( i = 0 ; i < InfoHeader.ColorsUsed.value_l ; i++)
	  {
		 strncpy( colorDef.allColors,
				  &ColorTable[i * sizeof( colorDef)],
				  sizeof( colorDef));
		 sprintf( lineBuffer, "³ bit %1d ³ %3d ³ %5d ³ %4d ³ => %s",
				  i , colorDef.singleColor.colorRed,
					  colorDef.singleColor.colorGreen,
					  colorDef.singleColor.colorBlue,
				  ( ( colorDef.singleColor.colorRed   == 0) &&
					( colorDef.singleColor.colorGreen == 0) &&
					( colorDef.singleColor.colorBlue  == 0)   )
				  ?"BLACK"
				  :( ( ( colorDef.singleColor.colorRed   == 255) &&
					   ( colorDef.singleColor.colorGreen == 255) &&
					   ( colorDef.singleColor.colorBlue  == 255)   )
					 ?"WHITE"
					 :"NOT SUPPORTED (will be seen as WHITE)"));
		 outtextxy( posX, ++posY * lineHeight, lineBuffer);
	  }
	  outtextxy( posX, ++posY * lineHeight, "ÀÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÁÄÄÄÄÄÄÙ");
   }

   ////////////////////////////////////////////////////////////////////////
   // CHECK BITMAP
   // ============
   posY += 2;
   if( BitmapOK) {
	  if( InfoHeader.Width.value_l  > MAX_LOGO_WIDTH  ||
		  InfoHeader.Height.value_l > MAX_LOGO_HEIGHT)  {
		 outtextxy( posX, posY++ * lineHeight, "Bitmap is too large !");
		 if( InfoHeader.Width.value_l  > maxX ||
			 InfoHeader.Height.value_l > maxY) {
			outtextxy( posX, posY * lineHeight, "Press [Q] to quit !");
			allowedKey = 'Q';
			flush_keyb();
			while( toupper( getch()) != allowedKey) {}
			exit( 1);
		 }
		 else {
			sprintf( lineBuffer, "Press [C] to view bitmap...");
			allowedKey = 'C';
		 }
	  }
	  else {
		 sprintf( lineBuffer, "Press [C] to view bitmap...");
		 allowedKey = 'C';
	  }
   }
   else {
	  outtextxy( posX, posY * lineHeight, "Bitmap not conform... Press [Q] to quit !");
	  allowedKey = 'Q';
	  flush_keyb();
	  while( toupper( getch()) != allowedKey) {}
	  exit( 1);
   }
   outtextxy( posX, posY * lineHeight, lineBuffer);
   flush_keyb();
   while( toupper( getch()) != allowedKey) {}
   setcolor( ERASE);
   outtextxy( posX, posY * lineHeight, lineBuffer);
   setcolor( WRITE);

   ////////////////////////////////////////////////////////////////////////
   // SHOW BITMAP
   // ===========
   cleardevice();

   SetBackgroundColor( WHITE);

   startX  = ( ( maxX + 1) / 2) - ( InfoHeader.Width.value_l  / 2) ;
   startY  = ( ( maxY + 1) / 2) + ( InfoHeader.Height.value_l / 2) ;

   RasterDataFromBitmapFile( startX, startY);

   if( InfoHeader.Width.value_l  > MAX_LOGO_WIDTH ||
	   InfoHeader.Height.value_l > MAX_LOGO_HEIGHT) {
	  outtextxy( 0, maxY - 20, "Gray area is too much !");
	  LogoOK = FALSE;
   }
   if( !WRITEcolorUsed) {
	  sprintf( lineBuffer, "Provided LOGO is completely %s !",
						   ERASE==BLACK?"BLACK":"WHITE");
	  if( InfoHeader.Width.value_l  > MAX_LOGO_WIDTH ||
		  InfoHeader.Height.value_l > MAX_LOGO_HEIGHT) {
		 outtextxy( 0, maxY - 30, lineBuffer);
	  }
	  else {
		 outtextxy( 0, maxY - 20, lineBuffer);
	  }
   }
   if( !LogoOK) {
	  for( i = 0 ; i < 3 ; i++) {
		 sound( 700);
		 delay( 100);
		 nosound();
		 delay( 100);
	  }
	  sleep( 1);
   }
   if( LogoOK)
	  outtextxy( 0, maxY - 10,
			   "Press [T] to toggle WHITE/BLACK of LOGO or [Q] to quit...");
   else
	  outtextxy( 0, maxY - 10,
			   "Press [Q] to quit...");
   allowedKey = 'Q';
   flush_keyb();
   while( AcceptKeys) {
	  switch( KeyPressed = getch()) {
		 case 'W':
			if( LogoOK == TRUE) {
			   WriteRasterDataToFile( NULL, RasterFile,
									  startX + 1,
									  startY - InfoHeader.Height.value_l + 1,
									  startX + InfoHeader.Width.value_l,
									  startY);
			}
			else {
			   AcceptKeys = FALSE;
			}
			break;

		 case 'R':
			if( LogoOK == TRUE) {
			   ShowRasterDataFromFile( startX + 1,
									   startY - InfoHeader.Height.value_l + 1,
									   InfoHeader.Width.value_l,
									   InfoHeader.Height.value_l,
									   0, RasterFile);
			}
			else {
			   AcceptKeys = FALSE;
			}
			break;

		 case '@':
			if( InfoHeader.Width.value_l  > MAX_LOGO_WIDTH ||
				InfoHeader.Height.value_l > MAX_LOGO_HEIGHT) {
			   setcolor( ERASE);
			   outtextxy( 0, maxY - 20, "Gray area is too much !");
			}
			sprintf( tmpText, "%.4ld%.4ld", InfoHeader.Width.value_l,
											InfoHeader.Height.value_l);
			WriteRasterDataToFile( tmpText, "SHOWLOGO.DAT",
								   startX + 1,
								   startY - InfoHeader.Height.value_l + 1,
								   startX + InfoHeader.Width.value_l,
								   startY);
			if( InfoHeader.Width.value_l  > MAX_LOGO_WIDTH ||
				InfoHeader.Height.value_l > MAX_LOGO_HEIGHT) {
			   setcolor( WRITE);
			   outtextxy( 0, maxY - 20, "Gray area is too much !");
			}
			break;

		 case 'T':
		 case 't':
			if( LogoOK)
			   ToggleLogoColors( startX + 1,
								 startY - InfoHeader.Height.value_l + 1,
								 startX + InfoHeader.Width.value_l,
								 startY);
			else
			   AcceptKeys = FALSE;
			break;

		 case 'Q':
		 case 'q':
			if( toupper( KeyPressed) == allowedKey) {
			   AcceptKeys = FALSE;
			}
			break;
	  }
   }
   setcolor( ERASE);
   outtextxy( 0, maxY - 20, "Gray area is too much !");
   outtextxy( 0, maxY - 10, "Press any key to quit...");
   setcolor( WRITE);

   free( tmpSignature);
   free( ColorTable);
   free( RasterData);
   free( RasterFile);
   free( BitmapFile);
   closegraph();
   return( 0);
}
