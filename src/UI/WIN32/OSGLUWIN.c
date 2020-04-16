/*
	OSGLUWIN.c

	Copyright (C) 2009 Philip Cummins, Weston Pawlowski,
	Bradford L. Barrett, Paul C. Pratt, Fabio Concas

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
	Operating System GLUe for microsoft WINdows

	All operating system dependent code for the
	Microsoft Windows platform should go here.

	This code is descended from Weston Pawlowski's Windows
	port of vMac, by Philip Cummins.

	The main entry point '_tWinMain' is at the end of this file.
*/

#include <windows.h>
#include <time.h>
#include <shlobj.h>
#include <tchar.h>

#include "CNFGRAPI.h"
#include "CNFGGLOB.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"

#include "UI/COMOSGLU.h"
#include "UI/MYOSGLUE.h"
#include "UI/CONTROLM.h"
#include "UI/WIN32/OSGLUWIN.h"


/* --- adapting to API/ABI version differences --- */

#define _CSIDL_APPDATA 0x001a

typedef BOOL (WINAPI *SHGetSpecialFolderPathProcPtr) (
	HWND hwndOwner,
	LPTSTR lpszPath,
	int nFolder,
	BOOL fCreate
);
LOCALVAR SHGetSpecialFolderPathProcPtr MySHGetSpecialFolderPath = NULL;
LOCALVAR bool DidSHGetSpecialFolderPath = false;

LOCALFUNC bool HaveMySHGetSpecialFolderPath(void)
{
	if (! DidSHGetSpecialFolderPath) {
		HMODULE hLibModule = LoadLibrary(TEXT("shell32.dll"));
		if (NULL != hLibModule) {
			MySHGetSpecialFolderPath =
				(SHGetSpecialFolderPathProcPtr)
				GetProcAddress(hLibModule,
#if UseUni
					TEXT("SHGetSpecialFolderPathW")
#else
					TEXT("SHGetSpecialFolderPathA")
#endif
				);
			/* FreeLibrary(hLibModule); */
		}
		DidSHGetSpecialFolderPath = true;
	}
	return (MySHGetSpecialFolderPath != NULL);
}


/* --- end of adapting to API/ABI version differences --- */

#include "STRCONST.h"

#if UseUni
#define NeedCell2UnicodeMap 1
#else
#define NeedCell2WinAsciiMap 1
#endif

#include "LANG/INTLCHAR.h"


LOCALPROC NativeStrFromCStr(LPTSTR r, char *s, bool AddEllipsis)
{
	uint8_t ps[ClStrMaxLength];
	int i;
	int L;

	ClStrFromSubstCStr(&L, ps, s);

	for (i = 0; i < L; ++i) {
		r[i] = (TCHAR)
#if UseUni
			Cell2UnicodeMap[ps[i]];
#else
			Cell2WinAsciiMap[ps[i]];
#endif
	}

	if (AddEllipsis) {
#if UseUni
		r[L++] = 0x2026;
#else
		r[L++] = '.';
		r[L++] = '.';
		r[L++] = '.';
#endif
	}

	r[L] = '\0';
}

LOCALFUNC LPTSTR FindLastTerm(LPTSTR s, TCHAR delim)
{
	TCHAR c;
	LPTSTR p0 = s;
	LPTSTR p = (LPTSTR)nullpr;

	while ((c = *p0++) != (TCHAR)('\0')) {
		if (c == delim) {
			p = p0;
		}
	}

	return p;
}

bool GetAppDir(LPTSTR pathName)
/* be sure at least _MAX_PATH long! */
{
	if (GetModuleFileName(AppInstance, pathName, _MAX_PATH) == 0) {
		/* MacMsg("error", "GetModuleFileName failed", false); */
	} else {
		LPTSTR p = FindLastTerm(pathName, (TCHAR)('\\'));
		if (p == nullpr) {
			/* MacMsg("error", "strrchr failed", false); */
		} else {
			*--p = (TCHAR)('\0');
			return true;
		}
	}
	return false;
}


/* --- some simple utilities --- */

#define TestBit(i, p) (((unsigned long)(i) & PowOf2(p)) != 0)

/* must work even if blocks overlap in memory */
GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memmove((char *)destPtr, (char *)srcPtr, byteCount);
}

/* --- Parameter buffers --- */

#if IncludePbufs
LOCALFUNC MacErr_t PbufNewFromHandle(HGLOBAL h, uint32_t count, tPbuf *r)
{
	tPbuf i;
	MacErr_t err;

	if (!FirstFreePbuf(&i)) {
		(void) GlobalFree(h);
		err = mnvm_miscErr;
	} else {
		*r = i;
		PbufDat[i] = h;
		PbufNewNotify(i, count);
		err = mnvm_noErr;
	}

	return err;
}

GLOBALOSGLUFUNC MacErr_t PbufNew(uint32_t count, tPbuf *r)
{
	HGLOBAL h;
	MacErr_t err = mnvm_miscErr;

	h = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, count);
	if (h != NULL) {
		/* need to clear h */
		err = PbufNewFromHandle(h, count, r);
	}

	return err;
}

GLOBALOSGLUPROC PbufDispose(tPbuf i)
{
	(void) GlobalFree(PbufDat[i]);
	PbufDisposeNotify(i);
}

LOCALPROC UnInitPbufs(void)
{
	tPbuf i;

	for (i = 0; i < NumPbufs; ++i) {
		if (PbufIsAllocated(i)) {
			PbufDispose(i);
		}
	}
}

LOCALFUNC uint8_t * PbufLock(tPbuf i)
{
	HGLOBAL h = PbufDat[i];
	return (uint8_t *)GlobalLock(h);
}

LOCALPROC PbufUnlock(tPbuf i)
{
	(void) GlobalUnlock(PbufDat[i]);
}

GLOBALOSGLUPROC PbufTransfer(uint8_t * Buffer,
	tPbuf i, uint32_t offset, uint32_t count, bool IsWrite)
{
	HGLOBAL h = PbufDat[i];
	uint8_t * p0 = GlobalLock(h);
	if (p0 != NULL) {
		void *p = p0 + offset;
		if (IsWrite) {
			(void) memcpy(p, Buffer, count);
		} else {
			(void) memcpy(Buffer, p, count);
		}
	}
	(void) GlobalUnlock(h);
}
#endif

/* main window info */

HWND MainWnd = NULL;
bool UseFullScreen = (WantInitFullScreen != 0);
bool UseMagnify = (WantInitMagnify != 0);

/* cursor hiding */

bool HaveCursorHidden = false;
bool WantCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		(void) ShowCursor(TRUE);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}
}

/* cursor moving */

bool MouseCaptured = false;

LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
	POINT NewMousePos;
	uint32_t difftime;
	bool IsOk;
	DWORD StartTime = GetTickCount();
	LONG x = h;
	LONG y = v;

	if (UseFullScreen && MayFullScreen) {
		x -= ViewHStart;
		y -= ViewVStart;
	}

	if (UseMagnify) {
		x *= WindowScale;
		y *= WindowScale;
	}

	if (UseFullScreen && MayFullScreen) {
		x += hOffset;
		y += vOffset;
	}

	x += WndX;
	y += WndY;

	do {
		(void) SetCursorPos(x, y);
		if (! GetCursorPos(&NewMousePos)) {
			IsOk = false;
		} else {
			IsOk = (x == NewMousePos.x) && (y == NewMousePos.y);
		}
		difftime = (uint32_t)(GetTickCount() - StartTime);
	} while ((! IsOk) && (difftime < 100));
	return IsOk;
}

LOCALPROC MouseCaptureSet(bool v)
{
	if (v != MouseCaptured) {
		if (v) {
			(void) SetCapture(MainWnd);
		} else {
			(void) ReleaseCapture();
		}
		MouseCaptured = v;
	}
}

void SetCurMouseButton(bool v)
{
	MouseButtonSet(v);
	MouseCaptureSet(v);
}

/* --- priority --- */

#ifndef EnableChangePriority
#define EnableChangePriority MayFullScreen
#endif /* EnableChangePriority */

#if EnableChangePriority
LOCALVAR bool PriorityRaised = false;
#endif

#if EnableChangePriority
LOCALPROC RaisePriority(void)
{
	if (! PriorityRaised) {
		if (! SetPriorityClass(
			GetCurrentProcess(), /* handle to the process */
			HIGH_PRIORITY_CLASS
				/* REALTIME_PRIORITY_CLASS (not, killer) */
				/* priority class value */
			))
		{
			/*
				not recursive:
				MacMsg("SetPriorityClass failed",
					"Sorry, Mini vMac encountered errors"
					" and cannot continue.", true);
			*/
		}
		PriorityRaised = true;
	}
}
#endif

#if EnableChangePriority
LOCALPROC LowerPriority(void)
{
	if (PriorityRaised) {
		if (! SetPriorityClass(
			GetCurrentProcess(),        /* handle to the process */
			NORMAL_PRIORITY_CLASS /* priority class value */
			))
		{
			/*
				not recursive:
				MacMsg("SetPriorityClass failed",
					"Sorry, Mini vMac encountered errors"
					" and cannot continue.", true);
			*/
		}
		PriorityRaised = false;
	}
}
#endif


/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

LOCALVAR uint32_t TrueEmulatedTime = 0;

#define InvTimeDivPow 16
#define InvTimeDiv (1 << InvTimeDivPow)
#define InvTimeDivMask (InvTimeDiv - 1)
#define InvTimeStep 1089590 /* 1000 / 60.14742 * InvTimeDiv */

LOCALVAR DWORD LastTime;

LOCALVAR DWORD NextIntTime;
LOCALVAR uint32_t NextFracTime;

LOCALPROC IncrNextTime(void)
{
	NextFracTime += InvTimeStep;
	NextIntTime += (NextFracTime >> InvTimeDivPow);
	NextFracTime &= InvTimeDivMask;
}

LOCALPROC InitNextTime(void)
{
	NextIntTime = LastTime;
	NextFracTime = 0;
	IncrNextTime();
}

LOCALFUNC bool UpdateTrueEmulatedTime(void)
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

LOCALVAR uint32_t TimeSecBase;
LOCALVAR DWORD TimeMilliBase;

#include "UTIL/DATE2SEC.h"

LOCALFUNC bool CheckDateTime(void)
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

LOCALFUNC bool Init60thCheck(void)
{
	SYSTEMTIME s;
#if AutoTimeZone
	TIME_ZONE_INFORMATION r;
	DWORD v;
#endif
	DWORD t;

	GetLocalTime(&s);
	t = timeGetTime();
	TimeSecBase = Date2MacSeconds(s.wSecond, s.wMinute, s.wHour,
		s.wDay, s.wMonth, s.wYear);
	TimeMilliBase = t - s.wMilliseconds;

#if AutoTimeZone
	v = GetTimeZoneInformation(&r);
	if ((v != 0xFFFFFFFF) && (v != TIME_ZONE_ID_UNKNOWN)) {
		int32_t dlsBias = (v != TIME_ZONE_ID_DAYLIGHT)
			? r.StandardBias : r.DaylightBias;
		CurMacDelta = (((uint32_t)(- (r.Bias + dlsBias) * 60))
			& 0x00FFFFFF)
			| (((v != TIME_ZONE_ID_DAYLIGHT) ? 0 : 0x80)
				<< 24);
	}
#endif

	LastTime = timeGetTime();
	InitNextTime();

	OnTrueTime = TrueEmulatedTime;

	(void) CheckDateTime();

	return true;
}

#ifndef TimeResolution
#define TimeResolution 3
#endif
	/*
		Setting TimeResolution to 1 seems to drastically slow down
		the clock in Virtual PC 7.0.2 for Mac. Using 3 is more polite
		anyway, and should not cause much observable difference.
	*/

#if (TimeResolution != 0)
LOCALVAR bool HaveSetTimeResolution = false;
#endif

#if (TimeResolution != 0)
LOCALPROC Timer_Suspend(void)
{
	if (HaveSetTimeResolution) {
		(void) timeEndPeriod(TimeResolution);
		HaveSetTimeResolution = false;
	}
}
#endif

#if (TimeResolution != 0)
LOCALPROC Timer_Resume(void)
{
	TIMECAPS tc;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS))
		== TIMERR_NOERROR)
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
#endif

/* --- sound --- */

#if SoundEnabled


#define kLn2SoundBuffers 4 /* kSoundBuffers must be a power of two */
#define kSoundBuffers (1 << kLn2SoundBuffers)
#define kSoundBuffMask (kSoundBuffers - 1)

#define DesiredMinFilledSoundBuffs 3
	/*
		if too big then sound lags behind emulation.
		if too small then sound will have pauses.
	*/

#define kLnOneBuffLen 9
#define kLnAllBuffLen (kLn2SoundBuffers + kLnOneBuffLen)
#define kOneBuffLen (1UL << kLnOneBuffLen)
#define kAllBuffLen (1UL << kLnAllBuffLen)
#define kLnOneBuffSz (kLnOneBuffLen + kLn2SoundSampSz - 3)
#define kLnAllBuffSz (kLnAllBuffLen + kLn2SoundSampSz - 3)
#define kOneBuffSz (1UL << kLnOneBuffSz)
#define kAllBuffSz (1UL << kLnAllBuffSz)
#define kOneBuffMask (kOneBuffLen - 1)
#define kAllBuffMask (kAllBuffLen - 1)
#define dbhBufferSize (kAllBuffSz + kOneBuffSz)

#define dbglog_SoundStuff (0 && dbglog_HAVE)
#define dbglog_SoundBuffStats (0 && dbglog_HAVE)

LOCALVAR tpSoundSamp TheSoundBuffer = nullpr;
LOCALVAR uint16_t ThePlayOffset;
LOCALVAR uint16_t TheFillOffset;
LOCALVAR bool wantplaying;
LOCALVAR uint16_t MinFilledSoundBuffs;
LOCALVAR uint16_t TheWriteOffset;

#define SOUND_SAMPLERATE /* 22050 */ 22255
	/* = round(7833600 * 2 / 704) */


LOCALPROC FillWithSilence(tpSoundSamp p, int n, trSoundSamp v)
{
	int i;

	for (i = n; --i >= 0; ) {
		*p++ = v;
	}
}


LOCALVAR HWAVEOUT hWaveOut = NULL;
LOCALVAR WAVEHDR whdr[kSoundBuffers];


LOCALPROC Sound_BeginPlaying(void)
{
#if dbglog_SoundStuff
	fprintf(stderr, "Sound_BeginPlaying\n");
#endif
}

LOCALPROC Sound_Start(void)
{
	if (hWaveOut == NULL) {
		WAVEFORMATEX wfex;
		MMRESULT mmr;
		int i;
		tpSoundSamp p;
		WAVEHDR *pwh;

		wfex.wFormatTag = WAVE_FORMAT_PCM;
		wfex.nChannels = 1;
		wfex.nSamplesPerSec = SOUND_SAMPLERATE;
		wfex.nAvgBytesPerSec = SOUND_SAMPLERATE;
#if 3 == kLn2SoundSampSz
		wfex.nBlockAlign = 1;
		wfex.wBitsPerSample = 8;
#elif 4 == kLn2SoundSampSz
		wfex.nBlockAlign = 2;
		wfex.wBitsPerSample = 16;
#else
#error "unsupported audio format"
#endif
		wfex.cbSize = 0;
		mmr = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfex, 0,
			0 /* (DWORD) AppInstance */, CALLBACK_NULL);
		if (mmr != MMSYSERR_NOERROR) {
			/*
				not recursive:
				MacMsg("waveOutOpen failed",
					"Sorry, Mini vMac encountered errors"
					" and cannot continue.", true);
			*/
		} else {
			p = TheSoundBuffer;
			pwh = whdr;
			for (i = 0; i < kSoundBuffers; ++i) {
				pwh->lpData = (LPSTR)p;
				pwh->dwBufferLength = kOneBuffSz;
				pwh->dwBytesRecorded = 0;
				pwh->dwUser = 0;
				pwh->dwFlags = 0;
				pwh->dwLoops = 0;
				mmr = waveOutPrepareHeader(hWaveOut, pwh,
					sizeof(WAVEHDR));
				if (mmr != MMSYSERR_NOERROR) {
					/*
						not recursive:
						MacMsg("waveOutPrepareHeader failed",
							"Sorry, Mini vMac encountered errors"
							" and cannot continue.", true);
					*/
				} else {
					pwh->dwFlags |= WHDR_DONE;
				}
				p += kOneBuffLen;
				++pwh;
			}

			TheFillOffset = 0;
			ThePlayOffset = 0;
			TheWriteOffset = 0;
			MinFilledSoundBuffs = kSoundBuffers;
			wantplaying = false;
		}
	}
}

LOCALPROC Sound_Stop(void)
{
	MMRESULT mmr;
	int i;

	wantplaying = false;
	if (hWaveOut != NULL) {
		DWORD StartTime = GetTickCount();
		for (i = 0; i < kSoundBuffers; ++i) {
			while (((whdr[i].dwFlags & WHDR_DONE) == 0)
				&& ((uint32_t)(GetTickCount() - StartTime) < 1000))
			{
				Sleep(1);
			}

			mmr = waveOutUnprepareHeader(hWaveOut, &whdr[i],
				sizeof(WAVEHDR));
			if (mmr != MMSYSERR_NOERROR) {
				/*
					not recursive:
					MacMsg("waveOutUnprepareHeader failed",
						"Sorry, Mini vMac encountered errors"
						" and cannot continue.", true);
				*/
			}
		}

		mmr = waveOutClose(hWaveOut);
		if (mmr != MMSYSERR_NOERROR) {
			/*
				MacMsg("waveOutClose failed",
					"Sorry, Mini vMac encountered errors"
					" and cannot continue.", true);
			*/
		}
		hWaveOut = NULL;
	}
}

LOCALPROC SoundCheckVeryOften(void)
{
	if ((hWaveOut != NULL) && (wantplaying)) {
label_retry:
		{
			uint16_t FilledSoundBuffs;
			uint16_t ToPlaySize = TheFillOffset - ThePlayOffset;
			uint16_t CurPlayBuffer =
				(ThePlayOffset >> kLnOneBuffLen) & kSoundBuffMask;

			if ((ToPlaySize > kOneBuffLen)
				&& ((whdr[CurPlayBuffer].dwFlags & WHDR_DONE) != 0))
			{
				ThePlayOffset += kOneBuffLen;
				goto label_retry;
			}
			FilledSoundBuffs = ToPlaySize >> kLnOneBuffLen;

			if (FilledSoundBuffs < MinFilledSoundBuffs) {
				MinFilledSoundBuffs = FilledSoundBuffs;
			}

			if (FilledSoundBuffs < 2) {
				MMRESULT mmr;
				uint16_t PrevPlayOffset = ThePlayOffset - kOneBuffLen;
				uint16_t PrevPlayBuffer =
					(PrevPlayOffset >> kLnOneBuffLen) & kSoundBuffMask;
				uint16_t LastPlayedOffset =
					((TheFillOffset >> kLnOneBuffLen) << kLnOneBuffLen)
						- 1;

				FillWithSilence(
					TheSoundBuffer + (PrevPlayOffset & kAllBuffMask),
					kOneBuffLen,
					*(TheSoundBuffer
						+ (LastPlayedOffset & kAllBuffMask)));
				mmr = waveOutWrite(
					hWaveOut, &whdr[PrevPlayBuffer], sizeof(WAVEHDR));
				if (mmr != MMSYSERR_NOERROR) {
					whdr[PrevPlayBuffer].dwFlags |= WHDR_DONE;
					/*
						not recursive:
						MacMsg("waveOutWrite failed",
							"Sorry, Mini vMac encountered errors"
							" and cannot continue.", true);
					*/
				}
				ThePlayOffset = PrevPlayOffset;
				goto label_retry;
			}
		}
	}
}

#if 4 == kLn2SoundSampSz
LOCALPROC ConvertSoundBlockToNative(tpSoundSamp p)
{
	int i;

	for (i = kOneBuffLen; --i >= 0; ) {
		*p++ -= 0x8000;
	}
}
#else
#define ConvertSoundBlockToNative(p)
#endif

LOCALPROC Sound_FilledBlocks(void)
{
	while (0 != ((TheWriteOffset - TheFillOffset) >> kLnOneBuffLen)) {
		uint16_t CurFillBuffer =
			(TheFillOffset >> kLnOneBuffLen) & kSoundBuffMask;
		bool IsOk = false;

		ConvertSoundBlockToNative((tpSoundSamp)
			whdr[CurFillBuffer].lpData);

		if (hWaveOut != NULL) {
			MMRESULT mmr = waveOutWrite(hWaveOut,
				&whdr[CurFillBuffer], sizeof(WAVEHDR));
			if (mmr == MMSYSERR_NOERROR) {
				IsOk = true;
			}
		}

		if (! IsOk) {
			/*
				not recursive:
				MacMsg("waveOutWrite failed",
					"Sorry, Mini vMac encountered errors"
					" and cannot continue.", true);
			*/
			whdr[CurFillBuffer].dwFlags |= WHDR_DONE;
		}

		TheFillOffset += kOneBuffLen;
	}
}

LOCALPROC Sound_WroteABlock(void)
{
	if (wantplaying) {
		Sound_FilledBlocks();
	} else if (((TheWriteOffset - ThePlayOffset) >> kLnOneBuffLen) < 12)
	{
		/* just wait */
	} else {
		Sound_FilledBlocks();
		wantplaying = true;
		Sound_BeginPlaying();
	}
}

GLOBALOSGLUPROC Sound_EndWrite(uint16_t actL)
{
	TheWriteOffset += actL;

	if (0 == (TheWriteOffset & kOneBuffMask)) {
		/* just finished a block */

		Sound_WroteABlock();
	}
}

GLOBALOSGLUFUNC tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL)
{
	uint16_t ToFillLen = kAllBuffLen - (TheWriteOffset - ThePlayOffset);
	uint16_t WriteBuffContig =
		kOneBuffLen - (TheWriteOffset & kOneBuffMask);

	if (WriteBuffContig < n) {
		n = WriteBuffContig;
	}
	if (ToFillLen < n) {
		/* overwrite previous buffer */
		TheWriteOffset -= kOneBuffLen;
	}

	*actL = n;
	return TheSoundBuffer + (TheWriteOffset & kAllBuffMask);
}

LOCALPROC Sound_SecondNotify(void)
{
	if (hWaveOut != NULL) {
		if (MinFilledSoundBuffs > DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too high");
#endif
			IncrNextTime();
		} else if (MinFilledSoundBuffs < DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too low");
#endif
			++TrueEmulatedTime;
		}
		MinFilledSoundBuffs = kSoundBuffers;
	}
}

#endif

/* --- overall grab --- */

#if MayFullScreen
LOCALPROC GrabTheMachine(void)
{
#if EnableChangePriority
	if ((uint8_t) -1 == SpeedValue) {
		RaisePriority();
	}
#endif
#if EnableGrabSpecialKeys
	GrabSpecialKeys();
#endif
}
#endif

#if MayFullScreen
LOCALPROC UnGrabTheMachine(void)
{
#if EnableGrabSpecialKeys
	UnGrabSpecialKeys();
#endif
#if EnableChangePriority
	LowerPriority();
#endif
}
#endif

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if MayFullScreen
LOCALPROC AdjustMachineGrab(void)
{
	if (GrabMachine) {
		if (MainWnd != NULL) {
			GrabTheMachine();
		}
	} else {
		UnGrabTheMachine();
	}
}
#endif

/* --- basic dialogs --- */

LOCALPROC MyBeginDialog(void)
{
	DisconnectKeyCodes3();
#if MayFullScreen
	GrabMachine = false;
	UnGrabTheMachine();
#endif
	ForceShowCursor();
}

LOCALPROC MyEndDialog(void)
{
	ReconnectKeyCodes3();
}

LOCALPROC CheckSavedMacMsg(void)
{
	if (nullpr != SavedBriefMsg) {
		TCHAR briefMsg0[ClStrMaxLength + 1];
		TCHAR longMsg0[ClStrMaxLength + 1];

		NativeStrFromCStr(briefMsg0, SavedBriefMsg, false);
		NativeStrFromCStr(longMsg0, SavedLongMsg, false);

		MessageBox(MainWnd, longMsg0, briefMsg0,
			MB_APPLMODAL | MB_OK | (SavedFatalMsg ? MB_ICONSTOP : 0));

		SavedBriefMsg = nullpr;
	}
}

/* --- main window --- */

enum {
	ID_MENU_NULL = 256,
	ID_FILE_INSERTDISK1,
	ID_FILE_QUIT,
	ID_SPECIAL_MORECOMMANDS,
	ID_HELP_ABOUT,

	kNum_ID_MENU
};


#define EnableScalingBuff 1
LOCALVAR uint8_t * ScalingBuff = NULL;

LOCALVAR HDC MainWndDC = NULL;

LOCALVAR int32_t CmdShow;

LOCALVAR TCHAR WndTitle[_MAX_PATH];
LOCALVAR const TCHAR WndClassName[] = TEXT(kStrAppName);

LOCALVAR bool gBackgroundFlag = false;
LOCALVAR bool gTrueBackgroundFlag = false;
LOCALVAR bool CurSpeedStopped = true;

LOCALPROC GetWndTitle(void)
{
	TCHAR pathName[_MAX_PATH];
	WIN32_FIND_DATA fd;
	bool IsOk = false;

	if (GetModuleFileName(AppInstance, pathName, _MAX_PATH) != 0) {
		HANDLE hf = FindFirstFile(pathName, &fd);

		if (hf != INVALID_HANDLE_VALUE) {
			/* get rid of extension, presumably '.exe' */
			LPTSTR p = FindLastTerm(fd.cFileName,
				(TCHAR)('.'));
			if (p != nullpr) {
				*--p = (TCHAR)('\0');
			}

			_tcscpy(WndTitle, fd.cFileName);
			IsOk = true;
			FindClose(hf);
		}
	}
	if (! IsOk) {
		_tcscpy(WndTitle, TEXT(kStrAppName));
	}
}

LOCALPROC DisposeMainWindow(void)
{
	if (MainWndDC != NULL) {
		ReleaseDC(MainWnd, MainWndDC);
	}
	if (MainWnd != NULL) {
		DestroyWindow(MainWnd);
		MainWnd = NULL; /* so MacMsg will still work */
	}
}

enum {
	kMagStateNormal,
#if 1
	kMagStateMagnifgy,
#endif
	kNumMagStates
};

#define kMagStateAuto kNumMagStates

#if MayNotFullScreen
LOCALVAR int CurWinIndx;
LOCALVAR bool HavePositionWins[kNumMagStates];
LOCALVAR POINT WinPositionWins[kNumMagStates];
#endif

#if MayNotFullScreen
LOCALPROC AppendConvertMenuItem(HMENU hMenu,
	UINT uIDNewItem, char *s, bool AddEllipsis)
{
	TCHAR ts[ClStrMaxLength + 1];

	NativeStrFromCStr(ts, s, AddEllipsis);

	(void) AppendMenu(hMenu, MF_ENABLED + MF_STRING,
		uIDNewItem, ts);
}
#endif

#if MayNotFullScreen
LOCALPROC AppendSubmenuConvertName(HMENU hMenu,
	HMENU hSubMenu, char *s)
{
	TCHAR ts[ClStrMaxLength + 1];
	MENUITEMINFO mii;

	NativeStrFromCStr(ts, s, false);

#if 0
	(void) InsertMenu(hMenu, 0xFFFFFFFF,
		MF_BYPOSITION + MF_POPUP + MF_STRING + MF_ENABLED,
		(UINT)hSubMenu, ts);
#endif

	memset(&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mii.fType = MFT_STRING;
	mii.hSubMenu = hSubMenu;
	mii.dwTypeData = ts;
	mii.cch = (UINT)_tcslen(ts);
	(void) InsertMenuItem(hMenu, (UINT) -1, TRUE,
		&mii);
}
#endif

#ifndef kStrMenuFile_win
#define kStrMenuFile_win kStrMenuFile
#endif

LOCALFUNC bool ReCreateMainWindow(void)
{
#if MayNotFullScreen
	HMENU m;
	int DfltWndX;
	int DfltWndY;
	int WinIndx;
#endif
	HMENU mb;
	HWND NewMainWindow;
	HDC NewMainWndDC = NULL;
	int ScreenX = GetSystemMetrics(SM_CXSCREEN);
	int ScreenY = GetSystemMetrics(SM_CYSCREEN);
	short NewWindowHeight = vMacScreenHeight;
	short NewWindowWidth = vMacScreenWidth;
	HWND OldMainWindow = MainWnd;
	HDC OldMainWndDC = MainWndDC;
	RECT NewWinR;
	DWORD WStyle;
	DWORD WExStyle;

#if 1
	if (! UseFullScreen)
#endif
#if MayNotFullScreen
	{
		/* save old position */
		if (OldMainWindow != NULL) {
			WinPositionWins[CurWinIndx].x = WndX;
			WinPositionWins[CurWinIndx].y = WndY;
		}
	}
#endif

#if MayNotFullScreen
#if 1
	if (WantMagnify) {
		WinIndx = kMagStateMagnifgy;
	} else
#endif
	{
		WinIndx = kMagStateNormal;
	}
#endif

#if 1
	if (WantMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}
#endif

#if 1
	if (WantFullScreen)
#endif
#if MayFullScreen
	{
		WStyle = WS_VISIBLE | WS_POPUP;
		WExStyle = WS_EX_TOPMOST;

		hOffset = (ScreenX - NewWindowWidth) / 2;
		vOffset = (ScreenY - NewWindowHeight) / 2;
		if (hOffset < 0) {
			hOffset = 0;
		}
		if (vOffset < 0) {
			vOffset = 0;
		}

		NewWinR.left = 0;
		NewWinR.top = 0;
		NewWinR.right = ScreenX;
		NewWinR.bottom = ScreenY;
	}
#endif
#if 1
	else
#endif
#if MayNotFullScreen
	{
		WStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
			| WS_MINIMIZEBOX;
		WExStyle = WS_EX_ACCEPTFILES;

		DfltWndX = (ScreenX - NewWindowWidth) / 2;
		DfltWndY = (ScreenY - NewWindowHeight) / 2;

		if (DfltWndX < 0) {
			DfltWndX = 0;
		}
		if (DfltWndY < 0) {
			DfltWndY = 0;
		}

		if (! HavePositionWins[WinIndx]) {
			WinPositionWins[WinIndx].x = DfltWndX;
			WinPositionWins[WinIndx].y = DfltWndY;
			HavePositionWins[WinIndx] = true;
		}

		NewWinR.left = WinPositionWins[WinIndx].x;
		NewWinR.top = WinPositionWins[WinIndx].y;
		NewWinR.right = NewWinR.left + NewWindowWidth;
		NewWinR.bottom = NewWinR.top + NewWindowHeight;

		(void) AdjustWindowRectEx(&NewWinR, WStyle, TRUE, WExStyle);

		if ((NewWinR.right <= 0)
			|| (NewWinR.left >= ScreenX)
			|| (NewWinR.bottom <= 0)
			|| (NewWinR.top >= ScreenY))
		{
			NewWinR.left = DfltWndX;
			NewWinR.top = DfltWndY;
			NewWinR.right = DfltWndX + NewWindowWidth;
			NewWinR.bottom = DfltWndY + NewWindowHeight;

			(void) AdjustWindowRectEx(&NewWinR,
				WStyle, TRUE, WExStyle);
		}
	}
#endif

	if ((OldMainWindow == NULL)
#if 1
		|| (WantFullScreen != UseFullScreen)
#endif
		)
	{

#if 1
		if (WantFullScreen)
#endif
#if MayFullScreen
		{
			mb = NULL;
		}
#endif
#if 1
		else
#endif
#if MayNotFullScreen
		{
			mb = CreateMenu();
			if (mb != NULL) {
				m = CreateMenu();
				if (m != NULL) {
					AppendConvertMenuItem(m, ID_FILE_INSERTDISK1,
						kStrMenuItemOpen, true);
					(void) AppendMenu(m, MF_SEPARATOR, 0, NULL);
					AppendConvertMenuItem(m, ID_FILE_QUIT,
						kStrMenuItemQuit, false);
					AppendSubmenuConvertName(mb, m, kStrMenuFile_win);
				}
				m = CreateMenu();
				if (m != NULL) {
					AppendConvertMenuItem(m, ID_SPECIAL_MORECOMMANDS,
						kStrMenuItemMore, true);
					AppendSubmenuConvertName(mb, m, kStrMenuSpecial);
				}
				m = CreateMenu();
				if (m != NULL) {
					AppendConvertMenuItem(m, ID_HELP_ABOUT,
						kStrMenuItemAbout, true);
					AppendSubmenuConvertName(mb, m, kStrMenuHelp);
				}
			}
		}
#endif

		NewMainWindow = CreateWindowEx(
			WExStyle,
			WndClassName,
			WndTitle,
			WStyle,
			NewWinR.left, NewWinR.top,
			NewWinR.right - NewWinR.left, NewWinR.bottom - NewWinR.top,
			NULL,
			mb,
			AppInstance, NULL);
		if (NewMainWindow == NULL) {
			MacMsg("CreateWindow failed",
				"Sorry, Mini vMac encountered errors"
				" and cannot continue.", true);
			return false;
		}

		NewMainWndDC = GetDC(NewMainWindow);
		if (NewMainWndDC == NULL) {
			MacMsg("GetDC failed",
				"Sorry, Mini vMac encountered errors"
				" and cannot continue.", true);
			DestroyWindow(NewMainWindow);
			return false;
		}
	} else {
		NewMainWndDC = OldMainWndDC;
		NewMainWindow = OldMainWindow;
		(void) MoveWindow(NewMainWindow, NewWinR.left, NewWinR.top,
			NewWinR.right - NewWinR.left, NewWinR.bottom - NewWinR.top,
			TRUE);
	}

	if (0 != vMacScreenDepth) {
		ColorModeWorks = true;
	}

	{
		POINT p;

		/*
			Find out where the window really went, on
			the off chance that the WM_MOVE message wasn't
			called on CreateWindowEx/MoveWindow, or that
			the window wasn't put where asked for.
		*/
		p.x = 0;
		p.y = 0;
		(void) MapWindowPoints(NewMainWindow, NULL, &p, 1);
		WndX = (int16_t)p.x;
		WndY = (int16_t)p.y;
	}

#if MayFullScreen
	GrabMachine = false;
	UnGrabTheMachine();
#endif

#if MayNotFullScreen
	CurWinIndx = WinIndx;
#endif

	MainWnd = NewMainWindow;
	MainWndDC = NewMainWndDC;
	gTrueBackgroundFlag = false;
	UseFullScreen = WantFullScreen;
	UseMagnify = WantMagnify;

	if (UseFullScreen)
#if MayFullScreen
	{
		ViewHSize = ScreenX;
		ViewVSize = ScreenY;
		if (UseMagnify) {
			ViewHSize /= WindowScale;
			ViewVSize /= WindowScale;
		}
		if (ViewHSize >= vMacScreenWidth) {
			ViewHStart = 0;
			ViewHSize = vMacScreenWidth;
		} else {
			ViewHSize &= ~ 1;
		}
		if (ViewVSize >= vMacScreenHeight) {
			ViewVStart = 0;
			ViewVSize = vMacScreenHeight;
		} else {
			ViewVSize &= ~ 1;
		}
	}
#endif

	if (NewMainWindow != OldMainWindow) {
		ShowWindow(NewMainWindow, SW_SHOW /* CmdShow */);
		if (OldMainWndDC != NULL) {
			ReleaseDC(MainWnd, OldMainWndDC);
		}
		if (OldMainWindow != NULL) {
			/* ShowWindow(OldMainWindow, SW_HIDE); */
			DestroyWindow(OldMainWindow);
		}

		DisconnectKeyCodes3();
			/* since key events per window */
	} else {
		(void) InvalidateRgn(MainWnd, NULL, FALSE);
	}
	if (HaveCursorHidden) {
		(void) MoveMouse(CurMouseH, CurMouseV);
		WantCursorHidden = true;
	}

	return true;
}

typedef struct BITMAPINFOHEADER256 {
	BITMAPINFOHEADER bmi;
	RGBQUAD colors[CLUT_size];
	//RGBQUAD colors[2];
} BITMAPINFOHEADER256;

#define ScaledHeight (WindowScale * vMacScreenHeight)
#define ScaledWidth (WindowScale * vMacScreenWidth)

LOCALPROC HaveChangedScreenBuff(int16_t top, int16_t left,
	int16_t bottom, int16_t right)
{
	BITMAPINFOHEADER256 bmh;
	uint8_t *cdb = GetCurDrawBuff();
	int XDest;
	int YDest;

	if (UseFullScreen)
#if MayFullScreen
	{
		if (top < ViewVStart) {
			top = ViewVStart;
		}
		if (left < ViewHStart) {
			left = ViewHStart;
		}
		if (bottom > ViewVStart + ViewVSize) {
			bottom = ViewVStart + ViewVSize;
		}
		if (right > ViewHStart + ViewHSize) {
			right = ViewHStart + ViewHSize;
		}

		if ((top >= bottom) || (left >= right)) {
			goto label_exit;
		}
	}
#endif

	XDest = left;
	YDest = top;

	if (UseFullScreen)
#if MayFullScreen
	{
		XDest -= ViewHStart;
		YDest -= ViewVStart;
	}
#endif

	if (UseMagnify) {
		XDest *= WindowScale;
		YDest *= WindowScale;
	}
	if (UseFullScreen)
#if MayFullScreen
	{
		XDest += hOffset;
		YDest += vOffset;
	}
#endif

	if (vMacScreenDepth > 0 && UseColorMode) {
		int i;
		int nDestWidth = (right - left);
		int nDestHeight = (bottom - top);
		uint8_t *p;
		if (vMacScreenDepth == 1) {
			p = ScalingBuff + ((uint32_t)vMacScreenWidth / 4) * top;
		} else if (vMacScreenDepth >= 4) {
			p = ScalingBuff + (uint32_t)vMacScreenByteWidth * top;
		} else {
			p = cdb + (uint32_t)vMacScreenByteWidth * top;
		}

		memset(&bmh, 0, sizeof (bmh));
		bmh.bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmh.bmi.biWidth = vMacScreenWidth;
		bmh.bmi.biHeight = - (bottom - top);
		bmh.bmi.biPlanes = 1;
		if (1 == vMacScreenDepth) {
			bmh.bmi.biBitCount = 4;
		} else {
			bmh.bmi.biBitCount = (1 << vMacScreenDepth);
		}
		bmh.bmi.biCompression= BI_RGB;
		bmh.bmi.biSizeImage = 0;
		bmh.bmi.biXPelsPerMeter = 0;
		bmh.bmi.biYPelsPerMeter = 0;
		if (vMacScreenDepth == 1) {
			bmh.bmi.biClrUsed = 4;
		} else {
			bmh.bmi.biClrUsed = 0;
		}
		bmh.bmi.biClrImportant = 0;

		if (vMacScreenDepth < 4) {
			for (i = 0; i < CLUT_size; ++i) {
				bmh.colors[i].rgbRed = CLUT_reds[i] >> 8;
				bmh.colors[i].rgbGreen = CLUT_greens[i] >> 8;
				bmh.colors[i].rgbBlue = CLUT_blues[i] >> 8;
				bmh.colors[i].rgbReserved = 0;
			}
		}

		if (1 == vMacScreenDepth) {
			int j;
			uint8_t *p1 = (uint8_t *)(cdb + (uint32_t)vMacScreenByteWidth * top);
			uint16_t *p2 = (uint16_t *)p;
			for (i = bottom - top; --i >= 0; ) {
				for (j = vMacScreenWidth / 4; --j >= 0; ) {
					uint16_t t0 = *p1++;
					*p2 ++
						= ((t0 & 0xC0) >> 2)
						| ((t0 & 0x30) >> 4)
						| ((t0 & 0x0C) << 10)
						| ((t0 & 0x03) << 8);
				}
			}
		} else if (4 == vMacScreenDepth) {
			int j;
			uint16_t *p1 = (uint16_t *)(cdb + (uint32_t)vMacScreenByteWidth * top);
			uint16_t *p2 = (uint16_t *)p;
			for (i = bottom - top; --i >= 0; ) {
				for (j = vMacScreenWidth; --j >= 0; ) {
					uint16_t t0 = *p1++;
					*p2 ++ =
						((t0 & 0xFF00) >> 8) | ((t0 & 0x00FF) << 8);
				}
			}
		} else if (5 == vMacScreenDepth) {
			int j;
			uint32_t *p1 = (uint32_t *)(cdb + (uint32_t)vMacScreenByteWidth * top);
			uint32_t *p2 = (uint32_t *)p;
			for (i = bottom - top; --i >= 0; ) {
				for (j = vMacScreenWidth; --j >= 0; ) {
					uint32_t t0 = *p1++;
					*p2++
						= ((t0 & 0xFF000000) >> 24)
						| ((t0 & 0x00FF0000) >> 8)
						| ((t0 & 0x0000FF00) << 8)
						| ((t0 & 0x000000FF) << 24);
				}
			}
		}

		if (UseMagnify) {
			nDestWidth *= WindowScale;
			nDestHeight *= WindowScale;
		}

		if (StretchDIBits(
			MainWndDC, /* handle of device context */
			XDest,
				/* x-coordinate of upper-left corner of dest. rect. */
			YDest,
				/* y-coordinate of upper-left corner of dest. rect. */
			nDestWidth, /* dest. rectangle width */
			nDestHeight, /* dest. rectangle height */
			left,
				/* x-coordinate of lower-left corner of source rect. */
			0, /* y-coordinate of lower-left corner of source rect. */
			(right - left), /* source rectangle width */
			(bottom - top), /* source rectangle height */
			(CONST VOID *)p, /* address of array with DIB bits */
			(const struct tagBITMAPINFO *)&bmh,
				/* address of structure with bitmap info. */
			DIB_RGB_COLORS, /* RGB or palette indices */
			SRCCOPY
		) == 0) {
			/* ReportWinLastError(); */
		}
	} else {
		uint8_t *p = cdb + (uint32_t)vMacScreenMonoByteWidth * top;

		memset(&bmh, 0, sizeof (bmh));
		bmh.bmi.biSize = sizeof(BITMAPINFOHEADER);
		bmh.bmi.biWidth = vMacScreenWidth;
		bmh.bmi.biHeight = - (bottom - top);
		bmh.bmi.biPlanes = 1;
		bmh.bmi.biBitCount = 1;
		bmh.bmi.biCompression= BI_RGB;
		bmh.bmi.biSizeImage = 0;
		bmh.bmi.biXPelsPerMeter = 0;
		bmh.bmi.biYPelsPerMeter = 0;
		bmh.bmi.biClrUsed = 0;
		bmh.bmi.biClrImportant = 0;
		bmh.colors[0].rgbRed = 255;
		bmh.colors[0].rgbGreen = 255;
		bmh.colors[0].rgbBlue = 255;
		bmh.colors[0].rgbReserved = 0;
		bmh.colors[1].rgbRed = 0;
		bmh.colors[1].rgbGreen = 0;
		bmh.colors[1].rgbBlue = 0;
		bmh.colors[1].rgbReserved = 0;

		if (UseMagnify) {
#if EnableScalingBuff
			if (ScalingBuff != NULL) {
				int i;
				int j;
				int k;
				uint16_t left1 = left & (~ 7);
				uint16_t right1 = (right + 7) & (~ 7);
				uint16_t jn = (right1 - left1) / 8;
				uint8_t *p1 =
					cdb + ((left1 + vMacScreenWidth * (uint32_t)top) / 8);
				uint8_t *p2 = ScalingBuff
					/*
						+ ((left1 + vMacScreenWidth * WindowScale
								* (uint32_t)top)
							* WindowScale / 8)
					*/
					;
				uint8_t *p3;
				uint8_t t0;
				uint8_t t1;
				uint8_t t2;
				uint8_t m;

				for (i = bottom - top; --i >= 0; ) {
					p3 = p2;
					for (j = jn; --j >= 0; ) {
						t0 = *p1++;
						t1 = t0;
						m = 0x80;
						t2 = 0;
						for (k = 4; --k >= 0; ) {
							t2 |= t1 & m;
							t1 >>= 1;
							m >>= 2;
						}
						*p2++ = t2 | (t2 >> 1);

						t1 = t0 << 4;
						m = 0x80;
						t2 = 0;
						for (k = 4; --k >= 0; ) {
							t2 |= t1 & m;
							t1 >>= 1;
							m >>= 2;
						}
						*p2++ = t2 | (t2 >> 1);
					}
					p1 += vMacScreenWidth / 8 - jn;
					p2 += ScaledWidth / 8 - (WindowScale * jn);
					for (j = WindowScale * jn; --j >= 0; ) {
						*p2++ = *p3++;
					}
					p2 += ScaledWidth / 8 - (WindowScale * jn);
				}

				bmh.bmi.biWidth = vMacScreenWidth * WindowScale;
				bmh.bmi.biHeight = - ((bottom - top) * WindowScale);
				if (SetDIBitsToDevice(
					MainWndDC, /* handle of device context */
					XDest,
						/*
							x-coordinate of upper-left corner
							of dest. rect.
						*/
					YDest,
						/*
							y-coordinate of upper-left corner
							of dest. rect.
						*/
					(right - left) * WindowScale,
						/* source rectangle width */
					(bottom - top) * WindowScale,
						/* source rectangle height */
					(left & 7) * WindowScale,
						/*
							x-coordinate of lower-left corner
							of source rect.
						*/
					0,
						/*
							y-coordinate of lower-left corner
							of source rect.
						*/
					0, /* first scan line in array */
					(bottom - top) * WindowScale,
						/* number of scan lines */
					(CONST VOID *)ScalingBuff,
						/* address of array with DIB bits */
					(const struct tagBITMAPINFO *)&bmh,
						/* address of structure with bitmap info. */
					DIB_RGB_COLORS /* RGB or palette indices */
				) == 0) {
					/* ReportWinLastError(); */
				}
			}
		} else
#endif

		{
			if (SetDIBitsToDevice(
				MainWndDC, /* handle of device context */
				XDest,
					/*
						x-coordinate of upper-left corner of dest. rect.
					*/
				YDest,
					/*
						y-coordinate of upper-left corner of dest. rect.
					*/
				(right - left), /* source rectangle width */
				(bottom - top), /* source rectangle height */
				left,
					/*
						x-coordinate of lower-left corner
						of source rect.
					*/
				0,
					/*
						y-coordinate of lower-left corner
						of source rect.
					*/
				0, /* first scan line in array */
				(bottom - top), /* number of scan lines */
				(CONST VOID *)p, /* address of array with DIB bits */
				(const struct tagBITMAPINFO *)&bmh,
					/* address of structure with bitmap info. */
				DIB_RGB_COLORS /* RGB or palette indices */
			) == 0) {
				/* ReportWinLastError(); */
			}
		}
	}

#if MayFullScreen
label_exit:
	;
#endif
}

LOCALPROC Screen_DrawAll(void)
{
	HaveChangedScreenBuff(0, 0, vMacScreenHeight, vMacScreenWidth);
}

LOCALPROC DrawChangesAndClear(void)
{
	if (ScreenChangedBottom > ScreenChangedTop) {
		HaveChangedScreenBuff(ScreenChangedTop, ScreenChangedLeft,
			ScreenChangedBottom, ScreenChangedRight);
		ScreenClearChanges();
	}
}

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
	DrawChangesAndClear();
}

LOCALFUNC bool InitTheCursor(void)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return true;
}

LOCALPROC MousePositionNotify(LONG NewMousePosx, LONG NewMousePosy)
{
	bool ShouldHaveCursorHidden = true;

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePosx -= hOffset;
		NewMousePosy -= vOffset;
	}
#endif

#if 1
	if (UseMagnify) {
		NewMousePosx /= WindowScale;
		NewMousePosy /= WindowScale;
	}
#endif

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePosx += ViewHStart;
		NewMousePosy += ViewVStart;
	}
#endif
	{
		if (NewMousePosx < 0) {
			NewMousePosx = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosx > vMacScreenWidth) {
			NewMousePosx = vMacScreenWidth - 1;
			ShouldHaveCursorHidden = false;
		}
		if (NewMousePosy < 0) {
			NewMousePosy = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosy > vMacScreenHeight) {
			NewMousePosy = vMacScreenHeight - 1;
			ShouldHaveCursorHidden = false;
		}

#if 1
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			ShouldHaveCursorHidden = true;
		}
#endif

		/* if (ShouldHaveCursorHidden || CurMouseButton) */
		/*
			for a game like arkanoid, would like mouse to still
			move even when outside window in one direction
		*/
		MousePositionSet(NewMousePosx, NewMousePosy);
	}

	WantCursorHidden = ShouldHaveCursorHidden;
}

LOCALPROC CheckMouseState(void)
{
	POINT NewMousePos;

	GetCursorPos(&NewMousePos);
	NewMousePos.x -= WndX;
	NewMousePos.y -= WndY;
	MousePositionNotify(NewMousePos.x, NewMousePos.y);
}

LOCALVAR const uint8_t Native2MacRomanTab[] = {
	0xAD, 0xB0, 0xE2, 0xC4, 0xE3, 0xC9, 0xA0, 0xE0,
	0xF6, 0xE4, 0xB6, 0xDC, 0xCE, 0xB2, 0xB3, 0xB7,
	0xB8, 0xD4, 0xD5, 0xD2, 0xD3, 0xA5, 0xD0, 0xD1,
	0xF7, 0xAA, 0xC5, 0xDD, 0xCF, 0xB9, 0xC3, 0xD9,
	0xCA, 0xC1, 0xA2, 0xA3, 0xDB, 0xB4, 0xBA, 0xA4,
	0xAC, 0xA9, 0xBB, 0xC7, 0xC2, 0xBD, 0xA8, 0xF8,
	0xA1, 0xB1, 0xC6, 0xD7, 0xAB, 0xB5, 0xA6, 0xE1,
	0xFC, 0xDA, 0xBC, 0xC8, 0xDE, 0xDF, 0xF0, 0xC0,
	0xCB, 0xE7, 0xE5, 0xCC, 0x80, 0x81, 0xAE, 0x82,
	0xE9, 0x83, 0xE6, 0xE8, 0xED, 0xEA, 0xEB, 0xEC,
	0xF5, 0x84, 0xF1, 0xEE, 0xEF, 0xCD, 0x85, 0xF9,
	0xAF, 0xF4, 0xF2, 0xF3, 0x86, 0xFA, 0xFB, 0xA7,
	0x88, 0x87, 0x89, 0x8B, 0x8A, 0x8C, 0xBE, 0x8D,
	0x8F, 0x8E, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,
	0xFD, 0x96, 0x98, 0x97, 0x99, 0x9B, 0x9A, 0xD6,
	0xBF, 0x9D, 0x9C, 0x9E, 0x9F, 0xFE, 0xFF, 0xD8
};

#if IncludePbufs
LOCALFUNC MacErr_t NativeTextToMacRomanPbuf(HGLOBAL x, tPbuf *r)
{
#if UseUni
#define UnsignedChar uint16_t
#else
#define UnsignedChar uint8_t
#endif
	HGLOBAL h;
	LPTSTR p1;
	uint32_t n;
	UnsignedChar v;
	MacErr_t err = mnvm_miscErr;

	p1 = GlobalLock(x);
	if (p1 != NULL) {
		n = 0;
		while ((v = *p1++) != 0) {
			if (v != 10) {
				++n;
			}
		}
		(void) GlobalUnlock(x);

		h = GlobalAlloc(GMEM_DDESHARE, n);
		if (h != NULL) {
			p1 = GlobalLock(x);
			if (p1 != NULL) {
				uint8_t *p2 = GlobalLock(h);
				if (p2 != NULL) {
					while ((v = (UnsignedChar)*p1++) != 0) {
						if (v >= 128) {
							*p2++ = Native2MacRomanTab[v & 0x7F];
								/*
									if UseUni, then for gives
									garbage for v > 256.
								*/
						} else if (v != 10) {
							*p2++ = v;
						}
					}

					err = mnvm_noErr;

					(void) GlobalUnlock(h);
				}
				(void) GlobalUnlock(x);
			}

			if (mnvm_noErr != err) {
				(void) GlobalFree(h);
			} else {
				err = PbufNewFromHandle(h, n, r);
			}
		}
	}

	return err;
}
#endif

LOCALVAR const uint8_t MacRoman2NativeTab[] = {
	0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1,
	0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
	0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3,
	0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
	0x86, 0xB0, 0xA2, 0xA3, 0xA7, 0x95, 0xB6, 0xDF,
	0xAE, 0xA9, 0x99, 0xB4, 0xA8, 0x80, 0xC6, 0xD8,
	0x81, 0xB1, 0x8D, 0x8E, 0xA5, 0xB5, 0x8A, 0x8F,
	0x90, 0x9D, 0xA6, 0xAA, 0xBA, 0xAD, 0xE6, 0xF8,
	0xBF, 0xA1, 0xAC, 0x9E, 0x83, 0x9A, 0xB2, 0xAB,
	0xBB, 0x85, 0xA0, 0xC0, 0xC3, 0xD5, 0x8C, 0x9C,
	0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xF7, 0xB3,
	0xFF, 0x9F, 0xB9, 0xA4, 0x8B, 0x9B, 0xBC, 0xBD,
	0x87, 0xB7, 0x82, 0x84, 0x89, 0xC2, 0xCA, 0xC1,
	0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xD3, 0xD4,
	0xBE, 0xD2, 0xDA, 0xDB, 0xD9, 0xD0, 0x88, 0x98,
	0xAF, 0xD7, 0xDD, 0xDE, 0xB8, 0xF0, 0xFD, 0xFE
};

#if IncludePbufs
LOCALFUNC bool MacRomanTextToNativeHand(tPbuf Pbuf_no,
	bool IsFileName, HGLOBAL *r)
{
	HGLOBAL h;
	uint32_t i;
	uint32_t rn = 0;
	HGLOBAL bh = PbufDat[Pbuf_no];
	uint32_t L = PbufSize[Pbuf_no];
	bool IsOk = false;

	if (IsFileName) {
		if (L > 255) {
			L = 255;
		}
	} else {
		uint8_t *Buffer = (uint8_t *)GlobalLock(bh);
		if (Buffer != NULL) {
			for (i = 0; i < L; ++i) {
				if (Buffer[i] == 13) {
					++rn;
				}
			}
			(void) GlobalUnlock(bh);
		}
	}

	h = GlobalAlloc(GMEM_DDESHARE, (L + rn + 1) * sizeof(TCHAR));
	if (h != NULL) {
		uint8_t *Buffer = (uint8_t *)GlobalLock(bh);
		if (Buffer != NULL) {
			LPTSTR p1 = GlobalLock(h);
			if (p1 != NULL) {
				for (i = 0; i < L; ++i) {
					TCHAR y;
					uint8_t x = ((uint8_t *)Buffer)[i];
					if (x >= 128) {
						y = (TCHAR)MacRoman2NativeTab[x - 128];
					} else {
						if (IsFileName) {
							if ((x < 32)
								|| ('\\' == x) || ('/' == x)
								|| (':' == x) || ('*' == x)
								|| ('?' == x) || ('"' == x)
								|| ('<' == x) || ('>' == x)
								|| ('|' == x))
							{
								y = (TCHAR)('-');
							} else {
								y = (TCHAR)x;
							}
						} else {
							if (13 == x) {
								*p1++ = (TCHAR)(13);
								y = (TCHAR)(10);
							} else {
								y = (TCHAR)x;
							}
						}
					}
					*p1++ = y;
				}
				*p1++ = (TCHAR) 0; /* null character */

				*r = h;
				IsOk = true;

				(void) GlobalUnlock(h);
			}
			(void) GlobalUnlock(bh);
		}
		if (! IsOk) {
			(void) GlobalFree(h);
		}
	}

	return IsOk;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC MacErr_t HTCEexport(tPbuf i)
{
	HGLOBAL h;
	MacErr_t err = mnvm_miscErr;

	if (MacRomanTextToNativeHand(i, false, &h)) {
		if (! OpenClipboard(MainWnd)) {
			/* ReportGetLastError(); */
		} else {
			if (! EmptyClipboard()) {
				/* ReportGetLastError(); */
			}
			if (SetClipboardData(CF_TEXT, h) == NULL) {
				/* ReportGetLastError(); */
			} else {
				err = mnvm_noErr;
			}
			h = NULL;
			if (! CloseClipboard()) {
				/* ReportGetLastError(); */
			}
		}
		if (h != NULL) {
			(void) GlobalFree(h);
		}
	}

	PbufDispose(i);

	return err;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC MacErr_t HTCEimport(tPbuf *r)
{
	MacErr_t err = mnvm_miscErr;

	if (IsClipboardFormatAvailable(CF_TEXT)) {
		if (! OpenClipboard(MainWnd)) {
			/* ReportGetLastError(); */
		} else {
			HGLOBAL h = GetClipboardData(CF_TEXT);
			if (h == NULL) {
				/* ReportGetLastError(); */
			} else {
				err = NativeTextToMacRomanPbuf(h, r);
			}
			if (! CloseClipboard()) {
				/* ReportGetLastError(); */
			}
		}
	}

	return err;
}
#endif

/* --- drives --- */

#define NotAfileRef INVALID_HANDLE_VALUE

LOCALVAR HANDLE Drives[NumDrives]; /* open disk image files */

#define NeedDriveNames (IncludeSonyGetName || IncludeSonyNew)

#if NeedDriveNames
LOCALVAR HGLOBAL DriveNames[NumDrives];
	/*
		It is supposed to be possible to use
		GetMappedFileName to get name of open file,
		but that seems ugly kludge, so instead
		save the name on open.
	*/
#endif

LOCALPROC InitDrives(void)
{
	/*
		This isn't really needed, Drives[i] and DriveNames[i]
		need not have valid values when not vSonyIsInserted[i].
	*/
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
#if NeedDriveNames
		DriveNames[i] = NULL;
#endif
	}
}

GLOBALOSGLUFUNC MacErr_t vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	HANDLE refnum;
	DWORD newL;
	MacErr_t result;
	DWORD BytesTransferred = 0;

	refnum = Drives[Drive_No];
	newL = SetFilePointer(
		refnum, /* handle of file */
		Sony_Start, /* number of bytes to move file pointer */
		nullpr, /* address of high-order word of distance to move */
		FILE_BEGIN /* how to move */
	);
	if (newL == 0xFFFFFFFF) {
		result = mnvm_miscErr; /*& figure out what really to return &*/
	} else if (Sony_Start != (uint32_t)newL) {
		/* not supposed to get here */
		result = mnvm_miscErr; /*& figure out what really to return &*/
	} else {
		if (IsWrite) {
			if (! WriteFile(refnum, /* handle of file to read */
				(LPVOID)Buffer
					, /* address of buffer that receives data */
				(DWORD)Sony_Count, /* number of bytes to read */
				&BytesTransferred, /* address of number of bytes read */
				nullpr)) /* address of structure for data */
			{
				result = mnvm_miscErr;
					/*& figure out what really to return &*/
			} else if ((uint32_t)BytesTransferred != Sony_Count) {
				result = mnvm_miscErr;
					/*& figure out what really to return &*/
			} else {
				result = mnvm_noErr;
			}
		} else {
			if (! ReadFile(refnum, /* handle of file to read */
				(LPVOID)Buffer,
					/* address of buffer that receives data */
				(DWORD)Sony_Count, /* number of bytes to read */
				&BytesTransferred,
					/* address of number of bytes read */
				nullpr)) /* address of structure for data */
			{
				result = mnvm_miscErr;
					/*& figure out what really to return &*/
			} else if ((uint32_t)BytesTransferred != Sony_Count) {
				result = mnvm_miscErr;
					/*& figure out what really to return &*/
			} else {
				result = mnvm_noErr;
			}
		}
	}

	if (nullpr != Sony_ActCount) {
		*Sony_ActCount = BytesTransferred;
	}

	return result;
}

GLOBALOSGLUFUNC MacErr_t vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	MacErr_t result;
	DWORD L;

	L = GetFileSize(Drives[Drive_No], nullpr);
	if (L == 0xFFFFFFFF) {
		result = mnvm_miscErr; /*& figure out what really to return &*/
	} else {
		*Sony_Count = L;
		result = mnvm_noErr;
	}

	return result;
}

LOCALFUNC MacErr_t vSonyEject0(tDrive Drive_No, bool deleteit)
{
	HANDLE refnum = Drives[Drive_No];

#if ! NeedDriveNames
	UnusedParam(deleteit);
#endif

	Drives[Drive_No] = NotAfileRef; /* not really needed */

	DiskEjectedNotify(Drive_No);

	(void) FlushFileBuffers(refnum);
	(void) CloseHandle(refnum);

#if NeedDriveNames
	{
		HGLOBAL h = DriveNames[Drive_No];
		if (NULL != h) {
			if (deleteit) {
				LPTSTR drivepath = GlobalLock(h);
				if (drivepath != NULL) {
					(void) DeleteFile(drivepath);
					(void) GlobalUnlock(h);
				}
			}
			(void) GlobalFree(h);
			DriveNames[Drive_No] = NULL; /* not really needed */
		}
	}
#endif

	return mnvm_noErr;
}

GLOBALOSGLUFUNC MacErr_t vSonyEject(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, false);
}

#if IncludeSonyNew
GLOBALOSGLUFUNC MacErr_t vSonyEjectDelete(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, true);
}
#endif

LOCALPROC UnInitDrives(void)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (vSonyIsInserted(i)) {
			(void) vSonyEject(i);
		}
	}
}

#if NeedDriveNames
LOCALFUNC bool LPTSTRtoHand(LPTSTR s, HGLOBAL *r)
{
	bool IsOk = false;

	size_t L = _tcslen(s);
	HGLOBAL h = GlobalAlloc(GMEM_DDESHARE,
		(L + 1) * sizeof(TCHAR));
	if (h != NULL) {
		LPTSTR p = GlobalLock(h);
		if (p != NULL) {
			_tcscpy(p, s);
			IsOk = true;
			(void) GlobalUnlock(h);
		}
		if (! IsOk) {
			(void) GlobalFree(h);
		} else {
			*r = h;
		}
	}

	return IsOk;
}
#endif

#if IncludeSonyGetName
GLOBALOSGLUFUNC MacErr_t vSonyGetName(tDrive Drive_No, tPbuf *r)
{
	WIN32_FIND_DATA fd;
	MacErr_t err = mnvm_miscErr;
	HGLOBAL ph = DriveNames[Drive_No];
	if (NULL != ph) {
		LPTSTR drivepath = GlobalLock(ph);
		if (drivepath != NULL) {
			HANDLE hf = FindFirstFile(drivepath, &fd);
			(void) GlobalUnlock(ph);

			if (hf != INVALID_HANDLE_VALUE) {
				HGLOBAL h;
				if (LPTSTRtoHand(fd.cFileName, &h)) {
					err = NativeTextToMacRomanPbuf(h, r);
				}
				FindClose(hf);
			}
		}
	}

	return err;
}
#endif

LOCALFUNC bool Sony_Insert0(HANDLE refnum, bool locked,
	LPTSTR drivepath)
{
	tDrive Drive_No;

#if ! NeedDriveNames
	UnusedParam(drivepath);
#endif

	if (! FirstFreeDisk(&Drive_No)) {
		(void) CloseHandle(refnum);
		MacMsg(kStrTooManyImagesTitle,
			kStrTooManyImagesMessage, false);
		return false;
	} else {
		Drives[Drive_No] = refnum;
		DiskInsertNotify(Drive_No, locked);
#if NeedDriveNames
		{
			HGLOBAL h;

			if (! LPTSTRtoHand(drivepath, &h)) {
				h = NULL;
			}

			DriveNames[Drive_No] = h;
		}
#endif
		return true;
	}
}

LOCALFUNC bool Sony_Insert1(LPTSTR drivepath, bool SilentOnMissing)
{
	bool locked = false;
	HANDLE refnum = CreateFile(
		drivepath, /* pointer to name of the file */
		GENERIC_READ + GENERIC_WRITE, /* access (read-write) mode */
		0, /* share mode */
		nullpr, /* pointer to security descriptor */
		OPEN_EXISTING, /* how to create */
		FILE_ATTRIBUTE_NORMAL, /* file attributes */
		nullpr /* handle to file with attributes to copy */
	);
	if (refnum == INVALID_HANDLE_VALUE) {
		if (ERROR_ACCESS_DENIED == GetLastError()) {
			locked = true;
			refnum = CreateFile(
				drivepath, /* pointer to name of the file */
				GENERIC_READ, /* access (read-write) mode */
				FILE_SHARE_READ, /* share mode */
				nullpr, /* pointer to security descriptor */
				OPEN_EXISTING, /* how to create */
				FILE_ATTRIBUTE_NORMAL, /* file attributes */
				nullpr /* handle to file with attributes to copy */
			);
		}
	}
	if (refnum == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (ERROR_SHARING_VIOLATION == err) {
			MacMsg(kStrImageInUseTitle,
				kStrImageInUseMessage, false);
		} else if ((ERROR_FILE_NOT_FOUND == err) && SilentOnMissing) {
			/* ignore it */
		} else {
			MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
		}
	} else {
		return Sony_Insert0(refnum, locked, drivepath);
	}
	return false;
}

LOCALFUNC bool LoadMacRomFromPath(LPTSTR drivepath)
{
	HANDLE refnum = INVALID_HANDLE_VALUE;
	bool IsOk = false;

	refnum = CreateFile(
		drivepath, /* pointer to name of the file */
		GENERIC_READ, /* access (read-write) mode */
		FILE_SHARE_READ, /* share mode */
		nullpr, /* pointer to security descriptor */
		OPEN_EXISTING, /* how to create */
		FILE_ATTRIBUTE_NORMAL, /* file attributes */
		nullpr /* handle to file with attributes to copy */
	);

	if (refnum == INVALID_HANDLE_VALUE) {
		/* MacMsg(kStrNoROMTitle, kStrNoROMMessage, true); */
	} else {
		DWORD BytesRead;

		if (! ReadFile(refnum, /* handle of file to read */
			(LPVOID)ROM, /* address of buffer that receives data */
			(DWORD)kROM_Size, /* number of bytes to read */
			&BytesRead, /* address of number of bytes read */
			nullpr)) /* address of structure for data */
		{
			MacMsgOverride(kStrNoReadROMTitle, kStrNoReadROMMessage);
		} else
		if ((uint32_t)BytesRead != kROM_Size) {
			MacMsgOverride(kStrShortROMTitle, kStrShortROMMessage);
		} else
		{
			IsOk = (mnvm_noErr == ROM_IsValid());
		}
		(void) CloseHandle(refnum);
	}

	return IsOk;
}

#ifndef EnableShellLinks
#define EnableShellLinks 1
#endif

#if EnableShellLinks
LOCALVAR bool COMinited = false;
LOCALVAR bool COMinitedOK;
#endif

#if EnableShellLinks
LOCALPROC UninitCOM(void)
{
	if (COMinited) {
		CoUninitialize();
	}
}
#endif

#if EnableShellLinks
LOCALFUNC bool NeedCOM(void)
{
	HRESULT hres;

	if (! COMinited) {
		COMinitedOK = false;
		hres = CoInitialize(NULL);
		if (SUCCEEDED(hres)) {
			COMinitedOK = true;
		}

		COMinited = true;
	}
	return COMinitedOK;
}
#endif

#if EnableShellLinks
LOCALFUNC bool ResolveShortcut(LPTSTR FilePath, bool *directory)
/* adapted from Microsoft example code */
{
	HRESULT hres;
	IShellLink *psl;
	IPersistFile* ppf;
	TCHAR szGotPath[MAX_PATH];
	WIN32_FIND_DATA wfd;
	bool IsOk = false;

	if (NeedCOM()) {
		hres = CoCreateInstance(&CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, &IID_IShellLink,
			(LPVOID *)(void *)&psl);
			/*
				the (void *) prevents a compiler warning
				from gcc
			*/
		if (SUCCEEDED(hres)) {
			/* Get a pointer to the IPersistFile interface. */
			hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile,
				(void **)(void *)&ppf);
			if (SUCCEEDED(hres)) {
				/* Ensure that the string is Unicode. */
#if UseUni
#define wsz FilePath
#else
				WORD wsz[MAX_PATH];
				MultiByteToWideChar(CP_ACP, 0, FilePath, -1, wsz,
					MAX_PATH);
#endif

				/* Load the shortcut. */
				hres = ppf->lpVtbl->Load(ppf, wsz, STGM_READ);
				if (SUCCEEDED(hres)) {
					/* Resolve the link. */
					hres = psl->lpVtbl->Resolve(psl, MainWnd,
						SLR_ANY_MATCH);
					if (SUCCEEDED(hres)) {
						/* Get the path to the link target. */
						hres = psl->lpVtbl->GetPath(psl, szGotPath,
							MAX_PATH, &wfd,
							SLGP_SHORTPATH);
						if (SUCCEEDED(hres)) {
							/*
								This is in the sample code, but doesn't
								seem to be needed:
								Get the description of the target.
								char szDescription[MAX_PATH];
								hres = psl->lpVtbl->GetDescription(psl,
									szDescription, MAX_PATH);
								if (SUCCEEDED(hres)) {
								}
							*/
							lstrcpy(FilePath, szGotPath);
							if (NULL != directory) {
								*directory = (0 != (wfd.dwFileAttributes
									& FILE_ATTRIBUTE_DIRECTORY));
							}
							IsOk = true;
						}
					}
				}

				ppf->lpVtbl->Release(ppf);
			}
			psl->lpVtbl->Release(psl);
		}
	}
	return IsOk;
}
#endif

#if EnableShellLinks
LOCALFUNC bool FileIsLink(LPTSTR drivepath)
{
	LPTSTR p = FindLastTerm(drivepath, (TCHAR)('.'));

	if (p != nullpr) {
		if (_tcscmp(p, TEXT("lnk")) == 0) {
			return true;
		}
	}
	return false;
}
#endif

LOCALFUNC bool InsertDiskOrAlias(LPTSTR drivepath,
	bool MaybeROM, bool MaybeAlias)
{
#if EnableShellLinks
	if (MaybeAlias && FileIsLink(drivepath)) {
		if (! ResolveShortcut(drivepath, NULL)) {
			return false;
		}
	}
#endif

	if (MaybeROM && ! ROM_loaded) {
		return LoadMacRomFromPath(drivepath);
	} else {
		return Sony_Insert1(drivepath, false);
	}
}

LOCALFUNC bool FileExists(LPTSTR pathName, bool *directory)
{
	WIN32_FIND_DATA fd;
	HANDLE hf = FindFirstFile(pathName, &fd);
	bool IsOk = false;

	if (hf != INVALID_HANDLE_VALUE) {
		if (NULL != directory) {
			*directory =
				(0 != (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}
		IsOk = true;
		FindClose(hf);
	}

	return IsOk;
}

LOCALFUNC MacErr_t ResolveNamedChild0(LPTSTR pathName,
	LPTSTR Child, bool *directory)
{
	size_t newlen;
	size_t oldlen = _tcslen(pathName);
	MacErr_t err = mnvm_miscErr;

	newlen = oldlen + 1 + _tcslen(Child);
	if (newlen + 1 < _MAX_PATH) {
		_tcscat(pathName, TEXT("\\"));
		_tcscat(pathName, Child);

		if (FileExists(pathName, directory)) {
			err = mnvm_noErr;
		} else {
			err = mnvm_fnfErr;
#if EnableShellLinks
			if (newlen + 5 < _MAX_PATH) {
				_tcscat(pathName, TEXT(".lnk"));
				if (FileExists(pathName, NULL))
				if (ResolveShortcut(pathName, directory))
				{
					err = mnvm_noErr;
				}
				if (mnvm_noErr != err) {
					pathName[newlen] = (TCHAR)('\0');
				}
			}
#endif
		}
	}

	return err;
}

LOCALFUNC MacErr_t ResolveNamedChild(LPTSTR pathName,
	char *Child, bool *directory)
{
	TCHAR Child0[ClStrMaxLength + 1];

	NativeStrFromCStr(Child0, Child, false);

	return ResolveNamedChild0(pathName, Child0, directory);
}

LOCALFUNC bool ResolveNamedChildDir(LPTSTR pathName, char *Child)
{
	bool directory;

	return (mnvm_noErr == ResolveNamedChild(
		pathName, Child, &directory))
		&& directory;
}

LOCALFUNC bool ResolveNamedChildFile(LPTSTR pathName, char *Child)
{
	bool directory;

	return (mnvm_noErr == ResolveNamedChild(
		pathName, Child, &directory))
		&& ! directory;
}

#if (IncludeSonyNew && ! SaveDialogEnable)
LOCALFUNC bool MakeNamedChildDir(LPTSTR pathName, char *Child)
{
	bool directory;
	bool IsOk = false;
	MacErr_t err = ResolveNamedChild(pathName, Child, &directory);

	if (mnvm_noErr == err) {
		IsOk = directory;
	} else if (mnvm_fnfErr == err) {
		if (CreateDirectory(pathName, NULL)) {
			IsOk = true;
		}
	}

	return IsOk;
}
#endif

LOCALFUNC bool GetAppDataPath(LPTSTR lpszPath,
	BOOL fCreate)
{
	bool IsOk = false;

	if (HaveMySHGetSpecialFolderPath())
	if (MySHGetSpecialFolderPath(
		NULL /* HWND hwndOwner */,
		lpszPath, _CSIDL_APPDATA, fCreate))
	{
		IsOk = true;
	}
	/*
		if not available, could perhaps
		use GetWindowsDirectory.
	*/
	/*
		SHGetFolderPath is more recent,
		could perhaps check for it first.
		might also be in "SHFolder.dll".

		SHGetKnownFolderPath is even
		more recent.
	*/

	return IsOk;
}

LOCALPROC InsertADisk0(void)
{
	OPENFILENAME ofn;
	TCHAR szDirName[256];
	TCHAR szFile[256];
	TCHAR szFileTitle[256];
	UINT i;
	size_t cbString;
	TCHAR chReplace;
	TCHAR szFilter[256];
	bool IsOk;

	szDirName[0] = (TCHAR)('\0');
	szFile[0] = (TCHAR)('\0');
	_tcscpy(szFilter,
		TEXT("Disk images|*.dsk;*.HF?;*.IMG;*.IMA;*.IMAGE")
		TEXT("|All files (*.*)|*.*|\0"));

	cbString = _tcslen(szFilter);

	chReplace = szFilter[cbString - 1];

	for (i = 0; szFilter[i] != (TCHAR)('\0'); ++i)
	{
		if (szFilter[i] == chReplace) {
			szFilter[i] = (TCHAR)('\0');
		}
	}

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = MainWnd;
	ofn.lpstrFilter = szFilter;
	ofn.nFilterIndex = 2;
	ofn.lpstrFile= szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.lpstrInitialDir = szDirName;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST
		| OFN_HIDEREADONLY;

	MyBeginDialog();
	IsOk = GetOpenFileName(&ofn);
	MyEndDialog();

	if(! IsOk) {
		/* report error */
	} else {
		(void) InsertDiskOrAlias(ofn.lpstrFile,
			true, false);
	}
}

LOCALFUNC bool LoadInitialImageFromName(char *ImageName)
{
	TCHAR ImageFile[_MAX_PATH];

	if (GetAppDir(ImageFile))
	if (ResolveNamedChildFile(ImageFile, ImageName))
	if (Sony_Insert1(ImageFile, true))
	{
		return true;
	}
	return false;
}

LOCALFUNC bool Sony_InsertIth(int i)
{
	bool v;

	if ((i > 9) || ! FirstFreeDisk(nullpr)) {
		v = false;
	} else {
		char s[] = "disk?.dsk";

		s[4] = '0' + i;

		/* stop on first error (including file not found) */
		v = LoadInitialImageFromName(s);
	}

	return v;
}

LOCALFUNC bool LoadInitialImages(void)
{
	if (! AnyDiskInserted()) {
		int i;

		for (i = 1; Sony_InsertIth(i); ++i) {
			/* stop on first error (including file not found) */
		}
	}

	return true;
}

#if IncludeSonyNew
LOCALFUNC bool WriteZero(HANDLE refnum, uint32_t L)
{
	if (SetFilePointer(
		refnum, /* handle of file */
		0, /* number of bytes to move file pointer */
		nullpr, /* address of high-order word of distance to move */
		FILE_BEGIN /* how to move */
		) != 0)
	{
		return false;
	} else {
#define ZeroBufferSize 2048
		uint32_t i;
		uint8_t buffer[ZeroBufferSize];
		DWORD BytesWritten;

		memset(&buffer, 0, ZeroBufferSize);

		while (L > 0) {
			i = (L > ZeroBufferSize) ? ZeroBufferSize : L;
			if (! WriteFile(refnum, /* handle of file to read */
				(LPVOID)buffer,
					/* address of buffer that receives data */
				(DWORD)i, /* number of bytes to read */
				&BytesWritten, /* address of number of bytes read */
				nullpr) /* address of structure for data */
				|| ((uint32_t)BytesWritten != i))
			{
				return false;
			}
			L -= i;
		}
		return true;
	}
}
#endif

#define MaxSavePathSize MAX_PATH

#if IncludeSonyNew
LOCALPROC MakeNewDisk0(uint32_t L, LPTSTR pathName)
{
	bool IsOk = false;
	HANDLE newrefNum;

	IsOk = false;
	newrefNum = CreateFile(
		pathName, /* pointer to name of the file */
		GENERIC_READ + GENERIC_WRITE, /* access (read-write) mode */
		0, /* share mode */
		NULL, /* pointer to security descriptor */
		CREATE_ALWAYS, /* how to create */
		FILE_ATTRIBUTE_NORMAL, /* file attributes */
		NULL /* handle to file with attributes to copy */
	);
	if (newrefNum == INVALID_HANDLE_VALUE) {
		/* report error */
	} else {
		if (SetFilePointer(
			newrefNum, /* handle of file */
			L, /* number of bytes to move file pointer */
			nullpr,
				/* address of high-order word of distance to move */
			FILE_BEGIN /* how to move */
			) != L)
		{
			/* report error */
		} else if (! SetEndOfFile(newrefNum)) {
			/* report error */
		} else if (! WriteZero(newrefNum, L)) {
			/* report error */
		} else {
			IsOk =
				Sony_Insert0(newrefNum, false, pathName);
			newrefNum = INVALID_HANDLE_VALUE;
		}
		if (INVALID_HANDLE_VALUE != newrefNum) {
			(void) CloseHandle(newrefNum);
		}
		if (! IsOk) {
			(void) DeleteFile(pathName);
		}
	}
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDisk(uint32_t L, HGLOBAL NewDiskNameDat)
{
#if SaveDialogEnable
	OPENFILENAME ofn;
	bool IsOk = false;
	TCHAR szFile[MaxSavePathSize];
	TCHAR szFileTitle[MaxSavePathSize];

	memset(&ofn, 0, sizeof(OPENFILENAME));
	szFile[0] = 0;
	szFileTitle[0] = 0;

#if IncludeSonyGetName
	if (NewDiskNameDat != NULL) {
		LPTSTR p = GlobalLock(NewDiskNameDat);
		if (p != NULL) {
			_tcscpy(szFile, p);
			(void) GlobalUnlock(NewDiskNameDat);
		}
	} else
#endif
	{
		NativeStrFromCStr(szFile, "untitled", false);
	}

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.hwndOwner = MainWnd;
	/* ofn.lpstrFilter = "All\0*.*\0Text\0*.txt\0Datafile\0*.dsk\0"; */
	/* ofn.lpstrFilter = NULL; */ /* szFilter */
	ofn.nMaxFile = MaxSavePathSize;
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = MaxSavePathSize;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT + OFN_HIDEREADONLY;
		/* + OFN_SHOWHELP */

	MyBeginDialog();
	IsOk = GetSaveFileName(&ofn);
	MyEndDialog();

	if (! IsOk) {
		/* report error */
	} else {
		MakeNewDisk0(L, ofn.lpstrFile);
	}
#else /* SaveDialogEnable */
	TCHAR pathName[MaxSavePathSize];

	if (GetAppDir(pathName))
	if (MakeNamedChildDir(pathName, "out"))
	{
		bool directory;
		LPTSTR p = GlobalLock(NewDiskNameDat);

		if (p != NULL) {
			MacErr_t err = ResolveNamedChild0(pathName, p,
				&directory);

			if (mnvm_fnfErr == err) {
				err = mnvm_noErr;
			} else if (mnvm_noErr == err) {
				if (directory) {
					err = mnvm_miscErr;
				}
			}

			if (mnvm_noErr == err) {
				MakeNewDisk0(L, pathName);
			}

			(void) GlobalUnlock(NewDiskNameDat);
		}
	}
#endif /* SaveDialogEnable */
}
#endif

LOCALFUNC bool LoadMacRom(void)
{
	TCHAR ROMFile[_MAX_PATH];
	bool IsOk = false;

	if (GetAppDir(ROMFile))
	if (ResolveNamedChildFile(ROMFile, RomFileName))
	{
		IsOk = true;
	}

	if (! IsOk) {
		if (GetAppDataPath(ROMFile, FALSE))
		if (ResolveNamedChildDir(ROMFile, "Gryphel"))
		if (ResolveNamedChildDir(ROMFile, "mnvm_rom"))
		if (ResolveNamedChildFile(ROMFile, RomFileName))
		{
			IsOk = true;
		}

	}

	if (IsOk) {
		IsOk = LoadMacRomFromPath(ROMFile);
	}

	return true;
}

#if InstallFileIcons
LOCALPROC SetRegKey(HKEY hKeyRoot,
	LPTSTR strRegKey, LPTSTR strRegValue)
{
	HKEY RegKey;
	DWORD dwDisposition;

	if (ERROR_SUCCESS == RegCreateKeyEx(hKeyRoot, strRegKey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
		NULL, &RegKey, &dwDisposition))
	{
		RegSetValueEx(RegKey, NULL, 0, REG_SZ,
			(CONST BYTE *)strRegValue,
			(DWORD)((_tcslen(strRegValue) + 1) * sizeof(TCHAR)));
		RegCloseKey(RegKey);
	}
}

LOCALPROC RegisterShellFileType(LPTSTR AppPath, LPTSTR strFilterExt,
	LPTSTR strFileTypeId, LPTSTR strFileTypeName,
	LPTSTR strIconId, bool CanOpen)
{
	TCHAR strRegKey[_MAX_PATH];
	TCHAR strRegValue[_MAX_PATH + 2];
		/* extra room for ","{strIconId} */

	SetRegKey(HKEY_CLASSES_ROOT, strFileTypeId, strFileTypeName);
	SetRegKey(HKEY_CLASSES_ROOT, strFilterExt, strFileTypeId);

	_tcscpy(strRegKey, strFileTypeId);
	_tcscat(strRegKey, TEXT("\\DefaultIcon"));
	_tcscpy(strRegValue, TEXT("\""));
	_tcscat(strRegValue, AppPath);
	_tcscat(strRegValue, TEXT("\","));
	_tcscat(strRegValue, strIconId);
	SetRegKey(HKEY_CLASSES_ROOT, strRegKey, strRegValue);

	if (CanOpen) {
		_tcscpy(strRegKey, strFileTypeId);
		_tcscat(strRegKey, TEXT("\\shell\\open\\command"));
		_tcscpy(strRegValue, TEXT("\""));
		_tcscat(strRegValue, AppPath);
		_tcscat(strRegValue, TEXT("\" \"%1\""));
		SetRegKey(HKEY_CLASSES_ROOT, strRegKey, strRegValue);
	}
}

LOCALFUNC bool RegisterInRegistry(void)
{
	TCHAR AppPath[_MAX_PATH];

	GetModuleFileName(NULL, AppPath, _MAX_PATH);
#if 0
	GetShortPathName(AppPath, AppPath, _MAX_PATH);
#endif

	RegisterShellFileType(AppPath, TEXT(".rom"), TEXT("minivmac.rom"),
		TEXT("Mini vMac ROM Image"), TEXT("1"), false);
	RegisterShellFileType(AppPath, TEXT(".dsk"), TEXT("minivmac.dsk"),
		TEXT("Mini vMac Disk Image"), TEXT("2"), true);

	return true;
}
#endif

LOCALVAR LPTSTR CommandLine;

LOCALFUNC bool ScanCommandLine(void)
{
	TCHAR *p = CommandLine;
	TCHAR *p1;
	TCHAR *p2;
	TCHAR v;
	size_t L;

	v = *p;
	while (0 != v) {
		if (' ' == v) {
			v = *++p;
		} else {
			if ('\"' == v) {
				v = *++p;
				p1 = p;
				while (('\"' != v) && (0 != v)) {
					v = *++p;
				}
				p2 = p;
				if ('\"' == v) {
					v = *++p;
				}
			} else {
				p1 = p;
				while ((' ' != v) && (0 != v)) {
					v = *++p;
				}
				p2 = p;
			}
			L = p2 - p1;
			if (L + 1 <= _MAX_PATH) {
				TCHAR fileName[_MAX_PATH];
				TCHAR *filePtr = fileName;
				size_t i = L;

				while (i > 0) {
					*filePtr++ = *p1++;
					--i;
				}
				*filePtr = (char)0;

				if ((L > 0)
					&& (('/' == fileName[0]) || ('-' == fileName[0])))
				{
#if 0
					TCHAR *p3 = &fileName[1];
					if (0 == _tcscmp(p3, TEXT("l"))) {
						SpeedValue = 0;
					} else
#endif
					{
						MacMsg(kStrBadArgTitle, kStrBadArgMessage,
							false);
					}
				} else {
					(void) InsertDiskOrAlias(fileName,
						false, true);
				}
			}
		}
	}

	return true;
}

#if EnableRecreateW
LOCALPROC CheckMagnifyAndFullScreen(void)
{
	if (
#if 1
		(UseMagnify != WantMagnify)
#endif
#if 1 && 1
		||
#endif
#if 1
		(UseFullScreen != WantFullScreen)
#endif
		)
	{
		(void) ReCreateMainWindow();
	}
}
#endif

#if 1 && 1
enum {
	kWinStateWindowed,
#if 1
	kWinStateFullScreen,
#endif
	kNumWinStates
};
#endif

#if 1 && 1
LOCALVAR int WinMagStates[kNumWinStates];
#endif

LOCALPROC ZapWinStateVars(void)
{
#if MayNotFullScreen
	{
		int i;

		for (i = 0; i < kNumMagStates; ++i) {
			HavePositionWins[i] = false;
		}
	}
#endif
#if 1 && 1
	{
		int i;

		for (i = 0; i < kNumWinStates; ++i) {
			WinMagStates[i] = kMagStateAuto;
		}
	}
#endif
}

#if 1
void ToggleWantFullScreen(void)
{
	WantFullScreen = ! WantFullScreen;

#if 1
	{
		int OldWinState =
			UseFullScreen ? kWinStateFullScreen : kWinStateWindowed;
		int OldMagState =
			UseMagnify ? kMagStateMagnifgy : kMagStateNormal;
		int NewWinState =
			WantFullScreen ? kWinStateFullScreen : kWinStateWindowed;
		int NewMagState = WinMagStates[NewWinState];

		WinMagStates[OldWinState] = OldMagState;
		if (kMagStateAuto != NewMagState) {
			WantMagnify = (kMagStateMagnifgy == NewMagState);
		} else {
			WantMagnify = false;
			if (WantFullScreen) {
				if ((GetSystemMetrics(SM_CXSCREEN)
						>= vMacScreenWidth * WindowScale)
					&& (GetSystemMetrics(SM_CYSCREEN)
						>= vMacScreenHeight * WindowScale)
					)
				{
					WantMagnify = true;
				}
			}
		}
	}
#endif
}
#endif

#if EnableDragDrop
LOCALPROC DragFunc(HDROP hDrop)
{
	WORD n;
	WORD i;
	TCHAR a[_MAX_PATH];

	n = DragQueryFile(hDrop, (UINT) -1, NULL, 0);
	for (i = 0; i < n; ++i) {
		if (DragQueryFile(hDrop, i, NULL, 0) < _MAX_PATH - 1) {
			(void) DragQueryFile(hDrop, i, a, _MAX_PATH);
			(void) InsertDiskOrAlias(a, true, true);
		}
	}

	DragFinish(hDrop);

	if (gTrueBackgroundFlag) {
		if (! SetForegroundWindow(MainWnd)) {
			/* error message here ? */
		}

		WantCmdOptOnReconnect = true;
	}
}
#endif

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
#if SoundEnabled
	SoundCheckVeryOften();
#endif
	(void) UpdateTrueEmulatedTime();
	return (TrueEmulatedTime == OnTrueTime);
}

/* --- platform independent code can be thought of as going here --- */

LOCALPROC LeaveBackground(void)
{
	ReconnectKeyCodes3();
}

LOCALPROC EnterBackground(void)
{
	DisconnectKeyCodes3();

#if 1
	if (WantFullScreen) {
		ToggleWantFullScreen();
	}
#endif
}

LOCALPROC LeaveSpeedStopped(void)
{
#if SoundEnabled
	Sound_Start();
#endif
#if (TimeResolution != 0)
	Timer_Resume();
#endif
}

LOCALPROC EnterSpeedStopped(void)
{
#if (TimeResolution != 0)
	Timer_Suspend();
#endif
#if SoundEnabled
	Sound_Stop();
#endif
}

LOCALPROC CheckForSavedTasks(void)
{
	/*
		Check for things to do that rather wouldn't
		have done at an awkward time.
	*/

	if (EvtQNeedRecover) {
		EvtQNeedRecover = false;

		/* attempt cleanup, EvtQNeedRecover may get set again */
		EvtQTryRecoverFromFull();
	}

	if (RequestMacOff) {
		RequestMacOff = false;
		if (AnyDiskInserted()) {
			MacMsgOverride(kStrQuitWarningTitle,
				kStrQuitWarningMessage);
		} else {
			ForceMacOff = true;
		}
	}

	if (ForceMacOff) {
		return;
	}

	if (gTrueBackgroundFlag != gBackgroundFlag) {
		gBackgroundFlag = gTrueBackgroundFlag;
		if (gTrueBackgroundFlag) {
			EnterBackground();
		} else {
			LeaveBackground();
		}
	}

	if (CurSpeedStopped != (SpeedStopped ||
		(gBackgroundFlag && ! RunInBackground)))
	{
		CurSpeedStopped = ! CurSpeedStopped;
		if (CurSpeedStopped) {
			EnterSpeedStopped();
		} else {
			LeaveSpeedStopped();
		}
	}

#if EnableRecreateW
	if (! (gTrueBackgroundFlag)) {
		CheckMagnifyAndFullScreen();
	}
#endif

#if MayFullScreen
	if (GrabMachine != (
#if 1
		UseFullScreen &&
#endif
		! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		GrabMachine = ! GrabMachine;
		AdjustMachineGrab();
	}
#endif

	if (gTrueBackgroundFlag) {
		/*
			wait til later
		*/
	} else {
#if IncludeSonyNew
		if (vSonyNewDiskWanted) {
#if IncludeSonyNameNew
			if (vSonyNewDiskName != NotAPbuf) {
				HGLOBAL NewDiskNameDat;
				if (MacRomanTextToNativeHand(vSonyNewDiskName, true,
					&NewDiskNameDat))
				{
					MakeNewDisk(vSonyNewDiskSize, NewDiskNameDat);
					GlobalFree(NewDiskNameDat);
				}
				PbufDispose(vSonyNewDiskName);
				vSonyNewDiskName = NotAPbuf;
			} else
#endif
			{
				MakeNewDisk(vSonyNewDiskSize, NULL);
			}
			vSonyNewDiskWanted = false;
				/* must be done after may have gotten disk */
		}
#endif
		if (RequestInsertDisk) {
			RequestInsertDisk = false;
			InsertADisk0();
		}
	}

#if NeedRequestIthDisk
	if (0 != RequestIthDisk) {
		Sony_InsertIth(RequestIthDisk);
		RequestIthDisk = 0;
	}
#endif

	if (HaveCursorHidden != (WantCursorHidden
		&& ! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		if (HaveCursorHidden) {
			(void) ShowCursor(FALSE);
		} else {
			(void) ShowCursor(TRUE);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}
}

LRESULT CALLBACK Win32WMProc(HWND hwnd,
	UINT uMessage, WPARAM wparam, LPARAM lparam);

LRESULT CALLBACK Win32WMProc(HWND hwnd,
	UINT uMessage, WPARAM wparam, LPARAM lparam)
{
	switch (uMessage)
	{
		case WM_PAINT:
			{
				PAINTSTRUCT ps;

				BeginPaint(hwnd, (LPPAINTSTRUCT)&ps);
#if 1
				if (UseFullScreen)
#endif
#if MayFullScreen
				{
					if (! FillRect(ps.hdc,
						&ps.rcPaint,
						GetStockObject(BLACK_BRUSH)))
					{
						/* ReportGetLastError(); */
					}
				}
#endif
				if (MainWnd == hwnd) {
					Screen_DrawAll();
				}
				EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
			}
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (! TestBit(lparam, 30)) { /* ignore repeats */
				DoVKcode(wparam, lparam >> 24, true);
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			DoVKcode(wparam, lparam >> 24, false);
			break;
#if ItnlKyBdFix
		case WM_INPUTLANGCHANGE:
			CheckKeyboardLayout();
			return TRUE;
			break;
#endif

		case WM_CLOSE:
			RequestMacOff = true;
			break;
		case WM_QUERYENDSESSION:
			if (AnyDiskInserted()) {
				RequestMacOff = true;
				return FALSE;
			} else {
				return TRUE;
			}
			break;
		case WM_ACTIVATE:
			if (MainWnd == hwnd) {
				gTrueBackgroundFlag = (LOWORD(wparam) == WA_INACTIVE);
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wparam))
			{
				case ID_FILE_INSERTDISK1:
					RequestInsertDisk = true;
					break;
				case ID_FILE_QUIT:
					RequestMacOff = true;
					break;
				case ID_SPECIAL_MORECOMMANDS:
					DoMoreCommandsMsg();
					break;
				case ID_HELP_ABOUT:
					DoAboutMsg();
					break;
			}
			break;
		case WM_MOVE:
			WndX = (int16_t) LOWORD(lparam);
			WndY = (int16_t) HIWORD(lparam);
			break;
		case WM_SYSCHAR:
		case WM_CHAR:
			/* prevent any further processing */
			break;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			MousePositionNotify(LOWORD (lparam), HIWORD (lparam));
			SetCurMouseButton(true);
			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			MousePositionNotify(LOWORD (lparam), HIWORD (lparam));
			SetCurMouseButton(false);
			break;
		case WM_MOUSEMOVE:
			/* windows may have messed up cursor */
			/*
				there is no notification when the mouse moves
				outside the window, and the cursor is automatically
				changed
			*/
			if (! HaveCursorHidden) {
				/* SetCursor(LoadCursor(NULL, IDC_ARROW)); */
			}
			break;
#if EnableDragDrop
		case WM_CREATE:
			DragAcceptFiles(hwnd, TRUE);
			break;
		case WM_DROPFILES:
			DragFunc((HDROP) wparam);
			break;
		case WM_DESTROY:
			DragAcceptFiles(hwnd, FALSE);
			break;
#endif
		default:
			return DefWindowProc(hwnd, uMessage, wparam, lparam);
	}
	return 0;
}

LOCALFUNC bool RegisterOurClass(void)
{
	WNDCLASS wc;

	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = (WNDPROC)Win32WMProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = AppInstance;
	wc.hIcon         = LoadIcon(AppInstance, MAKEINTRESOURCE(IDI_VMAC));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = WndClassName;

	if (! RegisterClass(&wc)) {
		MacMsg("RegisterClass failed",
			"Sorry, Mini vMac encountered errors"
			" and cannot continue.", true);
		return false;
	} else {
		return true;
	}
}

LOCALPROC WaitForTheNextEvent(void)
{
	MSG msg;

	if (-1 != GetMessage(&msg, NULL, 0, 0)) {
		DispatchMessage(&msg);
	}
}

LOCALPROC CheckForSystemEvents(void)
{
	MSG msg;
	uint8_t i = 0;

	while ((i < 32) && (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))) {
		DispatchMessage(&msg);
		++i;
	}
}

GLOBALOSGLUPROC WaitForNextTick(void)
{
label_retry:
	CheckForSystemEvents();
	CheckForSavedTasks();

	if (ForceMacOff) {
		return;
	}

	if (CurSpeedStopped) {
		DoneWithDrawingForTick();
		WaitForTheNextEvent();
		goto label_retry;
	}

	if (ExtraTimeNotOver()) {
		Sleep(NextIntTime - LastTime);
		goto label_retry;
	}

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	if (! (gBackgroundFlag)) {
		CheckMouseState();

#if EnableGrabSpecialKeys
		CheckForLostKeyUps();
#endif
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif
}

#include "PROGMAIN.h"

/* ************************ */

LOCALPROC ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
}

LOCALPROC ReserveAllocAll(void)
{
#if dbglog_HAVE
	dbglog_ReserveAlloc();
#endif
	ReserveAllocOneBlock(&ROM, kROM_Size, 5, false);
	ReserveAllocOneBlock(&screencomparebuff,
		vMacScreenNumBytes, 5, true);
#if UseControlKeys
	ReserveAllocOneBlock(&CntrlDisplayBuff,
		vMacScreenNumBytes, 5, false);
#endif
#if EnableScalingBuff
	{
		uint32_t n = vMacScreenMonoNumBytes
#if 1
			* WindowScale * WindowScale
#endif
			;
		if ((vMacScreenDepth == 1) && (vMacScreenNumBytes * 2 > n)) {
			n = vMacScreenNumBytes * 2;
		}
		else if ((vMacScreenDepth >= 4) && (vMacScreenNumBytes > n)) {
			n = vMacScreenNumBytes;
		}
		ReserveAllocOneBlock(&ScalingBuff, n, 5, false);
	}
#endif
#if SoundEnabled
	ReserveAllocOneBlock((uint8_t * *)&TheSoundBuffer,
		dbhBufferSize, 5, false);
#endif

	EmulationReserveAlloc();
}

LOCALFUNC bool AllocMemory(void)
{
	uimr n;
	bool IsOk = false;

	ReserveAllocOffset = 0;
	ReserveAllocBigBlock = nullpr;
	ReserveAllocAll();
	n = ReserveAllocOffset;
	ReserveAllocBigBlock =
		(uint8_t *)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, n);
	if (NULL == ReserveAllocBigBlock) {
		MacMsg(kStrOutOfMemTitle, kStrOutOfMemMessage, true);
	} else {
		ReserveAllocOffset = 0;
		ReserveAllocAll();
		if (n != ReserveAllocOffset) {
			/* oops, program error */
		} else {
			IsOk = true;
		}
	}

	return IsOk;
}

LOCALPROC UnallocMemory(void)
{
	if (nullpr != ReserveAllocBigBlock) {
		if (GlobalFree(ReserveAllocBigBlock) != NULL) {
			MacMsg("error", "GlobalFree failed", false);
		}
	}
}

LOCALFUNC bool InitOSGLU(void)
{
	if (AllocMemory())
#if dbglog_HAVE
	if (dbglog_open())
#endif
	if (RegisterOurClass())
	if (ScanCommandLine())
	if (LoadInitialImages())
#if InstallFileIcons
	if (RegisterInRegistry())
#endif
	if (LoadMacRom())
	if (ReCreateMainWindow())
	if (InitWinKey2Mac())
	if (InitTheCursor())
	if (Init60thCheck())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

LOCALPROC UnInitOSGLU(void)
{
#if (TimeResolution != 0)
	Timer_Suspend();
#endif
	MouseCaptureSet(false);

	if (MacMsgDisplayed) {
		MacMsgDisplayOff();
	}

#if MayFullScreen
	UnGrabTheMachine();
#endif
#if SoundEnabled
	Sound_Stop();
#endif
#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	ForceShowCursor();

#if EnableShellLinks
	UninitCOM();
#endif

	if (! gTrueBackgroundFlag) {
		CheckSavedMacMsg();
	}

	DisposeMainWindow();

#if dbglog_HAVE
	dbglog_close();
#endif

	UnallocMemory();
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)
{
	UnusedParam(hPrevInstance);
	AppInstance = hInstance;
	CmdShow = nCmdShow;
	CommandLine = lpCmdLine;

	GetWndTitle();
	ZapOSGLUVars();
	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

	return(0);
}
