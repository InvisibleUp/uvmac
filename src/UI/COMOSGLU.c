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

 uint32_t vSonyWritableMask = 0;
 uint32_t vSonyInsertedMask = 0;

#if IncludeSonyRawMode
 bool vSonyRawMode = false;
#endif

#if IncludeSonyNew
 bool vSonyNewDiskWanted = false;
 uint32_t vSonyNewDiskSize;
#endif

#if IncludeSonyNameNew
 tPbuf vSonyNewDiskName = NotAPbuf;
#endif

 uint32_t CurMacDateInSeconds = 0;
#if AutoLocation
 uint32_t CurMacLatitude = 0;
 uint32_t CurMacLongitude = 0;
#endif
#if AutoTimeZone
 uint32_t CurMacDelta = 0;
#endif

 bool UseColorMode = false;
 bool ColorModeWorks = false;

 bool ColorMappingChanged = false;

 uint16_t CLUT_reds[CLUT_size];
 uint16_t CLUT_greens[CLUT_size];
 uint16_t CLUT_blues[CLUT_size];

 bool RequestMacOff = false;
 bool ForceMacOff = false;
 bool WantMacInterrupt = false;
 bool WantMacReset = false;

 uint8_t SpeedValue = WantInitSpeedValue;
 uint16_t CurMouseV = 0;
 uint16_t CurMouseH = 0;

 uint32_t QuietTime = 0;
 uint32_t QuietSubTicks = 0;

#if IncludePbufs
 bool FirstFreePbuf(tPbuf *r)
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

void PbufNewNotify(tPbuf Pbuf_No, uint32_t count)
{
	PbufSize[Pbuf_No] = count;
	PbufAllocatedMask |= ((uint32_t)1 << Pbuf_No);
}

void PbufDisposeNotify(tPbuf Pbuf_No)
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

 bool FirstFreeDisk(tDrive *Drive_No)
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

void DiskInsertNotify(tDrive Drive_No, bool locked)
{
	vSonyInsertedMask |= ((uint32_t)1 << Drive_No);
	if (! locked) {
		vSonyWritableMask |= ((uint32_t)1 << Drive_No);
	}

	QuietEnds();
}

void DiskEjectedNotify(tDrive Drive_No)
{
	vSonyWritableMask &= ~ ((uint32_t)1 << Drive_No);
	vSonyInsertedMask &= ~ ((uint32_t)1 << Drive_No);
}

 uint8_t * screencomparebuff = nullpr;

#if WantColorTransValid
static bool ColorTransValid = false;
#endif

 bool EmVideoDisable = false;
 int8_t EmLagTime = 0;
 uint32_t OnTrueTime = 0;

void ScreenClearChanges(void)
{
	ScreenChangedTop = vMacScreenHeight;
	ScreenChangedBottom = 0;
	ScreenChangedLeft = vMacScreenWidth;
	ScreenChangedRight = 0;
}

void ScreenChangedAll(void)
{
	ScreenChangedTop = 0;
	ScreenChangedBottom = vMacScreenHeight;
	ScreenChangedLeft = 0;
	ScreenChangedRight = vMacScreenWidth;
}

#if MayFullScreen
 uint16_t ViewHSize;
 uint16_t ViewVSize;
 uint16_t ViewHStart = 0;
 uint16_t ViewVStart = 0;
#endif

static void SetLongs(uint32_t *p, long n)
{
	long i;

	for (i = n; --i >= 0; ) {
		*p++ = (uint32_t) -1;
	}
}

 uimr ReserveAllocOffset;
 uint8_t * ReserveAllocBigBlock = nullpr;

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

static uimr dbglog_bufpos = 0;
static char *dbglog_bufp = nullpr;

static void dbglog_ReserveAlloc(void)
{
	ReserveAllocOneBlock((uint8_t * *)&dbglog_bufp, dbglog_bufsz,
		5, false);
}

static void dbglog_close(void)
{
	uimr n = ModPow2(dbglog_bufpos, dbglog_buflnsz);
	if (n != 0) {
		dbglog_write0(dbglog_bufp, n);
	}

	dbglog_close0();
}

static void dbglog_write(char *p, uimr L)
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

static uimr CStrLength(char *s)
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

static void dbglog_writeSpace(void)
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

static EvtQEl EvtQA[EvtQSz];
static uint16_t EvtQIn = 0;
static uint16_t EvtQOut = 0;

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

 bool EvtQNeedRecover = false;
	/* events lost because of full queue */

static EvtQEl * EvtQElPreviousIn(void)
{
	EvtQEl *p = NULL;
	if (EvtQIn - EvtQOut != 0) {
		p = &EvtQA[(EvtQIn - 1) & EvtQIMask];
	}

	return p;
}

static EvtQEl * EvtQElAlloc(void)
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

static uint32_t theKeys[4];

void Keyboard_UpdateKeyMap(uint8_t key, bool down)
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

static bool MouseButtonState = false;

void MouseButtonSet(bool down)
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

static uint16_t MousePosCurV = 0;
static uint16_t MousePosCurH = 0;

void MousePositionSet(uint16_t h, uint16_t v)
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

void InitKeyCodes(void)
{
	theKeys[0] = 0;
	theKeys[1] = 0;
	theKeys[2] = 0;
	theKeys[3] = 0;
}

void DisconnectKeyCodes(uint32_t KeepMask)
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

void EvtQTryRecoverFromFull(void)
{
	MouseButtonSet(false);
	DisconnectKeyCodes(0);
}

/* MacMsg */

 char *SavedBriefMsg = nullpr;
 char *SavedLongMsg = nullpr;
#if WantAbnormalReports
 uint16_t SavedIDMsg = 0;
#endif
 bool SavedFatalMsg = false;

void MacMsg(char *briefMsg, char *longMsg, bool fatal)
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
