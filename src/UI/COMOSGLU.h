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

#ifndef COMOSGLU_H
#define COMOSGLU_H

#include "SYSDEPNS.h"
#include "CNFGRAPI.h"
#include "GLOBGLUE.h"
#include "MYOSGLUE.h"
#define EnableRecreateW 1

extern GLOBALVAR uint8_t * ROM;
extern GLOBALVAR bool ROM_loaded;

extern GLOBALVAR uint32_t vSonyWritableMask;
extern GLOBALVAR uint32_t vSonyInsertedMask;

#if IncludeSonyRawMode
extern GLOBALVAR bool vSonyRawMode;
#endif

#if IncludeSonyNew
extern GLOBALVAR bool vSonyNewDiskWanted;
extern GLOBALVAR uint32_t vSonyNewDiskSize;
#endif

#if IncludeSonyNameNew
extern GLOBALVAR tPbuf vSonyNewDiskName;
#endif

extern GLOBALVAR uint32_t CurMacDateInSeconds;
#if AutoLocation
extern GLOBALVAR uint32_t CurMacLatitude;
extern GLOBALVAR uint32_t CurMacLongitude;
#endif
#if AutoTimeZone
extern GLOBALVAR uint32_t CurMacDelta;
#endif

extern GLOBALVAR bool UseColorMode;
extern GLOBALVAR bool ColorModeWorks;
extern GLOBALVAR bool ColorMappingChanged;

extern GLOBALVAR uint16_t CLUT_reds[CLUT_size];
extern GLOBALVAR uint16_t CLUT_greens[CLUT_size];
extern GLOBALVAR uint16_t CLUT_blues[CLUT_size];

extern GLOBALVAR bool RequestMacOff;
extern GLOBALVAR bool ForceMacOff;
extern GLOBALVAR bool WantMacInterrupt;
extern GLOBALVAR bool WantMacReset;
extern GLOBALVAR uint8_t SpeedValue;
extern GLOBALVAR uint16_t CurMouseV;
extern GLOBALVAR uint16_t CurMouseH;

extern GLOBALVAR uint32_t QuietTime;
extern GLOBALVAR uint32_t QuietSubTicks;

#ifndef GrabKeysFullScreen
#define GrabKeysFullScreen 1
#endif

#ifndef GrabKeysMaxFullScreen
#define GrabKeysMaxFullScreen 0
#endif

#if IncludePbufs
GLOBALVAR uint32_t PbufAllocatedMask;
GLOBALVAR uint32_t PbufSize[NumPbufs];
#define PbufIsAllocated(i) ((PbufAllocatedMask & ((uint32_t)1 << (i))) != 0)

GLOBALFUNC bool FirstFreePbuf(tPbuf *r);
GLOBALPROC PbufNewNotify(tPbuf Pbuf_No, uint32_t count);
GLOBALPROC PbufDisposeNotify(tPbuf Pbuf_No);
GLOBALOSGLUFUNC tMacErr CheckPbuf(tPbuf Pbuf_No);
GLOBALOSGLUFUNC tMacErr PbufGetSize(tPbuf Pbuf_No, uint32_t *Count);
#endif

GLOBALFUNC bool FirstFreeDisk(tDrive *Drive_No);
GLOBALOSGLUFUNC bool AnyDiskInserted(void);
GLOBALOSGLUPROC DiskRevokeWritable(tDrive Drive_No);
GLOBALPROC DiskInsertNotify(tDrive Drive_No, bool locked);
GLOBALPROC DiskEjectedNotify(tDrive Drive_No);

extern GLOBALVAR uint8_t * screencomparebuff;

/*
	block type - for operating on multiple uint8_t elements
		at a time.
*/

#if LittleEndianUnaligned || BigEndianUnaligned

#define uibb uint32_t
#define uibr uint32_t
#define ln2uiblockn 2

#else

#define uibb uint8_t
#define uibr uint8_t
#define ln2uiblockn 0

#endif

#define uiblockn (1 << ln2uiblockn)
#define ln2uiblockbitsn (3 + ln2uiblockn)
#define uiblockbitsn (8 * uiblockn)

#if BigEndianUnaligned
#define FlipCheckMonoBits (uiblockbitsn - 1)
#else
#define FlipCheckMonoBits 7
#endif

#define FlipCheckBits (FlipCheckMonoBits >> vMacScreenDepth)

#ifndef WantColorTransValid
#define WantColorTransValid 0
#endif

extern GLOBALVAR bool EmVideoDisable;
extern GLOBALVAR int8_t EmLagTime;
// The time slice we are currently dealing with,
// in the same units as TrueEmulatedTime.
extern GLOBALVAR uint32_t OnTrueTime;

GLOBALPROC ScreenClearChanges(void);
GLOBALPROC ScreenChangedAll(void);

GLOBALVAR int16_t ScreenChangedTop;
GLOBALVAR int16_t ScreenChangedLeft;
GLOBALVAR int16_t ScreenChangedBottom;
GLOBALVAR int16_t ScreenChangedRight;

GLOBALOSGLUPROC Screen_OutputFrame(uint8_t * screencurrentbuff);

#if MayFullScreen
extern GLOBALVAR uint16_t ViewHSize;
extern GLOBALVAR uint16_t ViewVSize;
extern GLOBALVAR uint16_t ViewHStart;
extern GLOBALVAR uint16_t ViewVStart;
#endif

#ifndef WantAutoScrollBorder
#define WantAutoScrollBorder 0
#endif

#define PowOf2(p) ((uimr)1 << (p))
#define Pow2Mask(p) (PowOf2(p) - 1)
#define ModPow2(i, p) ((i) & Pow2Mask(p))
#define FloorDivPow2(i, p) ((i) >> (p))
#define FloorPow2Mult(i, p) ((i) & (~ Pow2Mask(p)))
#define CeilPow2Mult(i, p) FloorPow2Mult((i) + Pow2Mask(p), (p))
	/* warning - CeilPow2Mult evaluates p twice */

extern GLOBALVAR uimr ReserveAllocOffset;
extern GLOBALVAR uint8_t * ReserveAllocBigBlock;
GLOBALOSGLUPROC ReserveAllocOneBlock(uint8_t * *p, uimr n,
	uint8_t align, bool FillOnes);

/* --- sending debugging info to file --- */

#if dbglog_HAVE
#define dbglog_bufsz PowOf2(dbglog_buflnsz)
#define dbglog_open dbglog_open0

GLOBALOSGLUPROC dbglog_writeCStr(char *s);
GLOBALOSGLUPROC dbglog_writeReturn(void);
GLOBALOSGLUPROC dbglog_writeHex(uimr x);
GLOBALOSGLUPROC dbglog_writeNum(uimr x);
GLOBALOSGLUPROC dbglog_writeMacChar(uint8_t x);
GLOBALOSGLUPROC dbglog_writeln(char *s);
GLOBALOSGLUPROC dbglog_writelnNum(char *s, simr v);
#endif

/* my event queue */

#define EvtQLg2Sz 4
#define EvtQSz (1 << EvtQLg2Sz)
#define EvtQIMask (EvtQSz - 1)

GLOBALOSGLUFUNC EvtQEl * EvtQOutP(void);
GLOBALOSGLUPROC EvtQOutDone(void);
extern GLOBALVAR bool EvtQNeedRecover;

#define kKeepMaskControl  (1 << 0)
#define kKeepMaskCapsLock (1 << 1)
#define kKeepMaskCommand  (1 << 2)
#define kKeepMaskOption   (1 << 3)
#define kKeepMaskShift    (1 << 4)


GLOBALPROC Keyboard_UpdateKeyMap(uint8_t key, bool down);
GLOBALPROC MouseButtonSet(bool down);
GLOBALPROC MousePositionSet(uint16_t h, uint16_t v);
GLOBALPROC InitKeyCodes(void);
GLOBALPROC DisconnectKeyCodes(uint32_t KeepMask);
GLOBALPROC EvtQTryRecoverFromFull(void);

/* MacMsg */

extern GLOBALVAR char *SavedBriefMsg;
extern GLOBALVAR char *SavedLongMsg;
#if WantAbnormalReports
extern GLOBALVAR uint16_t SavedIDMsg;
#endif
extern GLOBALVAR bool SavedFatalMsg;

GLOBALPROC MacMsg(char *briefMsg, char *longMsg, bool fatal);

#if WantAbnormalReports
GLOBALOSGLUPROC WarnMsgAbnormalID(uint16_t id);
#endif

#endif