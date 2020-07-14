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
#include <stdio.h>
#include "VIAEMDEV.h"
#include "GLOBGLUE.h"

/* Global state */
VIA_State_t VIA_State[VIA_MAXNUM]; 

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

// Hardware reset
bool VIA_Zap(void) {
	memset(VIA_State, 0, sizeof(VIA_State));
	VIA_Reset();
	return true;
}
// Software reset
void VIA_Reset(void) {
	for (int i = 0; i < VIA_MAXNUM; i += 1) {
		VIA_State[i].vBufA = 0x00;
		VIA_State[i].vBufB = 0x00;
		VIA_State[i].vDirA = 0x00;
		VIA_State[i].vDirB = 0x00;
		VIA_State[i].vIER  = 0x00;
		VIA_State[i].vIFR  = 0x00;
		VIA_State[i].vSR   = 0x00;
		VIA_State[i].vACR  = 0x00;
		VIA_State[i].vPCR  = 0x00;
		VIA_State[i].vT1C  = 0x00;
		VIA_State[i].vT1L  = 0x00;
		VIA_State[i].vT2C  = 0x00;
		VIA_State[i].vT2L  = 0x00;
		VIA_State[i].vCB2  = true;
	}
	// temporary
	VIA1_T1Running = false;
	VIA1_T1IntReady = false;
	VIA1_T1LastTime = 0;
	VIA1_T1_Active = 0;
	VIA1_T2Running = false;
	VIA1_T2IntReady = false;
	VIA1_T2LastTime = 0;
	VIA1_T2_Active = 0;
}

// Raise an interrupt by irq number
void VIA_RaiseInterrupt(uint8_t id, uint8_t irq)
{
	assert (id < VIA_MAXNUM);
	assert (irq < 7);
	
	// Set interrupt flag
	uint8_t vIFR_old = VIA_State[id].vIFR & VIA_State[id].vIER & 0b01111111;
	VIA_State[id].vIFR |= (1 << irq);// | (1 << 7);
	uint8_t vIFR_new = VIA_State[id].vIFR & VIA_State[id].vIER & 0b01111111;
	
	// Call interrupt handler, if required
	if (vIFR_old != vIFR_new) {
		//fprintf(stderr, "IRQ %d raised\n", irq);
		VIAorSCCinterruptChngNtfy();
	} else {
		//fprintf(stderr, "IRQ %d attempted\n", irq);
	}
}

// Lower an interrupt by irq number
void VIA_LowerInterrupt(uint8_t id, uint8_t irq)
{
	assert (id < VIA_MAXNUM);
	assert (irq < 7);
	
	// Set interrupt flag
	uint8_t vIFR_old = VIA_State[id].vIFR & VIA_State[id].vIER & 0b01111111;
	VIA_State[id].vIFR &= ~(1 << irq);
	uint8_t vIFR_new = VIA_State[id].vIFR & VIA_State[id].vIER & 0b01111111;
	
	// Call interrupt handler, if required
	if (vIFR_old != vIFR_new) {
		//fprintf(stderr, "IRQ %d lowered\n", irq);
		VIAorSCCinterruptChngNtfy();
	} else {
		//fprintf(stderr, "IRQ %d attempted (lower)\n", irq);
	}
}

// Register data state-change notification interrupts
void VIA_RegisterDataISR(uint8_t id, uint8_t port, uint8_t irq, VIA_ISR_t isr)
{
	assert(id < VIA_MAXNUM);
	assert(port == DataRegA || port == DataRegB);
	assert(irq < 8);
	
	if (port == DataRegA) {
		VIA_State[id].vISR_A[irq] = isr;
		//fprintf(stderr, "ISR PA%d registered\n", irq);
	} else {
		VIA_State[id].vISR_B[irq] = isr;
		//fprintf(stderr, "ISR PB%d registered\n", irq);
	}
}

static void VIA_RunDataISR(uint8_t id, VIA_Register_t reg, uint8_t bit)
{
	assert(id < VIA_MAXNUM);
	if (reg == rIRA || reg == rORA) {
		if (VIA_State[id].vISR_A[bit] != NULL) {
			//fprintf(stderr, "ISR PA%d running\n", bit);
			VIA_State[id].vISR_A[bit]();
		} else {
			//fprintf(stderr, "ISR PA%d not found\n", bit);
		}
	} else if (reg == rIRB) {
		if (VIA_State[id].vISR_B[bit] != NULL) {
			//fprintf(stderr, "ISR PB%d running\n", bit);
			VIA_State[id].vISR_B[bit]();
		} else {
			//fprintf(stderr, "ISR PB%d not found\n", bit);
		}
	}
}
	
// CB2 stuff
void VIA_RegisterCB2ISR(uint8_t id, VIA_ISR_t isr)
{
	assert(id < VIA_MAXNUM);
	VIA_State[id].vISR_CB2 = isr;
}

static void VIA_RunCB2ISR(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	if (VIA_State[id].vISR_CB2 != NULL) {
		VIA_State[id].vISR_CB2();
	}
}

// Get the value of CB2, specifically
bool VIA_GetCB2(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	return VIA_State[id].vCB2;
}

// And likewise, set it
void VIA_SetSB2(uint8_t id, bool value)
{
	assert(id < VIA_MAXNUM);
	VIA_State[id].vCB2 = value;
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

// temporary debugging stuff
#define BIN_PAT "%c%c%c%c_%c%c%c%c"
#define TO_BIN(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

// Write to a register, bypassing data ISRs
void VIA_Write(uint8_t id, VIA_Register_t reg, uint8_t data, bool runISR)
{
	assert(id < VIA_MAXNUM);
	assert(reg < rINVALID);
	VIA_State_t *via = &VIA_State[id];
	
	// Set up before/after comparisions
	uint8_t data_old = 0;
	if (reg == rIRA || reg == rORA) {
		data_old = via->vBufA;
	} else if (reg == rIRB) {
		data_old = via->vBufB;
	} else if (reg == rIFR || reg == rIER) {
		//data_old = via->vIFR & via->vIER & 0b01111111;
		data_old = via->vIFR;
	} else if (reg == rACR) {
		data_old = via->vACR;
	}
	
	/*if (reg == rIRA || reg == rORA) {
		fprintf(stderr, "Set PA to "BIN_PAT"\n", TO_BIN(data));
	} else {
		fprintf(stderr, "Set PB to "BIN_PAT"\n", TO_BIN(data));
	}*/
	//fprintf(stderr, "VIA%d Write %d <- %d\n", id+1, reg, data);
	/*if (reg == rSR) {
		fprintf(stderr, "vSR: %d, %d, %d\n", true, ((via->vACR & 0x1C) >> 2), data);
	}*/
	
	switch(reg) {
	case rIRB:  via->vIFR &= 0b11100111; // clear keyboard interrupts
	            via->vBufB = data; break;
	case rIRA:
	case rORA:  via->vIFR &= 0b11111100; // clear time interrupts
	            via->vBufA = data; break;
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
	case rSR:   VIA_ShiftInData_M68k(id, data); break;
	case rACR:  via->vACR = data; break;
	case rPCR:  via->vPCR = data; break;
	case rIFR:  via->vIFR &= (~data & 0b01111111); break; // seems to be correct?
	case rIER:
		switch(data & 0b10000000) {
		case 0b00000000:  // clear
			via->vIER &= ~(data & 0b01111111);
			break;
		default: // set
			via->vIER |=  (data & 0b01111111);
			break;
		}
		break;
	default: assert(true);
	}
	
	// Run vBufA or vBufB ISRs if needed
	uint8_t diff = (data ^ data_old);
	if (runISR && (reg == rIRA || reg == rORA || reg == rIRB)) {
		// Iterate through each bit
		uint8_t bit = 0;
		uint8_t mask = 0b1;
		while (mask != 0) {
			// Run ISR only on rising edge
			if (diff & mask) { VIA_RunDataISR(id, reg, bit); }
			bit += 1;
			mask <<= 1;
		}
	}
	
	// Check ISRs for changes if required
	uint8_t vIFR_new = via->vIFR & via->vIER & 0b01111111;
	uint8_t vIFR_old = data_old & via->vIER & 0b01111111;
	if (runISR && (reg == rIFR || reg == rIER) && (vIFR_old != vIFR_new))
	{
		/*fprintf(
			stderr, "IRQ changed ("BIN_PAT") -> ("BIN_PAT")\n",
			TO_BIN(vIFR_old & (vIFR_old ^ vIFR_new)),
			TO_BIN(vIFR_new & (vIFR_old ^ vIFR_new))
		);*/
		VIAorSCCinterruptChngNtfy();
	}
	/*else if (reg == rIFR || reg == rIER)
	{
		fprintf(stderr, "IRQ attempt ("BIN_PAT")\n", TO_BIN((data_old ^ vIFR_new)));
	}*/
	
	// Check if shift mode has changed
	if (reg == rACR) {
		uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
		uint8_t ShiftMode_old = (data_old & 0x1C) >> 2;
		// Clear keyboard interrupt flag if we modified shift register params
		if (ShiftMode == 0) {
			// lower interrupt if we're now disabling that
			VIA_LowerInterrupt(id, 2);
		} else {
			// Check if shift direction has changed
			if ((ShiftMode & 0b100) != (ShiftMode_old & 0b100)) {
				if ((ShiftMode & 0b100) == 0) { // if now shifting in
					// notify keyboard/ADB controller
					via->vCB2 = true;
					VIA_RunCB2ISR(id);
				}
			}
		}
	}
}

// Read to a register
uint8_t VIA_Read(uint8_t id, VIA_Register_t reg)
{
	assert(id < VIA_MAXNUM);
	assert(reg < rINVALID);
	VIA_State_t *via = &VIA_State[id];
	
	/*if (reg == rSR) {
		fprintf(stderr, "vSR: %d, %d, %d\n", false, ((via->vACR & 0x1C) >> 2), via->vSR);
	}*/
	
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
	case rSR:   return VIA_ShiftOutData_M68k(id);
	case rACR:  return via->vACR;
	case rPCR:  return via->vPCR;
	case rIFR:  return via->vIFR;
	case rIER:  if (via->vIER == 0) { return 0; }
	            else { return via->vIER |= 0b10000000; }
	default:    assert(true); return 0;
	}
}

// temporary debugging thing
/*uint8_t VIA_Read(uint8_t id, VIA_Register_t reg)
{
	uint8_t result = VIA_Read2(id, reg);
	fprintf(stderr, "VIA%d Read %d -> %d\n", id+1, reg, result);
	return result;
}*/

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
	VIA_Write(id, reg, data, runISR);
	
	// Call data-change ISR
	/*uint8_t diff = ~data & olddata;
	if (!runISR || (diff == 0)) { return; }
	VIA_RunDataISR(id, reg, bit);*/
}

// ShiftMode states (from M68k perspective)
// 0b000 (0): Shift in under external control w/ no interrupts.
// 0b001 (1): Shift in under control of Timer 2
// 0b010 (2): Shift in under system clock control
// 0b011 (3): Shift in under external control. ISR every byte. R/W resets ISR.
// 0b100 (4): Shift out under T2 control (forever)
// 0b101 (5): Shift out under T2 control
// 0b110 (6): Shift out under system clock control
// 0b111 (7): Shift out under external control. ISR every byte. R/W resets ISR.

// TODO: this probably shouldn't be instant.
// also i got the in/out mixed up compared to the datasheet. oh well.
void VIA_ShiftInData_M68k(uint8_t id, uint8_t v)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	
	//VIA_LowerInterrupt(id, 2); // data ready
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	assert(((ShiftMode & 0b100) == 0b100) || (ShiftMode == 0));
	
	VIA_State[id].vSR = v;
	
	// If shifting out under system clock...
	if (ShiftMode == 6) {
		// If data line is high, clear line and notify ADB/keyboard
		if (VIA_State[id].vCB2 == true) {
			VIA_State[id].vCB2 = false;
			VIA_RunCB2ISR(id);
		}
		// and raise SR data ready ISR
		VIA_RaiseInterrupt(id, 2);
	}
}

// TODO: this probably shouldn't be instant.
uint8_t VIA_ShiftOutData_M68k(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	assert((ShiftMode & 0b100) == 0b000);
	
	// Notify keyboard
	//via->vCB2 = true;
	//VIA_RunCB2ISR(id);
	return VIA_State[id].vSR;
}

// Same functions, from the peripheal end
void VIA_ShiftInData_Ext(uint8_t id, uint8_t v)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	//fprintf(stderr, "vSR: %d, %d, %d (ext)\n", true, ShiftMode, v);
	
	assert ((ShiftMode & 0b100) == 0b000);
	
	if (ShiftMode != 0) {
		VIA_State[id].vSR = v;
		VIA_RaiseInterrupt(id, 2); // data ready
	}
}

// TODO: this probably shouldn't be instant.
uint8_t VIA_ShiftOutData_Ext(uint8_t id)
{
	assert(id < VIA_MAXNUM);
	VIA_State_t *via = &VIA_State[id];
	
	uint8_t ShiftMode = (via->vACR & 0x1C) >> 2;
	//fprintf(stderr, "vSR: %d, %d, %d (ext)\n", false, ShiftMode, VIA_State[id].vSR);
	assert(((ShiftMode & 0b100) == 0b100) || (ShiftMode == 0));
	
	VIA_RaiseInterrupt(id, 2);
	VIA_RaiseInterrupt(id, 4);
	VIA_State[id].vCB2 = ((VIA_State[id].vSR & 1) == 1);
	return VIA_State[id].vSR;
}

/// Old messy VIA timer code (VIA-1 only for now) ///

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
