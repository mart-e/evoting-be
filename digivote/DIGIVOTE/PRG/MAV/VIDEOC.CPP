/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="videoc.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   videoc.cpp  $</workFile>
  <revision>$Revision: 1.1 $</revision>
  <workFileTimeStamp>$Modtime:   Dec 17 2003 12:32:10  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/DIGIVOTE/Archives/Digivote/Prg/videoc.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2005/04/22 09:03:59 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>videoc.cpp</fileName>
  <creationDate>01/01/2003</creationDate>
  <rights>Steria Benelux</rights>
  <author></author>
  <description>
    <fr></fr>
    <nl></nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <conio.h>                     // getch
#include <stdio.h>                     // sprintf
#include <graphics.h>

#include "..\gen\genvideo.h"

#define _DEBUGMODE   1   // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

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
/*<function id="lijnhor">
<heading>
  <description>
    <en>draws an horizontal line</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>
  </comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void lijnhor (                         // draws an horizontal line ( in px)
   int Row,                            // on row
   int ColFrom,                        // from column
   int ColTo,                          // to column
   int Color
   )
// ---------------------------------------------------------------------------
{
   //int ColorPrev;
   //ColorPrev = getcolor();

   setcolor( Color);
   line( ColFrom, Row, ColTo, Row);

   //setcolor( ColorPrev);

   return;
}
//</code></function>


/*<function id="lijnvert">
<heading>
  <description>
    <en>draws a vertical line in pixels</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>
  </comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void lijnvert (                        // draws a vertical line ( in px)
   int RowFrom,                        // from row
   int Col,                            // on column
   int RowTo,                          // to row
   int Color                           // line color
   )
// ---------------------------------------------------------------------------
{

   //int ColorPrev;
   //ColorPrev = getcolor();

   setcolor( Color);
   line( Col, RowFrom, Col, RowTo);

   //setcolor( ColorPrev);

   return;
}
//</code></function>

/*<function id="bladvul">
<heading>
  <description>
    <en>clear screen to background color</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>
  </comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void bladvul (                         // clear screen to background color
   int Color
   )
// ---------------------------------------------------------------------------
{
   if ( 99 == Color)
   {
      setcolor( BLACK);
      setbkcolor( WHITE);
   }
   else
   {
      setbkcolor( Color);
   }
   cleardevice();

   return;
}
//</code></function>


/*<function id="achtersp">
<heading>
  <description>
    <en>clear screen to background color</en>
  </description>
  <parameters/>
  <usage/>
  <checks/>
  <comment>
  </comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void achtersp (                     // draws a filled box in given color
   int rij1,
   int kol1,
   int rij2,
   int kol2,
   int Color                        // box color
   )
// ---------------------------------------------------------------------------
{

   #if _DEBUGMODE
      sprintf( gcDcio, "Color[%d] BkColor[%d]", getcolor(), getbkcolor());
   setcolor( BLACK);
      outtextxy( 1, getmaxy() - 32, gcDcio);
      getch();
   #endif // _DEBUGMODE

   //setfillstyle( EMPTY_FILL, Color);
   //bar( kol1, rij1, kol2, rij2);
   setcolor( BLACK);
   rectangle( kol1, rij1, kol2, rij2);

   return;
}
//</code></function>


//------------------------------------------------------ PRIVATE FUNCTIONS ---

void main (
   void
   )
{
      int ThisColor;


   Setup_Video();

   // test lijnhor
   //
   for ( ThisColor = 1; ThisColor <= getmaxcolor(); ThisColor++)
   {
      lijnhor( ( getmaxy()/2)+ThisColor*5, 10, getmaxx()- 10, ThisColor);
   }

   getch();
   bladvul ( 99);

   // test lijnvert
   //
   for ( ThisColor = 1; ThisColor <= getmaxcolor(); ThisColor++)
   {
      lijnvert( 10, ( getmaxx()/2)+( ThisColor*3), getmaxy() - 10, ThisColor);
   }

   getch();
   bladvul ( 99);

   // test achtersp
   //
   achtersp( 10, 10, 150, 150, RED);

   getch();
   bladvul ( BLACK);

   closegraph();

   return;
}


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

