/*
	SCRNEMDV.c

	Copyright (C) 2006 Philip Cummins, Richard F. Bannister,
		Paul C. Pratt

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
	SCReeN EMulated DeVice

	Emulation of the screen in the Mac Plus.

	This code descended from "Screen-MacOS.c" in Richard F. Bannister's
	Macintosh port of vMac, by Philip Cummins.
*/

#include <stddef.h>
#include "SYSDEPNS.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/SCREEN/SCRNEMDV.h"
#include "CFGMAN.h"

#if ! IncludeVidMem
#define kMain_Offset      0x5900
#define kAlternate_Offset 0xD900
#define kMain_Buffer      (kRAM_Size - kMain_Offset)
#define kAlternate_Buffer (kRAM_Size - kAlternate_Offset)
#endif

// Configuration variables
uint16_t vMacScreenHeight;
uint16_t vMacScreenWidth;
uint16_t vMacScreenDepth;
uint32_t vMacScreenNumPixels;
uint32_t vMacScreenNumBits;
uint32_t vMacScreenNumBytes;
uint32_t vMacScreenBitWidth;
uint32_t vMacScreenByteWidth;
uint32_t vMacScreenMonoNumBytes;
uint32_t vMacScreenMonoByteWidth;
bool UseLargeScreenHack;
char *ScreenColorBlack = NULL;
char *ScreenColorWhite = NULL;

bool Screen_Init(void)
{
	// enable a palette because heck it
	return true;
}

bool Screen_LoadCfg()
{
	// Load configuration
	int64_t temp;
	bool okay;
	
	okay = Config_GetInt(CONFIG_VIDEO_HEIGHT, &temp, 342);
	if (!okay) { return false; }
	vMacScreenHeight = temp;
	okay = Config_GetInt(CONFIG_VIDEO_WIDTH, &temp, 512);
	if (!okay) { return false; }
	vMacScreenWidth  = temp;
	okay = Config_GetInt(CONFIG_VIDEO_DEPTH, &temp, 0);
	if (!okay) { return false; }
	vMacScreenDepth  = temp;
	okay = Config_GetBool(CONFIG_VIDEO_USEHACK, &UseLargeScreenHack, false);
	if (!okay) { return false; }
	okay = Config_GetString(CONFIG_VIDEO_BLACK, &ScreenColorBlack, "#000000");
	if (!okay) { return false; }
	okay = Config_GetString(CONFIG_VIDEO_WHITE, &ScreenColorWhite, "#FFFFFF");
	if (!okay) { return false; }
	
	// Compute the other sorts of things
	vMacScreenNumPixels = vMacScreenHeight * vMacScreenWidth;
	vMacScreenNumBits = vMacScreenNumPixels << vMacScreenDepth;
	vMacScreenNumBytes = vMacScreenNumBits / 8;
	vMacScreenBitWidth = vMacScreenWidth << vMacScreenDepth;
	vMacScreenByteWidth = vMacScreenBitWidth / 8;
	vMacScreenMonoNumBytes = vMacScreenNumPixels / 8;
	vMacScreenMonoByteWidth = (long)vMacScreenWidth / 8;
	return true;
}

void Screen_EndTickNotify(void)
{
	uint8_t * screencurrentbuff;

#if IncludeVidMem
	screencurrentbuff = VidMem;
#else
	if (SCRNvPage2 == 1) {
		screencurrentbuff = get_ram_address(kMain_Buffer);
	} else {
		screencurrentbuff = get_ram_address(kAlternate_Buffer);
	}
#endif

	Screen_OutputFrame(screencurrentbuff);
}
