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

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "SYSDEPNS.h"

#include "UI/MYOSGLUE.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/M68K/M68KITAB.h"
#include "HW/M68K/MINEM68K.h"
#include "HW/VIA/VIAEMDEV.h"
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

// Let's define a bunch of function structure thingies

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

typedef struct DevMethods {
	bool (*init)(void);
	void (*reset)(void);
	void (*starttick)(void);
	void (*endtick)(void);
	void (*subtick)(int);
	void (*timebegin)(void);
	void (*timeend)(void);
} DevMethods_t;

const DevMethods_t DEVICES[] = {
	// VIA
	{
	.init = VIA_Zap,
	.reset = VIA_Reset,
	.starttick = NULL,//VIA_TickTimers,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = VIA1_ExtraTimeBegin,
	.timeend = VIA1_ExtraTimeEnd,
	},
	// RTC
	{
	.init = EmRTC ? RTC_Init : NULL,
	.reset = NULL,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// ROM
	{
	.init = ROM_Init,
	.reset = NULL,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// Memory
	{
	.init = AddrSpac_Init,
	.reset = Memory_Reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// IWM
	{
	.init = NULL,
	.reset = IWM_Reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// SCC
	{
	.init = NULL,
	.reset = SCC_Reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// SCSI
	{
	.init = NULL,
	.reset = SCSI_Reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// Sony disk drive
	{
	.init = NULL,
	.reset = Sony_Reset,
	.starttick = Sony_Update,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// Extn
	{
	.init = NULL,
	.reset = Extn_Reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// m68k
	{
	.init = NULL,
	.reset = m68k_reset,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// Mouse
	{
	.init = NULL,
	.reset = NULL,
	.starttick = Mouse_Update,
	.endtick = Mouse_EndTickNotify,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// Classic Keyboard
	{
	.init = NULL,
	.reset = NULL,
	.starttick = EmClassicKbrd ? KeyBoard_Update : NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
	// ADB
	/*{
	.init = NULL,
	.reset = NULL,
	.starttick = EmADB ? ADB_Update : NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},*/
	// LocalTalk
	/*{
	.init = NULL,
	.reset = NULL,
	.starttick = EmLocalTalk ? LocalTalkTick : NULL,
	.endtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},*/
	// Video card
	/*{
	.init = EmVidCard ? Vid_Init : NULL,
	.reset = NULL,
	.starttick = EmVidCard ? Vid_Update : NULL,
	.endtick = NULL,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},*/
	// ASC
/*	{
	.init = NULL,
	.reset = NULL,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = EmASC ? ASC_SubTick : NULL,
	.timebegin = NULL,
	.timeend = NULL
	},*/
	// Sound (non-ASC)
	{
	.init = NULL,
	.reset = NULL,
	.starttick = NULL,
	.endtick = NULL,
	.subtick = (SoundEnabled && (CurEmMd != kEmMd_PB100)) ? MacSound_SubTick : NULL,
	.timebegin = NULL,
	.timeend = NULL
	},
	// Screen
	{
	.init = Screen_Init,
	.reset = NULL,
	.starttick = Screen_RaiseVBlank,
	.endtick = Screen_EndTickNotify,
	.subtick = NULL,
	.timebegin = NULL,
	.timeend = NULL,
	},
};

LOCALPROC EmulatedHardwareZap(void)
{
	int i;
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].reset != NULL) { DEVICES[i].reset(); }
	}
}

LOCALPROC DoMacReset(void)
{
	Sony_EjectAllDisks();
	EmulatedHardwareZap();
}

LOCALPROC InterruptReset_Update(void)
{
	SetInterruptButton(false); // don't keep held over 1/60 sec

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
	int i;
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].subtick != NULL) { DEVICES[i].subtick(SubTick); }
	}
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
	/*int i;
	// Begin new frame
	InterruptReset_Update();
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].starttick != NULL) { DEVICES[i].starttick(); }
	}*/
	
	// Temporary fix to non-functional RTC
	
	Mouse_Update();
	InterruptReset_Update();
	if (EmClassicKbrd) { KeyBoard_Update(); }
	//if (EmADB) { ADB_Update(); }

	Screen_RaiseVBlank();
	Sony_Update();

#if EmLocalTalk
	LocalTalkTick();
#endif
	if (EmRTC) { RTC_Interrupt(); }
	//if (EmVidCard) { Vid_Update(); }

	SubTickTaskStart();
}

LOCALPROC SixtiethEndNotify(void)
{
	int i;
	SubTickTaskEnd();
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].endtick != NULL) { DEVICES[i].endtick(); }
	}
	// End frame
}

LOCALPROC ExtraTimeBeginNotify(void)
{
	int i;
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].timebegin != NULL) { DEVICES[i].timebegin(); }
	}
}

LOCALPROC ExtraTimeEndNotify(void)
{
	int i;
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].timeend != NULL) { DEVICES[i].timeend(); }
	}
}

GLOBALPROC EmulationReserveAlloc(void)
{
	ReserveAllocOneBlock(&RAM, kRAM_Size + RAMSafetyMarginFudge, 5, false);
#if EmVidCard
		ReserveAllocOneBlock(&VidROM, kVidROM_Size, 5, false);
#endif
#if IncludeVidMem
	ReserveAllocOneBlock(&VidMem, kVidMemRAM_Size + RAMSafetyMarginFudge, 5, true);
#endif
#if SmallGlobals
	MINEM68K_ReserveAlloc();
#endif
}

LOCALFUNC bool InitEmulation(void)
{
	int i;
	for ( i = 0; i < ARRAY_SIZE(DEVICES); i++ ) {
		if (DEVICES[i].init != NULL) {
			assert(DEVICES[i].init());
		}
		if (DEVICES[i].reset != NULL) {
			DEVICES[i].reset();
		}
	}
	
	// temporarily register some ISRs until I put these in a better place
	// Mac Plus only
	VIA_RegisterDataISR(VIA1, DataRegA, 4, MemOverlay_ChangeNtfy);
	VIA_RegisterCB2ISR(VIA1, Kybd_DataLineChngNtfy);
	return true;
}
 // VBlank interrupt

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
		default:
			ReportAbnormalID(0x1001, "unknown taskid in ICT_DoTask");
			break;
	}
}

LOCALPROC ICT_DoCurrentTasks(void)
{
	int i;
	uimr m;

	// For each ICT...
	for (m = ICTactive, i = 0; m != 0; m >>=1, i += 1) {
		// If no ICT here, continue
		if (0 == (m & 1)) { continue; }
		if (ICTwhen[i] != NextiCount) { continue; }

		/*
			A Task may set the time of any task, including
			itself. But it cannot set any task to execute
			immediately, so one pass is sufficient.
		*/
		ICTactive &= ~(1 << i); // Clear active bit
		ICT_DoTask(i);
	}
}

LOCALFUNC uint32_t ICT_DoGetNext(uint32_t maxn)
{
	int i;
	uimr m;
	uint32_t v = maxn;

	for (m = ICTactive, i = 0; m != 0; m >>=1, i += 1) {
		uint32_t d = ICTwhen[i] - NextiCount;
		if (0 == (m & 1)) { continue; }
		if (d < v) { v = d; }
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
		NextiCount += n2;
		m68k_go_nCycles(n2);
		n = StopiCount - NextiCount;
	} while (n != 0);
}

LOCALVAR uint32_t ExtraSubTicksToDo = 0;

LOCALPROC DoEmulateOneTick(void)
{
	// AutoSlow
	uint32_t NewQuietTime = QuietTime + 1;
	uint32_t NewQuietSubTicks = QuietSubTicks + kNumSubTicks;

	if (NewQuietTime > QuietTime) {
		/* if not overflow */
		QuietTime = NewQuietTime;
	}
	if (NewQuietSubTicks > QuietSubTicks) {
		/* if not overflow */
		QuietSubTicks = NewQuietSubTicks;
	}

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
		if ( (QuietSubTicks >= 16384) && (QuietTime >= 34)) {
			ExtraSubTicksToDo = 0;
		} else {
			v = true;
		}
	}

	return v;
}

/*
	DoEmulateExtraTime is used for anything over emulation speed of 1x. It
	periodically calls ExtraTimeNotOver and stops when this returns false
	(or it is finished with emulating the extra time).
*/
LOCALPROC DoEmulateExtraTime(void)
{
	if (MoreSubTicksToDo()) {
		ExtraTimeBeginNotify();
		do {
			uint32_t NewQuietSubTicks = QuietSubTicks + 1;

			if (NewQuietSubTicks > QuietSubTicks) {
				/* if not overflow */
				QuietSubTicks = NewQuietSubTicks;
			}
			m68k_go_nCycles_1(CyclesScaledPerSubTick);
			--ExtraSubTicksToDo;
		} while (MoreSubTicksToDo());
		ExtraTimeEndNotify();
	}
}

/*
	The number of ticks that have been emulated so far.
	That is, the number of times "DoEmulateOneTick" has been called.
*/
LOCALVAR uint32_t CurEmulatedTime = 0;

/*
	The general idea is to call DoEmulateOneTick once per tick.

	But if emulation is lagging, we'll try to catch up by calling
	DoEmulateOneTick multiple times, unless we're too far behind, in
	which case we forget it.

	If emulating one tick takes longer than a tick we don't want to sit
	here forever. So the maximum number of calls to DoEmulateOneTick is
	determined at the beginning, rather than just calling DoEmulateOneTick
	until CurEmulatedTime >= TrueEmulatedTime.
*/
LOCALPROC RunEmulatedTicksToTrueTime(void)
{
	int8_t lag = OnTrueTime - CurEmulatedTime;

	if (lag > 0) {
		DoEmulateOneTick();
		CurEmulatedTime += 1;
		//fprintf(stderr, "Tick %d\n", CurEmulatedTime);
		DoneWithDrawingForTick();

		if (lag > 8) {
			/* emulation not fast enough */
			lag = 8;
			CurEmulatedTime = OnTrueTime - lag;
		} else {
			lag -= 1;
		}

		while (ExtraTimeNotOver() && (lag > 0)) {
			/* Missed vblank due to lag; catch up */
			EmVideoDisable = true;
			DoEmulateOneTick();
			CurEmulatedTime += 1;
			lag -= 1;
			EmVideoDisable = false;
		}

		EmLagTime = lag;
	}
}

LOCALPROC MainEventLoop(void)
{
	while (true) {
		WaitForNextTick();
		if (ForceMacOff) { return; }
		RunEmulatedTicksToTrueTime();
		DoEmulateExtraTime();
	}
}

GLOBALPROC ProgramMain(void)
{
	if (InitEmulation() == false) {return;}
	MainEventLoop();
}
