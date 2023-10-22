/*
	RTCEMDEV.c

	Copyright (C) 2003 Philip Cummins, Paul C. Pratt

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
	Real Time Clock EMulated DEVice

	Emulates the RTC found in the Mac Plus.

	This code adapted from "RTC.c" in vMac by Philip Cummins.
*/

#include "SYSDEPNS.h"
#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"

/* define _RTC_Debug */
#ifdef _RTC_Debug
#include <stdio.h>
#endif

#include "HW/RTC/RTCEMDEV.h"

#define HaveXPRAM (CurEmMd >= kEmMd_Plus)

/*
	ReportAbnormalID unused 0x0805 - 0x08FF
*/

#if HaveXPRAM
#define PARAMRAMSize 256
#else
#define PARAMRAMSize 20
#endif

#if HaveXPRAM
#define Group1Base 0x10
#define Group2Base 0x08
#else
#define Group1Base 0x00
#define Group2Base 0x10
#endif

typedef struct
{
	/* RTC VIA Flags */
	uint8_t WrProtect;
	uint8_t DataOut;
	uint8_t DataNextOut;

	/* RTC Data */
	uint8_t ShiftData;
	uint8_t Counter;
	uint8_t Mode;
	uint8_t SavedCmd;
#if HaveXPRAM
	uint8_t Sector;
#endif

	/* RTC Registers */
	uint8_t Seconds_1[4];
	uint8_t PARAMRAM[PARAMRAMSize];
} RTC_Ty;

static RTC_Ty RTC;

/* RTC Functions */

static uint32_t LastRealDate;

#ifndef RTCinitPRAM
#define RTCinitPRAM 1
#endif

#ifndef TrackSpeed /* in 0..4 */
#define TrackSpeed 0
#endif

#ifndef AlarmOn /* in 0..1 */
#define AlarmOn 0
#endif

#ifndef DiskCacheSz /* in 1,2,3,4,6,8,12 */
/* actual cache size is DiskCacheSz * 32k */
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define DiskCacheSz 1
#else
#define DiskCacheSz 4
#endif
#endif

#ifndef StartUpDisk /* in 0..1 */
#define StartUpDisk 0
#endif

#ifndef DiskCacheOn /* in 0..1 */
#define DiskCacheOn 0
#endif

#ifndef MouseScalingOn /* in 0..1 */
#define MouseScalingOn 0
#endif

#define prb_fontHi 0
#define prb_fontLo 2
#define prb_kbdPrintHi (AutoKeyRate + (AutoKeyThresh << 4))
#define prb_kbdPrintLo 0
#define prb_volClickHi (SpeakerVol + (TrackSpeed << 3) + (AlarmOn << 7))
#define prb_volClickLo (CaretBlinkTime + (DoubleClickTime << 4))
#define prb_miscHi DiskCacheSz
#define prb_miscLo \
	((MenuBlink << 2) + (StartUpDisk << 4) \
		+ (DiskCacheOn << 5) + (MouseScalingOn << 6))

#if dbglog_HAVE && 0
extern void DumpRTC(void);

void DumpRTC(void)
{
	int Counter;

	dbglog_writeln("RTC Parameter RAM");
	for (Counter = 0; Counter < PARAMRAMSize; Counter++) {
		dbglog_writeNum(Counter);
		dbglog_writeCStr(", ");
		dbglog_writeHex(RTC.PARAMRAM[Counter]);
		dbglog_writeReturn();
	}
}
#endif

 bool RTC_Init(void)
{
	int Counter;
	uint32_t secs;

	RTC.Mode = RTC.ShiftData = RTC.Counter = 0;
	RTC.DataOut = RTC.DataNextOut = 0;
	RTC.WrProtect = false;

	secs = CurMacDateInSeconds;
	LastRealDate = secs;

	RTC.Seconds_1[0] = secs & 0xFF;
	RTC.Seconds_1[1] = (secs & 0xFF00) >> 8;
	RTC.Seconds_1[2] = (secs & 0xFF0000) >> 16;
	RTC.Seconds_1[3] = (secs & 0xFF000000) >> 24;

	for (Counter = 0; Counter < PARAMRAMSize; Counter++) {
		RTC.PARAMRAM[Counter] = 0;
	}

#if RTCinitPRAM
	RTC.PARAMRAM[0 + Group1Base] = 168; /* valid */
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[2 + Group1Base] = 1;
		/* node id hint for printer port (AppleTalk) */
#endif
	RTC.PARAMRAM[3 + Group1Base] = 34;
		/*
			serial ports config bits: 4-7 A, 0-3 B
				useFree   0 Use undefined
				useATalk  1 AppleTalk
				useAsync  2 Async
				useExtClk 3 externally clocked
		*/

	RTC.PARAMRAM[4 + Group1Base] = 204; /* portA, high */
	RTC.PARAMRAM[5 + Group1Base] = 10; /* portA, low */
	RTC.PARAMRAM[6 + Group1Base] = 204; /* portB, high */
	RTC.PARAMRAM[7 + Group1Base] = 10; /* portB, low */
	RTC.PARAMRAM[13 + Group1Base] = prb_fontLo;
	RTC.PARAMRAM[14 + Group1Base] = prb_kbdPrintHi;
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[15 + Group1Base] = 1;
		/*
			printer, if any, connected to modem port
			because printer port used for appletalk.
		*/
#endif

#if prb_volClickHi != 0
	RTC.PARAMRAM[0 + Group2Base] = prb_volClickHi;
#endif
	RTC.PARAMRAM[1 + Group2Base] = prb_volClickLo;
	RTC.PARAMRAM[2 + Group2Base] = prb_miscHi;
	RTC.PARAMRAM[3 + Group2Base] = prb_miscLo \
		| (vMacScreenDepth != 0) ? 0x80 : 0;

#if HaveXPRAM /* extended parameter ram initialized */
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[12] = 0x4e;
	RTC.PARAMRAM[13] = 0x75;
	RTC.PARAMRAM[14] = 0x4d;
	RTC.PARAMRAM[15] = 0x63;
#else
	RTC.PARAMRAM[12] = 0x42;
	RTC.PARAMRAM[13] = 0x75;
	RTC.PARAMRAM[14] = 0x67;
	RTC.PARAMRAM[15] = 0x73;
#endif
#endif

#if ((CurEmMd >= kEmMd_SE) && (CurEmMd <= kEmMd_Classic)) \
	|| (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)

	RTC.PARAMRAM[0x01] = 0x80;
	RTC.PARAMRAM[0x02] = 0x4F;
#endif
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[0x03] = 0x48;

	/* video board id */
	RTC.PARAMRAM[0x46] = /* 0x42 */ 0x76; /* 'v' */
	RTC.PARAMRAM[0x47] = /* 0x32 */ 0x4D; /* 'M' */
	/* mode */
if ((0 == vMacScreenDepth) || (vMacScreenDepth >= 4)) {
	RTC.PARAMRAM[0x48] = 0x80;
} else {
	RTC.PARAMRAM[0x48] = 0x81;
		/* 0x81 doesn't quite work right at boot */
			/* no, it seems to work now (?) */
			/* but only if depth <= 3 */
}
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[0x77] = 0x01;
#endif

#if ((CurEmMd >= kEmMd_SE) && (CurEmMd <= kEmMd_Classic)) \
	|| (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)

	/* start up disk (encoded how?) */
	RTC.PARAMRAM[0x78] = 0x00;
	RTC.PARAMRAM[0x79] = 0x01;
	RTC.PARAMRAM[0x7A] = 0xFF;
	RTC.PARAMRAM[0x7B] = 0xFE;
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	RTC.PARAMRAM[0x80] = 0x09;
	RTC.PARAMRAM[0x81] = 0x80;
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)

#define pr_HilColRedHi (pr_HilColRed >> 8)
#if 0 != pr_HilColRedHi
	RTC.PARAMRAM[0x82] = pr_HilColRedHi;
#endif
#define pr_HilColRedLo (pr_HilColRed & 0xFF)
#if 0 != pr_HilColRedLo
	RTC.PARAMRAM[0x83] = pr_HilColRedLo;
#endif

#define pr_HilColGreenHi (pr_HilColGreen >> 8)
#if 0 != pr_HilColGreenHi
	RTC.PARAMRAM[0x84] = pr_HilColGreenHi;
#endif
#define pr_HilColGreenLo (pr_HilColGreen & 0xFF)
#if 0 != pr_HilColGreenLo
	RTC.PARAMRAM[0x85] = pr_HilColGreenLo;
#endif

#define pr_HilColBlueHi (pr_HilColBlue >> 8)
#if 0 != pr_HilColBlueHi
	RTC.PARAMRAM[0x86] = pr_HilColBlueHi;
#endif
#define pr_HilColBlueLo (pr_HilColBlue & 0xFF)
#if 0 != pr_HilColBlueLo
	RTC.PARAMRAM[0x87] = pr_HilColBlueLo;
#endif

#endif /* (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx) */

#if HaveXPRAM /* extended parameter ram initialized */
	do_put_mem_long(&RTC.PARAMRAM[0xE4], CurMacLatitude);
	do_put_mem_long(&RTC.PARAMRAM[0xE8], CurMacLongitude);
	do_put_mem_long(&RTC.PARAMRAM[0xEC], CurMacDelta);
#endif

#endif /* RTCinitPRAM */

	return true;
}

#ifdef RTC_OneSecond_PulseNtfy
extern void RTC_OneSecond_PulseNtfy(void);
#endif

void RTC_Interrupt(void)
{
	uint32_t Seconds = 0;
	uint32_t NewRealDate = CurMacDateInSeconds;
	uint32_t DateDelta = NewRealDate - LastRealDate;

	if (DateDelta != 0) {
		Seconds = (RTC.Seconds_1[3] << 24) + (RTC.Seconds_1[2] << 16)
			+ (RTC.Seconds_1[1] << 8) + RTC.Seconds_1[0];
		Seconds += DateDelta;
		RTC.Seconds_1[0] = Seconds & 0xFF;
		RTC.Seconds_1[1] = (Seconds & 0xFF00) >> 8;
		RTC.Seconds_1[2] = (Seconds & 0xFF0000) >> 16;
		RTC.Seconds_1[3] = (Seconds & 0xFF000000) >> 24;

		LastRealDate = NewRealDate;

#ifdef RTC_OneSecond_PulseNtfy
		RTC_OneSecond_PulseNtfy();
#endif
	}
}

static uint8_t RTC_Access_PRAM_Reg(uint8_t Data, bool WriteReg, uint8_t t)
{
	if (WriteReg) {
		if (! RTC.WrProtect) {
			RTC.PARAMRAM[t] = Data;
#ifdef _RTC_Debug
			printf("Writing Address %2x, Data %2x\n", t, Data);
#endif
		}
	} else {
		Data = RTC.PARAMRAM[t];
	}
	return Data;
}

static uint8_t RTC_Access_Reg(uint8_t Data, bool WriteReg, uint8_t TheCmd)
{
	uint8_t t = (TheCmd & 0x7C) >> 2;
	if (t < 8) {
		if (WriteReg) {
			if (! RTC.WrProtect) {
				RTC.Seconds_1[t & 0x03] = Data;
			}
		} else {
			Data = RTC.Seconds_1[t & 0x03];
		}
	} else if (t < 12) {
		Data = RTC_Access_PRAM_Reg(Data, WriteReg,
			(t & 0x03) + Group2Base);
	} else if (t < 16) {
		if (WriteReg) {
			switch (t) {
				case 12 :
					break; /* Test Write, do nothing */
				case 13 :
					RTC.WrProtect = (Data & 0x80) != 0;
					break; /* Write_Protect Register */
				default :
					ReportAbnormalID(0x0801, "Write RTC Reg unknown");
					break;
			}
		} else {
			ReportAbnormalID(0x0802, "Read RTC Reg unknown");
		}
	} else {
		Data = RTC_Access_PRAM_Reg(Data, WriteReg,
			(t & 0x0F) + Group1Base);
	}
	return Data;
}

static void RTC_DoCmd(void)
{
	switch (RTC.Mode) {
		case 0: /* This Byte is a RTC Command */
#if HaveXPRAM
			if ((RTC.ShiftData & 0x78) == 0x38) { /* Extended Command */
				RTC.SavedCmd = RTC.ShiftData;
				RTC.Mode = 2;
#ifdef _RTC_Debug
				printf("Extended command %2x\n", RTC.ShiftData);
#endif
			} else
#endif
			{
				if ((RTC.ShiftData & 0x80) != 0x00) { /* Read Command */
					RTC.ShiftData =
						RTC_Access_Reg(0, false, RTC.ShiftData);
					RTC.DataNextOut = 1;
				} else { /* Write Command */
					RTC.SavedCmd = RTC.ShiftData;
					RTC.Mode = 1;
				}
			}
			break;
		case 1: /* This Byte is data for RTC Write */
			(void) RTC_Access_Reg(RTC.ShiftData,
				true, RTC.SavedCmd);
			RTC.Mode = 0;
			break;
#if HaveXPRAM
		case 2: /* This Byte is rest of Extended RTC command address */
#ifdef _RTC_Debug
			printf("Mode 2 %2x\n", RTC.ShiftData);
#endif
			RTC.Sector = ((RTC.SavedCmd & 0x07) << 5)
				| ((RTC.ShiftData & 0x7C) >> 2);
			if ((RTC.SavedCmd & 0x80) != 0x00) { /* Read Command */
				RTC.ShiftData = RTC.PARAMRAM[RTC.Sector];
				RTC.DataNextOut = 1;
				RTC.Mode = 0;
#ifdef _RTC_Debug
				printf("Reading X Address %2x, Data  %2x\n",
					RTC.Sector, RTC.ShiftData);
#endif
			} else {
				RTC.Mode = 3;
#ifdef _RTC_Debug
				printf("Writing X Address %2x\n", RTC.Sector);
#endif
			}
			break;
		case 3: /* This Byte is data for an Extended RTC Write */
			(void) RTC_Access_PRAM_Reg(RTC.ShiftData,
				true, RTC.Sector);
			RTC.Mode = 0;
			break;
#endif
	}
}

void RTCunEnabled_ChangeNtfy(void)
{
	if (RTCunEnabled) {
		/* abort anything going on */
		if (RTC.Counter != 0) {
#ifdef _RTC_Debug
			printf("aborting, %2x\n", RTC.Counter);
#endif
			ReportAbnormalID(0x0803, "RTC aborting");
		}
		RTC.Mode = 0;
		RTC.DataOut = 0;
		RTC.DataNextOut = 0;
		RTC.ShiftData = 0;
		RTC.Counter = 0;
	}
}

void RTCclock_ChangeNtfy(void)
{
	if (! RTCunEnabled) {
		if (RTCclock) {
			RTC.DataOut = RTC.DataNextOut;
			RTC.Counter = (RTC.Counter - 1) & 0x07;
			if (RTC.DataOut) {
				RTCdataLine = ((RTC.ShiftData >> RTC.Counter) & 0x01);
				/*
					should notify VIA if changed, so can check
					data direction
				*/
				if (RTC.Counter == 0) {
					RTC.DataNextOut = 0;
				}
			} else {
				RTC.ShiftData = (RTC.ShiftData << 1) | RTCdataLine;
				if (RTC.Counter == 0) {
					RTC_DoCmd();
				}
			}
		}
	}
}

void RTCdataLine_ChangeNtfy(void)
{
#if dbglog_HAVE
	if (RTC.DataOut) {
		if (! RTC.DataNextOut) {
			/*
				ignore. The ROM doesn't read from the RTC the
				way described in the Hardware Reference.
				It reads the data after setting the clock to
				one instead of before, and then immediately
				changes the VIA direction. So the RTC
				has no way of knowing to stop driving the
				data line, which certainly can't really be
				correct.
			*/
		} else {
			ReportAbnormalID(0x0804,
				"write RTC Data unexpected direction");
		}
	}
#endif
}
