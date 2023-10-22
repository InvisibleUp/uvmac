/*
	OSGLUSD2.c

	Copyright (C) 2012 Paul C. Pratt, Manuel Alfayate

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	Operating System GLUe for SDl 2.0 library

	All operating system dependent code for the
	SDL Library should go here.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "STRCONST.h"
#include "OSGLUSD2.h"
#include "LANG/INTLCHAR.h"
#include "HW/SCREEN/SCRNEMDV.h"
#include "HW/ROM/ROMEMDEV.h"
#include "CFGMAN.h"

/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

/* --- information about the environment --- */

#define WantColorTransValid 0

#include "UI/COMOSGLU.h"
#include "UTIL/PBUFSTDC.h"
#include "UI/CONTROLM.h"

/* --- basic dialogs --- */

static void CheckSavedMacMsg(void)
{
	/* called only on quit, if error saved but not yet reported */

	if (nullpr != SavedBriefMsg) {
		char briefMsg0[ClStrMaxLength + 1];
		char longMsg0[ClStrMaxLength + 1];

		NativeStrFromCStr(briefMsg0, SavedBriefMsg);
		NativeStrFromCStr(longMsg0, SavedLongMsg);

		if (0 != SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			SavedBriefMsg,
			SavedLongMsg,
			main_wind
			))
		{
			fprintf(stderr, "%s\n", briefMsg0);
			fprintf(stderr, "%s\n", longMsg0);
		}

		SavedBriefMsg = nullpr;
	}
}

/* --- event handling for main window --- */

#define UseMotionEvents 1

#if UseMotionEvents
static bool CaughtMouse = false;
#endif

static void HandleTheEvent(SDL_Event *event)
{
	switch (event->type) {
		case SDL_QUIT:
			RequestMacOff = true;
			break;
		case SDL_WINDOWEVENT:
			switch (event->window.event) {
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					gTrueBackgroundFlag = 0;
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					gTrueBackgroundFlag = 1;
					break;
				case SDL_WINDOWEVENT_ENTER:
					CaughtMouse = 1;
					break;
				case SDL_WINDOWEVENT_LEAVE:
					CaughtMouse = 0;
					break;
			}
			break;
		case SDL_MOUSEMOTION:
#if EnableFSMouseMotion && ! HaveWorkingWarp
			if (HaveMouseMotion) {
				MousePositionNotifyRelative(
					event->motion.xrel, event->motion.yrel);
			} else
#endif
			{
				MousePositionNotify(
					event->motion.x, event->motion.y);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			/* any mouse button, we don't care which */
#if EnableFSMouseMotion && ! HaveWorkingWarp
			if (HaveMouseMotion) {
				/* ignore position */
			} else
#endif
			{
				MousePositionNotify(
					event->button.x, event->button.y);
			}
			MouseButtonSet(true);
			break;
		case SDL_MOUSEBUTTONUP:
#if EnableFSMouseMotion && ! HaveWorkingWarp
			if (HaveMouseMotion) {
				/* ignore position */
			} else
#endif
			{
				MousePositionNotify(
					event->button.x, event->button.y);
			}
			MouseButtonSet(false);
			break;
		case SDL_KEYDOWN:
			DoKeyCode(&event->key.keysym, true);
			break;
		case SDL_KEYUP:
			DoKeyCode(&event->key.keysym, false);
			break;
		/*case SDL_MOUSEWHEEL:
			if (event->wheel.x < 0) {
				Keyboard_UpdateKeyMap2(MKC_Left, true);
				Keyboard_UpdateKeyMap2(MKC_Left, false);
			} else if (event->wheel.x > 0) {
				Keyboard_UpdateKeyMap2(MKC_Right, true);
				Keyboard_UpdateKeyMap2(MKC_Right, false);
			}
			if (event->wheel.y < 0) {
				Keyboard_UpdateKeyMap2(MKC_Down, true);
				Keyboard_UpdateKeyMap2(MKC_Down, false);
			} else if(event->wheel.y > 0) {
				Keyboard_UpdateKeyMap2(MKC_Up, true);
				Keyboard_UpdateKeyMap2(MKC_Up, false);
			}
			break;*/
		case SDL_DROPFILE:
			{
				char *s = event->drop.file;

				(void) Sony_Insert1a(s, false);
				SDL_RaiseWindow(main_wind);
				SDL_free(s);
			}
			break;
#if 0
		case Expose: /* SDL doesn't have an expose event */
			int x0 = event->expose.x;
			int y0 = event->expose.y;
			int x1 = x0 + event->expose.width;
			int y1 = y0 + event->expose.height;

			if (x0 < 0) {
				x0 = 0;
			}
			if (x1 > vMacScreenWidth) {
				x1 = vMacScreenWidth;
			}
			if (y0 < 0) {
				y0 = 0;
			}
			if (y1 > vMacScreenHeight) {
				y1 = vMacScreenHeight;
			}
			if ((x0 < x1) && (y0 < y1)) {
				HaveChangedScreenBuff(y0, x0, y1, x1);
			}
			break;
#endif
	}
}

/* --- main window creation and disposal --- */

static int argc;
static char **argv;

static bool SDL_InitDisplay(void)
{
	bool v = false;

	InitKeyCodes();

	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
	} else {
		v = true;
	}

	return v;
}

#if MayFullScreen
static bool GrabMachine = false;
#endif

#if MayFullScreen
static void GrabTheMachine(void)
{
#if GrabKeysFullScreen
	SDL_SetWindowGrab(main_wind, SDL_TRUE);
#endif

#if EnableFSMouseMotion

#if HaveWorkingWarp
	/*
		if magnification changes, need to reset,
		even if HaveMouseMotion already true
	*/
	if (MoveMouse(ViewHStart + (ViewHSize / 2),
		ViewVStart + (ViewVSize / 2)))
	{
		SavedMouseH = ViewHStart + (ViewHSize / 2);
		SavedMouseV = ViewVStart + (ViewVSize / 2);
		HaveMouseMotion = true;
	}
#else
	if (0 == SDL_SetRelativeMouseMode(SDL_ENABLE)) {
		HaveMouseMotion = true;
	}
#endif

#endif /* EnableFSMouseMotion */
}
#endif

#if MayFullScreen
static void UngrabMachine(void)
{
#if EnableFSMouseMotion

	if (HaveMouseMotion) {
#if HaveWorkingWarp
		(void) MoveMouse(CurMouseH, CurMouseV);
#else
		SDL_SetRelativeMouseMode(SDL_DISABLE);
#endif

		HaveMouseMotion = false;
	}

#endif /* EnableFSMouseMotion */

#if GrabKeysFullScreen
	SDL_SetWindowGrab(main_wind, SDL_FALSE);
#endif
}
#endif

#if EnableFSMouseMotion && HaveWorkingWarp
static void MouseConstrain(void)
{
	int16_t shiftdh;
	int16_t shiftdv;

	if (SavedMouseH < ViewHStart + (ViewHSize / 4)) {
		shiftdh = ViewHSize / 2;
	} else if (SavedMouseH > ViewHStart + ViewHSize - (ViewHSize / 4)) {
		shiftdh = - ViewHSize / 2;
	} else {
		shiftdh = 0;
	}
	if (SavedMouseV < ViewVStart + (ViewVSize / 4)) {
		shiftdv = ViewVSize / 2;
	} else if (SavedMouseV > ViewVStart + ViewVSize - (ViewVSize / 4)) {
		shiftdv = - ViewVSize / 2;
	} else {
		shiftdv = 0;
	}
	if ((shiftdh != 0) || (shiftdv != 0)) {
		SavedMouseH += shiftdh;
		SavedMouseV += shiftdv;
		if (! MoveMouse(SavedMouseH, SavedMouseV)) {
			HaveMouseMotion = false;
		}
	}
}
#endif

enum {
	kMagStateNormal,
#if 1
	kMagStateMagnifgy,
#endif
	kNumMagStates
};

#define kMagStateAuto kNumMagStates

#if MayNotFullScreen
static int CurWinIndx;
static bool HavePositionWins[kNumMagStates];
static int WinPositionsX[kNumMagStates];
static int WinPositionsY[kNumMagStates];
#endif

static bool CreateMainWindow(void)
{
	int NewWindowX;
	int NewWindowY;
	int NewWindowHeight = vMacScreenHeight;
	int NewWindowWidth = vMacScreenWidth;
	Uint32 flags = 0 /* SDL_WINDOW_HIDDEN */;
	bool v = false;

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		/*
			We don't want physical screen mode to be changed in modern
			displays, so we pass this _DESKTOP flag.
		*/
		flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		NewWindowX = SDL_WINDOWPOS_UNDEFINED;
		NewWindowY = SDL_WINDOWPOS_UNDEFINED;
	}
#endif
#if 1
	else
#endif
#if MayNotFullScreen
	{
		int WinIndx;

#if 1
		if (UseMagnify) {
			WinIndx = kMagStateMagnifgy;
		} else
#endif
		{
			WinIndx = kMagStateNormal;
		}

		if (! HavePositionWins[WinIndx]) {
			NewWindowX = SDL_WINDOWPOS_CENTERED;
			NewWindowY = SDL_WINDOWPOS_CENTERED;
		} else {
			NewWindowX = WinPositionsX[WinIndx];
			NewWindowY = WinPositionsY[WinIndx];
		}

		CurWinIndx = WinIndx;
	}
#endif

#if 0
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
#endif

	if (NULL == (main_wind = SDL_CreateWindow(
		kStrAppName,
		NewWindowX, NewWindowY,
		NewWindowWidth, NewWindowHeight,
		flags)))
	{
		fprintf(stderr, "SDL_CreateWindow fails: %s\n", SDL_GetError());
	} else
	if (NULL == (renderer = SDL_CreateRenderer(
		main_wind, -1,
		0 /* SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC */
			/*
				SDL_RENDERER_ACCELERATED not needed
				"no flags gives priority to available
				SDL_RENDERER_ACCELERATED renderers"
			*/
			/* would rather not require vsync */
		)))
	{
		fprintf(stderr, "SDL_CreateRenderer fails: %s\n", SDL_GetError());
	} else
	if (NULL == (texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_RGBX8888,
		SDL_TEXTUREACCESS_STREAMING,
		vMacScreenWidth, vMacScreenHeight
		)))
	{
		fprintf(stderr, "SDL_CreateTexture fails: %s\n", SDL_GetError());
	} else
	if (NULL == (format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888)))
	{
		fprintf(stderr, "SDL_AllocFormat fails: %s\n", SDL_GetError());
	} else
	{
		SDL_RenderClear(renderer);

		SDL_DisplayMode info;

		if (0 != SDL_GetCurrentDisplayMode(0, &info)) {
			fprintf(stderr, "SDL_GetCurrentDisplayMode fails: %s\n",
				SDL_GetError());

			return false;
		}

#if 1
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			int wr;
			int hr;

			SDL_GL_GetDrawableSize(main_wind, &wr, &hr);

			ViewHSize = wr;
			ViewVSize = hr;
#if 1
			if (UseMagnify) {
				ViewHSize /= WindowScale;
				ViewVSize /= WindowScale;
			}
#endif
			if (ViewHSize >= vMacScreenWidth) {
				ViewHStart = 0;
				ViewHSize = vMacScreenWidth;
			} else {
				ViewHSize &= ~ 1;
			}
			if (ViewVSize >= vMacScreenHeight) {
				ViewVStart = 0;
				ViewVSize = vMacScreenHeight;
			} else {
				ViewVSize &= ~ 1;
			}

			if (wr > NewWindowWidth) {
				hOffset = (wr - NewWindowWidth) / 2;
			} else {
				hOffset = 0;
			}
			if (hr > NewWindowHeight) {
				vOffset = (hr - NewWindowHeight) / 2;
			} else {
				vOffset = 0;
			}
		}
#endif

#if 0 != vMacScreenDepth
		ColorModeWorks = true;
#endif

		v = true;
	}

	return v;
}

static void CloseMainWindow(void)
{
	if (NULL != format) {
		SDL_FreeFormat(format);
		format = NULL;
	}

	if (NULL != texture) {
		SDL_DestroyTexture(texture);
		texture = NULL;
	}

	if (NULL != renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = NULL;
	}

	if (NULL != main_wind) {
		SDL_DestroyWindow(main_wind);
		main_wind = NULL;
	}
}

#if EnableRecreateW
static void ZapWState(void)
{
	main_wind = NULL;
	renderer = NULL;
	texture = NULL;
	format = NULL;
}
#endif

#if EnableRecreateW
struct WState {
#if MayFullScreen
	uint16_t f_ViewHSize;
	uint16_t f_ViewVSize;
	uint16_t f_ViewHStart;
	uint16_t f_ViewVStart;
	int f_hOffset;
	int f_vOffset;
#endif
#if 1
	bool f_UseFullScreen;
#endif
#if 1
	bool f_UseMagnify;
#endif
#if MayNotFullScreen
	int f_CurWinIndx;
#endif
	SDL_Window *f_main_wind;
	SDL_Renderer *f_renderer;
	SDL_Texture *f_texture;
	SDL_PixelFormat *f_format;
};
typedef struct WState WState;
#endif

#if EnableRecreateW
static void GetWState(WState *r)
{
#if MayFullScreen
	r->f_ViewHSize = ViewHSize;
	r->f_ViewVSize = ViewVSize;
	r->f_ViewHStart = ViewHStart;
	r->f_ViewVStart = ViewVStart;
	r->f_hOffset = hOffset;
	r->f_vOffset = vOffset;
#endif
#if 1
	r->f_UseFullScreen = UseFullScreen;
#endif
#if 1
	r->f_UseMagnify = UseMagnify;
#endif
#if MayNotFullScreen
	r->f_CurWinIndx = CurWinIndx;
#endif
	r->f_main_wind = main_wind;
	r->f_renderer = renderer;
	r->f_texture = texture;
	r->f_format = format;
}
#endif

#if EnableRecreateW
static void SetWState(WState *r)
{
#if MayFullScreen
	ViewHSize = r->f_ViewHSize;
	ViewVSize = r->f_ViewVSize;
	ViewHStart = r->f_ViewHStart;
	ViewVStart = r->f_ViewVStart;
	hOffset = r->f_hOffset;
	vOffset = r->f_vOffset;
#endif
#if 1
	UseFullScreen = r->f_UseFullScreen;
#endif
#if 1
	UseMagnify = r->f_UseMagnify;
#endif
#if MayNotFullScreen
	CurWinIndx = r->f_CurWinIndx;
#endif
	main_wind = r->f_main_wind;
	renderer = r->f_renderer;
	texture = r->f_texture;
	format = r->f_format;
}
#endif

#if 1 && 1
enum {
	kWinStateWindowed,
#if 1
	kWinStateFullScreen,
#endif
	kNumWinStates
};
#endif

#if 1 && 1
static int WinMagStates[kNumWinStates];
#endif

#if EnableRecreateW
static bool ReCreateMainWindow(void)
{
	WState old_state;
	WState new_state;
#if HaveWorkingWarp
	bool HadCursorHidden = HaveCursorHidden;
#endif
#if 1 && 1
	int OldWinState =
		UseFullScreen ? kWinStateFullScreen : kWinStateWindowed;
	int OldMagState =
		UseMagnify ? kMagStateMagnifgy : kMagStateNormal;

	WinMagStates[OldWinState] =
		OldMagState;
#endif

#if 1
	if (! UseFullScreen)
#endif
#if MayNotFullScreen
	{
		SDL_GetWindowPosition(main_wind,
			&WinPositionsX[CurWinIndx],
			&WinPositionsY[CurWinIndx]);
		HavePositionWins[CurWinIndx] = true;
	}
#endif

	ForceShowCursor(); /* hide/show cursor api is per window */

#if MayFullScreen
	if (GrabMachine) {
		GrabMachine = false;
		UngrabMachine();
	}
#endif

	GetWState(&old_state);

	ZapWState();

#if 1
	UseMagnify = WantMagnify;
#endif
#if 1
	UseFullScreen = WantFullScreen;
#endif

	if (! CreateMainWindow()) {
		CloseMainWindow();
		SetWState(&old_state);

		/* avoid retry */
#if 1
		WantFullScreen = UseFullScreen;
#endif
#if 1
		WantMagnify = UseMagnify;
#endif

	} else {
		GetWState(&new_state);
		SetWState(&old_state);
		CloseMainWindow();
		SetWState(&new_state);

#if HaveWorkingWarp
		if (HadCursorHidden) {
			(void) MoveMouse(CurMouseH, CurMouseV);
		}
#endif
	}

	return true;
}
#endif

static void ZapWinStateVars(void)
{
#if MayNotFullScreen
	{
		int i;

		for (i = 0; i < kNumMagStates; ++i) {
			HavePositionWins[i] = false;
		}
	}
#endif
#if 1 && 1
	{
		int i;

		for (i = 0; i < kNumWinStates; ++i) {
			WinMagStates[i] = kMagStateAuto;
		}
	}
#endif
}

#if 1
void ToggleWantFullScreen(void)
{
	WantFullScreen = ! WantFullScreen;

#if 1
	{
		int OldWinState =
			UseFullScreen ? kWinStateFullScreen : kWinStateWindowed;
		int OldMagState =
			UseMagnify ? kMagStateMagnifgy : kMagStateNormal;
		int NewWinState =
			WantFullScreen ? kWinStateFullScreen : kWinStateWindowed;
		int NewMagState = WinMagStates[NewWinState];

		WinMagStates[OldWinState] = OldMagState;
		if (kMagStateAuto != NewMagState) {
			WantMagnify = (kMagStateMagnifgy == NewMagState);
		} else {
			WantMagnify = false;
			if (WantFullScreen) {
				SDL_Rect r;

				if (0 == SDL_GetDisplayBounds(0, &r)) {
					if ((r.w >= vMacScreenWidth * WindowScale)
						&& (r.h >= vMacScreenHeight * WindowScale)
						)
					{
						WantMagnify = true;
					}
				}
			}
		}
	}
#endif
}
#endif

/* --- SavedTasks --- */

static void LeaveBackground(void)
{
	ReconnectKeyCodes3();
	DisableKeyRepeat();
}

static void EnterBackground(void)
{
	RestoreKeyRepeat();
	DisconnectKeyCodes3();

	ForceShowCursor();
}

void LeaveSpeedStopped(void)
{
#if SoundEnabled
	Sound_Start();
#endif

	StartUpTimeAdjust();
}

void EnterSpeedStopped(void)
{
#if SoundEnabled
	Sound_Stop();
#endif
}

static void CheckForSavedTasks(void)
{
	if (EvtQNeedRecover) {
		EvtQNeedRecover = false;

		/* attempt cleanup, EvtQNeedRecover may get set again */
		EvtQTryRecoverFromFull();
	}

#if EnableFSMouseMotion && HaveWorkingWarp
	if (HaveMouseMotion) {
		MouseConstrain();
	}
#endif

	if (RequestMacOff) {
		RequestMacOff = false;
		/*if (AnyDiskInserted()) {
			MacMsgOverride(kStrQuitWarningTitle,
				kStrQuitWarningMessage);
		} else {*/
			ForceMacOff = true;
		//}
	}

	if (ForceMacOff) {
		return;
	}

	if (gTrueBackgroundFlag != gBackgroundFlag) {
		gBackgroundFlag = gTrueBackgroundFlag;
		if (gTrueBackgroundFlag) {
			EnterBackground();
		} else {
			LeaveBackground();
		}
	}

	// TODO: fix this
	/*if (CurSpeedStopped != (SpeedStopped ||
		(gBackgroundFlag && ! RunInBackground))){
		} else {
			LeaveSpeedStopped();
		}*/

#if EnableRecreateW
	if (0
#if 1
		|| (UseMagnify != WantMagnify)
#endif
#if 1
		|| (UseFullScreen != WantFullScreen)
#endif
		)
	{
		(void) ReCreateMainWindow();
	}
#endif

#if MayFullScreen
	if (GrabMachine != (
#if 1
		UseFullScreen &&
#endif
		! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		GrabMachine = ! GrabMachine;
		if (GrabMachine) {
			GrabTheMachine();
		} else {
			UngrabMachine();
		}
	}
#endif

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}

#if NeedRequestIthDisk
	if (0 != RequestIthDisk) {
		Sony_InsertIth(RequestIthDisk);
		RequestIthDisk = 0;
	}
#endif

	if (HaveCursorHidden != (WantCursorHidden
		&& ! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		(void) SDL_ShowCursor(
			HaveCursorHidden ? SDL_DISABLE : SDL_ENABLE);
	}
}

/* --- command line parsing --- */

// TODO: reimplement with an actual argument parsing library
static bool ScanCommandLine(void)
{
	return true;
}

/* --- main program flow --- */

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

static void WaitForTheNextEvent(void)
{
	SDL_Event event;

	if (SDL_WaitEvent(&event)) {
		HandleTheEvent(&event);
	}
}

static void CheckForSystemEvents(void)
{
	SDL_Event event;
	int i = 10;

	while ((--i >= 0) && SDL_PollEvent(&event)) {
		HandleTheEvent(&event);
	}
}

GLOBALOSGLUPROC WaitForNextTick(void)
{
label_retry:
	CheckForSystemEvents();
	CheckForSavedTasks();

	if (ForceMacOff) {
		return;
	}

	if (CurSpeedStopped) {
		DoneWithDrawingForTick();
		WaitForTheNextEvent();
		goto label_retry;
	}

	if (ExtraTimeNotOver()) {
		(void) SDL_Delay(NextIntTime - LastTime);
		goto label_retry;
	}

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	if ((! gBackgroundFlag)
#if UseMotionEvents
		&& (! CaughtMouse)
#endif
		)
	{
		CheckMouseState();
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif
}

/* --- platform independent code can be thought of as going here --- */

#include "PROGMAIN.h"

static void ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
}

static void ReserveAllocAll(void)
{
#if dbglog_HAVE
	dbglog_ReserveAlloc();
#endif
	ReserveAllocOneBlock(&ROM, kROM_Size, 5, false);

	ReserveAllocOneBlock(&screencomparebuff,
		vMacScreenNumBytes, 5, true);
#if UseControlKeys
	ReserveAllocOneBlock(&CntrlDisplayBuff,
		vMacScreenNumBytes, 5, false);
#endif

	ReserveAllocOneBlock(&CLUT_final, CLUT_finalsz, 5, false);
#if SoundEnabled
	ReserveAllocOneBlock((uint8_t * *)&TheSoundBuffer,
		dbhBufferSize, 5, false);
#endif

	EmulationReserveAlloc();
}

static bool AllocMemory(void)
{
	uimr n;
	bool IsOk = false;

	ReserveAllocOffset = 0;
	ReserveAllocBigBlock = nullpr;
	ReserveAllocAll();
	n = ReserveAllocOffset;
	ReserveAllocBigBlock = (uint8_t *)calloc(1, n);
	if (NULL == ReserveAllocBigBlock) {
		MacMsg(kStrOutOfMemTitle, kStrOutOfMemMessage, true);
	} else {
		ReserveAllocOffset = 0;
		ReserveAllocAll();
		if (n != ReserveAllocOffset) {
			/* oops, program error */
		} else {
			IsOk = true;
		}
	}

	return IsOk;
}

static void UnallocMemory(void)
{
	if (nullpr != ReserveAllocBigBlock) {
		free((char *)ReserveAllocBigBlock);
	}
}

#if CanGetAppPath
static bool InitWhereAmI(void)
{
	app_parent = SDL_GetBasePath();

	pref_dir = SDL_GetPrefPath("gryphel", "minivmac");

	return true; /* keep going regardless */
}
#endif

#if CanGetAppPath
static void UninitWhereAmI(void)
{
	SDL_free(pref_dir);

	SDL_free(app_parent);
}
#endif

static bool InitOSGLU(void)
{
	if (Config_TryInit())
	if (AllocMemory())
#if CanGetAppPath
	if (InitWhereAmI())
#endif
#if dbglog_HAVE
	if (dbglog_open())
#endif
	if (ScanCommandLine())
	if (LoadMacRom())
	if (LoadInitialImages())
	if (InitLocationDat())
#if SoundEnabled
	if (Sound_Init())
#endif
	if (SDL_InitDisplay())
	if (CreateMainWindow())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

static void UnInitOSGLU(void)
{
	RestoreKeyRepeat();
#if MayFullScreen
	UngrabMachine();
#endif
#if SoundEnabled
	Sound_Stop();
#endif
#if SoundEnabled
	Sound_UnInit();
#endif
#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	ForceShowCursor();

#if dbglog_HAVE
	dbglog_close();
#endif

#if CanGetAppPath
	UninitWhereAmI();
#endif
	UnallocMemory();

	CheckSavedMacMsg();

	CloseMainWindow();

	SDL_Quit();
}

int main(int argc, char **argv)
{
	argc = argc;
	argv = argv;

	ZapOSGLUVars();
	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

	return 0;
}
