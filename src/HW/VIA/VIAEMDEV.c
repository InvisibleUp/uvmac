/*
	VIAEMDEV.c

	Copyright (C) 2020 InvisibleUp

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	Versatile Interface Adapter EMulated DEVice

	Emulates the Synertek SY6522 VIA in every system that uvMac targets.
	This code rewritten for target-independence and non-enum based config
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "VIAEMDEV.h"

/* Global state */
VIA_State_t VIA_State[VIA_MAXNUM]; 

// Hardware reset
bool VIA_Zap(void) {
	memset(VIA_State, 0, sizeof(VIA_State));
	return true;
}
// Software reset
void VIA_Reset(void) {
	VIA_Zap();
}

// Raise an interrupt by irq number
void VIA_RaiseInterrupt(uint8_t id, uint8_t irq)
{
	assert (id == 0 || id == 1);
	assert (irq <= 6);
	
	// Set interrupt flag
	VIA_State[id].vIFR |= (1 << irq) | (1 << 7);
	
	// Call interrupt handler, if required
	if (VIA_State[id].vISR[irq] != NULL) {
		VIA_State[id].vISR[irq]();
	}
}

// Register a VIA interrupt service routine
void VIA_RegisterISR(uint8_t id, uint8_t irq, VIA_ISR_t isr)
{
	VIA_State[id].vISR[irq] = isr;
}

// Tick VIA timer 1 (and set PB7 as needed)
static void VIA_TickTimer1(uint8_t id)
{
	VIA_State_t *via  = &VIA_State[id];
	uint8_t T1_MODE   = (via->vACR & 0b11000000) >> 6;
	bool T1_PULSE     = (T1_MODE & 0b01) != 0;
	bool T1_SNDTOGGLE = (T1_MODE & 0b10) != 0;
	
	if (via->vT1C == 0) {
		if (T1_PULSE)     { via->vT1C = via->vT1L; }
		if (T1_SNDTOGGLE) { via->vBufB ^= (1 << 7);  }
		VIA_RaiseInterrupt(id, 6); // always IRQ6
	} else {
		via->vT1C -= 1;
	}
}

// Tick VIA timer 2. Raise interrupt if required
static void VIA_TickTimer2(uint8_t id, bool forceTick)
{
	VIA_State_t *via  = &VIA_State[id];
	// Check we're not in the mode where T2 counts PB6 falling edges
	bool T2_ISCOUNTER = VIA_ReadBit(id, rACR, 5);
	if (T2_ISCOUNTER && !forceTick) { return; }
	
	// Do a normal tick
	if (via->vT2C != 0) { via->vT2C -= 1; }
	else                { VIA_RaiseInterrupt(id, 5); } // always IRQ5
}

// Tick all timers by one step (call every 1.2766 us)
void VIA_TickTimers()
{
	for (uint8_t id = 0; id < 2; id += 1) {
		VIA_TickTimer1(id);
		VIA_TickTimer2(id, false);
	}
}


// Write to a register
void VIA_Write(uint8_t id, VIA_Register_t reg, uint8_t data)
{
	assert(id == 0 || id == 1);
	assert(reg < rINVALID);
	
	// store PB6's state
	bool PB6_old = VIA_ReadBit(id, rIRB, 6);
	VIA_State_t *via = &VIA_State[id];
	
	switch(reg) {
	case rIRB:  via->vBufB = data; break;
	case rIRA:
	case rORA:  via->vBufA = data; break;
	case rDDRB: via->vDirB = data; break;
	case rDDRA: via->vDirA = data; break;
	case rT1CL:
	case rT1LL: via->vT1L &= 0xFF00 | data; break;
	case rT1CH: via->vT1L &= 0x00FF | (data << 8);
	            via->vT1C = via->vT1L;
	            via->vIFR &= 0b10111111;
	            break;
	case rT1LH: via->vT1L &= 0x00FF | (data << 8); break;
	case rT2CL: via->vT2L &= 0xFF00 | data; break;
	case rT2CH: via->vT2L &= 0x00FF | (data << 8);
	            via->vT2C = via->vT2L;
	            via->vIFR &= 0b10111111;
	            break;
	case rSR:   via->vSR = data; break;
	case rACR:  via->vACR = data; break;
	case rPCR:  via->vPCR = data; break;
	case rIFR:  via->vIFR = data; break;
	case rIER:
		switch(data & 0b10000000) {
		case 0b00000000:  // clear
			via->vIER &= ~(data & 0b01111111);
			break;
		case 0b10000000:  // set
			via->vIER |=  (data & 0b01111111);
			break;
		}
		break;
	default: assert(true);
	}
	
	// Decrement timer 2 if in counter mode and PB6 hits falling edge
	bool PB6_new = VIA_ReadBit(id, rIRB, 6);
	bool T2_ISCOUNTER = VIA_ReadBit(id, rACR, 5);
	if (PB6_old == true && PB6_new == false && T2_ISCOUNTER) {
		VIA_TickTimer2(id, true);
	}
}

// Read to a register
uint8_t VIA_Read(uint8_t id, VIA_Register_t reg)
{
	assert(id == 0 || id == 1);
	assert(reg < rINVALID);
	VIA_State_t *via = &VIA_State[id];
	
	switch(reg) {
	// not sure if reading *all* of vBufA or vBufB is correct,
	// but it shouldn't matter. 
	case rIRB:  return via->vBufB;
	case rIRA:
	case rORA:  return via->vBufA;
	case rDDRB: return via->vDirB;
	case rDDRA: return via->vDirA;
	case rT1CL: via->vIFR   &= 0b10111111;
	            return (via->vT1C & 0xFF00);
	case rT1CH: return (via->vT1C & 0x00FF) >> 8;
	case rT1LL: return (via->vT1L & 0xFF00);
	case rT1LH: return (via->vT1L & 0x00FF) >> 8;
	case rT2CL: via->vIFR   &= 0b11011111;
	            return (via->vT2C & 0xFF00);
	case rT2CH: return (via->vT2C & 0x00FF) >> 8;
	case rSR:   return via->vSR;
	case rACR:  return via->vACR;
	case rPCR:  return via->vPCR;
	case rIFR:  return via->vIFR;
	case rIER:  return via->vIER |= 0b10000000;
	default:    assert(true); return 0;
	}
}

// Read a single bit
bool VIA_ReadBit(uint8_t id, VIA_Register_t reg, uint8_t bit)
{
	return ((VIA_Read(id, reg) >> bit) & 0b1);
}

// Write a single bit
void VIA_WriteBit(uint8_t id, VIA_Register_t reg, uint8_t bit, bool set)
{
	uint8_t data = VIA_Read(id, reg);
	if (set) { data |=  (1 << bit); }
	else     { data &= ~(1 << bit); }
	VIA_Write(id, reg, data);
}

// Called by either end; store data in vSR.
// TODO: this probably shouldn't be instant.
void VIA_ShiftInData(uint8_t id, uint8_t v)
{
	assert(id == 0 | id == 1);
	VIA_State[id].vSR = v;
}

// Called by either end, get data out of vSR
// TODO: this probably shouldn't be instant.
uint8_t VIA_ShiftOutData(uint8_t id)
{
	return VIA_State[id].vSR;
}

