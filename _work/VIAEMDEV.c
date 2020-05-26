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

	Emulates the Synertek SY6522 VIA found up until the Mac Plus.
	This code rewritten for target-independance and non-enum based config

	The VIA1 contains the following functionality:
	- Two timers (1 and 2)
	- Serial-to-parallel / parallel-to-serial bi-directional shift register
	- Two 8-bit bi-directional ports (A and B)
	- Ports are per-line settable to input or output
	- Square wave generation
	- Pulse counting

	The M68000 addresses the VIA simply via an interrupt line and a pair of
	registers, much like any other peripheal

	Because just about everything on the Mac is attached to a VIA, it's
	important to track what device is mapped to what line on what port.

	Summary of SY6522 pins:
	1. Phase 2 Clock (Φ2)
		Must be high for data transfer from M68k to occur
		Time base for timers, shift registers, etc.
	2. Chip select (CS1, ¬CS2)
		is a chip select line
	3. Register Select (RS0, RS1, RS2, RS3)
	               Name      Write                     Read
		0000 - ORB, IRB  Port B output             Port B input
		0001 - ORA, IRA  Port A output             Port A input
		0010 - DDRB      Data Direction, port B
		0011 - DDRA      Data Direction, port A
		0100 - T1L-L     T1 Low-Order Latches      T1 Low-Order Counter
		0101 - T1C-H     T1 High-Order Counter
		0110 - T1L-L     T1 Low-Order Latches
		0111 - T1L-H     T1 High-Order Latches
		1000 - T2C-L     T2 Low-Order Latches      T2 Low-Order Counter
		1001 - T2C-H     T2 High-Order Counter
		1010 - SR        Shift Register
		1011 - ACR       Auxillary Control Register
		1100 - PCR       Peripheal Control Register
		1101 - IFR       Interrupt flag
		1110 - IER       Interrupt enable
		1111 - ORA       Same as 0001 but no effect on handshake
	4. Read/Write line
		If low, write (processor to VIA register)
		If high, read (VIA register to processor)
	5. Data Bus (DB0-7)
		Transfers data between VIA and M68l
	6. Reset
		Clears all registers to 0.
		All I/O set to input, disablse timers, SR, interrupts, etc.
	7. IRQ
	8. Peripheal A Port (PA0 - PA7)
		8 lines. Each can be input or output, depending on value of DDRA
	9. Port A Control Lines (CA1/CA2)
		Interrupt inputs or handshake outputs
		Each line controls an internal interrupt flag w/ enable bit
		CA1 controls latching of data on P1 input
	10. Peripheal Port B (PB0 - PB7)
		Same as Port A, with timers!
		PB7 polarity can be controller by timer
		PB6 can count pulses w/ second timer
	11. Port B control lines (CB1/CB2)
		Same as CA
		Shift register serial I/O goes through one of these

	I'm not going to bother to emulate handshaking
*/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "VIAEMDEV.h"

/* Global state */

typedef struct {
	uint8_t  PortA_data;       // Port A data
	uint8_t  PortA_dir;        // Direction of Port A bits (0 = in, 1 = out)
	uint8_t  PortB_data;       // Port A data
	uint8_t  PortB_dir;        // Direction of Port B bits (0 = in, 1 = out)
	uint8_t  SR;               // current shift register state
	uint16_t T1_C;             // timer 1 count
	uint16_t T1_L;             // timer 1 latches
	uint16_t T2_C;             // timer 2 count
	uint16_t T2_L;             // timer 2 latches
	
	uint8_t  AUX_T1;           // Aux - Timer 1 Control      (0-3)
	uint8_t  AUX_T2;           // Aux - Timer 2 Control      (0-1)
	uint8_t  AUX_SR;           // Aux - SR Control           (0-7)
	bool     AUX_PALE;         // Aux - Port A Latch Enable  (0-1)
	bool     AUX_PBLE;         // Aux - Port B Latch Enable  (0-1)

	uint8_t  PC_CB2;           // Perip. - CB2 Control
	uint8_t  PC_CB1;           // Perip. - CB1 Edge Polarity (0 = -, 1 = +)
	uint8_t  PC_CA2;           // Perip. - CA2 Control
	uint8_t  PC_CA1;           // Perip. - CA1 Edge Polarity (0 = -, 1 = +)

	uint8_t  IF;               // Interrupt Flags
	uint8_t  IE;               // Interrupt Enable
} VIA1_State_t;

VIA1_State_t VIA1_State = {0};

/*
	For reference: Mac 128/512k port allocations:
	PA7   (I ) SCC ready for read
	PA6   ( O) Alt. screen buffer in use?
	PA5   ( O) Disk SEL line
	PA4   ( O) ROM low-mem overlay
	PA3   ( O) Alt. sound buffer
	PA0-2 ( O) Sound volume
	PB7   ( O) Sound on/off
	PB6   (I ) Horiz. Blank
	PB5   (I ) Mouse Y2
	PB4   (I ) Mouse X2
	PB3   (I ) Mouse button
	PB2   ( O) RTC serial enable
	RB1   ( O) RTC data-clock line
	RB0   (IO) RTC clock serial data

	SR - Keyboard data line

	Timer 1 - Sound generator stuff
	Timer 2 - Disk I/O events
	(or both can be for your own use!)

	IRQ7 - IRQ (all enabled VIA interrupts)
	IRQ6 - Timer 1
	IRQ5 - Timer 2
	IRQ4 - Keyboard clock
	IRQ3 - Keyboard data bit
	IRQ2 - Keyboard data ready
	IRQ1 - VBlank
	IRQ0 - One-second interrupt from RTC
*/

// Hardware reset
void VIA1_Zap(void) {
	memset(&VIA1_State, 0, sizeof(VIA1_State));
}
// Software reset
void VIA1_Reset(void) {
	VIA1_Zap();
}

// Write to a register
void VIA1_Write(uint8_t reg, uint8_t data)
{
	switch(reg) {
	case 0: // Port B data
		VIA1_State.PortB_data = data;
		break;
	case 1: // Port A data
	case 15:
		VIA1_State.PortA_data = data;
		break;
	case 2: // Port B direction
		VIA1_State.PortB_dir = data;
		break;
	case 3: // Port A direction
		VIA1_State.PortA_dir = data;
		break;
	case 4: // Timer 1 Low-Order Counter
	case 6: // Timer 1 Low-Order Latches
		VIA1_State.T1_L &= 0xFF00 | data;
		break;
	case 5: // Timer 1 High-Order Counter
		VIA1_State.T1_L &= 0x00FF | (data << 8);
		VIA1_State.T1_C = VIA1_State.T1_L;
		VIA1_State.IF   &= 0b10111111;
		break;
	case 7: // Timer 1 High-Order Latches
		VIA1_State.T1_L &= 0x00FF | (data << 8);
		break;
	case 8: // Timer 1 Low-Order Counter
		VIA1_State.T2_L &= 0xFF00 | data;
		break;
	case 9: // Timer 2 High-Order Counter
		VIA1_State.T2_L &= 0x00FF | (data << 8);
		VIA1_State.T2_C = VIA1_State.T2_L;
		VIA1_State.IF   &= 0b10111111;
		break;
	case 10:
		VIA1_State.SR = data;
		break;
	case 11:
		VIA1_State.AUX_T1   = (data & 0b11000000) >> 6;
		VIA1_State.AUX_T2   = (data & 0b00100000) >> 5;
		VIA1_State.AUX_SR   = (data & 0b00011100) >> 2;
		VIA1_State.AUX_PBLE = (data & 0b00000010) >> 1;
		VIA1_State.AUX_PALE = (data & 0b00000001) >> 0;
		break;
	case 12:
		VIA1_State.PC_CB2   = (data & 0b11100000) >> 5;
		VIA1_State.PC_CB1   = (data & 0b00010000) >> 4;
		VIA1_State.PC_CA2   = (data & 0b00001110) >> 1;
		VIA1_State.PC_CA1   = (data & 0b00000001) >> 0;
		break;
	case 13: // Interrupt Flag
		VIA1_State.IF = data;
		break;
	case 14: // Interrupt Enable
		VIA1_State.IE = data;
		break;
	}
}

// Read to a register
uint8_t VIA1_Read(uint8_t reg)
{
	switch(reg) {
	case 0: // Port B data (technically incorrect but meh)
		return VIA1_State.PortB_data & ~VIA1_State.PortB_dir;
	case 1: // Port A data
	case 15:
		return VIA1_State.PortA_data & ~VIA1_State.PortA_dir;
	case 2: // Port B direction
		return VIA1_State.PortB_dir;
	case 3: // Port A direction
		return VIA1_State.PortA_dir;
	case 4: // Timer 1 Low-Order Counter
		VIA1_State.IF   &= 0b10111111;
		return (VIA1_State.T1_C & 0xFF00);
	case 5: // Timer 1 High-Order Counter
		return (VIA1_State.T1_C & 0x00FF) >> 8;
	case 6: // Timer 1 Low-Order Latches
		return (VIA1_State.T1_L & 0xFF00);
	case 7: // Timer 1 High-Order Latches
		return (VIA1_State.T1_L & 0x00FF) >> 8;
	case 8: // Timer 2 Low-Order Counter
		VIA1_State.IF   &= 0b11011111;
		return (VIA1_State.T2_C & 0xFF00);
	case 9: // Timer 2 High-Order Counter
		return (VIA1_State.T2_C & 0x00FF) >> 8;
	case 10:
		return VIA1_State.SR;
	case 11:
		return (
			(VIA1_State.AUX_T1   << 6) |
			(VIA1_State.AUX_T2   << 5) |
			(VIA1_State.AUX_SR   << 2) |
			(VIA1_State.AUX_PBLE << 1) |
			(VIA1_State.AUX_PALE << 0)
		);
	case 12:
		return (
			(VIA1_State.PC_CB2 << 5) |
			(VIA1_State.PC_CB1 << 4) |
			(VIA1_State.PC_CA2 << 1) |
			(VIA1_State.PC_CA1 << 0)
		);
	case 13: // Interrupt Flag
		return VIA1_State.IF;
		break;
	case 14: // Interrupt Enable
		return VIA1_State.IE;
		break;
	default:
		return 0;
	}
}

// Tick timers
void VIA1_Tick() {

}


// Shift in one byte of data to keyboard shift register
void VIA1_ShiftInData(uint8_t v)
{
	VIA1_State.SR = v;
	// somehow signal to keyboard that we're ready
}

// Shift out one byte of data from keyboard shift register
uint8_t VIA1_ShiftOutData(void)
{
	// signal to keyboard to get new data?
	return VIA1_State.SR;
	
}
