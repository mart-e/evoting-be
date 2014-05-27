/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="select.cpp"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
  <workFile>$Workfile:   SELECT.CPP  $</workFile>
  <revision>$Revision: 1.2 $</revision>
  <workFileTimeStamp>$Modtime:   Oct 27 2004 15:26:36  $</workFileTimeStamp>
  <archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Mav/SELECT.cpp-arc  $</archive>
  <archiveTimeStamp>$Date: 2007/02/05 10:42:41 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>select.cpp</fileName>
  <creationDate></creationDate>
  <rights>Steria Benelux</rights>
  <author></author>
  <description>
    <fr>Selection écrans candidats</fr>
    <nl>Selectie kandidaatschermen</nl></description>
  <history></history>
  <comment></comment>
</comments>*/

//============================================================= BOF HEADER ===
//<header>

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#include "..\mav\display.h"    // makes funcions of Display.cpp available for others
#include "..\mav\drawings.h"   // external functions of display.cpp
#include "..\mav\select.h"     // makes funcions of InputCan.cpp available for others

#include "..\mav\mavvideo.h"   // for Screen_White
#include "..\mav\mavasm.h"     // for VGA_MODE
#include "..\mav\mavlight.h"   // for Get_Pen
#include "..\mav\mavmes.h"  // for Message display
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines/>

//--------------------------------------------------------------- TYPEDEFS ---

//------------------------------------------- PROTOTYPES PRIVATE FUNCTIONS ---
//<prototypes>
static BOOL Are_Candidates_Selected (
   struct Select_Candidate* Candidates,
   int PenCol,
   int PenRow
   );
static void Clear_Votes (
   struct Select_Candidate* Candidates
   );
static BOOL Validate_Candidates (
   struct Select_Candidate* Candidates,
   int CandidateType
   );
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

/*<function id="Input_Candidates">
<heading>
  <description>
    <en>Drives the pen clicks on the candidate screen.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = a successfull selection was made,
      FALSE = the user made a wrong vote.</en></description>
    </parameter>
    <parameter type="arg">
      <datatype>struct Select_Candidate*</datatype><name>Candidates</name>
      <description><en>the structure with the election descript</en></description>
    </parameter>
  </parameters>
  <comment></comment>
  <author></author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
BOOL Input_Candidates (
   struct Select_Candidate* Candidates
   )
// ---------------------------------------------------------------------------
{
   int   PenCol,PenRow;  // X (Colomn) and Y (Row) coordinate of the pen
   int   CandidateType;  // 0 for effectives, 1 for suppliant,
                            // needed for Validate_Candidates

   for( ; ; )
   {
      Get_Pen( &PenCol, &PenRow);

      if ( Is_New_Party_Selected( Candidates->PartyMode, PenCol, PenRow))
      {
         // user has not voted, requests another party
         return FALSE;
      }
      else if ( Is_Confirm_Selected( Candidates->PartyMode, PenCol, PenRow))
      {
         // user has voted and confirms
         return TRUE;
      }
      else if ( Is_Cancel_Selected( Candidates->PartyMode, PenCol, PenRow))
      {
         // user has voted but cancels his votes
         Clear_Votes( Candidates);
         Display_Candidates( Candidates);
      }
      else if ( Is_Head_Vote_Selected( Candidates->PartyMode, PenCol, PenRow))
      {
         Candidates->Party_Vote = TRUE;
         Update_Head_Vote( Candidates->PartyMode, TRUE);
      }
      else
      {
         Are_Candidates_Selected( Candidates, PenCol, PenRow);
      }

      // display buttons in the footer
      if ( Candidates->Party_Vote ||
           Candidates->EffectivesSelected ||
           Candidates->SuppliantsSelected
         )
      {
         // votes made, cancel or confirm allowed
         Display_Footer_Cancel_Confirm( Candidates->PartyMode);
      }
      else
      {
         // no votes made, no confirm button allowed
         Display_Footer_New_Party( Candidates->PartyMode);
      }
   } // eofor
}
//</code></function>

//------------------------------------------------------ PRIVATE FUNCTIONS ---

/*<function id="Are_Candidates_Selected">
<heading>
  <description>
    <en>Looks if the pen position is a coordinate of a candidate.
    Checks if the selection is valid.
    Updates the graphic state of the name box if necessary.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>returns TRUE if a valid selection is made,
      returns FALSE if a non valid selection is made,
      returns FALSE if the selection was already made.</en></description>
    </parameter>
  </parameters>
  <comment>This function calls Validate_Candidates that puts a warning message
  on the screen.
  From 2007 onwards the name box can be toggled.</comment>
  <author>avn</author>
</heading>
<code>*/
// ---------------------------------------------------------------------------
static BOOL Are_Candidates_Selected (
   struct Select_Candidate*   Candidates,
   int                        PenCol,
   int                        PenRow
   )
// ---------------------------------------------------------------------------
{
   int     i;
   int     CandidateType;  // 0 for effectives, 1 for suppliant
                              // needed for Validate_Candidates
   int     Toggle;

   CandidateType = 0;
   for ( i = 0; i < Candidates->NbrOfEffectives; i++)
   {
      if ( Is_Candidate_Selected( Candidates->Effective[i].DisplayBox,
                                  Candidates->PartyMode, PenCol, PenRow)
         )
      {
         if ( Validate_Candidates( Candidates, CandidateType))
         {
            // pen clicked in the name box and choice is valid, toggle selection
            Toggle = ( Candidates->Effective[i].Flag ? FALSE : TRUE);
            Display_One_Candidate( Candidates->Effective[i].DisplayBox,
               Candidates->Effective[i].Ind,
               Candidates->Effective[i].Name1,
               Candidates->PartyMode, Toggle);
            Candidates->Effective[i].Flag = Toggle;
            if ( Toggle)
            {
               Candidates->EffectivesSelected++;
            }
            else
            {
               Candidates->EffectivesSelected--;
            }
            return TRUE;
         }
         else
         {
            // invalid combination voted
            Clear_Votes( Candidates);
            Display_Candidates( Candidates);
            return FALSE;
         }
      };
   } // eofor loop Effectives

   CandidateType = 1;
   for ( i = 0; i < Candidates->NbrOfSuppliants; i++)
   {
      if ( Is_Candidate_Selected( Candidates->Suppliant[i].DisplayBox,
                                  Candidates->PartyMode, PenCol, PenRow)
         )
      {
         if ( Validate_Candidates( Candidates, CandidateType))
         {
            // pen clicked in the name box and choice is valid, toggle selection
            Toggle = ( Candidates->Suppliant[i].Flag ? FALSE : TRUE);
            Display_One_Candidate( Candidates->Suppliant[i].DisplayBox,
               Candidates->Suppliant[i].Ind,
               Candidates->Suppliant[i].Name1,
               Candidates->PartyMode, Toggle);
            Candidates->Suppliant[i].Flag = Toggle;
            if ( Toggle)
            {
               Candidates->SuppliantsSelected++;
            }
            else
            {
               Candidates->SuppliantsSelected--;
            }
            return TRUE;
         }
         else
         {
            // invalid combination voted
            Clear_Votes( Candidates);
            Display_Candidates( Candidates);
            return FALSE;
         }
      };
   } // eofor loop Suppliants

   // no candidate selected
   return FALSE;
}
//</code></function>

/*<function id="Clear_Votes">
<heading>
  <description>
    <en>reset votes for the current party of the current election.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>void</datatype>
    </parameter>
  </parameters>
  <comment>This function is called when the user
  -  makes a mistake
  -  cancels his election choice.</comment>
  <author></author>
</heading>
<code>*/
//-----------------------------------------------------------------------------
static void Clear_Votes (
   struct Select_Candidate* Candidates
   )
//-----------------------------------------------------------------------------
{
   Candidates->Party_Vote = FALSE;
   Candidates->EffectivesSelected = 0;
   Candidates->SuppliantsSelected = 0;

   for( int le=0; le < Candidates->NbrOfEffectives; ++le)
         Candidates->Effective [le].Flag = FALSE;

   for( int ls=0; ls < Candidates->NbrOfSuppliants; ++ls)
         Candidates->Suppliant[ls].Flag = FALSE;
}
//</code></function>

/*<function id="Validate_Candidates">
<heading>
  <description>
    <en>check if the vote on the candidate is valid, according to the election
     type.</en>
  </description>
  <parameters>
    <parameter type="return">
      <datatype>BOOL</datatype>
      <description><en>TRUE  = valid choice, FALSE = invalid choice.</en></description>
    </parameter>
  </parameters>
  <comment>When an invalid choice is made, an error message is displayed;
that message is displayed during 5 seconds and can be shortened
by pushing the lightpen on the screen. The function then
returns with the FALSE result which will exit the candidate
selection process to force the loop to redisplay the list and
reenter the selection process.</comment>
  <author></author>
</heading>
<code>*/
//-----------------------------------------------------------------------------
static BOOL Validate_Candidates (
   struct Select_Candidate* Candidates,
   int CandidateType
   )
//-----------------------------------------------------------------------------
{

   if ( Candidates->ElectionType == 0)
   {
      // only one vote allowed per candidate type
      if ( CandidateType == 0)  // effectif
      {
         if ( Candidates->EffectivesSelected + 1 > 1)
         {
            Display_Message( M_TOO_MANY_EFF, NO, 5, YES, 1,1,1,1, 80,60,560,410);
            return FALSE;
         }
      }
      if ( CandidateType == 1)  // suppliant
      {
         if ( Candidates->SuppliantsSelected + 1 > 1)
         {
            Display_Message( M_TOO_MANY_SUP, NO, 5, YES, 1,1,1,1, 80,60,560,410);
            return FALSE;
         }
      }
   }

   // voted combination is ok
   return TRUE;
}
//</code></function>

//=============================================================== EOF BODY ===
//</body>
//</source>