#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include "CNFGRAPI.h"
#include "EMCONFIG.h"
#include "SYSDEPNS.h"
#include "UI/COMOSGLU.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "STRCONST.h"

/* --- keyboard input --- */

uint8_t SDLScan2MacKeyCode(SDL_Scancode i)
{
	uint8_t v = MKC_None;

	switch (i) {
		case SDL_SCANCODE_BACKSPACE: v = MKC_BackSpace; break;
		case SDL_SCANCODE_TAB: v = MKC_Tab; break;
		case SDL_SCANCODE_CLEAR: v = MKC_Clear; break;
		case SDL_SCANCODE_RETURN: v = MKC_Return; break;
		case SDL_SCANCODE_PAUSE: v = MKC_Pause; break;
		case SDL_SCANCODE_ESCAPE: v = MKC_formac_Escape; break;
		case SDL_SCANCODE_SPACE: v = MKC_Space; break;
		case SDL_SCANCODE_APOSTROPHE: v = MKC_SingleQuote; break;
		case SDL_SCANCODE_COMMA: v = MKC_Comma; break;
		case SDL_SCANCODE_MINUS: v = MKC_Minus; break;
		case SDL_SCANCODE_PERIOD: v = MKC_Period; break;
		case SDL_SCANCODE_SLASH: v = MKC_formac_Slash; break;
		case SDL_SCANCODE_0: v = MKC_0; break;
		case SDL_SCANCODE_1: v = MKC_1; break;
		case SDL_SCANCODE_2: v = MKC_2; break;
		case SDL_SCANCODE_3: v = MKC_3; break;
		case SDL_SCANCODE_4: v = MKC_4; break;
		case SDL_SCANCODE_5: v = MKC_5; break;
		case SDL_SCANCODE_6: v = MKC_6; break;
		case SDL_SCANCODE_7: v = MKC_7; break;
		case SDL_SCANCODE_8: v = MKC_8; break;
		case SDL_SCANCODE_9: v = MKC_9; break;
		case SDL_SCANCODE_SEMICOLON: v = MKC_SemiColon; break;
		case SDL_SCANCODE_EQUALS: v = MKC_Equal; break;

		case SDL_SCANCODE_LEFTBRACKET: v = MKC_LeftBracket; break;
		case SDL_SCANCODE_BACKSLASH: v = MKC_formac_BackSlash; break;
		case SDL_SCANCODE_RIGHTBRACKET: v = MKC_RightBracket; break;
		case SDL_SCANCODE_GRAVE: v = MKC_formac_Grave; break;

		case SDL_SCANCODE_A: v = MKC_A; break;
		case SDL_SCANCODE_B: v = MKC_B; break;
		case SDL_SCANCODE_C: v = MKC_C; break;
		case SDL_SCANCODE_D: v = MKC_D; break;
		case SDL_SCANCODE_E: v = MKC_E; break;
		case SDL_SCANCODE_F: v = MKC_F; break;
		case SDL_SCANCODE_G: v = MKC_G; break;
		case SDL_SCANCODE_H: v = MKC_H; break;
		case SDL_SCANCODE_I: v = MKC_I; break;
		case SDL_SCANCODE_J: v = MKC_J; break;
		case SDL_SCANCODE_K: v = MKC_K; break;
		case SDL_SCANCODE_L: v = MKC_L; break;
		case SDL_SCANCODE_M: v = MKC_M; break;
		case SDL_SCANCODE_N: v = MKC_N; break;
		case SDL_SCANCODE_O: v = MKC_O; break;
		case SDL_SCANCODE_P: v = MKC_P; break;
		case SDL_SCANCODE_Q: v = MKC_Q; break;
		case SDL_SCANCODE_R: v = MKC_R; break;
		case SDL_SCANCODE_S: v = MKC_S; break;
		case SDL_SCANCODE_T: v = MKC_T; break;
		case SDL_SCANCODE_U: v = MKC_U; break;
		case SDL_SCANCODE_V: v = MKC_V; break;
		case SDL_SCANCODE_W: v = MKC_W; break;
		case SDL_SCANCODE_X: v = MKC_X; break;
		case SDL_SCANCODE_Y: v = MKC_Y; break;
		case SDL_SCANCODE_Z: v = MKC_Z; break;

		case SDL_SCANCODE_KP_0: v = MKC_KP0; break;
		case SDL_SCANCODE_KP_1: v = MKC_KP1; break;
		case SDL_SCANCODE_KP_2: v = MKC_KP2; break;
		case SDL_SCANCODE_KP_3: v = MKC_KP3; break;
		case SDL_SCANCODE_KP_4: v = MKC_KP4; break;
		case SDL_SCANCODE_KP_5: v = MKC_KP5; break;
		case SDL_SCANCODE_KP_6: v = MKC_KP6; break;
		case SDL_SCANCODE_KP_7: v = MKC_KP7; break;
		case SDL_SCANCODE_KP_8: v = MKC_KP8; break;
		case SDL_SCANCODE_KP_9: v = MKC_KP9; break;

		case SDL_SCANCODE_KP_PERIOD: v = MKC_Decimal; break;
		case SDL_SCANCODE_KP_DIVIDE: v = MKC_KPDevide; break;
		case SDL_SCANCODE_KP_MULTIPLY: v = MKC_KPMultiply; break;
		case SDL_SCANCODE_KP_MINUS: v = MKC_KPSubtract; break;
		case SDL_SCANCODE_KP_PLUS: v = MKC_KPAdd; break;
		case SDL_SCANCODE_KP_ENTER: v = MKC_formac_Enter; break;
		case SDL_SCANCODE_KP_EQUALS: v = MKC_KPEqual; break;

		case SDL_SCANCODE_UP: v = MKC_Up; break;
		case SDL_SCANCODE_DOWN: v = MKC_Down; break;
		case SDL_SCANCODE_RIGHT: v = MKC_Right; break;
		case SDL_SCANCODE_LEFT: v = MKC_Left; break;
		case SDL_SCANCODE_INSERT: v = MKC_formac_Help; break;
		case SDL_SCANCODE_HOME: v = MKC_formac_Home; break;
		case SDL_SCANCODE_END: v = MKC_formac_End; break;
		case SDL_SCANCODE_PAGEUP: v = MKC_formac_PageUp; break;
		case SDL_SCANCODE_PAGEDOWN: v = MKC_formac_PageDown; break;

		case SDL_SCANCODE_F1: v = MKC_formac_F1; break;
		case SDL_SCANCODE_F2: v = MKC_formac_F2; break;
		case SDL_SCANCODE_F3: v = MKC_formac_F3; break;
		case SDL_SCANCODE_F4: v = MKC_formac_F4; break;
		case SDL_SCANCODE_F5: v = MKC_formac_F5; break;
		case SDL_SCANCODE_F6: v = MKC_F6; break;
		case SDL_SCANCODE_F7: v = MKC_F7; break;
		case SDL_SCANCODE_F8: v = MKC_F8; break;
		case SDL_SCANCODE_F9: v = MKC_F9; break;
		case SDL_SCANCODE_F10: v = MKC_F10; break;
		case SDL_SCANCODE_F11: v = MKC_F11; break;
		case SDL_SCANCODE_F12: v = MKC_F12; break;

		case SDL_SCANCODE_NUMLOCKCLEAR:
			v = MKC_formac_ForwardDel; break;
		case SDL_SCANCODE_CAPSLOCK: v = MKC_formac_CapsLock; break;
		case SDL_SCANCODE_SCROLLLOCK: v = MKC_ScrollLock; break;
		case SDL_SCANCODE_RSHIFT: v = MKC_formac_RShift; break;
		case SDL_SCANCODE_LSHIFT: v = MKC_formac_Shift; break;
		case SDL_SCANCODE_RCTRL: v = MKC_formac_RControl; break;
		case SDL_SCANCODE_LCTRL: v = MKC_formac_Control; break;
		case SDL_SCANCODE_RALT: v = MKC_formac_ROption; break;
		case SDL_SCANCODE_LALT: v = MKC_formac_Option; break;
		case SDL_SCANCODE_RGUI: v = MKC_formac_RCommand; break;
		case SDL_SCANCODE_LGUI: v = MKC_formac_Command; break;
		/* case SDLK_LSUPER: v = MKC_formac_Option; break; */
		/* case SDLK_RSUPER: v = MKC_formac_ROption; break; */

		case SDL_SCANCODE_HELP: v = MKC_formac_Help; break;
		case SDL_SCANCODE_PRINTSCREEN: v = MKC_Print; break;

		case SDL_SCANCODE_UNDO: v = MKC_formac_F1; break;
		case SDL_SCANCODE_CUT: v = MKC_formac_F2; break;
		case SDL_SCANCODE_COPY: v = MKC_formac_F3; break;
		case SDL_SCANCODE_PASTE: v = MKC_formac_F4; break;

		case SDL_SCANCODE_AC_HOME: v = MKC_formac_Home; break;

		case SDL_SCANCODE_KP_A: v = MKC_A; break;
		case SDL_SCANCODE_KP_B: v = MKC_B; break;
		case SDL_SCANCODE_KP_C: v = MKC_C; break;
		case SDL_SCANCODE_KP_D: v = MKC_D; break;
		case SDL_SCANCODE_KP_E: v = MKC_E; break;
		case SDL_SCANCODE_KP_F: v = MKC_F; break;

		case SDL_SCANCODE_KP_BACKSPACE: v = MKC_BackSpace; break;
		case SDL_SCANCODE_KP_CLEAR: v = MKC_Clear; break;
		case SDL_SCANCODE_KP_COMMA: v = MKC_Comma; break;
		case SDL_SCANCODE_KP_DECIMAL: v = MKC_Decimal; break;

		default:
			break;
	}

	return v;
}

void DoKeyCode(SDL_Keysym *r, bool down)
{
	uint8_t v = SDLScan2MacKeyCode(r->scancode);
	if (MKC_None != v) {
		// this probably breaks keyboard support
		//Keyboard_UpdateKeyMap2(v, down);
	}
}

void DisableKeyRepeat(void)
{
}

void RestoreKeyRepeat(void)
{
}

void ReconnectKeyCodes3(void)
{
}

void DisconnectKeyCodes3(void)
{
	//DisconnectKeyCodes2();
	MouseButtonSet(false);
}
