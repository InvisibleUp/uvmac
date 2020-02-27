/*
	OSGLUMAC.c

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
	Operating System GLUe for MACintosh

	All operating system dependent code for the
	Macintosh (pre OS X) platform should go here.

	This is also the "reference" implementation. General
	comments about what the platform dependent code
	does should go here, and not be repeated for each
	platform.

	This code is descended from Richard F. Bannister's Macintosh
	port of vMac, by Philip Cummins.

	The main entry point 'main' is at the end of this file.
*/

#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "ENDIANAC.h"

#include "UI/MYOSGLUE.h"

#include "STRCONST.h"

#ifndef NavigationAvail
#define NavigationAvail 1
#endif

#ifndef AppearanceAvail
#define AppearanceAvail 1
#endif

#ifndef NewNamesAvail
#define NewNamesAvail 1
#endif

#ifndef CALL_NOT_IN_CARBON
#define CALL_NOT_IN_CARBON 1
#endif /* !defined(CALL_NOT_IN_CARBON) */


/* --- information about the environment --- */

/*
	code to determine characteristics
	of the current run time environment.
*/

#define TestBit(i, p) (((uimr)(i) & ((uimr)1 << (p))) != 0)

#ifndef HaveCPUfamM68K
#define HaveCPUfamM68K 0
#endif

#if HaveCPUfamM68K

/* MACENVRN.i */

#ifndef Support64kROM
#define Support64kROM 1
#endif

#define Have64kROM() (LMGetROM85() < 0)

/* -- basic environment checking -- */

#ifndef DebugTrapAvailableChecks
#define DebugTrapAvailableChecks 0
#endif

LOCALFUNC bool LowToolTrapAvailable(short trap_num)
{
#if DebugTrapAvailableChecks
	if ((trap_num & 0x0800) == 0) {
		DebugStr("\pOSTrap in LowToolTrapAvailable");
		return false;
	}
	if ((trap_num & 0x07ff) >= 0x200) {
		DebugStr("\pHiToolTrap in LowToolTrapAvailable");
		return false;
	}
#endif
	return GetToolTrapAddress(trap_num) !=
		GetToolTrapAddress(_Unimplemented);
}

LOCALFUNC bool HiToolTrapAvailable(short trap_num)
{
#if DebugTrapAvailableChecks
	if ((trap_num & 0x0800) == 0) {
		DebugStr("\pOSTrap in HiToolTrapAvailable");
		return false;
	}
	if (((trap_num & 0x07ff) < 0x200)
		|| ((trap_num & 0x07ff) >= 0x400))
	{
		DebugStr("\pLowToolTrap in HiToolTrapAvailable");
		return false;
	}
#endif
	if (GetToolTrapAddress(_InitGraf)
		== GetToolTrapAddress(0xAA6E))
	{
		return false;
	} else {
		return GetToolTrapAddress(trap_num) !=
			GetToolTrapAddress(_Unimplemented);
	}
}

LOCALFUNC bool OSTrapAvailable(short trap_num)
{
#if DebugTrapAvailableChecks
	if ((trap_num & 0x0800) != 0) {
		DebugStr("\pToolTrap in ToolTrapAvailable");
		return false;
	}
#endif
	return GetOSTrapAddress(trap_num) !=
		GetToolTrapAddress(_Unimplemented);
}

LOCALVAR bool EnvrAttrWaitNextEventAvail;
LOCALVAR bool HaveEnvrAttrWaitNextEventAvail = false;

LOCALFUNC bool HaveWaitNextEventAvail(void)
{
	if (! HaveEnvrAttrWaitNextEventAvail) {
		EnvrAttrWaitNextEventAvail =
			LowToolTrapAvailable(_WaitNextEvent);
		HaveEnvrAttrWaitNextEventAvail = true;
	}
	return EnvrAttrWaitNextEventAvail;
}

LOCALVAR bool EnvrAttrGestaltAvail;
LOCALVAR bool HaveEnvrAttrGestaltAvail = false;

LOCALFUNC bool HaveGestaltAvail(void)
{
	if (! HaveEnvrAttrGestaltAvail) {
		EnvrAttrGestaltAvail =
			(! Have64kROM()) &&
			OSTrapAvailable(_Gestalt);
			/*
				contrary to all the documentation,
				TrapAvailable check by itself doesn't
				work on 64k ROM, because a tool box trap
				has the same trap number, and both
				GetOSTrapAddress and GetToolTrapAddress
				are the same as GetTrapAddress on a 64k ROM.
			*/
		HaveEnvrAttrGestaltAvail = true;
	}
	return EnvrAttrGestaltAvail;
}

#else

/* for PowerPC, assume always have these routines */

#define HaveWaitNextEventAvail() true
#define HaveGestaltAvail() true

#endif

LOCALVAR bool EnvrAttrAliasMgrAvail;
LOCALVAR bool HaveEnvrAttrAliasMgrAvail = false;

LOCALFUNC bool HaveAliasMgrAvail(void)
{
	if (! HaveEnvrAttrAliasMgrAvail) {
		long reply;

		EnvrAttrAliasMgrAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltAliasMgrAttr, &reply))
			&& TestBit(reply, gestaltAliasMgrPresent);
		HaveEnvrAttrAliasMgrAvail = true;
	}
	return EnvrAttrAliasMgrAvail;
}

LOCALVAR bool EnvrAttrAppleEvtMgrAvail;
LOCALVAR bool HaveEnvrAttrAppleEvtMgrAvail = false;

LOCALFUNC bool HaveAppleEvtMgrAvail(void)
{
	if (! HaveEnvrAttrAppleEvtMgrAvail) {
		long reply;

		EnvrAttrAppleEvtMgrAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltAppleEventsAttr, &reply))
			&& TestBit(reply, gestaltAppleEventsPresent);
		HaveEnvrAttrAppleEvtMgrAvail = true;
	}
	return EnvrAttrAppleEvtMgrAvail;
}

#if EnableDragDrop

LOCALVAR bool EnvrAttrDragMgrAvail;
LOCALVAR bool HaveEnvrAttrDragMgrAvail = false;

LOCALFUNC bool HaveDragMgrAvail(void)
{
	if (! HaveEnvrAttrDragMgrAvail) {
		long reply;

		EnvrAttrDragMgrAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltDragMgrAttr, &reply))
			&& TestBit(reply, gestaltDragMgrPresent);
		HaveEnvrAttrDragMgrAvail = true;
	}
	return EnvrAttrDragMgrAvail;
}

#endif

#ifndef Windows85APIAvail
#define Windows85APIAvail 1
#endif

#if Windows85APIAvail
LOCALVAR bool EnvrAttrHideShowMenuAvail;
LOCALVAR bool HaveEnvrAttrHideShowMenuAvail = false;

LOCALFUNC bool HaveHideShowMenuAvail(void)
{
	if (! HaveEnvrAttrHideShowMenuAvail) {
		long reply;

		EnvrAttrHideShowMenuAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltMenuMgrAttr, &reply));
		HaveEnvrAttrHideShowMenuAvail = true;
	}
	return EnvrAttrHideShowMenuAvail;
}
#endif

#if AppearanceAvail
LOCALVAR bool EnvrAttrAppearanceAvail;
LOCALVAR bool HaveEnvrAttrAppearanceAvail = false;

LOCALFUNC bool HaveAppearanceAvail(void)
{
	if (! HaveEnvrAttrAppearanceAvail) {
		long reply;

		EnvrAttrAppearanceAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltAppearanceAttr, &reply));
		HaveEnvrAttrAppearanceAvail = true;
	}
	return EnvrAttrAppearanceAvail;
}
#endif

#if HaveCPUfamM68K
LOCALVAR bool EnvrAttrSndMngrAvail;
LOCALVAR bool HaveEnvrAttrSndMngrAvail = false;

LOCALFUNC bool HaveSndMngrAvail(void)
{
	if (! HaveEnvrAttrSndMngrAvail) {
		EnvrAttrSndMngrAvail = LowToolTrapAvailable(_SndNewChannel);
		HaveEnvrAttrSndMngrAvail = true;
	}
	return EnvrAttrSndMngrAvail;
}
#endif

#if NavigationAvail
LOCALVAR bool EnvrAttrNavServicesAvail;
LOCALVAR bool HaveEnvrAttrNavServicesAvail = false;

LOCALFUNC bool HaveNavServicesAvail(void)
{
	if (! HaveEnvrAttrNavServicesAvail) {
		EnvrAttrNavServicesAvail = NavServicesAvailable();
		HaveEnvrAttrNavServicesAvail = true;
	}
	return EnvrAttrNavServicesAvail;
}
#endif

#if HaveCPUfamM68K
LOCALVAR bool EnvrAttrFSSpecCallsAvail;
LOCALVAR bool HaveEnvrAttrFSSpecCallsAvail = false;

LOCALFUNC bool HaveFSSpecCallsAvail(void)
{
	if (! HaveEnvrAttrFSSpecCallsAvail) {
		long reply;

		EnvrAttrFSSpecCallsAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltFSAttr, &reply))
			&& TestBit(reply, gestaltHasFSSpecCalls);
		HaveEnvrAttrFSSpecCallsAvail = true;
	}
	return EnvrAttrFSSpecCallsAvail;
}
#endif

#if Windows85APIAvail
LOCALVAR bool EnvrAttrNewWndMgrAvail;
LOCALVAR bool HaveEnvrAttrNewWndMgrAvail = false;

LOCALFUNC bool HaveNewWndMgrAvail(void)
{
	if (! HaveEnvrAttrNewWndMgrAvail) {
		long reply;

		EnvrAttrNewWndMgrAvail =
			HaveGestaltAvail()
			&& (noErr == Gestalt(gestaltWindowMgrAttr, &reply))
			&& TestBit(reply, gestaltWindowMgrPresentBit);
		HaveEnvrAttrNewWndMgrAvail = true;
	}
	return EnvrAttrNewWndMgrAvail;
}
#endif

/* --- initial initialization --- */

#if defined(__SC__) || ((defined(powerc) || defined(__powerc)) \
	&& ! defined(__MWERKS__))

/* GLOBALVAR */ QDGlobals qd;
#endif

LOCALFUNC bool InitMacManagers(void)
{
	MaxApplZone();

	{
		int i;

		for (i = 7; --i >= 0; ) {
			MoreMasters();
		}
	}

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	return true;
}


/* --- mac style errors --- */

#define CheckSavetMacErr(result) (mnvm_noErr == (err = (result)))
	/*
		where 'err' is a variable of type tMacErr in the function
		this is used in
	*/

#define To_tMacErr(result) ((tMacErr)(uint16_t)(result))

#define CheckSaveMacErr(result) (CheckSavetMacErr(To_tMacErr(result)))


/*
	define NotAfileRef to some value that is different
	from any valid open file reference.
*/
#define NotAfileRef (-1)

struct Dir_R {
	long DirId;
	short VRefNum;
};
typedef struct Dir_R Dir_R;

LOCALFUNC tMacErr OpenNamedFileInFolder(Dir_R *d,
	ps3p fileName, short *refnum)
{
	tMacErr err;

#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		err = To_tMacErr(FSOpen(fileName, d->VRefNum, refnum));
	} else
#endif
	{
		Boolean isFolder;
		Boolean isAlias;
		FSSpec spec;

		if (CheckSaveMacErr(
			FSMakeFSSpec(d->VRefNum, d->DirId, fileName, &spec)))
		if (CheckSaveMacErr(
			ResolveAliasFile(&spec, true, &isFolder, &isAlias)))
		if (CheckSaveMacErr(
			FSpOpenDF(&spec, fsRdPerm, refnum)))
		{
		}
	}

	return err;
}

/* --- sending debugging info to file --- */

#if dbglog_HAVE

#define Support64kROM 0
#define tErr tMacErr

typedef unsigned char * Ptr;

LOCALFUNC tErr HGetDir_v2(Dir_R *d)
{
	tErr err;
	WDPBRec r;

	r.ioCompletion = NULL;
	r.ioNamePtr = NULL;

#if Support64kROM
	if (Have64kROM()) {
		err = PBGetVolSync((ParamBlockRec *)&r);
		d->VRefNum = r.ioVRefNum;
		d->DirId = 0;
	} else
#endif
	{
		err = PBHGetVolSync(&r);
		d->VRefNum = r.ioWDVRefNum;
		d->DirId = r.ioWDDirID;
	}

	return err;
}

LOCALFUNC tErr WriteBytes_v2(short refNum, Ptr p, uimr L)
{
	ParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioRefNum = refNum;
	r.ioParam.ioBuffer = (Ptr)p;
	r.ioParam.ioReqCount = L;
	r.ioParam.ioPosMode = (short) fsFromMark;
	r.ioParam.ioPosOffset = 0;

	return PBWriteSync(&r);
}

LOCALFUNC tErr CloseFile_v2(short refNum)
{
	ParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioRefNum = refNum;

	return PBCloseSync(&r);
#if 0
	return (tErr)FSClose(refNum);
#endif
}

#define NotAfileRef (-1)

/*
	Probably should use PBHOpenDF instead
	of PBHOpen when it is available.
	(System 7 according to Technical Note FL515)
*/

LOCALFUNC tErr FileOpen_v2(Dir_R *d, StringPtr s,
	char Permssn, short *refnum)
{
	tErr err;
	HParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = s;
	r.ioParam.ioVRefNum = d->VRefNum;
	r.ioParam.ioPermssn = Permssn;
	r.ioParam.ioMisc = 0; /* use volume buffer */
	r.ioParam.ioVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = PBOpenSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHOpenSync(&r);
	}

	if (noErr == err) {
		*refnum = r.ioParam.ioRefNum;
		/*
			Don't change *refnum unless file opened,
			so can initialize to NotAfileRef, and
			compare later before closing in uninit.
		*/
	}
	return err;
}

LOCALFUNC tErr FileOpenWrite_v2(Dir_R *d, StringPtr s,
	short *refnum)
{
	return FileOpen_v2(d, s, (char)fsWrPerm, refnum);
}

LOCALFUNC tErr DeleteFile_v2(Dir_R *d, StringPtr s)
{
	tErr err;
	HParamBlockRec r;

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioNamePtr = s;
	r.fileParam.ioFVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = PBDeleteSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHDeleteSync(&r);
	}

	return err;
}

LOCALFUNC tErr CreateFile_v2(Dir_R *d, StringPtr s)
{
	tErr err;
	HParamBlockRec r;

	r.fileParam.ioFlVersNum = 0;
		/*
			Think reference says to do this,
			but not Inside Mac IV
		*/

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioNamePtr = s;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioFVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = PBCreateSync((ParamBlockRec *)&r);
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = PBHCreateSync(&r);
	}

	return err;
}

LOCALFUNC tErr FileGetInfo_v2(Dir_R *d, StringPtr s,
	HParamBlockRec *r)
{
	tErr err;

	r->fileParam.ioCompletion = NULL;
	r->fileParam.ioNamePtr = s;
	r->fileParam.ioVRefNum = d->VRefNum;
	r->fileParam.ioFVersNum = (char)0; /* needed if MFS volume */
	r->fileParam.ioFDirIndex = (short)0;

#if Support64kROM
	if (Have64kROM()) {
		err = PBGetFInfoSync((ParamBlockRec *)r);
	} else
#endif
	{
		r->fileParam.ioDirID = d->DirId;
		err = PBHGetFInfoSync(r);
	}

	return err;
}

LOCALFUNC tErr FileSetInfo_v2(Dir_R *d, StringPtr s,
	HParamBlockRec *r)
{
	tErr err;

	r->fileParam.ioCompletion = NULL;
	r->fileParam.ioNamePtr = s;
	r->fileParam.ioVRefNum = d->VRefNum;
	r->fileParam.ioFVersNum = (char)0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = PBSetFInfoSync((ParamBlockRec *)r);
	} else
#endif
	{
		r->fileParam.ioDirID = d->DirId;
		err = PBHSetFInfoSync(r);
	}

	return err;
}

LOCALFUNC tErr FileSetTypeCreator_v2(Dir_R *d, StringPtr s,
	OSType creator, OSType fileType)
{
	tErr err;
	HParamBlockRec r;

	if (noErr == (err = FileGetInfo_v2(d, s, &r))) {
		r.fileParam.ioFlFndrInfo.fdType = fileType;
		r.fileParam.ioFlFndrInfo.fdCreator = creator;
		err = FileSetInfo_v2(d, s, &r);
	}

	return err;
}

LOCALFUNC tErr CreateFileOverWrite_v2(Dir_R *d, StringPtr s)
{
	tErr err;

	err = CreateFile_v2(d, s);
	if (dupFNErr == err) {
		if (noErr == (err = DeleteFile_v2(d, s))) {
			err = CreateFile_v2(d, s);
		}
	}

	return err;
}

LOCALFUNC tErr OpenNewFile_v3(Dir_R *d, StringPtr s,
	OSType creator, OSType fileType,
	short *refnum)
/*
	Deletes old file if already exists.
*/
{
	tErr err;

	err = CreateFileOverWrite_v2(d, s);
	if (noErr == err) {
		err = FileSetTypeCreator_v2(d, s,
			creator, fileType);
		if (noErr == err) {
			err = FileOpenWrite_v2(d, s, refnum);
		}

		if (noErr != err) {
			(void) DeleteFile_v2(d, s);
				/* ignore any error, since already got one */
		}
	}

	return err;
}


LOCALVAR short dbglog_File = NotAfileRef;
LOCALVAR tErr dbglog_err = noErr;

LOCALFUNC bool dbglog_open0(void)
{
	tMacErr err;
	Dir_R d;

	if (noErr == (err = HGetDir_v2(&d))) {
		err = FileOpen_v2(&d, (StringPtr)"\pdbglog",
				(char)fsWrPerm, &dbglog_File);
		if (mnvm_noErr /* fnfErr */ == err) {
			err = SetEOF(dbglog_File, 0);
		} else {
			err = OpenNewFile_v3(&d, (StringPtr)"\pdbglog",
				'MPS ', 'TEXT', &dbglog_File);
			err = mnvm_noErr;
		}

	}

	return (mnvm_noErr == err);
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
	if (NotAfileRef != dbglog_File)
	if (noErr == dbglog_err)
	{
		dbglog_err = WriteBytes_v2(dbglog_File, (Ptr)s, L);
	}
}

LOCALPROC dbglog_close0(void)
{
	if (NotAfileRef != dbglog_File) {
		(void)  CloseFile_v2(dbglog_File);
		dbglog_File = NotAfileRef;
	}
}

#endif /* dbglog_HAVE */


/* --- control mode and internationalization --- */

#define NeedCell2MacAsciiMap 1

#define WantColorTransValid 1

#include "INTLCHAR.h"

#include "UI/COMOSGLU.h"

#define WantKeyboard_RemapMac 1

#include "CONTROLM.h"

/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	BlockMove((Ptr)srcPtr, (Ptr)destPtr, byteCount);
}

/* don't want to include c libraries, so: */
LOCALFUNC int32_t CStrLen(char *src)
{
	char *p = src;
	while (*p++ != 0) {
	}
	return ((int32_t)p) - ((int32_t)src) - 1;
}

#define PStrMaxLength 255

LOCALPROC PStrFromCStr(ps3p r, /* CONST */ char *s)
{
	unsigned short L;

	L = CStrLen(s);
	if (L > PStrMaxLength) {
		L = PStrMaxLength;
	}
	*r++ = L;
	MoveBytes((anyp)s, (anyp)r, L);
}

LOCALPROC PStrFromChar(ps3p r, char x)
{
	r[0] = 1;
	r[1] = (char)x;
}

LOCALPROC PStrFromHandle(ps3p r, Handle h, uint32_t MaxL)
{
	uint32_t L = GetHandleSize(h);

	if (L > MaxL) {
		L = MaxL;
	}

	*r++ = L;
	BlockMove(*h, (Ptr)r, L);
}

LOCALFUNC tMacErr PStrToHand(ps3p s, Handle *r)
{
	return To_tMacErr(PtrToHand((Ptr)(s + 1), r, s[0]));
}

/* --- utilities for adapting to the environment --- */

#ifndef MightNotHaveAppearanceMgrAvail
#define MightNotHaveAppearanceMgrAvail 1
#endif

#ifndef MightNotHaveWindows85Avail
#define MightNotHaveWindows85Avail MightNotHaveAppearanceMgrAvail
#endif

#ifndef LowMemAPIAvail
#define LowMemAPIAvail 1
#endif

#ifndef _LMGetTime
#if LowMemAPIAvail
#define _LMGetTime LMGetTime
#else
#define _LMGetTime() (*(SInt32 *)(0x020C))
#endif
#endif

#ifndef _LMGetMBarHeight
#if LowMemAPIAvail
#define _LMGetMBarHeight LMGetMBarHeight
#else
#define _LMGetMBarHeight() (*(short *)(0x0BAA))
#endif
#endif

#ifndef _GetGrayRgn
#if /* LowMemAPIAvail */ 0
#define _GetGrayRgn LMGetGrayRgn
#else
#define _GetGrayRgn() (*(RgnHandle *)(0x9EE))
#endif
#endif

#ifndef _LMGetCurApName
#if LowMemAPIAvail
#define _LMGetCurApName LMGetCurApName
#else
#define _LMGetCurApName() ((StringPtr) 0x0910)
#endif
#endif

#define GetScreenBitsBounds(r) (*r) = qd.screenBits.bounds

#define _GetRegionBounds(region, bounds) *(bounds) = \
	(**(region)).rgnBBox

#define _GetPortPixMap(p) ((p)->portPixMap)

#ifndef _WindowRef
#if NewNamesAvail
#define _WindowRef WindowRef
#else
#define _WindowRef WindowPtr
#endif
#endif

#define _SetPortWindowPort(w) SetPort(w)

LOCALPROC _InvalWindowRect(_WindowRef mw, Rect *r)
{
	GrafPtr SavePort;

	GetPort(&SavePort);
	_SetPortWindowPort(mw);
	InvalRect(r);
	SetPort(SavePort);
}

#define _GetWindowPortBounds(w, r) *(r) = ((w)->portRect)

LOCALPROC InvalWholeWindow(_WindowRef mw)
{
	Rect bounds;

	_GetWindowPortBounds(mw, &bounds);
	_InvalWindowRect(mw, &bounds);
}

LOCALPROC SetMacWindContRect(_WindowRef mw, Rect *r)
{
#if Windows85APIAvail
	if (HaveNewWndMgrAvail()) {
		(void) SetWindowBounds (mw, kWindowContentRgn, r);
	} else
#endif
	{
#if MightNotHaveWindows85Avail
		MoveWindow(mw, r->left, r->top, false);
		SizeWindow(mw, r->right - r->left, r->bottom - r->top,
			true);
#endif
	}
	InvalWholeWindow(mw);
}

LOCALFUNC bool GetWindowTitleBounds(_WindowRef mw, Rect *r)
{
#if Windows85APIAvail
	if (HaveNewWndMgrAvail()) {
		return (noErr == GetWindowBounds(mw,
				kWindowTitleBarRgn, r));
	} else
#endif
	{
#if MightNotHaveWindows85Avail
		_GetRegionBounds(((WindowPeek)mw)->strucRgn, r);
		r->bottom = r->top + 15;
		r->left += 4;
		r->right -= 4;
#endif
		return true;
	}
}

#define topLeft(r) (((Point *) &(r))[0])
#define botRight(r) (((Point *) &(r))[1])

LOCALFUNC bool GetWindowContBounds(_WindowRef mw, Rect *r)
{
#if Windows85APIAvail
	if (HaveNewWndMgrAvail()) {
		return (noErr == GetWindowBounds(mw,
				kWindowContentRgn, r));
	} else
#endif
	{
#if MightNotHaveWindows85Avail
		GrafPtr oldPort;
		GetPort(&oldPort);
		_SetPortWindowPort(mw);
		_GetWindowPortBounds(mw, r);
		LocalToGlobal(&topLeft(*r));
		LocalToGlobal(&botRight(*r));
		SetPort(oldPort);
#endif
		return true;
	}
}

LOCALPROC GetGrayRgnBounds(Rect *r)
{
	_GetRegionBounds(_GetGrayRgn(), (Rect *)r);
}

/* --- main window data --- */

LOCALVAR WindowPtr gMainWindow = NULL;

#if MayFullScreen
LOCALVAR short hOffset;
LOCALVAR short vOffset;
#endif

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if VarFullScreen
LOCALVAR bool UseFullScreen = (WantInitFullScreen != 0);
#endif

#if EnableMagnify
LOCALVAR bool UseMagnify = (WantInitMagnify != 0);
#endif

#if EnableMagnify
LOCALPROC ScaleRect(Rect *r)
{
	r->left *= WindowScale;
	r->right *= WindowScale;
	r->top *= WindowScale;
	r->bottom *= WindowScale;
}
#endif

LOCALPROC SetScrnRectFromCoords(Rect *r,
	int16_t top, int16_t left, int16_t bottom, int16_t right)
{
	r->left = left;
	r->right = right;
	r->top = top;
	r->bottom = bottom;

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		OffsetRect(r, - ViewHStart, - ViewVStart);
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
		ScaleRect(r);
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		OffsetRect(r, hOffset, vOffset);
	}
#endif

}

#if EnableMagnify
#define ScaledHeight (WindowScale * vMacScreenHeight)
#define ScaledWidth (WindowScale * vMacScreenWidth)
#endif

#if EnableMagnify
LOCALVAR uint8_t * ScalingBuff = nullpr;
#endif

#if EnableMagnify

LOCALVAR uint8_t * ScalingTabl = nullpr;
#define ScalingTablsz (256 * WindowScale)

#define ScrnMapr_DoMap UpdateScaledBWCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 0
#define ScrnMapr_Map ScalingTabl
#define ScrnMapr_Scale WindowScale

#include "SCRNMAPR.h"

#endif

#if EnableMagnify
LOCALPROC SetUpScalingTabl(void)
{
	uint8_t *p4;
	int i;
	int j;
	int k;
	uint8_t bitsRemaining;
	uint8_t t1;
	uint8_t t2;

	p4 = ScalingTabl;
	for (i = 0; i < 256; ++i) {
		bitsRemaining = 8;
		t2 = 0;
		for (j = 8; --j >= 0; ) {
			t1 = (i >> j) & 1;
			for (k = WindowScale; --k >= 0; ) {
				t2 = (t2 << 1) | t1;
				if (--bitsRemaining == 0) {
					*p4++ = t2;
					bitsRemaining = 8;
					t2 = 0;
				}
			}
		}
	}
}
#endif

LOCALPROC DefaultDrawScreenBuff(int16_t top, int16_t left,
	int16_t bottom, int16_t right)
{
	BitMap src;
	Rect SrcRect;
	Rect DstRect;

	SrcRect.left = left;
	SrcRect.right = right;
	SrcRect.top = top;
	SrcRect.bottom = bottom;

	src.rowBytes = vMacScreenMonoByteWidth;
	SetRect(&src.bounds, 0, 0, vMacScreenWidth, vMacScreenHeight);
#if EnableMagnify
	if (UseMagnify) {

		if (! ColorTransValid) {
			SetUpScalingTabl();
			ColorTransValid = true;
		}

		UpdateScaledBWCopy(top, left, bottom, right);

		ScaleRect(&SrcRect);
		ScaleRect(&src.bounds);

		src.baseAddr = (Ptr)ScalingBuff;
		src.rowBytes *= WindowScale;
	} else
#endif
	{
		src.baseAddr = (Ptr)GetCurDrawBuff();
	}
	SetScrnRectFromCoords(&DstRect, top, left, bottom, right);
	CopyBits(&src,
		&gMainWindow->portBits,
		&SrcRect, &DstRect, srcCopy, NULL);
	/* FrameRect(&SrcRect); for testing */
}

LOCALPROC Update_Screen(void)
{
	GrafPtr savePort;

	GetPort(&savePort);
	_SetPortWindowPort(gMainWindow);

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		PaintRect(&gMainWindow->portRect);
	}
#endif

	DefaultDrawScreenBuff(0, 0, vMacScreenHeight, vMacScreenWidth);
	SetPort(savePort);
}

LOCALPROC HaveChangedScreenBuff(int16_t top, int16_t left,
	int16_t bottom, int16_t right)
{
#if 0 /* experimental code in progress */
	if (UseFullScreen)
	{
		{
			PixMapHandle pm= (**GetMainDevice()).gdPMap;

			/* LockPixels(pm); */
#if EnableMagnify
			if (! UseMagnify) {
#define PixelT uint32_t
				PixelT *p1 = (PixelT *)GetPixBaseAddr(pm);
				int i;
				int j;
				int k;
				uint32_t *p0 = (uint32_t *)GetCurDrawBuff();
				uint32_t SkipBytes = GetPixRowBytes(pm)
					- sizeof(PixelT) * vMacScreenWidth;
				uint32_t t0;
				PixelT a[2];

				((Ptr)p1) += (long)GetPixRowBytes(pm) * (top + vOffset);
				p1 += hOffset;
				p0 += (long)top * vMacScreenWidth / 32;

				a[0] = (PixelT) -1;
				a[1] = 0;

#if 1
				for (i = bottom - top; --i >= 0; ) {
					for (j = vMacScreenWidth / 32; --j >= 0; ) {
						t0 = *p0++;

						for (k = 32; --k >= 0; ) {
							PixelT v = a[(t0 >> k) & 1];
							*p1++ = v;
						}
					}
					((Ptr)p1) += SkipBytes;
				}
#endif
			} else {
#define PixelT uint32_t
				PixelT *p1 = (PixelT *)GetPixBaseAddr(pm);
				int i;
				int j;
				int k;
				uint32_t *p0 = (uint32_t *)GetCurDrawBuff();
				PixelT *p2;
				uint32_t t0;
				PixelT a[2];

				p1 += vOffset * ScaledWidth;
				p1 += (long)WindowScale * (long)ScaledWidth * top;
				p0 += (long)top * vMacScreenWidth / 32;

				a[0] = (PixelT) -1;
				a[1] = 0;

#if 1
				for (i = bottom - top; --i >= 0; ) {
					p2 = p1;
					for (j = vMacScreenWidth / 32; --j >= 0; ) {
						t0 = *p0++;

						for (k = 32; --k >= 0; ) {
							PixelT v = a[(t0 >> k) & 1];
								/* ((t0 >> k) & 1) - 1 */
							*p1++ = v;
							*p1++ = v;
						}
					}
					for (j = ScaledWidth; --j >= 0; ) {
						*p1++ = *p2++;
					}
				}
#endif
			}
#endif
			/* UnlockPixels(pm); */
		}
	} else
#endif
	{
		GrafPtr savePort;

		GetPort(&savePort);
		_SetPortWindowPort(gMainWindow);
		DefaultDrawScreenBuff(top, left, bottom, right);
		SetPort(savePort);
	}
}

LOCALPROC DrawChangesAndClear(void)
{
	if (ScreenChangedBottom > ScreenChangedTop) {
		HaveChangedScreenBuff(ScreenChangedTop, ScreenChangedLeft,
			ScreenChangedBottom, ScreenChangedRight);
		ScreenClearChanges();
	}
}

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		AutoScrollScreen();
	}
#endif
	DrawChangesAndClear();
}

/* --- keyboard --- */

LOCALVAR uint32_t LastEmKeys[4];

LOCALPROC ZapEmKeys(void)
{
	LastEmKeys[0] = 0;
	LastEmKeys[1] = 0;
	LastEmKeys[2] = 0;
	LastEmKeys[3] = 0;
}

LOCALPROC CheckKeyBoardState(void)
{
	int i;
	int j;
	uint32_t NewKeys[4];

	GetKeys(*(KeyMap *)NewKeys);

	for (j = 0; j < 16; ++j) {
		uint8_t k1 = ((uint8_t *)NewKeys)[j];
		uint8_t k2 = ((uint8_t *)LastEmKeys)[j];
		uint8_t k3 = k1 ^ k2;

		if (k3 != 0) {
			for (i = 0; i < 8; ++i) {
				if ((k3 & (1 << i)) != 0) {
					Keyboard_UpdateKeyMap2(Keyboard_RemapMac(j * 8 + i),
						(k1 & (1 << i)) != 0);
				}
			}
		}
	}
	for (i = 0; i < 4; ++i) {
		LastEmKeys[i] = NewKeys[i];
	}
}

LOCALVAR WantCmdOptOnReconnect = false;

#define KeyMap_TestBit(m, key) \
	((((uint8_t *)m)[(key) / 8] & (1 << ((key) & 7))) != 0)

LOCALPROC ReconnectKeyCodes3(void)
/* so keys already pressed will be ignored */
{
	int i;
	int j;
	bool oldv;
	bool newv;
	uint32_t NewKeys[4];

	GetKeys(*(KeyMap *)NewKeys);

	/* except check CapsLock */
	oldv = KeyMap_TestBit(LastEmKeys, MKC_CapsLock);
	newv = KeyMap_TestBit(NewKeys, MKC_CapsLock);
	if (oldv != newv) {
		Keyboard_UpdateKeyMap2(MKC_formac_CapsLock, newv);
	}

	/* and except for command/option on receive drop */
	if (WantCmdOptOnReconnect) {
		WantCmdOptOnReconnect = false;

		for (i = 0; i < 16; ++i) {
			uint8_t v = ((uint8_t *)NewKeys)[i];
			for (j = 0; j < 8; ++j) {
				if (0 != ((1 << j) & v)) {
					uint8_t k = i * 8 + j;
					if (MKC_CapsLock != k) {
						Keyboard_UpdateKeyMap2(Keyboard_RemapMac(k),
							true);
					}
				}
			}
		}
	}

	for (i = 0; i < 4; ++i) {
		LastEmKeys[i] = NewKeys[i];
	}
}

/* --- cursor hiding --- */

LOCALVAR bool HaveCursorHidden = false;
LOCALVAR bool WantCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		ShowCursor();
	}
}

LOCALPROC SetCursorArrow(void)
{
	SetCursor(&qd.arrow);
}

/* --- cursor moving --- */

/*
	When "EnableFSMouseMotion" the platform
	specific code can get relative mouse
	motion, instead of absolute coordinates
	on the emulated screen. It should
	set HaveMouseMotion to true when
	it is doing this (normally when in
	full screen mode.)

	This can usually be implemented by
	hiding the platform specific cursor,
	and then keeping it within a box,
	moving the cursor back to the center whenever
	it leaves the box. This requires the
	ability to move the cursor (in MoveMouse).
*/

/*
	mouse moving code (non OS X) adapted from
	MoveMouse.c by Dan Sears, which says that
	"Based on code from Jon Wtte, Denis Pelli,
	Apple, and a timely suggestion from Bo Lindbergh."
	Also says 'For documentation of the CDM, see Apple
	Tech Note "HW 01 - ADB (The Untold Story: Space Aliens
	ate my mouse)"'
*/

#ifndef TARGET_CPU_PPC
#error "TARGET_CPU_PPC undefined"
#endif

#if TARGET_CPU_PPC
enum {
	glueUppCursorDeviceMoveToProcInfo =
		kD0DispatchedPascalStackBased |
		DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode) |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		DISPATCHED_STACK_ROUTINE_PARAMETER(1,
			SIZE_CODE(sizeof(CursorDevicePtr))) |
		DISPATCHED_STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long))) |
		DISPATCHED_STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long))),
	glueUppCursorDeviceNextDeviceProcInfo =
		kD0DispatchedPascalStackBased |
		DISPATCHED_STACK_ROUTINE_SELECTOR_SIZE(kTwoByteCode) |
		RESULT_SIZE(SIZE_CODE(sizeof(OSErr))) |
		DISPATCHED_STACK_ROUTINE_PARAMETER(1,
			SIZE_CODE(sizeof(CursorDevicePtr *)))
};
#endif

#if TARGET_CPU_PPC
LOCALFUNC OSErr
CallCursorDeviceMoveTo(
	CursorDevicePtr ourDevice,
	long absX,
	long absY)
{
	return CallUniversalProc(
		GetToolboxTrapAddress(_CursorDeviceDispatch),
		glueUppCursorDeviceMoveToProcInfo,
		1, ourDevice, absX, absY);
}
#else
#define CallCursorDeviceMoveTo CursorDeviceMoveTo
#endif

#if TARGET_CPU_PPC
LOCALFUNC OSErr
CallCursorDeviceNextDevice(
	CursorDevicePtr *ourDevice)
{
	return CallUniversalProc(
		GetToolboxTrapAddress(_CursorDeviceDispatch),
		glueUppCursorDeviceNextDeviceProcInfo,
		0xB, ourDevice);
}
#else
#define CallCursorDeviceNextDevice CursorDeviceNextDevice
#endif

#if ! TARGET_CPU_PPC
pascal void CallCursorTask(void) =
{
	0x2078, 0x08EE,  /* MOVE.L jCrsrTask,A0 */
	0x4E90           /* JSR (A0) */
};
#endif

/*
	Low memory globals for the mouse
*/

#define RawMouse   0x082C
	/* low memory global that has current mouse loc */
#define MTemp      0x0828
	/* low memory global that has current mouse loc */
#define CrsrNew    0x08CE
	/* set after you change mtemp and rawmouse */
#define CrsrCouple 0x08CF
	/* true if the cursor is tied to the mouse */

LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
	/*
		Move the cursor to the point h, v
		on the emulated screen.
		if detect that this fails return false,
		otherwise return true.
		(on some platforms it is possible to
		move the curser, but there is no
		way to detect failure.)
	*/
	GrafPtr oldPort;
	Point CurMousePos;
	Point NewMousePos;
	uint32_t difftime;
	bool IsOk;
	long int StartTime = TickCount();

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		h -= ViewHStart;
		v -= ViewVStart;
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
		h *= WindowScale;
		v *= WindowScale;
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		h += hOffset;
		v += vOffset;
	}
#endif

	CurMousePos.h = h;
	CurMousePos.v = v;

	GetPort(&oldPort);
	_SetPortWindowPort(gMainWindow);
	LocalToGlobal(&CurMousePos);

	do {

#if HaveCPUfamM68K
		if (! HiToolTrapAvailable(_CursorDeviceDispatch)) {
			*(Point *)RawMouse = CurMousePos;
			*(Point *)MTemp = CurMousePos;
			*(Ptr)CrsrNew = *(Ptr)CrsrCouple;
#if ! TARGET_CPU_PPC
			CallCursorTask();
#endif
		} else
#endif
		{
			CursorDevice *firstMouse = NULL;
			CallCursorDeviceNextDevice(&firstMouse);
			if (firstMouse != NULL) {
				CallCursorDeviceMoveTo(firstMouse,
					(long) CurMousePos.h,
					(long) CurMousePos.v);
			}
		}

		GetMouse(&NewMousePos);
		IsOk = (h == NewMousePos.h) && (v == NewMousePos.v);
		difftime = (uint32_t)(TickCount() - StartTime);
	} while ((! IsOk) && (difftime < 5));

	SetPort(oldPort);
	return IsOk;
}

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

LOCALPROC MousePositionNotify(Point NewMousePos)
{
	bool ShouldHaveCursorHidden = true;

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePos.h -= hOffset;
		NewMousePos.v -= vOffset;
	}
#endif
#if VarFullScreen
	else
#endif
#if MayNotFullScreen
	{
		if (! PtInRgn(NewMousePos, gMainWindow->visRgn)) {
			ShouldHaveCursorHidden = false;
		}
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
		NewMousePos.h /= WindowScale;
		NewMousePos.v /= WindowScale;
	}
#endif

#if VarFullScreen
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
		MousePositionSetDelta(
			NewMousePos.h - SavedMouseH, NewMousePos.v - SavedMouseV);
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

#if VarFullScreen
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			ShouldHaveCursorHidden = true;
		}
#endif

		/* if (ShouldHaveCursorHidden || CurMouseButton) */
		/*
			for a game like arkanoid, would like mouse to still
			move even when outside window in one direction
		*/
		MousePositionSet(NewMousePos.h, NewMousePos.v);
	}

	WantCursorHidden = ShouldHaveCursorHidden;
}

LOCALPROC MousePositionNotifyFromGlobal(Point NewMousePos)
{
	GrafPtr oldPort;

	GetPort(&oldPort);
	_SetPortWindowPort(gMainWindow);
	GlobalToLocal(&NewMousePos);
	SetPort(oldPort);

	MousePositionNotify(NewMousePos);
}

LOCALPROC CheckMouseState(void)
{
	Point NewMousePos;
	GrafPtr oldPort;

	GetPort(&oldPort);
	_SetPortWindowPort(gMainWindow);
	GetMouse(&NewMousePos);
	SetPort(oldPort);

	MousePositionNotify(NewMousePos);
}

LOCALPROC DisconnectKeyCodes3(void)
{
	DisconnectKeyCodes2();

	MouseButtonSet(false);

	ForceShowCursor();
}


/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

/*
	be sure to avoid getting confused if TickCount
	overflows and wraps.
*/

LOCALVAR uint32_t TrueEmulatedTime = 0;
	/*
		The amount of time the program has
		been running, measured in Macintosh
		"ticks". There are 60.14742 ticks per
		second.

		(time when the emulation is
		stopped for more than a few ticks
		should not be counted.)
	*/

LOCALVAR long int LastTime;

LOCALPROC StartUpTimeAdjust(void)
{
	/*
		prepare to call UpdateTrueEmulatedTime.

		will be called again when haven't been
		regularly calling UpdateTrueEmulatedTime,
		(such as the emulation has been stopped).
	*/
	LastTime = TickCount();
}

LOCALPROC UpdateTrueEmulatedTime(void)
{
	/*
		Update TrueEmulatedTime. usually
		need to convert between how the
		host operating system measures
		time and Macintosh ticks. But
		not for this port.
	*/
	long int LatestTime = TickCount();
	int32_t TimeDiff = LatestTime - LastTime;

	if (TimeDiff != 0) {
		LastTime = LatestTime;

		if (TimeDiff >= 0) {
			if (TimeDiff > 16) {
				/* emulation interrupted, forget it */
				++TrueEmulatedTime;

#if dbglog_TimeStuff
				dbglog_writelnNum("emulation interrupted",
					TrueEmulatedTime);
#endif
			} else {
				TrueEmulatedTime += TimeDiff;
			}
		}
	}
}

LOCALFUNC bool CheckDateTime(void)
{
	/*
		Update CurMacDateInSeconds, the number
		of seconds since midnight January 1, 1904.

		return true if CurMacDateInSeconds is
		different than it was on the last
		call to CheckDateTime.
	*/
	uint32_t NewMacDateInSecond;

	NewMacDateInSecond = _LMGetTime();
	if (CurMacDateInSeconds != NewMacDateInSecond) {
		CurMacDateInSeconds = NewMacDateInSecond;
		return true;
	} else {
		return false;
	}
}

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

	(void) CheckDateTime();

	return true;
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
			++LastTime;
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
	if (NULL == sndChannel)
#if HaveCPUfamM68K
	if (HaveSndMngrAvail())
#endif
	{
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

#define IgnorableEventMask \
	(mUpMask | keyDownMask | keyUpMask | autoKeyMask)

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
			EventRecord theEvent;

#if dbglog_SoundStuff
			dbglog_writeln("busy, so sleep");
#endif

#if HaveCPUfamM68K
			if (! HaveWaitNextEventAvail()) {
				(void) GetNextEvent(IgnorableEventMask, &theEvent);
			} else
#endif
			{
				(void) WaitNextEvent(IgnorableEventMask,
					&theEvent, 1, NULL);
			}
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

#if MayFullScreen
LOCALPROC AdjustMachineGrab(void)
{
#if EnableFSMouseMotion
	AdjustMouseMotionGrab();
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

/* --- basic dialogs --- */

LOCALPROC NativeStrFromCStr(ps3p r, char *s, bool AddEllipsis)
{
	int i;
	int L;
	uint8_t ps[ClStrMaxLength];

	ClStrFromSubstCStr(&L, ps, s);
	if (AddEllipsis) {
		ClStrAppendChar(&L, ps, kCellEllipsis);
	}

	if (L > 255) {
		L = 255;
	}

	for (i = 0; i < L; ++i) {
		r[i + 1] = Cell2MacAsciiMap[ps[i]];
	}

	r[0] = L;
}

#ifndef HogCPU
#define HogCPU 1
#endif

#if HogCPU
LOCALVAR long NoEventsCounter = 0;
#endif

LOCALVAR bool gBackgroundFlag = false;
LOCALVAR bool gTrueBackgroundFlag = false;
LOCALVAR bool CurSpeedStopped = true;

LOCALVAR bool ADialogIsUp = false;

LOCALPROC BeginDialog(void)
{
	DisconnectKeyCodes3();
	ADialogIsUp = true;
#if MayFullScreen
	UngrabMachine();
#endif
}

LOCALPROC EndDialog(void)
{
#if HogCPU
	NoEventsCounter = 0;
#endif
	ADialogIsUp = false;
	ReconnectKeyCodes3();
}


#define kStandardAlert 128

LOCALPROC CheckSavedMacMsg(void)
{
	/*
		This is currently only used in the
		rare case where there is a message
		still pending as the program quits.
	*/
	Str255 briefMsgp;
	Str255 longMsgp;

	if (nullpr != SavedBriefMsg) {
		NativeStrFromCStr(briefMsgp, SavedBriefMsg, false);
		NativeStrFromCStr(longMsgp, SavedLongMsg, false);
#if AppearanceAvail
		if (HaveAppearanceAvail()) {
			AlertStdAlertParamRec param;
			short itemHit;

			param.movable = 0;
			param.filterProc = nil;
			param.defaultText = "\pOK";
			param.cancelText = nil;
			param.otherText = nil;
			param.helpButton = false;
			param.defaultButton = kAlertStdAlertOKButton;
			param.cancelButton = 0;
			param.position = kWindowDefaultPosition;

			StandardAlert(
				(SavedFatalMsg) ? kAlertStopAlert : kAlertCautionAlert,
				briefMsgp, longMsgp, &param, &itemHit);
		} else
#endif
		{
			ParamText(briefMsgp, longMsgp, "\p", "\p");
			if (SavedFatalMsg) {
				while (StopAlert(kStandardAlert, NULL) != 1) {
				}
			} else {
				while (CautionAlert(kStandardAlert, NULL) != 1) {
				}
			}
			/* Alert (kStandardAlert, 0L); */
		}

		SavedBriefMsg = nullpr;
	}
}

/* --- hide/show menubar --- */

#if MayFullScreen
#if MightNotHaveWindows85Avail
LOCALVAR RgnHandle GrayRgnSave = NULL;
LOCALVAR short mBarHeightSave;
#endif
#endif

#if MayFullScreen
LOCALPROC _HideMenuBar(void)
{
#if Windows85APIAvail
	if (HaveHideShowMenuAvail()) {
		if (IsMenuBarVisible()) {
			HideMenuBar();
		}
	} else
#endif
	{
#if MightNotHaveWindows85Avail
		if (NULL == GrayRgnSave) {
			RgnHandle mBarRgn = NewRgn();
			if (mBarRgn != NULL) {
				GrayRgnSave = NewRgn();
				if (GrayRgnSave != NULL) {
					CopyRgn(_GetGrayRgn(), GrayRgnSave);
					RectRgn(mBarRgn, &qd.screenBits.bounds);
					DiffRgn(mBarRgn, _GetGrayRgn(), mBarRgn);
						/*
							menu bar rgn, plus corner areas
							of main screen
						*/
					mBarHeightSave = _LMGetMBarHeight();
					LMSetMBarHeight(0);
					UnionRgn(_GetGrayRgn(), mBarRgn, _GetGrayRgn());
					PaintBehind(LMGetWindowList(), mBarRgn);
					CalcVisBehind(LMGetWindowList(), mBarRgn);
#if 0
					controlStripHidden = false;
					if (noErr == Gestalt(
						gestaltControlStripVersion, &result))
					{
						if (SBIsControlStripVisible()) {
							controlStripHidden = true;
							SBShowHideControlStrip(false);
						}
					}
#endif
				}
				DisposeRgn(mBarRgn);
			}
		}
#endif
	}
}
#endif

#if MayFullScreen
LOCALPROC _ShowMenuBar(void)
{
#if Windows85APIAvail
	if (HaveHideShowMenuAvail()) {
		if (! IsMenuBarVisible()) {
			ShowMenuBar();
		}
	} else
#endif
	{
#if MightNotHaveWindows85Avail
		if (GrayRgnSave != NULL) {
			LMSetMBarHeight(mBarHeightSave);
			CopyRgn(GrayRgnSave, _GetGrayRgn());
			/*
				PaintBehind(LMGetWindowList(), GrayRgnSave);
				CalcVisBehind(LMGetWindowList(), GrayRgnSave);
			*/
			DisposeRgn(GrayRgnSave);
			DrawMenuBar();
			GrayRgnSave = NULL;
#if 0
			if (controlStripHidden) {
				controlStripHidden = false;
				if (noErr ==
					Gestalt(gestaltControlStripVersion, &result))
				{
					SBShowHideControlStrip(true);
				}
			}
#endif
		}
#endif
	}
}
#endif

#if IncludePbufs
LOCALVAR Handle PbufDat[NumPbufs];
#endif

#if IncludePbufs
LOCALFUNC tMacErr PbufNewFromHandle(Handle h, uint32_t count, tPbuf *r)
{
	tPbuf i;
	tMacErr err;

	if (! FirstFreePbuf(&i)) {
		DisposeHandle(h);
		err = mnvm_miscErr;
	} else {
		*r = i;
		PbufDat[i] = h;
		PbufNewNotify(i, count);
		err = mnvm_noErr;
	}

	return err;
}
#endif

#if IncludePbufs
GLOBALOSGLUFUNC tMacErr PbufNew(uint32_t count, tPbuf *r)
{
	Handle h;
	tMacErr err = mnvm_miscErr;

	h = NewHandleClear(count);
	if (h != NULL) {
		err = PbufNewFromHandle(h, count, r);
	}

	return err;
}
#endif

#if IncludePbufs
GLOBALOSGLUPROC PbufDispose(tPbuf i)
{
	DisposeHandle(PbufDat[i]);
	PbufDisposeNotify(i);
}
#endif

#if IncludePbufs
LOCALPROC UnInitPbufs(void)
{
	tPbuf i;

	for (i = 0; i < NumPbufs; ++i) {
		if (PbufIsAllocated(i)) {
			PbufDispose(i);
		}
	}
}
#endif

#if IncludePbufs
#define PbufHaveLock 1
#endif

#if IncludePbufs
LOCALFUNC uint8_t * PbufLock(tPbuf i)
{
	uint8_t * p;

	Handle h = PbufDat[i];

	if (NULL == h) {
		p = nullpr;
	} else {
		HLock(h);
		p = (uint8_t *)*h;
	}

	return p;
}
#endif

#if IncludePbufs
LOCALPROC PbufUnlock(tPbuf i)
{
	HUnlock(PbufDat[i]);
}
#endif

#if IncludePbufs
GLOBALOSGLUPROC PbufTransfer(uint8_t * Buffer,
	tPbuf i, uint32_t offset, uint32_t count, bool IsWrite)
{
	Handle h = PbufDat[i];

	HLock(h);
	{
		void *p = ((uint8_t *)*h) + offset;
		if (IsWrite) {
			BlockMove(Buffer, p, count);
		} else {
			BlockMove(p, Buffer, count);
		}
	}
	HUnlock(h);
}
#endif

/* --- clipboard --- */

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEexport(tPbuf i)
{
	/*
		PBuf i is an array of macintosh
		style characters. (using the
		MacRoman character set.)

		Should export this Buffer to the
		native clipboard, performing character
		set translation, and eof character translation
		as needed. (Not needed for this port.)

		return 0 if it succeeds, nonzero (a
		Macintosh style error code, but -1
		will do) on failure.
	*/
	OSErr err;

	err = ZeroScrap();
	if (noErr == err) {
		uint32_t L = PbufSize[i];
		Handle h = PbufDat[i];
		HLock(h);
		err = PutScrap(L, 'TEXT', *h);
		HUnlock(h);
	}

	PbufDispose(i);

	return (tMacErr)err;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEimport(tPbuf *r)
{
	/*
		Import the native clipboard as text,
		and convert it to Macintosh format,
		in a Pbuf.

		return 0 if it succeeds, nonzero (a
		Macintosh style error code, but -1
		will do) on failure.
	*/

	long off;
	long v;
	Handle h;
	OSErr err = (OSErr)mnvm_miscErr;

	h = NewHandle(0);
	if (h != NULL) {
		v = GetScrap(h, 'TEXT', &off);
		if (v < 0) {
			err = v;
		} else {
			err = (OSErr)PbufNewFromHandle(h, v, r);
			h = NULL;
		}
		if (NULL != h) {
			DisposeHandle(h);
		}
	}

	return (tMacErr)err;
}
#endif

/* --- drives --- */

LOCALVAR short Drives[NumDrives]; /* open disk image files */
#if (IncludeSonyGetName || IncludeSonyNew) && HaveCPUfamM68K
LOCALVAR Handle DriveNames[NumDrives];
#endif

LOCALPROC InitDrives(void)
{
	/*
		This isn't really needed, Drives[i] and DriveNames[i]
		need not have valid values when not vSonyIsInserted[i].
	*/
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
#if (IncludeSonyGetName || IncludeSonyNew) && HaveCPUfamM68K
		DriveNames[i] = NULL;
#endif
	}
}

GLOBALOSGLUFUNC tMacErr vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	/*
		return 0 if it succeeds, nonzero (a
		Macintosh style error code, but -1
		will do) on failure.
	*/
	tMacErr result;
	uint32_t NewSony_Count = Sony_Count;

	result =
		(tMacErr)SetFPos(Drives[Drive_No], fsFromStart, Sony_Start);
	if (mnvm_noErr == result) {
		if (IsWrite) {
			/*
				write Sony_Count bytes from Buffer, to disk image
				number Drive_No, starting at offset Sony_Start.
			*/

			result = (tMacErr)FSWrite(Drives[Drive_No],
				(long *)&NewSony_Count, Buffer);
		} else {
			/*
				read Sony_Count bytes into Buffer, from disk image
				number Drive_No, starting at offset Sony_Start.
			*/

			result = (tMacErr)FSRead(Drives[Drive_No],
				(long *)&NewSony_Count, Buffer);
		}
	}

	if (nullpr != Sony_ActCount) {
		*Sony_ActCount = NewSony_Count;
	}

	return result;
}

GLOBALOSGLUFUNC tMacErr vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	/*
		set Sony_Count to the size of disk image number Drive_No.

		return 0 if it succeeds, nonzero (a
		Macintosh style error code, but -1
		will do) on failure.
	*/
	return GetEOF(Drives[Drive_No], (long *)Sony_Count);
}

LOCALFUNC OSErr vSonyEject0(tDrive Drive_No)
{
	/*
		close disk image number Drive_No.

		return 0 if it succeeds, nonzero (a
		Macintosh style error code, but -1
		will do) on failure.
	*/
	short refnum = Drives[Drive_No];
	Drives[Drive_No] = NotAfileRef; /* not really needed */

	DiskEjectedNotify(Drive_No);

#if (IncludeSonyGetName || IncludeSonyNew) && HaveCPUfamM68K
	{
		Handle h = DriveNames[Drive_No];
		if (NULL != h) {
			DisposeHandle(h);
			DriveNames[Drive_No] = NULL; /* not really needed */
		}
	}
#endif

	return FSClose(refnum);
}

GLOBALOSGLUFUNC tMacErr vSonyEject(tDrive Drive_No)
{
	OSErr result;
	short vRefNum;
	bool DidEject = false;
	short refnum = Drives[Drive_No];

	result = GetVRefNum(refnum, &vRefNum);
	if (noErr == result) {
		DidEject = true;
		result = vSonyEject0(Drive_No);
		(void) FlushVol(NULL, vRefNum);
	}

	if (! DidEject) {
		result = vSonyEject0(Drive_No);
	}

	return (tMacErr)result;
}

#if IncludeSonyNew
GLOBALOSGLUFUNC tMacErr vSonyEjectDelete(tDrive Drive_No)
{
	OSErr result;
	Str255 s;
	bool DidEject = false;
	short refnum = Drives[Drive_No];

#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		Handle h = DriveNames[Drive_No];
		if (NULL != h) {
			short vRefNum;
			result = GetVRefNum(refnum, &vRefNum);
			if (noErr == result) {
				PStrFromHandle(s, h, 255);
				result = vSonyEject0(Drive_No);
				DidEject = true;
				(void) FSDelete(s, vRefNum);
			}
		}
	} else
#endif
	{
		FCBPBRec b;

		b.ioCompletion = NULL;
		b.ioNamePtr = (StringPtr)s;
		b.ioVRefNum = 0;
		b.ioRefNum = refnum;
		b.ioFCBIndx = 0;
		result = PBGetFCBInfoSync(&b);
		if (noErr == result) {
			FSSpec spec;
			result = FSMakeFSSpec(b.ioFCBVRefNum, b.ioFCBParID,
				s, &spec);
			if (noErr == result) {
				result = vSonyEject0(Drive_No);
				DidEject = true;
				(void) FSpDelete(&spec);
			}
		}
	}

	if (! DidEject) {
		(void) vSonyEject0(Drive_No);
	}

	return (tMacErr)result;
}
#endif

LOCALPROC UnInitDrives(void)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (vSonyIsInserted(i)) {
			(void) vSonyEject(i);
		}
	}
}

#if IncludeSonyGetName
GLOBALOSGLUFUNC tMacErr vSonyGetName(tDrive Drive_No, tPbuf *r)
{
	FCBPBRec b;
	Str255 s;
	tMacErr err = mnvm_miscErr;

#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		Handle h = DriveNames[Drive_No];
		if (NULL != h) {
			PStrFromHandle(s, h, 255);
			err = mnvm_noErr;
		}
	} else
#endif
	{
		b.ioCompletion = NULL;
		b.ioNamePtr = (StringPtr)s;
		b.ioVRefNum = 0;
		b.ioRefNum = Drives[Drive_No];
		b.ioFCBIndx = 0;
		err = To_tMacErr(PBGetFCBInfoSync(&b));
	}

	if (mnvm_noErr == err) {
		Handle h;
		err = PStrToHand(s, &h);
		if (noErr == err) {
			err = PbufNewFromHandle(h, s[0], r);
		}
	}

	return err;
}
#endif

LOCALFUNC tMacErr Sony_Insert0(short refnum, bool locked, ps3p s)
{
	/*
		Given reference to open file, mount it as
		a disk image file. if "locked", then mount
		it as a locked disk.
	*/

	tDrive Drive_No;

#if ! ((IncludeSonyGetName || IncludeSonyNew) && HaveCPUfamM68K)
	UnusedParam(s);
#endif

	if (! FirstFreeDisk(&Drive_No)) {
		(void) FSClose(refnum);
		return mnvm_tmfoErr; /* too many files open */
	} else {
		Drives[Drive_No] = refnum;
		DiskInsertNotify(Drive_No, locked);
#if (IncludeSonyGetName || IncludeSonyNew) && HaveCPUfamM68K
		if (s != NULL) {
			Handle h;

			if (mnvm_noErr != PStrToHand(s, &h)) {
				h = NULL;
			}
			DriveNames[Drive_No] = h;
		}
#endif
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

LOCALFUNC tMacErr InsertADiskFromFileRef(FSSpec *spec)
{
	short refnum;
	tMacErr err;
	bool locked = false;

	err = To_tMacErr(FSpOpenDF(spec, fsRdWrPerm, &refnum));
	switch (err) {
		case mnvm_permErr:
		case mnvm_wrPermErr:
		case mnvm_afpAccessDenied:
			locked = true;
			err = To_tMacErr(FSpOpenDF(spec, fsRdPerm, &refnum));
			break;
		default:
			break;
	}
	if (mnvm_noErr == err) {
		err = Sony_Insert0(refnum, locked, NULL);
	}

	return err;
}

#if HaveCPUfamM68K
LOCALFUNC tMacErr InsertADiskFromNamevRef(ConstStr255Param fileName,
	short vRefNum)
{
	ParamBlockRec R;
	tMacErr err;
	bool locked = false;

	R.ioParam.ioCompletion = NULL;
	R.ioParam.ioNamePtr = (StringPtr)fileName;
	R.ioParam.ioVRefNum = vRefNum;
	R.ioParam.ioVersNum = 0;
	R.ioParam.ioPermssn = fsRdWrPerm;
	R.ioParam.ioMisc = NULL;
	err = To_tMacErr(PBOpen(&R, false));
	switch (err) {
		case mnvm_permErr:
		case mnvm_wrPermErr:
		case mnvm_afpAccessDenied:
			locked = true;
			R.ioParam.ioPermssn = fsRdPerm;
			err = To_tMacErr(PBOpen(&R, false));
			break;
		default:
			break;
	}
	if (mnvm_noErr == err) {
		err = Sony_Insert0(R.ioParam.ioRefNum, locked, (ps3p)fileName);
	}

	return err;
}
#endif

LOCALFUNC tMacErr LoadMacRomFromRefNum(short refnum)
{
	/*
		load the ROM image file into ptr ROM
	*/
	tMacErr err;
	long count = kROM_Size;

	if (mnvm_noErr != (err = To_tMacErr(
		FSRead(refnum, &count, ROM))))
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

LOCALFUNC tMacErr LoadMacRomFromFSSpec(FSSpec *spec)
{
	tMacErr err;
	short refnum;

	if (mnvm_noErr == (err =
		To_tMacErr(FSpOpenDF(spec, fsRdPerm, &refnum))))
	{
		err = LoadMacRomFromRefNum(refnum);
		(void) FSClose(refnum);
	}

	return err;
}

#if HaveCPUfamM68K
LOCALFUNC tMacErr LoadMacRomFromNamevRef(ConstStr255Param fileName,
	short vRefNum)
{
	tMacErr err;
	ParamBlockRec R;

	R.ioParam.ioCompletion = NULL;
	R.ioParam.ioNamePtr = (StringPtr)fileName;
	R.ioParam.ioVRefNum = vRefNum;
	R.ioParam.ioVersNum = 0;
	R.ioParam.ioPermssn = fsRdPerm;
	R.ioParam.ioMisc = NULL;
	if (mnvm_noErr == (err = To_tMacErr(PBOpen(&R, false)))) {
		err = LoadMacRomFromRefNum(R.ioParam.ioRefNum);
		(void) FSClose(R.ioParam.ioRefNum);
	}

	return err;
}
#endif

#if HaveCPUfamM68K
LOCALFUNC tMacErr InsertADiskFromNamevRef1(ConstStr255Param fileName,
	short vRefNum)
{
	tMacErr err;

	if (! ROM_loaded) {
		err = LoadMacRomFromNamevRef(fileName, vRefNum);
	} else {
		err = InsertADiskFromNamevRef(fileName, vRefNum);
	}

	return err;
}
#endif

LOCALFUNC tMacErr InsertADiskOrAliasFromSpec(FSSpec *spec,
	bool MaybeROM, bool MaybeAlias)
{
	Boolean isFolder;
	Boolean isAlias;
	tMacErr err;

	if ((! MaybeAlias)
		|| CheckSaveMacErr(ResolveAliasFile(spec, true,
			&isFolder, &isAlias)))
	{
		if (MaybeROM && ! ROM_loaded) {
			err = LoadMacRomFromFSSpec(spec);
		} else {
			err = InsertADiskFromFileRef(spec);
		}
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
	FSSpec spec;
	Size actualSize;

	if (CheckSaveMacErr(AECountItems(docList, &itemsInList))) {
		for (index = 1; index <= itemsInList; ++index) {
			if (CheckSaveMacErr(AEGetNthPtr(docList, index, typeFSS,
				&keyword, &typeCode, (Ptr)&spec, sizeof(FSSpec),
				&actualSize)))
			if (CheckSavetMacErr(InsertADiskOrAliasFromSpec(&spec,
				true, false)))
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

LOCALFUNC tMacErr InsertADiskFromNameEtc(Dir_R *d,
	ConstStr255Param fileName)
{
	tMacErr err;

#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		err = InsertADiskFromNamevRef(fileName, d->VRefNum);
	} else
#endif
	{
		FSSpec spec;

		if (CheckSaveMacErr(
			FSMakeFSSpec(d->VRefNum, d->DirId, fileName, &spec)))
		{
			err = InsertADiskOrAliasFromSpec(&spec,
				false, true);
		}
	}

	return err;
}

#if NavigationAvail
pascal Boolean NavigationFilterProc(
	AEDesc* theItem, void* info, void* NavCallBackUserData,
	NavFilterModes theNavFilterModes);
pascal Boolean NavigationFilterProc(
	AEDesc* theItem, void* info, void* NavCallBackUserData,
	NavFilterModes theNavFilterModes)
{
	Boolean display = true;
	NavFileOrFolderInfo* theInfo = (NavFileOrFolderInfo*)info;
	UnusedParam(theNavFilterModes);
	UnusedParam(NavCallBackUserData);

	if (typeFSS == theItem->descriptorType) {
		if (! theInfo->isFolder) {
			/*
				use:
					'theInfo->fileAndFolder.fileInfo.finderInfo.fdType'
				to check for the file type you want to filter.
			*/
		}
	}
	return display;
}
#endif


#if NavigationAvail
pascal void NavigationEventProc(
	NavEventCallbackMessage callBackSelector,
	NavCBRecPtr callBackParms, void *NavCallBackUserData);
pascal void NavigationEventProc(
	NavEventCallbackMessage callBackSelector,
	NavCBRecPtr callBackParms, void *NavCallBackUserData)
{
	UnusedParam(NavCallBackUserData);

	if (kNavCBEvent == callBackSelector) {
		switch (callBackParms->eventData.eventDataParms.event->what) {
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
	}
}
#endif

#define PStrConstBlank ((ps3p)"\000")

LOCALPROC InsertADisk0(void)
{
#if NavigationAvail
#define DisposeNavEventUPP(userUPP) \
	DisposeRoutineDescriptor(userUPP)
#define DisposeNavObjectFilterUPP(userUPP) \
	DisposeRoutineDescriptor(userUPP)
#define NewNavObjectFilterUPP NewNavObjectFilterProc
#define NewNavEventUPP NewNavEventProc

	if (HaveNavServicesAvail()) {
		NavReplyRecord theReply;
		NavDialogOptions dialogOptions;
		OSErr theErr = noErr;
		NavTypeListHandle openList = NULL;
		NavObjectFilterUPP filterUPP =
			NewNavObjectFilterUPP(
				/* (NavObjectFilterProcPtr) */ NavigationFilterProc);
		NavEventUPP eventUPP = NewNavEventUPP(
			/* (NavEventProcPtr) */ NavigationEventProc);

		theErr = NavGetDefaultDialogOptions(&dialogOptions);

		dialogOptions.dialogOptionFlags |= kNavDontAutoTranslate;
		/*
			dialogOptions.dialogOptionFlags &= ~ kNavAllowMultipleFiles;
		*/
		dialogOptions.dialogOptionFlags &= ~ kNavAllowPreviews;

		BeginDialog();
		theErr = NavGetFile(NULL,
						&theReply,
						&dialogOptions,
						/* NULL */ eventUPP,
						NULL,
						filterUPP,
						(NavTypeListHandle)openList,
						NULL);
		EndDialog();

		DisposeNavObjectFilterUPP(filterUPP);
		DisposeNavEventUPP(eventUPP);


		if (noErr == theErr) {
			if (theReply.validRecord) {
				ReportStandardOpenDiskError(InsertDisksFromDocList(
					&theReply.selection));
			}

			NavDisposeReply(&theReply);
		}

	} else
#endif
#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		Point where;
		SFReply reply;

		where.h = 50;
		where.v = 50;
		BeginDialog();
		SFGetFile(*(Point *)&where, PStrConstBlank, NULL,
			-1 /* kNumFileTypes */, NULL /* fileTypes */,
			NULL, &reply);
		EndDialog();
		if (reply.good) {
			ReportStandardOpenDiskError(
				InsertADiskFromNamevRef1(reply.fName, reply.vRefNum));
		}
	} else
#endif
	{
		StandardFileReply reply;

		BeginDialog();
		StandardGetFile(0L, -1, 0L, &reply);
		EndDialog();
		if (reply.sfGood) {
			ReportStandardOpenDiskError(
				InsertADiskOrAliasFromSpec(&reply.sfFile,
					true, false));
		}
	}
}

#ifndef AppIsBundle
#define AppIsBundle 0
#endif

LOCALVAR Dir_R DatDir;

#if AppIsBundle
LOCALFUNC bool DirectorySpec2DirId(FSSpec *spec, long *dirID)
{
	CInfoPBRec b;

	b.hFileInfo.ioCompletion = NULL;
	b.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	b.hFileInfo.ioVRefNum = spec->vRefNum;
	b.dirInfo.ioFDirIndex = 0;
	b.dirInfo.ioDrDirID = spec->parID;
	if (noErr == PBGetCatInfo(&b, false)) {
		*dirID = b.dirInfo.ioDrDirID;
		return true;
	} else {
		return false;
	}
}
#endif

#if AppIsBundle
LOCALFUNC bool FindNamedChildDirId(short TrueParentVol,
	long ParentDirId, StringPtr ChildName,
	short *TrueChildVol, long *ChildDirId)
{

	FSSpec temp_spec;
	Boolean isFolder;
	Boolean isAlias;

	if (noErr == FSMakeFSSpec(TrueParentVol, ParentDirId,
		ChildName, &temp_spec))
	if (noErr == ResolveAliasFile(&temp_spec, true,
		&isFolder, &isAlias))
	if (isFolder)
	if (DirectorySpec2DirId(&temp_spec, ChildDirId))
	{
		*TrueChildVol = temp_spec.vRefNum;
		return true;
	}
	return false;
}
#endif

LOCALFUNC bool InitApplInfo(void)
{
#if HaveCPUfamM68K
	if (! HaveFSSpecCallsAvail()) {
		if (noErr == GetVol(NULL, &DatDir.VRefNum)) {
			DatDir.DirId = 0;
			return true;
		}
	} else
#endif
	{
		FCBPBRec pb;
		Str255 fileName;

		pb.ioCompletion = NULL;
		pb.ioNamePtr = fileName;
		pb.ioVRefNum = 0;
		pb.ioRefNum = CurResFile();
		pb.ioFCBIndx = 0;
		if (noErr == PBGetFCBInfoSync(&pb)) {
			DatDir.VRefNum = pb.ioFCBVRefNum;
			DatDir.DirId = pb.ioFCBParID;
			return true;
		}
	}

	return false;
}

LOCALFUNC tMacErr DirFromWD_v2(short VRefNum, Dir_R *d)
{
	tMacErr err;
	Str63 s;
	WDPBRec pb;

#if Support64kROM
	if (Have64kROM()) {
		d->VRefNum = VRefNum;
		d->DirId = 0;
		err = mnvm_noErr;
	} else
#endif
	{
		pb.ioCompletion = NULL;
		pb.ioNamePtr = s;
		pb.ioVRefNum = VRefNum;
		pb.ioWDIndex = 0;
		pb.ioWDProcID = 0;
		err = To_tMacErr(PBGetWDInfoSync(&pb));
		if (mnvm_noErr == err) {
			d->VRefNum = pb.ioWDVRefNum;
			d->DirId = pb.ioWDDirID;
		}
	}

	return err;
}

LOCALFUNC tMacErr FindPrefFolder(Dir_R *d)
{
	tMacErr err;
	long reply;

	if (HaveGestaltAvail()
		&& (noErr == Gestalt(gestaltFindFolderAttr, &reply))
		&& TestBit(reply, gestaltFindFolderPresent)
		)
	{
		err = To_tMacErr(FindFolder(
			kOnSystemDisk,
			kPreferencesFolderType,
			kDontCreateFolder,
			&d->VRefNum,
			&d->DirId));
	} else {
		SysEnvRec info;

		err = To_tMacErr(SysEnvirons(1, &info));
		if (mnvm_noErr == err) {
			err = DirFromWD_v2(info.sysVRefNum, d);
		}
	}

	return err;
}

#define CatInfoIsFolder(cPB) \
	(((cPB)->hFileInfo.ioFlAttrib & kioFlAttribDirMask) != 0)

#define PStrLength(s) (*(s))
#define SizeOfListChar(n) (n)
#define PStrToTotSize(s) (SizeOfListChar(PStrLength(s) + 1))
	/* + 1 for length byte */

LOCALPROC PStrCopy(ps3p r, ps3p s)
{
	MoveBytes((anyp)s, (anyp)r, PStrToTotSize(s));
}

LOCALFUNC tMacErr FindNamedChildDir_v2(Dir_R *src_d, StringPtr s,
	Dir_R *dst_d)
{
	tMacErr err;
	Str255 NameBuffer;
	CInfoPBRec cPB;

	cPB.hFileInfo.ioCompletion = NULL;
	cPB.hFileInfo.ioVRefNum = src_d->VRefNum;
	cPB.dirInfo.ioDrDirID = src_d->DirId;
	cPB.hFileInfo.ioNamePtr = NameBuffer;
	PStrCopy(NameBuffer, s);
	cPB.dirInfo.ioFDirIndex = 0;

	err = To_tMacErr(PBGetCatInfoSync(&cPB));

	if (mnvm_noErr == err) {
		if (! CatInfoIsFolder(&cPB)) {
			err = mnvm_dirNFErr;
		} else {
			dst_d->VRefNum = cPB.hFileInfo.ioVRefNum;
			dst_d->DirId = cPB.dirInfo.ioDrDirID;
		}
	}

	return err;
}

LOCALFUNC tMacErr ResolveAliasDir_v2(Dir_R *src_d, StringPtr s,
	Dir_R *dst_d)
{
	tMacErr err;
	FSSpec spec;
	Boolean isFolder;
	Boolean isAlias;
	Dir_R src2_d;

	spec.vRefNum = src_d->VRefNum;
	spec.parID = src_d->DirId;
	PStrCopy(spec.name, s);
	err = To_tMacErr(
		ResolveAliasFile(&spec, true, &isFolder, &isAlias));
	if (mnvm_noErr == err) {
		if (! isAlias) {
			err = mnvm_dirNFErr;
		} else {
			src2_d.VRefNum = spec.vRefNum;
			src2_d.DirId = spec.parID;
			err = FindNamedChildDir_v2(&src2_d, spec.name, dst_d);
		}
	}

	return err;
}

LOCALFUNC tMacErr ResolveNamedChildDir_v2(Dir_R *src_d, StringPtr s,
	Dir_R *dst_d)
{
	tMacErr err;

	err = FindNamedChildDir_v2(src_d, s, dst_d);
	if (mnvm_dirNFErr == err) {
		if (HaveAliasMgrAvail()) {
			err = ResolveAliasDir_v2(src_d, s, dst_d);
		}
	}

	return err;
}

LOCALFUNC tMacErr OpenNamedFileInFolderCStr(Dir_R *d,
	char *s, short *refnum)
{
	Str255 fileName;

	PStrFromCStr(fileName, s);
	return OpenNamedFileInFolder(d, fileName, refnum);
}

LOCALFUNC tMacErr ResolveNamedChildDirCStr(Dir_R *src_d,
	char *s, Dir_R *dst_d)
{
	Str255 fileName;

	PStrFromCStr(fileName, s);
	return ResolveNamedChildDir_v2(src_d, fileName, dst_d);
}

LOCALFUNC tMacErr LoadMacRomFromNameFolder(Dir_R *d,
	char *s)
{
	tMacErr err;
	short refnum;

	if (mnvm_noErr == (err =
		OpenNamedFileInFolderCStr(d, s, &refnum)))
	{
		err = LoadMacRomFromRefNum(refnum);
		(void) FSClose(refnum);
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromPrefDir(void)
{
	tMacErr err;
	Dir_R PrefRef;
	Dir_R GryphelRef;
	Dir_R ROMsRef;

	if (mnvm_noErr == (err = FindPrefFolder(&PrefRef)))
	if (mnvm_noErr == (err = ResolveNamedChildDirCStr(&PrefRef,
		"Gryphel", &GryphelRef)))
	if (mnvm_noErr == (err = ResolveNamedChildDirCStr(&GryphelRef,
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
		LoadMacRomFromNameFolder(&DatDir, RomFileName)))
	if (mnvm_fnfErr == (err =
		LoadMacRomFromPrefDir()))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}

LOCALFUNC bool Sony_InsertIth(int i)
{
	if ((i > 9) || ! FirstFreeDisk(nullpr)) {
		return false;
	} else {
		Str255 s;
		tMacErr err = mnvm_noErr;

		PStrFromCStr(s, "disk?.dsk");

		s[5] = '0' + i;
		if (! CheckSavetMacErr(InsertADiskFromNameEtc(&DatDir, s))) {
			if (mnvm_fnfErr != err) {
				ReportStandardOpenDiskError(err);
			}
			return false;
		}

		return true;
	}
}

LOCALFUNC bool LoadInitialImages(void)
{
	int i;

	for (i = 1; Sony_InsertIth(i); ++i) {
		/* stop on first error (including file not found) */
	}

	return true;
}

#if IncludeSonyNew
LOCALFUNC tMacErr WriteZero(SInt16 refnum, uint32_t L)
{
#define ZeroBufferSize 2048
	tMacErr err;
	uint32_t i;
	uint8_t buffer[ZeroBufferSize];

	if (CheckSaveMacErr(SetFPos(refnum, fsFromStart, 0))) {

		for (i = 0; i < ZeroBufferSize; ++i) {
			buffer[i] = 0;
		}
		while (L > 0) {
			i = (L > ZeroBufferSize) ? ZeroBufferSize : L;
			err = To_tMacErr(FSWrite(refnum, (long *)&i, buffer));
			if (mnvm_noErr != err) {
				goto label_fail;
			}
			L -= i;
		}
	}

label_fail:
	return err;
}
#endif

#if HaveCPUfamM68K && IncludeSonyNew
LOCALPROC MakeNewDiskFromNamevRef(ps3p Name, short vRefNum,
	uint32_t L)
{
	short refNum;
	tMacErr err;

	err = To_tMacErr(Create(Name, vRefNum, '????', '????'));
	if (mnvm_dupFNErr == err) {
		if (CheckSaveMacErr(FSDelete(Name, vRefNum))) {
			err = To_tMacErr(Create(Name, vRefNum, '????', '????'));
		}
	}
	if (mnvm_noErr == err) {
		if (CheckSaveMacErr(FSOpen(Name, vRefNum, &refNum))) {
			if (CheckSaveMacErr(SetEOF(refNum, L))) {
				if (CheckSavetMacErr(WriteZero(refNum, L))) {
					err = Sony_Insert0(refNum, false, Name);
					ReportStandardOpenDiskError(err);
					refNum = NotAfileRef;
				}
			}
			if (NotAfileRef != refNum) {
				(void) FSClose(refNum);
			}
		}
		if (mnvm_noErr != err) {
			(void) FSDelete(Name, vRefNum);
		}
	}
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDiskFromSpec(FSSpec *NewFileSpec,
	uint32_t L)
{
	short refNum;
	tMacErr err;

	err = To_tMacErr(FSpCreate(NewFileSpec,
		'????', '????', smSystemScript));
	if (mnvm_dupFNErr == err) {
		err = To_tMacErr(FSpDelete(NewFileSpec));
		if (mnvm_noErr == err) {
			err = To_tMacErr(FSpCreate(NewFileSpec,
				'????', '????', smSystemScript));
		}
	}
	if (mnvm_noErr == err) {
		if (CheckSaveMacErr(
			FSpOpenDF(NewFileSpec, fsRdWrPerm, &refNum)))
		{
			if (CheckSaveMacErr(SetEOF(refNum, L))) {
				if (CheckSavetMacErr(WriteZero(refNum, L))) {
					err = Sony_Insert0(refNum, false, NULL);
					ReportStandardOpenDiskError(err);
					refNum = NotAfileRef;
				}
			}
			if (NotAfileRef != refNum) {
				(void) FSClose(refNum);
			}
		}
		if (mnvm_noErr != err) {
			(void) FSpDelete(NewFileSpec);
		}
	}
}
#endif

#if UseActvFile || (IncludeSonyNew && ! SaveDialogEnable)
LOCALFUNC tMacErr MakeNamedDir_v2(Dir_R *d, StringPtr s,
	Dir_R *new_d)
{
	tMacErr err;
	HParamBlockRec r;

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioDirID = d->DirId;
	r.fileParam.ioNamePtr = s;
	err = To_tMacErr(PBDirCreateSync(&r));
	if (mnvm_noErr == err) {
		new_d->VRefNum = d->VRefNum;
		new_d->DirId = r.fileParam.ioDirID;
	}

	return err;
}
#endif

#if UseActvFile || (IncludeSonyNew && ! SaveDialogEnable)
LOCALFUNC tMacErr FindOrMakeMakeNamedDir_v2(Dir_R *new_d,
	Dir_R *d, StringPtr s)
{
	tMacErr err;

	err = ResolveNamedChildDir_v2(d, s, new_d);
	if (mnvm_fnfErr == err) {
		err = MakeNamedDir_v2(d, s, new_d);
	}

	return err;
}
#endif

#if UseActvFile || (IncludeSonyNew && ! SaveDialogEnable)
LOCALFUNC tMacErr FindOrMakeChildDirCStr(Dir_R *new_d,
	Dir_R *d, char *name)
{
	Str255 s;

	PStrFromCStr(s, name);
	return FindOrMakeMakeNamedDir_v2(new_d, d, s);
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDisk(uint32_t L, Handle NewDiskName)
{
#if SaveDialogEnable
	OSErr theErr;

#if NavigationAvail
	if (HaveNavServicesAvail()) {
		NavReplyRecord theReply;
		NavDialogOptions dialogOptions;
		NavEventUPP eventUPP = NewNavEventUPP(
			/* (NavEventProcPtr) */ NavigationEventProc);

		theErr = NavGetDefaultDialogOptions(&dialogOptions);
		dialogOptions.dialogOptionFlags |= kNavNoTypePopup;
#if IncludeSonyNameNew
		if (NewDiskName != NULL) {
			PStrFromHandle(dialogOptions.savedFileName,
				NewDiskName, 255);
		}
#endif
		BeginDialog();
		theErr = NavPutFile(NULL, &theReply, &dialogOptions,
			/* NULL */ eventUPP, '????', '????', NULL);
		EndDialog();

		DisposeNavEventUPP(eventUPP);

		if (noErr == theErr) {
			if (theReply.validRecord) {
				long itemsInList;
				AEKeyword keyword;
				DescType typeCode;
				Size actualSize;
				FSSpec NewFileSpec;

				if (noErr ==
					AECountItems(&theReply.selection, &itemsInList))
				if (1 == itemsInList)
				if (noErr == AEGetNthPtr(&theReply.selection,
					1, typeFSS, &keyword, &typeCode,
					(Ptr)&NewFileSpec, sizeof(FSSpec), &actualSize))
				{
					MakeNewDiskFromSpec(&NewFileSpec, L);
				}
			}
			NavDisposeReply(&theReply);
		}
	} else
#endif
	{
		Str255 Title;
		Str255 prompt;

#if IncludeSonyNameNew
		if (NewDiskName != NULL) {
			PStrFromHandle(Title, NewDiskName, 255);
		} else
#endif
		{
			NativeStrFromCStr(Title, "untitled", false);
		}
		NativeStrFromCStr(prompt, "Please select a file", false);

#if HaveCPUfamM68K
		if (! HaveFSSpecCallsAvail()) {
			Point where;
			SFReply reply;

			where.h = 50;
			where.v = 50;
			BeginDialog();
			SFPutFile(*(Point *)&where, prompt, Title, NULL, &reply);
			EndDialog();

			if (reply.good) {
				MakeNewDiskFromNamevRef(reply.fName,
					reply.vRefNum, L);
			}
		} else
#endif
		{
			StandardFileReply reply;

			BeginDialog();
			StandardPutFile(prompt, Title, &reply);
			EndDialog();

			if (reply.sfGood) {
				MakeNewDiskFromSpec(&reply.sfFile, L);
			}
		}
	}
#else /* SaveDialogEnable */
	tMacErr err;
	Str255 Title;
	Dir_R OutDir;
	FSSpec spec;

#if IncludeSonyNameNew
	if (NewDiskName != NULL) {
		PStrFromHandle(Title, NewDiskName, 255);
	} else
#endif
	{
		NativeStrFromCStr(Title, "untitled", false);
	}

	if (mnvm_noErr == (err = FindOrMakeChildDirCStr(&OutDir,
		&DatDir, "out")))
	{
#if HaveCPUfamM68K
		if (! HaveFSSpecCallsAvail()) {
			MakeNewDiskFromNamevRef(Title, OutDir.VRefNum, L);
		} else
#endif
		{
			err = To_tMacErr(FSMakeFSSpec(OutDir.VRefNum, OutDir.DirId,
				Title, &spec));
			if ((mnvm_noErr == err) || (mnvm_fnfErr == err)) {
				MakeNewDiskFromSpec(&spec, L);
			}
		}
	}
#endif /* SaveDialogEnable */
}
#endif

#if UseActvFile

LOCALFUNC tMacErr CreateFile_v2(Dir_R *d, StringPtr s)
{
	tMacErr err;
	HParamBlockRec r;

	r.fileParam.ioFlVersNum = 0;
		/*
			Think reference says to do this,
			but not Inside Mac IV
		*/

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioNamePtr = s;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioFVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = To_tMacErr(PBCreateSync((ParamBlockRec *)&r));
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = To_tMacErr(PBHCreateSync(&r));
	}

	return err;
}

LOCALFUNC tMacErr DeleteFile_v2(Dir_R *d, StringPtr s)
{
	tMacErr err;
	HParamBlockRec r;

	r.fileParam.ioCompletion = NULL;
	r.fileParam.ioVRefNum = d->VRefNum;
	r.fileParam.ioNamePtr = s;
	r.fileParam.ioFVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = To_tMacErr(PBDeleteSync((ParamBlockRec *)&r));
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = To_tMacErr(PBHDeleteSync(&r));
	}

	return err;
}

LOCALFUNC tMacErr CreateFileOverWrite_v2(Dir_R *d, StringPtr s)
{
	tMacErr err;

	err = CreateFile_v2(d, s);
	if (mnvm_dupFNErr == err) {
		if (mnvm_noErr == (err = DeleteFile_v2(d, s))) {
			err = CreateFile_v2(d, s);
		}
	}

	return err;
}

LOCALFUNC tMacErr FileOpen_v2(Dir_R *d, StringPtr s,
	char Permssn, short *refnum)
{
	tMacErr err;
	HParamBlockRec r;

	r.ioParam.ioCompletion = NULL;
	r.ioParam.ioNamePtr = s;
	r.ioParam.ioVRefNum = d->VRefNum;
	r.ioParam.ioPermssn = Permssn;
	r.ioParam.ioMisc = 0; /* use volume buffer */
	r.ioParam.ioVersNum = 0; /* needed if MFS volume */

#if Support64kROM
	if (Have64kROM()) {
		err = To_tMacErr(PBOpenSync((ParamBlockRec *)&r));
	} else
#endif
	{
		r.fileParam.ioDirID = d->DirId;
		err = To_tMacErr(PBHOpenSync(&r));
	}

	if (noErr == err) {
		*refnum = r.ioParam.ioRefNum;
		/*
			Don't change *refnum unless file opened,
			so can initialize to NotAfileRef, and
			compare later before closing in uninit.
		*/
	}
	return err;
}

LOCALFUNC tMacErr FileOpenWrite_v2(Dir_R *d, StringPtr s,
	short *refnum)
{
	return FileOpen_v2(d, s, (char)fsWrPerm, refnum);
}

LOCALFUNC tMacErr OpenOverWriteFile_v2(Dir_R *d, StringPtr s,
	short *refnum)
{
	tMacErr err;

	err = CreateFileOverWrite_v2(d, s);
	if (mnvm_noErr == err) {
		err = FileOpenWrite_v2(d, s, refnum);

		if (mnvm_noErr != err) {
			(void) DeleteFile_v2(d, s);
				/* ignore any error, since already got one */
		}
	}

	return err;
}

LOCALFUNC tMacErr OpenOverWriteFileCStr(Dir_R *d, char *name,
	short *refnum)
{
	Str255 s;

	PStrFromCStr(s, name);
	return OpenOverWriteFile_v2(d, s, refnum);
}

#define ActvCodeFileName "act_1"

LOCALFUNC tMacErr OpenActvCodeFile(short *refnum)
{
	tMacErr err;
	Dir_R PrefRef;
	Dir_R GryphelRef;
	Dir_R ActRef;

	if (mnvm_noErr == (err = FindPrefFolder(&PrefRef)))
	if (mnvm_noErr == (err = ResolveNamedChildDirCStr(&PrefRef,
		"Gryphel", &GryphelRef)))
	if (mnvm_noErr == (err = ResolveNamedChildDirCStr(&GryphelRef,
		"mnvm_act", &ActRef)))
	if (mnvm_noErr == (err = OpenNamedFileInFolderCStr(&ActRef,
		ActvCodeFileName, refnum)))
	{
		/* ok */
	}

	return err;
}

LOCALFUNC tMacErr ActvCodeFileLoad(uint8_t * p)
{
	tMacErr err;
	short refnum;

	if (CheckSavetMacErr(OpenActvCodeFile(&refnum))) {
		long count = ActvCodeFileLen;
		err = To_tMacErr(FSRead(refnum, &count, p));
		(void) FSClose(refnum);
	}

	return err;
}

LOCALFUNC tMacErr ActvCodeFileSave(uint8_t * p)
{
	tMacErr err;
	short refnum;
	Dir_R PrefRef;
	Dir_R GryphelRef;
	Dir_R ActRef;
	long count = ActvCodeFileLen;

	if (mnvm_noErr == (err = FindPrefFolder(&PrefRef)))
	if (mnvm_noErr == (err = FindOrMakeChildDirCStr(&GryphelRef,
		&PrefRef, "Gryphel")))
	if (mnvm_noErr == (err = FindOrMakeChildDirCStr(&ActRef,
		&GryphelRef, "mnvm_act")))
	if (mnvm_noErr == (err = OpenOverWriteFileCStr(&ActRef,
		ActvCodeFileName, &refnum)))
	{
		err = To_tMacErr(FSWrite(refnum, &count, p));
		(void) FSClose(refnum);
	}

	return err;
	/* return mnvm_miscErr; */
}

#endif /* UseActvFile */

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
	if (noErr == (AEGetParamDesc(theAppleEvent,
		keyDirectObject, typeAEList, &docList)))
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

#define NewAEEventHandlerUPP NewAEEventHandlerProc

LOCALFUNC bool InstallEventHandler(AEEventClass theAEEventClass,
	AEEventID theAEEventID, ProcPtr p,
	long handlerRefcon, bool isSysHandler)
{
	return noErr == (AEInstallEventHandler(theAEEventClass,
		theAEEventID,
#if /* useUPP */ 1
		NewAEEventHandlerUPP((AEEventHandlerProcPtr)p),
#else
		(AEEventHandlerUPP)p,
#endif
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

#if EnableDragDrop
static pascal OSErr GlobalTrackingHandler(short message,
	WindowRef pWindow, void *handlerRefCon, DragReference theDragRef)
{
	RgnHandle hilightRgn;
	Rect Bounds;

	UnusedParam(pWindow);
	UnusedParam(handlerRefCon);
	if (! ADialogIsUp) {
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
	}

	return noErr;
}
#endif

#if EnableDragDrop
static DragTrackingHandlerUPP gGlobalTrackingHandler = NULL;
#endif

#if EnableDragDrop
static pascal OSErr GlobalReceiveHandler(WindowRef pWindow,
	void *handlerRefCon, DragReference theDragRef)
{
	unsigned short items;
	unsigned short index;
	ItemReference theItem;
	Size SentSize;
	HFSFlavor r;

	UnusedParam(pWindow);
	UnusedParam(handlerRefCon);
	if (! ADialogIsUp)
	if (noErr == CountDragItems(theDragRef, &items))
	{
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
					InsertADiskOrAliasFromSpec(&r.fileSpec,
						true, true));
			}
		}

		if (gTrueBackgroundFlag) {
			ProcessSerialNumber currentProcess = {0, kCurrentProcess};

			(void) SetFrontProcess(&currentProcess);

			WantCmdOptOnReconnect = true;
		}
	}

	return noErr;
}
#endif

#if EnableDragDrop
static DragReceiveHandlerUPP gGlobalReceiveHandler = NULL;
#endif

#if EnableDragDrop
#define NewDragTrackingHandlerUPP NewDragTrackingHandlerProc
#define NewDragReceiveHandlerUPP NewDragReceiveHandlerProc
#if ! OPAQUE_UPP_TYPES
#define DisposeDragReceiveHandlerUPP(userUPP) \
	DisposeRoutineDescriptor(userUPP)
#define DisposeDragTrackingHandlerUPP(userUPP) \
	DisposeRoutineDescriptor(userUPP)
#else
#define DisposeDragReceiveHandlerUPP DisposeDragReceiveHandlerUPP
#define DisposeDragTrackingHandlerUPP DisposeDragTrackingHandlerUPP
#endif
#endif

#if EnableDragDrop
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
#endif

#if EnableDragDrop
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
#endif

#if EnableMagnify
#define ScaleBuffSzMult (WindowScale * WindowScale)
#endif

LOCALFUNC bool CreateNewWindow(Rect *Bounds, WindowPtr *theWindow)
{
	WindowPtr ResultWin;
	bool IsOk = false;

	ResultWin = NewWindow(
		0L, Bounds, LMGetCurApName() /* "\pMini vMac" */, false,
		noGrowDocProc, /* Could use kWindowSimpleProc for Full Screen */
		(WindowPtr) -1, true, 0);
	if (ResultWin != NULL) {
		*theWindow = ResultWin;

		IsOk = true;
	}

	return IsOk;
}

LOCALPROC ZapWState(void)
{
	gMainWindow = NULL;
	gGlobalReceiveHandler = NULL;
	gGlobalTrackingHandler = NULL;
}

LOCALPROC CloseMainWindow(void)
{
	/*
		Dispose of anything set up by CreateMainWindow.
	*/

#if EnableDragDrop
	UnPrepareForDragging();
#endif

	if (gMainWindow != NULL) {
		DisposeWindow(gMainWindow);
		gMainWindow = NULL;
	}
}

enum {
	kMagStateNormal,
#if EnableMagnify
	kMagStateMagnifgy,
#endif
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
	/*
		Set up somewhere for us to draw the emulated screen and
		receive mouse input. i.e. usually a window, as is the case
		for this port.

		The window should not be resizeable.

		Should look at the current value of UseMagnify and
		UseFullScreen.

	*/
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

#if VarFullScreen
	if (UseFullScreen) {
		_HideMenuBar();
	}  else {
		_ShowMenuBar();
	}
#else
#if MayFullScreen
		_HideMenuBar();
#endif
#endif

	GetGrayRgnBounds(&AllScrnBounds);
	GetScreenBitsBounds(&MainScrnBounds);

#if EnableMagnify
	if (UseMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}
#endif

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

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		ViewHSize = MainScrnBounds.right - MainScrnBounds.left;
		ViewVSize = MainScrnBounds.bottom - MainScrnBounds.top;
#if EnableMagnify
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
	}
#endif

	/* Create window rectangle and centre it on the screen */
	SetRect(&MainScrnBounds, 0, 0, NewWindowWidth, NewWindowHeight);
	OffsetRect(&MainScrnBounds, leftPos, topPos);

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewWinRect = AllScrnBounds;
	}
#endif
#if VarFullScreen
	else
#endif
#if MayNotFullScreen
	{
#if EnableMagnify
		if (UseMagnify) {
			WinIndx = kMagStateMagnifgy;
		} else
#endif
		{
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

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		hOffset = MainScrnBounds.left - AllScrnBounds.left;
		vOffset = MainScrnBounds.top - AllScrnBounds.top;
	}
#endif

	if (CreateNewWindow(&NewWinRect, &gMainWindow)) {
		ShowWindow(gMainWindow);

		/* check if window rect valid */
#if VarFullScreen
		if (! UseFullScreen)
#endif
#if MayNotFullScreen
		{
			Rect tr;

			if (GetWindowTitleBounds(gMainWindow, &tr)) {
				if (! RectInRgn(&tr, _GetGrayRgn())) {
					SetMacWindContRect(gMainWindow,
						&MainScrnBounds);
					if (GetWindowTitleBounds(gMainWindow, &tr)) {
						if (! RectInRgn(&tr, _GetGrayRgn())) {
							OffsetRect(&MainScrnBounds,
								0, AllScrnBounds.top - tr.top);
							SetMacWindContRect(gMainWindow,
								&MainScrnBounds);
						}
					}
				}
			}
		}
#endif

#if EnableDragDrop
		if (HaveDragMgrAvail()) {
			(void) PrepareForDragging();
		}
#endif

		IsOk = true;
	}

	return IsOk;
}

struct WState {
	WindowPtr f_MainWindow;
#if MayFullScreen
	short f_hOffset;
	short f_vOffset;
	uint16_t f_ViewHSize;
	uint16_t f_ViewVSize;
	uint16_t f_ViewHStart;
	uint16_t f_ViewVStart;
#endif
#if VarFullScreen
	bool f_UseFullScreen;
#endif
#if EnableMagnify
	bool f_UseMagnify;
#endif
#if MayNotFullScreen
	int f_CurWinIndx;
#endif
	DragTrackingHandlerUPP f_gGlobalTrackingHandler;
	DragReceiveHandlerUPP f_gGlobalReceiveHandler;
};
typedef struct WState WState;

LOCALPROC GetWState(WState *r)
{
	r->f_MainWindow = gMainWindow;
#if MayFullScreen
	r->f_hOffset = hOffset;
	r->f_vOffset = vOffset;
	r->f_ViewHSize = ViewHSize;
	r->f_ViewVSize = ViewVSize;
	r->f_ViewHStart = ViewHStart;
	r->f_ViewVStart = ViewVStart;
#endif
#if VarFullScreen
	r->f_UseFullScreen = UseFullScreen;
#endif
#if EnableMagnify
	r->f_UseMagnify = UseMagnify;
#endif
#if MayNotFullScreen
	r->f_CurWinIndx = CurWinIndx;
#endif
	r->f_gGlobalTrackingHandler = gGlobalTrackingHandler;
	r->f_gGlobalReceiveHandler = gGlobalReceiveHandler;
}

LOCALPROC SetWState(WState *r)
{
	gMainWindow = r->f_MainWindow;
#if MayFullScreen
	hOffset = r->f_hOffset;
	vOffset = r->f_vOffset;
	ViewHSize = r->f_ViewHSize;
	ViewVSize = r->f_ViewVSize;
	ViewHStart = r->f_ViewHStart;
	ViewVStart = r->f_ViewVStart;
#endif
#if VarFullScreen
	UseFullScreen = r->f_UseFullScreen;
#endif
#if EnableMagnify
	UseMagnify = r->f_UseMagnify;
#endif
#if MayNotFullScreen
	CurWinIndx = r->f_CurWinIndx;
#endif
	gGlobalTrackingHandler = r->f_gGlobalTrackingHandler;
	gGlobalReceiveHandler = r->f_gGlobalReceiveHandler;
}

LOCALFUNC bool ReCreateMainWindow(void)
{
	/*
		Like CreateMainWindow (which it calls), except may be
		called when already have window, without CloseMainWindow
		being called first. (Usually with different
		values of WantMagnify and WantFullScreen than
		on the previous call.)

		If there is existing window, and fail to create
		the new one, then existing window must be left alone,
		in valid state. (and return false. otherwise,
		if succeed, return true)

		i.e. can allocate the new one before disposing
		of the old one.
	*/
	WState old_state;
	WState new_state;

#if VarFullScreen
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

	GetWState(&old_state);

	ZapWState();

#if VarFullScreen
	UseFullScreen = WantFullScreen;
#endif
#if EnableMagnify
	UseMagnify = WantMagnify;
#endif

	ColorTransValid = false;

	if (! CreateMainWindow()) {
		CloseMainWindow();
		SetWState(&old_state);

#if VarFullScreen
		if (UseFullScreen) {
			_HideMenuBar();
		} else {
			_ShowMenuBar();
		}
#endif

		/* avoid retry */
#if VarFullScreen
		WantFullScreen = UseFullScreen;
#endif
#if EnableMagnify
		WantMagnify = UseMagnify;
#endif

		return false;
	} else {
		GetWState(&new_state);
		SetWState(&old_state);
		CloseMainWindow();
		SetWState(&new_state);

		if (HaveCursorHidden) {
			(void) MoveMouse(CurMouseH, CurMouseV);
			WantCursorHidden = true;
		}

		return true;
	}
}

#if VarFullScreen && EnableMagnify
enum {
	kWinStateWindowed,
#if EnableMagnify
	kWinStateFullScreen,
#endif
	kNumWinStates
};
#endif

#if VarFullScreen && EnableMagnify
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
#if VarFullScreen && EnableMagnify
	{
		int i;

		for (i = 0; i < kNumWinStates; ++i) {
			WinMagStates[i] = kMagStateAuto;
		}
	}
#endif
}

#if VarFullScreen
LOCALPROC ToggleWantFullScreen(void)
{
	WantFullScreen = ! WantFullScreen;

#if EnableMagnify
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
				Rect r;

				GetScreenBitsBounds(&r);
				if (((r.right - r.left)
					>= vMacScreenWidth * WindowScale)
					&& ((r.bottom - r.top)
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

LOCALPROC LeaveBackground(void)
{
#if HogCPU
	NoEventsCounter = 0;
#endif

	SetCursorArrow();
	ReconnectKeyCodes3();
}

LOCALPROC EnterBackground(void)
{
	DisconnectKeyCodes3();

#if VarFullScreen
	if (WantFullScreen) {
		ToggleWantFullScreen();
	}
#endif
}

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

	if (CurSpeedStopped != (SpeedStopped ||
		(gBackgroundFlag && ! RunInBackground
#if EnableAutoSlow && 0
			&& (QuietSubTicks >= 4092)
#endif
		)))
	{
		CurSpeedStopped = ! CurSpeedStopped;
		if (CurSpeedStopped) {
			EnterSpeedStopped();
		} else {
			LeaveSpeedStopped();
		}
	}

#if EnableRecreateW
	if (! (gTrueBackgroundFlag)) {
		if (0
#if EnableMagnify
			|| (UseMagnify != WantMagnify)
#endif
#if VarFullScreen
			|| (UseFullScreen != WantFullScreen)
#endif
			)
		{
			(void) ReCreateMainWindow();
#if HogCPU
			NoEventsCounter = 0;
#endif
		}
	}
#endif

#if MayFullScreen
	if (GrabMachine != (
#if VarFullScreen
		UseFullScreen &&
#endif
		! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		GrabMachine = ! GrabMachine;
		AdjustMachineGrab();
	}
#endif

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}

	if (gTrueBackgroundFlag) {
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
				MakeNewDisk(vSonyNewDiskSize,
					PbufDat[vSonyNewDiskName]);
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

	if (HaveCursorHidden != (WantCursorHidden
		&& ! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		if (HaveCursorHidden) {
			HideCursor();
		} else {
			ShowCursor();
		}
	}
}

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

#define CheckItem CheckMenuItem

/* Menu Constants */

#define kAppleMenu   128
#define kFileMenu    129
#define kSpecialMenu 130

/* Apple */

enum {
	kAppleNull,

	kAppleAboutItem,
	kAppleSep1,

	kNumAppleItems
};

/* File */

enum {
	kFileNull,

	kFileOpenDiskImage,
	kFileSep1,
	kFileQuitItem,

	kNumFileItems
};

/* Special */

enum {
	kSpecialNull,

	kSpecialMoreCommandsItem,

	kNumSpecialItems
};

LOCALPROC DoOpenDA(short menuItem)
{
	Str32 name;
	GrafPtr savePort;

	GetPort(&savePort);
	GetMenuItemText(GetMenuHandle(kAppleMenu), menuItem, name);
	OpenDeskAcc(name);
	SystemTask();
	SetPort(savePort);
}

LOCALPROC MacOS_HandleMenu(short menuID, short menuItem)
{
	switch (menuID) {
		case kAppleMenu:
			if (kAppleAboutItem == menuItem) {
				DoAboutMsg();
			} else {
				DoOpenDA(menuItem);
			}
			break;

		case kFileMenu:
			switch (menuItem) {
				case kFileOpenDiskImage:
					RequestInsertDisk = true;
					break;

				case kFileQuitItem:
					RequestMacOff = true;
					break;
			}
			break;

		case kSpecialMenu:
			switch (menuItem) {
				case kSpecialMoreCommandsItem:
					DoMoreCommandsMsg();
					break;
			}
			break;

		default:
			/* if 0 == menuID, then no command chosen from menu */
			/* do nothing */
			break;
	}
}

LOCALPROC HandleMacEvent(EventRecord *theEvent)
{
	WindowPtr whichWindow;
	GrafPtr savePort;

	switch(theEvent->what) {
		case mouseDown:
			switch (FindWindow(theEvent->where, &whichWindow)) {
				case inSysWindow:
					SystemClick(theEvent, whichWindow);
					break;
				case inMenuBar:
					ForceShowCursor();
					{
						long menuSelection =
							MenuSelect(theEvent->where);
						MacOS_HandleMenu(HiWord(menuSelection),
							LoWord(menuSelection));
					}
					HiliteMenu(0);
					break;

				case inDrag:
					{
						Rect r;

						GetScreenBitsBounds(&r);
						DragWindow(whichWindow, theEvent->where, &r);
					}
					break;

				case inContent:
					if (FrontWindow() != whichWindow) {
						SelectWindow(whichWindow);
					}
					if (whichWindow == gMainWindow) {
						MousePositionNotifyFromGlobal(theEvent->where);
						MouseButtonSet(true);
					}
					break;

				case inGoAway:
					if (TrackGoAway(whichWindow, theEvent->where)) {
						RequestMacOff = true;
					}
					break;

				case inZoomIn:
				case inZoomOut:
					/* Zoom Boxes */
					break;
			}
			break;
		case mouseUp:
			MousePositionNotifyFromGlobal(theEvent->where);
			MouseButtonSet(false);
			break;

		case updateEvt:
			GetPort(&savePort);
			BeginUpdate((WindowPtr) theEvent->message);

			if ((WindowPtr)theEvent->message == gMainWindow) {
				Update_Screen();
			}

			EndUpdate((WindowPtr) theEvent->message);
			SetPort(savePort);
			break;

		case keyDown:
		case autoKey:
		case keyUp:
			/* ignore it */
			break;
		case osEvt:
			if ((theEvent->message >> 24) & suspendResumeMessage) {
				if (theEvent->message & 1) {
					gTrueBackgroundFlag = false;
				} else {
					gTrueBackgroundFlag = true;
				}
			}
			break;
		case kHighLevelEvent:
			if (kCoreEventClass == (AEEventClass)theEvent->message) {
				if (/* CheckSysCode */ noErr ==
					(AEProcessAppleEvent(theEvent)))
				{
				}
			} else {
				/* vCheckSysCode(errAENotAppleEvent); */
			}
			break;
	}
}

LOCALPROC WaitForTheNextEvent(void)
{
	/*
		Wait for the next event
		from the operating system, we have nothing better
		to do. Call HandleTheEvent and return (only
		wait for one event).
	*/

	EventRecord theEvent;

	if (
#if HaveCPUfamM68K
		(! HaveWaitNextEventAvail()) ?
		GetNextEvent(everyEvent, &theEvent) :
#endif
		WaitNextEvent(everyEvent, &theEvent,
			(gTrueBackgroundFlag && ! RunInBackground)
				? 5 * 60 * 60
				: 5,
				/*
					still need to check for
					control key when SpeedStopped,
					don't get event
				*/
			NULL))
	{
		HandleMacEvent(&theEvent);
	}
}

LOCALPROC DontWaitForEvent(void)
{
	/* we're busy, but see what system wants */

	EventRecord theEvent;
	int i = 0;

#if 0 /* this seems to cause crashes on some machines */
	if (EventAvail(everyEvent, &theEvent)) {
		NoEventsCounter = 0;
#endif

		while ((
#if HaveCPUfamM68K
			(! HaveWaitNextEventAvail()) ?
			GetNextEvent(everyEvent, &theEvent) :
#endif
			WaitNextEvent(everyEvent, &theEvent, 0, NULL))
			&& (i < 10))
		{
			HandleMacEvent(&theEvent);
#if HogCPU
			NoEventsCounter = 0;
#endif
			++i;
		}
#if 0
	}
#endif
}

#define PrivateEventMask \
	(mDownMask | mUpMask | keyDownMask | keyUpMask | autoKeyMask)

#define IsPowOf2(x) (0 == ((x) & ((x) - 1)))

LOCALPROC CheckForSystemEvents(void)
{
	/*
		Handle any events that are waiting for us.
		Return immediately when no more events
		are waiting, don't wait for more.
	*/

#if HogCPU && MayFullScreen
	/*
		only hog cpu in full screen mode
	*/
	if (
#if VarFullScreen
		UseFullScreen &&
#endif
		((uint8_t) -1 == SpeedValue) && ! CurSpeedStopped)
	{
		EventRecord theEvent;

		if (! OSEventAvail(everyEvent, &theEvent)) {
			/*
				if no OSEvent now, and not looking for aftermath of
				event, assume there is no event of any kind we need
				to look at
			*/
			if (NoEventsCounter < 256) {
				++NoEventsCounter;
				if (IsPowOf2(NoEventsCounter)) {
					DontWaitForEvent();
				}
			}
		} else {
			WindowPtr whichWindow;

			bool PrivateEvent = false;
			switch (theEvent.what) {
				case keyDown:
				case autoKey:
				case keyUp:
				case mouseUp:
					PrivateEvent = true;
					break;
				case mouseDown:
					if ((inContent ==
							FindWindow(theEvent.where, &whichWindow))
						&& (whichWindow == gMainWindow)
						&& (FrontWindow() == whichWindow))
					{
						PrivateEvent = true;
					}
					break;
			}
			if (PrivateEvent) {
				/*
					if event can effect only us, and not looking out
					for aftermath of another event, then hog the cpu
				*/
				if (GetOSEvent(PrivateEventMask, &theEvent)) {
					HandleMacEvent(&theEvent);
				}
			} else {
				NoEventsCounter = 0;
				/*
					Have an Event, so reset NoEventsCounter, no matter
					what. WaitNextEvent can return false, even if it did
					handle an event. Such as a click in the collapse
					box. In this case we need to look out for update
					events.
				*/
				DontWaitForEvent();
			}
		}
	} else
#endif
	{
		DontWaitForEvent();
	}

	if (! gBackgroundFlag) {
		CheckKeyBoardState();
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

	/*
		Wait until the end of the current
		tick, then emulate the next tick.
	*/

	if (ExtraTimeNotOver()) {
#if HaveCPUfamM68K
		if (HaveWaitNextEventAvail())
#endif
		{
			EventRecord theEvent;

			if (WaitNextEvent(everyEvent, &theEvent, 1, NULL)) {
				HandleMacEvent(&theEvent);
#if HogCPU
				NoEventsCounter = 0;
#endif
			}
		}
		goto label_retry;
	}

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	if (! (gBackgroundFlag)) {
		CheckMouseState();
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif
}

#include "PROGMAIN.h"

LOCALPROC AppendMenuCStr(MenuHandle menu, char *s)
{
	Str255 t;

	PStrFromCStr(t, s);
	AppendMenu(menu, t);
}

LOCALPROC AppendMenuConvertCStr(MenuHandle menu,
	char *s, bool WantEllipsis)
{
	Str255 t;

	NativeStrFromCStr(t, s, WantEllipsis);
	AppendMenu(menu, t);
}

LOCALPROC AppendMenuSep(MenuHandle menu)
{
	AppendMenuCStr(menu, "(-");
}

LOCALFUNC MenuHandle NewMenuFromConvertCStr(short menuID, char *s)
{
	Str255 r;

	NativeStrFromCStr(r, s, false);
	return NewMenu(menuID, r);
}

LOCALFUNC bool InstallOurMenus(void)
{
	MenuHandle menu;
	Str255 s;

	PStrFromChar(s, (char)20);
	menu = NewMenu(kAppleMenu, s);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kStrMenuItemAbout, true);
		AppendMenuSep(menu);
		AppendResMenu(menu, 'DRVR');
		InsertMenu(menu, 0);
	}

	menu = NewMenuFromConvertCStr(kFileMenu, kStrMenuFile);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kStrMenuItemOpen, true);
		{
			AppendMenuSep(menu);
			AppendMenuConvertCStr(menu,
				kStrMenuItemQuit, false);
		}
		InsertMenu(menu, 0);
	}

	menu = NewMenuFromConvertCStr(kSpecialMenu, kStrMenuSpecial);
	if (menu != NULL) {
		AppendMenuConvertCStr(menu,
			kStrMenuItemMore, true);
		InsertMenu(menu, 0);
	}

	DrawMenuBar();

	return true;
}

#if AppearanceAvail
LOCALFUNC bool InstallOurAppearanceClient(void)
{
	if (HaveAppearanceAvail()) {
		RegisterAppearanceClient();
	}
	return true;
}
#endif

LOCALFUNC bool InstallOurEventHandlers(void)
{
	InitKeyCodes();

	if (HaveAppleEvtMgrAvail()) {
		InstallAppleEventHandlers();
	}
	return true;
}

LOCALPROC ZapOSGLUVars(void)
{
	/*
		Set initial values of variables for
		platform dependent code, where not
		done using c initializers. (such
		as for arrays.)
	*/

	ZapEmKeys();
	InitDrives();
	ZapWinStateVars();
}

LOCALPROC ReserveAllocAll(void)
{
	/* !! must match ChooseTotMemSize in build system !! */

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
#if EnableMagnify
	ReserveAllocOneBlock(&ScalingBuff,
		vMacScreenNumBytes * (ScaleBuffSzMult), 5, false);
	ReserveAllocOneBlock(&ScalingTabl,
		ScalingTablsz, 5, false);
#endif
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
	ReserveAllocBigBlock = (uint8_t *)NewPtr(n);
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
	/*
		run all the initializations
		needed for the program.
	*/

	if (InitMacManagers())
	if (AllocMemory())
	if (InitApplInfo())
#if dbglog_HAVE
	if (dbglog_open())
#endif
#if AppearanceAvail
	if (InstallOurAppearanceClient())
#endif
	if (InstallOurEventHandlers())
	if (InstallOurMenus())
#if SoundEnabled
	if (Sound_Init())
#endif
	if (ReCreateMainWindow())
	if (LoadMacRom())
	if (LoadInitialImages())
	if (InitLocationDat())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

LOCALPROC UnInitOSGLU(void)
{
	/*
		Do all clean ups needed
		before the program quits.
	*/

	if (MacMsgDisplayed) {
		MacMsgDisplayOff();
	}

#if MayFullScreen
	UngrabMachine();
#endif

#if SoundEnabled
	Sound_Stop();
#endif

	CloseMainWindow();

#if MayFullScreen
	_ShowMenuBar();
#endif

#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

#if dbglog_HAVE
	dbglog_close();
#endif

	ForceShowCursor();

	if (! gTrueBackgroundFlag) {
		CheckSavedMacMsg();
	}
}

#ifndef MainReturnsInt
#define MainReturnsInt 0
#endif

#ifndef NeedLongGlue
#define NeedLongGlue 0
#endif

#if NeedLongGlue
#define main long_main
#endif

#if MainReturnsInt
int
#else
void
#endif
main(void)
{
	ZapOSGLUVars();
	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

#if MainReturnsInt
	return 0;
#endif
}
