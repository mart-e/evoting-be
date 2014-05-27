/*<?xml version="1.0" encoding="ISO-8859-1"?><source name="genvideo.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENVIDEO.CPP  $</workFile>
<revision>$Revision: 1.3 $</revision>
<workFileTimeStamp>$Modtime:   Dec 08 2004 17:07:08  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENVIDEO.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2005/04/22 09:03:57 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENVIDEO.CPP</fileName>
  <creationDate>01/01/93</creationDate>

  <rights/>
  <author>B.S.S.</author>

  <description>
    <fr>Librairie pour initialiser le mode graphique et les fonctions video</fr>
    <nl>Library voor initialisatie vd grafische mode en video functies</nl></description>
  <history>
    Revision History
    ################

    01/06/93:   revision history starting date</history>
 <comment></comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>                     // printf
#include <stdlib.h>                    // exit
#include <graphics.h>

#include "..\gen\genvideo.h"

#if defined(_PREVIEW) || defined(_PARTLOGO)
   #include "..\divers\logging.h"
#endif // _PREVIEW || _PARTLOGO
//</includes>

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
int MaxPixelX;
int MaxPixelY;
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---
/*<function id="ForceVGA16">
<heading>
  <description><en>Force mode 640x480x256</en>
  </description>
  <parameters></parameters>
  <usage>The function must be used as 2nd parameter in the installuserdriver
  function</usage>
  <checks></checks>
  <comment>Because of limitations of the Borland EGAVGA.BGI driver,
we have to use a Third Party Video driver that is capable
of using the latest technology to be able to present
DEFAULT_FONT in a normal way.</comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int huge ForceVGA16 ( void)
// ---------------------------------------------------------------------------
{
   int Vid = 3;
/*
   printf("Which video mode would you like to use? \n");
   printf("  0) 320x200x16\n");
   printf("  1) 640x200x16\n");
   printf("  2) 640x350x16\n");
   printf("  3) 640x480x256\n");
   printf("  4) 800x600x16\n");
   printf("  5) 1024x768x16\n\n>");
   scanf("%d",&Vid);
*/
   return( Vid);
}
//</code></function>

/*<function id="Setup_Video">
<heading>
  <description><en>
    links the EGA graphic driver and the triplex font to the
    application and puts the screen in graphics mode</en></description>

  <parameters>
    <parameter type="return">
      <datatype>void</datatype><description></description></parameter>

    <parameter type="arg">
      <datatype>void</datatype><description></description></parameter>
  </parameters>

  <usage>
    Setup_Video();    // the function must be called as soon as
                      // possible by the main program</usage>
  <checks>
    The program can only use TRIPLEX font.</checks>
  <comment>
    The EGAVGA.BGI and TRIP.CHR files from the \BORLAND\BGI directory
    were converted in .OBJ files with the BGIOBJ program to avoid the
    necessity to have these files on disk along with the application.
    The error message is in english because we don't know the used
    language at this point.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Setup_Video ( void)
// ---------------------------------------------------------------------------
{
   #if defined(_PREVIEW) || defined(_PARTLOGO)
   char cString[80];
   #endif // _PREVIEW || _PARTLOGO

   int   gdriver,
      gmode,
      iRet;
   char error   = 0;                                             // error flag

   asm {                                           // Grayscale
    mov AH,12H
    mov BL,33H
    mov AL,0
    INT 10H
    }

   gdriver = DETECT;
   installuserdriver( "SVGA16", ForceVGA16);

   if( registerbgidriver( EGAVGA_driver) < 0 ||             // register driver
      registerbgifont( triplex_font)    < 0)         // register triplex font
    error = 1;

   initgraph( &gdriver, &gmode, "");            // initialise the graphic mode

   if( error == 1 || ( iRet = graphresult()) != grOk)                 // check for error
   {
     #if defined(_PREVIEW) || defined(_PARTLOGO)
     sprintf( cString, "Graphics Error - %s", grapherrormsg( iRet));
     Add_To_Logging( ERR, cString);
     exit( 1);
     #else
     printf("\7");                                                    // beep
     printf( "\n\nError: Unable to initialize graphic mode. (%s)\n",
                 grapherrormsg( iRet));
     printf( "       System is halted\n");
     for(;;);                                              // lock the system
     #endif // _PREVIEW || _PARTLOGO
   }
   cleardevice();

   MaxPixelX = getmaxx();
   MaxPixelY = getmaxy();
}
//</code></function>


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
