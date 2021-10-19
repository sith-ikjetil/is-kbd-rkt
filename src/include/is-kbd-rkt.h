////////////////////////////////////////////////////////
//: Filename    : is-kbd-rkt.h
//: Date        : 2021-10-18
//: Author      : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version     : 1.0
//: Description : is-kbd-rkt LKM header file
//
// #ifndef
//
#ifndef __IS_KBD_RKT_H__
#define __IS_KBD_RKT_H__

//
// #define
//
#define RCBA_MASK				0xFFFFC000
#define MAX_STRING_BUFFER_SIZE	0xFF
#define APIC_MASK				0xFFFFF000
#define IO_APIC_IRQ_COUNT		24
#define IOTRn_COUNT				4
#define DEVICE_NAME 	        "iskbdrkt"
#define CLASS_NAME 		        "iskbdrkt"
#define RCBA_CHUNK_SIZE         0x2000
#define APIC_CHUNK_SIZE         0x1000
#define IRQ_APIC_KEYBOARD       0x1
#define IO_APIC_BASE_ADDRESS    0xFEC00000
#define L_APIC_BASE_ADDRESS     0xFEE00000

//
// #endif
//
#endif
