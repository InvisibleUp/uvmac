/*
	Configuration options used by both platform specific
	and platform independent code.

	Hopefully, one day, we can remove this file entirely.
*/

#include <stdint.h>

#define MayInline inline __attribute__((always_inline))
#define SmallGlobals 0
#define cIncludeUnused 0
#define UnusedParam(p) (void) p

/* capabilities provided by platform specific code */

#define SoundEnabled 1
#define kLn2SoundSampSz 3

#define dbglog_HAVE 0
#define WantAbnormalReports 0

#define NumDrives 6
#define IncludeSonyRawMode 1
#define IncludeSonyGetName 1
#define IncludeSonyNew 1
#define IncludeSonyNameNew 1

#define vMacScreenHeight 342
#define vMacScreenWidth 512
#define vMacScreenDepth 0

#define kROM_Size 0x00020000

#define IncludePbufs 1
#define NumPbufs 4

#define EnableMouseMotion 1

#define IncludeHostTextClipExchange 1
#define EnableAutoSlow 1
#define EmLocalTalk 0
#define AutoLocation 1
#define AutoTimeZone 1
