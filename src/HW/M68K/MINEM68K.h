/*
	MINEM68K.h

	Copyright (C) 2004 Bernd Schmidt, Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifndef MINEM68K_H
#define MINEM68K_H

extern void MINEM68K_Init(
	uint8_t *fIPL);
#if SmallGlobals
extern void MINEM68K_ReserveAlloc(void);
#endif

extern void m68k_IPLchangeNtfy(void);
extern void DiskInsertedPsuedoException(CPTR newpc, uint32_t data);
extern void m68k_reset(void);

extern int32_t GetCyclesRemaining(void);
extern void SetCyclesRemaining(int32_t n);

extern void m68k_go_nCycles(uint32_t n);

/*
	general purpose access of address space
	of emulated computer. (memory and
	memory mapped hardware.)
*/

extern uint8_t get_vm_byte(CPTR addr);
extern uint16_t get_vm_word(CPTR addr);
extern uint32_t get_vm_long(CPTR addr);

extern void put_vm_byte(CPTR addr, uint8_t b);
extern void put_vm_word(CPTR addr, uint16_t w);
extern void put_vm_long(CPTR addr, uint32_t l);

extern void SetHeadATTel(ATTep p);
extern ATTep FindATTel(CPTR addr);

#endif
