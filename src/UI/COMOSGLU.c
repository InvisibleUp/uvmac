/*
	COMOSGLU.h

	Copyright (C) 2009 Paul C. Pratt

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
	COMmon code for Operating System GLUe
*/

#include <stdlib.h>
#include "SYSDEPNS.h"
#include "GLOBGLUE.h"
#include "MYOSGLUE.h"
#include "CNFGRAPI.h"
#include "COMOSGLU.h"
#include "HW/SCREEN/SCRNEMDV.h"

GLOBALVAR uint8_t * ROM = nullpr;
GLOBALVAR bool ROM_loaded = false;

GLOBALVAR uint32_t vSonyWritableMask = 0;
GLOBALVAR uint32_t vSonyInsertedMask = 0;

#if IncludeSonyRawMode
GLOBALVAR bool vSonyRawMode = false;
#endif

#if IncludeSonyNew
GLOBALVAR bool vSonyNewDiskWanted = false;
GLOBALVAR uint32_t vSonyNewDiskSize;
#endif

#if IncludeSonyNameNew
GLOBALVAR tPbuf vSonyNewDiskName = NotAPbuf;
#endif

GLOBALVAR uint32_t CurMacDateInSeconds = 0;
#if AutoLocation
GLOBALVAR uint32_t CurMacLatitude = 0;
GLOBALVAR uint32_t CurMacLongitude = 0;
#endif
#if AutoTimeZone
GLOBALVAR uint32_t CurMacDelta = 0;
#endif

GLOBALVAR bool UseColorMode = false;
GLOBALVAR bool ColorModeWorks = false;

GLOBALVAR bool ColorMappingChanged = false;

GLOBALVAR uint16_t CLUT_reds[CLUT_size];
GLOBALVAR uint16_t CLUT_greens[CLUT_size];
GLOBALVAR uint16_t CLUT_blues[CLUT_size];

GLOBALVAR bool RequestMacOff = false;
GLOBALVAR bool ForceMacOff = false;
GLOBALVAR bool WantMacInterrupt = false;
GLOBALVAR bool WantMacReset = false;

GLOBALVAR uint8_t SpeedValue = WantInitSpeedValue;
GLOBALVAR uint16_t CurMouseV = 0;
GLOBALVAR uint16_t CurMouseH = 0;

GLOBALVAR uint32_t QuietTime = 0;
GLOBALVAR uint32_t QuietSubTicks = 0;

#if IncludePbufs
GLOBALFUNC bool FirstFreePbuf(tPbuf *r)
{
	tPbuf i;

	for (i = 0; i < NumPbufs; ++i) {
		if (! PbufIsAllocated(i)) {
			*r = i;
			return true;
		}
	}
	return false;
}

GLOBALPROC PbufNewNotify(tPbuf Pbuf_No, uint32_t count)
{
	PbufSize[Pbuf_No] = count;
	PbufAllocatedMask |= ((uint32_t)1 << Pbuf_No);
}

GLOBALPROC PbufDisposeNotify(tPbuf Pbuf_No)
{
	PbufAllocatedMask &= ~ ((uint32_t)1 << Pbuf_No);
}

GLOBALOSGLUFUNC MacErr_t CheckPbuf(tPbuf Pbuf_No)
{
	MacErr_t result;

	if (Pbuf_No >= NumPbufs) {
		result = mnvm_nsDrvErr;
	} else if (! PbufIsAllocated(Pbuf_No)) {
		result = mnvm_offLinErr;
	} else {
		result = mnvm_noErr;
	}

	return result;
}

GLOBALOSGLUFUNC MacErr_t PbufGetSize(tPbuf Pbuf_No, uint32_t *Count)
{
	MacErr_t result = CheckPbuf(Pbuf_No);

	if (mnvm_noErr == result) {
		*Count = PbufSize[Pbuf_No];
	}

	return result;
}
#endif

GLOBALFUNC bool FirstFreeDisk(tDrive *Drive_No)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (! vSonyIsInserted(i)) {
			if (nullpr != Drive_No) {
				*Drive_No = i;
			}
			return true;
		}
	}
	return false;
}

GLOBALOSGLUFUNC bool AnyDiskInserted(void)
{
	return 0 != vSonyInsertedMask;
}

GLOBALOSGLUPROC DiskRevokeWritable(tDrive Drive_No)
{
	vSonyWritableMask &= ~ ((uint32_t)1 << Drive_No);
}

GLOBALPROC DiskInsertNotify(tDrive Drive_No, bool locked)
{
	vSonyInsertedMask |= ((uint32_t)1 << Drive_No);
	if (! locked) {
		vSonyWritableMask |= ((uint32_t)1 << Drive_No);
	}

	QuietEnds();
}

GLOBALPROC DiskEjectedNotify(tDrive Drive_No)
{
	vSonyWritableMask &= ~ ((uint32_t)1 << Drive_No);
	vSonyInsertedMask &= ~ ((uint32_t)1 << Drive_No);
}

LOCALFUNC bool FindFirstChangeInLVecs(uibb *ptr1, uibb *ptr2,
					uimr L, uimr *j)
{
/*
	find index of first difference
*/
	uibb *p1 = ptr1;
	uibb *p2 = ptr2;
	uimr i;

	for (i = L; i != 0; --i) {
		if (*p1++ != *p2++) {
			--p1;
			*j = p1 - ptr1;
			return true;
		}
	}
	return false;
}

LOCALPROC FindLastChangeInLVecs(uibb *ptr1, uibb *ptr2,
					uimr L, uimr *j)
{
/*
	find index of last difference, assuming there is one
*/
	uibb *p1 = ptr1 + L;
	uibb *p2 = ptr2 + L;

	while (*--p1 == *--p2) {
	}
	*j = p1 - ptr1;
}

LOCALPROC FindLeftRightChangeInLMat(uibb *ptr1, uibb *ptr2,
	uimr width, uimr top, uimr bottom,
	uimr *LeftMin0, uibr *LeftMask0,
	uimr *RightMax0, uibr *RightMask0)
{
	uimr i;
	uimr j;
	uibb *p1;
	uibb *p2;
	uibr x;
	uint32_t offset = top * width;
	uibb *p10 = (uibb *)ptr1 + offset;
	uibb *p20 = (uibb *)ptr2 + offset;
	uimr LeftMin = *LeftMin0;
	uimr RightMax = *RightMax0;
	uibr LeftMask = 0;
	uibr RightMask = 0;
	for (i = top; i < bottom; ++i) {
		p1 = p10;
		p2 = p20;
		for (j = 0; j < LeftMin; ++j) {
			x = *p1++ ^ *p2++;
			if (0 != x) {
				LeftMin = j;
				LeftMask = x;
				goto Label_3;
			}
		}
		LeftMask |= (*p1 ^ *p2);
Label_3:
		p1 = p10 + RightMax;
		p2 = p20 + RightMax;
		RightMask |= (*p1++ ^ *p2++);
		for (j = RightMax + 1; j < width; ++j) {
			x = *p1++ ^ *p2++;
			if (0 != x) {
				RightMax = j;
				RightMask = x;
			}
		}

		p10 += width;
		p20 += width;
	}
	*LeftMin0 = LeftMin;
	*RightMax0 = RightMax;
	*LeftMask0 = LeftMask;
	*RightMask0 = RightMask;
}

GLOBALVAR uint8_t * screencomparebuff = nullpr;
LOCALVAR uimr NextDrawRow = 0;

#if WantColorTransValid
LOCALVAR bool ColorTransValid = false;
#endif

LOCALFUNC bool ScreenFindChanges(uint8_t * screencurrentbuff,
	int8_t TimeAdjust, int16_t *top, int16_t *left, int16_t *bottom, int16_t *right)
{
	uimr j0;
	uimr j1;
	uimr j0h;
	uimr j1h;
	uimr j0v;
	uimr j1v;
	uimr copysize;
	uimr copyoffset;
	uimr copyrows;
	uimr LimitDrawRow;
	uimr MaxRowsDrawnPerTick;
	uimr LeftMin;
	uimr RightMax;
	uibr LeftMask;
	uibr RightMask;
	int j;

	if (TimeAdjust < 4) {
		MaxRowsDrawnPerTick = vMacScreenHeight;
	} else if (TimeAdjust < 6) {
		MaxRowsDrawnPerTick = vMacScreenHeight / 2;
	} else {
		MaxRowsDrawnPerTick = vMacScreenHeight / 4;
	}

	if (UseColorMode && vMacScreenDepth > 0) {
		if (ColorMappingChanged) {
			ColorMappingChanged = false;
			j0h = 0;
			j1h = vMacScreenWidth;
			j0v = 0;
			j1v = vMacScreenHeight;
#if WantColorTransValid
			ColorTransValid = false;
#endif
		} else {
			if (! FindFirstChangeInLVecs(
				(uibb *)screencurrentbuff
					+ NextDrawRow * (vMacScreenBitWidth / uiblockbitsn),
				(uibb *)screencomparebuff
					+ NextDrawRow * (vMacScreenBitWidth / uiblockbitsn),
				((uimr)(vMacScreenHeight - NextDrawRow)
					* (uimr)vMacScreenBitWidth) / uiblockbitsn,
				&j0))
			{
				NextDrawRow = 0;
				return false;
			}
			j0v = j0 / (vMacScreenBitWidth / uiblockbitsn);
			j0h = j0 - j0v * (vMacScreenBitWidth / uiblockbitsn);
			j0v += NextDrawRow;
			LimitDrawRow = j0v + MaxRowsDrawnPerTick;
			if (LimitDrawRow >= vMacScreenHeight) {
				LimitDrawRow = vMacScreenHeight;
				NextDrawRow = 0;
			} else {
				NextDrawRow = LimitDrawRow;
			}
			FindLastChangeInLVecs((uibb *)screencurrentbuff,
				(uibb *)screencomparebuff,
				((uimr)LimitDrawRow
					* (uimr)vMacScreenBitWidth) / uiblockbitsn,
				&j1);
			j1v = j1 / (vMacScreenBitWidth / uiblockbitsn);
			j1h = j1 - j1v * (vMacScreenBitWidth / uiblockbitsn);
			j1v++;

			if (j0h < j1h) {
				LeftMin = j0h;
				RightMax = j1h;
			} else {
				LeftMin = j1h;
				RightMax = j0h;
			}

			FindLeftRightChangeInLMat((uibb *)screencurrentbuff,
				(uibb *)screencomparebuff,
				(vMacScreenBitWidth / uiblockbitsn),
				j0v, j1v, &LeftMin, &LeftMask, &RightMax, &RightMask);

			if (vMacScreenDepth > ln2uiblockbitsn) {
				j0h =  (LeftMin >> (vMacScreenDepth - ln2uiblockbitsn));
			} else if (ln2uiblockbitsn > vMacScreenDepth) {
				for (j = 0; j < (1 << (ln2uiblockbitsn - vMacScreenDepth));
					++j)
				{
					if (0 != (LeftMask
						& (((((uibr)1) << (1 << vMacScreenDepth)) - 1)
							<< ((j ^ FlipCheckBits) << vMacScreenDepth))))
					{
						goto Label_1c;
					}
				}
	Label_1c:
				j0h =  (LeftMin << (ln2uiblockbitsn - vMacScreenDepth)) + j;
			} else {
				j0h =  LeftMin;
			}

			if (vMacScreenDepth > ln2uiblockbitsn) {
				j1h = (RightMax >> (vMacScreenDepth - ln2uiblockbitsn)) + 1;
			} else if (ln2uiblockbitsn > vMacScreenDepth) {
				for (j = (uiblockbitsn >> vMacScreenDepth); --j >= 0; ) {
					if (0 != (RightMask
						& (((((uibr)1) << (1 << vMacScreenDepth)) - 1)
							<< ((j ^ FlipCheckBits) << vMacScreenDepth))))
					{
						goto Label_2c;
					}
				}
	Label_2c:
				j1h = (RightMax << (ln2uiblockbitsn - vMacScreenDepth)) + j + 1;
			} else {
				j1h = RightMax + 1;
			}
		}

		copyrows = j1v - j0v;
		copyoffset = j0v * vMacScreenByteWidth;
		copysize = copyrows * vMacScreenByteWidth;
	} else {
		if (vMacScreenDepth > 0 && ColorMappingChanged) {
			ColorMappingChanged = false;
			j0h = 0;
			j1h = vMacScreenWidth;
			j0v = 0;
			j1v = vMacScreenHeight;
#if WantColorTransValid
			ColorTransValid = false;
#endif
		} else {
			if (! FindFirstChangeInLVecs(
				(uibb *)screencurrentbuff
					+ NextDrawRow * (vMacScreenWidth / uiblockbitsn),
				(uibb *)screencomparebuff
					+ NextDrawRow * (vMacScreenWidth / uiblockbitsn),
				((uimr)(vMacScreenHeight - NextDrawRow)
					* (uimr)vMacScreenWidth) / uiblockbitsn,
				&j0))
			{
				NextDrawRow = 0;
				return false;
			}
			j0v = j0 / (vMacScreenWidth / uiblockbitsn);
			j0h = j0 - j0v * (vMacScreenWidth / uiblockbitsn);
			j0v += NextDrawRow;
			LimitDrawRow = j0v + MaxRowsDrawnPerTick;
			if (LimitDrawRow >= vMacScreenHeight) {
				LimitDrawRow = vMacScreenHeight;
				NextDrawRow = 0;
			} else {
				NextDrawRow = LimitDrawRow;
			}
			FindLastChangeInLVecs((uibb *)screencurrentbuff,
				(uibb *)screencomparebuff,
				((uimr)LimitDrawRow
					* (uimr)vMacScreenWidth) / uiblockbitsn,
				&j1);
			j1v = j1 / (vMacScreenWidth / uiblockbitsn);
			j1h = j1 - j1v * (vMacScreenWidth / uiblockbitsn);
			j1v++;

			if (j0h < j1h) {
				LeftMin = j0h;
				RightMax = j1h;
			} else {
				LeftMin = j1h;
				RightMax = j0h;
			}

			FindLeftRightChangeInLMat((uibb *)screencurrentbuff,
				(uibb *)screencomparebuff,
				(vMacScreenWidth / uiblockbitsn),
				j0v, j1v, &LeftMin, &LeftMask, &RightMax, &RightMask);

			for (j = 0; j < uiblockbitsn; ++j) {
				if (0 != (LeftMask
					& (((uibr)1) << (j ^ FlipCheckMonoBits))))
				{
					goto Label_1;
				}
			}
Label_1:
			j0h = LeftMin * uiblockbitsn + j;

			for (j = uiblockbitsn; --j >= 0; ) {
				if (0 != (RightMask
					& (((uibr)1) << (j ^ FlipCheckMonoBits))))
				{
					goto Label_2;
				}
			}
Label_2:
			j1h = RightMax * uiblockbitsn + j + 1;
		}

		copyrows = j1v - j0v;
		copyoffset = j0v * vMacScreenMonoByteWidth;
		copysize = copyrows * vMacScreenMonoByteWidth;
	}

	MoveBytes((anyp)screencurrentbuff + copyoffset,
		(anyp)screencomparebuff + copyoffset,
		copysize);

	*top = j0v;
	*left = j0h;
	*bottom = j1v;
	*right = j1h;

	return true;
}

GLOBALVAR bool EmVideoDisable = false;
GLOBALVAR int8_t EmLagTime = 0;
GLOBALVAR uint32_t OnTrueTime = 0;

GLOBALPROC ScreenClearChanges(void)
{
	ScreenChangedTop = vMacScreenHeight;
	ScreenChangedBottom = 0;
	ScreenChangedLeft = vMacScreenWidth;
	ScreenChangedRight = 0;
}

GLOBALPROC ScreenChangedAll(void)
{
	ScreenChangedTop = 0;
	ScreenChangedBottom = vMacScreenHeight;
	ScreenChangedLeft = 0;
	ScreenChangedRight = vMacScreenWidth;
}

#if MayFullScreen
GLOBALVAR uint16_t ViewHSize;
GLOBALVAR uint16_t ViewVSize;
GLOBALVAR uint16_t ViewHStart = 0;
GLOBALVAR uint16_t ViewVStart = 0;
#endif

LOCALPROC SetLongs(uint32_t *p, long n)
{
	long i;

	for (i = n; --i >= 0; ) {
		*p++ = (uint32_t) -1;
	}
}

GLOBALVAR uimr ReserveAllocOffset;
GLOBALVAR uint8_t * ReserveAllocBigBlock = nullpr;

GLOBALOSGLUPROC ReserveAllocOneBlock(uint8_t * *p, uimr n,
	uint8_t align, bool FillOnes)
{
	ReserveAllocOffset = CeilPow2Mult(ReserveAllocOffset, align);
	if (nullpr == ReserveAllocBigBlock) {
		*p = nullpr;
	} else {
		*p = ReserveAllocBigBlock + ReserveAllocOffset;
		if (FillOnes) {
			SetLongs((uint32_t *)*p, n / 4);
		}
	}
	ReserveAllocOffset += n;
}

/* --- sending debugging info to file --- */

#if dbglog_HAVE

LOCALVAR uimr dbglog_bufpos = 0;
LOCALVAR char *dbglog_bufp = nullpr;

LOCALPROC dbglog_ReserveAlloc(void)
{
	ReserveAllocOneBlock((uint8_t * *)&dbglog_bufp, dbglog_bufsz,
		5, false);
}

LOCALPROC dbglog_close(void)
{
	uimr n = ModPow2(dbglog_bufpos, dbglog_buflnsz);
	if (n != 0) {
		dbglog_write0(dbglog_bufp, n);
	}

	dbglog_close0();
}

LOCALPROC dbglog_write(char *p, uimr L)
{
	uimr r;
	uimr bufposmod;
	uimr curbufdiv;
	uimr newbufpos = dbglog_bufpos + L;
	uimr newbufdiv = FloorDivPow2(newbufpos, dbglog_buflnsz);

label_retry:
	curbufdiv = FloorDivPow2(dbglog_bufpos, dbglog_buflnsz);
	bufposmod = ModPow2(dbglog_bufpos, dbglog_buflnsz);
	if (newbufdiv != curbufdiv) {
		r = dbglog_bufsz - bufposmod;
		MoveBytes((anyp)p, (anyp)(dbglog_bufp + bufposmod), r);
		dbglog_write0(dbglog_bufp, dbglog_bufsz);
		L -= r;
		p += r;
		dbglog_bufpos += r;
		goto label_retry;
	}
	MoveBytes((anyp)p, (anyp)dbglog_bufp + bufposmod, L);
	dbglog_bufpos = newbufpos;
}

LOCALFUNC uimr CStrLength(char *s)
{
	char *p = s;

	while (*p++ != 0) {
	}
	return p - s - 1;
}

GLOBALOSGLUPROC dbglog_writeCStr(char *s)
{
	/* fprintf(DumpFile, "%s", s); */
	dbglog_write(s, CStrLength(s));
}

GLOBALOSGLUPROC dbglog_writeReturn(void)
{
	dbglog_writeCStr("\n");
	/* fprintf(DumpFile, "\n"); */
}

GLOBALOSGLUPROC dbglog_writeHex(uimr x)
{
	uint8_t v;
	char s[16];
	char *p = s + 16;
	uimr n = 0;

	do {
		v = x & 0x0F;
		if (v < 10) {
			*--p = '0' + v;
		} else {
			*--p = 'A' + v - 10;
		}
		x >>= 4;
		++n;
	} while (x != 0);

	dbglog_write(p, n);
	/* fprintf(DumpFile, "%d", (int)x); */
}

GLOBALOSGLUPROC dbglog_writeNum(uimr x)
{
	uimr newx;
	char s[16];
	char *p = s + 16;
	uimr n = 0;

	do {
		newx = x / (uimr)10;
		*--p = '0' + (x - newx * 10);
		x = newx;
		++n;
	} while (x != 0);

	dbglog_write(p, n);
	/* fprintf(DumpFile, "%d", (int)x); */
}

GLOBALOSGLUPROC dbglog_writeMacChar(uint8_t x)
{
	char s;

	if ((x > 32) && (x < 127)) {
		s = x;
	} else {
		s = '?';
	}

	dbglog_write(&s, 1);
}

LOCALPROC dbglog_writeSpace(void)
{
	dbglog_writeCStr(" ");
}

GLOBALOSGLUPROC dbglog_writeln(char *s)
{
	dbglog_writeCStr(s);
	dbglog_writeReturn();
}

GLOBALOSGLUPROC dbglog_writelnNum(char *s, simr v)
{
	dbglog_writeCStr(s);
	dbglog_writeSpace();
	dbglog_writeNum(v);
	dbglog_writeReturn();
}

#endif

/* my event queue */

LOCALVAR EvtQEl EvtQA[EvtQSz];
LOCALVAR uint16_t EvtQIn = 0;
LOCALVAR uint16_t EvtQOut = 0;

GLOBALOSGLUFUNC EvtQEl * EvtQOutP(void)
{
	EvtQEl *p = nullpr;
	if (EvtQIn != EvtQOut) {
		p = &EvtQA[EvtQOut & EvtQIMask];
	}
	return p;
}

GLOBALOSGLUPROC EvtQOutDone(void)
{
	++EvtQOut;
}

GLOBALVAR bool EvtQNeedRecover = false;
	/* events lost because of full queue */

LOCALFUNC EvtQEl * EvtQElPreviousIn(void)
{
	EvtQEl *p = NULL;
	if (EvtQIn - EvtQOut != 0) {
		p = &EvtQA[(EvtQIn - 1) & EvtQIMask];
	}

	return p;
}

LOCALFUNC EvtQEl * EvtQElAlloc(void)
{
	EvtQEl *p = NULL;
	if (EvtQIn - EvtQOut >= EvtQSz) {
		EvtQNeedRecover = true;
	} else {
		p = &EvtQA[EvtQIn & EvtQIMask];

		++EvtQIn;
	}

	return p;
}

LOCALVAR uint32_t theKeys[4];

GLOBALPROC Keyboard_UpdateKeyMap(uint8_t key, bool down)
{
	uint8_t k = key & 127; /* just for safety */
	uint8_t bit = 1 << (k & 7);
	uint8_t *kp = (uint8_t *)theKeys;
	uint8_t *kpi = &kp[k / 8];
	bool CurDown = ((*kpi & bit) != 0);
	if (CurDown != down) {
		EvtQEl *p = EvtQElAlloc();
		if (NULL != p) {
			p->kind = EvtQElKindKey;
			p->u.press.key = k;
			p->u.press.down = down;

			if (down) {
				*kpi |= bit;
			} else {
				*kpi &= ~ bit;
			}
		}

		QuietEnds();
	}
}

LOCALVAR bool MouseButtonState = false;

GLOBALPROC MouseButtonSet(bool down)
{
	if (MouseButtonState != down) {
		EvtQEl *p = EvtQElAlloc();
		if (NULL != p) {
			p->kind = EvtQElKindMouseButton;
			p->u.press.down = down;

			MouseButtonState = down;
		}

		QuietEnds();
	}
}

LOCALVAR uint16_t MousePosCurV = 0;
LOCALVAR uint16_t MousePosCurH = 0;

GLOBALPROC MousePositionSet(uint16_t h, uint16_t v)
{
	if ((h != MousePosCurH) || (v != MousePosCurV)) {
		EvtQEl *p = EvtQElPreviousIn();
		if ((NULL == p) || (EvtQElKindMousePos != p->kind)) {
			p = EvtQElAlloc();
		}
		if (NULL != p) {
			p->kind = EvtQElKindMousePos;
			p->u.pos.h = h;
			p->u.pos.v = v;

			MousePosCurH = h;
			MousePosCurV = v;
		}

		QuietEnds();
	}
}

GLOBALPROC InitKeyCodes(void)
{
	theKeys[0] = 0;
	theKeys[1] = 0;
	theKeys[2] = 0;
	theKeys[3] = 0;
}

GLOBALPROC DisconnectKeyCodes(uint32_t KeepMask)
{
	/*
		Called when may miss key ups,
		so act is if all pressed keys have been released,
		except maybe for control, caps lock, command,
		option and shift.
	*/

	int j;
	int b;
	int key;
	uint32_t m;

	for (j = 0; j < 16; ++j) {
		uint8_t k1 = ((uint8_t *)theKeys)[j];
		if (0 != k1) {
			uint8_t bit = 1;
			for (b = 0; b < 8; ++b) {
				if (0 != (k1 & bit)) {
					key = j * 8 + b;
					switch (key) {
						case MKC_Control: m = kKeepMaskControl; break;
						case MKC_CapsLock: m = kKeepMaskCapsLock; break;
						case MKC_Command: m = kKeepMaskCommand; break;
						case MKC_Option: m = kKeepMaskOption; break;
						case MKC_Shift: m = kKeepMaskShift; break;
						default: m = 0; break;
					}
					if (0 == (KeepMask & m)) {
						Keyboard_UpdateKeyMap(key, false);
					}
				}
				bit <<= 1;
			}
		}
	}
}

GLOBALPROC EvtQTryRecoverFromFull(void)
{
	MouseButtonSet(false);
	DisconnectKeyCodes(0);
}

/* MacMsg */

GLOBALVAR char *SavedBriefMsg = nullpr;
GLOBALVAR char *SavedLongMsg = nullpr;
#if WantAbnormalReports
GLOBALVAR uint16_t SavedIDMsg = 0;
#endif
GLOBALVAR bool SavedFatalMsg = false;

GLOBALPROC MacMsg(char *briefMsg, char *longMsg, bool fatal)
{
	if (nullpr != SavedBriefMsg) {
		/*
			ignore the new message, only display the
			first error.
		*/
	} else {
		SavedBriefMsg = briefMsg;
		SavedLongMsg = longMsg;
		SavedFatalMsg = fatal;
	}
}

#if WantAbnormalReports
GLOBALOSGLUPROC WarnMsgAbnormalID(uint16_t id)
{
	MacMsg(kStrReportAbnormalTitle,
		kStrReportAbnormalMessage, false);

	if (0 != SavedIDMsg) {
		/*
			ignore the new message, only display the
			first error.
		*/
	} else {
		SavedIDMsg = id;
	}
}
#endif
