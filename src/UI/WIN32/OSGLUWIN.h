#ifndef OSGLUWIN_H
#define OSGLUWIN_H

#include "SYSDEPNS.h"
#include "CNFGRAPI.h"
#include "CNFGGLOB.h"
#include "UI/COMOSGLU.h"
#include "UI/CONTROLM.h"

/* Define the undefined */

#ifndef ItnlKyBdFix
#define ItnlKyBdFix 0
#endif

#ifdef UNICODE
#define UseUni 1
#else
#define UseUni 0
#endif

#ifndef InstallFileIcons
#define InstallFileIcons 0
#endif

#ifndef EnableGrabSpecialKeys
#define EnableGrabSpecialKeys (MayFullScreen && GrabKeysFullScreen)
#endif /* EnableGrabSpecialKeys */

/* Resource Ids */

#define IDI_VMAC      256
#if InstallFileIcons
#define IDI_ROM       257
#define IDI_DISK      258
#endif

/* API differences */

bool GetAppDir(LPTSTR pathName);

/* Utilities (move?) */

#define TestBit(i, p) (((unsigned long)(i) & PowOf2(p)) != 0)

/* Parameter buffers */

#if IncludePbufs
#define PbufHaveLock 1
HGLOBAL PbufDat[NumPbufs];
#endif

/* Main window info */

extern HWND MainWnd;
HINSTANCE AppInstance;
int WndX;
int WndY;
extern bool UseFullScreen;
extern bool UseMagnify;

#if MayFullScreen
short hOffset;
short vOffset;
#endif

void ToggleWantFullScreen(void);

/* cursor */

extern bool HaveCursorHidden;
extern bool WantCursorHidden;
extern bool MouseCaptured;

void SetCurMouseButton(bool v);

/* Keyboard */

#ifdef ItnlKyBdFix
uint8_t VkMapA[256];
void InitCheckKeyboardLayout(void);
void CheckKeyboardLayout(void);
#endif

/* these constants weren't in the header files I have */
#define myVK_Subtract 0xBD
#define myVK_Equal 0xBB
#define myVK_BackSlash 0xDC
#define myVK_Comma 0xBC
#define myVK_Period 0xBE
#define myVK_Slash 0xBF
#define myVK_SemiColon 0xBA
#define myVK_SingleQuote 0xDE
#define myVK_LeftBracket 0xDB
#define myVK_RightBracket 0xDD
#define myVK_Grave 0xC0

/* some new ones, need to check if in all header versions */
#define myVK_PRIOR 0x21
#define myVK_NEXT 0x22
#define myVK_END 0x23
#define myVK_HOME 0x24
#define myVK_INSERT 0x2D
#define myVK_DELETE 0x2E
#define myVK_HELP 0x2F
#define myVK_SCROLL 0x91
#define myVK_SNAPSHOT 0x2C
#define myVK_PAUSE 0x13
#define myVK_CLEAR 0x0C

#define myVK_OEM_8 0xDF
#define myVK_OEM_102 0xE2

extern bool WantCmdOptOnReconnect;

void DoVKcode(int i, uint8_t flags, bool down);
void DisconnectKeyCodes3(void);
void ReconnectKeyCodes3(void);
bool InitWinKey2Mac(void);

#if EnableGrabSpecialKeys
void GrabSpecialKeys(void);
void UnGrabSpecialKeys(void);
void CheckForLostKeyUps(void);
#endif

#endif // OSGLUWIN_H
