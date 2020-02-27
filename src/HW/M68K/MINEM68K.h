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

EXPORTPROC MINEM68K_Init(
	uint8_t *fIPL);
#if SmallGlobals
EXPORTPROC MINEM68K_ReserveAlloc(void);
#endif

EXPORTPROC m68k_IPLchangeNtfy(void);
EXPORTPROC DiskInsertedPsuedoException(CPTR newpc, uint32_t data);
EXPORTPROC m68k_reset(void);

EXPORTFUNC int32_t GetCyclesRemaining(void);
EXPORTPROC SetCyclesRemaining(int32_t n);

EXPORTPROC m68k_go_nCycles(uint32_t n);

/*
	general purpose access of address space
	of emulated computer. (memory and
	memory mapped hardware.)
*/

EXPORTFUNC uint8_t get_vm_byte(CPTR addr);
EXPORTFUNC uint16_t get_vm_word(CPTR addr);
EXPORTFUNC uint32_t get_vm_long(CPTR addr);

EXPORTPROC put_vm_byte(CPTR addr, uint8_t b);
EXPORTPROC put_vm_word(CPTR addr, uint16_t w);
EXPORTPROC put_vm_long(CPTR addr, uint32_t l);

EXPORTPROC SetHeadATTel(ATTep p);
EXPORTFUNC ATTep FindATTel(CPTR addr);

#endif
