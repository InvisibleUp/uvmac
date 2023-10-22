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

extern uint32_t vSonyWritableMask;
extern uint32_t vSonyInsertedMask;

#if IncludeSonyRawMode
extern bool vSonyRawMode;
#endif

#if IncludeSonyNew
extern bool vSonyNewDiskWanted;
extern uint32_t vSonyNewDiskSize;
#endif

#if IncludeSonyNameNew
extern tPbuf vSonyNewDiskName;
#endif

extern uint32_t CurMacDateInSeconds;
#if AutoLocation
extern uint32_t CurMacLatitude;
extern uint32_t CurMacLongitude;
#endif
#if AutoTimeZone
extern uint32_t CurMacDelta;
#endif

extern bool UseColorMode;
extern bool ColorModeWorks;
extern bool ColorMappingChanged;

extern uint16_t CLUT_reds[CLUT_size];
extern uint16_t CLUT_greens[CLUT_size];
extern uint16_t CLUT_blues[CLUT_size];

extern bool RequestMacOff;
extern bool ForceMacOff;
extern bool WantMacInterrupt;
extern bool WantMacReset;
extern uint8_t SpeedValue;
extern uint16_t CurMouseV;
extern uint16_t CurMouseH;

extern uint32_t QuietTime;
extern uint32_t QuietSubTicks;

#ifndef GrabKeysFullScreen
#define GrabKeysFullScreen 1
#endif

#ifndef GrabKeysMaxFullScreen
#define GrabKeysMaxFullScreen 0
#endif

#if IncludePbufs
 uint32_t PbufAllocatedMask;
 uint32_t PbufSize[NumPbufs];
#define PbufIsAllocated(i) ((PbufAllocatedMask & ((uint32_t)1 << (i))) != 0)

 bool FirstFreePbuf(tPbuf *r);
void PbufNewNotify(tPbuf Pbuf_No, uint32_t count);
void PbufDisposeNotify(tPbuf Pbuf_No);
GLOBALOSGLUFUNC MacErr_t CheckPbuf(tPbuf Pbuf_No);
GLOBALOSGLUFUNC MacErr_t PbufGetSize(tPbuf Pbuf_No, uint32_t *Count);
#endif

 bool FirstFreeDisk(tDrive *Drive_No);
GLOBALOSGLUFUNC bool AnyDiskInserted(void);
GLOBALOSGLUPROC DiskRevokeWritable(tDrive Drive_No);
void DiskInsertNotify(tDrive Drive_No, bool locked);
void DiskEjectedNotify(tDrive Drive_No);

extern uint8_t * screencomparebuff;

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

extern bool EmVideoDisable;
extern int8_t EmLagTime;
// The time slice we are currently dealing with,
// in the same units as TrueEmulatedTime.
extern uint32_t OnTrueTime;

void ScreenClearChanges(void);
void ScreenChangedAll(void);

 int16_t ScreenChangedTop;
 int16_t ScreenChangedLeft;
 int16_t ScreenChangedBottom;
 int16_t ScreenChangedRight;

GLOBALOSGLUPROC Screen_OutputFrame(uint8_t * screencurrentbuff);

#if MayFullScreen
extern uint16_t ViewHSize;
extern uint16_t ViewVSize;
extern uint16_t ViewHStart;
extern uint16_t ViewVStart;
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

extern uimr ReserveAllocOffset;
extern uint8_t * ReserveAllocBigBlock;
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
extern bool EvtQNeedRecover;

#define kKeepMaskControl  (1 << 0)
#define kKeepMaskCapsLock (1 << 1)
#define kKeepMaskCommand  (1 << 2)
#define kKeepMaskOption   (1 << 3)
#define kKeepMaskShift    (1 << 4)


void Keyboard_UpdateKeyMap(uint8_t key, bool down);
void MouseButtonSet(bool down);
void MousePositionSet(uint16_t h, uint16_t v);
void MousePositionNotify(int h, int v);
void InitKeyCodes(void);
void DisconnectKeyCodes(uint32_t KeepMask);
void EvtQTryRecoverFromFull(void);

/* MacMsg */

extern char *SavedBriefMsg;
extern char *SavedLongMsg;
#if WantAbnormalReports
extern uint16_t SavedIDMsg;
#endif
extern bool SavedFatalMsg;

void MacMsg(char *briefMsg, char *longMsg, bool fatal);

#if WantAbnormalReports
GLOBALOSGLUPROC WarnMsgAbnormalID(uint16_t id);
#endif

#endif
