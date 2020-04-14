/*
	OSGLUOSX.c

	Copyright (C) 2009 Philip Cummins, Richard F. Bannister,
		Paul C. Pratt

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
	Operating System GLUe for macintosh OS X

	All operating system dependent code for the
	Macintosh platform should go here.

	This code is descended from Richard F. Bannister's Macintosh
	port of vMac, by Philip Cummins.

	The main entry point 'main' is at the end of this file.
*/

#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"

#include "UI/MYOSGLUE.h"

/* --- adapting to API/ABI version differences --- */

/*
	if UsingCarbonLib, instead of native Macho,
	then some APIs are missing
*/
#ifndef UsingCarbonLib
#define UsingCarbonLib 0
#endif

LOCALVAR CFBundleRef AppServBunRef;

LOCALVAR bool DidApplicationServicesBun = false;

LOCALFUNC bool HaveApplicationServicesBun(void)
{
	if (! DidApplicationServicesBun) {
		AppServBunRef = CFBundleGetBundleWithIdentifier(
			CFSTR("com.apple.ApplicationServices"));
		DidApplicationServicesBun = true;
	}
	return (AppServBunRef != NULL);
}

#if MayFullScreen || UsingCarbonLib

LOCALVAR CFBundleRef HIToolboxBunRef;

LOCALVAR bool DidHIToolboxBunRef = false;

LOCALFUNC bool HaveHIToolboxBunRef(void)
{
	if (! DidHIToolboxBunRef) {
		HIToolboxBunRef = CFBundleGetBundleWithIdentifier(
			CFSTR("com.apple.HIToolbox"));
		DidHIToolboxBunRef = true;
	}
	return (HIToolboxBunRef != NULL);
}

#endif

LOCALVAR CFBundleRef AGLBunRef;

LOCALVAR bool DidAGLBunRef = false;

LOCALFUNC bool HaveAGLBunRef(void)
{
	if (! DidAGLBunRef) {
		AGLBunRef = CFBundleGetBundleWithIdentifier(
			CFSTR("com.apple.agl"));
		DidAGLBunRef = true;
	}
	return (AGLBunRef != NULL);
}


#if MayFullScreen

/* SetSystemUIModeProcPtr API always not available */

typedef UInt32                          SystemUIMode;
typedef OptionBits                      SystemUIOptions;

enum {
	kUIModeNormal                 = 0,
	kUIModeAllHidden              = 3
};

enum {
	kUIOptionAutoShowMenuBar      = 1 << 0,
	kUIOptionDisableAppleMenu     = 1 << 2,
	kUIOptionDisableProcessSwitch = 1 << 3,
	kUIOptionDisableForceQuit     = 1 << 4,
	kUIOptionDisableSessionTerminate = 1 << 5,
	kUIOptionDisableHide          = 1 << 6
};

typedef OSStatus (*SetSystemUIModeProcPtr)
	(SystemUIMode inMode, SystemUIOptions inOptions);
LOCALVAR SetSystemUIModeProcPtr SetSystemUIMode = NULL;
LOCALVAR bool DidSetSystemUIMode = false;

LOCALFUNC bool HaveSetSystemUIMode(void)
{
	if (! DidSetSystemUIMode) {
		if (HaveHIToolboxBunRef()) {
			SetSystemUIMode =
				(SetSystemUIModeProcPtr)
				CFBundleGetFunctionPointerForName(
					HIToolboxBunRef, CFSTR("SetSystemUIMode"));
		}
		DidSetSystemUIMode = true;
	}
	return (SetSystemUIMode != NULL);
}

#endif

/* In 10.1 or later */

typedef OSStatus (*LSCopyDisplayNameForRefProcPtr)
	(const FSRef *inRef, CFStringRef *outDisplayName);
LOCALVAR LSCopyDisplayNameForRefProcPtr LSCopyDisplayNameForRef
	= NULL;
LOCALVAR bool DidLSCopyDisplayNameForRef = false;

LOCALFUNC bool HaveLSCopyDisplayNameForRef(void)
{
	if (! DidLSCopyDisplayNameForRef) {
		if (HaveApplicationServicesBun()) {
			LSCopyDisplayNameForRef =
				(LSCopyDisplayNameForRefProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("LSCopyDisplayNameForRef"));
		}
		DidLSCopyDisplayNameForRef = true;
	}
	return (LSCopyDisplayNameForRef != NULL);
}

/* In 10.5 or later */

typedef GLboolean (*aglSetWindowRefProcPtr)
	(AGLContext ctx, WindowRef window);
LOCALVAR aglSetWindowRefProcPtr aglSetWindowRef = NULL;
LOCALVAR bool DidaglSetWindowRef = false;

LOCALFUNC bool HaveaglSetWindowRef(void)
{
	if (! DidaglSetWindowRef) {
		if (HaveAGLBunRef()) {
			aglSetWindowRef =
				(aglSetWindowRefProcPtr)
				CFBundleGetFunctionPointerForName(
					AGLBunRef, CFSTR("aglSetWindowRef"));
		}
		DidaglSetWindowRef = true;
	}
	return (aglSetWindowRef != NULL);
}

/* Deprecated as of 10.5 */

typedef CGrafPtr _AGLDrawable;
typedef GLboolean (*aglSetDrawableProcPtr)
	(AGLContext ctx, _AGLDrawable draw);
LOCALVAR aglSetDrawableProcPtr aglSetDrawable = NULL;
LOCALVAR bool DidaglSetDrawable = false;

LOCALFUNC bool HaveaglSetDrawable(void)
{
	if (! DidaglSetDrawable) {
		if (HaveAGLBunRef()) {
			aglSetDrawable =
				(aglSetDrawableProcPtr)
				CFBundleGetFunctionPointerForName(
					AGLBunRef, CFSTR("aglSetDrawable"));
		}
		DidaglSetDrawable = true;
	}
	return (aglSetDrawable != NULL);
}

/* routines not in carbon lib */


#if UsingCarbonLib

typedef CGDisplayErr
(*CGGetActiveDisplayListProcPtr) (
	CGDisplayCount       maxDisplays,
	CGDirectDisplayID *  activeDspys,
	CGDisplayCount *     dspyCnt);
LOCALVAR CGGetActiveDisplayListProcPtr CGGetActiveDisplayList = NULL;
LOCALVAR bool DidCGGetActiveDisplayList = false;

LOCALFUNC bool HaveCGGetActiveDisplayList(void)
{
	if (! DidCGGetActiveDisplayList) {
		if (HaveApplicationServicesBun()) {
			CGGetActiveDisplayList =
				(CGGetActiveDisplayListProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGGetActiveDisplayList"));
		}
		DidCGGetActiveDisplayList = true;
	}
	return (CGGetActiveDisplayList != NULL);
}

#else

#define HaveCGGetActiveDisplayList() true
#define CGGetActiveDisplayList CGGetActiveDisplayList

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef CGRect
(*CGDisplayBoundsProcPtr) (CGDirectDisplayID display);
LOCALVAR CGDisplayBoundsProcPtr CGDisplayBounds = NULL;
LOCALVAR bool DidCGDisplayBounds = false;

LOCALFUNC bool HaveCGDisplayBounds(void)
{
	if (! DidCGDisplayBounds) {
		if (HaveApplicationServicesBun()) {
			CGDisplayBounds =
				(CGDisplayBoundsProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayBounds"));
		}
		DidCGDisplayBounds = true;
	}
	return (CGDisplayBounds != NULL);
}

#else

#define HaveCGDisplayBounds() true
#define CGDisplayBounds CGDisplayBounds

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef size_t
(*CGDisplayPixelsWideProcPtr) (CGDirectDisplayID display);
LOCALVAR CGDisplayPixelsWideProcPtr CGDisplayPixelsWide = NULL;
LOCALVAR bool DidCGDisplayPixelsWide = false;

LOCALFUNC bool HaveCGDisplayPixelsWide(void)
{
	if (! DidCGDisplayPixelsWide) {
		if (HaveApplicationServicesBun()) {
			CGDisplayPixelsWide =
				(CGDisplayPixelsWideProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayPixelsWide"));
		}
		DidCGDisplayPixelsWide = true;
	}
	return (CGDisplayPixelsWide != NULL);
}

#else

#define HaveCGDisplayPixelsWide() true
#define CGDisplayPixelsWide CGDisplayPixelsWide

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef size_t
(*CGDisplayPixelsHighProcPtr) (CGDirectDisplayID display);
LOCALVAR CGDisplayPixelsHighProcPtr CGDisplayPixelsHigh = NULL;
LOCALVAR bool DidCGDisplayPixelsHigh = false;

LOCALFUNC bool HaveCGDisplayPixelsHigh(void)
{
	if (! DidCGDisplayPixelsHigh) {
		if (HaveApplicationServicesBun()) {
			CGDisplayPixelsHigh =
				(CGDisplayPixelsHighProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayPixelsHigh"));
		}
		DidCGDisplayPixelsHigh = true;
	}
	return (CGDisplayPixelsHigh != NULL);
}

#else

#define HaveCGDisplayPixelsHigh() true
#define CGDisplayPixelsHigh CGDisplayPixelsHigh

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef CGDisplayErr
(*CGDisplayHideCursorProcPtr) (CGDirectDisplayID display);
LOCALVAR CGDisplayHideCursorProcPtr CGDisplayHideCursor = NULL;
LOCALVAR bool DidCGDisplayHideCursor = false;

LOCALFUNC bool HaveCGDisplayHideCursor(void)
{
	if (! DidCGDisplayHideCursor) {
		if (HaveApplicationServicesBun()) {
			CGDisplayHideCursor =
				(CGDisplayHideCursorProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayHideCursor"));
		}
		DidCGDisplayHideCursor = true;
	}
	return (CGDisplayHideCursor != NULL);
}

#else

#define HaveCGDisplayHideCursor() true
#define CGDisplayHideCursor CGDisplayHideCursor

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef CGDisplayErr
(*CGDisplayShowCursorProcPtr) (CGDirectDisplayID display);
LOCALVAR CGDisplayShowCursorProcPtr CGDisplayShowCursor = NULL;
LOCALVAR bool DidCGDisplayShowCursor = false;

LOCALFUNC bool HaveCGDisplayShowCursor(void)
{
	if (! DidCGDisplayShowCursor) {
		if (HaveApplicationServicesBun()) {
			CGDisplayShowCursor =
				(CGDisplayShowCursorProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayShowCursor"));
		}
		DidCGDisplayShowCursor = true;
	}
	return (CGDisplayShowCursor != NULL);
}

#else

#define HaveCGDisplayShowCursor() true
#define CGDisplayShowCursor CGDisplayShowCursor

#endif /* ! UsingCarbonLib */


#if 0

typedef CGDisplayErr (*CGDisplayMoveCursorToPointProcPtr)
	(CGDirectDisplayID display, CGPoint point);
LOCALVAR CGDisplayMoveCursorToPointProcPtr CGDisplayMoveCursorToPoint
	= NULL;
LOCALVAR bool DidCGDisplayMoveCursorToPoint = false;

LOCALFUNC bool HaveCGDisplayMoveCursorToPoint(void)
{
	if (! DidCGDisplayMoveCursorToPoint) {
		if (HaveApplicationServicesBun()) {
			CGDisplayMoveCursorToPoint =
				(CGDisplayMoveCursorToPointProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGDisplayMoveCursorToPoint"));
		}
		DidCGDisplayMoveCursorToPoint = true;
	}
	return (CGDisplayMoveCursorToPoint != NULL);
}

#endif /* 0 */


#if UsingCarbonLib

typedef CGEventErr
(*CGWarpMouseCursorPositionProcPtr) (CGPoint newCursorPosition);
LOCALVAR CGWarpMouseCursorPositionProcPtr CGWarpMouseCursorPosition
	= NULL;
LOCALVAR bool DidCGWarpMouseCursorPosition = false;

LOCALFUNC bool HaveCGWarpMouseCursorPosition(void)
{
	if (! DidCGWarpMouseCursorPosition) {
		if (HaveApplicationServicesBun()) {
			CGWarpMouseCursorPosition =
				(CGWarpMouseCursorPositionProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGWarpMouseCursorPosition"));
		}
		DidCGWarpMouseCursorPosition = true;
	}
	return (CGWarpMouseCursorPosition != NULL);
}

#else

#define HaveCGWarpMouseCursorPosition() true
#define CGWarpMouseCursorPosition CGWarpMouseCursorPosition

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef CGEventErr
(*CGSetLocalEventsSuppressionIntervalProcPtr) (CFTimeInterval seconds);
LOCALVAR CGSetLocalEventsSuppressionIntervalProcPtr
	CGSetLocalEventsSuppressionInterval = NULL;
LOCALVAR bool DidCGSetLocalEventsSuppressionInterval = false;

LOCALFUNC bool HaveCGSetLocalEventsSuppressionInterval(void)
{
	if (! DidCGSetLocalEventsSuppressionInterval) {
		if (HaveApplicationServicesBun()) {
			CGSetLocalEventsSuppressionInterval =
				(CGSetLocalEventsSuppressionIntervalProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef,
					CFSTR("CGSetLocalEventsSuppressionInterval"));
		}
		DidCGSetLocalEventsSuppressionInterval = true;
	}
	return (CGSetLocalEventsSuppressionInterval != NULL);
}

#else

#define HaveCGSetLocalEventsSuppressionInterval() true
#define CGSetLocalEventsSuppressionInterval \
	CGSetLocalEventsSuppressionInterval

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef OSStatus (*CreateStandardAlertProcPtr) (
	AlertType alertType,
	CFStringRef error,
	CFStringRef explanation,
	const AlertStdCFStringAlertParamRec * param,
	DialogRef * outAlert
);
LOCALVAR CreateStandardAlertProcPtr CreateStandardAlert = NULL;
LOCALVAR bool DidCreateStandardAlert = false;

LOCALFUNC bool HaveCreateStandardAlert(void)
{
	if (! DidCreateStandardAlert) {
		if (HaveHIToolboxBunRef()) {
			CreateStandardAlert =
				(CreateStandardAlertProcPtr)
				CFBundleGetFunctionPointerForName(
					HIToolboxBunRef, CFSTR("CreateStandardAlert"));
		}
		DidCreateStandardAlert = true;
	}
	return (CreateStandardAlert != NULL);
}

#else

#define HaveCreateStandardAlert() true
#define CreateStandardAlert CreateStandardAlert

#endif /* ! UsingCarbonLib */


#if UsingCarbonLib

typedef OSStatus (*RunStandardAlertProcPtr) (
	DialogRef inAlert,
	ModalFilterUPP filterProc,
	DialogItemIndex * outItemHit
);
LOCALVAR RunStandardAlertProcPtr RunStandardAlert = NULL;
LOCALVAR bool DidRunStandardAlert = false;

LOCALFUNC bool HaveRunStandardAlert(void)
{
	if (! DidRunStandardAlert) {
		if (HaveHIToolboxBunRef()) {
			RunStandardAlert =
				(RunStandardAlertProcPtr)
				CFBundleGetFunctionPointerForName(
					HIToolboxBunRef, CFSTR("RunStandardAlert"));
		}
		DidRunStandardAlert = true;
	}
	return (RunStandardAlert != NULL);
}

#else

#define HaveRunStandardAlert() true
#define RunStandardAlert RunStandardAlert

#endif /* ! UsingCarbonLib */


typedef CGDirectDisplayID (*CGMainDisplayIDProcPtr)(void);

LOCALVAR CGMainDisplayIDProcPtr CGMainDisplayID = NULL;
LOCALVAR bool DidCGMainDisplayID = false;

LOCALFUNC bool HaveCGMainDisplayID(void)
{
	if (! DidCGMainDisplayID) {
		if (HaveApplicationServicesBun()) {
			CGMainDisplayID =
				(CGMainDisplayIDProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGMainDisplayID"));
		}
		DidCGMainDisplayID = true;
	}
	return (CGMainDisplayID != NULL);
}

#ifndef kCGNullDirectDisplay /* not in MPW Headers */
#define kCGNullDirectDisplay ((CGDirectDisplayID)0)
#endif

typedef CGError
(*CGDisplayRegisterReconfigurationCallbackProcPtr) (
	CGDisplayReconfigurationCallBack proc,
	void *userInfo
	);
LOCALVAR CGDisplayRegisterReconfigurationCallbackProcPtr
	CGDisplayRegisterReconfigurationCallback = NULL;
LOCALVAR bool DidCGDisplayRegisterReconfigurationCallback = false;

LOCALFUNC bool HaveCGDisplayRegisterReconfigurationCallback(void)
{
	if (! DidCGDisplayRegisterReconfigurationCallback) {
		if (HaveApplicationServicesBun()) {
			CGDisplayRegisterReconfigurationCallback =
				(CGDisplayRegisterReconfigurationCallbackProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef,
					CFSTR("CGDisplayRegisterReconfigurationCallback"));
		}
		DidCGDisplayRegisterReconfigurationCallback = true;
	}
	return (CGDisplayRegisterReconfigurationCallback != NULL);
}


typedef CGError
(*CGDisplayRemoveReconfigurationCallbackProcPtr) (
	CGDisplayReconfigurationCallBack proc,
	void *userInfo
	);
LOCALVAR CGDisplayRemoveReconfigurationCallbackProcPtr
	CGDisplayRemoveReconfigurationCallback = NULL;
LOCALVAR bool DidCGDisplayRemoveReconfigurationCallback = false;

LOCALFUNC bool HaveCGDisplayRemoveReconfigurationCallback(void)
{
	if (! DidCGDisplayRemoveReconfigurationCallback) {
		if (HaveApplicationServicesBun()) {
			CGDisplayRemoveReconfigurationCallback =
				(CGDisplayRemoveReconfigurationCallbackProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef,
					CFSTR("CGDisplayRemoveReconfigurationCallback"));
		}
		DidCGDisplayRemoveReconfigurationCallback = true;
	}
	return (CGDisplayRemoveReconfigurationCallback != NULL);
}


typedef boolean_t (*CGCursorIsVisibleProcPtr)(void);

LOCALVAR CGCursorIsVisibleProcPtr CGCursorIsVisible = NULL;
LOCALVAR bool DidCGCursorIsVisible = false;

LOCALFUNC bool HaveCGCursorIsVisible(void)
{
	if (! DidCGCursorIsVisible) {
		if (HaveApplicationServicesBun()) {
			CGCursorIsVisible =
				(CGCursorIsVisibleProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef, CFSTR("CGCursorIsVisible"));
		}
		DidCGCursorIsVisible = true;
	}
	return (CGCursorIsVisible != NULL);
}


/* --- end of adapting to API/ABI version differences --- */

/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

LOCALPROC PStrFromChar(ps3p r, char x)
{
	r[0] = 1;
	r[1] = (char)x;
}

/* --- mac style errors --- */

#define CheckSavetMacErr(result) (mnvm_noErr == (err = (result)))
	/*
		where 'err' is a variable of type tMacErr in the function
		this is used in
	*/

#define To_tMacErr(result) ((tMacErr)(uint16_t)(result))

#define CheckSaveMacErr(result) (CheckSavetMacErr(To_tMacErr(result)))


#include "STRCONST.h"

#define NeedCell2UnicodeMap 1

#include "LANG/INTLCHAR.h"

LOCALPROC UniCharStrFromSubstCStr(int *L, UniChar *x, char *s)
{
	int i;
	int L0;
	uint8_t ps[ClStrMaxLength];

	ClStrFromSubstCStr(&L0, ps, s);

	for (i = 0; i < L0; ++i) {
		x[i] = Cell2UnicodeMap[ps[i]];
	}

	*L = L0;
}

#define NotAfileRef (-1)

LOCALFUNC tMacErr MakeFSRefUniChar(FSRef *ParentRef,
	UniCharCount fileNameLength, const UniChar *fileName,
	bool *isFolder, FSRef *ChildRef)
{
	tMacErr err;
	Boolean isFolder0;
	Boolean isAlias;

	if (CheckSaveMacErr(FSMakeFSRefUnicode(ParentRef,
		fileNameLength, fileName, kTextEncodingUnknown,
		ChildRef)))
	if (CheckSaveMacErr(FSResolveAliasFile(ChildRef,
		TRUE, &isFolder0, &isAlias)))
	{
		*isFolder = isFolder0;
	}

	return err;
}

LOCALFUNC tMacErr MakeFSRefC(FSRef *ParentRef, char *fileName,
	bool *isFolder, FSRef *ChildRef)
{
	int L;
	UniChar x[ClStrMaxLength];

	UniCharStrFromSubstCStr(&L, x, fileName);
	return MakeFSRefUniChar(ParentRef, L, x,
		isFolder, ChildRef);
}

#if UseActvFile
LOCALFUNC tMacErr OpenNamedFileInFolderRef(FSRef *ParentRef,
	char *fileName, short *refnum)
{
	tMacErr err;
	bool isFolder;
	FSRef ChildRef;
	HFSUniStr255 forkName;

	if (CheckSavetMacErr(MakeFSRefC(ParentRef, fileName,
		&isFolder, &ChildRef)))
	if (CheckSaveMacErr(FSGetDataForkName(&forkName)))
	if (CheckSaveMacErr(FSOpenFork(&ChildRef, forkName.length,
		forkName.unicode, fsRdPerm, refnum)))
	{
		/* ok */
	}

	return err;
}
#endif

#if dbglog_HAVE || UseActvFile
LOCALFUNC tMacErr OpenWriteNamedFileInFolderRef(FSRef *ParentRef,
	char *fileName, short *refnum)
{
	tMacErr err;
	bool isFolder;
	FSRef ChildRef;
	HFSUniStr255 forkName;
	int L;
	UniChar x[ClStrMaxLength];

	UniCharStrFromSubstCStr(&L, x, fileName);
	err = MakeFSRefUniChar(ParentRef, L, x, &isFolder, &ChildRef);
	if (mnvm_fnfErr == err) {
		err = To_tMacErr(FSCreateFileUnicode(ParentRef, L, x, 0, NULL,
			&ChildRef, NULL));
	}
	if (mnvm_noErr == err) {
		if (CheckSaveMacErr(FSGetDataForkName(&forkName)))
		if (CheckSaveMacErr(FSOpenFork(&ChildRef, forkName.length,
			forkName.unicode, fsRdWrPerm, refnum)))
		{
			/* ok */
		}
	}

	return err;
}
#endif

LOCALFUNC tMacErr FindNamedChildRef(FSRef *ParentRef,
	char *ChildName, FSRef *ChildRef)
{
	tMacErr err;
	bool isFolder;

	if (CheckSavetMacErr(MakeFSRefC(ParentRef, ChildName,
		&isFolder, ChildRef)))
	{
		if (! isFolder) {
			err = mnvm_miscErr;
		}
	}

	return err;
}

#if UseActvFile || (IncludeSonyNew && ! SaveDialogEnable)
LOCALFUNC tMacErr FindOrMakeNamedChildRef(FSRef *ParentRef,
	char *ChildName, FSRef *ChildRef)
{
	tMacErr err;
	bool isFolder;
	int L;
	UniChar x[ClStrMaxLength];

	UniCharStrFromSubstCStr(&L, x, ChildName);
	if (CheckSavetMacErr(MakeFSRefUniChar(ParentRef, L, x,
		&isFolder, ChildRef)))
	{
		if (! isFolder) {
			err = mnvm_miscErr;
		}
	}
	if (mnvm_fnfErr == err) {
		err = To_tMacErr(FSCreateDirectoryUnicode(
			ParentRef, L, x, kFSCatInfoNone, NULL,
			ChildRef, NULL, NULL));
	}

	return err;
}
#endif

LOCALVAR FSRef DatDirRef;

LOCALVAR CFStringRef AppName = NULL;

LOCALPROC UnInitApplInfo(void)
{
	if (AppName != NULL) {
		CFRelease(AppName);
	}
}

LOCALFUNC bool InitApplInfo(void)
{
	ProcessSerialNumber currentProcess = {0, kCurrentProcess};
	FSRef fsRef;
	FSRef parentRef;

	if (noErr == GetProcessBundleLocation(&currentProcess,
		&fsRef))
	if (noErr == FSGetCatalogInfo(&fsRef, kFSCatInfoNone,
		NULL, NULL, NULL, &parentRef))
	{
		FSRef ContentsRef;
		FSRef DatRef;

		DatDirRef = parentRef;
		if (mnvm_noErr == FindNamedChildRef(&fsRef, "Contents",
			&ContentsRef))
		if (mnvm_noErr == FindNamedChildRef(&ContentsRef, "mnvm_dat",
			&DatRef))
		{
			DatDirRef = DatRef;
		}

		if (HaveLSCopyDisplayNameForRef()) {
			if (noErr == LSCopyDisplayNameForRef(&fsRef, &AppName))
			{
				return true;
			}
		}

		if (noErr == CopyProcessName(&currentProcess, &AppName)) {
			return true;
		}
	}
	return false;
}

/* --- sending debugging info to file --- */

#if dbglog_HAVE

LOCALVAR SInt16 dbglog_File = NotAfileRef;

LOCALFUNC bool dbglog_open0(void)
{
	tMacErr err;

	err = OpenWriteNamedFileInFolderRef(&DatDirRef,
		"dbglog.txt", &dbglog_File);

	return (mnvm_noErr == err);
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
	ByteCount actualCount;

	if (dbglog_File != NotAfileRef) {
		(void) FSWriteFork(
			dbglog_File,
			fsFromMark,
			0,
			L,
			s,
			&actualCount);
	}
}

LOCALPROC dbglog_close0(void)
{
	if (dbglog_File != NotAfileRef) {
		(void) FSSetForkSize(dbglog_File, fsFromMark, 0);
		(void) FSCloseFork(dbglog_File);
		dbglog_File = NotAfileRef;
	}
}

#endif

#define WantColorTransValid 1

#include "UI/COMOSGLU.h"

/* --- time, date --- */

/*
	be sure to avoid getting confused if TickCount
	overflows and wraps.
*/

#define dbglog_TimeStuff (0 && dbglog_HAVE)

LOCALVAR uint32_t TrueEmulatedTime = 0;

LOCALVAR EventTime NextTickChangeTime;

#define TickDuration (kEventDurationSecond / 60.14742)

LOCALPROC UpdateTrueEmulatedTime(void)
{
	EventTime LatestTime = GetCurrentEventTime();
	EventTime TimeDiff = LatestTime - NextTickChangeTime;

	if (TimeDiff >= 0.0) {
		if (TimeDiff > 16 * TickDuration) {
			/* emulation interrupted, forget it */
			++TrueEmulatedTime;
			NextTickChangeTime = LatestTime + TickDuration;

#if dbglog_TimeStuff
			dbglog_writelnNum("emulation interrupted",
				TrueEmulatedTime);
#endif
		} else {
			do {
				++TrueEmulatedTime;
				TimeDiff -= TickDuration;
				NextTickChangeTime += TickDuration;
			} while (TimeDiff >= 0.0);
		}
	}
}

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

/* LOCALVAR EventTime ETimeBase; */
LOCALVAR CFAbsoluteTime ATimeBase;
LOCALVAR uint32_t TimeSecBase;

LOCALFUNC bool CheckDateTime(void)
{
	uint32_t NewMacDateInSecond = TimeSecBase
		+ (uint32_t)(CFAbsoluteTimeGetCurrent() - ATimeBase);
	/*
		uint32_t NewMacDateInSecond = TimeSecBase
			+ (uint32_t)(GetCurrentEventTime() - ETimeBase);
	*/
	/*
		uint32_t NewMacDateInSecond = ((uint32_t)(CFAbsoluteTimeGetCurrent()))
			+ 3061137600UL;
	*/

	if (CurMacDateInSeconds != NewMacDateInSecond) {
		CurMacDateInSeconds = NewMacDateInSecond;
		return true;
	} else {
		return false;
	}
}

/* --- parameter buffers --- */

#include "UTILS/PBUFSTDC.h"

/* --- drives --- */

LOCALVAR SInt16 Drives[NumDrives]; /* open disk image files */

GLOBALOSGLUFUNC tMacErr vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	ByteCount actualCount;
	tMacErr result;

	if (IsWrite) {
		result = To_tMacErr(FSWriteFork(
			Drives[Drive_No],
			fsFromStart,
			Sony_Start,
			Sony_Count,
			Buffer,
			&actualCount));
	} else {
		result = To_tMacErr(FSReadFork(
			Drives[Drive_No],
			fsFromStart,
			Sony_Start,
			Sony_Count,
			Buffer,
			&actualCount));
	}

	if (nullpr != Sony_ActCount) {
		*Sony_ActCount = actualCount;
	}

	return result;
}

GLOBALOSGLUFUNC tMacErr vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	SInt64 forkSize;
	tMacErr err = To_tMacErr(
		FSGetForkSize(Drives[Drive_No], &forkSize));
	*Sony_Count = forkSize;
	return err;
}

GLOBALOSGLUFUNC tMacErr vSonyEject(tDrive Drive_No)
{
	SInt16 refnum = Drives[Drive_No];
	Drives[Drive_No] = NotAfileRef;

	DiskEjectedNotify(Drive_No);

	(void) FSCloseFork(refnum);

	return mnvm_noErr;
}

#if IncludeSonyNew
GLOBALOSGLUFUNC tMacErr vSonyEjectDelete(tDrive Drive_No)
{
	FSRef ref;
	tMacErr err0;
	tMacErr err;

	err0 = To_tMacErr(FSGetForkCBInfo(Drives[Drive_No], 0,
		NULL /* iterator */,
		NULL /* actualRefNum */,
		NULL /* forkInfo */,
		&ref /* ref */,
		NULL /* outForkName */));
	err = vSonyEject(Drive_No);

	if (mnvm_noErr != err0) {
		err = err0;
	} else {
		(void) FSDeleteObject(&ref);
	}

	return err;
}
#endif

#if IncludeSonyGetName
GLOBALOSGLUFUNC tMacErr vSonyGetName(tDrive Drive_No, tPbuf *r)
{
	FSRef ref;
	HFSUniStr255 outName;
	CFStringRef DiskName;
	tMacErr err;

	if (CheckSaveMacErr(FSGetForkCBInfo(Drives[Drive_No], 0,
		NULL /* iterator */,
		NULL /* actualRefNum */,
		NULL /* forkInfo */,
		&ref /* ref */,
		NULL /* outForkName */)))
	if (CheckSaveMacErr(FSGetCatalogInfo(&ref,
		kFSCatInfoNone /* whichInfo */,
		NULL /* catalogInfo */,
		&outName /* outName */,
		NULL /* fsSpec */,
		NULL /* parentRef */)))
	{
		DiskName = CFStringCreateWithCharacters(
			kCFAllocatorDefault, outName.unicode,
			outName.length);
		if (NULL != DiskName) {
			tPbuf i;

			if (CheckSavetMacErr(PbufNew(outName.length, &i))) {
				if (CFStringGetBytes(DiskName,
					CFRangeMake(0, outName.length),
					kCFStringEncodingMacRoman,
					'?', false,
					PbufDat[i],
					outName.length,
					NULL) != outName.length)
				{
					err = mnvm_miscErr;
				}
				if (mnvm_noErr != err) {
					PbufDispose(i);
				} else {
					*r = i;
				}
			}
			CFRelease(DiskName);
		}
	}

	return err;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEexport(tPbuf i)
{
	tMacErr err;
	ScrapRef scrapRef;

	if (CheckSaveMacErr(ClearCurrentScrap()))
	if (CheckSaveMacErr(GetCurrentScrap(&scrapRef)))
	if (CheckSaveMacErr(PutScrapFlavor(
		scrapRef,
		FOUR_CHAR_CODE('TEXT'),
		kScrapFlavorMaskNone,
		PbufSize[i],
		PbufDat[i])))
	{
		/* ok */
	}

	PbufDispose(i);

	return err;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEimport(tPbuf *r)
{
	tMacErr err;
	ScrapRef scrap;
	ScrapFlavorFlags flavorFlags;
	Size byteCount;
	tPbuf i;

	if (CheckSaveMacErr(GetCurrentScrap(&scrap)))
	if (CheckSaveMacErr(GetScrapFlavorFlags(scrap,
		'TEXT', &flavorFlags)))
	if (CheckSaveMacErr(GetScrapFlavorSize(scrap,
		'TEXT', &byteCount)))
	if (CheckSavetMacErr(PbufNew(byteCount, &i)))
	{
		Size byteCount2 = byteCount;
		if (CheckSaveMacErr(GetScrapFlavorData(scrap,
			'TEXT', &byteCount2,
			PbufDat[i])))
		{
			if (byteCount != byteCount2) {
				err = mnvm_miscErr;
			}
		}
		if (mnvm_noErr != err) {
			PbufDispose(i);
		} else {
			*r = i;
		}
	}

	return err;
}
#endif


#if EmLocalTalk

#include "UTIL/BPFILTER.h"

#endif


/* --- control mode and internationalization --- */

#define WantKeyboard_RemapMac 1

#include "UI/CONTROLM.h"


/* --- video out --- */

LOCALVAR WindowPtr gMainWindow = NULL;
LOCALVAR WindowPtr gOldWindow = NULL;

#if MayFullScreen
LOCALVAR short hOffset;
LOCALVAR short vOffset;
#endif

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if 1
LOCALVAR bool UseFullScreen = (WantInitFullScreen != 0);
#endif

LOCALVAR bool UseMagnify = (WantInitMagnify != 0);

LOCALPROC ScaleRect(Rect *r)
{
	r->left *= WindowScale;
	r->right *= WindowScale;
	r->top *= WindowScale;
	r->bottom *= WindowScale;
}

LOCALPROC SetScrnRectFromCoords(Rect *r,
	int16_t top, int16_t left, int16_t bottom, int16_t right)
{
	r->left = left;
	r->right = right;
	r->top = top;
	r->bottom = bottom;

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		OffsetRect(r, - ViewHStart, - ViewVStart);
	}
#endif

	if (UseMagnify) {
		ScaleRect(r);
	}

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		OffsetRect(r, hOffset, vOffset);
	}
#endif
}

LOCALVAR uint8_t * ScalingBuff = nullpr;

LOCALVAR uint8_t * CLUT_final;

#define CLUT_finalsz1 (256 * 8)

#define CLUT_finalClrSz (256 << (5 - vMacScreenDepth))

#define CLUT_finalsz ((CLUT_finalClrSz > CLUT_finalsz1) \
	? CLUT_finalClrSz : CLUT_finalsz1)

#define CLUT_finalsz CLUT_finalsz1


#define ScrnMapr_DoMap UpdateBWLuminanceCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"


#define ScrnMapr_DoMap UpdateMappedColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#define ScrnTrns_DoTrans UpdateTransColorCopy
#define ScrnTrns_Src GetCurDrawBuff()
#define ScrnTrns_Dst ScalingBuff
#define ScrnTrns_SrcDepth vMacScreenDepth
#define ScrnTrns_DstDepth 5
#define ScrnTrns_DstZLo 1

#include "HW/SCREEN/SCRNTRNS.h"

LOCALPROC UpdateLuminanceCopy(int16_t top, int16_t left,
	int16_t bottom, int16_t right)
{
	int i;

	if (vMacScreenDepth > 0 && UseColorMode) {

		if (vMacScreenDepth < 4) {
			if (! ColorTransValid) {
				int j;
				int k;
				uint32_t * p4;

				p4 = (uint32_t *)CLUT_final;
				for (i = 0; i < 256; ++i) {
					for (k = 1 << (3 - vMacScreenDepth); --k >= 0; ) {
						j = (i >> (k << vMacScreenDepth)) & (CLUT_size - 1);
						*p4++ = (((long)CLUT_reds[j] & 0xFF00) << 16)
							| (((long)CLUT_greens[j] & 0xFF00) << 8)
							| ((long)CLUT_blues[j] & 0xFF00);
					}
				}
				ColorTransValid = true;
			}
			UpdateMappedColorCopy(top, left, bottom, right);
		} else {
			UpdateTransColorCopy(top, left, bottom, right);
		}
	} else {
		if (! ColorTransValid) {
			int k;
			uint8_t * p4 = (uint8_t *)CLUT_final;

			for (i = 0; i < 256; ++i) {
				for (k = 8; --k >= 0; ) {
					*p4++ = ((i >> k) & 0x01) - 1;
				}
			}
			ColorTransValid = true;
		}

		UpdateBWLuminanceCopy(top, left, bottom, right);
	}
}

LOCALVAR AGLContext ctx = NULL;
LOCALVAR short GLhOffset;
LOCALVAR short GLvOffset;

#ifndef UseAGLdoublebuff
#define UseAGLdoublebuff 0
#endif

LOCALPROC DrawWithOpenGL(uint16_t top, uint16_t left, uint16_t bottom, uint16_t right)
{
	if (NULL == ctx) {
		/* oops */
	} else if (GL_TRUE != aglSetCurrentContext(ctx)) {
		/* err = aglReportError() */
	} else {
		int16_t top2;
		int16_t left2;

#if UseAGLdoublebuff
		/* redraw all */
		top = 0;
		left = 0;
		bottom = vMacScreenHeight;
		right = vMacScreenWidth;
#endif

#if 1
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			if (top < ViewVStart) {
				top = ViewVStart;
			}
			if (left < ViewHStart) {
				left = ViewHStart;
			}
			if (bottom > ViewVStart + ViewVSize) {
				bottom = ViewVStart + ViewVSize;
			}
			if (right > ViewHStart + ViewHSize) {
				right = ViewHStart + ViewHSize;
			}

			if ((top >= bottom) || (left >= right)) {
				goto label_exit;
			}
		}
#endif

		top2 = top;
		left2 = left;

#if 1
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			left2 -= ViewHStart;
			top2 -= ViewVStart;
		}
#endif

		if (UseMagnify) {
			top2 *= WindowScale;
			left2 *= WindowScale;
		}

#if 0
		glClear(GL_COLOR_BUFFER_BIT);
		glBitmap(vMacScreenWidth,
			vMacScreenHeight,
			0,
			0,
			0,
			0,
			(const GLubyte *)GetCurDrawBuff());
#endif
#if 1
		UpdateLuminanceCopy(top, left, bottom, right);
		glRasterPos2i(GLhOffset + left2, GLvOffset - top2);
#if 0 != vMacScreenDepth
		if (UseColorMode) {
			glDrawPixels(right - left,
				bottom - top,
				GL_RGBA,
				GL_UNSIGNED_INT_8_8_8_8,
				ScalingBuff + (left + top * vMacScreenWidth) * 4
				);
		} else
#endif
		{
			glDrawPixels(right - left,
				bottom - top,
				GL_LUMINANCE,
				GL_UNSIGNED_BYTE,
				ScalingBuff + (left + top * vMacScreenWidth)
				);
		}
#endif

#if 0 /* a very quick and dirty check of where drawing */
		glDrawPixels(right - left,
			1,
			GL_RED,
			GL_UNSIGNED_BYTE,
			ScalingBuff + (left + top * vMacScreenWidth)
			);

		glDrawPixels(1,
			bottom - top,
			GL_RED,
			GL_UNSIGNED_BYTE,
			ScalingBuff + (left + top * vMacScreenWidth)
			);
#endif

#if UseAGLdoublebuff
		aglSwapBuffers(ctx);
#else
		glFlush();
#endif
	}

#if MayFullScreen
label_exit:
	;
#endif
}

LOCALPROC Update_Screen(void)
{
	DrawWithOpenGL(0, 0, vMacScreenHeight, vMacScreenWidth);
}

LOCALPROC DrawChangesAndClear(void)
{
	if (ScreenChangedBottom > ScreenChangedTop) {
		DrawWithOpenGL(ScreenChangedTop, ScreenChangedLeft,
			ScreenChangedBottom, ScreenChangedRight);
		ScreenClearChanges();
	}
}


LOCALVAR bool MouseIsOutside = false;
	/*
		MouseIsOutside true if sure mouse outside our window. If in
		our window, or not sure, set false.
	*/

LOCALVAR bool WantCursorHidden = false;

LOCALPROC MousePositionNotify(Point NewMousePos)
{
	/*
		Not MouseIsOutside includes in the title bar, etc, so have
		to check if in content area.
	*/

	Rect r;
	bool ShouldHaveCursorHidden = ! MouseIsOutside;

	GetWindowBounds(gMainWindow, kWindowContentRgn, &r);

	NewMousePos.h -= r.left;
	NewMousePos.v -= r.top;

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePos.h -= hOffset;
		NewMousePos.v -= vOffset;
	}
#endif

	if (UseMagnify) {
		NewMousePos.h /= WindowScale;
		NewMousePos.v /= WindowScale;
	}

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePos.h += ViewHStart;
		NewMousePos.v += ViewVStart;
	}
#endif

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MousePositionSetDelta(NewMousePos.h - SavedMouseH,
			NewMousePos.v - SavedMouseV);
		SavedMouseH = NewMousePos.h;
		SavedMouseV = NewMousePos.v;
	} else
#endif
	{
		if (NewMousePos.h < 0) {
			NewMousePos.h = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePos.h >= vMacScreenWidth) {
			NewMousePos.h = vMacScreenWidth - 1;
			ShouldHaveCursorHidden = false;
		}
		if (NewMousePos.v < 0) {
			NewMousePos.v = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePos.v >= vMacScreenHeight) {
			NewMousePos.v = vMacScreenHeight - 1;
			ShouldHaveCursorHidden = false;
		}

		/* if (ShouldHaveCursorHidden || CurMouseButton) */
		/*
			for a game like arkanoid, would like mouse to still
			move even when outside window in one direction
		*/
		MousePositionSet(NewMousePos.h, NewMousePos.v);
	}

	WantCursorHidden = ShouldHaveCursorHidden;
}

LOCALPROC CheckMouseState(void)
{
	Point NewMousePos;
	GetGlobalMouse(&NewMousePos);
		/*
			Deprecated, but haven't found usable replacement.
			Between window deactivate and then reactivate,
			mouse can move without getting kEventMouseMoved.
			Also no way to get initial position.
			(Also don't get kEventMouseMoved after
			using menu bar. Or while using menubar, but
			that isn't too important.)
		*/
	MousePositionNotify(NewMousePos);
}

LOCALVAR bool gLackFocusFlag = false;
LOCALVAR bool gWeAreActive = false;

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		AutoScrollScreen();
	}
#endif
	DrawChangesAndClear();
}

LOCALVAR bool CurSpeedStopped = true;

/* --- keyboard --- */

LOCALVAR UInt32 SavedModifiers = 0;

LOCALPROC UpdateKeyboardModifiers(UInt32 theModifiers)
{
	UInt32 ChangedModifiers = theModifiers ^ SavedModifiers;

	if (0 != ChangedModifiers) {
		if (0 != (ChangedModifiers & shiftKey)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Shift,
				(shiftKey & theModifiers) != 0);
		}
		if (0 != (ChangedModifiers & cmdKey)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Command,
				(cmdKey & theModifiers) != 0);
		}
		if (0 != (ChangedModifiers & alphaLock)) {
			Keyboard_UpdateKeyMap2(MKC_formac_CapsLock,
				(alphaLock & theModifiers) != 0);
		}
		if (0 != (ChangedModifiers & optionKey)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Option,
				(optionKey & theModifiers) != 0);
		}
		if (0 != (ChangedModifiers & controlKey)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Control,
				(controlKey & theModifiers) != 0);
		}

		SavedModifiers = theModifiers;
	}
}

LOCALPROC ReconnectKeyCodes3(void)
{
	/*
		turn off any modifiers (other than alpha)
		that were turned on by drag and drop,
		unless still being held down.
	*/

	UInt32 theModifiers = GetCurrentKeyModifiers();

	UpdateKeyboardModifiers(theModifiers
		& (SavedModifiers | alphaLock));

	SavedModifiers = theModifiers;
}

/* --- display utilities --- */

/* DoForEachDisplay adapted from Apple Technical Q&A QA1168 */

typedef void
(*ForEachDisplayProcPtr) (CGDirectDisplayID display);

LOCALPROC DoForEachDisplay0(CGDisplayCount dspCount,
	CGDirectDisplayID *displays, ForEachDisplayProcPtr p)
{
	CGDisplayCount i;

	if (noErr == CGGetActiveDisplayList(dspCount,
		displays, &dspCount))
	{
		for (i = 0; i < dspCount; ++i) {
			p(displays[i]);
		}
	}
}

LOCALPROC DoForEachDisplay(ForEachDisplayProcPtr p)
{
	CGDisplayCount dspCount = 0;

	if (HaveCGGetActiveDisplayList()
		&& (noErr == CGGetActiveDisplayList(0, NULL, &dspCount)))
	{
		if (dspCount <= 2) {
			CGDirectDisplayID displays[2];
			DoForEachDisplay0(dspCount, displays, p);
		} else {
			CGDirectDisplayID *displays =
				calloc((size_t)dspCount, sizeof(CGDirectDisplayID));
			if (NULL != displays) {
				DoForEachDisplay0(dspCount, displays, p);
				free(displays);
			}
		}
	}
}

LOCALVAR void *datp;

LOCALPROC MainDisplayIDProc(CGDirectDisplayID display)
{
	CGDirectDisplayID *p = (CGDirectDisplayID *)datp;

	if (kCGNullDirectDisplay == *p) {
		*p = display;
	}
}

LOCALFUNC CGDirectDisplayID MainDisplayID(void)
{
	if (HaveCGMainDisplayID()) {
		return CGMainDisplayID();
	} else {
		/* for before OS X 10.2 */
		CGDirectDisplayID r = kCGNullDirectDisplay;
		void *savedatp = datp;
		datp = (void *)&r;
		DoForEachDisplay(MainDisplayIDProc);
		datp = savedatp;
		return r;
	}
}

/* --- cursor hiding --- */

#if 0
LOCALPROC ShowCursorProc(CGDirectDisplayID display)
{
	(void) CGDisplayShowCursor(display);
}
#endif

LOCALPROC ShowCursor(void)
{
#if 0
	/* ShowCursor(); deprecated */
	DoForEachDisplay(ShowCursorProc);
#endif
	if (HaveCGDisplayShowCursor()) {
		(void) CGDisplayShowCursor(MainDisplayID());
			/* documentation now claims argument ignored */
	}
}

#if 0
LOCALPROC HideCursorProc(CGDirectDisplayID display)
{
	(void) CGDisplayHideCursor(display);
}
#endif

LOCALPROC HideCursor(void)
{
#if 0
	/* HideCursor(); deprecated */
	DoForEachDisplay(HideCursorProc);
#endif
	if (HaveCGDisplayHideCursor()) {
		(void) CGDisplayHideCursor(MainDisplayID());
			/* documentation now claims argument ignored */
	}
}

LOCALVAR bool HaveCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		ShowCursor();
	}
}

/* --- cursor moving --- */

LOCALPROC SetCursorArrow(void)
{
	SetThemeCursor(kThemeArrowCursor);
}

#if EnableMoveMouse
LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
	Point CurMousePos;
	Rect r;

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		h -= ViewHStart;
		v -= ViewVStart;
	}
#endif

	if (UseMagnify) {
		h *= WindowScale;
		v *= WindowScale;
	}

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		h += hOffset;
		v += vOffset;
	}
#endif

	GetWindowBounds(gMainWindow, kWindowContentRgn, &r);
	CurMousePos.h = r.left + h;
	CurMousePos.v = r.top + v;

	/*
		This method from SDL_QuartzWM.m, "Simple DirectMedia Layer",
		Copyright (C) 1997-2003 Sam Lantinga
	*/
	if (HaveCGSetLocalEventsSuppressionInterval()) {
		if (noErr != CGSetLocalEventsSuppressionInterval(0.0)) {
			/* don't use MacMsg which can call MoveMouse */
		}
	}
	if (HaveCGWarpMouseCursorPosition()) {
		CGPoint pt;
		pt.x = CurMousePos.h;
		pt.y = CurMousePos.v;
		if (noErr != CGWarpMouseCursorPosition(pt)) {
			/* don't use MacMsg which can call MoveMouse */
		}
	}
#if 0
	if (HaveCGDisplayMoveCursorToPoint()) {
		CGPoint pt;
		pt.x = CurMousePos.h;
		pt.y = CurMousePos.v;
		if (noErr != CGDisplayMoveCursorToPoint(
			MainDisplayID(), pt))
		{
			/* don't use MacMsg which can call MoveMouse */
		}
	}
#endif

	return true;
}
#endif

#if EnableFSMouseMotion
LOCALPROC AdjustMouseMotionGrab(void)
{
	if (gMainWindow != NULL) {
#if MayFullScreen
		if (GrabMachine) {
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
		} else
#endif
		{
			if (HaveMouseMotion) {
				(void) MoveMouse(CurMouseH, CurMouseV);
				HaveMouseMotion = false;
			}
		}
	}
}
#endif

#if EnableFSMouseMotion
LOCALPROC MouseConstrain(void)
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

#if 0
LOCALFUNC bool InitMousePosition(void)
{
	/*
		Since there doesn't seem to be any nondeprecated
		way to get initial cursor position, instead
		start by moving cursor to known position.
	*/

#if 1
	if (! UseFullScreen)
#endif
#if MayNotFullScreen
	{
		CurMouseH = 16;
		CurMouseV = 16;
		WantCursorHidden = true;
		(void) MoveMouse(CurMouseH, CurMouseV);
	}
#endif

	return true;
}
#endif

/* --- time, date, location, part 2 --- */

#include "UTIL/DATE2SEC.h"

LOCALFUNC bool InitLocationDat(void)
{
#if AutoLocation || AutoTimeZone
	MachineLocation loc;

	ReadLocation(&loc);
#if AutoLocation
	CurMacLatitude = (uint32_t)loc.latitude;
	CurMacLongitude = (uint32_t)loc.longitude;
#endif
#if AutoTimeZone
	CurMacDelta = (uint32_t)loc.u.gmtDelta;
#endif
#endif

	{
		CFTimeZoneRef tz = CFTimeZoneCopySystem();
		if (tz) {
			/* CFAbsoluteTime */ ATimeBase = CFAbsoluteTimeGetCurrent();
			/* ETimeBase = GetCurrentEventTime(); */
			{
				CFGregorianDate d = CFAbsoluteTimeGetGregorianDate(
					ATimeBase, tz);
				double floorsec = floor(d.second);
				ATimeBase -= (d.second - floorsec);
				/* ETimeBase -= (d.second - floorsec); */
				TimeSecBase = Date2MacSeconds(floorsec,
					d.minute, d.hour,
					d.day, d.month, d.year);

				(void) CheckDateTime();
			}
			CFRelease(tz);
		}
	}

	OnTrueTime = TrueEmulatedTime;

	return true;
}

LOCALPROC StartUpTimeAdjust(void)
{
	NextTickChangeTime = GetCurrentEventTime() + TickDuration;
}

/* --- sound --- */

#if SoundEnabled

#define kLn2SoundBuffers 4 /* kSoundBuffers must be a power of two */
#define kSoundBuffers (1 << kLn2SoundBuffers)
#define kSoundBuffMask (kSoundBuffers - 1)

#define DesiredMinFilledSoundBuffs 3
	/*
		if too big then sound lags behind emulation.
		if too small then sound will have pauses.
	*/

#define kLnOneBuffLen 9
#define kLnAllBuffLen (kLn2SoundBuffers + kLnOneBuffLen)
#define kOneBuffLen (1UL << kLnOneBuffLen)
#define kAllBuffLen (1UL << kLnAllBuffLen)
#define kLnOneBuffSz (kLnOneBuffLen + kLn2SoundSampSz - 3)
#define kLnAllBuffSz (kLnAllBuffLen + kLn2SoundSampSz - 3)
#define kOneBuffSz (1UL << kLnOneBuffSz)
#define kAllBuffSz (1UL << kLnAllBuffSz)
#define kOneBuffMask (kOneBuffLen - 1)
#define kAllBuffMask (kAllBuffLen - 1)
#define dbhBufferSize (kAllBuffSz + kOneBuffSz)

#define dbglog_SoundStuff (0 && dbglog_HAVE)
#define dbglog_SoundBuffStats (0 && dbglog_HAVE)

LOCALVAR tpSoundSamp TheSoundBuffer = nullpr;
volatile static uint16_t ThePlayOffset;
volatile static uint16_t TheFillOffset;
volatile static uint16_t MinFilledSoundBuffs;
#if dbglog_SoundBuffStats
LOCALVAR uint16_t MaxFilledSoundBuffs;
#endif
LOCALVAR uint16_t TheWriteOffset;

LOCALPROC Sound_Start0(void)
{
	/* Reset variables */
	ThePlayOffset = 0;
	TheFillOffset = 0;
	TheWriteOffset = 0;
	MinFilledSoundBuffs = kSoundBuffers + 1;
#if dbglog_SoundBuffStats
	MaxFilledSoundBuffs = 0;
#endif
}

GLOBALOSGLUFUNC tpSoundSamp Sound_BeginWrite(uint16_t n, uint16_t *actL)
{
	uint16_t ToFillLen = kAllBuffLen - (TheWriteOffset - ThePlayOffset);
	uint16_t WriteBuffContig =
		kOneBuffLen - (TheWriteOffset & kOneBuffMask);

	if (WriteBuffContig < n) {
		n = WriteBuffContig;
	}
	if (ToFillLen < n) {
		/* overwrite previous buffer */
#if dbglog_SoundStuff
		dbglog_writeln("sound buffer over flow");
#endif
		TheWriteOffset -= kOneBuffLen;
	}

	*actL = n;
	return TheSoundBuffer + (TheWriteOffset & kAllBuffMask);
}

#if 4 == kLn2SoundSampSz
LOCALPROC ConvertSoundBlockToNative(tpSoundSamp p)
{
	int i;

	for (i = kOneBuffLen; --i >= 0; ) {
		*p++ -= 0x8000;
	}
}
#else
#define ConvertSoundBlockToNative(p)
#endif

LOCALPROC Sound_WroteABlock(void)
{
#if (4 == kLn2SoundSampSz)
	uint16_t PrevWriteOffset = TheWriteOffset - kOneBuffLen;
	tpSoundSamp p = TheSoundBuffer + (PrevWriteOffset & kAllBuffMask);
#endif

#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_WroteABlock");
#endif

	ConvertSoundBlockToNative(p);

	TheFillOffset = TheWriteOffset;

#if dbglog_SoundBuffStats
	{
		uint16_t ToPlayLen = TheFillOffset
			- ThePlayOffset;
		uint16_t ToPlayBuffs = ToPlayLen >> kLnOneBuffLen;

		if (ToPlayBuffs > MaxFilledSoundBuffs) {
			MaxFilledSoundBuffs = ToPlayBuffs;
		}
	}
#endif
}

LOCALFUNC bool Sound_EndWrite0(uint16_t actL)
{
	bool v;

	TheWriteOffset += actL;

	if (0 != (TheWriteOffset & kOneBuffMask)) {
		v = false;
	} else {
		/* just finished a block */

		Sound_WroteABlock();

		v = true;
	}

	return v;
}

LOCALPROC Sound_SecondNotify0(void)
{
	if (MinFilledSoundBuffs <= kSoundBuffers) {
		if (MinFilledSoundBuffs > DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too high");
#endif
			NextTickChangeTime += TickDuration;
		} else if (MinFilledSoundBuffs < DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too low");
#endif
			++TrueEmulatedTime;
		}
#if dbglog_SoundBuffStats
		dbglog_writelnNum("MinFilledSoundBuffs",
			MinFilledSoundBuffs);
		dbglog_writelnNum("MaxFilledSoundBuffs",
			MaxFilledSoundBuffs);
		MaxFilledSoundBuffs = 0;
#endif
		MinFilledSoundBuffs = kSoundBuffers + 1;
	}
}

LOCALPROC RampSound(tpSoundSamp p,
	trSoundSamp BeginVal, trSoundSamp EndVal)
{
	int i;
	uint32_t v = (((uint32_t)BeginVal) << kLnOneBuffLen) + (kLnOneBuffLen >> 1);

	for (i = kOneBuffLen; --i >= 0; ) {
		*p++ = v >> kLnOneBuffLen;
		v = v + EndVal - BeginVal;
	}
}

#if 4 == kLn2SoundSampSz
#define ConvertSoundSampleFromNative(v) ((v) + 0x8000)
#else
#define ConvertSoundSampleFromNative(v) (v)
#endif

struct SoundR {
	tpSoundSamp fTheSoundBuffer;
	volatile uint16_t (*fPlayOffset);
	volatile uint16_t (*fFillOffset);
	volatile uint16_t (*fMinFilledSoundBuffs);

	volatile bool PlayingBuffBlock;
	volatile trSoundSamp lastv;
	volatile bool wantplaying;
	volatile bool StartingBlocks;

	CmpSoundHeader /* ExtSoundHeader */ soundHeader;
};
typedef struct SoundR   SoundR;


/*
	Some of this code descended from CarbonSndPlayDB, an
	example from Apple, as found being used in vMac for Mac OS.
*/

LOCALPROC InsertSndDoCommand(SndChannelPtr chan, SndCommand * newCmd)
{
	if (-1 == chan->qHead) {
		chan->qHead = chan->qTail;
	}

	if (1 <= chan->qHead) {
		chan->qHead--;
	} else {
		chan->qHead = chan->qTail;
	}

	chan->queue[chan->qHead] = *newCmd;
}

/* call back */ static pascal void
Sound_CallBack(SndChannelPtr theChannel, SndCommand * theCallBackCmd)
{
	SoundR *datp =
		(SoundR *)(theCallBackCmd->param2);
	bool wantplaying0 = datp->wantplaying;
	trSoundSamp v0 = datp->lastv;

#if dbglog_SoundStuff
	dbglog_writeln("Enter Sound_CallBack");
#endif

	if (datp->PlayingBuffBlock) {
		/* finish with last sample */
#if dbglog_SoundStuff
		dbglog_writeln("done with sample");
#endif

		*datp->fPlayOffset += kOneBuffLen;
		datp->PlayingBuffBlock = false;
	}

	if ((! wantplaying0) && (kCenterSound == v0)) {
#if dbglog_SoundStuff
		dbglog_writeln("terminating");
#endif
	} else {
		SndCommand playCmd;
		tpSoundSamp p;
		trSoundSamp v1 = v0;
		bool WantRamp = false;
		uint16_t CurPlayOffset = *datp->fPlayOffset;
		uint16_t ToPlayLen = *datp->fFillOffset - CurPlayOffset;
		uint16_t FilledSoundBuffs = ToPlayLen >> kLnOneBuffLen;

		if (FilledSoundBuffs < *datp->fMinFilledSoundBuffs) {
			*datp->fMinFilledSoundBuffs = FilledSoundBuffs;
		}

		if (! wantplaying0) {
#if dbglog_SoundStuff
			dbglog_writeln("playing end transistion");
#endif
			v1 = kCenterSound;

			WantRamp = true;
		} else
		if (datp->StartingBlocks) {
#if dbglog_SoundStuff
			dbglog_writeln("playing start block");
#endif

			if ((ToPlayLen >> kLnOneBuffLen) < 12) {
				datp->StartingBlocks = false;
#if dbglog_SoundStuff
				dbglog_writeln("have enough samples to start");
#endif

				p = datp->fTheSoundBuffer
					+ (CurPlayOffset & kAllBuffMask);
				v1 = ConvertSoundSampleFromNative(*p);
			}

			WantRamp = true;
		} else
		if (0 == FilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("playing under run");
#endif

			WantRamp = true;
		} else
		{
			/* play next sample */
			p = datp->fTheSoundBuffer
				+ (CurPlayOffset & kAllBuffMask);
			datp->PlayingBuffBlock = true;
			v1 =
				ConvertSoundSampleFromNative(*(p + kOneBuffLen - 1));
#if dbglog_SoundStuff
			dbglog_writeln("playing sample");
#endif
		}

		if (WantRamp) {
			p = datp->fTheSoundBuffer + kAllBuffLen;

#if dbglog_SoundStuff
			dbglog_writelnNum("v0", v0);
			dbglog_writelnNum("v1", v1);
#endif

			RampSound(p, v0, v1);
			ConvertSoundBlockToNative(p);
		}

		datp->soundHeader.samplePtr = (Ptr)p;
		datp->soundHeader.numFrames =
			(unsigned long)kOneBuffLen;

		/* Insert our callback command */
		InsertSndDoCommand (theChannel, theCallBackCmd);

#if 0
		{
			int i;
			tpSoundSamp pS =
				(tpSoundSamp)datp->soundHeader.samplePtr;

			for (i = datp->soundHeader.numFrames; --i >= 0; )
			{
				fprintf(stderr, "%d\n", *pS++);
			}
		}
#endif

		/* Play the next buffer */
		playCmd.cmd = bufferCmd;
		playCmd.param1 = 0;
		playCmd.param2 = (long)&(datp->soundHeader);
		InsertSndDoCommand (theChannel, &playCmd);

		datp->lastv = v1;
	}
}

LOCALVAR SoundR cur_audio;

LOCALVAR SndCallBackUPP gCarbonSndPlayDoubleBufferCallBackUPP = NULL;

LOCALVAR SndChannelPtr sndChannel = NULL; /* our sound channel */

LOCALPROC Sound_Start(void)
{
#if dbglog_SoundStuff
	dbglog_writeln("Sound_Start");
#endif

	if (NULL == sndChannel) {
		SndCommand callBack;
		SndChannelPtr chan = NULL;

		cur_audio.wantplaying = false;
		cur_audio.StartingBlocks = false;

		Sound_Start0();

		SndNewChannel(&chan, sampledSynth, initMono, nil);
		if (NULL != chan) {
			sndChannel = chan;

			cur_audio.PlayingBuffBlock = false;
			cur_audio.lastv = kCenterSound;
			cur_audio.StartingBlocks = true;
			cur_audio.wantplaying = true;

			callBack.cmd = callBackCmd;
			callBack.param1 = 0; /* unused */
			callBack.param2 = (long)&cur_audio;

			sndChannel->callBack =
				gCarbonSndPlayDoubleBufferCallBackUPP;

			(void) SndDoCommand (sndChannel, &callBack, true);
		}
	}
}

LOCALPROC Sound_Stop(void)
{
#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_Stop");
#endif

	if (NULL != sndChannel) {
		uint16_t retry_limit = 50; /* half of a second */
		SCStatus r;

		cur_audio.wantplaying = false;
#if dbglog_SoundStuff
		dbglog_writeln("cleared wantplaying");
#endif

label_retry:
		r.scChannelBusy = false; /* what is this for? */
		if (noErr != SndChannelStatus(sndChannel,
			sizeof(SCStatus), &r))
		{
			/* fail */
		} else
		if ((! r.scChannelBusy) && (kCenterSound == cur_audio.lastv)) {
			/* done */

			/*
				observed reporting not busy unexpectedly,
				so also check lastv.
			*/
		} else
		if (0 == --retry_limit) {
#if dbglog_SoundStuff
			dbglog_writeln("retry limit reached");
#endif
			/*
				don't trust SndChannelStatus, make
				sure don't get in infinite loop.
			*/

			/* done */
		} else
		{
			/*
				give time back, particularly important
				if got here on a suspend event.
			*/
			struct timespec rqt;
			struct timespec rmt;

#if dbglog_SoundStuff
			dbglog_writeln("busy, so sleep");
#endif

			rqt.tv_sec = 0;
			rqt.tv_nsec = 10000000;
			(void) nanosleep(&rqt, &rmt);

			goto label_retry;
		}

		SndDisposeChannel(sndChannel, true);
		sndChannel = NULL;
	}

#if dbglog_SoundStuff
	dbglog_writeln("leave Sound_Stop");
#endif
}

#define SOUND_SAMPLERATE rate22khz
	/* = 0x56EE8BA3 = (7833600 * 2 / 704) << 16 */

LOCALFUNC bool Sound_Init(void)
{
#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_Init");
#endif

	cur_audio.fTheSoundBuffer = TheSoundBuffer;

	cur_audio.fPlayOffset = &ThePlayOffset;
	cur_audio.fFillOffset = &TheFillOffset;
	cur_audio.fMinFilledSoundBuffs = &MinFilledSoundBuffs;
	cur_audio.wantplaying = false;

	/* Init basic per channel information */
	cur_audio.soundHeader.sampleRate = SOUND_SAMPLERATE;
		/* sample rate */
	cur_audio.soundHeader.numChannels = 1; /* one channel */
	cur_audio.soundHeader.loopStart = 0;
	cur_audio.soundHeader.loopEnd = 0;
	cur_audio.soundHeader.encode = cmpSH /* extSH */;
	cur_audio.soundHeader.baseFrequency = kMiddleC;
	cur_audio.soundHeader.numFrames =
		(unsigned long)kOneBuffLen;
	/* cur_audio.soundHeader.AIFFSampleRate = 0; */
		/* unused */
	cur_audio.soundHeader.markerChunk = nil;
	cur_audio.soundHeader.futureUse2 = 0;
	cur_audio.soundHeader.stateVars = nil;
	cur_audio.soundHeader.leftOverSamples = nil;
	cur_audio.soundHeader.compressionID = 0;
		/* no compression */
	cur_audio.soundHeader.packetSize = 0;
		/* no compression */
	cur_audio.soundHeader.snthID = 0;
	cur_audio.soundHeader.sampleSize = (1 << kLn2SoundSampSz);
		/* 8 or 16 bits per sample */
	cur_audio.soundHeader.sampleArea[0] = 0;
#if 3 == kLn2SoundSampSz
	cur_audio.soundHeader.format = kSoundNotCompressed;
#elif 4 == kLn2SoundSampSz
	cur_audio.soundHeader.format = k16BitNativeEndianFormat;
#else
#error "unsupported kLn2SoundSampSz"
#endif
	cur_audio.soundHeader.samplePtr = (Ptr)TheSoundBuffer;

	gCarbonSndPlayDoubleBufferCallBackUPP =
		NewSndCallBackUPP(Sound_CallBack);
	if (gCarbonSndPlayDoubleBufferCallBackUPP != NULL) {

		Sound_Start();
			/*
				This should be taken care of by LeaveSpeedStopped,
				but since takes a while to get going properly,
				start early.
			*/

		return true;
	}
	return false;
}

GLOBALOSGLUPROC Sound_EndWrite(uint16_t actL)
{
	if (Sound_EndWrite0(actL)) {
	}
}

LOCALPROC Sound_SecondNotify(void)
{
	if (sndChannel != NULL) {
		Sound_SecondNotify0();
	}
}

#endif


LOCALPROC AdjustGLforSize(int h, int v)
{
	if (GL_TRUE != aglSetCurrentContext(ctx)) {
		/* err = aglReportError() */
	} else {

		glClearColor (0.0, 0.0, 0.0, 1.0);

#if 1
		glViewport(0, 0, h, v);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, h, 0, v, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
#endif

		glColor3f(0.0, 0.0, 0.0);
		if (UseMagnify) {
			glPixelZoom(WindowScale, - WindowScale);
		} else {
			glPixelZoom(1, -1);
		}
		glPixelStorei(GL_UNPACK_ROW_LENGTH, vMacScreenWidth);

		glClear(GL_COLOR_BUFFER_BIT);

		if (GL_TRUE != aglSetCurrentContext(NULL)) {
			/* err = aglReportError() */
		}

		ScreenChangedAll();
	}
}

LOCALVAR bool WantScreensChangedCheck = false;

LOCALPROC UpdateOpenGLContext(void)
{
	if (NULL == ctx) {
		/* oops */
	} else if (GL_TRUE != aglSetCurrentContext(ctx)) {
		/* err = aglReportError() */
	} else {
		aglUpdateContext(ctx);
	}
}

#if 0 /* some experiments */
LOCALVAR CGrafPtr GrabbedPort = NULL;
#endif

#if 0
LOCALPROC AdjustMainScreenGrab(void)
{
	if (GrabMachine) {
		if (NULL == GrabbedPort) {
			/* CGDisplayCapture(MainDisplayID()); */
			CGCaptureAllDisplays();
			/* CGDisplayHideCursor( MainDisplayID() ); */
			GrabbedPort =
				CreateNewPortForCGDisplayID((UInt32)MainDisplayID());
			LockPortBits (GrabbedPort);
		}
	} else {
		if (GrabbedPort != NULL) {
			UnlockPortBits (GrabbedPort);
			/* CGDisplayShowCursor( MainDisplayID() ); */
			/* CGDisplayRelease(MainDisplayID()); */
			CGReleaseAllDisplays();
			GrabbedPort = NULL;
		}
	}
}
#endif

#if 0
typedef CGDisplayErr (*CGReleaseAllDisplaysProcPtr)
	(void);

LOCALPROC ReleaseAllDisplays(void)
{
	if (HaveApplicationServicesBun()) {
		CGReleaseAllDisplaysProcPtr ReleaseAllDisplaysProc =
			(CGReleaseAllDisplaysProcPtr)
			CFBundleGetFunctionPointerForName(
				AppServBunRef, CFSTR("CGReleaseAllDisplays"));
		if (ReleaseAllDisplaysProc != NULL) {
			ReleaseAllDisplaysProc();
		}
	}
}

typedef CGDisplayErr (*CGCaptureAllDisplaysProcPtr)
	(void);

LOCALPROC CaptureAllDisplays(void)
{
	if (HaveApplicationServicesBun()) {
		CGCaptureAllDisplaysProcPtr CaptureAllDisplaysProc =
			(CGCaptureAllDisplaysProcPtr)
			CFBundleGetFunctionPointerForName(
				AppServBunRef, CFSTR("CGCaptureAllDisplays"));
		if (CaptureAllDisplaysProc != NULL) {
			CaptureAllDisplaysProc();
		}
	}
}
#endif

LOCALVAR AGLPixelFormat window_fmt = NULL;
LOCALVAR AGLContext window_ctx = NULL;

#ifndef UseAGLfullscreen
#define UseAGLfullscreen 0
#endif

#if UseAGLfullscreen
LOCALVAR AGLPixelFormat fullscreen_fmt = NULL;
LOCALVAR AGLContext fullscreen_ctx = NULL;
#endif

#if UseAGLfullscreen
LOCALPROC MaybeFullScreen(void)
{
	if (
#if 1
		UseFullScreen &&
#endif
		(NULL == fullscreen_ctx)
		&& HaveCGDisplayPixelsWide()
		&& HaveCGDisplayPixelsHigh())
	{
		static const GLint fullscreen_attrib[] = {AGL_RGBA,
			AGL_NO_RECOVERY,
			AGL_FULLSCREEN,
			AGL_SINGLE_RENDERER, AGL_ACCELERATED,
#if UseAGLdoublebuff
			AGL_DOUBLEBUFFER,
#endif
			AGL_NONE};
		GDHandle theDevice;
		CGDirectDisplayID CurMainDisplayID = MainDisplayID();

		DMGetGDeviceByDisplayID(
			(DisplayIDType)CurMainDisplayID, &theDevice, false);
		fullscreen_fmt = aglChoosePixelFormat(
			&theDevice, 1, fullscreen_attrib);
		if (NULL == fullscreen_fmt) {
			/* err = aglReportError() */
		} else {
			fullscreen_ctx = aglCreateContext(fullscreen_fmt, NULL);
			if (NULL == fullscreen_ctx) {
				/* err = aglReportError() */
			} else {
				/* CaptureAllDisplays(); */
				if (GL_TRUE != aglSetFullScreen(fullscreen_ctx,
					0, 0, 0, 0))
				{
					/* err = aglReportError() */
				} else {
					Rect r;

					int h = CGDisplayPixelsWide(CurMainDisplayID);
					int v = CGDisplayPixelsHigh(CurMainDisplayID);

					GetWindowBounds(gMainWindow, kWindowContentRgn,
						&r);

					GLhOffset = r.left + hOffset;
					GLvOffset = v - (r.top + vOffset);

					ctx = fullscreen_ctx;
					AdjustGLforSize(h, v);
					return;
				}
				/* ReleaseAllDisplays(); */

				if (GL_TRUE != aglDestroyContext(fullscreen_ctx)) {
					/* err = aglReportError() */
				}
				fullscreen_ctx = NULL;
			}

			aglDestroyPixelFormat (fullscreen_fmt);
			fullscreen_fmt = NULL;
		}
	}
}
#endif

#if UseAGLfullscreen
LOCALPROC NoFullScreen(void)
{
	if (fullscreen_ctx != NULL) {
		Rect r;
		int h;
		int v;

		GetWindowBounds(gMainWindow, kWindowContentRgn, &r);

		h = r.right - r.left;
		v = r.bottom - r.top;

		GLhOffset = hOffset;
		GLvOffset = v - vOffset;

		ctx = window_ctx;

		AdjustGLforSize(h, v);

		Update_Screen();

		if (fullscreen_ctx != NULL) {
			if (GL_TRUE != aglDestroyContext(fullscreen_ctx)) {
				/* err = aglReportError() */
			}
			fullscreen_ctx = NULL;
		}
		if (fullscreen_fmt != NULL) {
			aglDestroyPixelFormat(fullscreen_fmt);
			fullscreen_fmt = NULL;
		}
	}
}
#endif

#if UseAGLfullscreen
LOCALPROC AdjustOpenGLGrab(void)
{
	if (GrabMachine) {
		MaybeFullScreen();
	} else {
		NoFullScreen();
	}
}
#endif

#if MayFullScreen
LOCALPROC AdjustMachineGrab(void)
{
#if EnableFSMouseMotion
	AdjustMouseMotionGrab();
#endif
#if UseAGLfullscreen
	AdjustOpenGLGrab();
#endif
#if 0
	AdjustMainScreenGrab();
#endif
}
#endif

#if MayFullScreen
LOCALPROC UngrabMachine(void)
{
	GrabMachine = false;
	AdjustMachineGrab();
}
#endif

LOCALPROC ClearWeAreActive(void)
{
	if (gWeAreActive) {
		gWeAreActive = false;

		DisconnectKeyCodes2();

		SavedModifiers &= alphaLock;

		MouseButtonSet(false);

#if MayFullScreen
		UngrabMachine();
#endif

		ForceShowCursor();
	}
}

/* --- basic dialogs --- */

LOCALFUNC CFStringRef CFStringCreateFromSubstCStr(char *s)
{
	int L;
	UniChar x[ClStrMaxLength];

	UniCharStrFromSubstCStr(&L, x, s);

	return CFStringCreateWithCharacters(kCFAllocatorDefault, x, L);
}

#define kStandardAlert 128

LOCALPROC CheckSavedMacMsg(void)
{
	/* called only on quit, if error saved but not yet reported */

	if (nullpr != SavedBriefMsg) {
		if (HaveCreateStandardAlert() && HaveRunStandardAlert()) {
			CFStringRef briefMsgu = CFStringCreateFromSubstCStr(
				SavedBriefMsg);
			if (NULL != briefMsgu) {
				CFStringRef longMsgu = CFStringCreateFromSubstCStr(
					SavedLongMsg);
				if (NULL != longMsgu) {
					DialogRef TheAlert;
					OSStatus err = CreateStandardAlert(
						(SavedFatalMsg) ? kAlertStopAlert
							: kAlertCautionAlert,
						briefMsgu, longMsgu, NULL,
						&TheAlert);
					if (noErr == err) {
						(void) RunStandardAlert(TheAlert, NULL, NULL);
					}
					CFRelease(longMsgu);
				}
				CFRelease(briefMsgu);
			}
		}

		SavedBriefMsg = nullpr;
	}
}

LOCALVAR bool gTrueBackgroundFlag = false;

LOCALPROC BeginDialog(void)
{
	ClearWeAreActive();
}

LOCALPROC EndDialog(void)
{
}

/* --- hide/show menubar --- */

#if MayFullScreen
LOCALPROC _HideMenuBar(void)
{
	if (HaveSetSystemUIMode()) {
		(void) SetSystemUIMode(kUIModeAllHidden,
			kUIOptionDisableAppleMenu
#if GrabKeysFullScreen
			| kUIOptionDisableProcessSwitch
#if GrabKeysMaxFullScreen /* dangerous !! */
			| kUIOptionDisableForceQuit
			| kUIOptionDisableSessionTerminate
#endif
#endif
			);
	} else {
		if (IsMenuBarVisible()) {
			HideMenuBar();
		}
	}
}
#endif

#if MayFullScreen
LOCALPROC _ShowMenuBar(void)
{
	if (HaveSetSystemUIMode()) {
		(void) SetSystemUIMode(kUIModeNormal,
			0);
	} else {
		if (! IsMenuBarVisible()) {
			ShowMenuBar();
		}
	}
}
#endif

/* --- drives, part 2 --- */

LOCALPROC InitDrives(void)
{
	/*
		This isn't really needed, Drives[i]
		need not have valid value when not vSonyIsInserted[i].
	*/
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
	}
}

LOCALPROC UnInitDrives(void)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (vSonyIsInserted(i)) {
			(void) vSonyEject(i);
		}
	}
}

LOCALFUNC tMacErr Sony_Insert0(SInt16 refnum, bool locked)
{
	tDrive Drive_No;

	if (! FirstFreeDisk(&Drive_No)) {
		(void) FSCloseFork(refnum);
		return mnvm_tmfoErr;
	} else {
		Drives[Drive_No] = refnum;
		DiskInsertNotify(Drive_No, locked);
		return mnvm_noErr;
	}
}

LOCALPROC ReportStandardOpenDiskError(tMacErr err)
{
	if (mnvm_noErr != err) {
		if (mnvm_tmfoErr == err) {
			MacMsg(kStrTooManyImagesTitle,
				kStrTooManyImagesMessage, false);
		} else if (mnvm_opWrErr == err) {
			MacMsg(kStrImageInUseTitle,
				kStrImageInUseMessage, false);
		} else {
			MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
		}
	}
}

LOCALFUNC tMacErr InsertADiskFromFSRef(FSRef *theRef)
{
	tMacErr err;
	HFSUniStr255 forkName;
	SInt16 refnum;
	bool locked = false;

	if (CheckSaveMacErr(FSGetDataForkName(&forkName))) {
		err = To_tMacErr(FSOpenFork(theRef, forkName.length,
			forkName.unicode, fsRdWrPerm, &refnum));
		switch (err) {
			case mnvm_permErr:
			case mnvm_wrPermErr:
			case mnvm_afpAccessDenied:
				locked = true;
				err = To_tMacErr(FSOpenFork(theRef, forkName.length,
					forkName.unicode, fsRdPerm, &refnum));
				break;
			default:
				break;
		}
		if (mnvm_noErr == err) {
			err = Sony_Insert0(refnum, locked);
		}
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromRefNum(SInt16 refnum)
{
	tMacErr err;
	ByteCount actualCount;

	if (mnvm_noErr != (err = To_tMacErr(
		FSReadFork(refnum, fsFromStart, 0,
			kROM_Size, ROM, &actualCount))))
	{
		if (mnvm_eofErr == err) {
			MacMsgOverride(kStrShortROMTitle, kStrShortROMMessage);
		} else {
			MacMsgOverride(kStrNoReadROMTitle, kStrNoReadROMMessage);
		}
	} else
	{
		err = ROM_IsValid();
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromFSRef(FSRef *theRef)
{
	tMacErr err;
	HFSUniStr255 forkName;
	SInt16 refnum;

	if (mnvm_noErr == (err =
		To_tMacErr(FSGetDataForkName(&forkName))))
	if (mnvm_noErr == (err = To_tMacErr(
		FSOpenFork(theRef, forkName.length,
			forkName.unicode, fsRdPerm, &refnum))))
	{
		err = LoadMacRomFromRefNum(refnum);

		(void) FSCloseFork(refnum);
	}

	return err;
}

LOCALFUNC tMacErr InsertADiskFromFSRef1(FSRef *theRef)
{
	tMacErr err;

	if (! ROM_loaded) {
		err = LoadMacRomFromFSRef(theRef);
	} else {
		err = InsertADiskFromFSRef(theRef);
	}

	return err;
}

LOCALFUNC tMacErr InsertDisksFromDocList(AEDescList *docList)
{
	tMacErr err = mnvm_noErr;
	long itemsInList;
	long index;
	AEKeyword keyword;
	DescType typeCode;
	FSRef theRef;
	Size actualSize;

	if (CheckSaveMacErr(AECountItems(docList, &itemsInList))) {
		for (index = 1; index <= itemsInList; ++index) {
			if (CheckSaveMacErr(AEGetNthPtr(docList, index, typeFSRef,
				&keyword, &typeCode, (Ptr)&theRef,
				sizeof(FSRef), &actualSize)))
			if (CheckSavetMacErr(InsertADiskFromFSRef1(&theRef)))
			{
			}
			if (mnvm_noErr != err) {
				goto label_fail;
			}
		}
	}

label_fail:
	return err;
}

LOCALFUNC tMacErr InsertADiskFromNameEtc(FSRef *ParentRef,
	char *fileName)
{
	tMacErr err;
	bool isFolder;
	FSRef ChildRef;

	if (CheckSavetMacErr(MakeFSRefC(ParentRef, fileName,
		&isFolder, &ChildRef)))
	{
		if (isFolder) {
			err = mnvm_miscErr;
		} else {
			if (CheckSavetMacErr(InsertADiskFromFSRef(&ChildRef))) {
				/* ok */
			}
		}
	}

	return err;
}

#if IncludeSonyNew
LOCALFUNC tMacErr WriteZero(SInt16 refnum, uint32_t L)
{
#define ZeroBufferSize 2048
	tMacErr err = mnvm_noErr;
	uint32_t i;
	uint8_t buffer[ZeroBufferSize];
	ByteCount actualCount;
	uint32_t offset = 0;

	memset(&buffer, 0, ZeroBufferSize);

	while (L > 0) {
		i = (L > ZeroBufferSize) ? ZeroBufferSize : L;
		if (! CheckSaveMacErr(FSWriteFork(refnum,
			fsFromStart,
			offset,
			i,
			buffer,
			&actualCount)))
		{
			goto label_fail;
		}
		L -= i;
		offset += i;
	}

label_fail:
	return err;
}
#endif

#if IncludeSonyNew
LOCALFUNC tMacErr CreateFileCFStringRef(FSRef *saveFileParent,
	CFStringRef saveFileName, FSRef *NewRef)
{
	tMacErr err;
	UniChar buffer[255];

	CFIndex len = CFStringGetLength(saveFileName);

	if (len > 255) {
		len = 255;
	}

	CFStringGetCharacters(saveFileName, CFRangeMake(0, len), buffer);

	err = To_tMacErr(FSMakeFSRefUnicode(saveFileParent, len,
		buffer, kTextEncodingUnknown, NewRef));
	if (mnvm_fnfErr == err) { /* file is not there yet - create it */
		err = To_tMacErr(FSCreateFileUnicode(saveFileParent,
			len, buffer, 0, NULL, NewRef, NULL));
	}

	return err;
}
#endif

#if IncludeSonyNew
LOCALFUNC tMacErr MakeNewDisk0(FSRef *saveFileParent,
	CFStringRef saveFileName, uint32_t L)
{
	tMacErr err;
	FSRef NewRef;
	HFSUniStr255 forkName;
	SInt16 refnum;

	if (CheckSavetMacErr(
		CreateFileCFStringRef(saveFileParent, saveFileName, &NewRef)))
	{
		if (CheckSaveMacErr(FSGetDataForkName(&forkName)))
		if (CheckSaveMacErr(FSOpenFork(&NewRef, forkName.length,
			forkName.unicode, fsRdWrPerm, &refnum)))
		{
			SInt64 forkSize = L;
			if (CheckSaveMacErr(
				FSSetForkSize(refnum, fsFromStart, forkSize)))
			/*
				zero out fork. It looks like this is already done,
				but documentation says this isn't guaranteed.
			*/
			if (CheckSavetMacErr(WriteZero(refnum, L)))
			{
				err = Sony_Insert0(refnum, false);
				refnum = NotAfileRef;
			}
			if (NotAfileRef != refnum) {
				(void) FSCloseFork(refnum);
			}
		}
		if (mnvm_noErr != err) {
			(void) FSDeleteObject(&NewRef);
		}
	}

	return err;
}
#endif

#if IncludeSonyNameNew
LOCALFUNC CFStringRef CFStringCreateWithPbuf(tPbuf i)
{
	return CFStringCreateWithBytes(NULL,
		(UInt8 *)PbufDat[i], PbufSize[i],
		kCFStringEncodingMacRoman, false);
}
#endif

pascal Boolean NavigationFilterProc(
	AEDesc* theItem, void* info, void* NavCallBackUserData,
	NavFilterModes theNavFilterModes);
pascal Boolean NavigationFilterProc(
	AEDesc* theItem, void* info, void* NavCallBackUserData,
	NavFilterModes theNavFilterModes)
{
	/* NavFileOrFolderInfo* theInfo = (NavFileOrFolderInfo*)info; */
	UnusedParam(theItem);
	UnusedParam(info);
	UnusedParam(theNavFilterModes);
	UnusedParam(NavCallBackUserData);

	return true; /* display all items */
}


pascal void NavigationEventProc(
	NavEventCallbackMessage callBackSelector,
	NavCBRecPtr callBackParms, void *NavCallBackUserData);
pascal void NavigationEventProc(
	NavEventCallbackMessage callBackSelector,
	NavCBRecPtr callBackParms, void *NavCallBackUserData)
{
	UnusedParam(NavCallBackUserData);

	switch (callBackSelector) {
		case kNavCBEvent: /* probably not needed in os x */
			switch (callBackParms->eventData.eventDataParms.event->what)
			{
				case updateEvt:
					{
						WindowPtr which =
							(WindowPtr)callBackParms
								->eventData.eventDataParms.event
								->message;

						BeginUpdate(which);

						if (which == gMainWindow) {
							Update_Screen();
						}

						EndUpdate(which);
					}
					break;
			}
			break;
#if 0
		case kNavCBUserAction:
			{
				NavUserAction userAction = NavDialogGetUserAction(
					callBackParms->context);
				switch (userAction) {
					case kNavUserActionOpen: {
						/* got something to open */
						break;
					}
				}
			}
			break;
		case kNavCBTerminate:
			break;
#endif
	}
}

LOCALPROC InsertADisk0(void)
{
	NavDialogRef theOpenDialog;
	NavDialogCreationOptions dialogOptions;
	NavReplyRecord theReply;
	NavTypeListHandle openList = NULL;
	NavEventUPP gEventProc = NewNavEventUPP(NavigationEventProc);
	NavObjectFilterUPP filterUPP =
		NewNavObjectFilterUPP(NavigationFilterProc);

	if (noErr == NavGetDefaultDialogCreationOptions(&dialogOptions)) {
		dialogOptions.modality = kWindowModalityAppModal;
		dialogOptions.optionFlags |= kNavDontAutoTranslate;
		dialogOptions.optionFlags &= ~ kNavAllowPreviews;
		if (noErr == NavCreateGetFileDialog(&dialogOptions,
			(NavTypeListHandle)openList,
			gEventProc, NULL,
			filterUPP, NULL, &theOpenDialog))
		{
			BeginDialog();
			(void) NavDialogRun(theOpenDialog);
			EndDialog();
			if (noErr == NavDialogGetReply(theOpenDialog,
				&theReply))
			{
				if (theReply.validRecord) {
					ReportStandardOpenDiskError(
						InsertDisksFromDocList(&theReply.selection));
				}
				(void) NavDisposeReply(&theReply);
			}
			NavDialogDispose(theOpenDialog);
		}
	}

	DisposeNavEventUPP(gEventProc);
	DisposeNavObjectFilterUPP(filterUPP);
}

#if IncludeSonyNew
LOCALPROC MakeNewDisk(uint32_t L, CFStringRef NewDiskName)
{
#if SaveDialogEnable
	NavDialogRef theSaveDialog;
	NavDialogCreationOptions dialogOptions;
	NavReplyRecord theReply;
	NavEventUPP gEventProc = NewNavEventUPP(NavigationEventProc);

	if (noErr == NavGetDefaultDialogCreationOptions(&dialogOptions)) {
		dialogOptions.modality = kWindowModalityAppModal;
		dialogOptions.saveFileName = NewDiskName;
		if (noErr == NavCreatePutFileDialog(&dialogOptions,
			'TEXT', 'MPS ',
			gEventProc, NULL,
			&theSaveDialog))
		{
			BeginDialog();
			(void) NavDialogRun(theSaveDialog);
			EndDialog();
			if (noErr == NavDialogGetReply(theSaveDialog,
				&theReply))
			{
				if (theReply.validRecord) {
					long itemsInList;
					AEKeyword keyword;
					DescType typeCode;
					FSRef theRef;
					Size actualSize;

					if (noErr == AECountItems(
						&theReply.selection, &itemsInList))
					{
						if (itemsInList == 1) {
							if (noErr == AEGetNthPtr(
								&theReply.selection, 1, typeFSRef,
								&keyword, &typeCode, (Ptr)&theRef,
								sizeof(FSRef), &actualSize))
							{
								ReportStandardOpenDiskError(
									MakeNewDisk0(&theRef,
										theReply.saveFileName, L));
							}
						}
					}
				}
				(void) NavDisposeReply(&theReply);
			}
			NavDialogDispose(theSaveDialog);
		}
	}

	DisposeNavEventUPP(gEventProc);
#else /* SaveDialogEnable */
	FSRef OutRef;

	if (mnvm_noErr == FindOrMakeNamedChildRef(&DatDirRef,
		"out", &OutRef))
	{
		MakeNewDisk0(&OutRef, NewDiskName, L);
	}
#endif /* SaveDialogEnable */
}
#endif

LOCALFUNC tMacErr LoadMacRomFromNameFolder(FSRef *ParentRef,
	char *fileName)
{
	tMacErr err;
	bool isFolder;
	FSRef ChildRef;

	if (mnvm_noErr == (err =
		MakeFSRefC(ParentRef, fileName, &isFolder, &ChildRef)))
	if (mnvm_noErr == (err =
		LoadMacRomFromFSRef(&ChildRef)))
	{
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromPrefDir(void)
{
	tMacErr err;
	FSRef PrefRef;
	FSRef GryphelRef;
	FSRef ROMsRef;

	if (mnvm_noErr == (err = To_tMacErr(FSFindFolder(kUserDomain,
		kPreferencesFolderType, kDontCreateFolder, &PrefRef))))
	if (mnvm_noErr == (err = FindNamedChildRef(&PrefRef,
		"Gryphel", &GryphelRef)))
	if (mnvm_noErr == (err = FindNamedChildRef(&GryphelRef,
		"mnvm_rom", &ROMsRef)))
	if (mnvm_noErr == (err = LoadMacRomFromNameFolder(&ROMsRef,
		RomFileName)))
	{
		/* ok */
	}

	return err;
}

LOCALFUNC bool LoadMacRom(void)
{
	tMacErr err;

	if (mnvm_fnfErr == (err =
		LoadMacRomFromNameFolder(&DatDirRef, RomFileName)))
	if (mnvm_fnfErr == (err =
		LoadMacRomFromPrefDir()))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}

LOCALFUNC bool Sony_InsertIth(int i)
{
	tMacErr err = mnvm_noErr;

	if ((i > 9) || ! FirstFreeDisk(nullpr)) {
		return false;
	} else {
		char s[16] = "disk?.dsk";

		s[4] = '0' + i;

		if (! CheckSavetMacErr(InsertADiskFromNameEtc(&DatDirRef, s)))
		{
			if (mnvm_fnfErr != err) {
				ReportStandardOpenDiskError(err);
			}
			/* stop on first error (including file not found) */
			return false;
		}
	}

	return true;
}

LOCALFUNC bool LoadInitialImages(void)
{
	int i;

	for (i = 1; Sony_InsertIth(i); ++i) {
		/* stop on first error (including file not found) */
	}

	return true;
}

#if UseActvFile

#define ActvCodeFileName "act_1"

LOCALFUNC tMacErr OpenActvCodeFile(SInt16 *refnum)
{
	tMacErr err;
	FSRef PrefRef;
	FSRef GryphelRef;
	FSRef ActRef;

	if (CheckSaveMacErr(FSFindFolder(kUserDomain,
		kPreferencesFolderType, kDontCreateFolder, &PrefRef)))
	if (CheckSavetMacErr(FindNamedChildRef(&PrefRef,
		"Gryphel", &GryphelRef)))
	if (CheckSavetMacErr(FindNamedChildRef(&GryphelRef,
		"mnvm_act", &ActRef)))
	if (CheckSavetMacErr(OpenNamedFileInFolderRef(&ActRef,
		ActvCodeFileName, refnum)))
	{
		/* ok */
	}

	return err;
}

LOCALFUNC tMacErr ActvCodeFileLoad(uint8_t * p)
{
	tMacErr err;
	SInt16 refnum;

	if (CheckSavetMacErr(OpenActvCodeFile(&refnum))) {
		ByteCount actualCount;
		err = To_tMacErr(FSReadFork(refnum, fsFromStart, 0,
			ActvCodeFileLen, p, &actualCount));
		(void) FSCloseFork(refnum);
	}

	return err;
}

LOCALFUNC tMacErr ActvCodeFileSave(uint8_t * p)
{
	tMacErr err;
	SInt16 refnum;
	FSRef PrefRef;
	FSRef GryphelRef;
	FSRef ActRef;
	ByteCount count = ActvCodeFileLen;

	if (CheckSaveMacErr(FSFindFolder(kUserDomain,
		kPreferencesFolderType, kDontCreateFolder, &PrefRef)))
	if (CheckSavetMacErr(FindOrMakeNamedChildRef(&PrefRef,
		"Gryphel", &GryphelRef)))
	if (CheckSavetMacErr(FindOrMakeNamedChildRef(&GryphelRef,
		"mnvm_act", &ActRef)))
	if (CheckSavetMacErr(OpenWriteNamedFileInFolderRef(&ActRef,
		ActvCodeFileName, &refnum)))
	{
		ByteCount actualCount;
		err = To_tMacErr(FSWriteFork(refnum, fsFromStart, 0,
			count, p, &actualCount));
		(void) FSCloseFork(refnum);
	}

	return err;
}

#endif /* UseActvFile */

/* --- utilities for adapting to the environment --- */

LOCALPROC GetScreenBitsBounds(Rect *r)
{
	if (HaveCGDisplayBounds()) {
		CGRect cgr = CGDisplayBounds(MainDisplayID());

		r->left = cgr.origin.x;
		r->top = cgr.origin.y;
		r->right = cgr.origin.x + cgr.size.width;
		r->bottom = cgr.origin.y + cgr.size.height;
	}
}

#if MayNotFullScreen
LOCALPROC InvalWholeWindow(WindowRef mw)
{
	Rect bounds;

	GetWindowPortBounds(mw, &bounds);
	InvalWindowRect(mw, &bounds);
}
#endif

#if MayNotFullScreen
LOCALPROC SetMacWindContRect(WindowRef mw, Rect *r)
{
	(void) SetWindowBounds (mw, kWindowContentRgn, r);
	InvalWholeWindow(mw);
}
#endif

#if MayNotFullScreen
LOCALFUNC bool GetWindowTitleBounds(WindowRef mw, Rect *r)
{
	return (noErr == GetWindowBounds(mw, kWindowTitleBarRgn, r));
}
#endif

#if EnableRecreateW && MayNotFullScreen
LOCALFUNC bool GetWindowContBounds(WindowRef mw, Rect *r)
{
	return (noErr == GetWindowBounds(mw, kWindowContentRgn, r));
}
#endif

LOCALPROC GetGrayRgnBounds(Rect *r)
{
	GetRegionBounds(GetGrayRgn(), (Rect *)r);
}

#define openOnly 1
#define openPrint 2

LOCALFUNC bool GotRequiredParams(AppleEvent *theAppleEvent)
{
	DescType typeCode;
	Size actualSize;
	OSErr theErr;

	theErr = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr,
				typeWildCard, &typeCode, NULL, 0, &actualSize);
	if (errAEDescNotFound == theErr) { /* No more required params. */
		return true;
	} else if (noErr == theErr) { /* More required params! */
		return /* CheckSysCode(errAEEventNotHandled) */ false;
	} else { /* Unexpected Error! */
		return /* CheckSysCode(theErr) */ false;
	}
}

LOCALFUNC bool GotRequiredParams0(AppleEvent *theAppleEvent)
{
	DescType typeCode;
	Size actualSize;
	OSErr theErr;

	theErr = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr,
				typeWildCard, &typeCode, NULL, 0, &actualSize);
	if (errAEDescNotFound == theErr) { /* No more required params. */
		return true;
	} else if (noErr == theErr) { /* More required params! */
		return true; /* errAEEventNotHandled; */ /*^*/
	} else { /* Unexpected Error! */
		return /* CheckSysCode(theErr) */ false;
	}
}

/* call back */ static pascal OSErr OpenOrPrintFiles(
	AppleEvent *theAppleEvent, AppleEvent *reply, long aRefCon)
{
	/*
		Adapted from IM VI: AppleEvent Manager:
		Handling Required AppleEvents
	*/
	AEDescList docList;

	UnusedParam(reply);
	UnusedParam(aRefCon);
	/* put the direct parameter (a list of descriptors) into docList */
	if (noErr == (AEGetParamDesc(theAppleEvent, keyDirectObject,
		typeAEList, &docList)))
	{
		if (GotRequiredParams0(theAppleEvent)) {
			/* Check for missing required parameters */
			/* printIt = (openPrint == aRefCon) */
			ReportStandardOpenDiskError(
				InsertDisksFromDocList(&docList));
		}
		/* vCheckSysCode */ (void) (AEDisposeDesc(&docList));
	}

	return /* GetASysResultCode() */ 0;
}

/* call back */ static pascal OSErr DoOpenEvent(
	AppleEvent *theAppleEvent, AppleEvent *reply, long aRefCon)
/*
	This is the alternative to getting an
	open document event on startup.
*/
{
	UnusedParam(reply);
	UnusedParam(aRefCon);
	if (GotRequiredParams0(theAppleEvent)) {
	}
	return /* GetASysResultCode() */ 0;
		/* Make sure there are no additional "required" parameters. */
}


/* call back */ static pascal OSErr DoQuitEvent(
	AppleEvent *theAppleEvent, AppleEvent *reply, long aRefCon)
{
	UnusedParam(reply);
	UnusedParam(aRefCon);
	if (GotRequiredParams(theAppleEvent)) {
		RequestMacOff = true;
	}

	return /* GetASysResultCode() */ 0;
}

LOCALFUNC bool InstallEventHandler(AEEventClass theAEEventClass,
	AEEventID theAEEventID, ProcPtr p,
	long handlerRefcon, bool isSysHandler)
{
	return noErr == (AEInstallEventHandler(theAEEventClass,
		theAEEventID, NewAEEventHandlerUPP((AEEventHandlerProcPtr)p),
		handlerRefcon, isSysHandler));
}

LOCALPROC InstallAppleEventHandlers(void)
{
	if (noErr == AESetInteractionAllowed(kAEInteractWithLocal))
	if (InstallEventHandler(kCoreEventClass, kAEOpenApplication,
		(ProcPtr)DoOpenEvent, 0, false))
	if (InstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		(ProcPtr)OpenOrPrintFiles, openOnly, false))
	if (InstallEventHandler(kCoreEventClass, kAEPrintDocuments,
		(ProcPtr)OpenOrPrintFiles, openPrint, false))
	if (InstallEventHandler(kCoreEventClass, kAEQuitApplication,
		(ProcPtr)DoQuitEvent, 0, false))
	{
	}
}

static pascal OSErr GlobalTrackingHandler(DragTrackingMessage message,
	WindowRef theWindow, void *handlerRefCon, DragReference theDragRef)
{
	RgnHandle hilightRgn;
	Rect Bounds;

	UnusedParam(theWindow);
	UnusedParam(handlerRefCon);
	switch(message) {
		case kDragTrackingEnterWindow:
			hilightRgn = NewRgn();
			if (hilightRgn != NULL) {
				SetScrnRectFromCoords(&Bounds,
					0, 0, vMacScreenHeight, vMacScreenWidth);
				RectRgn(hilightRgn, &Bounds);
				ShowDragHilite(theDragRef, hilightRgn, true);
				DisposeRgn(hilightRgn);
			}
			break;
		case kDragTrackingLeaveWindow:
			HideDragHilite(theDragRef);
			break;
	}

	return noErr;
}

LOCALFUNC tMacErr InsertADiskOrAliasFromFSRef(FSRef *theRef)
{
	tMacErr err;
	Boolean isFolder;
	Boolean isAlias;

	if (CheckSaveMacErr(FSResolveAliasFile(theRef, true,
		&isFolder, &isAlias)))
	{
		err = InsertADiskFromFSRef1(theRef);
	}

	return err;
}

LOCALFUNC tMacErr InsertADiskOrAliasFromSpec(FSSpec *spec)
{
	tMacErr err;
	FSRef newRef;

	if (CheckSaveMacErr(FSpMakeFSRef(spec, &newRef)))
	if (CheckSavetMacErr(InsertADiskOrAliasFromFSRef(&newRef)))
	{
		/* ok */
	}

	return err;
}

static pascal OSErr GlobalReceiveHandler(WindowRef pWindow,
	void *handlerRefCon, DragReference theDragRef)
{
	SInt16 mouseUpModifiers;
	unsigned short items;
	unsigned short index;
	ItemReference theItem;
	Size SentSize;
	HFSFlavor r;

	UnusedParam(pWindow);
	UnusedParam(handlerRefCon);

	if (noErr == GetDragModifiers(theDragRef,
		NULL, NULL, &mouseUpModifiers))
	{
		UpdateKeyboardModifiers(mouseUpModifiers);
	}

	if (noErr == CountDragItems(theDragRef, &items)) {
		for (index = 1; index <= items; ++index) {
			if (noErr == GetDragItemReferenceNumber(theDragRef,
				index, &theItem))
			if (noErr == GetFlavorDataSize(theDragRef,
				theItem, flavorTypeHFS, &SentSize))
				/*
					On very old macs SentSize might only be big enough
					to hold the actual file name. Have not seen this
					in OS X, but still leave the check
					as '<=' instead of '=='.
				*/
			if (SentSize <= sizeof(HFSFlavor))
			if (noErr == GetFlavorData(theDragRef, theItem,
				flavorTypeHFS, (Ptr)&r, &SentSize, 0))
			{
				ReportStandardOpenDiskError(
					InsertADiskOrAliasFromSpec(&r.fileSpec));
			}
		}

#if 1
		if (gTrueBackgroundFlag) {
			ProcessSerialNumber currentProcess = {0, kCurrentProcess};

			(void) SetFrontProcess(&currentProcess);
		}
#endif
	}

	return noErr;
}

LOCALVAR DragTrackingHandlerUPP gGlobalTrackingHandler = NULL;
LOCALVAR DragReceiveHandlerUPP gGlobalReceiveHandler = NULL;

LOCALPROC UnPrepareForDragging(void)
{
	if (NULL != gGlobalReceiveHandler) {
		RemoveReceiveHandler(gGlobalReceiveHandler, gMainWindow);
		DisposeDragReceiveHandlerUPP(gGlobalReceiveHandler);
		gGlobalReceiveHandler = NULL;
	}
	if (NULL != gGlobalTrackingHandler) {
		RemoveTrackingHandler(gGlobalTrackingHandler, gMainWindow);
		DisposeDragTrackingHandlerUPP(gGlobalTrackingHandler);
		gGlobalTrackingHandler = NULL;
	}
}

LOCALFUNC bool PrepareForDragging(void)
{
	bool IsOk = false;

	gGlobalTrackingHandler = NewDragTrackingHandlerUPP(
		GlobalTrackingHandler);
	if (gGlobalTrackingHandler != NULL) {
		gGlobalReceiveHandler = NewDragReceiveHandlerUPP(
			GlobalReceiveHandler);
		if (gGlobalReceiveHandler != NULL) {
			if (noErr == InstallTrackingHandler(gGlobalTrackingHandler,
				gMainWindow, nil))
			{
				if (noErr == InstallReceiveHandler(
					gGlobalReceiveHandler, gMainWindow, nil))
				{
					IsOk = true;
				}
			}
		}
	}

	return IsOk;
}

LOCALPROC HandleEventLocation(EventRef theEvent)
{
	Point NewMousePos;

	if (! gWeAreActive) {
		return;
	}

	if (noErr == GetEventParameter(theEvent,
		kEventParamMouseLocation,
		typeQDPoint,
		NULL,
		sizeof(NewMousePos),
		NULL,
		&NewMousePos))
	{
		MousePositionNotify(NewMousePos);
	}
}

LOCALPROC HandleEventModifiers(EventRef theEvent)
{
	UInt32 theModifiers;

	if (gWeAreActive) {
		GetEventParameter(theEvent, kEventParamKeyModifiers,
			typeUInt32, NULL, sizeof(typeUInt32), NULL, &theModifiers);

		UpdateKeyboardModifiers(theModifiers);
	}
}

LOCALVAR bool IsOurMouseMove;

static pascal OSStatus windowEventHandler(
	EventHandlerCallRef nextHandler, EventRef theEvent,
	void* userData)
{
	OSStatus result = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(theEvent);
	UInt32 eventKind = GetEventKind(theEvent);

	UnusedParam(nextHandler);
	UnusedParam(userData);
	switch(eventClass) {
		case kEventClassWindow:
			switch(eventKind) {
				case kEventWindowClose:
					RequestMacOff = true;
					result = noErr;
					break;
				case kEventWindowDrawContent:
					Update_Screen();
					result = noErr;
					break;
				case kEventWindowClickContentRgn:
					MouseIsOutside = false;
					HandleEventLocation(theEvent);
					HandleEventModifiers(theEvent);
					MouseButtonSet(true);
					result = noErr;
					break;
				case kEventWindowFocusAcquired:
					gLackFocusFlag = false;
					result = noErr;
					break;
				case kEventWindowFocusRelinquish:
					{
						WindowRef window;

						(void) GetEventParameter(theEvent,
							kEventParamDirectObject, typeWindowRef,
							NULL, sizeof(WindowRef), NULL, &window);
						if ((window == gMainWindow)
							&& (window != gOldWindow))
						{
							ClearWeAreActive();
							gLackFocusFlag = true;
						}
					}
					result = noErr;
					break;
			}
			break;
		case kEventClassMouse:
			switch(eventKind) {
				case kEventMouseMoved:
				case kEventMouseDragged:
					MouseIsOutside = false;
#if 0 /* don't bother, CheckMouseState will take care of it, better */
					HandleEventLocation(theEvent);
					HandleEventModifiers(theEvent);
#endif
					IsOurMouseMove = true;
					result = noErr;
					break;
			}
			break;
	}

	return result;
}

LOCALFUNC bool CreateNewWindow(Rect *Bounds, WindowPtr *theWindow)
{
	WindowPtr ResultWin;
	bool IsOk = false;

	if (noErr == CreateNewWindow(

#if 1
		UseFullScreen ?
#endif
#if MayFullScreen
			/*
				appears not to work properly with aglSetWindowRef
				at least in OS X 10.5.5
				also doesn't seem to be needed. maybe dates from when
				didn't cover all screens in full screen mode.
			*/
			/*
				Oops, no, kDocumentWindowClass doesn't seem to
				work quite right if made immediately after
				launch, title bar gets moved onscreen. At least
				in Intel 10.5.6, doesn't happen in PowerPC 10.4.11.
				Oddly, this problem doesn't happen if icon
				already in dock before launch, but perhaps
				that is just a timing issue.
				So use kPlainWindowClass after all.
			*/
			kPlainWindowClass
#endif
#if 1
			:
#endif
#if MayNotFullScreen
			kDocumentWindowClass
#endif
		,

#if 1
		UseFullScreen ?
#endif
#if MayFullScreen
			/* kWindowStandardHandlerAttribute */ 0
#endif
#if 1
			:
#endif
#if MayNotFullScreen
			kWindowCloseBoxAttribute
				| kWindowCollapseBoxAttribute
#endif
		,

		Bounds, &ResultWin
	))
	{
#if 1
		if (! UseFullScreen)
#endif
#if MayNotFullScreen
		{
			SetWindowTitleWithCFString(ResultWin,
				AppName /* CFSTR("Mini vMac") */);
		}
#endif
		InstallStandardEventHandler(GetWindowEventTarget(ResultWin));
		{
			static const EventTypeSpec windowEvents[] =
				{
					{kEventClassWindow, kEventWindowClose},
					{kEventClassWindow, kEventWindowDrawContent},
					{kEventClassWindow, kEventWindowClickContentRgn},
					{kEventClassMouse, kEventMouseMoved},
					{kEventClassMouse, kEventMouseDragged},
					{kEventClassWindow, kEventWindowFocusAcquired},
					{kEventClassWindow, kEventWindowFocusRelinquish}
				};
			InstallWindowEventHandler(ResultWin,
				NewEventHandlerUPP(windowEventHandler),
				GetEventTypeCount(windowEvents),
				windowEvents, 0, NULL);
		}

		*theWindow = ResultWin;

		IsOk = true;
	}

	return IsOk;
}

LOCALPROC CloseAglCurrentContext(void)
{
	if (ctx != NULL) {
		/*
			Only because DrawWithOpenGL doesn't
			bother to do this. No one
			uses the CurrentContext
			without settting it first.
		*/
		if (GL_TRUE != aglSetCurrentContext(NULL)) {
			/* err = aglReportError() */
		}
		ctx = NULL;
	}
}

LOCALPROC CloseMainWindow(void)
{
	UnPrepareForDragging();

	if (window_ctx != NULL) {
		if (GL_TRUE != aglDestroyContext(window_ctx)) {
			/* err = aglReportError() */
		}
		window_ctx = NULL;
	}

	if (window_fmt != NULL) {
		aglDestroyPixelFormat(window_fmt);
		window_fmt = NULL;
	}

	if (gMainWindow != NULL) {
		DisposeWindow(gMainWindow);
		gMainWindow = NULL;
	}
}

enum {
	kMagStateNormal,
	kMagStateMagnifgy,
	kNumMagStates
};

#define kMagStateAuto kNumMagStates

#if MayNotFullScreen
LOCALVAR int CurWinIndx;
LOCALVAR bool HavePositionWins[kNumMagStates];
LOCALVAR Point WinPositionWins[kNumMagStates];
#endif

LOCALFUNC bool CreateMainWindow(void)
{
#if MayNotFullScreen
	int WinIndx;
#endif
	Rect MainScrnBounds;
	Rect AllScrnBounds;
	Rect NewWinRect;
	short leftPos;
	short topPos;
	short NewWindowHeight = vMacScreenHeight;
	short NewWindowWidth = vMacScreenWidth;
	bool IsOk = false;

#if 1
	if (UseFullScreen) {
		_HideMenuBar();
	} else {
		_ShowMenuBar();
	}
#else
#if MayFullScreen
	_HideMenuBar();
#endif
#endif

	GetGrayRgnBounds(&AllScrnBounds);
	GetScreenBitsBounds(&MainScrnBounds);

	if (UseMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}

	leftPos = MainScrnBounds.left
		+ ((MainScrnBounds.right - MainScrnBounds.left)
			- NewWindowWidth) / 2;
	topPos = MainScrnBounds.top
		+ ((MainScrnBounds.bottom - MainScrnBounds.top)
			- NewWindowHeight) / 2;
	if (leftPos < MainScrnBounds.left) {
		leftPos = MainScrnBounds.left;
	}
	if (topPos < MainScrnBounds.top) {
		topPos = MainScrnBounds.top;
	}

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		ViewHSize = MainScrnBounds.right - MainScrnBounds.left;
		ViewVSize = MainScrnBounds.bottom - MainScrnBounds.top;
		if (UseMagnify) {
			ViewHSize /= WindowScale;
			ViewVSize /= WindowScale;
		}
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
	}
#endif

	/* Create window rectangle and centre it on the screen */
	SetRect(&MainScrnBounds, 0, 0, NewWindowWidth, NewWindowHeight);
	OffsetRect(&MainScrnBounds, leftPos, topPos);

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewWinRect = AllScrnBounds;
	}
#endif
#if 1
	else
#endif
#if MayNotFullScreen
	{
		if (UseMagnify) {
			WinIndx = kMagStateMagnifgy;
		} else {
			WinIndx = kMagStateNormal;
		}

		if (! HavePositionWins[WinIndx]) {
			WinPositionWins[WinIndx].h = leftPos;
			WinPositionWins[WinIndx].v = topPos;
			HavePositionWins[WinIndx] = true;
			NewWinRect = MainScrnBounds;
		} else {
			SetRect(&NewWinRect, 0, 0, NewWindowWidth, NewWindowHeight);
			OffsetRect(&NewWinRect,
				WinPositionWins[WinIndx].h, WinPositionWins[WinIndx].v);
		}
	}
#endif

#if MayNotFullScreen
	CurWinIndx = WinIndx;
#endif

#if 1
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		hOffset = MainScrnBounds.left - AllScrnBounds.left;
		vOffset = MainScrnBounds.top - AllScrnBounds.top;
	}
#endif

	if (CreateNewWindow(&NewWinRect, &gMainWindow)) {
		static const GLint window_attrib[] = {AGL_RGBA,
#if UseAGLdoublebuff
			AGL_DOUBLEBUFFER,
#endif
			/* AGL_DEPTH_SIZE, 16,  */
			AGL_NONE};

#if 0 != vMacScreenDepth
		ColorModeWorks = true;
#endif

		window_fmt = aglChoosePixelFormat(NULL, 0, window_attrib);
		if (NULL == window_fmt) {
			/* err = aglReportError() */
		} else {
			window_ctx = aglCreateContext(window_fmt, NULL);
			if (NULL == window_ctx) {
				/* err = aglReportError() */
			} else {

				ShowWindow(gMainWindow);

				if (GL_TRUE != (
					/*
						aglSetDrawable is deprecated, but use it anyway
						if at all possible, because aglSetWindowRef
						doesn't seeem to work properly on a
						kPlainWindowClass window.
						Should move to Cocoa.
					*/
					HaveaglSetDrawable()
					? aglSetDrawable(window_ctx,
						GetWindowPort(gMainWindow))
					:
					HaveaglSetWindowRef()
					? aglSetWindowRef(window_ctx, gMainWindow)
					:
					GL_FALSE))
				{
					/* err = aglReportError() */
				} else {
					ctx = window_ctx;

#if 1
					if (UseFullScreen)
#endif
#if MayFullScreen
					{
						int h = NewWinRect.right - NewWinRect.left;
						int v = NewWinRect.bottom - NewWinRect.top;

						GLhOffset = hOffset;
						GLvOffset = v - vOffset;
						AdjustGLforSize(h, v);
					}
#endif
#if 1
					else
#endif
#if MayNotFullScreen
					{
						GLhOffset = 0;
						GLvOffset = NewWindowHeight;
						AdjustGLforSize(NewWindowWidth,
							NewWindowHeight);
					}
#endif

#if UseAGLdoublebuff && 1
					{
						GLint agSwapInterval = 1;
						if (GL_TRUE != aglSetInteger(
							window_ctx, AGL_SWAP_INTERVAL,
							&agSwapInterval))
						{
							MacMsg("oops", "aglSetInteger failed",
								false);
						} else {
							/*
								MacMsg("good", "aglSetInteger ok",
									false);
							*/
						}
					}
#endif

#if 1
					if (! UseFullScreen)
#endif
#if MayNotFullScreen
					{
						/* check if window rect valid */
						Rect tr;

						if (GetWindowTitleBounds(gMainWindow, &tr))
						{
							if (! RectInRgn(&tr, GetGrayRgn())) {
								SetMacWindContRect(gMainWindow,
									&MainScrnBounds);
								if (GetWindowTitleBounds(
									gMainWindow, &tr))
								{
									if (! RectInRgn(&tr, GetGrayRgn()))
									{
										OffsetRect(&MainScrnBounds, 0,
											AllScrnBounds.top - tr.top);
										SetMacWindContRect(
											gMainWindow,
											&MainScrnBounds);
									}
								}
							}
						}
					}
#endif

					(void) PrepareForDragging();

					IsOk = true;
				}
			}
		}
	}

	return IsOk;
}

#if EnableRecreateW
LOCALPROC ZapWState(void)
{
	gMainWindow = NULL;
	window_fmt = NULL;
	window_ctx = NULL;
	gGlobalReceiveHandler = NULL;
	gGlobalTrackingHandler = NULL;
}
#endif

#if EnableRecreateW
struct WState {
	WindowPtr f_MainWindow;
	AGLPixelFormat f_fmt;
	AGLContext f_ctx;
#if MayFullScreen
	short f_hOffset;
	short f_vOffset;
	uint16_t f_ViewHSize;
	uint16_t f_ViewVSize;
	uint16_t f_ViewHStart;
	uint16_t f_ViewVStart;
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
	short f_GLhOffset;
	short f_GLvOffset;
	DragTrackingHandlerUPP f_gGlobalTrackingHandler;
	DragReceiveHandlerUPP f_gGlobalReceiveHandler;
};
typedef struct WState WState;
#endif

#if EnableRecreateW
LOCALPROC GetWState(WState *r)
{
	r->f_MainWindow = gMainWindow;
	r->f_fmt = window_fmt;
	r->f_ctx = window_ctx;
#if MayFullScreen
	r->f_hOffset = hOffset;
	r->f_vOffset = vOffset;
	r->f_ViewHSize = ViewHSize;
	r->f_ViewVSize = ViewVSize;
	r->f_ViewHStart = ViewHStart;
	r->f_ViewVStart = ViewVStart;
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
	r->f_GLhOffset = GLhOffset;
	r->f_GLvOffset = GLvOffset;
	r->f_gGlobalTrackingHandler = gGlobalTrackingHandler;
	r->f_gGlobalReceiveHandler = gGlobalReceiveHandler;
}
#endif

#if EnableRecreateW
LOCALPROC SetWState(WState *r)
{
	gMainWindow = r->f_MainWindow;
	window_fmt = r->f_fmt;
	window_ctx = r->f_ctx;
#if MayFullScreen
	hOffset = r->f_hOffset;
	vOffset = r->f_vOffset;
	ViewHSize = r->f_ViewHSize;
	ViewVSize = r->f_ViewVSize;
	ViewHStart = r->f_ViewHStart;
	ViewVStart = r->f_ViewVStart;
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
	GLhOffset = r->f_GLhOffset;
	GLvOffset = r->f_GLvOffset;
	gGlobalTrackingHandler = r->f_gGlobalTrackingHandler;
	gGlobalReceiveHandler = r->f_gGlobalReceiveHandler;

	ctx = window_ctx;
}
#endif

#if EnableRecreateW
LOCALFUNC bool ReCreateMainWindow(void)
{
	WState old_state;
	WState new_state;

#if 1
	if (! UseFullScreen)
#endif
#if MayNotFullScreen
	{
		/* save old position */
		if (gMainWindow != NULL) {
			Rect r;

			if (GetWindowContBounds(gMainWindow, &r)) {
				WinPositionWins[CurWinIndx].h = r.left;
				WinPositionWins[CurWinIndx].v = r.top;
			}
		}
	}
#endif

#if MayFullScreen
	UngrabMachine();
#endif

	CloseAglCurrentContext();

	gOldWindow = gMainWindow;

	GetWState(&old_state);

	ZapWState();

#if 1
	UseFullScreen = WantFullScreen;
#endif
#if 1
	UseMagnify = WantMagnify;
#endif

	if (! CreateMainWindow()) {
		gOldWindow = NULL;
		CloseMainWindow();
		SetWState(&old_state);

#if 1
		if (UseFullScreen) {
			_HideMenuBar();
		} else {
			_ShowMenuBar();
		}
#endif

		/* avoid retry */
#if 1
		WantFullScreen = UseFullScreen;
#endif
#if 1
		WantMagnify = UseMagnify;
#endif

		return false;
	} else {
		GetWState(&new_state);
		SetWState(&old_state);
		CloseMainWindow();
		gOldWindow = NULL;
		SetWState(&new_state);

		if (HaveCursorHidden) {
			(void) MoveMouse(CurMouseH, CurMouseV);
			WantCursorHidden = true;
		} else {
			MouseIsOutside = false; /* don't know */
		}

		return true;
	}
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
LOCALVAR int WinMagStates[kNumWinStates];
#endif

LOCALPROC ZapWinStateVars(void)
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
LOCALPROC ToggleWantFullScreen(void)
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
			if (WantFullScreen
				&& HaveCGDisplayPixelsWide()
				&& HaveCGDisplayPixelsHigh())
			{
				CGDirectDisplayID CurMainDisplayID = MainDisplayID();

				if ((CGDisplayPixelsWide(CurMainDisplayID)
					>= vMacScreenWidth * WindowScale)
					&& (CGDisplayPixelsHigh(CurMainDisplayID)
					>= vMacScreenHeight * WindowScale)
					)
				{
					WantMagnify = true;
				}
			}
		}
	}
#endif
}
#endif

LOCALPROC LeaveSpeedStopped(void)
{
#if SoundEnabled
	Sound_Start();
#endif

	StartUpTimeAdjust();
}

LOCALPROC EnterSpeedStopped(void)
{
#if SoundEnabled
	Sound_Stop();
#endif
}

LOCALPROC DoNotInBackgroundTasks(void)
{
#if 0
	if (HaveCGCursorIsVisible()) {
		HaveCursorHidden = ! CGCursorIsVisible();

		/*
			This shouldn't be needed, but have seen
			cursor state get messed up in 10.4.
			If the os x hide count gets off, this
			should fix it within a few ticks.

			oops, no, doesn't seem to be accurate,
			and makes things worse. particularly if
			cursor in obscured state after typing
			into dialog.
		*/
		/* trying a different approach further below */
	}
#endif

	if (HaveCursorHidden != (
#if MayNotFullScreen
		(WantCursorHidden
#if 1
			|| UseFullScreen
#endif
		) &&
#endif
		gWeAreActive && ! CurSpeedStopped))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		if (HaveCursorHidden) {
			HideCursor();
		} else {
			/*
				kludge for OS X, where mouse over Dock devider
				changes cursor, and never sets it back.
			*/
			SetCursorArrow();

			ShowCursor();
		}
	}

	/* check if actual cursor visibility is what it should be */
	if (HaveCGCursorIsVisible()) {
		/* but only in OS X 10.3 and later */
		if (CGCursorIsVisible()) {
			if (HaveCursorHidden) {
				HideCursor();
				if (CGCursorIsVisible()) {
					/*
						didn't work, attempt undo so that
						hide cursor count won't get large
					*/
					ShowCursor();
				}
			}
		} else {
			if (! HaveCursorHidden) {
				ShowCursor();
				/*
					don't check if worked, assume can't decrement
					hide cursor count below 0
				*/
			}
		}
	}
}

LOCALPROC CheckForSavedTasks(void)
{
	if (EvtQNeedRecover) {
		EvtQNeedRecover = false;

		/* attempt cleanup, EvtQNeedRecover may get set again */
		EvtQTryRecoverFromFull();
	}

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MouseConstrain();
	}
#endif

	if (RequestMacOff) {
		RequestMacOff = false;
		if (AnyDiskInserted()) {
			MacMsgOverride(kStrQuitWarningTitle,
				kStrQuitWarningMessage);
		} else {
			ForceMacOff = true;
		}
	}

	if (! gWeAreActive) {
		if (! (gTrueBackgroundFlag || gLackFocusFlag)) {
			gWeAreActive = true;
			ReconnectKeyCodes3();
			SetCursorArrow();
		}
	}

#if 1
	if (gTrueBackgroundFlag && WantFullScreen) {
		ToggleWantFullScreen();
	}
#endif

	if (CurSpeedStopped != (SpeedStopped ||
		(gTrueBackgroundFlag && ! RunInBackground)))
	{
		CurSpeedStopped = ! CurSpeedStopped;
		if (CurSpeedStopped) {
			EnterSpeedStopped();
		} else {
			LeaveSpeedStopped();
		}
	}

#if EnableRecreateW
	if (gWeAreActive) {
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
	}
#endif

#if MayFullScreen
	if (GrabMachine != (
#if 1
		UseFullScreen &&
#endif
		gWeAreActive && ! CurSpeedStopped))
	{
		GrabMachine = ! GrabMachine;
		AdjustMachineGrab();
	}
#endif

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

	if (WantScreensChangedCheck) {
		WantScreensChangedCheck = false;
		UpdateOpenGLContext();
	}

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}

	if (! gWeAreActive) {
		/*
			dialog during drag and drop hangs if in background
				and don't want recursive dialogs
			so wait til later to display dialog
		*/
	} else {
#if IncludeSonyNew
		if (vSonyNewDiskWanted) {
#if IncludeSonyNameNew
			if (vSonyNewDiskName != NotAPbuf) {
				CFStringRef NewDiskName =
					CFStringCreateWithPbuf(vSonyNewDiskName);
				MakeNewDisk(vSonyNewDiskSize, NewDiskName);
				if (NewDiskName != NULL) {
					CFRelease(NewDiskName);
				}
				PbufDispose(vSonyNewDiskName);
				vSonyNewDiskName = NotAPbuf;
			} else
#endif
			{
				MakeNewDisk(vSonyNewDiskSize, NULL);
			}
			vSonyNewDiskWanted = false;
				/* must be done after may have gotten disk */
		}
#endif
		if (RequestInsertDisk) {
			RequestInsertDisk = false;
			InsertADisk0();
		}
	}

#if NeedRequestIthDisk
	if (0 != RequestIthDisk) {
		Sony_InsertIth(RequestIthDisk);
		RequestIthDisk = 0;
	}
#endif

	if (gWeAreActive) {
		DoNotInBackgroundTasks();
	}
}

#define CheckItem CheckMenuItem

/* Menu Constants */

#define kAppleMenu   128
#define kFileMenu    129
#define kSpecialMenu 130

enum {
	kCmdIdNull,
	kCmdIdMoreCommands,

	kNumCmdIds
};

LOCALFUNC OSStatus ProcessCommand(MenuCommand inCommandID)
{
	OSStatus result = noErr;

	switch (inCommandID) {
		case kHICommandAbout:
			DoAboutMsg();
			break;
		case kHICommandQuit:
			RequestMacOff = true;
			break;
		case kHICommandOpen:
			RequestInsertDisk = true;
			break;
		case kCmdIdMoreCommands:
			DoMoreCommandsMsg();
			break;
		default:
			result = eventNotHandledErr;
			break;
	}

	return result;
}

LOCALFUNC OSStatus Keyboard_UpdateKeyMap3(EventRef theEvent, bool down)
{
	UInt32 uiKeyCode;

	HandleEventModifiers(theEvent);
	GetEventParameter(theEvent, kEventParamKeyCode, typeUInt32, NULL,
		sizeof(uiKeyCode), NULL, &uiKeyCode);
	Keyboard_UpdateKeyMap2(Keyboard_RemapMac(uiKeyCode & 0x000000FF),
		down);
	return noErr;
}

static pascal OSStatus EventHandler(EventHandlerCallRef nextHandler,
	EventRef theEvent, void * userData)
{
	OSStatus result = eventNotHandledErr;
	UInt32 eventClass = GetEventClass(theEvent);
	UInt32 eventKind = GetEventKind(theEvent);

	UnusedParam(userData);

	switch (eventClass) {
		case kEventClassMouse:
			switch (eventKind) {
				case kEventMouseDown:
#if MayFullScreen
					if (GrabMachine) {
						HandleEventLocation(theEvent);
						HandleEventModifiers(theEvent);
						MouseButtonSet(true);
						result = noErr;
					} else
#endif
					{
						result = CallNextEventHandler(
							nextHandler, theEvent);
					}
					break;
				case kEventMouseUp:
					HandleEventLocation(theEvent);
					HandleEventModifiers(theEvent);
					MouseButtonSet(false);
#if MayFullScreen
					if (GrabMachine) {
						result = noErr;
					} else
#endif
					{
						result = CallNextEventHandler(
							nextHandler, theEvent);
					}
					break;
				case kEventMouseMoved:
				case kEventMouseDragged:
					IsOurMouseMove = false;
					result = CallNextEventHandler(
						nextHandler, theEvent);
						/*
							Actually, mousing over window does't seem
							to go through here, it goes directly to
							the window routine. But since not documented
							either way, leave the check in case this
							changes.
						*/
					if (! IsOurMouseMove) {
						MouseIsOutside = true;
#if 0 /* don't bother, CheckMouseState will take care of it, better */
						HandleEventLocation(theEvent);
						HandleEventModifiers(theEvent);
#endif
					}
					break;
			}
			break;
		case kEventClassApplication:
			switch (eventKind) {
				case kEventAppActivated:
					gTrueBackgroundFlag = false;
					result = noErr;
					break;
				case kEventAppDeactivated:
					gTrueBackgroundFlag = true;
					result = noErr;
					ClearWeAreActive();
					break;
			}
			break;
		case kEventClassCommand:
			switch (eventKind) {
				case kEventProcessCommand:
					{
						HICommand hiCommand;

						GetEventParameter(theEvent,
							kEventParamDirectObject, typeHICommand,
							NULL, sizeof(HICommand), NULL, &hiCommand);
						result = ProcessCommand(hiCommand.commandID);
					}
					break;
			}
			break;
		case kEventClassKeyboard:
			if (! gWeAreActive) {
				return CallNextEventHandler(nextHandler, theEvent);
			} else {
				switch (eventKind) {
					case kEventRawKeyDown:
						result = Keyboard_UpdateKeyMap3(
							theEvent, true);
						break;
					case kEventRawKeyUp:
						result = Keyboard_UpdateKeyMap3(
							theEvent, false);
						break;
					case kEventRawKeyModifiersChanged:
						HandleEventModifiers(theEvent);
						result = noErr;
						break;
					default:
						break;
				}
			}
			break;
		default:
			break;
	}
	return result;
}

LOCALPROC AppendMenuConvertCStr(
	MenuRef menu,
	MenuCommand inCommandID,
	char *s)
{
	CFStringRef cfStr = CFStringCreateFromSubstCStr(s);
	if (NULL != cfStr) {
		AppendMenuItemTextWithCFString(menu, cfStr,
			0, inCommandID, NULL);
		CFRelease(cfStr);
	}
}

LOCALPROC AppendMenuSep(MenuRef menu)
{
	AppendMenuItemTextWithCFString(menu, NULL,
		kMenuItemAttrSeparator, 0, NULL);
}

LOCALFUNC MenuRef NewMenuFromConvertCStr(short menuID, char *s)
{
	MenuRef menu = NULL;

	CFStringRef cfStr = CFStringCreateFromSubstCStr(s);
	if (NULL != cfStr) {
		OSStatus err = CreateNewMenu(menuID, 0, &menu);
		if (err != noErr) {
			menu = NULL;
		} else {
			(void) SetMenuTitleWithCFString(menu, cfStr);
		}
		CFRelease(cfStr);
	}

	return menu;
}

LOCALPROC RemoveCommandKeysInMenu(MenuRef theMenu)
{
	MenuRef outHierMenu;
	int i;
	UInt16 n = CountMenuItems(theMenu);

	for (i = 1; i <= n; ++i) {
		SetItemCmd(theMenu, i, 0);
		if (noErr == GetMenuItemHierarchicalMenu(
			theMenu, i, &outHierMenu)
			&& (NULL != outHierMenu))
		{
			RemoveCommandKeysInMenu(outHierMenu);
		}
	}
}

LOCALFUNC bool InstallOurMenus(void)
{
	MenuRef menu;
	Str255 s;

	PStrFromChar(s, (char)20);
	menu = NewMenu(kAppleMenu, s);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kHICommandAbout,
			kStrMenuItemAbout);
		AppendMenuSep(menu);
		InsertMenu(menu, 0);
	}

	menu = NewMenuFromConvertCStr(kFileMenu, kStrMenuFile);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kHICommandOpen,
			kStrMenuItemOpen ";ll");
		InsertMenu(menu, 0);
	}

	menu = NewMenuFromConvertCStr(kSpecialMenu, kStrMenuSpecial);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kCmdIdMoreCommands,
			kStrMenuItemMore ";ll");
		InsertMenu(menu, 0);
	}

	{
		MenuRef outMenu;
		MenuItemIndex outIndex;

		if (noErr == GetIndMenuItemWithCommandID(
			NULL, kHICommandQuit, 1, &outMenu, &outIndex))
		{
			RemoveCommandKeysInMenu(outMenu);
		}
	}

	DrawMenuBar();

	return true;
}

LOCALFUNC bool InstallOurAppearanceClient(void)
{
	RegisterAppearanceClient(); /* maybe not needed ? */
	return true;
}

LOCALVAR bool DisplayRegistrationCallBackSuccessful = false;

static void DisplayRegisterReconfigurationCallback(
	CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags,
	void *userInfo)
{
	UnusedParam(display);
	UnusedParam(userInfo);

	if (0 != (flags & kCGDisplayBeginConfigurationFlag)) {
		/* fprintf(stderr, "kCGDisplayBeginConfigurationFlag\n"); */
	} else {
#if 0
		if (0 != (flags & kCGDisplayMovedFlag)) {
			fprintf(stderr, "kCGDisplayMovedFlag\n");
		}
		if (0 != (flags & kCGDisplaySetMainFlag)) {
			fprintf(stderr, "kCGDisplaySetMainFlag\n");
		}
		if (0 != (flags & kCGDisplaySetModeFlag)) {
			fprintf(stderr, "kCGDisplaySetModeFlag\n");
		}

		if (0 != (flags & kCGDisplayAddFlag)) {
			fprintf(stderr, "kCGDisplayAddFlag\n");
		}
		if (0 != (flags & kCGDisplayRemoveFlag)) {
			fprintf(stderr, "kCGDisplayRemoveFlag\n");
		}

		if (0 != (flags & kCGDisplayEnabledFlag)) {
			fprintf(stderr, "kCGDisplayEnabledFlag\n");
		}
		if (0 != (flags & kCGDisplayDisabledFlag)) {
			fprintf(stderr, "kCGDisplayDisabledFlag\n");
		}

		if (0 != (flags & kCGDisplayMirrorFlag)) {
			fprintf(stderr, "kCGDisplayMirrorFlag\n");
		}
		if (0 != (flags & kCGDisplayUnMirrorFlag)) {
			fprintf(stderr, "kCGDisplayMirrorFlag\n");
		}
#endif

		WantScreensChangedCheck = true;

#if 1
		if (WantFullScreen) {
			ToggleWantFullScreen();
		}
#endif
	}
}

LOCALFUNC bool InstallOurEventHandlers(void)
{
	EventTypeSpec eventTypes[] = {
		{kEventClassMouse, kEventMouseDown},
		{kEventClassMouse, kEventMouseUp},
		{kEventClassMouse, kEventMouseMoved},
		{kEventClassMouse, kEventMouseDragged},
		{kEventClassKeyboard, kEventRawKeyModifiersChanged},
		{kEventClassKeyboard, kEventRawKeyDown},
		{kEventClassKeyboard, kEventRawKeyUp},
		{kEventClassApplication, kEventAppActivated},
		{kEventClassApplication, kEventAppDeactivated},
		{kEventClassCommand, kEventProcessCommand}
	};

	InstallApplicationEventHandler(
		NewEventHandlerUPP(EventHandler),
		GetEventTypeCount(eventTypes),
		eventTypes, NULL, NULL);

	InitKeyCodes();

	InstallAppleEventHandlers();

	if (HaveCGDisplayRegisterReconfigurationCallback()) {
		if (kCGErrorSuccess ==
			CGDisplayRegisterReconfigurationCallback(
				DisplayRegisterReconfigurationCallback, NULL))
		{
			DisplayRegistrationCallBackSuccessful = true;
		}
	}

	/* (void) SetMouseCoalescingEnabled(false, NULL); */

	return true;
}

LOCALPROC UnInstallOurEventHandlers(void)
{
	if (DisplayRegistrationCallBackSuccessful) {
		if (HaveCGDisplayRemoveReconfigurationCallback()) {
			CGDisplayRemoveReconfigurationCallback(
				DisplayRegisterReconfigurationCallback, NULL);
		}
	}
}

GLOBALOSGLUPROC WaitForNextTick(void)
{
	OSStatus err;
	EventRef theEvent;
	uint8_t NumChecks;
	EventTimeout inTimeout;
	EventTargetRef theTarget = GetEventDispatcherTarget();

	inTimeout = kEventDurationNoWait;

label_retry:
	NumChecks = 0;
	while ((NumChecks < 32) && (noErr == (err =
		ReceiveNextEvent(0, NULL, inTimeout,
			true, &theEvent))))
	{
		(void) SendEventToEventTarget(theEvent, theTarget);
		ReleaseEvent(theEvent);
		inTimeout = kEventDurationNoWait;
		++NumChecks;
	}

	CheckForSavedTasks();

	if (ForceMacOff) {
		return;
	}

	if (CurSpeedStopped) {
		DoneWithDrawingForTick();
		inTimeout = kEventDurationForever;
		goto label_retry;
	}

	if (ExtraTimeNotOver()) {
		inTimeout =
			NextTickChangeTime - GetCurrentEventTime();
		if (inTimeout > 0.0) {
#if 1
			struct timespec rqt;
			struct timespec rmt;

			rqt.tv_sec = 0;
			rqt.tv_nsec = inTimeout / kEventDurationNanosecond;
			(void) nanosleep(&rqt, &rmt);
			inTimeout = kEventDurationNoWait;
			goto label_retry;
#elif 1
			usleep(inTimeout / kEventDurationMicrosecond);
			inTimeout = kEventDurationNoWait;
			goto label_retry;
#else
			/*
				This has higher overhead.
			*/
			goto label_retry;
#endif
		}
	}

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	if (gWeAreActive) {
		CheckMouseState();
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif
}

/* --- platform independent code can be thought of as going here --- */

#include "PROGMAIN.h"

LOCALPROC ReserveAllocAll(void)
{
#if dbglog_HAVE
	dbglog_ReserveAlloc();
#endif
	ReserveAllocOneBlock(&ROM, kROM_Size, 5, false);
	ReserveAllocOneBlock(&screencomparebuff,
		vMacScreenNumBytes, 5, true);
	ReserveAllocOneBlock(&CntrlDisplayBuff,
		vMacScreenNumBytes, 5, false);
	ReserveAllocOneBlock(&ScalingBuff, vMacScreenNumPixels
#if 0 != vMacScreenDepth
		* 4
#endif
		, 5, false);
	ReserveAllocOneBlock(&CLUT_final, CLUT_finalsz, 5, false);
#if SoundEnabled
	ReserveAllocOneBlock((uint8_t * *)&TheSoundBuffer,
		dbhBufferSize, 5, false);
#endif

	EmulationReserveAlloc();
}

LOCALFUNC bool AllocMemory(void)
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

LOCALFUNC bool InitOSGLU(void)
{
	if (AllocMemory())
	if (InitApplInfo())
#if dbglog_HAVE
	if (dbglog_open())
#endif
#if SoundEnabled
	if (Sound_Init())
#endif
	if (InstallOurAppearanceClient())
	if (InstallOurEventHandlers())
	if (InstallOurMenus())
	if (CreateMainWindow())
	if (LoadMacRom())
	if (LoadInitialImages())
#if EmLocalTalk
	if (InitLocalTalk())
#endif
	if (InitLocationDat())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

#if dbglog_HAVE && 0
IMPORTPROC DoDumpTable(void);
#endif
#if dbglog_HAVE && 0
IMPORTPROC DumpRTC(void);
#endif

LOCALPROC UnInitOSGLU(void)
{
#if dbglog_HAVE && 0
	DoDumpTable();
#endif
#if dbglog_HAVE && 0
	DumpRTC();
#endif

	if (MacMsgDisplayed) {
		MacMsgDisplayOff();
	}

#if MayFullScreen
	UngrabMachine();
#endif

#if SoundEnabled
	Sound_Stop();
#endif

	CloseAglCurrentContext();
	CloseMainWindow();

#if MayFullScreen
	_ShowMenuBar();
#endif

#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	if (! gTrueBackgroundFlag) {
		CheckSavedMacMsg();
	}

	UnInstallOurEventHandlers();

#if dbglog_HAVE
	dbglog_close();
#endif

	UnInitApplInfo();

	ForceShowCursor();
}

LOCALPROC ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
}

/* adapted from Apple "Technical Q&A QA1061" */

static pascal OSStatus EventLoopEventHandler(
	EventHandlerCallRef inHandlerCallRef, EventRef inEvent,
	void *inUserData)
/*
	This code contains the standard Carbon event dispatch loop,
	as per "Inside Macintosh: Handling Carbon Events", Listing 3-10,
	except:

	o this loop supports yielding to cooperative threads based on the
		application maintaining the gNumberOfRunningThreads global
		variable, and

	o it also works around a problem with the Inside Macintosh code
		which unexpectedly quits when run on traditional Mac OS 9.

	See RunApplicationEventLoopWithCooperativeThreadSupport for
	an explanation of why this is inside a Carbon event handler.

	The code in Inside Mac has a problem in that it quits the
	event loop when ReceiveNextEvent returns an error.  This is
	wrong because ReceiveNextEvent can return eventLoopQuitErr
	when you call WakeUpProcess on traditional Mac OS.  So, rather
	than relying on an error from ReceiveNextEvent, this routine tracks
	whether the application is really quitting by installing a
	customer handler for the kEventClassApplication/kEventAppQuit
	Carbon event.  All the custom handler does is call through
	to the previous handler and, if it returns noErr (which indicates
	the application is quitting, it sets quitNow so that our event
	loop quits.

	Note that this approach continues to support
	QuitApplicationEventLoop, which is a simple wrapper that just posts
	a kEventClassApplication/kEventAppQuit event to the event loop.
*/
{
	UnusedParam(inHandlerCallRef);
	UnusedParam(inEvent);
	UnusedParam(inUserData);

	ProgramMain();
	QuitApplicationEventLoop();

	return noErr;
}

LOCALPROC RunApplicationEventLoopWithCooperativeThreadSupport(void)
/*
	A reimplementation of RunApplicationEventLoop that supports
	yielding time to cooperative threads.
*/
{
	static const EventTypeSpec eventSpec = {'KWIN', 'KWIN'};
	EventHandlerUPP theEventLoopEventHandlerUPP = nil;
	EventHandlerRef installedHandler = NULL;
	EventRef dummyEvent = nil;

/*
	Install EventLoopEventHandler, create a dummy event and post it,
	and then call RunApplicationEventLoop.  The rationale for this
	is as follows:  We want to unravel RunApplicationEventLoop so
	that we can can yield to cooperative threads.  In fact, the
	core code for RunApplicationEventLoop is pretty easy (you
	can see it above in EventLoopEventHandler).  However, if you
	just execute this code you miss out on all the standard event
	handlers.  These are relatively easy to reproduce (handling
	the quit event and so on), but doing so is a pain because
	a) it requires a bunch boilerplate code, and b) if Apple
	extends the list of standard event handlers, your application
	wouldn't benefit.  So, we execute our event loop from within
	a Carbon event handler that we cause to be executed by
	explicitly posting an event to our event loop.  Thus, the
	standard event handlers are installed while our event loop runs.
*/
	if (nil == (theEventLoopEventHandlerUPP
		= NewEventHandlerUPP(EventLoopEventHandler)))
	{
		/* fail */
	} else
	if (noErr != InstallEventHandler(GetApplicationEventTarget(),
		theEventLoopEventHandlerUPP, 1, &eventSpec, nil,
		&installedHandler))
	{
		/* fail */
	} else
	if (noErr != MacCreateEvent(nil, 'KWIN', 'KWIN',
		GetCurrentEventTime(), kEventAttributeNone,
		&dummyEvent))
	{
		/* fail */
	} else
	if (noErr != PostEventToQueue(GetMainEventQueue(),
		dummyEvent, kEventPriorityHigh))
	{
		/* fail */
	} else
	{
		RunApplicationEventLoop();
	}

	if (nil != dummyEvent) {
		ReleaseEvent(dummyEvent);
	}

	if (NULL != installedHandler) {
		(void) RemoveEventHandler(installedHandler);
	}
}

int main(void)
{
	ZapOSGLUVars();
	if (InitOSGLU()) {
		RunApplicationEventLoopWithCooperativeThreadSupport();
	}
	UnInitOSGLU();

	return 0;
}
