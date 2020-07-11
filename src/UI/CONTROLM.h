/*
	CONTROL Mode
*/

#ifndef CONTROLM_H
#define CONTROLM_H

#include "SYSDEPNS.h"
#include "ERRCODES.h"

/* Globals */

extern uimr SpecialModes;
extern bool NeedWholeScreenDraw;
extern uint8_t * CntrlDisplayBuff;

/* Public Functions */
void MacMsgOverride(char *title, char *msg);

#endif // CONTROLM_H
