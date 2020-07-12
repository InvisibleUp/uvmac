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
#include "GLOBGLUE.h"

/* Global state */
VIA_State_t VIA_State[VIA_MAXNUM]; 

// Hardware reset
bool VIA_Zap(void) {
	memset(VIA_State, 0, sizeof(VIA_State));
	for (int i = 0; i < VIA_MAXNUM; i += 1) {
		VIA_State[i].vBufA = 0xFF;
		VIA_State[i].vBufB = 0xFF;
	}
	return true;
}
// Software reset
void VIA_Reset(void) {
	VIA_Zap();
}

// Raise an interrupt by irq number
void VIA_RaiseInterrupt(uint8_t id, uint8_t irq)
{
	assert (id < VIA_MAXNUM);
	assert (irq < 7);
	
	// Set interrupt flag
	VIA_State[id].vIFR |= (1 << irq) | (1 << 7);
	
	// Call interrupt handler, if required
	if (VIA_State[id].vISR[irq] != NULL) {
		VIA_State[id].vISR[irq]();
	}
	VIAorSCCinterruptChngNtfy();
}

// Lower an interrupt by irq number
void VIA_LowerInterrupt(uint8_t id, uint8_t irq)
{
	assert (id < VIA_MAXNUM);
	assert (irq < 7);
	
	// Set interrupt flag
	VIA_State[id].vIFR &= ~(1 << irq);
	
	// Call interrupt handler, if required
	if (VIA_State[id].vISR[irq] != NULL) {
		VIA_State[id].vISR[irq]();
	}
	VIAorSCCinterruptChngNtfy();
}

// Register a VIA interrupt service routine
void VIA_RegisterISR(uint8_t id, uint8_t irq, VIA_ISR_t isr)
{
	assert(irq < 7);
	assert(id < VIA_MAXNUM);
	VIA_State[id].vISR[irq] = isr;
}

// Register data state-change notification interrupts
void VIA_RegisterDataISR(uint8_t port, uint8_t id, uint8_t irq, VIA_ISR_t isr)
{
	assert(id < VIA_MAXNUM);
	assert(port == DataRegA || port == DataRegB);
	assert(irq < 8);
	
	if (port == DataRegA) {
		VIA_State[id].vISR_A[irq] = isr;
	} else {
		VIA_State[id].vISR_B[irq] = isr;
	}
}
/*
const int TEMPSKIP = 1;

// Tick VIA timer 1 (and set PB7 as needed)
static void VIA_TickTimer1(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via  = &VIA_State[id];
	uint8_t T1_MODE   = (via->vACR & 0b11000000) >> 6;
	bool T1_PULSE     = (T1_MODE & 0b01) != 0;
	bool T1_SNDTOGGLE = (T1_MODE & 0b10) != 0;
	
	if (via->vT1C == 0) {
		if (T1_PULSE)     { via->vT1C = via->vT1L; }
		if (T1_SNDTOGGLE) {
			bool snd_old = VIA_ReadBit(id, rIRB, 7);
			VIA_WriteBit(id, rIRB, 7, !snd_old, true);
		}
		VIA_RaiseInterrupt(id, 6); // always IRQ6
	} else {
		if (via->vT1C < TEMPSKIP) { via->vT1C = 0; }
		else { via->vT1C -= TEMPSKIP; }
		//via->vT1C -= 1;
	}
}

// Tick VIA timer 2. Raise interrupt if required
static void VIA_TickTimer2(uint8_t id, bool forceTick)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via  = &VIA_State[id];
	// Check we're not in the mode where T2 counts PB6 falling edges
	bool T2_ISCOUNTER = VIA_ReadBit(id, rACR, 5);
	if (T2_ISCOUNTER && !forceTick) { return; }
	
	// Do a normal tick
	if (via->vT2C != 0) { 
		if (via->vT2C < TEMPSKIP) {
			via->vT2C = 0;
		} else {
			via->vT2C -= 1;
		}
	}
	else { 
		VIA_RaiseInterrupt(id, 5); // always IRQ5
	}
}

// Tick all timers by one step (call every 1.2766 us)
void VIA_TickTimers()
{
	for (uint8_t id = 0; id < VIA_MAXNUM; id += 1) {
		VIA_TickTimer1(id);
		VIA_TickTimer2(id, false);
	}
}*/

// Do fancy shift register stuff
void VIA_TickShiftRegister(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via  = &VIA_State[id];
	
	switch ((via->vACR & 0x1C) >> 2) {
	case 3 : /* Shifting In */
		break;
	case 6 : /* shift out under o2 clock */
		VIA_LowerInterrupt(id, 3);
		VIA_RaiseInterrupt(id, 2);
		break;
	case 7 : /* Shifting Out */
		break;
	}
}


// Write to a register, bypassing data ISRs
void VIA_Write(uint8_t id, VIA_Register_t reg, uint8_t data)
{
	assert(id < VIA_MAXNUM);
	assert(reg < rINVALID);
	VIA_State_t *via = &VIA_State[id];
	
	uint8_t data_old = VIA_Read(id, reg);
	
	switch(reg) {
	case rIRB:  via->vIFR &= 0b11100111; // clear keyboard interrupts
	            via->vBufB = data; break;
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
	
	if ((data_old & via->vIFR) != (data & via->vIFR) && reg == rIFR) {
		VIAorSCCinterruptChngNtfy();
	}
}

// Read to a register
uint8_t VIA_Read(uint8_t id, VIA_Register_t reg)
{
	assert(id < VIA_MAXNUM);
	assert(reg < rINVALID);
	VIA_State_t *via = &VIA_State[id];
	
	switch(reg) {
	// not sure if reading *all* of vBufA or vBufB is correct,
	// but it shouldn't matter. 
	case rIRB:  via->vIFR &= 0b11100111; // clear keyboard interrupts
	            return via->vBufB;
	case rIRA:
	case rORA:  return via->vBufA;
	case rDDRB: return via->vDirB;
	case rDDRA: return via->vDirA;
	case rT1CL: via->vIFR &= 0b10111111; // TODO: is this used correctly?
	            return (via->vT1C & 0xFF00) >> 8;
	case rT1CH: return (via->vT1C & 0x00FF);
	case rT1LL: return (via->vT1L & 0xFF00) >> 8;
	case rT1LH: return (via->vT1L & 0x00FF);
	case rT2CL: via->vIFR &= 0b11011111; // TODO: is this used correctly?
	            return (via->vT2C & 0xFF00) >> 8;
	case rT2CH: return (via->vT2C & 0x00FF);
	case rSR:   
		        return via->vSR;
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
void VIA_WriteBit(uint8_t id, VIA_Register_t reg, uint8_t bit, bool value, bool runISR)
{
	uint8_t olddata = VIA_Read(id, reg);
	uint8_t data = olddata;
	if (value) { data |=  (1 << bit); } // set
	else       { data &= ~(1 << bit); } // clear
	VIA_Write(id, reg, data);
	
	// Call data-change ISR
	if (!runISR || (data == olddata)) { return; }
	if (reg == rIRA || reg == rORA) {
		if (VIA_State[id].vISR_A[bit] != NULL) {
			VIA_State[id].vISR_A[bit]();
		}
	} else if (reg == rIRB) {
		if (VIA_State[id].vISR_B[bit] != NULL) {
			VIA_State[id].vISR_B[bit]();
		}
	}
}

// Called by either end; store data in vSR.
// TODO: this probably shouldn't be instant.
void VIA_ShiftInData(uint8_t id, uint8_t v)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	assert(ShiftMode == 0 || ShiftMode == 3);
	VIA_State[id].vSR = v;
	VIA_RaiseInterrupt(id, 2); // data ready
	VIA_RaiseInterrupt(id, 4); // data clock
}

// Called by either end, get data out of vSR
// TODO: this probably shouldn't be instant.
uint8_t VIA_ShiftOutData(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	assert(ShiftMode == 7);
	VIA_RaiseInterrupt(id, 2); // data ready
	VIA_RaiseInterrupt(id, 3); // data rx
	VIA_WriteBit(id, rIFR, 4, (via->vSR & 1), false); // data clock
	return VIA_State[id].vSR;
}

/// Old messy VIA timer code (VIA-1 only for now) ///

#define CyclesPerViaTime (10 * ClockMult)
#define CyclesScaledPerViaTime (kCycleScale * CyclesPerViaTime)

// TODO: Move these into the VIA_State_t struct
bool VIA1_T1Running = true;
bool VIA1_T1IntReady = false;
iCountt VIA1_T1LastTime = 0;
uint8_t VIA1_T1_Active = 0;
const uint8_t VIA_T1_IRQ = 6;

bool VIA1_T2Running = true;
bool VIA1_T2IntReady = false;
bool VIA1_T2C_ShortTime = false;
iCountt VIA1_T2LastTime = 0;
uint8_t VIA1_T2_Active = 0;
const uint8_t VIA_T2_IRQ = 5;


void VIA1_DoTimer1Check()
{
	if (!VIA1_T1Running) { return; }
	VIA_State_t *via = &VIA_State[VIA1];
	
	iCountt NewTime = GetCuriCount();
	iCountt deltaTime = (NewTime - VIA1_T1LastTime);
	if (deltaTime != 0) {
		uint32_t Temp = via->vT1C; /* Get Timer 1 Counter */
		uint32_t deltaTemp = (deltaTime / CyclesPerViaTime) << (16 - kLn2CycleScale);
		/* may overflow */
		uint32_t NewTemp = Temp - deltaTemp;
		if (
			(deltaTime > (0x00010000UL * CyclesScaledPerViaTime)) ||
			((Temp <= deltaTemp) && (Temp != 0))
		) {
			//VIA_TickTimer1(VIA1);
		}

		via->vT1C = NewTemp;
		VIA1_T1LastTime = NewTime;
	}

	VIA1_T1IntReady = false;
	if ((via->vIFR & (1 << VIA_T1_IRQ)) == 0) {
		if (((via->vACR & 0x40) != 0) || (VIA1_T1_Active == 1)) {
			uint32_t NewTemp = via->vT1C; /* Get Timer 1 Counter */
			uint32_t NewTimer;
#ifdef _VIA_Debug
			fprintf(stderr, "posting Timer1Check, %d, %d\n",
				Temp, GetCuriCount());
#endif
			if (NewTemp == 0) {
				NewTimer = (0x00010000UL * CyclesScaledPerViaTime);
			} else {
				NewTimer =
					(1 + (NewTemp >> (16 - kLn2CycleScale)))
						* CyclesPerViaTime;
			}
			ICT_add(kICT_VIA1_Timer1Check, NewTimer);
			VIA1_T1IntReady = true;
		}
	}
}

/*
static void VIA1_CheckT1IntReady(void)
{
	VIA_State_t *via = &VIA_State[VIA1];
	if (VIA1_T1Running) {
		bool NewT1IntReady = false;

		if ((via->vIFR & (1 << VIA_T1_IRQ)) == 0) {
			if (((via->vACR & 0x40) != 0) || (VIA1_T1_Active == 1)) {
				NewT1IntReady = true;
			}
		}

		if (VIA1_T1IntReady != NewT1IntReady) {
			VIA1_T1IntReady = NewT1IntReady;
			if (NewT1IntReady) {
				VIA1_DoTimer1Check();
			}
		}
	}
}*/

uint16_t VIA1_GetT1InvertTime(void)
{
	VIA_State_t *via = &VIA_State[VIA1];

	if ((via->vACR & 0xC0) == 0xC0) {
		return via->vT1L;
	} else {
		return 0;
	}
}

void VIA1_DoTimer2Check(void)
{
	VIA_State_t *via = &VIA_State[VIA1];
	if (VIA1_T2Running) {
		iCountt NewTime = GetCuriCount();
		/* Get Timer 2 Counter */
		uint32_t Temp = via->vT2C;
		iCountt deltaTime = (NewTime - VIA1_T2LastTime);
		/* may overflow */
		uint32_t deltaTemp = (deltaTime / CyclesPerViaTime) << (16 - kLn2CycleScale);
		uint32_t NewTemp = Temp - deltaTemp;
		if (VIA1_T2_Active == 1) {
			if (
				(deltaTime > (0x00010000UL * CyclesScaledPerViaTime)) ||
				((Temp <= deltaTemp) && (Temp != 0))
			) {
				VIA1_T2C_ShortTime = false;
				VIA1_T2_Active = 0;
				VIA_RaiseInterrupt(VIA1, VIA_T2_IRQ);
#if VIA1_dolog && 1
				dbglog_WriteNote("VIA1 Timer 2 Interrupt");
#endif
			} else {
				uint32_t NewTimer;
#ifdef _VIA_Debug
				fprintf(stderr, "posting Timer2Check, %d, %d\n",
					Temp, GetCuriCount());
#endif
				if (NewTemp == 0) {
					NewTimer = (0x00010000UL * CyclesScaledPerViaTime);
				} else {
					NewTimer = (1 + (NewTemp >> (16 - kLn2CycleScale)))
						* CyclesPerViaTime;
				}
				ICT_add(kICT_VIA1_Timer2Check, NewTimer);
			}
		}
		via->vT2C = NewTemp;
		VIA1_T2LastTime = NewTime;
	}
}

void VIA1_ExtraTimeBegin(void)
{
	if (VIA1_T1Running) {
		VIA1_DoTimer1Check(); /* run up to this moment */
		VIA1_T1Running = false;
	}
	if (VIA1_T2Running & (! VIA1_T2C_ShortTime)) {
		VIA1_DoTimer2Check(); /* run up to this moment */
		VIA1_T2Running = false;
	}
}

void VIA1_ExtraTimeEnd(void)
{
	if (! VIA1_T1Running) {
		VIA1_T1Running = true;
		VIA1_T1LastTime = GetCuriCount();
		VIA1_DoTimer1Check();
	}
	if (! VIA1_T2Running) {
		VIA1_T2Running = true;
		VIA1_T2LastTime = GetCuriCount();
		VIA1_DoTimer2Check();
	}
}
