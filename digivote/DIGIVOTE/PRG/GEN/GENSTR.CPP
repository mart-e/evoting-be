/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="genstr.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENSTR.CPP  $</workFile>
<revision>$Revision: 1.2 $</revision>
<workFileTimeStamp>$Modtime:   Dec 08 2004 17:06:40  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENSTR.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2005/02/28 15:33:24 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENSTR.CPP</fileName>
  <creationDate>22/09/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Fonctions sur les chaines de caractères</fr>
    <nl>String functies</nl></description>
  <history>
   22/09/93: revision history starting date
   23/11/93: buf fixed in the Length_Of_String function, the passed string must
             be unsigned char * and not char *. This causes an error when
             accessing the array with a character > 127, because the index
             was negative.</history>
  <comment>Functions for string manipulation (splitting ....)</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include <string.h>
#include <alloc.h>
#include <conio.h> // getch() -> test-purpose only
#include <stdlib.h> // exit()

#include "..\gen\gentype.h"
#include "..\gen\genstr.h"

#ifdef _APPLICATION_MAV
   #include "..\mav\mavmes.h"

   #ifdef _PREVIEW
      #include "..\divers\logging.h"
   #endif // _PREVIEW
#endif // _APPLICATION_MAV

#ifdef _APPLICATION_URN
   #include "..\urn\urnmes.h"
#endif // _APPLICATION_URN

//</includes>

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Alloc_String">
<heading>
  <description><en>
    Allocates the memory and copies the Token into the allocated
    string</en></description>
  <parameters>
    <parameter type="return">
      <datatype>char*</datatype>
      <description><en>returns the pointer to the allocated string</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>String</name>
      <description><en>the string to allocate</en></description></parameter>
  </parameters>
  <usage>Name = Alloc_String( String);</usage>
  <checks>memory allocation</checks>
</heading>
<code>*/
// ---------------------------------------------------------------------------
char* Alloc_String (
   char* String
   )
// ---------------------------------------------------------------------------
{
#ifdef _PREVIEW
   if( strlen( String))
   {
      char* Destination = new char [strlen( String) + 1];
      if( Destination == NULL)
      {
         closegraph();
       Add_To_Logging( ERR, "Error allocating memory");

       exit( 1);
     }

      strcpy( Destination, String);
      return Destination;
   }
   else
       return NULL;

#else
   char* Destination = new char [ strlen( String) + 1];

   if ( Destination == NULL)
      Error_Exit( E_OUT_OF_MEMORY);

   return strcpy( Destination, String);

#endif // _PREVIEW
}
//</code></function>


/*<function id="Split_Text">
<heading>
  <description><en>
    retrieve the strings delimited with '|' from another string</en></description>
  <parameters>
    <parameter type="return">
      <datatype>int</datatype>
      <description><en>the real number of strings used to split the original string</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>InText</name>
      <description><en>the input text</en></description></parameter>
    <parameter type="arg">
      <datatype>char *</datatype><name>OutText []</name>
      <description><en>the output text</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Dim</name>
      <description><en>the number of strings to retrieve</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Length</name>
      <description><en>the maximum length of each string in pixels</en></description></parameter>
  </parameters>
  <usage>Split_Text( ElectionName, Text, 3, 620);</usage>
  <comment>
   When you specify a Length value you must take the scale of the
   font in account. For example when you specify
   setusercharsize( 8, 10, 8, 10); the Length should be multiplied
   by 8/10 .</comment>
</heading>
<code>*/
#ifdef _APPLICATION_MAV
// ---------------------------------------------------------------------------
int Split_Text (
   char* InText,                       // text to split
   char* OutText [],                   // pointers to the text bits
   int   Dim,                          // max number of text bits
   int   Length,                       // max length
   UNITS Units                         // units of length
   )
// ---------------------------------------------------------------------------
{
   int i=0;
   int start=0, end=1, cut=0;
   int n_chars = strlen( InText);
   char * Buffer = Alloc_String( InText);

   while( start < n_chars)
   {
      // remove all the lewading spaces
      if( InText [start] == ' ')
      {
         start++;
         cut++;
         end++;
         continue;
      }

      memcpy( Buffer, InText + start, end - start);
      Buffer [end-start] = 0;

      // forget the trailing space to allow correct string centering
      if( Buffer [end-start-1] == ' ') Buffer [end-start-1] = 0;

      // cut the string if it is too long
      if( ( Units == PIXELS && textwidth( Buffer) > Length) ||
          ( Units == CHARS && strlen( Buffer) > Length))
      {
         // there was no space in the string, so we cut at the last position
         if( cut <= start) cut = end - 1;

         // the length is too smalln we can't even display one character
         if( cut == start)
             Error_Exit( E_OUT_OF_MEMORY);

         memcpy( Buffer, InText + start, cut - start);
         Buffer [cut-start] = 0;
         end = start = cut;

         OutText [i] = Alloc_String( Buffer);

         if( ++i >= Dim)
         {
            free( Buffer);
            return i+1;
         }
      }

      // we have found a space that can be used to cut the string
      if( InText [end] == ' ') cut = end;

      // we are at the end of the string and the last part was not too long
      if( end == n_chars)
      {
         OutText [i++] = Alloc_String( Buffer);
         break;
      }

      end++;
   }
   free( Buffer);
   return i;
}
#endif // _APPLICATION_MAV
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->
