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

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#endif

#include "incbin/incbin.h"
#include "PATCHES/ROMEMDEV.h"

// Temporarily disable due to CMake errors
/*#define UseSonyPatch \
	((CurEmMd <= kEmMd_Classic) || (CurEmMd == kEmMd_II) \
		|| (CurEmMd == kEmMd_IIx))*/

#ifndef UseLargeScreenHack
#define UseLargeScreenHack 0
#endif

#if UseSonyPatch
/*
	Replacement for .Sony driver
	68k machine code, compiled from mydriver.a
*/
INCBIN(SonyDriver, "SONY.bin");
#endif

#if UseSonyPatch
LOCALVAR const uint8_t disk_icon[] = {
	0x7F, 0xFF, 0xFF, 0xF0,
	0x81, 0x00, 0x01, 0x08,
	0x81, 0x00, 0x71, 0x04,
	0x81, 0x00, 0x89, 0x02,
	0x81, 0x00, 0x89, 0x01,
	0x81, 0x00, 0x89, 0x01,
	0x81, 0x00, 0x89, 0x01,
	0x81, 0x00, 0x89, 0x01,
	0x81, 0x00, 0x89, 0x01,
	0x81, 0x00, 0x71, 0x01,
	0x81, 0x00, 0x01, 0x01,
	0x80, 0xFF, 0xFE, 0x01,
	0x80, 0x00, 0x00, 0x01,
	0x80, 0x00, 0x00, 0x01,
	0x80, 0x00, 0x00, 0x01,
	0x80, 0x00, 0x00, 0x01,
	0x83, 0xFF, 0xFF, 0xC1,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x06, 0x30, 0x21,
	0x84, 0x06, 0x60, 0x21,
	0x84, 0x06, 0xC0, 0x21,
	0x84, 0x07, 0x80, 0x21,
	0x84, 0x07, 0x00, 0x21,
	0x84, 0x06, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x84, 0x00, 0x00, 0x21,
	0x7F, 0xFF, 0xFF, 0xFE,

	/* mask */

	0x3F, 0xFF, 0xFF, 0xF0,
	0x7F, 0xFF, 0xFF, 0xF0,
	0xFF, 0xFF, 0xFF, 0xFC,
	0xFF, 0xFF, 0xFF, 0xFC,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0x7F, 0xFF, 0xFF, 0xFC,
	0x3F, 0xFF, 0xFF, 0xFC,

	/* empty pascal string */
	0x00, 0x00,
};
#endif

#if CurEmMd <= kEmMd_Twig43
#define Sony_DriverBase 0x1836
#elif CurEmMd <= kEmMd_Twiggy
#define Sony_DriverBase 0x16E4
#elif CurEmMd <= kEmMd_128K
#define Sony_DriverBase 0x1690
#elif CurEmMd <= kEmMd_Plus
#define Sony_DriverBase 0x17D30
#elif CurEmMd <= kEmMd_Classic
#define Sony_DriverBase 0x34680
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define Sony_DriverBase 0x2D72C
#endif

#define kVidMem_Base 0x00540000

#if UseSonyPatch
LOCALPROC Sony_Install(void)
{
	uint8_t * pto = Sony_DriverBase + ROM;

	MoveBytes((anyp)gSonyDriverData, (anyp)pto, gSonyDriverSize);
#if CurEmMd <= kEmMd_Twiggy
	do_put_mem_long(pto + 0x14, 0x4469736B);
		/* 'Disk' instead of 'Sony' */
#if CurEmMd <= kEmMd_Twig43
	do_put_mem_word(pto + 0xEA, 0x0C8A);
#else
	do_put_mem_word(pto + 0xEA, 0x0B74);
#endif
#endif

	pto += gSonyDriverSize;

	do_put_mem_word(pto, kcom_callcheck);
	pto += 2;
	do_put_mem_word(pto, kExtnSony);
	pto += 2;
	do_put_mem_long(pto, kExtn_Block_Base); /* pokeaddr */
	pto += 4;

	disk_icon_addr = (pto - ROM) + kROM_Base;
	MoveBytes((anyp)disk_icon, (anyp)pto, sizeof(disk_icon));
	pto += sizeof(disk_icon);

#if UseLargeScreenHack
	{
		uint8_t * patchp = pto;

#include "PATCHES/SCRNHACK.h"
	}
#endif

	(void) pto; /* avoid warning about unused */
}
#endif

#ifndef DisableRomCheck
#define DisableRomCheck 1
#endif

#ifndef DisableRamTest
#define DisableRamTest 1
#endif

#ifdef CurAltHappyMac
#include "HPMCHACK.h"
#endif

#ifdef ln2mtb
LOCALPROC ROMscrambleForMTB(void)
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

GLOBALFUNC bool ROM_Init(void)
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

#if UseSonyPatch
	Sony_Install();
#endif

#ifdef ln2mtb
	ROMscrambleForMTB();
#endif

	return true;
}
