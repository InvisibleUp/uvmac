/*
	PROGMAIN.c

	Copyright (C) 2009 Bernd Schmidt, Philip Cummins, Paul C. Pratt

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
	PROGram MAIN.
*/

#include "SYSDEPNS.h"

#include "UI/MYOSGLUE.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/M68K/M68KITAB.h"
#include "HW/M68K/MINEM68K.h"
#include "HW/VIA/VIAEMDEV.h"
#include "HW/VIA/VIA2EMDV.h"
#include "HW/DISK/IWMEMDEV.h"
#include "HW/SCC/SCCEMDEV.h"
#include "HW/RTC/RTCEMDEV.h"
#include "PATCHES/ROMEMDEV.h"
#include "HW/SCSI/SCSIEMDV.h"
#include "HW/DISK/SONYEMDV.h"
#include "HW/SCREEN/SCRNEMDV.h"
#include "HW/VIDCARD/VIDEMDEV.h"
#include "HW/KBRD/KBRDEMDV.h"
#include "HW/POWERMAN/PMUEMDEV.h"
#include "HW/ADB/ADBEMDEV.h"
#include "HW/SOUND/ASCEMDEV.h"
#include "HW/SOUND/SNDEMDEV.h"
#include "HW/MOUSE/MOUSEMDV.h"

#include "PROGMAIN.h"

/*
	ReportAbnormalID unused 0x1002 - 0x10FF
*/

const bool _EmVIA2        = false;
const bool _EmRTC         = true;
const bool _EmVidCard     = false;
const bool _EmClassicKbrd = true;
const bool _EmPMU         = false;
const bool _EmMMU         = false;
const bool _EmASC         = false;
const bool _EmADB         = false;

LOCALPROC EmulatedHardwareZap(void)
{
	Memory_Reset();
	ICT_Zap();
	IWM_Reset();
	SCC_Reset();
	SCSI_Reset();
	VIA1_Zap();
	if (_EmVIA2) { VIA2_Zap(); }
	Sony_Reset();
	Extn_Reset();
	m68k_reset();
}

LOCALPROC DoMacReset(void)
{
	Sony_EjectAllDisks();
	EmulatedHardwareZap();
}

LOCALPROC InterruptReset_Update(void)
{
	SetInterruptButton(false);
		/*
			in case has been set. so only stays set
			for 60th of a second.
		*/

	if (WantMacInterrupt) {
		SetInterruptButton(true);
		WantMacInterrupt = false;
	}
	if (WantMacReset) {
		DoMacReset();
		WantMacReset = false;
	}
}

LOCALPROC SubTickNotify(int SubTick)
{
#if 0
	dbglog_writeCStr("ending sub tick ");
	dbglog_writeNum(SubTick);
	dbglog_writeReturn();
#endif
#if EmASC
		ASC_SubTick(SubTick);
#else
#if SoundEnabled && (CurEmMd != kEmMd_PB100)
	MacSound_SubTick(SubTick);
#else
	UnusedParam(SubTick);
#endif
#endif
}

#define CyclesScaledPerTick (130240UL * ClockMult * kCycleScale)
#define CyclesScaledPerSubTick (CyclesScaledPerTick / kNumSubTicks)

LOCALVAR uint16_t SubTickCounter;

LOCALPROC SubTickTaskDo(void)
{
	SubTickNotify(SubTickCounter);
	++SubTickCounter;
	if (SubTickCounter < (kNumSubTicks - 1)) {
		/*
			final SubTick handled by SubTickTaskEnd,
			since CyclesScaledPerSubTick * kNumSubTicks
			might not equal CyclesScaledPerTick.
		*/

		ICT_add(kICT_SubTick, CyclesScaledPerSubTick);
	}
}

LOCALPROC SubTickTaskStart(void)
{
	SubTickCounter = 0;
	ICT_add(kICT_SubTick, CyclesScaledPerSubTick);
}

LOCALPROC SubTickTaskEnd(void)
{
	SubTickNotify(kNumSubTicks - 1);
}

LOCALPROC SixtiethSecondNotify(void)
{
#if dbglog_HAVE && 0
	dbglog_WriteNote("begin new Sixtieth");
#endif
	Mouse_Update();
	InterruptReset_Update();
	if (_EmClassicKbrd) { KeyBoard_Update(); }
	if (_EmADB) { ADB_Update(); }

	Sixtieth_PulseNtfy(); /* Vertical Blanking Interrupt */
	Sony_Update();

#if EmLocalTalk
	LocalTalkTick();
#endif
	if (_EmRTC) { RTC_Interrupt(); }
	if (_EmVidCard) { Vid_Update(); }

	SubTickTaskStart();
}

LOCALPROC SixtiethEndNotify(void)
{
	SubTickTaskEnd();
	Mouse_EndTickNotify();
	Screen_EndTickNotify();
#if dbglog_HAVE && 0
	dbglog_WriteNote("end Sixtieth");
#endif
}

LOCALPROC ExtraTimeBeginNotify(void)
{
#if 0
	dbglog_writeCStr("begin extra time");
	dbglog_writeReturn();
#endif
	VIA1_ExtraTimeBegin();
	if (_EmVIA2) { VIA2_ExtraTimeBegin(); }
}

LOCALPROC ExtraTimeEndNotify(void)
{
	VIA1_ExtraTimeEnd();
	if (_EmVIA2) { VIA2_ExtraTimeEnd(); }
#if 0
	dbglog_writeCStr("end extra time");
	dbglog_writeReturn();
#endif
}

GLOBALPROC EmulationReserveAlloc(void)
{
	ReserveAllocOneBlock(&RAM,
		kRAM_Size + RAMSafetyMarginFudge, 5, false);
#if EmVidCard
		ReserveAllocOneBlock(&VidROM, kVidROM_Size, 5, false);
#endif
#if IncludeVidMem
	ReserveAllocOneBlock(&VidMem,
		kVidMemRAM_Size + RAMSafetyMarginFudge, 5, true);
#endif
#if SmallGlobals
	MINEM68K_ReserveAlloc();
#endif
}

LOCALFUNC bool InitEmulation(void)
{
	bool retval = true;

	retval &= ROM_Init();
	retval &= AddrSpac_Init();
	if (_EmRTC) { retval &= RTC_Init(); }
	if (_EmVidCard) { retval &= Vid_Init(); }

	if (retval == true) { EmulatedHardwareZap(); }
	return retval;
}

LOCALPROC ICT_DoTask(int taskid)
{
	switch (taskid) {
		case kICT_SubTick:
			SubTickTaskDo();
			break;
#if EmClassicKbrd
		case kICT_Kybd_ReceiveEndCommand:
			DoKybd_ReceiveEndCommand();
			break;
		case kICT_Kybd_ReceiveCommand:
			DoKybd_ReceiveCommand();
			break;
#endif
#if EmADB
		case kICT_ADB_NewState:
			ADB_DoNewState();
			break;
#endif
#if EmPMU
		case kICT_PMU_Task:
			PMU_DoTask();
			break;
#endif
		case kICT_VIA1_Timer1Check:
			VIA1_DoTimer1Check();
			break;
		case kICT_VIA1_Timer2Check:
			VIA1_DoTimer2Check();
			break;
#if EmVIA2
		case kICT_VIA2_Timer1Check:
			VIA2_DoTimer1Check();
			break;
		case kICT_VIA2_Timer2Check:
			VIA2_DoTimer2Check();
			break;
#endif
		default:
			ReportAbnormalID(0x1001, "unknown taskid in ICT_DoTask");
			break;
	}
}

LOCALPROC ICT_DoCurrentTasks(void)
{
	int i = 0;
	uimr m = ICTactive;

	while (0 != m) {
		if (0 != (m & 1)) {
			if (i >= kNumICTs) {
				/* shouldn't happen */
				ICTactive &= ((1 << kNumICTs) - 1);
				m = 0;
			} else if (ICTwhen[i] == NextiCount) {
				ICTactive &= ~ (1 << i);
#ifdef _VIA_Debug
				fprintf(stderr, "doing task %d, %d\n", NextiCount, i);
#endif
				ICT_DoTask(i);

				/*
					A Task may set the time of
					any task, including itself.
					But it cannot set any task
					to execute immediately, so
					one pass is sufficient.
				*/
			}
		}
		++i;
		m >>= 1;
	}
}

LOCALFUNC uint32_t ICT_DoGetNext(uint32_t maxn)
{
	int i = 0;
	uimr m = ICTactive;
	uint32_t v = maxn;

	while (0 != m) {
		if (0 != (m & 1)) {
			if (i >= kNumICTs) {
				/* shouldn't happen */
				m = 0;
			} else {
				uint32_t d = ICTwhen[i] - NextiCount;
				/* at this point d must be > 0 */
				if (d < v) {
#ifdef _VIA_Debug
					fprintf(stderr, "coming task %d, %d, %d\n",
						NextiCount, i, d);
#endif
					v = d;
				}
			}
		}
		++i;
		m >>= 1;
	}

	return v;
}

LOCALPROC m68k_go_nCycles_1(uint32_t n)
{
	uint32_t n2;
	uint32_t StopiCount = NextiCount + n;
	do {
		ICT_DoCurrentTasks();
		n2 = ICT_DoGetNext(n);
#if dbglog_HAVE && 0
		dbglog_StartLine();
		dbglog_writeCStr("before m68k_go_nCycles, NextiCount:");
		dbglog_writeHex(NextiCount);
		dbglog_writeCStr(", n2:");
		dbglog_writeHex(n2);
		dbglog_writeCStr(", n:");
		dbglog_writeHex(n);
		dbglog_writeReturn();
#endif
		NextiCount += n2;
		m68k_go_nCycles(n2);
		n = StopiCount - NextiCount;
	} while (n != 0);
}

LOCALVAR uint32_t ExtraSubTicksToDo = 0;

LOCALPROC DoEmulateOneTick(void)
{
#if EnableAutoSlow
	{
		uint32_t NewQuietTime = QuietTime + 1;

		if (NewQuietTime > QuietTime) {
			/* if not overflow */
			QuietTime = NewQuietTime;
		}
	}
#endif
#if EnableAutoSlow
	{
		uint32_t NewQuietSubTicks = QuietSubTicks + kNumSubTicks;

		if (NewQuietSubTicks > QuietSubTicks) {
			/* if not overflow */
			QuietSubTicks = NewQuietSubTicks;
		}
	}
#endif

	SixtiethSecondNotify();

	m68k_go_nCycles_1(CyclesScaledPerTick);

	SixtiethEndNotify();

	if ((uint8_t) -1 == SpeedValue) {
		ExtraSubTicksToDo = (uint32_t) -1;
	} else {
		uint32_t ExtraAdd = (kNumSubTicks << SpeedValue) - kNumSubTicks;
		uint32_t ExtraLimit = ExtraAdd << 3;

		ExtraSubTicksToDo += ExtraAdd;
		if (ExtraSubTicksToDo > ExtraLimit) {
			ExtraSubTicksToDo = ExtraLimit;
		}
	}
}

LOCALFUNC bool MoreSubTicksToDo(void)
{
	bool v = false;

	if (ExtraTimeNotOver() && (ExtraSubTicksToDo > 0)) {
#if EnableAutoSlow
		if ((QuietSubTicks >= 16384)
			&& (QuietTime >= 34)
			&& ! WantNotAutoSlow)
		{
			ExtraSubTicksToDo = 0;
		} else
#endif
		{
			v = true;
		}
	}

	return v;
}

LOCALPROC DoEmulateExtraTime(void)
{
	/*
		DoEmulateExtraTime is used for
		anything over emulation speed
		of 1x. It periodically calls
		ExtraTimeNotOver and stops
		when this returns false (or it
		is finished with emulating the
		extra time).
	*/

	if (MoreSubTicksToDo()) {
		ExtraTimeBeginNotify();
		do {
#if EnableAutoSlow
			{
				uint32_t NewQuietSubTicks = QuietSubTicks + 1;

				if (NewQuietSubTicks > QuietSubTicks) {
					/* if not overflow */
					QuietSubTicks = NewQuietSubTicks;
				}
			}
#endif
			m68k_go_nCycles_1(CyclesScaledPerSubTick);
			--ExtraSubTicksToDo;
		} while (MoreSubTicksToDo());
		ExtraTimeEndNotify();
	}
}

LOCALVAR uint32_t CurEmulatedTime = 0;
	/*
		The number of ticks that have been
		emulated so far.

		That is, the number of times
		"DoEmulateOneTick" has been called.
	*/

LOCALPROC RunEmulatedTicksToTrueTime(void)
{
	/*
		The general idea is to call DoEmulateOneTick
		once per tick.

		But if emulation is lagging, we'll try to
		catch up by calling DoEmulateOneTick multiple
		times, unless we're too far behind, in
		which case we forget it.

		If emulating one tick takes longer than
		a tick we don't want to sit here
		forever. So the maximum number of calls
		to DoEmulateOneTick is determined at
		the beginning, rather than just
		calling DoEmulateOneTick until
		CurEmulatedTime >= TrueEmulatedTime.
	*/

	int8_t n = OnTrueTime - CurEmulatedTime;

	if (n > 0) {
		DoEmulateOneTick();
		++CurEmulatedTime;

		DoneWithDrawingForTick();

		if (n > 8) {
			/* emulation not fast enough */
			n = 8;
			CurEmulatedTime = OnTrueTime - n;
		}

		if (ExtraTimeNotOver() && (--n > 0)) {
			/* lagging, catch up */

			EmVideoDisable = true;

			do {
				DoEmulateOneTick();
				++CurEmulatedTime;
			} while (ExtraTimeNotOver()
				&& (--n > 0));

			EmVideoDisable = false;
		}

		EmLagTime = n;
	}
}

LOCALPROC MainEventLoop(void)
{
	for (; ; ) {
		WaitForNextTick();
		if (ForceMacOff) {
			return;
		}

		RunEmulatedTicksToTrueTime();

		DoEmulateExtraTime();
	}
}

GLOBALPROC ProgramMain(void)
{
	if (InitEmulation())
	{
		MainEventLoop();
	}
}
