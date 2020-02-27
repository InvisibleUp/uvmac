#include "HW/KBRD/ALTKEYSM.h"
#include "SYSDEPNS.h"
#include "MYOSGLUE.h"
#include "UI/COMOSGLU.h"
#include "UI/CONTROLM.h"
#include "LANG/INTLCHAR.h"

LOCALVAR bool AltKeysLockText = false;
LOCALVAR bool AltKeysTrueCmnd = false;
LOCALVAR bool AltKeysTrueOption = false;
LOCALVAR bool AltKeysTrueShift = false;
LOCALVAR bool AltKeysModOn = false;
LOCALVAR bool AltKeysTextOn = false;

LOCALPROC CheckAltKeyUseMode(void)
{
	bool NewAltKeysTextOn;

	AltKeysModOn = AltKeysTrueCmnd
		|| AltKeysTrueOption || AltKeysTrueShift;
	NewAltKeysTextOn = AltKeysLockText || AltKeysModOn;
	if (NewAltKeysTextOn != AltKeysTextOn) {
		DisconnectKeyCodes(kKeepMaskControl | kKeepMaskCapsLock
			| (AltKeysTrueCmnd ? kKeepMaskCommand : 0)
			| (AltKeysTrueOption ? kKeepMaskOption : 0)
			| (AltKeysTrueShift ? kKeepMaskShift : 0));
		AltKeysTextOn = NewAltKeysTextOn;
	}
}

LOCALPROC Keyboard_UpdateKeyMap1(uint8_t key, bool down)
{
	if (MKC_Command == key) {
		AltKeysTrueCmnd = down;
		CheckAltKeyUseMode();
		Keyboard_UpdateKeyMap(key, down);
	} else if (MKC_Option == key) {
		AltKeysTrueOption = down;
		CheckAltKeyUseMode();
		Keyboard_UpdateKeyMap(key, down);
	} else if (MKC_Shift == key) {
		AltKeysTrueShift = down;
		CheckAltKeyUseMode();
		Keyboard_UpdateKeyMap(key, down);
	} else if (MKC_SemiColon == key) {
		if (down && ! AltKeysModOn) {
			if (AltKeysLockText) {
				AltKeysLockText = false;
				NeedWholeScreenDraw = true;
				SpecialModeClr(SpclModeAltKeyText);

				CheckAltKeyUseMode();
			}
		} else {
			Keyboard_UpdateKeyMap(key, down);
		}
	} else if (AltKeysTextOn) {
		Keyboard_UpdateKeyMap(key, down);
	} else if (MKC_M == key) {
		if (down) {
			if (! AltKeysLockText) {
				AltKeysLockText = true;
				SpecialModeSet(SpclModeAltKeyText);
				NeedWholeScreenDraw = true;
				CheckAltKeyUseMode();
			}
		}
	} else {
		switch (key) {
			case MKC_A:
				key = MKC_SemiColon;
				break;
			case MKC_B:
				key = MKC_BackSlash;
				break;
			case MKC_C:
				key = MKC_F3;
				break;
			case MKC_D:
				key = MKC_Option;
				break;
			case MKC_E:
				key = MKC_BackSpace;
				break;
			case MKC_F:
				key = MKC_Command;
				break;
			case MKC_G:
				key = MKC_Enter;
				break;
			case MKC_H:
				key = MKC_Equal;
				break;
			case MKC_I:
				key = MKC_Up;
				break;
			case MKC_J:
				key = MKC_Left;
				break;
			case MKC_K:
				key = MKC_Down;
				break;
			case MKC_L:
				key = MKC_Right;
				break;
			case MKC_M:
				/* handled above */
				break;
			case MKC_N:
				key = MKC_Minus;
				break;
			case MKC_O:
				key = MKC_RightBracket;
				break;
			case MKC_P:
				return; /* none */
				break;
			case MKC_Q:
				key = MKC_Grave;
				break;
			case MKC_R:
				key = MKC_Return;
				break;
			case MKC_S:
				key = MKC_Shift;
				break;
			case MKC_T:
				key = MKC_Tab;
				break;
			case MKC_U:
				key = MKC_LeftBracket;
				break;
			case MKC_V:
				key = MKC_F4;
				break;
			case MKC_W:
				return; /* none */
				break;
			case MKC_X:
				key = MKC_F2;
				break;
			case MKC_Y:
				key = MKC_Escape;
				break;
			case MKC_Z:
				key = MKC_F1;
				break;
			default:
				break;
		}
		Keyboard_UpdateKeyMap(key, down);
	}
}

LOCALPROC DisconnectKeyCodes1(uint32_t KeepMask)
{
	DisconnectKeyCodes(KeepMask);

	if (! (0 != (KeepMask & kKeepMaskCommand))) {
		AltKeysTrueCmnd = false;
	}
	if (! (0 != (KeepMask & kKeepMaskOption))) {
		AltKeysTrueOption = false;
	}
	if (! (0 != (KeepMask & kKeepMaskShift))) {
		AltKeysTrueShift = false;
	}
	AltKeysModOn = AltKeysTrueCmnd
		|| AltKeysTrueOption || AltKeysTrueShift;
	AltKeysTextOn = AltKeysLockText || AltKeysModOn;
}

LOCALPROC DrawAltKeyMode(void)
{
	int i;

	CurCellv0 = ControlBoxv0;
	CurCellh0 = ControlBoxh0;

	DrawCellAdvance(kInsertText00);
	for (i = (ControlBoxw - 4) / 2; --i >= 0; ) {
		DrawCellAdvance(kInsertText04);
	}
	DrawCellAdvance(kInsertText01);
	DrawCellAdvance(kInsertText02);
	for (i = (ControlBoxw - 4) / 2; --i >= 0; ) {
		DrawCellAdvance(kInsertText04);
	}
	DrawCellAdvance(kInsertText03);
}