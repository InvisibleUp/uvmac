/*
	MOUSEMDV.c

	Copyright (C) 2006 Philip Cummins, Paul C. Pratt

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
	MOUSe EMulated DeVice

	Emulation of the mouse in the Mac Plus.

	This code descended from "Mouse-MacOS.c" in Richard F. Bannister's
	Macintosh port of vMac, by Philip Cummins.
*/

#include "SYSDEPNS.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/SCC/SCCEMDEV.h"
#include "HW/MOUSE/MOUSEMDV.h"
#include "HW/VIA/VIAEMDEV.h"

static void Mouse_SetButtonUp(bool value)
{
	switch(CurEmMd) {
	case kEmMd_Twig43:
	case kEmMd_Twiggy:
	case kEmMd_128K:
	case kEmMd_512Ke:
	case kEmMd_Plus:
		VIA_WriteBit(VIA1, rIRB, 3, value, false);
		return;
	default:
		return;
	}
}

static bool Mouse_Enabled()
{
	switch(CurEmMd) {
	case kEmMd_Twig43:
	case kEmMd_Twiggy:
	case kEmMd_128K:
	case kEmMd_512Ke:
	case kEmMd_Plus:
		//return SCC_InterruptsEnabled();
		return true;
	default:
		return false;
	}
}

GLOBALPROC Mouse_Update(void)
{
#if HaveMasterEvtQLock
	if (0 != MasterEvtQLock) {
		--MasterEvtQLock;
	}
#endif

	/*
		Check mouse position first. After mouse button or key event,
		can't process another mouse position until following tick,
		otherwise button or key will be in wrong place.
	*/

	/*
		if start doing this too soon after boot,
		will mess up memory check
	*/
	if (Mouse_Enabled()) {
		EvtQEl *p;

		if (
#if HaveMasterEvtQLock
			(0 == MasterEvtQLock) &&
#endif
			(nullpr != (p = EvtQOutP())))
		{
#if EmClassicKbrd
#if EnableMouseMotion
			if (EvtQElKindMouseDelta == p->kind) {

				if ((p->u.pos.h != 0) || (p->u.pos.v != 0)) {
					put_ram_word(0x0828,
						get_ram_word(0x0828) + p->u.pos.v);
					put_ram_word(0x082A,
						get_ram_word(0x082A) + p->u.pos.h);
					put_ram_byte(0x08CE, get_ram_byte(0x08CF));
						/* Tell MacOS to redraw the Mouse */
				}
				EvtQOutDone();
			} else
#endif
#endif
			if (EvtQElKindMousePos == p->kind) {
				uint32_t NewMouse = (p->u.pos.v << 16) | p->u.pos.h;

				if (get_ram_long(0x0828) != NewMouse) {
					put_ram_long(0x0828, NewMouse);
						/* Set Mouse Position */
					put_ram_long(0x082C, NewMouse);
#if EmClassicKbrd
					put_ram_byte(0x08CE, get_ram_byte(0x08CF));
						/* Tell MacOS to redraw the Mouse */
#else
					put_ram_long(0x0830, NewMouse);
					put_ram_byte(0x08CE, 0xFF);
						/* Tell MacOS to redraw the Mouse */
#endif
				}
				EvtQOutDone();
			}
		}
	}

#if EmClassicKbrd
	{
		EvtQEl *p;

		if (
#if HaveMasterEvtQLock
			(0 == MasterEvtQLock) &&
#endif
			(nullpr != (p = EvtQOutP())))
		{
			if (EvtQElKindMouseButton == p->kind) {
				Mouse_SetButtonUp(p->u.press.down ? 0 : 1);
				EvtQOutDone();
				MasterEvtQLock = 4;
			}
		}
	}
#endif
}

GLOBALPROC Mouse_EndTickNotify(void)
{
	if (Mouse_Enabled()) {
		/* tell platform specific code where the mouse went */
		CurMouseV = get_ram_word(0x082C);
		CurMouseH = get_ram_word(0x082E);
	}
}
