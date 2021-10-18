///////////////////////////////////////////////////////////
//: Filename    : is-kbd-rkt-data.h
//: Date        : 2021-10-18
//: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version     : 1.0
//: Description : Output data from LKM.
//
// #ifndef
//
#ifndef __IS_KEYBOARD_RKT_DATA_H__
#define __IS_KEYBOARD_RKT_DATA_H__

//
// #include
//
#include "is-kbd-rkt.h"

//
// Main Transfer Data
//
typedef struct IS_KEYBOARD_RKT_DATA {
	unsigned short		cbSize;						// Size of Structure.
	unsigned int		dwRootComplexBaseAddress;	// Root Complex Base Address Register
	unsigned int		dwApicBaseAddress;			// Apic Base Address
	unsigned int		dwIoApicBaseAddress;		// IO Apic Base Address
	unsigned long long 	qwIOTRn[IOTRn_COUNT];					// IOTR0 Chipset Configuration Register
	unsigned long long 	qwIOAPIC_REDTBL[IO_APIC_IRQ_COUNT];		// IOAPIC IRQ0
	int					ntStatusCode;							// Driver NTSTATUS Error Code
	char				szErrorMessage[MAX_STRING_BUFFER_SIZE];	// Driver ERROR MESSAGE
} IS_KEYBOARD_RKT_DATA, *PIS_KEYBOARD_RKT_DATA;

//
// #endif
//
#endif
