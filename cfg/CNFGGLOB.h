/*
	Configuration options used by both platform specific
	and platform independent code.

	Hopefully, one day, we can remove this file entirely.
*/

#ifndef CNFGGLOB_H
#define CNFGGLOB_H

#include <stdint.h>

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

#define IncludePbufs 1
#define NumPbufs 4

#define EnableMouseMotion 0

#define IncludeHostTextClipExchange 0
#define EmLocalTalk 0
#define AutoLocation 1
#define AutoTimeZone 1

// Variable versions of configuration flags
extern uint16_t vMacScreenHeight;
extern uint16_t vMacScreenWidth;
extern uint16_t vMacScreenDepth;

#endif
