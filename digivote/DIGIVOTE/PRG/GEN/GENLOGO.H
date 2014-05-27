/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genlogo.h" ><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENLOGO.H  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Apr 25 2003 15:49:00  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/Gen/GENLOGO.H-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:23 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>genlogo</fileName>
  <creationDate>01/02/03</creationDate>
  <rights>Steria Benelux</rights>
  <author>JRU</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _genlogo_h              // avoids multiple inclusions of this file
   #define _genlogo_h

#ifndef _APPLICATION_MAV
#ifndef DIAGNOST
//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define TRUE                 1
#define FALSE                0

#define lineHeight           10

#define LOGO_SIGNATURE       "BM"
#define MAX_LOGO_HEIGHT      26
#define MAX_LOGO_WIDTH       MAX_LOGO_HEIGHT * 3
#define MAX_LOGO_BITCOUNT    1
#define MAX_LOGO_COMPRESSION 0
#define MAX_LOGO_COLORSUSED  2

union intel2bytes {
   unsigned int  value_l;
   unsigned char value_c[2];
};

union intel4bytes {
   unsigned long value_l;
   unsigned char value_c[4];
};

// Windows structure : BITMAPFILEHEADER
struct BitmapHeader {
   intel2bytes Signature;       //must be LOGO_SIGNATURE
   intel4bytes FileSize;        //file size in bytes
   intel4bytes reserved;        //unused = '0'
   intel4bytes DataOffset;      //file offset of Raster Data
};

// Windows structure : BITMAPINFOHEADER
struct BitmapInfoHeader {
   intel4bytes Size;            //size of InfoHeader = '40'
   intel4bytes Width;           //bitmap width
   intel4bytes Height;          //bitmap height
   intel2bytes Planes;          //number of planes = '1'
   intel2bytes BitCount;        //bits per pixel
                         // 1 =  monochrome palette | #colors = 1
                         // 4 =  4bit palletized    | #colors = 16
                         // 8 =  8bit palletized    | #colors = 256
                         //16 = 16bit palletized    | #colors = 65536
                         //24 = 24bit palletized    | #colors = 16M
   intel4bytes Compression;     //type of compression
                         // 0 = BI_RGB  no compression
                         // 1 = BI_RLE8 8bit RLE encoding
                         // 2 = BI_RLE4 4bit RLE encoding
   intel4bytes ImageSize;       //(compressed) size of image
   intel4bytes XpixelsPerM;     //horizontal resolution
   intel4bytes YpixelsPerM;     //vertical resolution
   intel4bytes ColorsUsed;      //number of actual used colors
   intel4bytes ColorsImportant; //number of important colors ('0'=all)
};

struct colorsRGB{
   unsigned char colorRed;      //Red intensity
   unsigned char colorGreen;    //Green intensity
   unsigned char colorBlue;     //Blue intensity
   unsigned char reserved;      //unused = '0'
};

union aColorDef {
   unsigned char allColors[4];
   colorsRGB     singleColor;
};

extern BitmapHeader     Header;
extern BitmapInfoHeader InfoHeader;
extern aColorDef        colorDef;

extern intel4bytes realImageSize;

extern unsigned char   *ColorTable;
extern unsigned char   *RasterData;

extern long            sizeColorTable;
extern long            sizeRasterData;

extern int             BytesPerLine;

extern int WRITE,
         ERASE,
         WRITEcolorUsed;
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
extern void flush_keyb( void);
extern void SetBackgroundColor( int bkColor);

extern int LoadBitmapFile( char *BitmapFile);
extern int BitmapStatus( void);
extern int BitmapOK( void);
extern void RasterDataFromBitmapFile( long startX, long startY);
extern long RasterDataFromScreenToFile( char *Dimensions,
                              char *RasterFile,
                              int  leftX, int    topY,
                              int rightX, int bottomY,
							  int showWrittenPixel);
#endif // DIAGNOST
#endif // _APPLICATION_MAV

extern long RasterDataFromFile( int leftX,     int topY,
                        int LogoWidth, int LogoHeight,
                        int SkipBytes,
                        char *RasterFile);
//</prototypes>

#endif // _genlogo_h
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->