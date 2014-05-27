/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="logo.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   logo.cpp  $</workFile>
<revision>$Revision: 1.1 $</revision>
<workFileTimeStamp>$Modtime:   Mar 04 2003 15:41:46  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/logo.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/07 11:32:28 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>logo.cpp</fileName>
  <creationDate>01/02/2003</creationDate>
  <rights></rights>
  <author></author>
  <description>
    <fr>Gestion des logo's</fr>
    <nl>Logo beheer</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <dir.h>                       // defines MAXPATH
#include <fcntl.h>
#include <alloc.h>
#include <graphics.h>
#include <io.h>
#include <stdio.h>                     // printf()

#include "..\mav\logo.h"
#include "..\gen\genlogo.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>

// :WARNING: avn 20020201 -> turn OFF for production code!
#define _DEBUGMODE      0       // debug mode OFF = 0, ON = 1
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

//</defines>
//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
   extern char* Alloc_String( char* String);
//</prototypes>

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//---------------------------------------------------------------- GLOBALS ---
//<globals>
   char* gcpLogoPath;                  // path to logo files
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="setLogoPath">
<heading>
  <description>
    <en>Memorizes the path to the logo files.</en>
  </description>
  <parameters>
    <parameter type="return"><datatype>void</datatype></parameter>
  </parameters>
  <usage>setLogoPath( "C:\\TEMP\");</usage>
  <checks>System will be halted in case memory allocation fails.</checks>
  <comment>This function allows reading logo files that are not located in the
  root of the default drive by setting the path first. It is assumed that the
  given path ALWAYS ends with a '\'.</comment>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void setLogoPath (
   char* cpPath                        // the path string
   )
// ---------------------------------------------------------------------------
{
   gcpLogoPath = Alloc_String( cpPath);
   return;
}
//</code></function>


/*<function id="displayLogo">
<heading>
  <description>
    <en>Display a single logo from raster data file.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description>
        <en>0 indicates succes, negatif values for failure.</en>
      </description>
    </parameter>
  </parameters>
  <usage></usage>
  <checks></checks>
  <comment></comment>
  <author>AVN</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
int displayLogo (
   positionType Position,        // position where to put the logo ( top-left)
   logoType Logo,                // logo properties ( width & height)
   int ElectionId,               // used to identify the raster data file
   int CollegeId,                //  "
   int PartyId                   //  "
   )
// ---------------------------------------------------------------------------
{
      char cFilename[MAXPATH];
      BYTE ByteRead;
      FILE* fHandle;
      long BytesRead;
      int x, y, i;

   // prepare dynamic filename
   if ( !gcpLogoPath) setLogoPath( "C:\\");
   sprintf( cFilename, "%sPL%02d%02d%02d.DAT", gcpLogoPath,
      ElectionId, CollegeId, PartyId);

   // display logo
   RasterDataFromFile( Position.Col, Position.Row,
      Logo.Width, Logo.Height, 0, cFilename);

   return 0;

}
//</code></function>
//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

