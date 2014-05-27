#include <conio.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

#include "..\gen\genlpn.h"
#include "..\gen\genlight.h"
#include "..\Diagnost\General.h"

//====================== G L O B A L   V A R I A B L E S ======================

typeLPN type_lpn = UNKNOWN;

//======================= L O C A L   V A R I A B L E S =======================

unsigned short isaAddress = 0;
unsigned short baseAddress[]= {
	0x210,
	0x230,
	0x310,
	0x360,
	0x300,
	0x270,
	0x330,
	0x370,
	0x380,
	0x390,
	0
};

// this structure defines the interrupts to be scanned
struct _iScan
{
	unsigned char irq;
	unsigned char pxlIrq;       // PXL-395 IRQ select
	unsigned char picIrq;       // corresponding PIC IRQ bit
};

// place these in the order for interrupts to be scanned
struct _iScan iScan[]=
{
	10,6, 0x04,
	0,0,0,           // stop here, this can be moved.
	3, 2, 0x08,
	4, 3, 0x10,
	5, 4, 0x20,
	7, 5, 0x80,
	11,7, 0x08,
	12,8, 0x10,
	15,16,0x80,
	0,0,0
};

// forward declarations
typeLPN CheckInt( unsigned char irq,
				  unsigned char pxlIrq,
				  unsigned char picIrq);

//============================ F U N C T I O N S ==============================

BOOLEAN lpnDriverLoaded( void)
{
	return( pxl_loaded()
			? TRUE
			: FALSE);
}

//=============================================================================

typeLPN interfacePXL( void)
{
	int           i,
				  scan;
	unsigned char byte;

	if( type_lpn != UNKNOWN)
		return( type_lpn);

	type_lpn   = NO_PXL;
	isaAddress = 0;

	if( pciAvail())
		if( found595())
			type_lpn = PXL595;

	if( type_lpn == NO_PXL)
	{
		i = -1;
		while( baseAddress[++i])
		{
			if( inp( baseAddress[i]) & 0x10)          // b4 should be low
				continue;

			inp(baseAddress[i]+4);                    // clear video latch
			if( !( inp( baseAddress[i]) & 0x88))      // b3 and b7 should be high (inactive)
				continue;

			if( !lpnDriverLoaded())
			{
				inp(baseAddress[i]+5);                // set video latch
				if( inp( baseAddress[i]) & 0x88)      // b3 and b7 should be low (active)
				{
					// could be a PXL-385 without a lightpen
					if( inp( baseAddress[i]) & 0x20)  // pen missing = low
						continue;
				}
			}

			byte = inp( baseAddress[i]);
			if( ( byte & 0x40) ^ (( byte & 0x01)<<6)) // b0 and b6 should be equal
				continue;

			// board found
			break;
		}

		if( baseAddress[i])
		{
			type_lpn   = PXL385_395;
			isaAddress = baseAddress[i];

			// skip the next test if pxl-bios is loaded
			if( !lpnDriverLoaded())
			{
				// find an interrupt associated with a lightpen
				// to determine the board type
				scan = -1;
				while( iScan[++scan].irq)
				{
					type_lpn = CheckInt( iScan[scan].irq,
										 iScan[scan].pxlIrq,
										 iScan[scan].picIrq);
					if( type_lpn != PXL385_395)
						break;
				}
				if( !iScan[scan].irq)
					type_lpn = PXL385_395;
			}
		}
	}
	return( type_lpn);
}

//=============================================================================

typeLPN CheckInt( unsigned char irq,
				  unsigned char pxlIrq,
				  unsigned char picIrq)
{
	unsigned char picBase;
	unsigned char mask;
	unsigned char IR, x1, x2;

	// which PIC chip to address?
	if (irq > 7)
		picBase = 0xA0;
	else
		picBase = 0x20;

	_asm {CLI}
	// note: printing seems to enable interrupts, don't print

	mask = inp( picBase + 1);
	outp( isaAddress, pxlIrq);		// select IRQ

	// enable access to IR register
	outp( picBase + 1, mask);
	outp( picBase, 0x40);
	outp( picBase, 0x0A);

	// check for interrupt activity
	inp( isaAddress + 4);			// clr video
	inp( isaAddress + 7);			// enable int, clr v-sync interrupt,
									// should pulse int line low
	inp( isaAddress + 5);			// activate int

	IR = inp( picBase);				// read IR register

	// make sure interrupt is not active
	// note
	// ---- PXL-385 without a pen won't set the interrupt, could wait for
	//      vsync, but what if no sync cable ?
	//      If we knew for a PXL-385 was attached to this IRQ, then this
	//      test alone would indicate a PXL-385 present without a pen.
	if( !( IR & picIrq))
	{
		_asm {STI}

		if( !( inp( isaAddress) & 0x20)) // pen missing ? (low)
			return( PXL385);             // yes

		//puts("IRQ not responding (expected 0)");
		return( PXL385_395);
	}

	inp( isaAddress + 4);			// clr video
	inp( isaAddress + 7);	   		// enable int, clr v-sync intterupt,
									// should pulse int line low

	IR = inp( picBase);				// read IR register

	// make sure interrupt is NOT ACTIVE
	if( IR & picIrq)
	{
		_asm {STI}
		//puts("IRQ not responding (expected 0)");
		return( PXL385_395);
	}

	// now check for inactivity
	outp( isaAddress, 0);			// disable PXL-395 IRQ,
									// a PXL-385 will ignore this

	inp( isaAddress + 7);			// enable int
	inp( isaAddress + 5);			// test int, set video

	IR = inp( picBase);				// read IR register

	if( IR & picIrq)				// interrupt ?
	{
		// yes, now check for interrupt low
		inp( isaAddress + 4);		// clr video
		inp( isaAddress + 7);	   	// enable int, clr v-sync intterupt,
									// should pulse int line low

		// now check for interrupt low
		IR = inp( picBase);			// read IR register

		if( !( IR & picIrq))		// interrupt low?
		{
			 _asm {STI}
			return( PXL385);
		}
	}
	 _asm {STI}
	return( PXL395);
}

//=============================================================================
