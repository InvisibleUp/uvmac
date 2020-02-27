/*
	HPMCHACK.c

	Copyright (C) 2016 Steve Chamberlin, Paul C. Pratt

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
	HaPpy MaCintosh Hack

	Patch the ROM for alternatives to the
	Happy Macintosh icon displayed on boot
	when a disk is inserted.

	Bitmaps from http://www.bigmessowires.com/mac-rom-inator/
*/

#include <string.h>
#include "SYSDEPNS.h"
#include "UI/COMOSGLU.h"

// Enum of alternate icons
typedef enum AHM_types {
	kAHM_aside,
	kAHM_cheese,
	kAHM_evil,
	kAHM_horror,
	kAHM_lady_mac,
	kAHM_moustache,
	kAHM_nerdy,
	kAHM_pirate,
	kAHM_sleepy,
	kAHM_sly,
	kAHM_sunglasses,
	kAHM_surprise,
	kAHM_tongue,
	kAHM_yuck,
	kAHM_zombie
} AHM_t;

// Bitmap includes
#include "aside.xbm"
#include "cheese.xbm"
#include "evil.xbm"
#include "horror.xbm"
#include "lady.xbm"
#include "moustache.xbm"
#include "nerdy.xbm"
#include "pirate.xbm"
#include "sleepy.xbm"
#include "sly.xbm"
#include "sunglasses.xbm"
#include "surprise.xbm"
#include "tongue.xbm"
#include "yuck.xbm"
#include "zombie.xbm"

// Lookup table
unsigned char * HMAC_BITS[] = {
	HMAC_ASIDE_BITS,
	HMAC_CHEESE_BITS
};

#if CurEmMd <= kEmMd_Twig43
#define HappyMacBase 0xA34
#elif CurEmMd <= kEmMd_Twiggy
#define HappyMacBase 0x8F4
#elif CurEmMd <= kEmMd_128K
#define HappyMacBase 0x8A0
#elif CurEmMd <= kEmMd_Plus
#define HappyMacBase 0xFD2
#elif CurEmMd <= kEmMd_Classic
#define HappyMacBase 0x125C
#elif CurEmMd <= kEmMd_PB100
#define HappyMacBase 0x2BB0 - 0x18
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define HappyMacBase 0x1948 - 0x18
#endif

LOCALPROC PatchHappyMac(AHM_t ahm)
{
	uint8_t *dst = ROM + HappyMacBase + 0x18;
	uint8_t *src = HMAC_BITS[ahm];
	memcpy(dst, src, 20);
}
