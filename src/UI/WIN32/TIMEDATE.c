#include "OSGLUWIN.h"

/* --- time, date, location --- */

uint32_t TrueEmulatedTime = 0;
LOCALVAR uint32_t TimeSecBase;
LOCALVAR DWORD TimeMilliBase;
LOCALVAR uint32_t NextFracTime;
bool HaveSetTimeResolution = false;

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

	LatestTime = timeGetTime();
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

bool Init60thCheck(void)
{
	SYSTEMTIME s;
	TIME_ZONE_INFORMATION r;
	DWORD v;
	DWORD t;

	GetLocalTime(&s);
	t = timeGetTime();
	TimeSecBase = Date2MacSeconds(s.wSecond, s.wMinute, s.wHour,
		s.wDay, s.wMonth, s.wYear);
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

	LastTime = timeGetTime();
	InitNextTime();

	OnTrueTime = TrueEmulatedTime;

	(void) CheckDateTime();

	return true;
}

void Timer_Suspend(void)
{
	if (HaveSetTimeResolution) {
		(void) timeEndPeriod(TimeResolution);
		HaveSetTimeResolution = false;
	}
}

void Timer_Resume(void)
{
	TIMECAPS tc;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
	{
		if ((TimeResolution >= tc.wPeriodMin)
			&& (TimeResolution <= tc.wPeriodMax))
		{
			if (timeBeginPeriod(TimeResolution)
				== TIMERR_NOERROR)
			{
				HaveSetTimeResolution = true;
			}
		}
	}
}
