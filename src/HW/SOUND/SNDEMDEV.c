/*
	SNDEMDEV.c

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
	SouND EMulated DEVice

	Emulation of Sound in the Mac Plus could go here.

	This code adapted from "Sound.c" in vMac by Philip Cummins.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"

#include "UI/MYOSGLUE.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"
#include "HW/M68K/MINEM68K.h"
#endif

#include "HW/SOUND/SNDEMDEV.h"


#if SoundEnabled

#define kSnd_Main_Offset   0x0300
#define kSnd_Alt_Offset    0x5F00

#define kSnd_Main_Buffer (kRAM_Size - kSnd_Main_Offset)
#define kSnd_Alt_Buffer (kRAM_Size - kSnd_Alt_Offset)

/*
	approximate volume levels of vMac, so:

	x * vol_mult[SoundVolume] >> 16
		+ vol_offset[SoundVolume]
	= {approx} (x - kCenterSound) / (8 - SoundVolume) + kCenterSound;
*/

static const uint16_t vol_mult[] = {
	8192, 9362, 10922, 13107, 16384, 21845, 32768
};

static const trSoundSamp vol_offset[] = {
#if 3 == kLn2SoundSampSz
	112, 110, 107, 103, 96, 86, 64, 0
#elif 4 == kLn2SoundSampSz
	28672, 28087, 27307, 26215, 24576, 21846, 16384, 0
#else
#error "unsupported kLn2SoundSampSz"
#endif
};

static const uint16_t SubTick_offset[kNumSubTicks] = {
	0,    25,  50,  90, 102, 115, 138, 161,
	185, 208, 231, 254, 277, 300, 323, 346
};

static const uint8_t SubTick_n[kNumSubTicks] = {
	25,   25,  40,  12,  13,  23,  23,  24,
	23,   23,  23,  23,  23,  23,  23,  24
};

/*
	One version of free form sound driver
	spends around 18000 cycles writing
	offsets 50 to 370, then around another 3000
	cycles writing 0 to 50. So be done
	with 0 to 50 at end of second sixtieth.
*/

/*
	Different in system 6.0.4:
	spends around 23500 cycles writing
	offsets 90 to 370, then around another 7500
	cycles writing 0 to 90. This is nastier,
	because gets to be a very small gap
	between where is being read and
	where written. So read a bit in
	advance for third subtick.
*/

/*
	startup sound spends around 19500 cycles
	writing offsets 0 to 370. presumably
	writing offset 0 before it is read.
*/

static uint32_t SoundInvertPhase = 0;
static uint16_t SoundInvertState = 0;

extern uint16_t GetSoundInvertTime(void);

void MacSound_SubTick(int SubTick)
{
	uint16_t actL;
	tpSoundSamp p;
	uint16_t i;
	uint32_t StartOffset = SubTick_offset[SubTick];
	uint16_t n = SubTick_n[SubTick];
	unsigned long addy =
#ifdef SoundBuffer
		(SoundBuffer == 0) ? kSnd_Alt_Buffer :
#endif
		kSnd_Main_Buffer;
#ifndef ln2mtb
	uint8_t * addr = addy + (2 * StartOffset) + RAM;
#else
	CPTR addr = addy + (2 * StartOffset);
#endif
	uint16_t SoundInvertTime = GetSoundInvertTime();
	uint8_t SoundVolume = SoundVolb0
		| (SoundVolb1 << 1)
		| (SoundVolb2 << 2);

#if dbglog_HAVE && 0
	dbglog_StartLine();
	dbglog_writeCStr("reading sound buffer ");
	dbglog_writeHex(StartOffset);
	dbglog_writeCStr(" to ");
	dbglog_writeHex(StartOffset + n);
	dbglog_writeReturn();
#endif

label_retry:
	p = Sound_BeginWrite(n, &actL);
	if (actL > 0) {
		if (SoundDisable && (SoundInvertTime == 0)) {
			for (i = 0; i < actL; i++) {
#if 0
				*p++ = 0x00; /* this is believed more accurate */
#else
				/* But this avoids more clicks. */
				*p++ = kCenterSound;
#endif
			}
		} else {
			for (i = 0; i < actL; i++) {
				/* Copy sound data, high byte of each word */
				*p++ =
#ifndef ln2mtb
					*addr
#else
					get_vm_byte(addr)
#endif
#if 4 == kLn2SoundSampSz
					<< 8
#endif
					;

				/* Move the address on */
				addr += 2;
			}

			if (SoundInvertTime != 0) {
				uint32_t PhaseIncr = (uint32_t)SoundInvertTime * (uint32_t)20;
				p -= actL;

				for (i = 0; i < actL; i++) {
					if (SoundInvertPhase < 704) {
						uint32_t OnPortion = 0;
						uint32_t LastPhase = 0;
						do {
							if (! SoundInvertState) {
								OnPortion +=
									(SoundInvertPhase - LastPhase);
							}
							SoundInvertState = ! SoundInvertState;
							LastPhase = SoundInvertPhase;
							SoundInvertPhase += PhaseIncr;
						} while (SoundInvertPhase < 704);
						if (! SoundInvertState) {
							OnPortion += 704 - LastPhase;
						}
						*p = (*p * OnPortion) / 704;
					} else {
						if (SoundInvertState) {
							*p = 0;
						}
					}
					SoundInvertPhase -= 704;
					p++;
				}
			}
		}

		if (SoundVolume < 7) {
			/*
				Usually have volume at 7, so this
				is just for completeness.
			*/
			uint32_t mult = (uint32_t)vol_mult[SoundVolume];
			trSoundSamp offset = vol_offset[SoundVolume];

			p -= actL;
			for (i = 0; i < actL; i++) {
				*p = (trSoundSamp)((uint32_t)(*p) * mult >> 16) + offset;
				++p;
			}
		}

		Sound_EndWrite(actL);
		n -= actL;
		if (n > 0) {
			goto label_retry;
		}
	}
}

#endif
