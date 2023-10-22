#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <SDL.h>
#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "STRCONST.h"

/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

uint32_t TrueEmulatedTime = 0;

#define InvTimeDivPow 16
#define InvTimeDiv (1 << InvTimeDivPow)
#define InvTimeDivMask (InvTimeDiv - 1)
#define InvTimeStep 1089590 /* 1000 / 60.14742 * InvTimeDiv */

uint32_t LastTime;
uint32_t NextIntTime;
uint32_t NextFracTime;

void IncrNextTime(void)
{
	NextFracTime += InvTimeStep;
	NextIntTime += (NextFracTime >> InvTimeDivPow);
	NextFracTime &= InvTimeDivMask;
}

void InitNextTime(void)
{
	NextIntTime = LastTime;
	NextFracTime = 0;
	IncrNextTime();
}

static uint32_t NewMacDateInSeconds;

bool UpdateTrueEmulatedTime(void)
{
	Uint32 LatestTime;
	int32_t TimeDiff;

	LatestTime = SDL_GetTicks();
	if (LatestTime != LastTime) {

		NewMacDateInSeconds = LatestTime / 1000;
			/* no date and time api in SDL */

		LastTime = LatestTime;
		TimeDiff = (LatestTime - NextIntTime);
			/* this should work even when time wraps */
		if (TimeDiff >= 0) {
			if (TimeDiff > 256) {
				/* emulation interrupted, forget it */
				++TrueEmulatedTime;
				InitNextTime();

#if dbglog_TimeStuff
				dbglog_writelnNum("emulation interrupted",
					TrueEmulatedTime);
#endif
			} else {
				do {
					++TrueEmulatedTime;
					IncrNextTime();
					TimeDiff = (LatestTime - NextIntTime);
				} while (TimeDiff >= 0);
			}
			return true;
		} else {
			if (TimeDiff < -256) {
#if dbglog_TimeStuff
				dbglog_writeln("clock set back");
#endif
				/* clock goofed if ever get here, reset */
				InitNextTime();
			}
		}
	}
	return false;
}


bool CheckDateTime(void)
{
	if (CurMacDateInSeconds != NewMacDateInSeconds) {
		CurMacDateInSeconds = NewMacDateInSeconds;
		return true;
	} else {
		return false;
	}
}

void StartUpTimeAdjust(void)
{
	LastTime = SDL_GetTicks();
	InitNextTime();
}

bool InitLocationDat(void)
{
	LastTime = SDL_GetTicks();
	InitNextTime();
	NewMacDateInSeconds = LastTime / 1000;
	CurMacDateInSeconds = NewMacDateInSeconds;

	return true;
}
