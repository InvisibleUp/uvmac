#ifndef WIN32_SOUND_H
#define WIN32_SOUND_H

#include <windows.h>

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

extern tpSoundSamp TheSoundBuffer;
uint16_t ThePlayOffset;
uint16_t TheFillOffset;
bool wantplaying;
uint16_t MinFilledSoundBuffs;
uint16_t TheWriteOffset;

extern HWAVEOUT hWaveOut;
WAVEHDR whdr[kSoundBuffers];

#define SOUND_SAMPLERATE /* 22050 */ 22255
	/* = round(7833600 * 2 / 704) */
	
void FillWithSilence(tpSoundSamp p, int n, trSoundSamp v);
void Sound_BeginPlaying(void);
void Sound_Start(void);
void Sound_Stop(void);
void SoundCheckVeryOften(void);
void ConvertSoundBlockToNative(tpSoundSamp p);
void Sound_FilledBlocks(void);
void Sound_WroteABlock(void);
void Sound_EndWrite(uint16_t actL);
tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL);
void Sound_SecondNotify(void);
	
#endif
