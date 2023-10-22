/*
	ROMEMDEV.c

	Copyright (C) 2007 Philip Cummins, Paul C. Pratt

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
	Read Only Memory EMulated DEVice

	Checks the header of the loaded ROM image, and then patches
	the ROM image.

	This code descended from "ROM.c" in vMac by Philip Cummins.

	Support for "Twiggy" Mac by Mathew Hybler.
*/

#include "EMCONFIG.h"
#include "SYSDEPNS.h"
#include "GLOBGLUE.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "PATCHES/ROMEMDEV.h"
#include "PATCHES/SCRNHACK.h"
#include "PATCHES/SONYDRV.h"
#ifdef CurAltHappyMac
#include "HPMCHACK.h"
#endif

#ifdef ln2mtb
static void ROMscrambleForMTB(void)
{
	int32_t j;
	uint8_t * p = ROM;
	uint8_t * p2 = ROM + (1 << ln2mtb);

	for (j = kROM_Size / (1 << ln2mtb) / 2; --j >= 0; ) {
		int32_t i;

		for (i = (1 << ln2mtb); --i >= 0; ) {
			uint8_t t0 = *p;
			uint8_t t1 = *p2;
			*p++ = t1;
			*p2++ = t0;
		}

		p += (1 << ln2mtb);
		p2 += (1 << ln2mtb);
	}
}
#endif

 bool ROM_Init(void)
{
#if DisableRomCheck

/* skip the rom checksum */
#if CurEmMd <= kEmMd_Twig43
	/* no checksum code */
#elif CurEmMd <= kEmMd_Twiggy
	do_put_mem_word(0x136 + ROM, 0x6004);
#elif CurEmMd <= kEmMd_128K
	do_put_mem_word(0xE2 + ROM, 0x6004);
#elif CurEmMd <= kEmMd_Plus
	do_put_mem_word(0xD7A + ROM, 0x6022);
#elif CurEmMd <= kEmMd_Classic
	do_put_mem_word(0x1C68 + ROM, 0x6008);
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	do_put_mem_word(0x2AB0 + ROM, 0x6008);
#endif

#endif /* DisableRomCheck */


#if DisableRamTest

#if CurEmMd <= kEmMd_128K
#elif CurEmMd <= kEmMd_Plus
	do_put_mem_word(3752 + ROM, 0x4E71);
		/* shorten the ram check read */
	do_put_mem_word(3728 + ROM, 0x4E71);
		/* shorten the ram check write */
#elif CurEmMd <= kEmMd_Classic
	do_put_mem_word(134 + ROM, 0x6002);
	do_put_mem_word(286 + ROM, 0x6002);
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	do_put_mem_word(0xEE + ROM, 0x6002);
	do_put_mem_word(0x1AA + ROM, 0x6002);
#endif

#endif /* DisableRamTest */

#ifdef CurAltHappyMac
	PatchHappyMac();
#endif

	/* do_put_mem_word(862 + ROM, 0x4E71); */ /* shorten set memory */

	Sony_Install();
	//ScreenHack_Install();	in above fcn temporarily

#ifdef ln2mtb
	ROMscrambleForMTB();
#endif

	return true;
}
