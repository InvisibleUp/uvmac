/*
	Replacement for .Sony driver
	68k machine code, compiled from mydriver.a
	Included in the resource file for WIN32 builds
*/
#include <stdint.h>
#include <string.h>
#include "incbin/incbin.h"

#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "PATCHES/SONYDRV.h"
#include "PATCHES/SCRNHACK.h"

// Include binaries
INCBIN(SonyDriver, "rsrc/SONYDRV.bin");
INCBIN(SonyIcon, "rsrc/SONYICO.bin");

void Sony_LoadDriver(uint8_t *pto, int *size)
{
	memcpy(pto, gSonyDriverData, gSonyDriverSize);
	*size = gSonyDriverSize;
}

void Sony_LoadIcon(uint8_t *pto, int *icoSize)
{
	disk_icon_addr = (pto - ROM) + kROM_Base;
	memcpy(pto, gSonyIconData, gSonyIconSize);
	*icoSize = gSonyIconSize;
}

void Sony_TwiggyPatch(uint8_t *pto)
{
	if (CurEmMd == kEmMd_Twiggy || CurEmMd == kEmMd_Twig43) {
		/* 'Disk' instead of 'Sony' */
		do_put_mem_long(pto + 0x14, 0x4469736B); 
		if (CurEmMd == kEmMd_Twig43) {
			do_put_mem_word(pto + 0xEA, 0x0C8A);
		} else {
			do_put_mem_word(pto + 0xEA, 0x0B74);
		}
	}
}

void Sony_CallPatch(uint8_t *pto, int drvSize)
{
	do_put_mem_word(pto, kcom_callcheck);
	do_put_mem_word(pto+2, kExtnSony);
	do_put_mem_long(pto+4, kExtn_Block_Base); /* pokeaddr */
}

void Sony_Install(void)
{
	uint8_t * pto = Sony_DriverBase + ROM;
	int drvSize, icoSize;
	if (!UseSonyPatch) { return; }
	Sony_LoadDriver(pto, &drvSize);
	Sony_TwiggyPatch(pto);

	pto += drvSize;
	Sony_CallPatch(pto, drvSize);
	pto += 8;

	Sony_LoadIcon(pto, &icoSize);
	pto += icoSize;

	// yeah this sucks but it's so awful and intertwined that i have no choice
	//ScreenHack_Install(pto);
}
