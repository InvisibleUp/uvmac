#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>
#include "UI/MYOSGLUE.h"

/* --- defines and macros and such --- */

// video defines
#ifndef UseSDLscaling
#define UseSDLscaling 0
#endif

#if 1 && ! UseSDLscaling
#define MaxScale WindowScale
#else
#define MaxScale 1
#endif

#define CLUT_finalsz (256 * 8 * 4 * MaxScale)

// sound defines
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

#ifndef UseRWops
#define UseRWops 0
#endif

#if UseRWops
#define FilePtr SDL_RWops *
#define Seek SDL_RWseek
#define SeekSet RW_SEEK_SET
#define SeekCur RW_SEEK_CUR
#define SeekEnd RW_SEEK_END
#define FileRead(ptr, size, nmemb, stream) \
	SDL_RWread(stream, ptr, size, nmemb)
#define FileWrite(ptr, size, nmemb, stream) \
	SDL_RWwrite(stream, ptr, size, nmemb)
#define FileTell SDL_RWtell
#define FileClose SDL_RWclose
#define FileOpen SDL_RWFromFile
#else
#define FilePtr FILE *
#define Seek fseek
#define SeekSet SEEK_SET
#define SeekCur SEEK_CUR
#define SeekEnd SEEK_END
#define FileRead fread
#define FileWrite fwrite
#define FileTell ftell
#define FileClose fclose
#define FileOpen fopen
#define FileEof feof
#endif



/* --- globals --- */

#if MayFullScreen
extern int hOffset;
extern int vOffset;
#endif
extern bool UseFullScreen;
extern bool UseMagnify;

extern uint32_t TrueEmulatedTime;
extern uint32_t LastTime;
extern uint32_t NextIntTime;
extern uint32_t NextFracTime;

extern bool gBackgroundFlag;
extern bool gTrueBackgroundFlag;
extern bool CurSpeedStopped;

extern SDL_Window *main_wind;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;
extern SDL_PixelFormat *format;

extern uint8_t * ScalingBuff;
extern uint8_t * CLUT_final;

extern bool HaveCursorHidden;
extern bool WantCursorHidden;

extern tpSoundSamp TheSoundBuffer;

// Functions

void DoKeyCode(SDL_Keysym *r, bool down);
