/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="gendata.h"><!-- use lower case for name -->
<pvcs><!-- these tags are filled out by pvcs during check in -->
<workFile>$Workfile:   GENDATA.H  $</workFile>
<revision>$Revision: 1.7 $</revision>
<workFileTimeStamp>$Modtime:   Jan 19 2005 16:10:40  $</workFileTimeStamp>
<archive>$Archive:   C:/PVCS VM/v6.8.00/Digivote/Archives/Digivote/Prg/Gen/GENDATA.H-arc  $</archive>
<archiveTimeStamp>$Date: 2008/10/21 12:38:50 $</archiveTimeStamp>
<pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>GENDATA.H</fileName>
  <creationDate>20/02/93</creationDate>
  <rights/>
  <author>B.S.S.</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Header file</nl></description>
  <history></history>
  <comment></comment>
</comments>*/
//============================================================= BOF HEADER ===
//<header>
#ifndef _GENDATA                     // avoid multiple inclusions of this file
   #define _GENDATA

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
enum CardUsage { BLANCO, VOTED, CANCELED=9 };   // possible usage flags

//</defines>

//------------------------------------------------------------- PROTOTYPES ---
//<prototypes>
void Two_Bytes_Int( int * Int, char TB [2]);
void Format_Blanco_Card( void);
void Format_Blanco_Votes( void);

RET_CODE Split_Card_Buffer (
   BYTE  Card_Buffer [],               // physical data on the card
   int*  Flag,                         // the card's usage flag
   BYTE* Data                          // the votes per election
   );

void Encrypt_Vote( BYTE* Vote_Data, BYTE* Vote_Encrypted, int Data_Length);
void Decrypt_Vote( BYTE* Vote_Encrypted, BYTE* Vote_Data, int Data_Length);
RET_CODE Verify_MAC_On_Vote( BYTE* Vote_Encrypted_With_MAC, int Data_Length);

#ifdef _APPLICATION_MAV
   void initCard( int iFlag, BYTE* PhysicalCardData);
   CARD_POSITION Write_Votes( int iFlag, BYTE* PhysicalCardData);
   void Init_Card_Buffer( void);
   void Buffer_Vote(ElectionList Election);
#endif

#ifdef _APPLICATION_MAV
   RET_CODE Build_Election( int ElectId);   //JRU: Build Election In Memory
   RET_CODE Destroy_Election( int ElectId); //JRU: Destroy Election From Memory
   void ClearVoteBuffers ();
#endif

RET_CODE Setup_Database( void);

#ifdef _APPLICATION_MAV
   void Init_Colleges( int ElectionInd);
   void Init_Parties( int ElectionInd, int CollegeInd);
   void Init_Candidates( int ElectionInd, int CollegeInd, int PartyInd);
#endif

#ifdef _APPLICATION_URN
   SessionList Find_Session( int SessionInd);
   RET_CODE Write_Totals_File( void);

   RET_CODE Copy_Totals_File();
   RET_CODE Remove_Totals_File();

   RET_CODE CheckVotesCounters ( void);
   void ResetVotesCounters ( void);

   void ResetPrng (
      void
      );
#endif

RET_CODE Count_Vote( OPCODE_COUNT Operation);

//rvh
void Get_Max_Candidates_Per_Election(void);

BOOL Skip_Election(
   int IndElection,
   int TypeElector
   );

void InitApplVariables( unsigned int combination_code );

void AllowSpecialElection(
   BOOL Permission
   );

void ErrorArc( char* fmt);
extern int DecompressFile( char* cpFileName);   // cfr. ar.cpp lib

//</prototypes>


//rvh
const int MAX_VOTE_BUF = 200;

struct cardinfo
{
  int Max_Candidates;                  // effectives + suppliants
  int Type_Election;
};

extern const long int MAX_CARDS_IN_BAC;

#endif                                              // end of #ifndef _GENDATA
//============================================================= EOF HEADER ===
//</header>
//</source>
//<!-- eof -->