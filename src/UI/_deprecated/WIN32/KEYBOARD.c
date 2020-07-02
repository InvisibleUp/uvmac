#include <windows.h>
#include "SYSDEPNS.h"
#include "CNFGGLOB.h"
#include "CNFGRAPI.h"
#include "HW/KBRD/KEYCODES.h"
#include "UI/COMOSGLU.h"
#include "UI/WIN32/OSGLUWIN.h"
#include "UI/CONTROLM.h"
#include "UTIL/ENDIANAC.h"

/* keyboard */

static uint8_t WinKey2Mac[256];

static inline void AssignOneMacKey(uint8_t WinKey, uint8_t MacKey)
{
	WinKey2Mac[WinKey] = MacKey;
}

bool InitWinKey2Mac(void)
{
	memset(WinKey2Mac, MKC_None, sizeof(WinKey2Mac));

	AssignOneMacKey('A', MKC_A);
	AssignOneMacKey('S', MKC_S);
	AssignOneMacKey('D', MKC_D);
	AssignOneMacKey('F', MKC_F);
	AssignOneMacKey('H', MKC_H);
	AssignOneMacKey('G', MKC_G);
	AssignOneMacKey('Z', MKC_Z);
	AssignOneMacKey('X', MKC_X);
	AssignOneMacKey('C', MKC_C);
	AssignOneMacKey('V', MKC_V);
	AssignOneMacKey('B', MKC_B);
	AssignOneMacKey('Q', MKC_Q);
	AssignOneMacKey('W', MKC_W);
	AssignOneMacKey('E', MKC_E);
	AssignOneMacKey('R', MKC_R);
	AssignOneMacKey('Y', MKC_Y);
	AssignOneMacKey('T', MKC_T);
	AssignOneMacKey('1', MKC_1);
	AssignOneMacKey('2', MKC_2);
	AssignOneMacKey('3', MKC_3);
	AssignOneMacKey('4', MKC_4);
	AssignOneMacKey('6', MKC_6);
	AssignOneMacKey('5', MKC_5);
	AssignOneMacKey(myVK_Equal, MKC_Equal);
	AssignOneMacKey('9', MKC_9);
	AssignOneMacKey('7', MKC_7);
	AssignOneMacKey(myVK_Subtract, MKC_Minus);
	AssignOneMacKey('8', MKC_8);
	AssignOneMacKey('0', MKC_0);
	AssignOneMacKey(myVK_RightBracket, MKC_RightBracket);
	AssignOneMacKey('O', MKC_O);
	AssignOneMacKey('U', MKC_U);
	AssignOneMacKey(myVK_LeftBracket, MKC_LeftBracket);
	AssignOneMacKey('I', MKC_I);
	AssignOneMacKey('P', MKC_P);
	AssignOneMacKey(VK_RETURN, MKC_Return);
	AssignOneMacKey('L', MKC_L);
	AssignOneMacKey('J', MKC_J);
	AssignOneMacKey(myVK_SingleQuote, MKC_SingleQuote);
	AssignOneMacKey('K', MKC_K);
	AssignOneMacKey(myVK_SemiColon, MKC_SemiColon);
	AssignOneMacKey(myVK_BackSlash, MKC_formac_BackSlash);
	AssignOneMacKey(myVK_Comma, MKC_Comma);
	AssignOneMacKey(myVK_Slash, MKC_formac_Slash);
	AssignOneMacKey('N', MKC_N);
	AssignOneMacKey('M', MKC_M);
	AssignOneMacKey(myVK_Period, MKC_Period);

	AssignOneMacKey(VK_TAB, MKC_Tab);
	AssignOneMacKey(VK_SPACE, MKC_Space);
	AssignOneMacKey(myVK_Grave, MKC_formac_Grave);
	AssignOneMacKey(VK_BACK, MKC_BackSpace);
	AssignOneMacKey(VK_ESCAPE, MKC_formac_Escape);

	AssignOneMacKey(VK_MENU, MKC_formac_Command);

	AssignOneMacKey(VK_LMENU, MKC_formac_Command);

	AssignOneMacKey(VK_RMENU, MKC_formac_RCommand);

	AssignOneMacKey(VK_SHIFT, MKC_formac_Shift);
	AssignOneMacKey(VK_LSHIFT, MKC_formac_Shift);
	AssignOneMacKey(VK_RSHIFT, MKC_formac_RShift);

	AssignOneMacKey(VK_CAPITAL, MKC_formac_CapsLock);

	AssignOneMacKey(VK_APPS, MKC_formac_ROption);
	AssignOneMacKey(VK_LWIN, MKC_formac_Option);
	AssignOneMacKey(VK_RWIN, MKC_formac_ROption);
	AssignOneMacKey(VK_CONTROL, MKC_formac_Control);
	AssignOneMacKey(VK_LCONTROL, MKC_formac_Control);
	AssignOneMacKey(VK_RCONTROL, MKC_formac_RControl);

	AssignOneMacKey(VK_F1, MKC_formac_F1);
	AssignOneMacKey(VK_F2, MKC_formac_F2);
	AssignOneMacKey(VK_F3, MKC_formac_F3);
	AssignOneMacKey(VK_F4, MKC_formac_F4);
	AssignOneMacKey(VK_F5, MKC_formac_F5);
	AssignOneMacKey(VK_F6, MKC_F6);
	AssignOneMacKey(VK_F7, MKC_F7);
	AssignOneMacKey(VK_F8, MKC_F8);
	AssignOneMacKey(VK_F9, MKC_F9);
	AssignOneMacKey(VK_F10, MKC_F10);
	AssignOneMacKey(VK_F11, MKC_F11);
	AssignOneMacKey(VK_F12, MKC_F12);

	AssignOneMacKey(VK_DECIMAL, MKC_Decimal);
	AssignOneMacKey(VK_DELETE, MKC_Decimal);
	/* AssignOneMacKey(VK_RIGHT, 0x42); */
	AssignOneMacKey(VK_MULTIPLY, MKC_KPMultiply);
	AssignOneMacKey(VK_ADD, MKC_KPAdd);
	/* AssignOneMacKey(VK_LEFT, 0x46); */
	AssignOneMacKey(VK_NUMLOCK, MKC_Clear);

	/* AssignOneMacKey(VK_DOWN, 0x48); */
	AssignOneMacKey(VK_DIVIDE, MKC_KPDevide);
	/* AssignOneMacKey(VK_RETURN, MKC_formac_Enter); */
	/* AssignOneMacKey(VK_UP, 0x4D); */
	AssignOneMacKey(VK_DIVIDE, MKC_KPDevide);
	AssignOneMacKey(VK_SUBTRACT, MKC_KPSubtract);

	AssignOneMacKey(VK_SEPARATOR, MKC_KPEqual);
	AssignOneMacKey(VK_NUMPAD0, MKC_KP0);
	AssignOneMacKey(VK_NUMPAD1, MKC_KP1);
	AssignOneMacKey(VK_NUMPAD2, MKC_KP2);
	AssignOneMacKey(VK_NUMPAD3, MKC_KP3);
	AssignOneMacKey(VK_NUMPAD4, MKC_KP4);
	AssignOneMacKey(VK_NUMPAD5, MKC_KP5);

	AssignOneMacKey(VK_NUMPAD6, MKC_KP6);
	AssignOneMacKey(VK_NUMPAD7, MKC_KP7);
	AssignOneMacKey(VK_NUMPAD8, MKC_KP8);
	AssignOneMacKey(VK_NUMPAD9, MKC_KP9);

	AssignOneMacKey(VK_LEFT, MKC_Left);
	AssignOneMacKey(VK_RIGHT, MKC_Right);
	AssignOneMacKey(VK_DOWN, MKC_Down);
	AssignOneMacKey(VK_UP, MKC_Up);

	AssignOneMacKey(myVK_PRIOR, MKC_formac_PageUp);
	AssignOneMacKey(myVK_NEXT, MKC_formac_PageDown);
	AssignOneMacKey(myVK_END, MKC_formac_End);
	AssignOneMacKey(myVK_HOME, MKC_formac_Home);
	AssignOneMacKey(myVK_INSERT, MKC_formac_Help);
	AssignOneMacKey(myVK_DELETE, MKC_formac_ForwardDel);
	AssignOneMacKey(myVK_HELP, MKC_formac_Help);
	AssignOneMacKey(myVK_SNAPSHOT, MKC_Print);
	AssignOneMacKey(myVK_SCROLL, MKC_ScrollLock);
	AssignOneMacKey(myVK_PAUSE, MKC_Pause);

	AssignOneMacKey(myVK_OEM_102, MKC_AngleBracket);

	InitKeyCodes();

#if ItnlKyBdFix
	InitCheckKeyboardLayout();
#endif

	return true;
}

LOCALPROC DoKeyCode(int i, bool down)
{
	uint8_t key = WinKey2Mac[ItnlKyBdFix ? VkMapA[i] : i];
	if (MKC_None != key) {
		Keyboard_UpdateKeyMap2(key, down);
	}
}

#if EnableGrabSpecialKeys
LOCALVAR bool HaveSetSysParam = false;
#endif

LOCALPROC CheckTheCapsLock(void)
{
	DoKeyCode(VK_CAPITAL, (GetKeyState(VK_CAPITAL) & 1) != 0);
}

#if EnableGrabSpecialKeys
LOCALVAR bool VK_LWIN_pressed = false;
LOCALVAR bool VK_RWIN_pressed = false;

void CheckForLostKeyUps(void)
{
	if (HaveSetSysParam) {
		/* check for lost key ups */
		if (VK_LWIN_pressed) {
			if ((GetAsyncKeyState(VK_LWIN) & 0x8000) == 0) {
				DoKeyCode(VK_LWIN, false);
				VK_LWIN_pressed = false;
			}
		}
		if (VK_RWIN_pressed) {
			if ((GetAsyncKeyState(VK_RWIN) & 0x8000) == 0) {
				DoKeyCode(VK_RWIN, false);
				VK_RWIN_pressed = false;
			}
		}
	}
}
#endif

LOCALPROC DoVKcode0(int i, bool down)
{
#if EnableGrabSpecialKeys
	if (HaveSetSysParam) {
		/* will need to check for lost key ups */
		if (VK_LWIN == i) {
			VK_LWIN_pressed = down;
		} else if (VK_RWIN == i) {
			VK_RWIN_pressed = down;
		}
	}
#endif
	DoKeyCode(i, down);
}

void DoVKcode(int i, uint8_t flags, bool down)
{
	switch (i) {
#if MKC_formac_Control != MKC_formac_RControl
		case VK_CONTROL:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_RControl : MKC_formac_Control,
				down);
			break;
#endif
#if MKC_formac_RCommand != MKC_formac_Command
		case VK_MENU:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_RCommand : MKC_formac_Command,
				down);
			break;
#endif
		case VK_RETURN:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_Enter : MKC_Return,
				down);
			break;
		case myVK_HOME:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_Home : MKC_KP7,
				down);
			break;
		case VK_UP:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_Up : MKC_KP8,
				down);
			break;
		case myVK_PRIOR:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_PageUp : MKC_KP9,
				down);
			break;
		case VK_LEFT:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_Left : MKC_KP4,
				down);
			break;
		case myVK_CLEAR:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_Clear : MKC_KP5,
				down);
			break;
		case VK_RIGHT:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_Right : MKC_KP6,
				down);
			break;
		case myVK_END:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_End : MKC_KP1,
				down);
			break;
		case VK_DOWN:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_Down : MKC_KP2,
				down);
			break;
		case myVK_NEXT:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_PageDown : MKC_KP3,
				down);
			break;
		case myVK_INSERT:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_Help : MKC_KP0,
				down);
			break;
		case myVK_DELETE:
			Keyboard_UpdateKeyMap2(TestBit(flags, 0)
				? MKC_formac_ForwardDel : MKC_Decimal,
				down);
			break;
		case VK_CAPITAL:
			CheckTheCapsLock();
			break;
		default:
			if ((i >= 0) && (i < 256)) {
				DoVKcode0(i, down);
			}
			break;
	}
}

bool WantCmdOptOnReconnect = false;

void ReconnectKeyCodes3(void)
{
	int i;

	CheckTheCapsLock();

	if (WantCmdOptOnReconnect) {
		WantCmdOptOnReconnect = false;

		for (i = 0; i < 256; ++i) {
			if ((GetKeyState(i) & 0x8000) != 0) {
				if ((VK_CAPITAL != i)
					&& (VK_RETURN != i))
				{
					DoVKcode0(i, true);
				}
			}
		}
	}
}

void DisconnectKeyCodes3(void)
{
	DisconnectKeyCodes2();
	SetCurMouseButton(false);
}

#if EnableGrabSpecialKeys
static HHOOK hKeyHook = NULL;
#endif

#if EnableGrabSpecialKeys
typedef struct {
	DWORD   vkCode;
	DWORD   scanCode;
	DWORD   flags;
	DWORD   time;
	DWORD   dwExtraInfo;
} _KBDLLHOOKSTRUCT;
#endif

#if EnableGrabSpecialKeys
LRESULT CALLBACK LowLevelKeyboardProc(
	int nCode,     /* hook code */
	WPARAM wParam, /* message identifier */
	LPARAM lParam  /* pointer to structure with message data */
);
#endif

#if EnableGrabSpecialKeys
LRESULT CALLBACK LowLevelKeyboardProc(
	int nCode,     /* hook code */
	WPARAM wParam, /* message identifier */
	LPARAM lParam  /* pointer to structure with message data */
)
{
	if (nCode == HC_ACTION) {
		_KBDLLHOOKSTRUCT *p = (_KBDLLHOOKSTRUCT *)lParam;
		if (p->vkCode != VK_CAPITAL) {
			switch (wParam) {
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					DoVKcode(p->vkCode, p->flags, true);
					return 1;
					break;
				case WM_KEYUP:
				case WM_SYSKEYUP:
					DoVKcode(p->vkCode, p->flags, false);
					return 1;
					break;
			}
		}
	}
	return CallNextHookEx(hKeyHook, /* handle to current hook */
		nCode, /* hook code passed to hook procedure */
		wParam, /* value passed to hook procedure */
		lParam /* value passed to hook procedure */
	);

}
#endif

#if EnableGrabSpecialKeys
#define _WH_KEYBOARD_LL 13
#define _SPI_SETSCREENSAVERRUNNING 0x0061
#endif

#if EnableGrabSpecialKeys
LOCALVAR UINT nPreviousState;
#endif

#if EnableGrabSpecialKeys
void GrabSpecialKeys(void)
{
	if ((hKeyHook == NULL) && ! HaveSetSysParam) {
		/* this works on Windows XP */
		hKeyHook = SetWindowsHookEx(
			_WH_KEYBOARD_LL, /* type of hook to install */
			(HOOKPROC)LowLevelKeyboardProc,
				/* address of hook procedure */
			AppInstance,    /* handle to application instance */
			0   /* identity of thread to install hook for */
		);
		if (hKeyHook == NULL) {
			/* this works on Windows 95/98 */
			SystemParametersInfo(_SPI_SETSCREENSAVERRUNNING, TRUE,
				&nPreviousState, 0);
			HaveSetSysParam = true;
		}
	}
}

void UnGrabSpecialKeys(void)
{
	if (hKeyHook != NULL) {
		(void) UnhookWindowsHookEx(hKeyHook);
		hKeyHook = NULL;
	}
	if (HaveSetSysParam) {
		SystemParametersInfo(_SPI_SETSCREENSAVERRUNNING, FALSE,
			&nPreviousState, 0);
		HaveSetSysParam = false;
	}
}
#endif
