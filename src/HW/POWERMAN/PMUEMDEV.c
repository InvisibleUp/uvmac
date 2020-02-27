/*
	PMUEMDEV.c

	Copyright (C) 2008 Paul C. Pratt

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
	Power Management Unit EMulated DEVice
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "UI/MYOSGLUE.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/VIA/VIAEMDEV.h"
#endif

#include "HW/POWERMAN/PMUEMDEV.h"

/*
	ReportAbnormalID unused 0x0E0E - 0x0EFF
*/

enum {
	kPMUStateReadyForCommand,
	kPMUStateRecievingLength,
	kPMUStateRecievingBuffer,
	kPMUStateRecievedCommand,
	kPMUStateSendLength,
	kPMUStateSendBuffer,

	kPMUStates
};

#define PMU_BuffSz 8
LOCALVAR uint8_t PMU_BuffA[PMU_BuffSz];
LOCALVAR uint8_t * PMU_p;
LOCALVAR uint8_t PMU_rem;
LOCALVAR uint8_t PMU_i;

LOCALVAR int PMUState = kPMUStateReadyForCommand;

LOCALVAR uint8_t PMU_CurCommand;
LOCALVAR uint8_t PMU_SendNext;
LOCALVAR uint8_t PMU_BuffL;

LOCALPROC PmuStartSendResult(uint8_t ResultCode, uint8_t L)
{
	PMU_SendNext = ResultCode;
	PMU_BuffL = L;
	PMUState = kPMUStateSendLength;
}

LOCALVAR uint8_t PARAMRAM[128];

LOCALPROC PmuCheckCommandOp(void)
{
	switch (PMU_CurCommand) {
		case 0x10: /* kPMUpowerCntl - power plane/clock control */
			break;
		case 0x32: /* kPMUxPramWrite - write extended PRAM byte(s) */
			if (kPMUStateRecievingBuffer == PMUState) {
				if (0 == PMU_i) {
					if (PMU_BuffL >= 2) {
						PMU_p = PMU_BuffA;
						PMU_rem = 2;
					} else {
						ReportAbnormalID(0x0E01,
							"PMU_BuffL too small for kPMUxPramWrite");
					}
				} else if (2 == PMU_i) {
					if ((PMU_BuffA[1] + 2 == PMU_BuffL)
						&& (PMU_BuffA[0] + PMU_BuffA[1] <= 0x80))
					{
						PMU_p = &PARAMRAM[PMU_BuffA[0]];
						PMU_rem = PMU_BuffA[1];
					} else {
						ReportAbnormalID(0x0E02,
							"bad range for kPMUxPramWrite");
					}
				} else {
					ReportAbnormalID(0x0E03,
						"Wrong PMU_i for kPMUpramWrite");
				}
			} else if (kPMUStateRecievedCommand == PMUState) {
				/* already done */
			}
			break;
#if 0
		case 0xE2: /* kPMUdownloadStatus - PRAM status */
			break;
#endif
		case 0xE0: /* kPMUwritePmgrRAM - write to internal PMGR RAM */
			break;
		case 0x21: /* kPMUpMgrADBoff - turn ADB auto-poll off */
			if (kPMUStateRecievedCommand == PMUState) {
				if (0 != PMU_BuffL) {
					ReportAbnormalID(0x0E04,
						"kPMUpMgrADBoff nonzero length");
				}
			}
			break;
		case 0xEC: /* kPMUPmgrSelfTest - run the PMGR selftest */
			if (kPMUStateRecievedCommand == PMUState) {
				PmuStartSendResult(0, 0);
			}
			break;
		case 0x78:
			/* kPMUreadINT - get PMGR interrupt data */
		case 0x68:
			/*
				kPMUbatteryRead - read battery/charger level and status
			*/
		case 0x7F:
			/*
				kPMUsleepReq - put the system to sleep (sleepSig='MATT')
			*/
			if (kPMUStateRecievedCommand == PMUState) {
				PMU_BuffA[0] = 0;
				PmuStartSendResult(0, 1);
			}
			break;
		case 0xE8: /* kPMUreadPmgrRAM - read from internal PMGR RAM */
			if (kPMUStateRecievedCommand == PMUState) {
				if ((3 == PMU_BuffL)
					&& (0 == PMU_BuffA[0])
					&& (0xEE == PMU_BuffA[1])
					&& (1 == PMU_BuffA[2]))
				{
					PMU_BuffA[0] = 1 << 5;
					PmuStartSendResult(0, 1);
				} else {
					PMU_BuffA[0] = 0;
					PmuStartSendResult(0, 1);
					/* ReportAbnormal("Unknown kPMUreadPmgrRAM op"); */
				}
			}
			break;
		case 0x3A: /* kPMUxPramRead - read extended PRAM byte(s) */
			if (kPMUStateRecievedCommand == PMUState) {
				if ((2 == PMU_BuffL)
					&& (PMU_BuffA[0] + PMU_BuffA[1] <= 0x80))
				{
					PMU_p = &PARAMRAM[PMU_BuffA[0]];
					PMU_rem = PMU_BuffA[1];
					PmuStartSendResult(0, PMU_rem);
				} else {
					ReportAbnormalID(0x0E05,
						"Unknown kPMUxPramRead op");
				}
			}
			break;
		case 0x38:
			/* kPMUtimeRead - read the time from the clock chip */
			if (kPMUStateRecievedCommand == PMUState) {
				if (0 == PMU_BuffL) {
					PMU_BuffA[0] = 0;
					PMU_BuffA[1] = 0;
					PMU_BuffA[2] = 0;
					PMU_BuffA[3] = 0;
					PmuStartSendResult(0, 4);
				} else {
					ReportAbnormalID(0x0E06, "Unknown kPMUtimeRead op");
				}
			}
			break;
		case 0x31:
			/*
				kPMUpramWrite - write the original 20 bytes of PRAM
				(Portable only)
			*/
			if (kPMUStateRecievedCommand == PMUState) {
				if (20 == PMU_BuffL) {
					/* done */
				} else {
					ReportAbnormalID(0x0E07,
						"Unknown kPMUpramWrite op");
				}
			} else if (kPMUStateRecievingBuffer == PMUState) {
				if (20 == PMU_BuffL) {
					if (0 == PMU_i) {
						PMU_p = &PARAMRAM[16];
						PMU_rem = 16;
					} else if (16 == PMU_i) {
						PMU_p = &PARAMRAM[8];
						PMU_rem = 4;
					} else {
						ReportAbnormalID(0x0E08,
							"Wrong PMU_i for kPMUpramWrite");
					}
				}
			}
			break;
		case 0x39:
			/*
				kPMUpramRead - read the original 20 bytes of PRAM
				(Portable only)
			*/
			if (kPMUStateRecievedCommand == PMUState) {
				if (0 == PMU_BuffL) {
					PmuStartSendResult(0, 20);
				} else {
					ReportAbnormalID(0x0E09, "Unknown kPMUpramRead op");
				}
			} else if (kPMUStateSendBuffer == PMUState) {
#if 0
				{
					int i;

					for (i = 0; i < PMU_BuffSz; ++i) {
						PMU_BuffA[i] = 0;
					}
				}
#endif
				if (0 == PMU_i) {
					PMU_p = &PARAMRAM[16];
					PMU_rem = 16;
				} else if (16 == PMU_i) {
					PMU_p = &PARAMRAM[8];
					PMU_rem = 4;
				} else {
					ReportAbnormalID(0x0E0A,
						"Wrong PMU_i for kPMUpramRead");
				}
			}
			break;
		default:
			if (kPMUStateRecievedCommand == PMUState) {
				ReportAbnormalID(0x0E0B, "Unknown PMU op");
#if dbglog_HAVE
				dbglog_writeCStr("Unknown PMU op ");
				dbglog_writeHex(PMU_CurCommand);
				dbglog_writeReturn();
				dbglog_writeCStr("PMU_BuffL = ");
				dbglog_writeHex(PMU_BuffL);
				dbglog_writeReturn();
				if (PMU_BuffL <= PMU_BuffSz) {
					int i;

					for (i = 0; i < PMU_BuffL; ++i) {
						dbglog_writeCStr("PMU_BuffA[");
						dbglog_writeNum(i);
						dbglog_writeCStr("] = ");
						dbglog_writeHex(PMU_BuffA[i]);
						dbglog_writeReturn();
					}
				}
#endif
			}
			break;
	}
}

LOCALPROC LocBuffSetUpNextChunk(void)
{
	PMU_p = PMU_BuffA;
	PMU_rem = PMU_BuffL - PMU_i;
	if (PMU_rem >= PMU_BuffSz) {
		PMU_rem = PMU_BuffSz;
	}
}

LOCALFUNC uint8_t GetPMUbus(void)
{
	uint8_t v;

	v = VIA1_iA7;
	v <<= 1;
	v |= VIA1_iA6;
	v <<= 1;
	v |= VIA1_iA5;
	v <<= 1;
	v |= VIA1_iA4;
	v <<= 1;
	v |= VIA1_iA3;
	v <<= 1;
	v |= VIA1_iA2;
	v <<= 1;
	v |= VIA1_iA1;
	v <<= 1;
	v |= VIA1_iA0;

	return v;
}

LOCALPROC SetPMUbus(uint8_t v)
{
	VIA1_iA0 = v & 0x01;
	v >>= 1;
	VIA1_iA1 = v & 0x01;
	v >>= 1;
	VIA1_iA2 = v & 0x01;
	v >>= 1;
	VIA1_iA3 = v & 0x01;
	v >>= 1;
	VIA1_iA4 = v & 0x01;
	v >>= 1;
	VIA1_iA5 = v & 0x01;
	v >>= 1;
	VIA1_iA6 = v & 0x01;
	v >>= 1;
	VIA1_iA7 = v & 0x01;
}

LOCALVAR bool PMU_Sending = false;

LOCALPROC PmuCheckCommandCompletion(void)
{
	if (PMU_i == PMU_BuffL) {
		PMUState = kPMUStateRecievedCommand;
		PmuCheckCommandOp();
		if ((PMU_CurCommand & 0x08) == 0) {
			PMUState = kPMUStateReadyForCommand;
			SetPMUbus(0xFF);
		} else {
			if (PMUState != kPMUStateSendLength) {
				PmuStartSendResult(0xFF, 0);
				PMUState = kPMUStateSendLength;
			}
			PMU_i = 0;
			PMU_Sending = true;
			ICT_add(kICT_PMU_Task,
				20400UL * kCycleScale / 64 * ClockMult);
		}
	}
}

GLOBALPROC PmuToReady_ChangeNtfy(void)
{
	if (PMU_Sending) {
		PMU_Sending = false;
		ReportAbnormalID(0x0E0C,
			"PmuToReady_ChangeNtfy while PMU_Sending");
		PmuFromReady = 0;
	}
	switch (PMUState) {
		case kPMUStateReadyForCommand:
			if (! PmuToReady) {
				PmuFromReady = 0;
			} else {
				PMU_CurCommand = GetPMUbus();
				PMUState = kPMUStateRecievingLength;
				PmuFromReady = 1;
			}
			break;
		case kPMUStateRecievingLength:
			if (! PmuToReady) {
				PmuFromReady = 0;
			} else {
				PMU_BuffL = GetPMUbus();
				PMU_i = 0;
				PMU_rem = 0;
				PMUState = kPMUStateRecievingBuffer;
				PmuCheckCommandCompletion();
				PmuFromReady = 1;
			}
			break;
		case kPMUStateRecievingBuffer:
			if (! PmuToReady) {
				PmuFromReady = 0;
			} else {
				uint8_t v = GetPMUbus();
				if (0 == PMU_rem) {
					PMU_p = nullpr;
					PmuCheckCommandOp();
					if (nullpr == PMU_p) {
						/* default handler */
						LocBuffSetUpNextChunk();
					}
				}
				if (nullpr == PMU_p) {
					/* mini vmac bug if ever happens */
					ReportAbnormalID(0x0E0D,
						"PMU_p null while kPMUStateRecievingBuffer");
				}
				*PMU_p++ = v;
				--PMU_rem;
				++PMU_i;
				PmuCheckCommandCompletion();
				PmuFromReady = 1;
			}
			break;
		case kPMUStateSendLength:
			if (! PmuToReady) {
				/* receiving */
				PmuFromReady = 1;
			} else {
				PMU_SendNext = PMU_BuffL;
				PMUState = kPMUStateSendBuffer;
				PMU_Sending = true;
				ICT_add(kICT_PMU_Task,
					20400UL * kCycleScale / 64 * ClockMult);
			}
			break;
		case kPMUStateSendBuffer:
			if (! PmuToReady) {
				/* receiving */
				PmuFromReady = 1;
			} else {
				if (PMU_i == PMU_BuffL) {
					PMUState = kPMUStateReadyForCommand;
					SetPMUbus(0xFF);
				} else {
					if (0 == PMU_rem) {
						PMU_p = nullpr;
						PmuCheckCommandOp();
						if (nullpr == PMU_p) {
							/* default handler */
							LocBuffSetUpNextChunk();
						}
					}
					PMU_SendNext = *PMU_p++;
					--PMU_rem;
					++PMU_i;
					PMU_Sending = true;
					ICT_add(kICT_PMU_Task,
						20400UL * kCycleScale / 64 * ClockMult);
				}
			}
			break;
	}
}

GLOBALPROC PMU_DoTask(void)
{
	if (PMU_Sending) {
		PMU_Sending = false;
		SetPMUbus(PMU_SendNext);
		PmuFromReady = 0;
	}
}
