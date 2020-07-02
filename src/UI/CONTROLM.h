/*
	CONTROL Mode
*/

#ifndef CONTROLM_H
#define CONTROLM_H

#include "SYSDEPNS.h"
#include "ERRCODES.h"

#ifndef CheckRomCheckSum
#define CheckRomCheckSum 1
#endif

#ifndef NeedRequestIthDisk
#define NeedRequestIthDisk 0
#endif

/* Globals */

extern uimr SpecialModes;
extern bool NeedWholeScreenDraw;
extern uint8_t * CntrlDisplayBuff;
extern bool ControlKeyPressed;
extern bool RequestInsertDisk;
extern bool WantMagnify;
extern bool RunInBackground;
extern bool SpeedStopped;
extern bool WantFullScreen;

/* Public Functions */
void MacMsgOverride(char *title, char *msg);
MacErr_t ROM_IsValid(void);
bool WaitForRom(void);

#endif // CONTROLM_H
