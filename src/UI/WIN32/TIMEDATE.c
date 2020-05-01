#include "OSGLUWIN.h"

/* --- time, date, location --- */

uint32_t TrueEmulatedTime = 0;
LOCALVAR uint32_t TimeSecBase;
LOCALVAR DWORD TimeMilliBase;
LOCALVAR uint32_t NextFracTime;
bool HaveSetTimeResolution = false;

// API wrapper defines

/*
	Timer resolution, as used by timeBeginPeriod(), in milliseconds.
	Setting TimeResolution to 1 seems to drastically slow down
	the clock in Virtual PC 7.0.2 for Mac. Using 3 is more polite
	anyway, and should not cause much observable difference.
	(note that 1/60 of a frame is 16.7 milliseconds)
*/
#ifndef TimeResolution
#define TimeResolution 3
#endif
/* 
	Windows NT: The default precision of the timeGetTime function can be five
	milliseconds or more, depending on the machine. You can use the
	timeBeginPeriod and timeEndPeriod functions to increase the precision of
	timeGetTime. If you do so, the minimum difference between successive values
	returned by timeGetTime can be as large as the minimum period value set using
	timeBeginPeriod and timeEndPeriod. Use the QueryPerformanceCounter and
	QueryPerformanceFrequency functions to measure short time intervals at a high
	resolution. (MSDN for Visual Studio '97)
	(TODO: use QueryPerformanceCounter instead? Always works on WinXP+)
*/
#define GetTimeMillisec timeGetTime

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

bool UpdateTrueEmulatedTime(void)
{
	DWORD LatestTime;
	int32_t TimeDiff;

	LatestTime = GetTimeMillisec();
	if (LatestTime != LastTime) {
		LastTime = LatestTime;
		TimeDiff = (LatestTime - NextIntTime);
			/* this should work even when time wraps */
		if (TimeDiff >= 0) {
			if (TimeDiff > 256) {
				/* emulation interrupted, forget it */
				++TrueEmulatedTime;
				InitNextTime();

#if dbglog_TimeStuff
				dbglog_writelnNum(
					"emulation interrupted",
					TrueEmulatedTime
				);
#endif
			} else {
				do {
					++TrueEmulatedTime;
					IncrNextTime();
					TimeDiff = (LatestTime - NextIntTime);
				} while (TimeDiff >= 0);
			}
			return true;
		} else if (TimeDiff < -256) {
			/* clock goofed if ever get here, reset */
#if dbglog_TimeStuff
			dbglog_writeln("clock set back");
#endif

			InitNextTime();
		}
	}
	return false;
}

// Check that emulated clock equals real clock
bool CheckDateTime(void)
{
	uint32_t NewMacDateInSecond;

	NewMacDateInSecond =
		((uint32_t)(LastTime - TimeMilliBase)) / 1000 + TimeSecBase;
	if (CurMacDateInSeconds != NewMacDateInSecond) {
		CurMacDateInSeconds = NewMacDateInSecond;
		return true;
	} else {
		return false;
	}
}

// Initialize emulated RTC check
bool Init60thCheck(void)
{
	SYSTEMTIME s;
	TIME_ZONE_INFORMATION r;
	DWORD v;
	DWORD t;

	GetLocalTime(&s);
	t = GetTimeMillisec();
	
	TimeSecBase = Date2MacSeconds(
		s.wSecond, s.wMinute, s.wHour,
		s.wDay, s.wMonth, s.wYear
	);
	TimeMilliBase = t - s.wMilliseconds;

	if (AutoTimeZone) {
		v = GetTimeZoneInformation(&r);
		if ((v != 0xFFFFFFFF) && (v != TIME_ZONE_ID_UNKNOWN)) {
			int32_t dlsBias = \
				(v != TIME_ZONE_ID_DAYLIGHT) ? r.StandardBias : r.DaylightBias;
			CurMacDelta = \
				(((uint32_t)(- (r.Bias + dlsBias) * 60)) & 0x00FFFFFF)
				| (((v != TIME_ZONE_ID_DAYLIGHT) ? 0 : 0x80) << 24 );
		}
	}

	LastTime = GetTimeMillisec();
	InitNextTime();

	OnTrueTime = TrueEmulatedTime;

	(void) CheckDateTime();

	return true;
}

void Timer_Suspend(void)
{
	// If using higher-precision timer, stop
	if (HaveSetTimeResolution) {
		(void) timeEndPeriod(TimeResolution);
		HaveSetTimeResolution = false;
	}
}

void Timer_Resume(void)
{
	TIMECAPS tc;

	// Try to use higher-precision timer
	if (
		timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR &&
		(TimeResolution >= tc.wPeriodMin) &&
		(TimeResolution <= tc.wPeriodMax) &&
		timeBeginPeriod(TimeResolution) == TIMERR_NOERROR
	) {
		HaveSetTimeResolution = true;
	}
}
