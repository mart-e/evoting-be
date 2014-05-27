//=======================================================================

#ifndef SELECT_H
#define SELECT_H

#include "..\GEN\GenType.h";

BOOL Input_Candidates( struct Select_Candidate *Candidates );
// Returns TRUE when a confirmed choice is made (CONFIRMED)
//         FALSE when a new party has been choosen (NEW PARTY)
// This function asks input from the voter and marks the selected candidates.
// The voter also has to confirm/cancel his vote within this function.
// When the vote is canceled, the screen is updated automatically.
// This function validates a made choice.


#endif

