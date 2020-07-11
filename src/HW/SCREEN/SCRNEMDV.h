/*
	HW/SCREEN/SCRNEMDV.h

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

#ifndef SCRNEMDV_H
#define SCRNEMDV_H

bool Screen_LoadCfg(void);
bool Screen_Init(void);
void Screen_EndTickNotify(void);
void Screen_RaiseVBlank();

extern uint16_t vMacScreenHeight;
extern uint16_t vMacScreenWidth;
extern uint16_t vMacScreenDepth;
extern uint32_t vMacScreenNumPixels;
extern uint32_t vMacScreenNumBits;
extern uint32_t vMacScreenNumBytes;
extern uint32_t vMacScreenBitWidth;
extern uint32_t vMacScreenByteWidth;
extern uint32_t vMacScreenMonoNumBytes;
extern uint32_t vMacScreenMonoByteWidth;
bool UseLargeScreenHack;
char *ScreenColorBlack;
char *ScreenColorWhite;

#endif
