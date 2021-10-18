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
#include "IsKbdRkt.h"

//
// Main Transfer Data
//
typedef struct IS_KEYBOARD_RKT_DATA {
	uint16_t		cbSize;						// Size of Structure.
	uint32_t		dwRootComplexBaseAddress;	// Root Complex Base Address Register
	uint32_t		dwApicBaseAddress;			// Apic Base Address
	uint32_t		dwIoApicBaseAddress;		// IO Apic Base Address
	uint64_t 		qwIOTRn[IOTRn_COUNT];					// IOTR0 Chipset Configuration Register
	uint64_t 		qwIOAPIC_REDTBL[IO_APIC_IRQ_COUNT];		// IOAPIC IRQ0
	int32_t			ntStatusCode;							// Driver NTSTATUS Error Code
	char			szErrorMessage[MAX_STRING_BUFFER_SIZE];	// Driver ERROR MESSAGE
} IS_KEYBOARD_RKT_DATA, *PIS_KEYBOARD_RKT_DATA;

//
// #endif
//
#endif
