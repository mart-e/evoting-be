/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="display.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   DISPLAY.CPP  $</workFile>
<revision>$Revision: 1.4 $</revision>
<workFileTimeStamp>$Modtime:   Jan 10 2005 15:08:48  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/DISPLAY.cpp-arc  $</archive>
<archiveTimeStamp>$Date: 2007/02/05 10:39:16 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>DISPLAY.CPP</fileName>
  <creationDate>05/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Affichage des candidats</fr>
    <nl>Voorstelling van de kandidaten</nl></description>
  <history/>
 <comment>
    Handles all the drawings of the complete screen
    for display of the candidates</comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
//======================== I N C L U D E   F I L E S ==========================
//      All  EXTERNAL  Variables and Functions  are declared in include files
//
//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\mav\display.h"    // makes functions of Display.cpp available for others
#include "..\mav\drawings.h"   // external functions of display.cpp

#include "..\mav\mavvideo.h"   // for Screen_White
#include "..\mav\mavasm.h"     // for VGA_MODE

#define _DEBUGMODE      FALSE   // :WARNING: avn 20020201 - must be FALSE for production code!
#if _DEBUGMODE
   #include "..\tools\debug.h"
#endif  // _DEBUGMODE

//</includes>

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>

//</prototypes>

//-------------------------------------------------------------- EXTERNALS ---
extern int Allready_Voted;

//============================================================= EOF HEADER ===
//</header>

//<body>
//=============================================================== BOF BODY ===

//------------------------------------------------------ PRIVATE FUNCTIONS ---


//------------------------------------------------------- PUBLIC FUNCTIONS ---

/*<function id="Calculate_Mode">
<heading>
  <description>
    <en>Defines the display mode for the current election.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>DISP_ELECT_MODE</datatype>
      <description><en>enum MODE14 or MODE22</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>ElectionList</datatype><name>ThisElection</name>
      <description>
        <en>Pointer to datastructure of current election.</en>
      </description>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>MaxCandidates</name>
      <description>
        <en>max nbr of candidates ( effectives + suppliants) for current election</en>
      </description>
    </parameter>
  </parameters>
  <comment>The display mode is either 14 candidates per column ( MODE14) or 22
  candidates per column ( MODE22). For the 2003 elections the display mode is
  triggered by the maximum number of suppliants for an election since the
  suppliants must always appear in a single separate column. This function is
  also used in the preview app.</comment>
  <author>cds</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
DISP_ELECT_MODE Calculate_Mode (
   ElectionList   ThisElection
   )
// ---------------------------------------------------------------------------
{
      DISP_ELECT_MODE iMode = UNDEFINED_ELECT_MODE;

   if ( SENAT == ThisElection->Ind)
   {
      // 20050110 avn - modified for elections after 2004
      iMode = MODE22;
   }
   else
   {
      // other elections as usual
      if ( ThisElection->MaxSuppliants)
      {
         // can we fit the suppliants + one header in a MODE14 column
         if ( ThisElection->MaxSuppliants <= 13)
         {
            if ( ThisElection->MaxEffectives <= 28)
            {
               // two effectives columns + one suppliants column maximum
               iMode = MODE14;
            }
            // with more than 28 effectives the mode remains undefined
         }
         // can we fit the suppliants + one header in a MODE22 column
         else if ( ThisElection->MaxSuppliants <= 21)
         {
            if ( ThisElection->MaxEffectives <= 66)
            {
               // three effectives columns + one suppliants column maximum
               iMode = MODE22;
            }
            else
            {
               // BXL councel needs 23 names per column
               iMode = MODE23;
            }
         }
         // with more than 21 suppliants the mode remains undefined
      }
      else
      {
         // elections without suppliants
         if ( ThisElection->MaxEffectives <= 42)
         {
            // three effectives columns maximum ( 42 = 3*14)
            iMode = MODE14;
         }
         else
         {
            // four effectives columns maximum
            iMode = MODE22;
         }
      }

   }

#if _DEBUGMODE
   sprintf( gcDcio, "Calculate_Mode E[%d] MaxEff[%d] MaxSupl[%d] >Mode[%d]",
      ThisElection->Ind, ThisElection->MaxEffectives,
      ThisElection->MaxSuppliants, iMode); dcio();
#endif // _DEBUGMODE

   return iMode;
}
//</code></function>


/*<function id="Set_Candidates_Display_Mode">
<heading>
  <description>
    <en>Defines the Display_Mode : Number of Columns, Number of lines
    for the display of the candidates of a selected party.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
    </parameter>
    <parameter type="arg">
      <datatype>struct ElectionMain*</datatype><name>Root</name>
    </parameter>
    <parameter type="arg">
      <datatype>struct Select_Party*</datatype><name>Parties</name>
    </parameter>
    <parameter type="arg">
      <datatype>struct Select_Candidate*</datatype><name>Candidates</name>
    </parameter>
    <parameter type="arg">
      <datatype>int</datatype><name>Nbr_Election</name>
    </parameter>
  </parameters>
  <comment>
    // if mode is 14 so 3 column
    // exception -> European Election !
    //              * Dutch & French : 3 colums with the posibility of
    //                                 Title + 14 Candidates in 1 column
    //                        (default:Title + 13 Candidates in 1 column)
    //              * German         : 1 column (Max = 1/3)
    // if mode is 22 so 4 column
  </comment>
  <author>cds</author>
</heading>
<code>*/
#pragma argsused
// ---------------------------------------------------------------------------
void Set_Candidates_Display_Mode (
   struct ElectionMain*       Root,
   struct Select_Party*       Parties,
   struct Select_Candidate*   Candidates,
   int                        Nbr_Election
   )
// ---------------------------------------------------------------------------
{
      DISP_PARTY_MODE iMode = UNDEFINED_PARTY_MODE;

   if ( MODE14 == Parties->ElectionMode)
   {
      if ( Candidates->NbrOfSuppliants)
      {
         if ( Candidates->NbrOfEffectives <= 14 )
         {
            // one column for effectives, one for supliants
            iMode = MODE14_2;
         }
         else
         {
            // two columns for effectives, one for supliants
            iMode = MODE14_3;
         }
      }
      else
      {
         // elections without suppliants
         if ( Candidates->NbrOfEffectives <= 14)
         {
            iMode = MODE14_1;
         }
         else
         if ( Candidates->NbrOfEffectives <= 28)
         {
            iMode = MODE14_2;
         }
         else
         {
            iMode = MODE14_3;
         }
      }
   }
   else if ( MODE22 == Parties->ElectionMode)
   {
      if ( Candidates->NbrOfSuppliants)
      {
         if ( Candidates->NbrOfEffectives <= 22)
         {
            iMode = MODE22_2;
         }
         else if ( Candidates->NbrOfEffectives <= 44)
         {
            iMode = MODE22_3;
         }
         else if ( Candidates->NbrOfEffectives <= 66)
         {
            iMode = MODE22_4;
         }
      }
      else
      {
         // elections without suppliants
         if ( Candidates->NbrOfEffectives <= 22)
         {
           iMode = MODE22_1;
         }
         else if ( Candidates->NbrOfEffectives <= 44)
         {
            iMode = MODE22_2;
         }
         else if ( Candidates->NbrOfEffectives <= 66)
         {
            iMode = MODE22_3;
         }
      }
   }
   else if ( MODE23 == Parties->ElectionMode)
   {
      if ( Candidates->NbrOfSuppliants)
      {
         if ( Candidates->NbrOfEffectives <= 23)
         {
            iMode = MODE23_2;
         }
         else if ( Candidates->NbrOfEffectives <= 46)
         {
            iMode = MODE23_3;
         }
         else if ( Candidates->NbrOfEffectives <= 69)
         {
            iMode = MODE23_4;
         }
         else
         {
            iMode = MODE23_4;
         }
      }
      else
      {
         // elections without suppliants
         if ( Candidates->NbrOfEffectives <= 23)
         {
           iMode = MODE23_1;
         }
         else if ( Candidates->NbrOfEffectives <= 46)
         {
            iMode = MODE23_2;
         }
         else if ( Candidates->NbrOfEffectives <= 69)
         {
            iMode = MODE23_3;
         }
         else
         {
            iMode = MODE23_4;
         }
      }
   }

#if _DEBUGMODE
   sprintf( gcDcio, "Dispmode() Emode[%d] C.eff[%d] C.sup[%d] >>Pmode[%d]",
      Parties->ElectionMode, Candidates->NbrOfEffectives,
      Candidates->NbrOfSuppliants, iMode); dcio();
#endif // _DEBUGMODE

   Candidates->PartyMode = iMode;
   return;
}
//</code></function>


/*<function id="Display_Candidates">
<heading>
  <description><en>
    Draws and displays the complete screen with the candidates
    of a selected party</en></description>
    <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>struct Select_Candidate*</datatype><name>Candidates</name>
      <description><en>
        points to the Select_Candidate structure,
        it contains all basic elements for the display
        of the candidates</en></description></parameter>
  </parameters>
  <comment>
    uses all the above mentionned .h files</comment>
  <author>rvh</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
void Display_Candidates (
   struct Select_Candidate* Candidates
   )
// ---------------------------------------------------------------------------
{
   int   i;

   if ( Candidates->PartyMode >= MODE22_1)
      Set_Mode( VGA_MODE, CARACT8x8);
   else
      Set_Mode( VGA_MODE, CARACT14x8);

   Screen_White();

   Display_Header( Candidates->ElectionInd,
      Candidates->CollegeInd,
      Candidates->PartyInd,
      Candidates->PartyName,
      Candidates->PartyLogo,
      Candidates->ElectionName,
      Candidates->PartyMode,
      Candidates->Party_Vote);

   if ( !Allready_Voted)
      Display_Footer_New_Party( Candidates->PartyMode);


   for ( i = 0; i < Candidates->NbrOfEffectives; i++)
   {
      Display_One_Candidate( Candidates->Effective[i].DisplayBox,
         Candidates->Effective[i].Ind,
         Candidates->Effective[i].Name1,
         Candidates->PartyMode,
         Candidates->Effective[i].Flag);
   }

   if ( Candidates->NbrOfSuppliants)
      Display_Title_Suppliant( Candidates->TitleSuppliant,
         Candidates->PartyMode);


   for ( i = 0; i < Candidates->NbrOfSuppliants; i++)
   {
      Display_One_Candidate( Candidates->Suppliant[i].DisplayBox,
         Candidates->Suppliant[i].Ind,
         Candidates->Suppliant[i].Name1,
         Candidates->PartyMode,
         Candidates->Suppliant[i].Flag);
   }

   return;
}
//</code></function>


/*<function id="Update_Candidates">
<heading>
  <description><en>
    Resets the candidates display to its default:
    no headvote, no candidate selected, new-party button.</en></description>
    <parameters>
    <parameter type="return">
      <datatype>void</datatype></parameter>
    <parameter type="arg">
      <datatype>struct Select_Candidate*</datatype><name>Candidates</name>
      <description><en>
        points to the Select_Candidate structure,
        it contains all basic elements for the display
        of the candidates</en></description></parameter>
  </parameters>
  <author>rvh</author>
</heading>
<code>*/
#ifndef _PREVIEW
// ---------------------------------------------------------------------------
void Update_Candidates (
   struct Select_Candidate* Candidates
   )
// ---------------------------------------------------------------------------
{
      int   i;

   Update_Head_Vote( Candidates->PartyMode, Candidates->Party_Vote);

   Display_Footer_New_Party( Candidates->PartyMode);

   for ( i = 0; i < Candidates->NbrOfEffectives; i++)
   {
      // 20040309 avn - update's background when voted
      Display_One_Candidate( Candidates->Effective[i].DisplayBox,
         Candidates->Effective[i].Ind,
         Candidates->Effective[i].Name1,
         Candidates->PartyMode, Candidates->Effective[i].Flag);
   }

   for ( i = 0; i < Candidates->NbrOfSuppliants; i++)
   {
      // 20040309 avn - update's background when voted
      Display_One_Candidate( Candidates->Suppliant[i].DisplayBox,
         Candidates->Suppliant[i].Ind,
         Candidates->Suppliant[i].Name1,
         Candidates->PartyMode, Candidates->Suppliant[i].Flag);
   }

   return;
}
#endif
//</code></function>


//=============================================================== EOF BODY ===
//</body>
//</source>
//<!-- eof -->

