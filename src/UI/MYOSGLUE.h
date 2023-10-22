/*
	MYOSGLUE.h

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

/*
	MY Operating System GLUE.

	header file for operating system dependent code.
	the same header is used for all platforms.

	This code is descended from Richard F. Bannister's Macintosh
	port of vMac, by Philip Cummins.
*/

#ifndef MYOSGLUE_H
#define MYOSGLUE_H

#include "UTIL/DATE2SEC.h"
#include "HW/KBRD/KEYCODES.h"
#include "ERRCODES.h"

#if WantAbnormalReports
EXPORTOSGLUPROC WarnMsgAbnormalID(uint16_t id);
#endif

#if dbglog_HAVE
EXPORTOSGLUPROC dbglog_writeCStr(char *s);
EXPORTOSGLUPROC dbglog_writeReturn(void);
EXPORTOSGLUPROC dbglog_writeHex(uint32_t x);
EXPORTOSGLUPROC dbglog_writeNum(uint32_t x);
EXPORTOSGLUPROC dbglog_writeMacChar(uint8_t x);
EXPORTOSGLUPROC dbglog_writeln(char *s);
EXPORTOSGLUPROC dbglog_writelnNum(char *s, simr v);
#endif

EXPORTOSGLUPROC ReserveAllocOneBlock(
	uint8_t * *p, uimr n, uint8_t align, bool FillOnes
);
EXPORTOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount);

extern uint8_t * ROM;

#if IncludePbufs

#define tPbuf uint16_t

#define NotAPbuf ((tPbuf)0xFFFF)

EXPORTOSGLUFUNC MacErr_t CheckPbuf(tPbuf Pbuf_No);
EXPORTOSGLUFUNC MacErr_t PbufGetSize(tPbuf Pbuf_No, uint32_t *Count);

EXPORTOSGLUFUNC MacErr_t PbufNew(uint32_t count, tPbuf *r);
EXPORTOSGLUPROC PbufDispose(tPbuf i);
EXPORTOSGLUPROC PbufTransfer(uint8_t * Buffer,
	tPbuf i, uint32_t offset, uint32_t count, bool IsWrite);

#endif

#define tDrive uint16_t

extern uint32_t vSonyWritableMask;
extern uint32_t vSonyInsertedMask;

#define vSonyIsInserted(Drive_No) \
	((vSonyInsertedMask & ((uint32_t)1 << (Drive_No))) != 0)

EXPORTOSGLUFUNC MacErr_t vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount);
EXPORTOSGLUFUNC MacErr_t vSonyEject(tDrive Drive_No);
EXPORTOSGLUFUNC MacErr_t vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count);

EXPORTOSGLUFUNC bool AnyDiskInserted(void);
EXPORTOSGLUPROC DiskRevokeWritable(tDrive Drive_No);

#if IncludeSonyRawMode
extern bool vSonyRawMode;
#endif

#if IncludeSonyNew
extern bool vSonyNewDiskWanted;
extern uint32_t vSonyNewDiskSize;
EXPORTOSGLUFUNC MacErr_t vSonyEjectDelete(tDrive Drive_No);
#endif

#if IncludeSonyNameNew
extern tPbuf vSonyNewDiskName;
#endif

#if IncludeSonyGetName
EXPORTOSGLUFUNC MacErr_t vSonyGetName(tDrive Drive_No, tPbuf *r);
#endif

#if IncludeHostTextClipExchange
EXPORTOSGLUFUNC MacErr_t HTCEexport(tPbuf i);
EXPORTOSGLUFUNC MacErr_t HTCEimport(tPbuf *r);
#endif

extern uint32_t OnTrueTime;

extern uint32_t CurMacDateInSeconds;
#if AutoLocation
extern uint32_t CurMacLatitude;
extern uint32_t CurMacLongitude;
#endif
#if AutoTimeZone
extern uint32_t CurMacDelta;
	/* (dlsDelta << 24) | (gmtDelta & 0x00FFFFFF) */
#endif


extern bool UseColorMode;
extern bool ColorModeWorks;

extern bool ColorMappingChanged;

//#define CLUT_size (1 << (1 << vMacScreenDepth))
#define CLUT_size 256 // total guesstimate

extern uint16_t CLUT_reds[CLUT_size];
extern uint16_t CLUT_greens[CLUT_size];
extern uint16_t CLUT_blues[CLUT_size];

extern bool EmVideoDisable;
extern int8_t EmLagTime;

EXPORTOSGLUPROC Screen_OutputFrame(uint8_t * screencurrentbuff);
EXPORTOSGLUPROC DoneWithDrawingForTick(void);

extern bool ForceMacOff;

extern bool WantMacInterrupt;

extern bool WantMacReset;

EXPORTOSGLUFUNC bool ExtraTimeNotOver(void);

extern uint8_t SpeedValue;

/* where emulated machine thinks mouse is */
extern uint16_t CurMouseV;
extern uint16_t CurMouseH;

extern uint32_t QuietTime;
extern uint32_t QuietSubTicks;

#define QuietEnds() \
{ \
	QuietTime = 0; \
	QuietSubTicks = 0; \
}

#if 3 == kLn2SoundSampSz
#define trSoundSamp uint8_t
#define tbSoundSamp uint8_t
#define tpSoundSamp uint8_t *
#define kCenterSound 0x80
#elif 4 == kLn2SoundSampSz
#define trSoundSamp uint16_t
#define tbSoundSamp uint16_t
#define tpSoundSamp uint16_t *
#define kCenterSound 0x8000
#else
#error "unsupported kLn2SoundSampSz"
#endif

#if SoundEnabled

EXPORTOSGLUFUNC tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL);
EXPORTOSGLUPROC Sound_EndWrite(uint16_t actL);

/* 370 samples per tick = 22,254.54 per second */
#endif

#if EmLocalTalk

#define LT_TxBfMxSz 1024
extern uint8_t * LT_TxBuffer;
extern uint16_t LT_TxBuffSz;

EXPORTOSGLUPROC LT_TransmitPacket(void);

extern uint8_t * LT_RxBuffer;
extern uint32_t LT_RxBuffSz;

EXPORTOSGLUPROC LT_ReceivePacket(void);

#endif

EXPORTOSGLUPROC WaitForNextTick(void);

typedef enum EvtQKind {
	EvtQElKindKey,
	EvtQElKindMouseButton,
	EvtQElKindMousePos,
	EvtQElKindMouseDelta
} EvtQKind_t;

struct EvtQEl {
	/* expected size : 8 bytes */
	EvtQKind_t kind;
	union {
		struct {
			uint8_t down;
			uint8_t key;
		} press;
		struct {
			uint16_t h;
			uint16_t v;
		} pos;
	} u;
};
typedef struct EvtQEl EvtQEl;

EXPORTOSGLUFUNC EvtQEl * EvtQOutP(void);
EXPORTOSGLUPROC EvtQOutDone(void);

/*** Might be SDL2-specific? ***/
// INTL.c
void NativeStrFromCStr(char *r, char *s);
// DRIVES.c
void InitDrives();
bool Sony_Insert1a(char *drivepath, bool silentfail);
bool LoadInitialImages();
void UnInitDrives();
// MOUSE.c
void ForceShowCursor();
void CheckMouseState();
// KEYBOARD.c
void DisconnectKeyCodes3();
void ReconnectKeyCodes3();
void DisableKeyRepeat();
void RestoreKeyRepeat();
// SOUND.c
void Sound_Start();
void Sound_Stop();
void Sound_SecondNotify();
bool Sound_Init();
void Sound_UnInit();
// TIMEDATE.c
void StartUpTimeAdjust();
bool UpdateTrueEmulatedTime();
bool CheckDateTime();
bool InitLocationDat();
void IncrNextTime(void);
// ROM.c
bool LoadMacRom();
MacErr_t LoadMacRomFrom(char *path);
// OSGLUSD2.c
void EnterSpeedStopped();
void LeaveSpeedStopped();

#endif
