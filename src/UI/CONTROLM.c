/*
	UI/CONTROLM.c

	Copyright (C) 2007 Paul C. Pratt

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
	CONTROL Mode
*/

#include "SYSDEPNS.h"
#include "CNFGGLOB.h"
#include "CNFGRAPI.h"
#include "STRCONST.h"
#include "ERRCODES.h"

#include "LANG/INTLCHAR.h"
#include "UI/COMOSGLU.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"

#include "UI/CONTROLM.h"
#include <SDL.h>
#include "UI/SDL2/OSGLUSD2.h"

/* Constants and globals */

uimr SpecialModes = 0;
bool NeedWholeScreenDraw = false;
uint8_t * CntrlDisplayBuff = nullpr;

#define ControlBoxh0 0
#define ControlBoxw 62
#define ControlBoxv0 0

#define hLimit (ControlBoxh0 + ControlBoxw - 1)
#define hStart (ControlBoxh0 + 1)

typedef void (*SpclModeBody) (void);

#define Keyboard_UpdateKeyMap1 Keyboard_UpdateKeyMap
#define DisconnectKeyCodes1 DisconnectKeyCodes

void MacMsgOverride(char *title, char *msg)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, msg, main_wind);
}

LOCALPROC WarnMsgCorruptedROM(void)
{
	MacMsgOverride(kStrCorruptedROMTitle, kStrCorruptedROMMessage);
}

LOCALPROC WarnMsgUnsupportedROM(void)
{
	MacMsgOverride(kStrUnsupportedROMTitle,
		kStrUnsupportedROMMessage);
}

