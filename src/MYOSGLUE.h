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

#ifdef MYOSGLUE_H
#ifndef AllFiles
#error "header already included"
#endif
#else
#define MYOSGLUE_H
#endif


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

EXPORTOSGLUPROC ReserveAllocOneBlock(ui3p *p, uimr n, uint8_t align,
	blnr FillOnes);

EXPORTOSGLUPROC MyMoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount);


EXPORTVAR(ui3p, ROM)

/*
	error codes returned by Mini vMac extensions
	(passed back to the emulated 68k code).
*/

#define tMacErr uint16_t

#define mnvm_noErr      ((tMacErr) 0x0000)
	/* (uint16_t)    0 - No Error */
#define mnvm_miscErr    ((tMacErr) 0xFFFF)
	/* (uint16_t) -  1 - Should probably replace these */
#define mnvm_controlErr ((tMacErr) 0xFFEF)
	/* (uint16_t) - 17 - I/O System Errors */
#define mnvm_statusErr  ((tMacErr) 0xFFEE)
	/* (uint16_t) - 18 - Driver can't respond to Status call */
#define mnvm_closErr    ((tMacErr) 0xFFE8)
	/* (uint16_t) - 24 - I/O System Errors */
#define mnvm_eofErr     ((tMacErr) 0xFFD9)
	/* (uint16_t) - 39 - End of file */
#define mnvm_tmfoErr    ((tMacErr) 0xFFD6)
	/* (uint16_t) - 42 - too many files open */
#define mnvm_fnfErr     ((tMacErr) 0xFFD5)
	/* (uint16_t) - 43 - File not found */
#define mnvm_wPrErr     ((tMacErr) 0xFFD4)
	/* (uint16_t) - 44 - diskette is write protected */
#define mnvm_vLckdErr   ((tMacErr) 0xFFD2)
	/* (uint16_t) - 46 - volume is locked */
#define mnvm_dupFNErr   ((tMacErr) 0xFFD0)
	/* (uint16_t) - 48 - duplicate filename */
#define mnvm_opWrErr    ((tMacErr) 0xFFCF)
	/* (uint16_t) - 49 - file already open with with write permission */
#define mnvm_paramErr   ((tMacErr) 0xFFCE)
	/* (uint16_t) - 50 - error in parameter list */
#define mnvm_permErr    ((tMacErr) 0xFFCA)
	/* (uint16_t) - 54 - permissions error (on file open) */
#define mnvm_nsDrvErr   ((tMacErr) 0xFFC8)
	/* (uint16_t) - 56 - No Such Drive */
#define mnvm_wrPermErr  ((tMacErr) 0xFFC3)
	/* (uint16_t) - 61 - write permissions error */
#define mnvm_offLinErr  ((tMacErr) 0xFFBF)
	/* (uint16_t) - 65 - off-line drive */
#define mnvm_dirNFErr  ((tMacErr) 0xFF88)
	/* (uint16_t) - 120 - directory not found */
#define mnvm_afpAccessDenied  ((tMacErr) 0xEC78)
	/* (uint16_t) - 5000 - Insufficient access privileges for operation */

#if IncludePbufs

#define tPbuf uint16_t

#define NotAPbuf ((tPbuf)0xFFFF)

EXPORTOSGLUFUNC tMacErr CheckPbuf(tPbuf Pbuf_No);
EXPORTOSGLUFUNC tMacErr PbufGetSize(tPbuf Pbuf_No, uint32_t *Count);

EXPORTOSGLUFUNC tMacErr PbufNew(uint32_t count, tPbuf *r);
EXPORTOSGLUPROC PbufDispose(tPbuf i);
EXPORTOSGLUPROC PbufTransfer(ui3p Buffer,
	tPbuf i, uint32_t offset, uint32_t count, blnr IsWrite);

#endif

#define tDrive uint16_t

EXPORTVAR(uint32_t, vSonyWritableMask)
EXPORTVAR(uint32_t, vSonyInsertedMask)

#define vSonyIsInserted(Drive_No) \
	((vSonyInsertedMask & ((uint32_t)1 << (Drive_No))) != 0)

EXPORTOSGLUFUNC tMacErr vSonyTransfer(blnr IsWrite, ui3p Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount);
EXPORTOSGLUFUNC tMacErr vSonyEject(tDrive Drive_No);
EXPORTOSGLUFUNC tMacErr vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count);

EXPORTOSGLUFUNC blnr AnyDiskInserted(void);
EXPORTOSGLUPROC DiskRevokeWritable(tDrive Drive_No);

#if IncludeSonyRawMode
EXPORTVAR(blnr, vSonyRawMode)
#endif

#if IncludeSonyNew
EXPORTVAR(blnr, vSonyNewDiskWanted)
EXPORTVAR(uint32_t, vSonyNewDiskSize)
EXPORTOSGLUFUNC tMacErr vSonyEjectDelete(tDrive Drive_No);
#endif

#if IncludeSonyNameNew
EXPORTVAR(tPbuf, vSonyNewDiskName)
#endif

#if IncludeSonyGetName
EXPORTOSGLUFUNC tMacErr vSonyGetName(tDrive Drive_No, tPbuf *r);
#endif

#if IncludeHostTextClipExchange
EXPORTOSGLUFUNC tMacErr HTCEexport(tPbuf i);
EXPORTOSGLUFUNC tMacErr HTCEimport(tPbuf *r);
#endif

EXPORTVAR(uint32_t, OnTrueTime)

EXPORTVAR(uint32_t, CurMacDateInSeconds)
#if AutoLocation
EXPORTVAR(uint32_t, CurMacLatitude)
EXPORTVAR(uint32_t, CurMacLongitude)
#endif
#if AutoTimeZone
EXPORTVAR(uint32_t, CurMacDelta)
	/* (dlsDelta << 24) | (gmtDelta & 0x00FFFFFF) */
#endif


#define vMacScreenNumPixels \
	((long)vMacScreenHeight * (long)vMacScreenWidth)
#define vMacScreenNumBits (vMacScreenNumPixels << vMacScreenDepth)
#define vMacScreenNumBytes (vMacScreenNumBits / 8)
#define vMacScreenBitWidth ((long)vMacScreenWidth << vMacScreenDepth)
#define vMacScreenByteWidth (vMacScreenBitWidth / 8)

#define vMacScreenMonoNumBytes (vMacScreenNumPixels / 8)
#define vMacScreenMonoByteWidth ((long)vMacScreenWidth / 8)

#if 0 != vMacScreenDepth
EXPORTVAR(blnr, UseColorMode)
EXPORTVAR(blnr, ColorModeWorks)
#endif

#if 0 != vMacScreenDepth
EXPORTVAR(blnr, ColorMappingChanged)
#endif

#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
#define CLUT_size (1 << (1 << vMacScreenDepth))

EXPORTVAR(uint16_t, CLUT_reds[CLUT_size])
EXPORTVAR(uint16_t, CLUT_greens[CLUT_size])
EXPORTVAR(uint16_t, CLUT_blues[CLUT_size])
#endif

EXPORTVAR(blnr, EmVideoDisable)
EXPORTVAR(int8_t, EmLagTime)

EXPORTOSGLUPROC Screen_OutputFrame(ui3p screencurrentbuff);
EXPORTOSGLUPROC DoneWithDrawingForTick(void);

EXPORTVAR(blnr, ForceMacOff)

EXPORTVAR(blnr, WantMacInterrupt)

EXPORTVAR(blnr, WantMacReset)

EXPORTOSGLUFUNC blnr ExtraTimeNotOver(void);

EXPORTVAR(uint8_t, SpeedValue)

#if EnableAutoSlow
EXPORTVAR(blnr, WantNotAutoSlow)
#endif

/* where emulated machine thinks mouse is */
EXPORTVAR(uint16_t, CurMouseV)
EXPORTVAR(uint16_t, CurMouseH)

#if EnableAutoSlow
EXPORTVAR(uint32_t, QuietTime)
EXPORTVAR(uint32_t, QuietSubTicks)

#define QuietEnds() \
{ \
	QuietTime = 0; \
	QuietSubTicks = 0; \
}
#else
#define QuietEnds()
#endif

#if 3 == kLn2SoundSampSz
#define trSoundSamp uint8_t
#define tbSoundSamp uint8_t
#define tpSoundSamp ui3p
#define kCenterSound 0x80
#elif 4 == kLn2SoundSampSz
#define trSoundSamp uint16_t
#define tbSoundSamp uint16_t
#define tpSoundSamp ui4p
#define kCenterSound 0x8000
#else
#error "unsupported kLn2SoundSampSz"
#endif

#if MySoundEnabled

EXPORTOSGLUFUNC tpSoundSamp MySound_BeginWrite(uint16_t n, uint16_t *actL);
EXPORTOSGLUPROC MySound_EndWrite(uint16_t actL);

/* 370 samples per tick = 22,254.54 per second */
#endif

#if EmLocalTalk

#define LT_TxBfMxSz 1024
EXPORTVAR(ui3p, LT_TxBuffer)
EXPORTVAR(uint16_t, LT_TxBuffSz)

EXPORTOSGLUPROC LT_TransmitPacket(void);

EXPORTVAR(ui3p, LT_RxBuffer)
EXPORTVAR(uint32_t, LT_RxBuffSz)

EXPORTOSGLUPROC LT_ReceivePacket(void);

#endif

EXPORTOSGLUPROC WaitForNextTick(void);

#define MyEvtQElKindKey 0
#define MyEvtQElKindMouseButton 1
#define MyEvtQElKindMousePos 2
#define MyEvtQElKindMouseDelta 3

struct MyEvtQEl {
	/* expected size : 8 bytes */
	uint8_t kind;
	uint8_t pad[3];
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
typedef struct MyEvtQEl MyEvtQEl;

EXPORTOSGLUFUNC MyEvtQEl * MyEvtQOutP(void);
EXPORTOSGLUPROC MyEvtQOutDone(void);

#define MKC_A 0x00
#define MKC_B 0x0B
#define MKC_C 0x08
#define MKC_D 0x02
#define MKC_E 0x0E
#define MKC_F 0x03
#define MKC_G 0x05
#define MKC_H 0x04
#define MKC_I 0x22
#define MKC_J 0x26
#define MKC_K 0x28
#define MKC_L 0x25
#define MKC_M 0x2E
#define MKC_N 0x2D
#define MKC_O 0x1F
#define MKC_P 0x23
#define MKC_Q 0x0C
#define MKC_R 0x0F
#define MKC_S 0x01
#define MKC_T 0x11
#define MKC_U 0x20
#define MKC_V 0x09
#define MKC_W 0x0D
#define MKC_X 0x07
#define MKC_Y 0x10
#define MKC_Z 0x06

#define MKC_1 0x12
#define MKC_2 0x13
#define MKC_3 0x14
#define MKC_4 0x15
#define MKC_5 0x17
#define MKC_6 0x16
#define MKC_7 0x1A
#define MKC_8 0x1C
#define MKC_9 0x19
#define MKC_0 0x1D

#define MKC_Command 0x37
#define MKC_Shift 0x38
#define MKC_CapsLock 0x39
#define MKC_Option 0x3A

#define MKC_Space 0x31
#define MKC_Return 0x24
#define MKC_BackSpace 0x33
#define MKC_Tab 0x30

#define MKC_Left /* 0x46 */ 0x7B
#define MKC_Right /* 0x42 */ 0x7C
#define MKC_Down /* 0x48 */ 0x7D
#define MKC_Up /* 0x4D */ 0x7E

#define MKC_Minus 0x1B
#define MKC_Equal 0x18
#define MKC_BackSlash 0x2A
#define MKC_Comma 0x2B
#define MKC_Period 0x2F
#define MKC_Slash 0x2C
#define MKC_SemiColon 0x29
#define MKC_SingleQuote 0x27
#define MKC_LeftBracket 0x21
#define MKC_RightBracket 0x1E
#define MKC_Grave 0x32
#define MKC_Clear 0x47
#define MKC_KPEqual 0x51
#define MKC_KPDevide 0x4B
#define MKC_KPMultiply 0x43
#define MKC_KPSubtract 0x4E
#define MKC_KPAdd 0x45
#define MKC_Enter 0x4C

#define MKC_KP1 0x53
#define MKC_KP2 0x54
#define MKC_KP3 0x55
#define MKC_KP4 0x56
#define MKC_KP5 0x57
#define MKC_KP6 0x58
#define MKC_KP7 0x59
#define MKC_KP8 0x5B
#define MKC_KP9 0x5C
#define MKC_KP0 0x52
#define MKC_Decimal 0x41

/* these aren't on the Mac Plus keyboard */

#define MKC_Control 0x3B
#define MKC_Escape 0x35
#define MKC_F1 0x7a
#define MKC_F2 0x78
#define MKC_F3 0x63
#define MKC_F4 0x76
#define MKC_F5 0x60
#define MKC_F6 0x61
#define MKC_F7 0x62
#define MKC_F8 0x64
#define MKC_F9 0x65
#define MKC_F10 0x6d
#define MKC_F11 0x67
#define MKC_F12 0x6f

#define MKC_Home 0x73
#define MKC_End 0x77
#define MKC_PageUp 0x74
#define MKC_PageDown 0x79
#define MKC_Help 0x72 /* = Insert */
#define MKC_ForwardDel 0x75
#define MKC_Print 0x69
#define MKC_ScrollLock 0x6B
#define MKC_Pause 0x71

#define MKC_AngleBracket 0x0A /* found on german keyboard */

/*
	Additional codes found in Apple headers

	#define MKC_RightShift 0x3C
	#define MKC_RightOption 0x3D
	#define MKC_RightControl 0x3E
	#define MKC_Function 0x3F

	#define MKC_VolumeUp 0x48
	#define MKC_VolumeDown 0x49
	#define MKC_Mute 0x4A

	#define MKC_F16 0x6A
	#define MKC_F17 0x40
	#define MKC_F18 0x4F
	#define MKC_F19 0x50
	#define MKC_F20 0x5A

	#define MKC_F13 MKC_Print
	#define MKC_F14 MKC_ScrollLock
	#define MKC_F15 MKC_Pause
*/

/* not Apple key codes, only for Mini vMac */

#define MKC_CM 0x80
#define MKC_None 0xFF
