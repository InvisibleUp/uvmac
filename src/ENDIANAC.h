/*
	ENDIANAC.h

	Copyright (C) 2006 Bernd Schmidt, Paul C. Pratt

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
	ENDIAN ACcess

	Deals with endian issues in memory access.

	This code is adapted from code in the Un*x Amiga Emulator by
	Bernd Schmidt, as found in vMac by Philip Cummins.
*/

#ifdef ENDIANAC_H
#ifndef AllFiles
#error "header already included"
#endif
#else
#define ENDIANAC_H
#endif


#define do_get_mem_byte(a) ((uint8_t)*((uint8_t *)(a)))

#if BigEndianUnaligned
#define do_get_mem_word(a) ((uint16_t)*((uint16_t *)(a)))
#else
LOCALINLINEFUNC uint16_t do_get_mem_word(ui3p a)
{
#if LittleEndianUnaligned
	uint16_t b = (*((uint16_t *)(a)));

	return ((b & 0x00FF) << 8) | ((b >> 8) & 0x00FF);
#else
	return (((uint16_t)*a) << 8) | ((uint16_t)*(a + 1));
#endif
}
#endif

#if BigEndianUnaligned
#define do_get_mem_long(a) ((uint32_t)*((uint32_t *)(a)))
#elif HaveSwapUi5r && LittleEndianUnaligned
#define do_get_mem_long(a) (SwapUi5r((uint32_t)*((uint32_t *)(a))))
#else
LOCALINLINEFUNC uint32_t do_get_mem_long(ui3p a)
{
#if LittleEndianUnaligned
#if 0
	uint32_t b = (*((uint32_t *)(a)));
	return ((b & 0x000000FF) << 24)
		|  ((b & 0x0000FF00) <<  8)
		|  ((b & 0x00FF0000) >>  8)
		|  ((b & 0xFF000000) >> 24);
#endif
#if 0
	uint32_t b = (*((uint32_t *)(a)));
	return ((b << 24) & 0xFF000000)
		|  ((b <<  8) & 0x00FF0000)
		|  ((b >>  8) & 0x0000FF00)
		|  ((b >> 24) & 0x000000FF);
	/*
		no, this doesn't do well with apple tools,
		instead try combining two 16 bit swaps.
	*/
#endif
	uint32_t b = (*((uint32_t *)(a)));
	uint16_t b1 = b;
	uint16_t b2 = b >> 16;
	uint16_t c1 = ((b1 & 0x00FF) << 8) | ((b1 >> 8) & 0x00FF);
	uint16_t c2 = ((b2 & 0x00FF) << 8) | ((b2 >> 8) & 0x00FF);

	return (((uint32_t)c1) << 16) | ((uint32_t)c2);
	/*
		better, though still doesn't use BSWAP
		instruction with apple tools for intel.
	*/
#else
	return (((uint32_t)*a) << 24) | (((uint32_t)*(a + 1)) << 16)
		| (((uint32_t)*(a + 2)) << 8) | ((uint32_t)*(a + 3));
#endif
}
#endif

#define do_put_mem_byte(a, v) ((*((uint8_t *)(a))) = (v))

#if BigEndianUnaligned
#define do_put_mem_word(a, v) ((*((uint16_t *)(a))) = (v))
#else
LOCALINLINEFUNC void do_put_mem_word(ui3p a, uint16_t v)
{
#if LittleEndianUnaligned
	uint16_t b = ((v & 0x00FF) << 8) | ((v >> 8) & 0x00FF);

	*(uint16_t *)a = b;
#else
	*a = v >> 8;
	*(a + 1) = v;
#endif
}
#endif

#if BigEndianUnaligned
#define do_put_mem_long(a, v) ((*((uint32_t *)(a))) = (v))
#elif HaveSwapUi5r && LittleEndianUnaligned
#define do_put_mem_long(a, v) ((*((uint32_t *)(a))) = SwapUi5r(v))
#else
LOCALINLINEFUNC void do_put_mem_long(ui3p a, uint32_t v)
{
#if LittleEndianUnaligned
	uint16_t b1 = v;
	uint16_t b2 = v >> 16;
	uint16_t c1 = ((b1 & 0x00FF) << 8) | ((b1 >> 8) & 0x00FF);
	uint16_t c2 = ((b2 & 0x00FF) << 8) | ((b2 >> 8) & 0x00FF);

	*(uint32_t *)a = (c1 << 16) | c2;
#else
	*a = v >> 24;
	*(a + 1) = v >> 16;
	*(a + 2) = v >> 8;
	*(a + 3) = v;
#endif
}
#endif
