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

/* Globals */

extern uimr SpecialModes;
extern bool NeedWholeScreenDraw;
extern uint8_t * CntrlDisplayBuff;

/* Macros and such */

enum {
	SpclModeNoRom,
	SpclModeMessage,
	SpclModeControl,
	kNumSpclModes
};

#define SpecialModeSet(i) SpecialModes |= (1 << (i))
#define SpecialModeClr(i) SpecialModes &= ~ (1 << (i))
#define SpecialModeTst(i) (0 != (SpecialModes & (1 << (i))))

#define MacMsgDisplayed SpecialModeTst(SpclModeMessage)

/* Message display */

void MacMsgOverride(char *briefMsg, char *longMsg);
void MacMsgDisplayOff(void);
void MacMsgDisplayOn(void);

#if NeedDoMoreCommandsMsg
void DoMoreCommandsMsg(void);
#endif
#if NeedDoAboutMsg
void DoAboutMsg(void);
#endif

/* Screen drawing */

uint8_t * GetCurDrawBuff(void);

/* Things that really don't belong here */

void Keyboard_UpdateKeyMap2(uint8_t key, bool down);
void DisconnectKeyCodes2(void);

MacErr_t ROM_IsValid(void);
bool WaitForRom(void);

#endif // CONTROLM_H
