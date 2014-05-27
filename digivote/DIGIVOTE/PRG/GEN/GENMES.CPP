/*<?xml version="1.0" encoding="ISO-8859-1"?>

<source name="genmes.cpp"><!-- use lower case for name -->

<pvcs/>

<comments>
  <fileName>genmes.cpp</fileName>
  <creationDate>01/10/2002</creationDate>

  <rights>Steria Belux</rights>
  <author>AVN</author>

  <description>
    <fr>Librairie de traitement des messages.</fr>
    <nl>Boodschappenbeheer library</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
//<system></system>
//<user></user>
#include "..\gen\genlang.h"
#include "..\gen\genmes.h"
//</includes>

//                                <!-- declarations do NOT allocate memory -->
//---------------------------------------------------------------- DEFINES ---
//<defines>
//</defines>

//--------------------------------------------------------------- TYPEDEFS ---

//                                        <!-- these functions are private -->
//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---

//                                     <!-- definitions DO allocate memory -->
//---------------------------------------------------------------- GLOBALS ---
//<globals>
#include "..\gen\genmsg.cpp"
//</globals>

//============================================================= EOF HEADER ===
//</header>

//=============================================================== BOF BODY ===
//<body>

/*<function id="Get_Text">
<heading>
  <description><en>
    returns the text corresponding to a Mnemonic and a Language from
    the message array</en></description>
  <parameters>
    <parameter type="return">
      <datatype>char*</datatype>
      <description><en>the desired text string</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iMnemo</name>
      <description><en>the mnemonic of the desired message</en></description></parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>iLanguage</name>
      <description><en>the language code defining the column in the message array</en></description></parameter>
    <parameter type="global">
      <datatype>sMSG</datatype><name>MSG_GEN[MAX_MSG_GEN]</name>
      <description><en>the message array</en></description></parameter>
  </parameters>
  <usage>
    strcpy ( Text, Get_Text(B_LANGUAGE, Root.Language [0]));</usage>
  <checks>
    domain of Mnemo and Language parameters</checks>
  <comment>
    This function was created to avoid the declaration of the
    message array in the other functions. All the functions that
    need a single line text from the message array MUST use this
    function.
    This function can only be used for single line text, the
    returned text is the text that's on the same line as the Mnemo.</comment>
</heading>
<code>*/
// ---------------------------------------------------------------------------
char* Get_Text( int iMnemo, int iLanguage)
// ---------------------------------------------------------------------------
{
    /* :WARNING: avn 20021009 - structure of message array has been modified
       but there is no need to change this function since it was used to retrieve
       single lines of text only. We can therefore suppose that existing call's
       will behave as usual... */

    if ( ( iMnemo < 0) || ( iMnemo > MAX_MSG_GEN))
    {
        return "Mnemo out of range!";
    }
    if ( ( iLanguage < 0) || ( iLanguage > N_LANG))
    {
        return "Language out of range!";
    }

   return ( MSG_GEN[iMnemo].text[iLanguage]);
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

