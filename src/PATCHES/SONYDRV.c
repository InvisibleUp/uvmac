/*
	Replacement for .Sony driver
	68k machine code, compiled from mydriver.a
	Included in the resource file for WIN32 builds
*/
#include <stdint.h>
#ifdef _WINDOWS
#include <windows.h>
#endif
#include "incbin/incbin.h"

#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "PATCHES/SONYDRV.h"
#include "PATCHES/SCRNHACK.h"

// Include binaries
#ifndef _WINDOWS
INCBIN(SonyDriver, "SONYDRV.bin");
INCBIN(SonyIcon, "SONYICO.bin");
#endif

void Sony_LoadDriver(uint8_t *pto, int *size)
{
#if defined(gSonyDriverData)
	memcpy(pto, gSonyDriverData, gSonyDriverSize);
	*size = gSonyDriverSize;
#elif defined(_WINDOWS)
	HRSRC hDrvInfo = FindResource(NULL, "SONY_DRV", RT_RCDATA);
	HGLOBAL hDrv = LoadResource(NULL, hDrvInfo);
	DWORD sDrv = SizeofResource(NULL, hDrvInfo);
	void *pDrv = LockResource(hDrv);
	memcpy(pto, pDrv, sDrv);
	*size = sDrv;
#else
#error("Unsupported platform/compiler")
#endif
}

void Sony_LoadIcon(uint8_t *pto)
{
	disk_icon_addr = (pto - ROM) + kROM_Base;
#if defined(gSonyIconData)
	memcpy(pto, gSonyIcon, gSonyIconSize;
	pto += sizeof(gSonyIconSize);
#elif defined(_WINDOWS)
	HRSRC hIcoInfo = FindResource(NULL, "SONY_ICO", RT_RCDATA);
	HGLOBAL hIco = LoadResource(NULL, hIcoInfo);
	DWORD sIco = SizeofResource(NULL, hIcoInfo);
	void *pIco = LockResource(hIco);
	memcpy(pto, pIco, sIco);
	pto += sizeof(sIco);
#else
#error("Unsupported platform/compiler")
#endif
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
	pto += drvSize;
	do_put_mem_word(pto, kcom_callcheck);
	pto += 2;
	do_put_mem_word(pto, kExtnSony);
	pto += 2;
	do_put_mem_long(pto, kExtn_Block_Base); /* pokeaddr */
	pto += 4;
}

void Sony_Install(void)
{
	uint8_t * pto = Sony_DriverBase + ROM;
	int drvSize = 0;
	if (!UseSonyPatch) { return; }
	Sony_LoadDriver(pto, &drvSize);
	Sony_TwiggyPatch(pto);
	Sony_CallPatch(pto, drvSize);
	Sony_LoadIcon(pto);
	// yeah this sucks but it's so awful and intertwined that i have no choice
	ScreenHack_Install(pto);
}
