/*<?xml version="1.0" encoding="ISO-8859-1"?>
<source name="mavasm.h"><!-- use lower case for name -->
<pvcs>
  <!-- these tags are filled out by version control system during check in -->
  <revision>$Revision: 1.4 $</revision>
  <archiveTimeStamp>$Date: 2005/07/01 15:13:17 $</archiveTimeStamp>
  <pvcsUser>$Author: avanderhoudelingen $</pvcsUser>
</pvcs>
<comments>
  <fileName>mavasm.h</fileName>
  <creationDate>10/01/93</creationDate>
  <rights>Steria Benelux</rights>
  <author>B.S.S.</author>
  <description>
    <fr>Fichier d'entête</fr>
    <nl>Headerfile</nl></description>
  <history/>
  <comment/>
</comments>*/

//============================================================= BOF HEADER ===
//<header>
#ifndef _MAVASM                       // avoid mutiple inclusions of this file
   #define _MAVASM

//--------------------------------------------------------------- INCLUDES ---
//<includes>
#ifndef _PREVIEW
   #include "..\mav\lpasm.h"
#endif
//</includes>

//----------------------------------------------------------------------------
//                                     /!\ declarations do NOT allocate memory
//---------------------------------------------------------------- DEFINES ---
//<defines>
#define CGA_MODE    14
#define EGA_MODE    16
#define VGA_MODE    18

#define CARACT16x8   16
#define CARACT14x8   14
#define CARACT8x8    8
//</defines>

//-------------------------------------------- PROTOTYPES PUBLIC FUNCTIONS ---
//<prototypes>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
extern "C" {                                            // only needed for C++
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void videom (int mode, int karak);        // set videomode
                          // mode=  14 -> CGA
                          // mode=  16 -> EGA
                          // mode=  18 -> VGA
                          // karak= 16 -> 16x8
                          // karak= 14 -> 14x8
                          // karak=  8 ->  8x8

void achtersp(int rij1, int kol1, int rij2, int kol2, int kleur);
void bladvul(int kleur);    // fill screen in color kleur
                            // 99 = wit -> sneller dan 15
void lijnvert(int rij1, int kol1, int rij2, int kleur);  // vertikale lijn
void lijnhor (int rij1, int kol1, int kol2, int kleur);  // horizontale lijn

void kad(int rij1, int kol1, int rij2, int kol2, int kleur);
                              // draws box in color kleur

void zegsp(int rij, int kol, char *tekst, int kleur, int leng, int inv);
       // writes text at offset rij,kol
void knopsp(int rij, int kol1, int kol2, char *tekst, int leng, int kleur);
       // button with text with variable width

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}                                                            // end extern "C"
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//</prototypes>


#endif // #ifndef _MAVASM
//============================================================= EOF HEADER ===
/*</header>
</source>
<!-- eof -->*/