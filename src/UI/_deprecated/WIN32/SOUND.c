/* --- sound --- */

#include "OSGLUWIN.h"

#if SoundEnabled
#include "UI/WIN32/SOUND.h"
tpSoundSamp TheSoundBuffer = nullpr;
HWAVEOUT hWaveOut = NULL;

void FillWithSilence(tpSoundSamp p, int n, trSoundSamp v)
{
	int i;

	for (i = n; --i >= 0; ) {
		*p++ = v;
	}
}

void Sound_BeginPlaying(void)
{
#if dbglog_SoundStuff
	fprintf(stderr, "Sound_BeginPlaying\n");
#endif
}

void Sound_Start(void)
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

void Sound_Stop(void)
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

void SoundCheckVeryOften(void)
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
void ConvertSoundBlockToNative(tpSoundSamp p)
{
	int i;

	for (i = kOneBuffLen; --i >= 0; ) {
		*p++ -= 0x8000;
	}
}
#else
#define ConvertSoundBlockToNative(p)
#endif

void Sound_FilledBlocks(void)
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

void Sound_WroteABlock(void)
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

void Sound_EndWrite(uint16_t actL)
{
	TheWriteOffset += actL;

	if (0 == (TheWriteOffset & kOneBuffMask)) {
		/* just finished a block */

		Sound_WroteABlock();
	}
}

tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL)
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

void Sound_SecondNotify(void)
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
