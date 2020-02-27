/*
	OSGLUSDL.c

	Copyright (C) 2012 Paul C. Pratt

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
	Operating System GLUe for SDL library

	All operating system dependent code for the
	SDL Library should go here.
*/

#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"

#include "UI/MYOSGLUE.h"

#include "STRCONST.h"

/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

/* --- control mode and internationalization --- */

#define NeedCell2PlainAsciiMap 1

#include "LANG/INTLCHAR.h"

/* --- sending debugging info to file --- */

#if dbglog_HAVE

#define dbglog_ToStdErr 0

#if ! dbglog_ToStdErr
LOCALVAR FILE *dbglog_File = NULL;
#endif

LOCALFUNC bool dbglog_open0(void)
{
#if dbglog_ToStdErr
	return true;
#else
	dbglog_File = fopen("dbglog.txt", "w");
	return (NULL != dbglog_File);
#endif
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
#if dbglog_ToStdErr
	(void) fwrite(s, 1, L, stderr);
#else
	if (dbglog_File != NULL) {
		(void) fwrite(s, 1, L, dbglog_File);
	}
#endif
}

LOCALPROC dbglog_close0(void)
{
#if ! dbglog_ToStdErr
	if (dbglog_File != NULL) {
		fclose(dbglog_File);
		dbglog_File = NULL;
	}
#endif
}

#endif

/* --- information about the environment --- */

#define WantColorTransValid 0

#include "UI/COMOSGLU.h"

#include "UTILS/PBUFSTDC.h"

#include "UI/CONTROLM.h"

/* --- text translation --- */

LOCALPROC NativeStrFromCStr(char *r, char *s)
{
	uint8_t ps[ClStrMaxLength];
	int i;
	int L;

	ClStrFromSubstCStr(&L, ps, s);

	for (i = 0; i < L; ++i) {
		r[i] = Cell2PlainAsciiMap[ps[i]];
	}

	r[L] = 0;
}

/* --- drives --- */

#define NotAfileRef NULL

LOCALVAR FILE *Drives[NumDrives]; /* open disk image files */

LOCALPROC InitDrives(void)
{
	/*
		This isn't really needed, Drives[i] and DriveNames[i]
		need not have valid values when not vSonyIsInserted[i].
	*/
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
	}
}

GLOBALOSGLUFUNC tMacErr vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	tMacErr err = mnvm_miscErr;
	FILE *refnum = Drives[Drive_No];
	uint32_t NewSony_Count = 0;

	if (0 == fseek(refnum, Sony_Start, SEEK_SET)) {
		if (IsWrite) {
			NewSony_Count = fwrite(Buffer, 1, Sony_Count, refnum);
		} else {
			NewSony_Count = fread(Buffer, 1, Sony_Count, refnum);
		}

		if (NewSony_Count == Sony_Count) {
			err = mnvm_noErr;
		}
	}

	if (nullpr != Sony_ActCount) {
		*Sony_ActCount = NewSony_Count;
	}

	return err; /*& figure out what really to return &*/
}

GLOBALOSGLUFUNC tMacErr vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	tMacErr err = mnvm_miscErr;
	FILE *refnum = Drives[Drive_No];
	long v;

	if (0 == fseek(refnum, 0, SEEK_END)) {
		v = ftell(refnum);
		if (v >= 0) {
			*Sony_Count = v;
			err = mnvm_noErr;
		}
	}

	return err; /*& figure out what really to return &*/
}

LOCALFUNC tMacErr vSonyEject0(tDrive Drive_No, bool deleteit)
{
	FILE *refnum = Drives[Drive_No];

	DiskEjectedNotify(Drive_No);

	fclose(refnum);
	Drives[Drive_No] = NotAfileRef; /* not really needed */

	return mnvm_noErr;
}

GLOBALOSGLUFUNC tMacErr vSonyEject(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, false);
}

LOCALPROC UnInitDrives(void)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (vSonyIsInserted(i)) {
			(void) vSonyEject(i);
		}
	}
}

LOCALFUNC bool Sony_Insert0(FILE *refnum, bool locked,
	char *drivepath)
{
	tDrive Drive_No;
	bool IsOk = false;

	if (! FirstFreeDisk(&Drive_No)) {
		MacMsg(kStrTooManyImagesTitle, kStrTooManyImagesMessage,
			false);
	} else {
		/* printf("Sony_Insert0 %d\n", (int)Drive_No); */

		{
			Drives[Drive_No] = refnum;
			DiskInsertNotify(Drive_No, locked);

			IsOk = true;
		}
	}

	if (! IsOk) {
		fclose(refnum);
	}

	return IsOk;
}

LOCALFUNC bool Sony_Insert1(char *drivepath, bool silentfail)
{
	bool locked = false;
	/* printf("Sony_Insert1 %s\n", drivepath); */
	FILE *refnum = fopen(drivepath, "rb+");
	if (NULL == refnum) {
		locked = true;
		refnum = fopen(drivepath, "rb");
	}
	if (NULL == refnum) {
		if (! silentfail) {
			MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
		}
	} else {
		return Sony_Insert0(refnum, locked, drivepath);
	}
	return false;
}

LOCALFUNC tMacErr LoadMacRomFrom(char *path)
{
	tMacErr err;
	FILE *ROM_File;
	int File_Size;

	ROM_File = fopen(path, "rb");
	if (NULL == ROM_File) {
		err = mnvm_fnfErr;
	} else {
		File_Size = fread(ROM, 1, kROM_Size, ROM_File);
		if (File_Size != kROM_Size) {
			if (feof(ROM_File)) {
				MacMsgOverride(kStrShortROMTitle,
					kStrShortROMMessage);
				err = mnvm_eofErr;
			} else {
				MacMsgOverride(kStrNoReadROMTitle,
					kStrNoReadROMMessage);
				err = mnvm_miscErr;
			}
		} else {
			err = ROM_IsValid();
		}
		fclose(ROM_File);
	}

	return err;
}

#if 0 /* no drag and drop to make use of this */
LOCALFUNC bool Sony_Insert1a(char *drivepath, bool silentfail)
{
	bool v;

	if (! ROM_loaded) {
		v = (mnvm_noErr == LoadMacRomFrom(drivepath));
	} else {
		v = Sony_Insert1(drivepath, silentfail);
	}

	return v;
}
#endif

LOCALFUNC bool Sony_Insert2(char *s)
{
	return Sony_Insert1(s, true);
}

LOCALFUNC bool Sony_InsertIth(int i)
{
	bool v;

	if ((i > 9) || ! FirstFreeDisk(nullpr)) {
		v = false;
	} else {
		char s[] = "disk?.dsk";

		s[4] = '0' + i;

		v = Sony_Insert2(s);
	}

	return v;
}

LOCALFUNC bool LoadInitialImages(void)
{
	if (! AnyDiskInserted()) {
		int i;

		for (i = 1; Sony_InsertIth(i); ++i) {
			/* stop on first error (including file not found) */
		}
	}

	return true;
}

/* --- ROM --- */

LOCALVAR char *rom_path = NULL;

LOCALFUNC bool LoadMacRom(void)
{
	tMacErr err;

	if ((NULL == rom_path)
		|| (mnvm_fnfErr == (err = LoadMacRomFrom(rom_path))))
	if (mnvm_fnfErr == (err = LoadMacRomFrom(RomFileName)))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}

/* --- video out --- */

#if VarFullScreen
LOCALVAR bool UseFullScreen = (WantInitFullScreen != 0);
#endif

#if EnableMagnify
LOCALVAR bool UseMagnify = (WantInitMagnify != 0);
#endif

LOCALVAR bool gBackgroundFlag = false;
LOCALVAR bool gTrueBackgroundFlag = false;
LOCALVAR bool CurSpeedStopped = true;

#if EnableMagnify
#define MaxScale WindowScale
#else
#define MaxScale 1
#endif


LOCALVAR SDL_Surface *surface = nullpr;

LOCALVAR uint8_t * ScalingBuff = nullpr;

LOCALVAR uint8_t * CLUT_final;

#define CLUT_finalsz (256 * 8 * 4 * MaxScale)
	/*
		256 possible values of one byte
		8 pixels per byte maximum (when black and white)
		4 bytes per destination pixel maximum
			multiplied by WindowScale if EnableMagnify
	*/

#define ScrnMapr_DoMap UpdateBWDepth3Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateBWDepth4Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 4
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateBWDepth5Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateBWDepth3ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateBWDepth4ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 4
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateBWDepth5ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"


#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)

#define ScrnMapr_DoMap UpdateColorDepth3Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateColorDepth4Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 4
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateColorDepth5Copy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateColorDepth3ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateColorDepth4ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 4
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnMapr_DoMap UpdateColorDepth5ScaledCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#endif


LOCALPROC HaveChangedScreenBuff(uint16_t top, uint16_t left,
	uint16_t bottom, uint16_t right)
{
	int i;
	int j;
	uint8_t *p;
	Uint32 pixel;
#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
	Uint32 CLUT_pixel[CLUT_size];
#endif
	Uint32 BWLUT_pixel[2];
	uint32_t top2 = top;
	uint32_t left2 = left;
	uint32_t bottom2 = bottom;
	uint32_t right2 = right;

#if EnableMagnify
	if (UseMagnify) {
		top2 *= WindowScale;
		left2 *= WindowScale;
		bottom2 *= WindowScale;
		right2 *= WindowScale;
	}
#endif

	if (SDL_MUSTLOCK(surface)) {
		if (SDL_LockSurface(surface) < 0) {
			return;
		}
	}

	{

	int bpp = surface->format->BytesPerPixel;
	uint32_t ExpectedPitch = vMacScreenWidth * bpp;

#if EnableMagnify
	if (UseMagnify) {
		ExpectedPitch *= WindowScale;
	}
#endif

#if 0 != vMacScreenDepth
	if (UseColorMode) {
#if vMacScreenDepth < 4
		for (i = 0; i < CLUT_size; ++i) {
			CLUT_pixel[i] = SDL_MapRGB(surface->format,
				CLUT_reds[i] >> 8,
				CLUT_greens[i] >> 8,
				CLUT_blues[i] >> 8);
		}
#endif
	} else
#endif
	{
		BWLUT_pixel[1] = SDL_MapRGB(surface->format, 0, 0, 0);
			/* black */
		BWLUT_pixel[0] = SDL_MapRGB(surface->format, 255, 255, 255);
			/* white */
	}

	if ((0 == ((bpp - 1) & bpp)) /* a power of 2 */
		&& (surface->pitch == ExpectedPitch)
#if (vMacScreenDepth > 3)
		&& ! UseColorMode
#endif
		)
	{
		int k;
		Uint32 v;
#if EnableMagnify
		int a;
#endif
		int PixPerByte =
#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
			UseColorMode ? (1 << (3 - vMacScreenDepth)) :
#endif
			8;
		Uint8 *p4 = (Uint8 *)CLUT_final;

		for (i = 0; i < 256; ++i) {
			for (k = PixPerByte; --k >= 0; ) {

#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
				if (UseColorMode) {
					v = CLUT_pixel[
#if 3 == vMacScreenDepth
						i
#else
						(i >> (k << vMacScreenDepth))
							& (CLUT_size - 1)
#endif
						];
				} else
#endif
				{
					v = BWLUT_pixel[(i >> k) & 1];
				}

#if EnableMagnify
				for (a = UseMagnify ? WindowScale : 1; --a >= 0; )
#endif
				{
					switch (bpp) {
						case 1: /* Assuming 8-bpp */
							*p4++ = v;
							break;
						case 2: /* Probably 15-bpp or 16-bpp */
							*(Uint16 *)p4 = v;
							p4 += 2;
							break;
						case 4: /* Probably 32-bpp */
							*(Uint32 *)p4 = v;
							p4 += 4;
							break;
					}
				}
			}
		}

		ScalingBuff = (uint8_t *)surface->pixels;

#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
		if (UseColorMode) {
#if EnableMagnify
			if (UseMagnify) {
				switch (bpp) {
					case 1:
						UpdateColorDepth3ScaledCopy(
							top, left, bottom, right);
						break;
					case 2:
						UpdateColorDepth4ScaledCopy(
							top, left, bottom, right);
						break;
					case 4:
						UpdateColorDepth5ScaledCopy(
							top, left, bottom, right);
						break;
				}
			} else
#endif
			{
				switch (bpp) {
					case 1:
						UpdateColorDepth3Copy(top, left, bottom, right);
						break;
					case 2:
						UpdateColorDepth4Copy(top, left, bottom, right);
						break;
					case 4:
						UpdateColorDepth5Copy(top, left, bottom, right);
						break;
				}
			}
		} else
#endif
		{
#if EnableMagnify
			if (UseMagnify) {
				switch (bpp) {
					case 1:
						UpdateBWDepth3ScaledCopy(
							top, left, bottom, right);
						break;
					case 2:
						UpdateBWDepth4ScaledCopy(
							top, left, bottom, right);
						break;
					case 4:
						UpdateBWDepth5ScaledCopy(
							top, left, bottom, right);
						break;
				}
			} else
#endif
			{
				switch (bpp) {
					case 1:
						UpdateBWDepth3Copy(top, left, bottom, right);
						break;
					case 2:
						UpdateBWDepth4Copy(top, left, bottom, right);
						break;
					case 4:
						UpdateBWDepth5Copy(top, left, bottom, right);
						break;
				}
			}
		}

	} else {
		uint8_t *the_data = (uint8_t *)GetCurDrawBuff();

		/* adapted from putpixel in SDL documentation */

		for (i = top2; i < bottom2; ++i) {
			for (j = left2; j < right2; ++j) {
				int i0 = i;
				int j0 = j;
				Uint8 *bufp = (Uint8 *)surface->pixels
					+ i * surface->pitch + j * bpp;

#if EnableMagnify
				if (UseMagnify) {
					i0 /= WindowScale;
					j0 /= WindowScale;
				}
#endif

#if 0 != vMacScreenDepth
				if (UseColorMode) {
#if vMacScreenDepth < 4
					p = the_data + ((i0 * vMacScreenWidth + j0)
						>> (3 - vMacScreenDepth));
					{
						uint8_t k = (*p >> (((~ j0)
								& ((1 << (3 - vMacScreenDepth)) - 1))
							<< vMacScreenDepth))
							& (CLUT_size - 1);
						pixel = CLUT_pixel[k];
					}
#elif 4 == vMacScreenDepth
					p = the_data + ((i0 * vMacScreenWidth + j0) << 1);
					{
						uint16_t t0 = do_get_mem_word(p);
						pixel = SDL_MapRGB(surface->format,
							((t0 & 0x7C00) >> 7)
								| ((t0 & 0x7000) >> 12),
							((t0 & 0x03E0) >> 2)
								| ((t0 & 0x0380) >> 7),
							((t0 & 0x001F) << 3)
								| ((t0 & 0x001C) >> 2));
					}
#elif 5 == vMacScreenDepth
					p = the_data + ((i0 * vMacScreenWidth + j0) << 2);
					pixel = SDL_MapRGB(surface->format,
						p[1],
						p[2],
						p[3]);
#endif
				} else
#endif
				{
					p = the_data + ((i0 * vMacScreenWidth + j0) / 8);
					pixel = BWLUT_pixel[(*p >> ((~ j0) & 0x7)) & 1];
				}

				switch (bpp) {
					case 1: /* Assuming 8-bpp */
						*bufp = pixel;
						break;
					case 2: /* Probably 15-bpp or 16-bpp */
						*(Uint16 *)bufp = pixel;
						break;
					case 3:
						/* Slow 24-bpp mode, usually not used */
						if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
							bufp[0] = (pixel >> 16) & 0xff;
							bufp[1] = (pixel >> 8) & 0xff;
							bufp[2] = pixel & 0xff;
						} else {
							bufp[0] = pixel & 0xff;
							bufp[1] = (pixel >> 8) & 0xff;
							bufp[2] = (pixel >> 16) & 0xff;
						}
						break;
					case 4: /* Probably 32-bpp */
						*(Uint32 *)bufp = pixel;
						break;
				}
			}
		}
	}

	}

	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
	}

	SDL_UpdateRect(surface, left2, top2,
		right2 - left2, bottom2 - top2);
}

LOCALPROC DrawChangesAndClear(void)
{
	if (ScreenChangedBottom > ScreenChangedTop) {
		HaveChangedScreenBuff(ScreenChangedTop, ScreenChangedLeft,
			ScreenChangedBottom, ScreenChangedRight);
		ScreenClearChanges();
	}
}

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		AutoScrollScreen();
	}
#endif
	DrawChangesAndClear();
}

/* --- mouse --- */

/* cursor hiding */

LOCALVAR bool HaveCursorHidden = false;
LOCALVAR bool WantCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		(void) SDL_ShowCursor(SDL_ENABLE);
	}
}

/* cursor moving */

LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
#if EnableMagnify
	if (UseMagnify) {
		h *= WindowScale;
		v *= WindowScale;
	}
#endif

	SDL_WarpMouse(h, v);

	return true;
}

/* cursor state */

LOCALPROC MousePositionNotify(int NewMousePosh, int NewMousePosv)
{
	bool ShouldHaveCursorHidden = true;

#if EnableMagnify
	if (UseMagnify) {
		NewMousePosh /= WindowScale;
		NewMousePosv /= WindowScale;
	}
#endif

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MousePositionSetDelta(NewMousePosh - SavedMouseH,
			NewMousePosv - SavedMouseV);
		SavedMouseH = NewMousePosh;
		SavedMouseV = NewMousePosv;
	} else
#endif
	{
		if (NewMousePosh < 0) {
			NewMousePosh = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosh >= vMacScreenWidth) {
			NewMousePosh = vMacScreenWidth - 1;
			ShouldHaveCursorHidden = false;
		}
		if (NewMousePosv < 0) {
			NewMousePosv = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosv >= vMacScreenHeight) {
			NewMousePosv = vMacScreenHeight - 1;
			ShouldHaveCursorHidden = false;
		}

#if VarFullScreen
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			ShouldHaveCursorHidden = true;
		}
#endif

		/* if (ShouldHaveCursorHidden || CurMouseButton) */
		/*
			for a game like arkanoid, would like mouse to still
			move even when outside window in one direction
		*/
		MousePositionSet(NewMousePosh, NewMousePosv);
	}

	WantCursorHidden = ShouldHaveCursorHidden;
}

LOCALPROC CheckMouseState(void)
{
	/*
		this doesn't work as desired, doesn't get mouse movements
		when outside of our window.
	*/
	int x;
	int y;

	(void) SDL_GetMouseState(&x, &y);
	MousePositionNotify(x, y);
}

/* --- keyboard input --- */

LOCALFUNC uint8_t SDLKey2MacKeyCode(SDLKey i)
{
	uint8_t v = MKC_None;

	switch (i) {
		case SDLK_BACKSPACE: v = MKC_BackSpace; break;
		case SDLK_TAB: v = MKC_Tab; break;
		case SDLK_CLEAR: v = MKC_Clear; break;
		case SDLK_RETURN: v = MKC_Return; break;
		case SDLK_PAUSE: v = MKC_Pause; break;
		case SDLK_ESCAPE: v = MKC_formac_Escape; break;
		case SDLK_SPACE: v = MKC_Space; break;
		case SDLK_EXCLAIM: /* ? */ break;
		case SDLK_QUOTEDBL: /* ? */ break;
		case SDLK_HASH: /* ? */ break;
		case SDLK_DOLLAR: /* ? */ break;
		case SDLK_AMPERSAND: /* ? */ break;
		case SDLK_QUOTE: v = MKC_SingleQuote; break;
		case SDLK_LEFTPAREN: /* ? */ break;
		case SDLK_RIGHTPAREN: /* ? */ break;
		case SDLK_ASTERISK: /* ? */ break;
		case SDLK_PLUS: /* ? */ break;
		case SDLK_COMMA: v = MKC_Comma; break;
		case SDLK_MINUS: v = MKC_Minus; break;
		case SDLK_PERIOD: v = MKC_Period; break;
		case SDLK_SLASH: v = MKC_formac_Slash; break;
		case SDLK_0: v = MKC_0; break;
		case SDLK_1: v = MKC_1; break;
		case SDLK_2: v = MKC_2; break;
		case SDLK_3: v = MKC_3; break;
		case SDLK_4: v = MKC_4; break;
		case SDLK_5: v = MKC_5; break;
		case SDLK_6: v = MKC_6; break;
		case SDLK_7: v = MKC_7; break;
		case SDLK_8: v = MKC_8; break;
		case SDLK_9: v = MKC_9; break;
		case SDLK_COLON: /* ? */ break;
		case SDLK_SEMICOLON: v = MKC_SemiColon; break;
		case SDLK_LESS: /* ? */ break;
		case SDLK_EQUALS: v = MKC_Equal; break;
		case SDLK_GREATER: /* ? */ break;
		case SDLK_QUESTION: /* ? */ break;
		case SDLK_AT: /* ? */ break;

		case SDLK_LEFTBRACKET: v = MKC_LeftBracket; break;
		case SDLK_BACKSLASH: v = MKC_formac_BackSlash; break;
		case SDLK_RIGHTBRACKET: v = MKC_RightBracket; break;
		case SDLK_CARET: /* ? */ break;
		case SDLK_UNDERSCORE: /* ? */ break;
		case SDLK_BACKQUOTE: v = MKC_formac_Grave; break;

		case SDLK_a: v = MKC_A; break;
		case SDLK_b: v = MKC_B; break;
		case SDLK_c: v = MKC_C; break;
		case SDLK_d: v = MKC_D; break;
		case SDLK_e: v = MKC_E; break;
		case SDLK_f: v = MKC_F; break;
		case SDLK_g: v = MKC_G; break;
		case SDLK_h: v = MKC_H; break;
		case SDLK_i: v = MKC_I; break;
		case SDLK_j: v = MKC_J; break;
		case SDLK_k: v = MKC_K; break;
		case SDLK_l: v = MKC_L; break;
		case SDLK_m: v = MKC_M; break;
		case SDLK_n: v = MKC_N; break;
		case SDLK_o: v = MKC_O; break;
		case SDLK_p: v = MKC_P; break;
		case SDLK_q: v = MKC_Q; break;
		case SDLK_r: v = MKC_R; break;
		case SDLK_s: v = MKC_S; break;
		case SDLK_t: v = MKC_T; break;
		case SDLK_u: v = MKC_U; break;
		case SDLK_v: v = MKC_V; break;
		case SDLK_w: v = MKC_W; break;
		case SDLK_x: v = MKC_X; break;
		case SDLK_y: v = MKC_Y; break;
		case SDLK_z: v = MKC_Z; break;

		case SDLK_KP0: v = MKC_KP0; break;
		case SDLK_KP1: v = MKC_KP1; break;
		case SDLK_KP2: v = MKC_KP2; break;
		case SDLK_KP3: v = MKC_KP3; break;
		case SDLK_KP4: v = MKC_KP4; break;
		case SDLK_KP5: v = MKC_KP5; break;
		case SDLK_KP6: v = MKC_KP6; break;
		case SDLK_KP7: v = MKC_KP7; break;
		case SDLK_KP8: v = MKC_KP8; break;
		case SDLK_KP9: v = MKC_KP9; break;

		case SDLK_KP_PERIOD: v = MKC_Decimal; break;
		case SDLK_KP_DIVIDE: v = MKC_KPDevide; break;
		case SDLK_KP_MULTIPLY: v = MKC_KPMultiply; break;
		case SDLK_KP_MINUS: v = MKC_KPSubtract; break;
		case SDLK_KP_PLUS: v = MKC_KPAdd; break;
		case SDLK_KP_ENTER: v = MKC_formac_Enter; break;
		case SDLK_KP_EQUALS: v = MKC_KPEqual; break;

		case SDLK_UP: v = MKC_Up; break;
		case SDLK_DOWN: v = MKC_Down; break;
		case SDLK_RIGHT: v = MKC_Right; break;
		case SDLK_LEFT: v = MKC_Left; break;
		case SDLK_INSERT: v = MKC_formac_Help; break;
		case SDLK_HOME: v = MKC_formac_Home; break;
		case SDLK_END: v = MKC_formac_End; break;
		case SDLK_PAGEUP: v = MKC_formac_PageUp; break;
		case SDLK_PAGEDOWN: v = MKC_formac_PageDown; break;

		case SDLK_F1: v = MKC_formac_F1; break;
		case SDLK_F2: v = MKC_formac_F2; break;
		case SDLK_F3: v = MKC_formac_F3; break;
		case SDLK_F4: v = MKC_formac_F4; break;
		case SDLK_F5: v = MKC_formac_F5; break;
		case SDLK_F6: v = MKC_F6; break;
		case SDLK_F7: v = MKC_F7; break;
		case SDLK_F8: v = MKC_F8; break;
		case SDLK_F9: v = MKC_F9; break;
		case SDLK_F10: v = MKC_F10; break;
		case SDLK_F11: v = MKC_F11; break;
		case SDLK_F12: v = MKC_F12; break;

		case SDLK_F13: /* ? */ break;
		case SDLK_F14: /* ? */ break;
		case SDLK_F15: /* ? */ break;

		case SDLK_NUMLOCK: v = MKC_formac_ForwardDel; break;
		case SDLK_CAPSLOCK: v = MKC_formac_CapsLock; break;
		case SDLK_SCROLLOCK: v = MKC_ScrollLock; break;
		case SDLK_RSHIFT: v = MKC_formac_RShift; break;
		case SDLK_LSHIFT: v = MKC_formac_Shift; break;
		case SDLK_RCTRL: v = MKC_formac_RControl; break;
		case SDLK_LCTRL: v = MKC_formac_Control; break;
		case SDLK_RALT: v = MKC_formac_RCommand; break;
		case SDLK_LALT: v = MKC_formac_Command; break;
		case SDLK_RMETA: v = MKC_formac_RCommand; break;
		case SDLK_LMETA: v = MKC_formac_Command; break;
		case SDLK_LSUPER: v = MKC_formac_Option; break;
		case SDLK_RSUPER: v = MKC_formac_ROption; break;

		case SDLK_MODE: /* ? */ break;
		case SDLK_COMPOSE: /* ? */ break;

		case SDLK_HELP: v = MKC_formac_Help; break;
		case SDLK_PRINT: v = MKC_Print; break;

		case SDLK_SYSREQ: /* ? */ break;
		case SDLK_BREAK: /* ? */ break;
		case SDLK_MENU: /* ? */ break;
		case SDLK_POWER: /* ? */ break;
		case SDLK_EURO: /* ? */ break;
		case SDLK_UNDO: /* ? */ break;

		default:
			break;
	}

	return v;
}

LOCALPROC DoKeyCode(SDL_keysym *r, bool down)
{
	uint8_t v = SDLKey2MacKeyCode(r->sym);
	if (MKC_None != v) {
		Keyboard_UpdateKeyMap2(v, down);
	}
}

LOCALPROC DisableKeyRepeat(void)
{
}

LOCALPROC RestoreKeyRepeat(void)
{
}

LOCALPROC ReconnectKeyCodes3(void)
{
}

LOCALPROC DisconnectKeyCodes3(void)
{
	DisconnectKeyCodes2();
	MouseButtonSet(false);
}

/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

LOCALVAR uint32_t TrueEmulatedTime = 0;

#define InvTimeDivPow 16
#define InvTimeDiv (1 << InvTimeDivPow)
#define InvTimeDivMask (InvTimeDiv - 1)
#define InvTimeStep 1089590 /* 1000 / 60.14742 * InvTimeDiv */

LOCALVAR Uint32 LastTime;

LOCALVAR Uint32 NextIntTime;
LOCALVAR uint32_t NextFracTime;

LOCALPROC IncrNextTime(void)
{
	NextFracTime += InvTimeStep;
	NextIntTime += (NextFracTime >> InvTimeDivPow);
	NextFracTime &= InvTimeDivMask;
}

LOCALPROC InitNextTime(void)
{
	NextIntTime = LastTime;
	NextFracTime = 0;
	IncrNextTime();
}

LOCALVAR uint32_t NewMacDateInSeconds;

LOCALFUNC bool UpdateTrueEmulatedTime(void)
{
	Uint32 LatestTime;
	int32_t TimeDiff;

	LatestTime = SDL_GetTicks();
	if (LatestTime != LastTime) {

		NewMacDateInSeconds = LatestTime / 1000;
			/* no date and time api in SDL */

		LastTime = LatestTime;
		TimeDiff = (LatestTime - NextIntTime);
			/* this should work even when time wraps */
		if (TimeDiff >= 0) {
			if (TimeDiff > 256) {
				/* emulation interrupted, forget it */
				++TrueEmulatedTime;
				InitNextTime();

#if dbglog_TimeStuff
				dbglog_writelnNum("emulation interrupted",
					TrueEmulatedTime);
#endif
			} else {
				do {
					++TrueEmulatedTime;
					IncrNextTime();
					TimeDiff = (LatestTime - NextIntTime);
				} while (TimeDiff >= 0);
			}
			return true;
		} else {
			if (TimeDiff < -256) {
#if dbglog_TimeStuff
				dbglog_writeln("clock set back");
#endif
				/* clock goofed if ever get here, reset */
				InitNextTime();
			}
		}
	}
	return false;
}


LOCALFUNC bool CheckDateTime(void)
{
	if (CurMacDateInSeconds != NewMacDateInSeconds) {
		CurMacDateInSeconds = NewMacDateInSeconds;
		return true;
	} else {
		return false;
	}
}

LOCALPROC StartUpTimeAdjust(void)
{
	LastTime = SDL_GetTicks();
	InitNextTime();
}

LOCALFUNC bool InitLocationDat(void)
{
	LastTime = SDL_GetTicks();
	InitNextTime();
	NewMacDateInSeconds = LastTime / 1000;
	CurMacDateInSeconds = NewMacDateInSeconds;

	return true;
}

/* --- sound --- */

#if SoundEnabled

#define kLn2SoundBuffers 4 /* kSoundBuffers must be a power of two */
#define kSoundBuffers (1 << kLn2SoundBuffers)
#define kSoundBuffMask (kSoundBuffers - 1)

#define DesiredMinFilledSoundBuffs 3
	/*
		if too big then sound lags behind emulation.
		if too small then sound will have pauses.
	*/

#define kLnOneBuffLen 9
#define kLnAllBuffLen (kLn2SoundBuffers + kLnOneBuffLen)
#define kOneBuffLen (1UL << kLnOneBuffLen)
#define kAllBuffLen (1UL << kLnAllBuffLen)
#define kLnOneBuffSz (kLnOneBuffLen + kLn2SoundSampSz - 3)
#define kLnAllBuffSz (kLnAllBuffLen + kLn2SoundSampSz - 3)
#define kOneBuffSz (1UL << kLnOneBuffSz)
#define kAllBuffSz (1UL << kLnAllBuffSz)
#define kOneBuffMask (kOneBuffLen - 1)
#define kAllBuffMask (kAllBuffLen - 1)
#define dbhBufferSize (kAllBuffSz + kOneBuffSz)

#define dbglog_SoundStuff (0 && dbglog_HAVE)
#define dbglog_SoundBuffStats (0 && dbglog_HAVE)

LOCALVAR tpSoundSamp TheSoundBuffer = nullpr;
volatile static uint16_t ThePlayOffset;
volatile static uint16_t TheFillOffset;
volatile static uint16_t MinFilledSoundBuffs;
#if dbglog_SoundBuffStats
LOCALVAR uint16_t MaxFilledSoundBuffs;
#endif
LOCALVAR uint16_t TheWriteOffset;

LOCALPROC Sound_Init0(void)
{
	ThePlayOffset = 0;
	TheFillOffset = 0;
	TheWriteOffset = 0;
}

LOCALPROC Sound_Start0(void)
{
	/* Reset variables */
	MinFilledSoundBuffs = kSoundBuffers + 1;
#if dbglog_SoundBuffStats
	MaxFilledSoundBuffs = 0;
#endif
}

GLOBALOSGLUFUNC tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL)
{
	uint16_t ToFillLen = kAllBuffLen - (TheWriteOffset - ThePlayOffset);
	uint16_t WriteBuffContig =
		kOneBuffLen - (TheWriteOffset & kOneBuffMask);

	if (WriteBuffContig < n) {
		n = WriteBuffContig;
	}
	if (ToFillLen < n) {
		/* overwrite previous buffer */
#if dbglog_SoundStuff
		dbglog_writeln("sound buffer over flow");
#endif
		TheWriteOffset -= kOneBuffLen;
	}

	*actL = n;
	return TheSoundBuffer + (TheWriteOffset & kAllBuffMask);
}

#if 4 == kLn2SoundSampSz
LOCALPROC ConvertSoundBlockToNative(tpSoundSamp p)
{
	int i;

	for (i = kOneBuffLen; --i >= 0; ) {
		*p++ -= 0x8000;
	}
}
#else
#define ConvertSoundBlockToNative(p)
#endif

LOCALPROC Sound_WroteABlock(void)
{
#if (4 == kLn2SoundSampSz)
	uint16_t PrevWriteOffset = TheWriteOffset - kOneBuffLen;
	tpSoundSamp p = TheSoundBuffer + (PrevWriteOffset & kAllBuffMask);
#endif

#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_WroteABlock");
#endif

	ConvertSoundBlockToNative(p);

	TheFillOffset = TheWriteOffset;

#if dbglog_SoundBuffStats
	{
		uint16_t ToPlayLen = TheFillOffset
			- ThePlayOffset;
		uint16_t ToPlayBuffs = ToPlayLen >> kLnOneBuffLen;

		if (ToPlayBuffs > MaxFilledSoundBuffs) {
			MaxFilledSoundBuffs = ToPlayBuffs;
		}
	}
#endif
}

LOCALFUNC bool Sound_EndWrite0(uint16_t actL)
{
	bool v;

	TheWriteOffset += actL;

	if (0 != (TheWriteOffset & kOneBuffMask)) {
		v = false;
	} else {
		/* just finished a block */

		Sound_WroteABlock();

		v = true;
	}

	return v;
}

LOCALPROC Sound_SecondNotify0(void)
{
	if (MinFilledSoundBuffs <= kSoundBuffers) {
		if (MinFilledSoundBuffs > DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too high");
#endif
			IncrNextTime();
		} else if (MinFilledSoundBuffs < DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too low");
#endif
			++TrueEmulatedTime;
		}
#if dbglog_SoundBuffStats
		dbglog_writelnNum("MinFilledSoundBuffs",
			MinFilledSoundBuffs);
		dbglog_writelnNum("MaxFilledSoundBuffs",
			MaxFilledSoundBuffs);
		MaxFilledSoundBuffs = 0;
#endif
		MinFilledSoundBuffs = kSoundBuffers + 1;
	}
}

typedef uint16_t trSoundTemp;

#define kCenterTempSound 0x8000

#define AudioStepVal 0x0040

#if 3 == kLn2SoundSampSz
#define ConvertTempSoundSampleFromNative(v) ((v) << 8)
#elif 4 == kLn2SoundSampSz
#define ConvertTempSoundSampleFromNative(v) ((v) + kCenterSound)
#else
#error "unsupported kLn2SoundSampSz"
#endif

#if 3 == kLn2SoundSampSz
#define ConvertTempSoundSampleToNative(v) ((v) >> 8)
#elif 4 == kLn2SoundSampSz
#define ConvertTempSoundSampleToNative(v) ((v) - kCenterSound)
#else
#error "unsupported kLn2SoundSampSz"
#endif

LOCALPROC SoundRampTo(trSoundTemp *last_val, trSoundTemp dst_val,
	tpSoundSamp *stream, int *len)
{
	trSoundTemp diff;
	tpSoundSamp p = *stream;
	int n = *len;
	trSoundTemp v1 = *last_val;

	while ((v1 != dst_val) && (0 != n)) {
		if (v1 > dst_val) {
			diff = v1 - dst_val;
			if (diff > AudioStepVal) {
				v1 -= AudioStepVal;
			} else {
				v1 = dst_val;
			}
		} else {
			diff = dst_val - v1;
			if (diff > AudioStepVal) {
				v1 += AudioStepVal;
			} else {
				v1 = dst_val;
			}
		}

		--n;
		*p++ = ConvertTempSoundSampleToNative(v1);
	}

	*stream = p;
	*len = n;
	*last_val = v1;
}

struct SoundR {
	tpSoundSamp fTheSoundBuffer;
	volatile uint16_t (*fPlayOffset);
	volatile uint16_t (*fFillOffset);
	volatile uint16_t (*fMinFilledSoundBuffs);

	volatile trSoundTemp lastv;

	bool wantplaying;
	bool HaveStartedPlaying;
};
typedef struct SoundR SoundR;

static void audio_callback(void *udata, Uint8 *stream, int len)
{
	uint16_t ToPlayLen;
	uint16_t FilledSoundBuffs;
	int i;
	SoundR *datp = (SoundR *)udata;
	tpSoundSamp CurSoundBuffer = datp->fTheSoundBuffer;
	uint16_t CurPlayOffset = *datp->fPlayOffset;
	trSoundTemp v0 = datp->lastv;
	trSoundTemp v1 = v0;
	tpSoundSamp dst = (tpSoundSamp)stream;

#if kLn2SoundSampSz > 3
	len >>= (kLn2SoundSampSz - 3);
#endif

#if dbglog_SoundStuff
	dbglog_writeln("Enter audio_callback");
	dbglog_writelnNum("len", len);
#endif

label_retry:
	ToPlayLen = *datp->fFillOffset - CurPlayOffset;
	FilledSoundBuffs = ToPlayLen >> kLnOneBuffLen;

	if (! datp->wantplaying) {
#if dbglog_SoundStuff
		dbglog_writeln("playing end transistion");
#endif

		SoundRampTo(&v1, kCenterTempSound, &dst, &len);

		ToPlayLen = 0;
	} else if (! datp->HaveStartedPlaying) {
#if dbglog_SoundStuff
		dbglog_writeln("playing start block");
#endif

		if ((ToPlayLen >> kLnOneBuffLen) < 8) {
			ToPlayLen = 0;
		} else {
			tpSoundSamp p = datp->fTheSoundBuffer
				+ (CurPlayOffset & kAllBuffMask);
			trSoundTemp v2 = ConvertTempSoundSampleFromNative(*p);

#if dbglog_SoundStuff
			dbglog_writeln("have enough samples to start");
#endif

			SoundRampTo(&v1, v2, &dst, &len);

			if (v1 == v2) {
#if dbglog_SoundStuff
				dbglog_writeln("finished start transition");
#endif

				datp->HaveStartedPlaying = true;
			}
		}
	}

	if (0 == len) {
		/* done */

		if (FilledSoundBuffs < *datp->fMinFilledSoundBuffs) {
			*datp->fMinFilledSoundBuffs = FilledSoundBuffs;
		}
	} else if (0 == ToPlayLen) {

#if dbglog_SoundStuff
		dbglog_writeln("under run");
#endif

		for (i = 0; i < len; ++i) {
			*dst++ = ConvertTempSoundSampleToNative(v1);
		}
		*datp->fMinFilledSoundBuffs = 0;
	} else {
		uint16_t PlayBuffContig = kAllBuffLen
			- (CurPlayOffset & kAllBuffMask);
		tpSoundSamp p = CurSoundBuffer
			+ (CurPlayOffset & kAllBuffMask);

		if (ToPlayLen > PlayBuffContig) {
			ToPlayLen = PlayBuffContig;
		}
		if (ToPlayLen > len) {
			ToPlayLen = len;
		}

		for (i = 0; i < ToPlayLen; ++i) {
			*dst++ = *p++;
		}
		v1 = ConvertTempSoundSampleFromNative(p[-1]);

		CurPlayOffset += ToPlayLen;
		len -= ToPlayLen;

		*datp->fPlayOffset = CurPlayOffset;

		goto label_retry;
	}

	datp->lastv = v1;
}

LOCALVAR SoundR cur_audio;

LOCALVAR bool HaveSoundOut = false;

LOCALPROC Sound_Stop(void)
{
#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_Stop");
#endif

	if (cur_audio.wantplaying && HaveSoundOut) {
		uint16_t retry_limit = 50; /* half of a second */

		cur_audio.wantplaying = false;

label_retry:
		if (kCenterTempSound == cur_audio.lastv) {
#if dbglog_SoundStuff
			dbglog_writeln("reached kCenterTempSound");
#endif

			/* done */
		} else if (0 == --retry_limit) {
#if dbglog_SoundStuff
			dbglog_writeln("retry limit reached");
#endif
			/* done */
		} else
		{
			/*
				give time back, particularly important
				if got here on a suspend event.
			*/

#if dbglog_SoundStuff
			dbglog_writeln("busy, so sleep");
#endif

			(void) SDL_Delay(10);

			goto label_retry;
		}

		SDL_PauseAudio(1);
	}

#if dbglog_SoundStuff
	dbglog_writeln("leave Sound_Stop");
#endif
}

LOCALPROC Sound_Start(void)
{
	if ((! cur_audio.wantplaying) && HaveSoundOut) {
		Sound_Start0();
		cur_audio.lastv = kCenterTempSound;
		cur_audio.HaveStartedPlaying = false;
		cur_audio.wantplaying = true;

		SDL_PauseAudio(0);
	}
}

LOCALPROC Sound_UnInit(void)
{
	if (HaveSoundOut) {
		SDL_CloseAudio();
	}
}

#define SOUND_SAMPLERATE 22255 /* = round(7833600 * 2 / 704) */

LOCALFUNC bool Sound_Init(void)
{
	SDL_AudioSpec desired;

	Sound_Init0();

	cur_audio.fTheSoundBuffer = TheSoundBuffer;
	cur_audio.fPlayOffset = &ThePlayOffset;
	cur_audio.fFillOffset = &TheFillOffset;
	cur_audio.fMinFilledSoundBuffs = &MinFilledSoundBuffs;
	cur_audio.wantplaying = false;

	desired.freq = SOUND_SAMPLERATE;

#if 3 == kLn2SoundSampSz
	desired.format = AUDIO_U8;
#elif 4 == kLn2SoundSampSz
	desired.format = AUDIO_S16SYS;
#else
#error "unsupported audio format"
#endif

	desired.channels = 1;
	desired.samples = 1024;
	desired.callback = audio_callback;
	desired.userdata = (void *)&cur_audio;

	/* Open the audio device */
	if (SDL_OpenAudio(&desired, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
	} else {
		HaveSoundOut = true;

		Sound_Start();
			/*
				This should be taken care of by LeaveSpeedStopped,
				but since takes a while to get going properly,
				start early.
			*/
	}

	return true; /* keep going, even if no sound */
}

GLOBALOSGLUPROC Sound_EndWrite(uint16_t actL)
{
	if (Sound_EndWrite0(actL)) {
	}
}

LOCALPROC Sound_SecondNotify(void)
{
	if (HaveSoundOut) {
		Sound_SecondNotify0();
	}
}

#endif

/* --- basic dialogs --- */

LOCALPROC CheckSavedMacMsg(void)
{
	/* called only on quit, if error saved but not yet reported */

	if (nullpr != SavedBriefMsg) {
		char briefMsg0[ClStrMaxLength + 1];
		char longMsg0[ClStrMaxLength + 1];

		NativeStrFromCStr(briefMsg0, SavedBriefMsg);
		NativeStrFromCStr(longMsg0, SavedLongMsg);

		fprintf(stderr, "%s\n", briefMsg0);
		fprintf(stderr, "%s\n", longMsg0);

		SavedBriefMsg = nullpr;
	}
}

/* --- clipboard --- */

#define UseMotionEvents 1

#if UseMotionEvents
LOCALVAR bool CaughtMouse = false;
#endif

/* --- event handling for main window --- */

LOCALPROC HandleTheEvent(SDL_Event *event)
{
	switch (event->type) {
		case SDL_QUIT:
			RequestMacOff = true;
			break;
		case SDL_ACTIVEEVENT:
			switch (event->active.state) {
				case SDL_APPINPUTFOCUS:
					gTrueBackgroundFlag = (0 == event->active.gain);
#if 0 && UseMotionEvents
					if (! gTrueBackgroundFlag) {
						CheckMouseState();
					}
#endif
					break;
				case SDL_APPMOUSEFOCUS:
					CaughtMouse = (0 != event->active.gain);
					break;
			}
			break;
		case SDL_MOUSEMOTION:
			MousePositionNotify(
				event->motion.x, event->motion.y);
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* any mouse button, we don't care which */
			MousePositionNotify(
				event->button.x, event->button.y);
			MouseButtonSet(true);
			break;
		case SDL_MOUSEBUTTONUP:
			MousePositionNotify(
				event->button.x, event->button.y);
			MouseButtonSet(false);
			break;
		case SDL_KEYDOWN:
			DoKeyCode(&event->key.keysym, true);
			break;
		case SDL_KEYUP:
			DoKeyCode(&event->key.keysym, false);
			break;
#if 0
		case Expose: /* SDL doesn't have an expose event */
			int x0 = event->expose.x;
			int y0 = event->expose.y;
			int x1 = x0 + event->expose.width;
			int y1 = y0 + event->expose.height;

			if (x0 < 0) {
				x0 = 0;
			}
			if (x1 > vMacScreenWidth) {
				x1 = vMacScreenWidth;
			}
			if (y0 < 0) {
				y0 = 0;
			}
			if (y1 > vMacScreenHeight) {
				y1 = vMacScreenHeight;
			}
			if ((x0 < x1) && (y0 < y1)) {
				HaveChangedScreenBuff(y0, x0, y1, x1);
			}
			break;
#endif
	}
}

/* --- main window creation and disposal --- */

LOCALVAR int argc;
LOCALVAR char **argv;

LOCALFUNC bool Screen_Init(void)
{
	bool v = false;

	InitKeyCodes();

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	} else {
		SDL_WM_SetCaption(kStrAppName, NULL);
		v = true;
	}

	return v;
}

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if MayFullScreen
LOCALPROC GrabTheMachine(void)
{
#if GrabKeysFullScreen
	(void) SDL_WM_GrabInput(SDL_GRAB_ON);
#endif

#if EnableFSMouseMotion
	/*
		if magnification changes, need to reset,
		even if HaveMouseMotion already true
	*/
	if (MoveMouse(ViewHStart + (ViewHSize / 2),
		ViewVStart + (ViewVSize / 2)))
	{
		SavedMouseH = ViewHStart + (ViewHSize / 2);
		SavedMouseV = ViewVStart + (ViewVSize / 2);
		HaveMouseMotion = true;
	}
#endif
}
#endif

#if MayFullScreen
LOCALPROC UngrabMachine(void)
{
#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		(void) MoveMouse(CurMouseH, CurMouseV);
		HaveMouseMotion = false;
	}
#endif

#if GrabKeysFullScreen
	(void) SDL_WM_GrabInput(SDL_GRAB_OFF);
#endif
}
#endif

#if EnableFSMouseMotion
LOCALPROC MouseConstrain(void)
{
	int16_t shiftdh;
	int16_t shiftdv;

	if (SavedMouseH < ViewHStart + (ViewHSize / 4)) {
		shiftdh = ViewHSize / 2;
	} else if (SavedMouseH > ViewHStart + ViewHSize - (ViewHSize / 4)) {
		shiftdh = - ViewHSize / 2;
	} else {
		shiftdh = 0;
	}
	if (SavedMouseV < ViewVStart + (ViewVSize / 4)) {
		shiftdv = ViewVSize / 2;
	} else if (SavedMouseV > ViewVStart + ViewVSize - (ViewVSize / 4)) {
		shiftdv = - ViewVSize / 2;
	} else {
		shiftdv = 0;
	}
	if ((shiftdh != 0) || (shiftdv != 0)) {
		SavedMouseH += shiftdh;
		SavedMouseV += shiftdv;
		if (! MoveMouse(SavedMouseH, SavedMouseV)) {
			HaveMouseMotion = false;
		}
	}
}
#endif

LOCALFUNC bool CreateMainWindow(void)
{
	int NewWindowHeight = vMacScreenHeight;
	int NewWindowWidth = vMacScreenWidth;
	Uint32 flags = SDL_SWSURFACE;
	bool v = false;

#if EnableMagnify && 1
	if (UseMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		flags |= SDL_FULLSCREEN;
	}
#endif

	ViewHStart = 0;
	ViewVStart = 0;
	ViewHSize = vMacScreenWidth;
	ViewVSize = vMacScreenHeight;

	surface = SDL_SetVideoMode(NewWindowWidth, NewWindowHeight,
#if 0 != vMacScreenDepth
		32,
#else
		/* 32 */ /* 24 */ /* 16 */ 8,
#endif
		flags);
	if (NULL == surface) {
		fprintf(stderr, "SDL_SetVideoMode fails: %s\n",
			SDL_GetError());
	} else {
#if 0 != vMacScreenDepth
		ColorModeWorks = true;
#endif
		v = true;
	}

	return v;
}

#if EnableRecreateW
LOCALFUNC bool ReCreateMainWindow(void)
{
	ForceShowCursor(); /* hide/show cursor api is per window */

#if MayFullScreen
	if (GrabMachine) {
		GrabMachine = false;
		UngrabMachine();
	}
#endif

#if EnableMagnify
	UseMagnify = WantMagnify;
#endif
#if VarFullScreen
	UseFullScreen = WantFullScreen;
#endif

	(void) CreateMainWindow();

	if (HaveCursorHidden) {
		(void) MoveMouse(CurMouseH, CurMouseV);
	}

	return true;
}
#endif

LOCALPROC ZapWinStateVars(void)
{
}

#if VarFullScreen
LOCALPROC ToggleWantFullScreen(void)
{
	WantFullScreen = ! WantFullScreen;
}
#endif

/* --- SavedTasks --- */

LOCALPROC LeaveBackground(void)
{
	ReconnectKeyCodes3();
	DisableKeyRepeat();
}

LOCALPROC EnterBackground(void)
{
	RestoreKeyRepeat();
	DisconnectKeyCodes3();

	ForceShowCursor();
}

LOCALPROC LeaveSpeedStopped(void)
{
#if SoundEnabled
	Sound_Start();
#endif

	StartUpTimeAdjust();
}

LOCALPROC EnterSpeedStopped(void)
{
#if SoundEnabled
	Sound_Stop();
#endif
}

LOCALPROC CheckForSavedTasks(void)
{
	if (EvtQNeedRecover) {
		EvtQNeedRecover = false;

		/* attempt cleanup, EvtQNeedRecover may get set again */
		EvtQTryRecoverFromFull();
	}

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MouseConstrain();
	}
#endif

	if (RequestMacOff) {
		RequestMacOff = false;
		if (AnyDiskInserted()) {
			MacMsgOverride(kStrQuitWarningTitle,
				kStrQuitWarningMessage);
		} else {
			ForceMacOff = true;
		}
	}

	if (ForceMacOff) {
		return;
	}

	if (gTrueBackgroundFlag != gBackgroundFlag) {
		gBackgroundFlag = gTrueBackgroundFlag;
		if (gTrueBackgroundFlag) {
			EnterBackground();
		} else {
			LeaveBackground();
		}
	}

	if (CurSpeedStopped != (SpeedStopped ||
		(gBackgroundFlag && ! RunInBackground
#if EnableAutoSlow && 0
			&& (QuietSubTicks >= 4092)
#endif
		)))
	{
		CurSpeedStopped = ! CurSpeedStopped;
		if (CurSpeedStopped) {
			EnterSpeedStopped();
		} else {
			LeaveSpeedStopped();
		}
	}

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

#if EnableRecreateW
	if (0
#if EnableMagnify
		|| (UseMagnify != WantMagnify)
#endif
#if VarFullScreen
		|| (UseFullScreen != WantFullScreen)
#endif
		)
	{
		(void) ReCreateMainWindow();
	}
#endif

#if MayFullScreen
	if (GrabMachine != (
#if VarFullScreen
		UseFullScreen &&
#endif
		! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		GrabMachine = ! GrabMachine;
		if (GrabMachine) {
			GrabTheMachine();
		} else {
			UngrabMachine();
		}
	}
#endif

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}

#if NeedRequestIthDisk
	if (0 != RequestIthDisk) {
		Sony_InsertIth(RequestIthDisk);
		RequestIthDisk = 0;
	}
#endif

	if (HaveCursorHidden != (WantCursorHidden
		&& ! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		(void) SDL_ShowCursor(
			HaveCursorHidden ? SDL_DISABLE : SDL_ENABLE);
	}
}

/* --- command line parsing --- */

LOCALFUNC bool ScanCommandLine(void)
{
	char *pa;
	int i = 1;

label_retry:
	if (i < argc) {
		pa = argv[i++];
		if ('-' == pa[0]) {
			if ((0 == strcmp(pa, "--rom"))
				|| (0 == strcmp(pa, "-r")))
			{
				if (i < argc) {
					rom_path = argv[i++];
					goto label_retry;
				}
			} else
			{
				MacMsg(kStrBadArgTitle, kStrBadArgMessage, false);
			}
		} else {
			(void) Sony_Insert1(pa, false);
			goto label_retry;
		}
	}

	return true;
}

/* --- main program flow --- */

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

LOCALPROC WaitForTheNextEvent(void)
{
	SDL_Event event;

	if (SDL_WaitEvent(&event)) {
		HandleTheEvent(&event);
	}
}

LOCALPROC CheckForSystemEvents(void)
{
	SDL_Event event;
	int i = 10;

	while ((--i >= 0) && SDL_PollEvent(&event)) {
		HandleTheEvent(&event);
	}
}

GLOBALOSGLUPROC WaitForNextTick(void)
{
label_retry:
	CheckForSystemEvents();
	CheckForSavedTasks();

	if (ForceMacOff) {
		return;
	}

	if (CurSpeedStopped) {
		DoneWithDrawingForTick();
		WaitForTheNextEvent();
		goto label_retry;
	}

	if (ExtraTimeNotOver()) {
		(void) SDL_Delay(NextIntTime - LastTime);
		goto label_retry;
	}

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	if ((! gBackgroundFlag)
#if UseMotionEvents
		&& (! CaughtMouse)
#endif
		)
	{
		CheckMouseState();
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif
}

/* --- platform independent code can be thought of as going here --- */

#include "PROGMAIN.h"

LOCALPROC ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
}

LOCALPROC ReserveAllocAll(void)
{
#if dbglog_HAVE
	dbglog_ReserveAlloc();
#endif
	ReserveAllocOneBlock(&ROM, kROM_Size, 5, false);

	ReserveAllocOneBlock(&screencomparebuff,
		vMacScreenNumBytes, 5, true);
#if UseControlKeys
	ReserveAllocOneBlock(&CntrlDisplayBuff,
		vMacScreenNumBytes, 5, false);
#endif

	ReserveAllocOneBlock(&CLUT_final, CLUT_finalsz, 5, false);
#if SoundEnabled
	ReserveAllocOneBlock((uint8_t * *)&TheSoundBuffer,
		dbhBufferSize, 5, false);
#endif

	EmulationReserveAlloc();
}

LOCALFUNC bool AllocMemory(void)
{
	uimr n;
	bool IsOk = false;

	ReserveAllocOffset = 0;
	ReserveAllocBigBlock = nullpr;
	ReserveAllocAll();
	n = ReserveAllocOffset;
	ReserveAllocBigBlock = (uint8_t *)calloc(1, n);
	if (NULL == ReserveAllocBigBlock) {
		MacMsg(kStrOutOfMemTitle, kStrOutOfMemMessage, true);
	} else {
		ReserveAllocOffset = 0;
		ReserveAllocAll();
		if (n != ReserveAllocOffset) {
			/* oops, program error */
		} else {
			IsOk = true;
		}
	}

	return IsOk;
}

LOCALPROC UnallocMemory(void)
{
	if (nullpr != ReserveAllocBigBlock) {
		free((char *)ReserveAllocBigBlock);
	}
}

LOCALFUNC bool InitOSGLU(void)
{
	if (AllocMemory())
#if dbglog_HAVE
	if (dbglog_open())
#endif
	if (ScanCommandLine())
	if (LoadMacRom())
	if (LoadInitialImages())
	if (InitLocationDat())
#if SoundEnabled
	if (Sound_Init())
#endif
	if (Screen_Init())
	if (CreateMainWindow())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

LOCALPROC UnInitOSGLU(void)
{
	if (MacMsgDisplayed) {
		MacMsgDisplayOff();
	}

	RestoreKeyRepeat();
#if MayFullScreen
	UngrabMachine();
#endif
#if SoundEnabled
	Sound_Stop();
#endif
#if SoundEnabled
	Sound_UnInit();
#endif
#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	ForceShowCursor();

#if dbglog_HAVE
	dbglog_close();
#endif

	UnallocMemory();

	CheckSavedMacMsg();

	SDL_Quit();
}

int main(int argc, char **argv)
{
	argc = argc;
	argv = argv;

	ZapOSGLUVars();
	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

	return 0;
}
