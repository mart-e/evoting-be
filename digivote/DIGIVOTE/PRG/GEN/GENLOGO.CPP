/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genlogo.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENLOGO.CPP  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Oct 27 2004 12:11:54  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/Gen/GENLOGO.CPP-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:23 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>LOGO.CPP</fileName>
  <creationDate>01/02/03</creationDate>
  <rights>Steria Benelux</rights>
  <author>JR</author>
  <description>
    <fr>Librairie de gestion des logo's</fr>
    <nl>Library voor beheer vd logo's</nl></description>
  <history></history>
  <comment>All functions needed for handling bitmaps ...</comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <io.h>
#include <math.h>
#include <alloc.h>
#include <conio.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <graphics.h>
#include <sys\stat.h>

#include "..\gen\genlogo.h"
//</includes>

#ifndef _APPLICATION_MAV
#ifndef DIAGNOST

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
BitmapHeader     Header;
BitmapInfoHeader InfoHeader;
aColorDef        colorDef;

intel4bytes realImageSize;

unsigned char   *ColorTable;
unsigned char   *RasterData;

long            sizeColorTable;
long            sizeRasterData;

int             BytesPerLine;

int WRITE,
   ERASE,
   WRITEcolorUsed = FALSE;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="flush_keyb">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void flush_keyb ( void)
// ---------------------------------------------------------------------------
{
   while( kbhit() != 0 )
     getch();
}
//</code></function>


/*<function id="SetBackgroundColor">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>bkColor</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void SetBackgroundColor ( int bkColor)
// ---------------------------------------------------------------------------
{
   setfillstyle( SOLID_FILL, bkColor);
   bar( 0, 0, getmaxx(), getmaxy());
   WRITE = bkColor==WHITE
         ?BLACK
         :WHITE;
   ERASE = bkColor;
   // Set default color to WRITE color
   setcolor( WRITE);
}
//</code></function>


/*<function id="LoadBitmapFile">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>BitmapFile</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int LoadBitmapFile ( char *BitmapFile)
// ---------------------------------------------------------------------------
{
   long bytes;
   int fHandle;

   if( ( fHandle = open( BitmapFile, O_RDONLY | O_BINARY)) == -1) {
     return( 1);
   }
   if( ( bytes = read( fHandle, &Header,
                  sizeof( Header))) == -1) {
     close( fHandle);
     return( 2);
   }
   if( ( bytes = read( fHandle, &InfoHeader,
                  sizeof( InfoHeader))) == -1) {
     close( fHandle);
     return( 3);
   }
   bytes = Header.DataOffset.value_l
       - sizeof( Header)
       - sizeof( InfoHeader);
   ColorTable = ( char *) malloc( bytes);
   if( ( bytes = read( fHandle, ColorTable,
                  bytes)) == -1) {
     close( fHandle);
     return( 4);
   }
   else {
     sizeColorTable    = bytes;
     ColorTable[bytes] = '\0';
   }
   realImageSize.value_l = InfoHeader.ImageSize.value_l;
   if(   Header.FileSize.value_l
      - Header.DataOffset.value_l != InfoHeader.ImageSize.value_l)
     InfoHeader.ImageSize.value_l = Header.FileSize.value_l
                           - Header.DataOffset.value_l;
   RasterData = ( char *) malloc( InfoHeader.ImageSize.value_l);
   if( ( bytes = read( fHandle, RasterData,
                  InfoHeader.ImageSize.value_l)) == -1) {
     close( fHandle);
     return( 5);
   }
   else {
     sizeRasterData    = bytes;
     RasterData[bytes] = '\0';
   }
   close( fHandle);
   BytesPerLine = InfoHeader.ImageSize.value_l
            / InfoHeader.Height.value_l;
   return( 0);
}
//</code></function>


/*<function id="BitmapStatus">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int BitmapStatus ( void)
// ---------------------------------------------------------------------------
{
   int Status = 0;

   if( strncmp( Header.Signature.value_c,
            LOGO_SIGNATURE,
            sizeof( LOGO_SIGNATURE) != 0)) {
     Status = Status ^ ( unsigned char) pow( 2, 0);
   }
   if( InfoHeader.BitCount.value_l    != MAX_LOGO_BITCOUNT) {
     Status = Status ^ ( unsigned char) pow( 2, 1);
   }
   if( InfoHeader.Compression.value_l != MAX_LOGO_COMPRESSION) {
     Status = Status ^ ( unsigned char) pow( 2, 2);
   }
   if( InfoHeader.ColorsUsed.value_l  >  MAX_LOGO_COLORSUSED) {
     Status = Status ^ ( unsigned char) pow( 2, 3);
   }
   if( InfoHeader.Width.value_l       >  MAX_LOGO_WIDTH) {
     Status = Status ^ ( unsigned char) pow( 2, 4);
   }
   if( InfoHeader.Height.value_l      >  MAX_LOGO_HEIGHT) {
     Status = Status ^ ( unsigned char) pow( 2, 5);
   }
   return( Status);
}
//</code></function>


/*<function id="BitmapOK">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg"><datatype>void</datatype></parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int BitmapOK ( void)
// ---------------------------------------------------------------------------
{
   return(
     strncmp( Header.Signature.value_c,
            LOGO_SIGNATURE,
            sizeof( LOGO_SIGNATURE)) == 0    &&
     InfoHeader.BitCount.value_l       == MAX_LOGO_BITCOUNT    &&
     InfoHeader.Compression.value_l    == MAX_LOGO_COMPRESSION &&
     InfoHeader.ColorsUsed.value_l     <= MAX_LOGO_COLORSUSED  &&
     Header.FileSize.value_l
     - Header.DataOffset.value_l       == InfoHeader.ImageSize.value_l);
}
//</code></function>


/*<function id="RasterDataFromBitmapFile">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>long</datatype><name>startX</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>long</datatype><name>startY</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void RasterDataFromBitmapFile ( long startX, long startY)
// ---------------------------------------------------------------------------
{
   int color0,  color1,
      colorX0, colorX1,
      i, j, x, y,
      BitsPerLine;

   strncpy( colorDef.allColors, ColorTable, sizeof( colorDef));
   color0  = ( ( colorDef.singleColor.colorRed   == 0) &&
            ( colorDef.singleColor.colorGreen == 0) &&
            ( colorDef.singleColor.colorBlue  == 0)   )
          ?BLACK
          :WHITE;
   color1  = color0==WHITE ? BLACK     : WHITE;
   // 'Expanded' mode => color aray LIGHTGRAY for WHITE/DARKGRAY for BLACK
   // when => height > MAX_LOGO_HEIGHT
   //      => width  > MAX_LOGO_WIDTH
   colorX0 = color0==WHITE ? LIGHTGRAY : DARKGRAY;
   colorX1 = color1==WHITE ? LIGHTGRAY : DARKGRAY;

   for( i = 0, x = startX, y = startY, BitsPerLine = 0 ;
      i < InfoHeader.ImageSize.value_l ;
      i++) {
     for( j = 7 ; j >= 0 ; j--) {
       if( BitsPerLine < InfoHeader.Width.value_l) {
         x++;
         BitsPerLine++;
         if( ( ( InfoHeader.Height.value_l
               - ( startY - y))         > MAX_LOGO_HEIGHT) ||
            ( ( x - startX)              > MAX_LOGO_WIDTH)) {
            putpixel( x, y, RasterData[i]>>j&0x01?colorX1:colorX0);
         }
         else {
            putpixel( x, y, RasterData[i]>>j&0x01?color1:color0);
         }
         if( RasterData[i]>>j&0x01?color1:color0 == WRITE) {
            WRITEcolorUsed = TRUE;
         }
       }
     }
     if( ( ( i + 1) % BytesPerLine) == 0 &&
        BitsPerLine >= InfoHeader.Width.value_l) {
       if( y <= ( startY - InfoHeader.Height.value_l + 1))
         i = InfoHeader.ImageSize.value_l;
       else {
         x = startX;
         y--;
         BitsPerLine = 0;
       }
     }
   }
}
//</code></function>


/*<function id="RasterDataFromScreenToFile">
<heading>
  <description><en></en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>long</datatype>
      <description><en></en></description>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>Dimensions</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>RasterFile</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>leftX</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>topY</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>rightX</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>bottomY</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>showWrittenPixel</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
long RasterDataFromScreenToFile ( char *Dimensions,
                         char *RasterFile,
                         int  leftX, int    topY,
                         int rightX, int bottomY,
                         int showWrittenPixel)
// ---------------------------------------------------------------------------
{
   unsigned char Byte2Write;
   FILE *fHandle;
   long BytesWritten;
   int i, x, y, BitCount,
      ByteWritten = TRUE;

   // Remove RasterFile if it exists ...
   if( !access( RasterFile, 0x00)) {
     chmod( RasterFile, S_IREAD | S_IWRITE);
     remove( RasterFile);
   }
   // Create RasterFile ...
   if( ( fHandle = fopen( RasterFile, "wb")) == NULL) {
     return( -1);
   }
   for( i = 0 ; i < strlen( Dimensions) ; i++) {
     if( putc( Dimensions[i], fHandle) != Dimensions[i]) {
       fclose( fHandle);
       return( -2);
     }
   }
   for( x  = leftX - 1, y  = topY,   BitCount     = 0,
                            Byte2Write   = 0,
                            BytesWritten = 0,
                            ByteWritten  = TRUE;
      x <= rightX &&  y <= bottomY                    ;
                            BitCount++          ) {
     if( BitCount == 8) {
       BytesWritten++;
       if( putc( Byte2Write, fHandle) != Byte2Write) {
         fclose( fHandle);
         return( -3);
       }
       BitCount    = 0;
       Byte2Write  = 0;
       ByteWritten = TRUE;
     }
     else {
       ByteWritten = FALSE;
     }
     if( Dimensions == NULL) {
       if( getpixel( ++x, y) == WHITE) {
         Byte2Write = Byte2Write ^ ( unsigned char) pow( 2, ( 7 - BitCount));
       }
       if( showWrittenPixel) {
         putpixel( x, y, getpixel( x, y) == WHITE
                     ? BLACK
                     : WHITE);
       }
     }
     else {
       ++x;
       if( getpixel( x, y) == WHITE     ||
          getpixel( x, y) == LIGHTGRAY   ) {
         Byte2Write = Byte2Write ^ ( unsigned char) pow( 2, ( 7 - BitCount));
       }
       if( showWrittenPixel) {
         putpixel( x, y, ( getpixel( x, y) == WHITE ||
                       getpixel( x, y) == LIGHTGRAY)
                     ? BLACK
                     : WHITE);
       }
     }
     if( x >= rightX) {
       x = leftX - 1;
       y++;
     }
   }
   // Check if last byte has also been saved !
   if( !ByteWritten && BitCount) {
     BytesWritten++;
     if( putc( Byte2Write, fHandle) != Byte2Write) {
       fclose( fHandle);
       return( -4);
     }
   }
   fclose( fHandle);
   return( BytesWritten);
}
//</code></function>

#endif // DIAGNOST
#endif // _APPLICATION_MAV

/*<function id="RasterDataFromFile">
<heading>
  <description><en></en>
  </description>
  <parameters>
   <parameter type="return">
     <datatype>long</datatype>
     <description><en></en></description>
   </parameter>
   <parameter type="arg">
      <datatype>int</datatype><name>leftX</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>topY</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>LogoWidth</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>LogoHeight</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>SkipBytes</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
    <parameter type="arg">
      <datatype>char*</datatype><name>RasterFile</name>
      <description><en></en></description>
      <defaultValue/>
    </parameter>
  </parameters>
</heading>
<code>*/
// ---------------------------------------------------------------------------
long RasterDataFromFile ( int leftX,     int topY,
                   int LogoWidth, int LogoHeight,
                   int SkipBytes,
                   char *RasterFile)
// ---------------------------------------------------------------------------
{
   unsigned char ByteRead;
   FILE *fHandle;
   long BytesRead;
   int x, y, i;

   // Read RasterFile ...
   if( ( fHandle = fopen( RasterFile, "rb")) == NULL)
     return( -1);

   //////////////////////////////////////////////////////////////////////
   // Drawing the LOGO ...
   // ====================
   x         = leftX - 1;
   y         = topY;
   BytesRead = 0;
   ByteRead  = getc( fHandle);
   while( !feof( fHandle)) {
     BytesRead++;
     if( BytesRead > SkipBytes) {
       for( i = 7 ; i >= 0 ; i--) {
         if( x < ( leftX + LogoWidth ) &&
            y < ( topY  + LogoHeight)   ) {
            putpixel( ++x, y, ByteRead>>i&0x01?WHITE:BLACK);
         }
         if( x >= ( leftX + LogoWidth - 1)) {
            x = leftX - 1;
            y++;
            if( y > ( topY + LogoHeight)) {
              i = 0;
            }
         }
       }
     }
     ByteRead = getc( fHandle);
   }
   //////////////////////////////////////////////////////////////////////
   fclose( fHandle);

   return( BytesRead);
}
//</code></function>


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

