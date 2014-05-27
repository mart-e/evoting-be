#ifndef _GENLPN                      // avoid mutiple inclusions of this file
   #define _GENLPN

#include "..\Diagnost\General.h"

//============================ T Y P E D E F S ================================

typedef enum {
	UNKNOWN,
	NO_PXL,
	PXL385_395,
	PXL385,
	PXL395,
	PXL595
} typeLPN;

//====================== G L O B A L   V A R I A B L E S ======================

extern typeLPN type_lpn;

//=========================== P R O T O T Y P E S =============================

extern BOOLEAN lpnDriverLoaded( void);
extern typeLPN interfacePXL( void);

#endif // _GENLPN