/*
	Versatile Interface Adapter EMulated DEVice

	Emulates the Synertek SY6522 VIA in every system that uvMac targets.
	This code rewritten for target-independence and non-enum based config
*/

#pragma once
#include <stdint.h>
#include <stdbool.h>

/// TYPEDEFS /////////////////////////////////////////////////////////////////

/* Mac II has two VIAs, all others have one */
#define VIA_MAXNUM (2)

/* VIA interrupt handler signature */
typedef void (*VIA_ISR_t)(void);

/* Names from Guide to Macintosh Family Hardware, Second Edition, pg. 159 */
typedef struct {
	uint8_t   vBufA;    // Data Register A
	uint8_t   vBufB;    // Data Register B
	uint8_t   vDirA;    // Data Direction A (0 = in, 1 = out)
	uint8_t   vDirB;    // Data Direction B (0 = in, 1 = out)
	uint8_t   vPCR;     // Peripheal Control
	uint8_t   vACR;     // Auxiliary Control
	uint8_t   vIFR;     // Interrupt Flag
	uint8_t   vIER;     // Interrupt Enable
	uint8_t   vSR;      // Shift register
	uint16_t  vT1L;     // Timer 1 latch
	uint16_t  vT1C;     // Timer 1 counter
	uint16_t  vT2L;     // Timer 2 latch
	uint16_t  vT2C;     // Timer 2 counter
	VIA_ISR_t vISR[8];  // ISRs to automatically call when interrupt is raised
} VIA_State_t;

/* Names from SY6522 datasheet */
typedef enum {
	rIRB     = 0,  // Data Register B
	rIRA     = 1,  // Data Register A
	rDDRB    = 2,  // Data Direction B
	rDDRA    = 3,  // Data Direction A
	rT1CL    = 4,  // Timer 1 Counter (low)
	rT1CH    = 5,  // Timer 1 Counter (high)
	rT1LL    = 6,  // Timer 1 Latches (low)
	rT1LH    = 7,  // Timer 1 Latches (high)
	rT2CL    = 8,  // Timer 1 Counter (low)
	rT2CH    = 9,  // Timer 1 Latches (high)
	rSR      = 10, // Shift Register
	rACR     = 11, // Auxiliary Control
	rPCR     = 12, // Peripheal Control
	rIFR     = 13, // Interrupt Flag
	rIER     = 14, // Interrupt Enable
	rORA     = 15, // duplicate of IRA w/o handshake (unused)
	rINVALID = 16, // end of list
} VIA_Register_t;

/// PUBLIC FUNCTIONS /////////////////////////////////////////////////////////

// Hardware reset
bool VIA_Zap(void);
// Software reset
void VIA_Reset(void);

// Raise an interrupt by irq number, calling registered ISR if required
void VIA_RaiseInterrupt(uint8_t id, uint8_t irq);
// Register a VIA interrupt service routine
void VIA_RegisterISR(uint8_t id, uint8_t irq, VIA_ISR_t isr);

// Tick all timers by one step (call every 1.2766 us)
void VIA_TickTimers();

// Write to a register
void VIA_Write(uint8_t id, VIA_Register_t reg, uint8_t data);
// Read to a register
uint8_t VIA_Read(uint8_t id, VIA_Register_t reg);
// Read a single bit
bool VIA_ReadBit(uint8_t id, VIA_Register_t reg, uint8_t bit);
// Write a single bit
void VIA_WriteBit(uint8_t id, VIA_Register_t reg, uint8_t bit, bool set);

// NOTE: for these, raise the interrupt manually w/ VIA_RaiseInterrupt
void VIA_ShiftInData(uint8_t id, uint8_t v);
uint8_t VIA_ShiftOutData(uint8_t id);
