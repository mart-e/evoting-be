/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="status.cpp">
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   status.cpp  $</workFile>
<revision>$Revision: 1.3 $</revision>
<workFileTimeStamp>$Modtime:   Mar 03 2003 14:55:38  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Dev/Digivote/Prg/Mav/status.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/02 15:04:23 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>status.cpp</fileName>
  <creationDate>08/2002</creationDate>
  <rights>Steria Benelux</rights>
  <author>AVN</author>
  <description>
    <fr>Librairie de fonctions utilitaires</fr>
    <nl>Utility functielibrary</nl></description>
  <history/>
  <comment>This code provides functions to manage status file between updates.</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <stdlib.h>
#include <fcntl.h>

#include "..\gen\gentype.h"
#include "..\gen\genflop.h"

#include "..\urn\status.h"
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define _DEBUGMODE   0 // :WARNING: turn OFF for production code! OFF=0, ON=1.
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

// Error codes returned by this library
#define LIB_ERRORS  9000
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static void ThrowStatusError (
   void
   );
static RET_CODE FlushDosBuffer (
   FILE* stream
   );
static RET_CODE OpenStatusFile (
   void
   );
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//----------------------------------------------------------------------------
//                                         /!\  definitions DO allocate memory
//-------------------------------------------------------- PRIVATE GLOBALS ---
//<globals>
static char*       		STATUS_FILENAME = "B007";
static int              gError = 0;
static FILE*            gFilePtr;
static struct ftime     gFileTime;      // holds timestamp of statusfile

static int              gState;         // holds state flag
static int              gIsOpen = 0;    // holds state flag
//</globals>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---
/*<function id="ThrowStatusError">
<heading>
  <description>
    <en>Handles library errors</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment>All errors are considered fatal</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static void ThrowStatusError (
   void
   )
{
   gError = 0 - ( LIB_ERRORS + gError);  // add error offset
   #if _DEBUGMODE
      sprintf( gcDcio, " Statuslib error[%d].", gError);
      dcio();
   #endif // _DEBUGMODE

}
//</code></function>

/*<function id="FlushDosBuffer">
<heading>
  <description>
    <en>Forces the DOS buffer to flush</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment>flush trick found in bc 3.1 help concerning fflush function.</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE FlushDosBuffer (
   FILE* stream
   )
{
      int duphandle;

   // flush the stream's internal buffer
   if ( fflush( stream) == EOF)
   {
      gError = 1; goto error;
   }
   else
   {
      // duplicate file handle
      duphandle = dup( fileno( stream));
      if ( duphandle == -1)
      {
         gError = 2; goto error;
      }
      else
      {
         // close duplicate to flush the DOS buffer
         if ( close( duphandle) == -1)
         {
            gError = 3; goto error;
         }
      }
   }

   return SUCCEEDED;

error:
   ThrowStatusError ();

   return ABORTED;

}
//</code></function>

/*<function id="OpenStatusFile">
<heading>
  <description>
    <en>Opens existing image status file.</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment />
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static RET_CODE OpenStatusFile (
   void
   )
// ---------------------------------------------------------------------------
{

   gError = 0;

   // open existing file for update
   if ( ( gFilePtr = fopen( STATUS_FILENAME, "r+b")) == NULL)
   {
      gError = 10; goto error;
   }
   else
   {
      if ( getftime( fileno( gFilePtr), &gFileTime) != 0)
      {
         gError = 20; goto error;
      }
      else
      {
         gIsOpen = 1;
      }
   }

   return SUCCEEDED;

error:
   ThrowStatusError ();

   return ABORTED;
}
//</code></function>

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="SetImageStatus">
<heading>
  <description>
    <en>Stores image state on disk</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment>we use file's timestamp for speed</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE SetImageStatus (
   IMAGE_STATE State                // state of image files
   )
// ---------------------------------------------------------------------------
{
   gError = 0;
   while( Floppy_Request_Disk() == ABORTED); // floppy?

   if ( !gIsOpen)
   {
      if ( OpenStatusFile() != SUCCEEDED)
      {
         gError = 110; goto error;
      }
   }

   switch ( State)
   {
      case IMAGE_STATE_BROKEN:
      {
         gFileTime.ft_hour = 1;
         gFileTime.ft_min = 0;
         break;
      }
      case IMAGE_STATE_CONSISTENT:
      {
         gFileTime.ft_hour = 1;
         gFileTime.ft_min = 1;
         break;
      }
      default:    // reset
      {
         gFileTime.ft_hour = 0;
         gFileTime.ft_min = 0;
         break;
      }
   } // eo switch

   if ( setftime( fileno( gFilePtr), &gFileTime) != 0)
   {
      gError = 120; goto error;
   }
   if ( IMAGE_STATE_CONSISTENT == State)
   {
      // close immediatly iso flush trick...
      if ( fclose( gFilePtr) == EOF)
      {
         gError = 130; goto error;
      }
      else
      {
         gIsOpen = 0;
      }
   }
   else
   {
      // force physical update, without closing
      if ( FlushDosBuffer( gFilePtr) != SUCCEEDED)
      {
         gError = 140; goto error;
      }
   }

   return SUCCEEDED;

error:
   ThrowStatusError ();

   return ABORTED;
}
//</code></function>

/*<function id="GetImageStatus">
<heading>
  <description>
    <en>Reads image state from disk</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment>we use file's timestamp for speed</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE GetImageStatus (
   IMAGE_STATE* ImageState
   )
// ---------------------------------------------------------------------------
{

   gError = 0;
   while( Floppy_Request_Disk() == ABORTED); // floppy?

   if ( !gIsOpen)
   {
      if ( OpenStatusFile() != SUCCEEDED)
      {
         gError = 210; goto error;
      }
   }

   // read timestamp of the statusfile
   if ( getftime( fileno( gFilePtr), &gFileTime) != 0)
   {
      gError = 220; goto error;
   }

   if ( 0 == gFileTime.ft_hour && 0 == gFileTime.ft_min)
   {
      *ImageState = IMAGE_STATE_RESET;
   }
   else
   {
      if ( 1 == gFileTime.ft_hour && 1 == gFileTime.ft_min)
      {
         *ImageState = IMAGE_STATE_CONSISTENT;
      }
      else
      {
         *ImageState = IMAGE_STATE_BROKEN;
      }
   }

   return SUCCEEDED;

error:
   ThrowStatusError ();

   return ABORTED;
}
//</code></function>

/*<function id="CreateStatusFile">
<heading>
  <description>
    <en>Creates the status file on disk</en>
  </description>
  <parameters />
  <usage />
  <checks />
  <comment/>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE CreateStatusFile (
   void
   )
// ---------------------------------------------------------------------------
{
   gError = 0;
   while( Floppy_Request_Disk() == ABORTED); // floppy?

   // create new file for update
   if ( ( gFilePtr = fopen( STATUS_FILENAME, "w+b")) == NULL)
   {
      gError = 410; goto error;
   }
   else
   {
      // close immediatly iso flush trick...
      if ( fclose( gFilePtr) == EOF)
      {
         gError = 420; goto error;
      }
      else
      {
         // forces open of existing file
         gIsOpen = 0;
      }
   }

   return SUCCEEDED;

error:
   ThrowStatusError ();

   return ABORTED;
}
//</code></function>

/*<function id="RemoveStatusFile">
<heading>
  <description>Removes the image status file</description>
  <parameters/>
  <comment/>
</heading>
<code>*/
// ---------------------------------------------------------------------------
RET_CODE RemoveStatusFile (
	void
	)
// ---------------------------------------------------------------------------
{
	if ( Floppy_Remove_File( STATUS_FILENAME) == ABORTED) return ABORTED;

	return SUCCEEDED;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
