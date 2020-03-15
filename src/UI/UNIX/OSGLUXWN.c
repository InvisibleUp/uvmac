/*
	OSGLUXWN.c

	Copyright (C) 2009 Michael Hanni, Christian Bauer,
	Stephan Kochen, Paul C. Pratt, and others

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
	Operating System GLUe for X WiNdow system

	All operating system dependent code for the
	X Window System should go here.

	This code is descended from Michael Hanni's X
	port of vMac, by Philip Cummins.
	I learned more about how X programs work by
	looking at other programs such as Basilisk II,
	the UAE Amiga Emulator, Bochs, QuakeForge,
	DooM Legacy, and the FLTK. A few snippets
	from them are used here.

	Drag and Drop support is based on the specification
	"XDND: Drag-and-Drop Protocol for the X Window System"
	developed by John Lindal at New Planet Software, and
	looking at included examples, one by Paul Sheer.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "CNFGGLOB.h"
#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "STRCONST.h"

/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

/* --- control mode and internationalization --- */

#define NeedCell2PlainAsciiMap 1

#include "LANG/INTLCHAR.h"


LOCALVAR char *d_arg = NULL;
LOCALVAR char *n_arg = NULL;

#ifdef CanGetAppPath
LOCALVAR char *app_parent = NULL;
LOCALVAR char *app_name = NULL;
#endif

LOCALFUNC tMacErr ChildPath(char *x, char *y, char **r)
{
	tMacErr err = mnvm_miscErr;
	int nx = strlen(x);
	int ny = strlen(y);
	{
		if ((nx > 0) && ('/' == x[nx - 1])) {
			--nx;
		}
		{
			int nr = nx + 1 + ny;
			char *p = malloc(nr + 1);
			if (p != NULL) {
				char *p2 = p;
				(void) memcpy(p2, x, nx);
				p2 += nx;
				*p2++ = '/';
				(void) memcpy(p2, y, ny);
				p2 += ny;
				*p2 = 0;
				*r = p;
				err = mnvm_noErr;
			}
		}
	}

	return err;
}

#if IncludeSonyNew
LOCALFUNC tMacErr FindOrMakeChild(char *x, char *y, char **r)
{
	tMacErr err;
	struct stat folder_info;
	char *r0;

	if (mnvm_noErr == (err = ChildPath(x, y, &r0))) {
		if (0 != stat(r0, &folder_info)) {
			if (0 != mkdir(r0, S_IRWXU)) {
				err = mnvm_miscErr;
			} else {
				*r = r0;
				err = mnvm_noErr;
			}
		} else {
			if (! S_ISDIR(folder_info.st_mode)) {
				err = mnvm_miscErr;
			} else {
				*r = r0;
				err = mnvm_noErr;
			}
		}
	}

	return err;
}
#endif

LOCALPROC MayFree(char *p)
{
	if (NULL != p) {
		free(p);
	}
}

/* --- sending debugging info to file --- */

#if dbglog_HAVE

#define dbglog_ToStdErr 0

#if ! dbglog_ToStdErr
LOCALVAR FILE *dbglog_File = NULL;
#endif

LOCALFUNC bool dbglog_open0(void)
{
#if dbglog_ToStdErr
	return true;
#else
	dbglog_File = fopen("dbglog.txt", "w");
	return (NULL != dbglog_File);
#endif
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
#if dbglog_ToStdErr
	(void) fwrite(s, 1, L, stderr);
#else
	if (dbglog_File != NULL) {
		(void) fwrite(s, 1, L, dbglog_File);
	}
#endif
}

LOCALPROC dbglog_close0(void)
{
#if ! dbglog_ToStdErr
	if (dbglog_File != NULL) {
		fclose(dbglog_File);
		dbglog_File = NULL;
	}
#endif
}

#endif

/* --- debug settings and utilities --- */

#if ! dbglog_HAVE
#define WriteExtraErr(s)
#else
LOCALPROC WriteExtraErr(char *s)
{
	dbglog_writeCStr("*** error: ");
	dbglog_writeCStr(s);
	dbglog_writeReturn();
}
#endif

LOCALVAR Display *x_display = NULL;

#define DbgEvents (dbglog_HAVE && 0)

#if DbgEvents
LOCALPROC WriteDbgAtom(char *s, Atom x)
{
	char *name = XGetAtomName(x_display, x);
	if (name != NULL) {
		dbglog_writeCStr("Atom ");
		dbglog_writeCStr(s);
		dbglog_writeCStr(": ");
		dbglog_writeCStr(name);
		dbglog_writeReturn();
		XFree(name);
	}
}
#endif

/* --- information about the environment --- */

LOCALVAR Atom XA_DeleteW = (Atom)0;
#if EnableDragDrop
LOCALVAR Atom XA_UriList = (Atom)0;
LOCALVAR Atom XA_DndAware = (Atom)0;
LOCALVAR Atom XA_DndEnter = (Atom)0;
LOCALVAR Atom XA_DndLeave = (Atom)0;
LOCALVAR Atom XA_DndDrop = (Atom)0;
LOCALVAR Atom XA_DndPosition = (Atom)0;
LOCALVAR Atom XA_DndStatus = (Atom)0;
LOCALVAR Atom XA_DndActionCopy = (Atom)0;
LOCALVAR Atom XA_DndActionPrivate = (Atom)0;
LOCALVAR Atom XA_DndSelection = (Atom)0;
LOCALVAR Atom XA_DndFinished = (Atom)0;
LOCALVAR Atom XA_MinivMac_DndXchng = (Atom)0;
LOCALVAR Atom XA_NetActiveWindow = (Atom)0;
LOCALVAR Atom XA_NetSupported = (Atom)0;
#endif
#if IncludeHostTextClipExchange
LOCALVAR Atom XA_CLIPBOARD = (Atom)0;
LOCALVAR Atom XA_TARGETS = (Atom)0;
LOCALVAR Atom XA_MinivMac_Clip = (Atom)0;
#endif

LOCALPROC LoadXA(void)
{
	XA_DeleteW = XInternAtom(x_display, "WM_DELETE_WINDOW", False);
#if EnableDragDrop
	XA_UriList = XInternAtom (x_display, "text/uri-list", False);
	XA_DndAware = XInternAtom (x_display, "XdndAware", False);
	XA_DndEnter = XInternAtom(x_display, "XdndEnter", False);
	XA_DndLeave = XInternAtom(x_display, "XdndLeave", False);
	XA_DndDrop = XInternAtom(x_display, "XdndDrop", False);
	XA_DndPosition = XInternAtom(x_display, "XdndPosition", False);
	XA_DndStatus = XInternAtom(x_display, "XdndStatus", False);
	XA_DndActionCopy = XInternAtom(x_display,
		"XdndActionCopy", False);
	XA_DndActionPrivate = XInternAtom(x_display,
		"XdndActionPrivate", False);
	XA_DndSelection = XInternAtom(x_display, "XdndSelection", False);
	XA_DndFinished = XInternAtom(x_display, "XdndFinished", False);
	XA_MinivMac_DndXchng = XInternAtom(x_display,
		"_MinivMac_DndXchng", False);
	XA_NetActiveWindow = XInternAtom(x_display,
		"_NET_ACTIVE_WINDOW", False);
	XA_NetSupported = XInternAtom(x_display,
		"_NET_SUPPORTED", False);
#endif
#if IncludeHostTextClipExchange
	XA_CLIPBOARD = XInternAtom(x_display, "CLIPBOARD", False);
	XA_TARGETS = XInternAtom(x_display, "TARGETS", False);
	XA_MinivMac_Clip = XInternAtom(x_display,
		"_MinivMac_Clip", False);
#endif
}

#if EnableDragDrop
LOCALFUNC bool NetSupportedContains(Atom x)
{
	/*
		Note that the window manager could be replaced at
		any time, so don't cache results of this function.
	*/
	Atom ret_type;
	int ret_format;
	unsigned long ret_item;
	unsigned long remain_byte;
	unsigned long i;
	unsigned char *s = 0;
	bool foundit = false;
	Window rootwin = XRootWindow(x_display,
		DefaultScreen(x_display));

	if (Success != XGetWindowProperty(x_display, rootwin,
		XA_NetSupported,
		0, 65535, False, XA_ATOM, &ret_type, &ret_format,
		&ret_item, &remain_byte, &s))
	{
		WriteExtraErr("XGetWindowProperty failed");
	} else if (! s) {
		WriteExtraErr("XGetWindowProperty failed");
	} else if (ret_type != XA_ATOM) {
		WriteExtraErr("XGetWindowProperty returns wrong type");
	} else {
		Atom *v = (Atom *)s;

		for (i = 0; i < ret_item; ++i) {
			if (v[i] == x) {
				foundit = true;
				/* fprintf(stderr, "found the hint\n"); */
			}
		}
	}
	if (s) {
		XFree(s);
	}
	return foundit;
}
#endif

#define WantColorTransValid 1

#include "UI/COMOSGLU.h"
#include "UTIL/PBUFSTDC.h"
#include "UI/CONTROLM.h"

/* --- text translation --- */

#if IncludePbufs
/* this is table for Windows, any changes needed for X? */
LOCALVAR const uint8_t Native2MacRomanTab[] = {
	0xAD, 0xB0, 0xE2, 0xC4, 0xE3, 0xC9, 0xA0, 0xE0,
	0xF6, 0xE4, 0xB6, 0xDC, 0xCE, 0xB2, 0xB3, 0xB7,
	0xB8, 0xD4, 0xD5, 0xD2, 0xD3, 0xA5, 0xD0, 0xD1,
	0xF7, 0xAA, 0xC5, 0xDD, 0xCF, 0xB9, 0xC3, 0xD9,
	0xCA, 0xC1, 0xA2, 0xA3, 0xDB, 0xB4, 0xBA, 0xA4,
	0xAC, 0xA9, 0xBB, 0xC7, 0xC2, 0xBD, 0xA8, 0xF8,
	0xA1, 0xB1, 0xC6, 0xD7, 0xAB, 0xB5, 0xA6, 0xE1,
	0xFC, 0xDA, 0xBC, 0xC8, 0xDE, 0xDF, 0xF0, 0xC0,
	0xCB, 0xE7, 0xE5, 0xCC, 0x80, 0x81, 0xAE, 0x82,
	0xE9, 0x83, 0xE6, 0xE8, 0xED, 0xEA, 0xEB, 0xEC,
	0xF5, 0x84, 0xF1, 0xEE, 0xEF, 0xCD, 0x85, 0xF9,
	0xAF, 0xF4, 0xF2, 0xF3, 0x86, 0xFA, 0xFB, 0xA7,
	0x88, 0x87, 0x89, 0x8B, 0x8A, 0x8C, 0xBE, 0x8D,
	0x8F, 0x8E, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,
	0xFD, 0x96, 0x98, 0x97, 0x99, 0x9B, 0x9A, 0xD6,
	0xBF, 0x9D, 0x9C, 0x9E, 0x9F, 0xFE, 0xFF, 0xD8
};
#endif

#if IncludePbufs
LOCALFUNC tMacErr NativeTextToMacRomanPbuf(char *x, tPbuf *r)
{
	if (NULL == x) {
		return mnvm_miscErr;
	} else {
		uint8_t * p;
		uint32_t L = strlen(x);

		p = (uint8_t *)malloc(L);
		if (NULL == p) {
			return mnvm_miscErr;
		} else {
			uint8_t *p0 = (uint8_t *)x;
			uint8_t *p1 = (uint8_t *)p;
			int i;

			for (i = L; --i >= 0; ) {
				uint8_t v = *p0++;
				if (v >= 128) {
					v = Native2MacRomanTab[v - 128];
				} else if (10 == v) {
					v = 13;
				}
				*p1++ = v;
			}

			return PbufNewFromPtr(p, L, r);
		}
	}
}
#endif

#if IncludePbufs
/* this is table for Windows, any changes needed for X? */
LOCALVAR const uint8_t MacRoman2NativeTab[] = {
	0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1,
	0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
	0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3,
	0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
	0x86, 0xB0, 0xA2, 0xA3, 0xA7, 0x95, 0xB6, 0xDF,
	0xAE, 0xA9, 0x99, 0xB4, 0xA8, 0x80, 0xC6, 0xD8,
	0x81, 0xB1, 0x8D, 0x8E, 0xA5, 0xB5, 0x8A, 0x8F,
	0x90, 0x9D, 0xA6, 0xAA, 0xBA, 0xAD, 0xE6, 0xF8,
	0xBF, 0xA1, 0xAC, 0x9E, 0x83, 0x9A, 0xB2, 0xAB,
	0xBB, 0x85, 0xA0, 0xC0, 0xC3, 0xD5, 0x8C, 0x9C,
	0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xF7, 0xB3,
	0xFF, 0x9F, 0xB9, 0xA4, 0x8B, 0x9B, 0xBC, 0xBD,
	0x87, 0xB7, 0x82, 0x84, 0x89, 0xC2, 0xCA, 0xC1,
	0xCB, 0xC8, 0xCD, 0xCE, 0xCF, 0xCC, 0xD3, 0xD4,
	0xBE, 0xD2, 0xDA, 0xDB, 0xD9, 0xD0, 0x88, 0x98,
	0xAF, 0xD7, 0xDD, 0xDE, 0xB8, 0xF0, 0xFD, 0xFE
};
#endif

#if IncludePbufs
LOCALFUNC bool MacRomanTextToNativePtr(tPbuf i, bool IsFileName,
	uint8_t * *r)
{
	uint8_t * p;
	void *Buffer = PbufDat[i];
	uint32_t L = PbufSize[i];

	p = (uint8_t *)malloc(L + 1);
	if (p != NULL) {
		uint8_t *p0 = (uint8_t *)Buffer;
		uint8_t *p1 = (uint8_t *)p;
		int j;

		if (IsFileName) {
			for (j = L; --j >= 0; ) {
				uint8_t x = *p0++;
				if (x < 32) {
					x = '-';
				} else if (x >= 128) {
					x = MacRoman2NativeTab[x - 128];
				} else {
					switch (x) {
						case '/':
						case '<':
						case '>':
						case '|':
						case ':':
							x = '-';
						default:
							break;
					}
				}
				*p1++ = x;
			}
			if ('.' == p[0]) {
				p[0] = '-';
			}
		} else {
			for (j = L; --j >= 0; ) {
				uint8_t x = *p0++;
				if (x >= 128) {
					x = MacRoman2NativeTab[x - 128];
				} else if (13 == x) {
					x = '\n';
				}
				*p1++ = x;
			}
		}
		*p1 = 0;

		*r = p;
		return true;
	}
	return false;
}
#endif

LOCALPROC NativeStrFromCStr(char *r, char *s)
{
	uint8_t ps[ClStrMaxLength];
	int i;
	int L;

	ClStrFromSubstCStr(&L, ps, s);

	for (i = 0; i < L; ++i) {
		r[i] = Cell2PlainAsciiMap[ps[i]];
	}

	r[L] = 0;
}

/* --- drives --- */

#define NotAfileRef NULL

LOCALVAR FILE *Drives[NumDrives]; /* open disk image files */
#if IncludeSonyGetName || IncludeSonyNew
LOCALVAR char *DriveNames[NumDrives];
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
#if IncludeSonyGetName || IncludeSonyNew
		DriveNames[i] = NULL;
#endif
	}
}

GLOBALOSGLUFUNC tMacErr vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	tMacErr err = mnvm_miscErr;
	FILE *refnum = Drives[Drive_No];
	uint32_t NewSony_Count = 0;

	if (0 == fseek(refnum, Sony_Start, SEEK_SET)) {
		if (IsWrite) {
			NewSony_Count = fwrite(Buffer, 1, Sony_Count, refnum);
		} else {
			NewSony_Count = fread(Buffer, 1, Sony_Count, refnum);
		}

		if (NewSony_Count == Sony_Count) {
			err = mnvm_noErr;
		}
	}

	if (nullpr != Sony_ActCount) {
		*Sony_ActCount = NewSony_Count;
	}

	return err; /*& figure out what really to return &*/
}

GLOBALOSGLUFUNC tMacErr vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	tMacErr err = mnvm_miscErr;
	FILE *refnum = Drives[Drive_No];
	long v;

	if (0 == fseek(refnum, 0, SEEK_END)) {
		v = ftell(refnum);
		if (v >= 0) {
			*Sony_Count = v;
			err = mnvm_noErr;
		}
	}

	return err; /*& figure out what really to return &*/
}

#ifndef HaveAdvisoryLocks
#define HaveAdvisoryLocks 1
#endif

/*
	What is the difference between fcntl(fd, F_SETLK ...
	and flock(fd ... ?
*/

#if HaveAdvisoryLocks
LOCALFUNC bool LockFile(FILE *refnum)
{
	bool IsOk = false;

#if 1
	struct flock fl;
	int fd = fileno(refnum);

	fl.l_start = 0; /* starting offset */
	fl.l_len = 0; /* len = 0 means until end of file */
	/* fl.pid_t l_pid; */ /* lock owner, don't need to set */
	fl.l_type = F_WRLCK; /* lock type: read/write, etc. */
	fl.l_whence = SEEK_SET; /* type of l_start */
	if (-1 == fcntl(fd, F_SETLK, &fl)) {
		MacMsg(kStrImageInUseTitle, kStrImageInUseMessage,
			false);
	} else {
		IsOk = true;
	}
#else
	int fd = fileno(refnum);

	if (-1 == flock(fd, LOCK_EX | LOCK_NB)) {
		MacMsg(kStrImageInUseTitle, kStrImageInUseMessage,
			false);
	} else {
		IsOk = true;
	}
#endif

	return IsOk;
}
#endif

#if HaveAdvisoryLocks
LOCALPROC UnlockFile(FILE *refnum)
{
#if 1
	struct flock fl;
	int fd = fileno(refnum);

	fl.l_start = 0; /* starting offset */
	fl.l_len = 0; /* len = 0 means until end of file */
	/* fl.pid_t l_pid; */ /* lock owner, don't need to set */
	fl.l_type = F_UNLCK;     /* lock type: read/write, etc. */
	fl.l_whence = SEEK_SET;   /* type of l_start */
	if (-1 == fcntl(fd, F_SETLK, &fl)) {
		/* an error occurred */
	}
#else
	int fd = fileno(refnum);

	if (-1 == flock(fd, LOCK_UN)) {
	}
#endif
}
#endif

LOCALFUNC tMacErr vSonyEject0(tDrive Drive_No, bool deleteit)
{
	FILE *refnum = Drives[Drive_No];

	DiskEjectedNotify(Drive_No);

#if HaveAdvisoryLocks
	UnlockFile(refnum);
#endif

	fclose(refnum);
	Drives[Drive_No] = NotAfileRef; /* not really needed */

#if IncludeSonyGetName || IncludeSonyNew
	{
		char *s = DriveNames[Drive_No];
		if (NULL != s) {
			if (deleteit) {
				remove(s);
			}
			free(s);
			DriveNames[Drive_No] = NULL; /* not really needed */
		}
	}
#endif

	return mnvm_noErr;
}

GLOBALOSGLUFUNC tMacErr vSonyEject(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, false);
}

#if IncludeSonyNew
GLOBALOSGLUFUNC tMacErr vSonyEjectDelete(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, true);
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
	char *drivepath = DriveNames[Drive_No];
	if (NULL == drivepath) {
		return mnvm_miscErr;
	} else {
		char *s = strrchr(drivepath, '/');
		if (NULL == s) {
			s = drivepath;
		} else {
			++s;
		}
		return NativeTextToMacRomanPbuf(s, r);
	}
}
#endif

LOCALFUNC bool Sony_Insert0(FILE *refnum, bool locked,
	char *drivepath)
{
	tDrive Drive_No;
	bool IsOk = false;

	if (! FirstFreeDisk(&Drive_No)) {
		MacMsg(kStrTooManyImagesTitle, kStrTooManyImagesMessage,
			false);
	} else {
		/* printf("Sony_Insert0 %d\n", (int)Drive_No); */

#if HaveAdvisoryLocks
		if (locked || LockFile(refnum))
#endif
		{
			Drives[Drive_No] = refnum;
			DiskInsertNotify(Drive_No, locked);

#if IncludeSonyGetName || IncludeSonyNew
			{
				uint32_t L = strlen(drivepath);
				char *p = malloc(L + 1);
				if (p != NULL) {
					(void) memcpy(p, drivepath, L + 1);
				}
				DriveNames[Drive_No] = p;
			}
#endif

			IsOk = true;
		}
	}

	if (! IsOk) {
		fclose(refnum);
	}

	return IsOk;
}

LOCALFUNC bool Sony_Insert1(char *drivepath, bool silentfail)
{
	bool locked = false;
	/* printf("Sony_Insert1 %s\n", drivepath); */
	FILE *refnum = fopen(drivepath, "rb+");
	if (NULL == refnum) {
		locked = true;
		refnum = fopen(drivepath, "rb");
	}
	if (NULL == refnum) {
		if (! silentfail) {
			MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
		}
	} else {
		return Sony_Insert0(refnum, locked, drivepath);
	}
	return false;
}

LOCALFUNC tMacErr LoadMacRomFrom(char *path)
{
	tMacErr err;
	FILE *ROM_File;
	int File_Size;

	ROM_File = fopen(path, "rb");
	if (NULL == ROM_File) {
		err = mnvm_fnfErr;
	} else {
		File_Size = fread(ROM, 1, kROM_Size, ROM_File);
		if (kROM_Size != File_Size) {
			if (feof(ROM_File)) {
				MacMsgOverride(kStrShortROMTitle,
					kStrShortROMMessage);
				err = mnvm_eofErr;
			} else {
				MacMsgOverride(kStrNoReadROMTitle,
					kStrNoReadROMMessage);
				err = mnvm_miscErr;
			}
		} else {
			err = ROM_IsValid();
		}
		fclose(ROM_File);
	}

	return err;
}

LOCALFUNC bool Sony_Insert1a(char *drivepath, bool silentfail)
{
	bool v;

	if (! ROM_loaded) {
		v = (mnvm_noErr == LoadMacRomFrom(drivepath));
	} else {
		v = Sony_Insert1(drivepath, silentfail);
	}

	return v;
}

LOCALFUNC bool Sony_Insert2(char *s)
{
	char *d =
#ifdef CanGetAppPath
		(NULL == d_arg) ? app_parent :
#endif
		d_arg;
	bool IsOk = false;

	if (NULL == d) {
		IsOk = Sony_Insert1(s, true);
	} else {
		char *t;

		if (mnvm_noErr == ChildPath(d, s, &t)) {
			IsOk = Sony_Insert1(t, true);
			free(t);
		}
	}

	return IsOk;
}

LOCALFUNC bool Sony_InsertIth(int i)
{
	bool v;

	if ((i > 9) || ! FirstFreeDisk(nullpr)) {
		v = false;
	} else {
		char s[] = "disk?.dsk";

		s[4] = '0' + i;

		v = Sony_Insert2(s);
	}

	return v;
}

LOCALFUNC bool LoadInitialImages(void)
{
	if (! AnyDiskInserted()) {
		int i;

		for (i = 1; Sony_InsertIth(i); ++i) {
			/* stop on first error (including file not found) */
		}
	}

	return true;
}

#if IncludeSonyNew
LOCALFUNC bool WriteZero(FILE *refnum, uint32_t L)
{
#define ZeroBufferSize 2048
	uint32_t i;
	uint8_t buffer[ZeroBufferSize];

	memset(&buffer, 0, ZeroBufferSize);

	while (L > 0) {
		i = (L > ZeroBufferSize) ? ZeroBufferSize : L;
		if (fwrite(buffer, 1, i, refnum) != i) {
			return false;
		}
		L -= i;
	}
	return true;
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDisk0(uint32_t L, char *drivepath)
{
	bool IsOk = false;
	FILE *refnum = fopen(drivepath, "wb+");
	if (NULL == refnum) {
		MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
	} else {
		if (WriteZero(refnum, L)) {
			IsOk = Sony_Insert0(refnum, false, drivepath);
			refnum = NULL;
		}
		if (refnum != NULL) {
			fclose(refnum);
		}
		if (! IsOk) {
			(void) remove(drivepath);
		}
	}
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDisk(uint32_t L, char *drivename)
{
	char *d =
#ifdef CanGetAppPath
		(NULL == d_arg) ? app_parent :
#endif
		d_arg;

	if (NULL == d) {
		MakeNewDisk0(L, drivename); /* in current directory */
	} else {
		tMacErr err;
		char *t = NULL;
		char *t2 = NULL;

		if (mnvm_noErr == (err = FindOrMakeChild(d, "out", &t)))
		if (mnvm_noErr == (err = ChildPath(t, drivename, &t2)))
		{
			MakeNewDisk0(L, t2);
		}

		MayFree(t2);
		MayFree(t);
	}
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDiskAtDefault(uint32_t L)
{
	char s[ClStrMaxLength + 1];

	NativeStrFromCStr(s, "untitled.dsk");
	MakeNewDisk(L, s);
}
#endif

/* --- ROM --- */

LOCALVAR char *rom_path = NULL;

#if 0
#include <pwd.h>
#include <unistd.h>
#endif

LOCALFUNC tMacErr FindUserHomeFolder(char **r)
{
	tMacErr err;
	char *s;
#if 0
	struct passwd *user;
#endif

	if (NULL != (s = getenv("HOME"))) {
		*r = s;
		err = mnvm_noErr;
	} else
#if 0
	if ((NULL != (user = getpwuid(getuid())))
		&& (NULL != (s = user->pw_dir)))
	{
		/*
			From getpwuid man page:
			"An application that wants to determine its user's
			home directory should inspect the value of HOME
			(rather than the value getpwuid(getuid())->pw_dir)
			since this allows the user to modify their notion of
			"the home directory" during a login session."

			But it is possible for HOME to not be set.
			Some sources say to use getpwuid in that case.
		*/
		*r = s;
		err = mnvm_noErr;
	} else
#endif
	{
		err = mnvm_fnfErr;
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromHome(void)
{
	tMacErr err;
	char *s;
	char *t = NULL;
	char *t2 = NULL;
	char *t3 = NULL;

	if (mnvm_noErr == (err = FindUserHomeFolder(&s)))
	if (mnvm_noErr == (err = ChildPath(s, ".gryphel", &t)))
	if (mnvm_noErr == (err = ChildPath(t, "mnvm_rom", &t2)))
	if (mnvm_noErr == (err = ChildPath(t2, RomFileName, &t3)))
	{
		err = LoadMacRomFrom(t3);
	}

	MayFree(t3);
	MayFree(t2);
	MayFree(t);

	return err;
}

#ifdef CanGetAppPath
LOCALFUNC tMacErr LoadMacRomFromAppPar(void)
{
	tMacErr err;
	char *d =
#ifdef CanGetAppPath
		(NULL == d_arg) ? app_parent :
#endif
		d_arg;
	char *t = NULL;

	if (NULL == d) {
		err = mnvm_fnfErr;
	} else {
		if (mnvm_noErr == (err = ChildPath(d, RomFileName,
			&t)))
		{
			err = LoadMacRomFrom(t);
		}
	}

	MayFree(t);

	return err;
}
#endif

LOCALFUNC bool LoadMacRom(void)
{
	tMacErr err;

	if ((NULL == rom_path)
		|| (mnvm_fnfErr == (err = LoadMacRomFrom(rom_path))))
#ifdef CanGetAppPath
	if (mnvm_fnfErr == (err = LoadMacRomFromAppPar()))
#endif
	if (mnvm_fnfErr == (err = LoadMacRomFromHome()))
	if (mnvm_fnfErr == (err = LoadMacRomFrom(RomFileName)))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}

/* --- video out --- */

LOCALVAR Window main_wind = 0;
LOCALVAR GC gc = NULL;
LOCALVAR bool NeedFinishOpen1 = false;
LOCALVAR bool NeedFinishOpen2 = false;

LOCALVAR XColor x_black;
LOCALVAR XColor x_white;

#if MayFullScreen
LOCALVAR short hOffset;
LOCALVAR short vOffset;
#endif

#if VarFullScreen
LOCALVAR bool UseFullScreen = (WantInitFullScreen != 0);
#endif

#if EnableMagnify
LOCALVAR bool UseMagnify = (WantInitMagnify != 0);
#endif

LOCALVAR bool gBackgroundFlag = false;
LOCALVAR bool gTrueBackgroundFlag = false;
LOCALVAR bool CurSpeedStopped = true;

#ifndef UseColorImage
#define UseColorImage (0 != vMacScreenDepth)
#endif

LOCALVAR XImage *image = NULL;

#if EnableMagnify
LOCALVAR XImage *Scaled_image = NULL;
#endif

#if EnableMagnify
#define MaxScale WindowScale
#else
#define MaxScale 1
#endif

#define WantScalingTabl (EnableMagnify || UseColorImage)

#if WantScalingTabl

LOCALVAR uint8_t * ScalingTabl = nullpr;

#define ScalingTablsz1 (256 * MaxScale)

#if UseColorImage
#define ScalingTablsz (ScalingTablsz1 << 5)
#else
#define ScalingTablsz ScalingTablsz1
#endif

#endif /* WantScalingTabl */


#define WantScalingBuff (EnableMagnify || UseColorImage)

#if WantScalingBuff

LOCALVAR uint8_t * ScalingBuff = nullpr;


#if UseColorImage
#define ScalingBuffsz \
	(vMacScreenNumPixels * 4 * MaxScale * MaxScale)
#else
#define ScalingBuffsz ((long)vMacScreenMonoNumBytes \
	* MaxScale * MaxScale)
#endif

#endif /* WantScalingBuff */


#if EnableMagnify && ! UseColorImage
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

//#if EnableMagnify && (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
LOCALPROC SetUpColorScalingTabl(void)
{
	int i;
	int j;
	int k;
	int a;
	uint32_t v;
	uint32_t * p4;

	p4 = (uint32_t *)ScalingTabl;
	for (i = 0; i < 256; ++i) {
		for (k = 1 << (3 - vMacScreenDepth); --k >= 0; ) {
			j = (i >> (k << vMacScreenDepth)) & (CLUT_size - 1);
			v = (((long)CLUT_reds[j] & 0xFF00) << 8)
				| ((long)CLUT_greens[j] & 0xFF00)
				| (((long)CLUT_blues[j] & 0xFF00) >> 8);
			for (a = WindowScale; --a >= 0; ) {
				*p4++ = v;
			}
		}
	}
}

//#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)
LOCALPROC SetUpColorTabl(void)
{
	int i;
	int j;
	int k;
	uint32_t * p4;

	p4 = (uint32_t *)ScalingTabl;
	for (i = 0; i < 256; ++i) {
		for (k = 1 << (3 - vMacScreenDepth); --k >= 0; ) {
			j = (i >> (k << vMacScreenDepth)) & (CLUT_size - 1);
			*p4++ = (((long)CLUT_reds[j] & 0xFF00) << 8)
				| ((long)CLUT_greens[j] & 0xFF00)
				| (((long)CLUT_blues[j] & 0xFF00) >> 8);
		}
	}
}

//#if EnableMagnify && UseColorImage
LOCALPROC SetUpBW2ColorScalingTabl(void)
{
	int i;
	int k;
	int a;
	uint32_t v;
	uint32_t * p4;

	p4 = (uint32_t *)ScalingTabl;
	for (i = 0; i < 256; ++i) {
		for (k = 8; --k >= 0; ) {
			if (0 != ((i >> k) & 1)) {
				v = 0;
			} else {
				v = 0xFFFFFF;
			}

			for (a = WindowScale; --a >= 0; ) {
				*p4++ = v;
			}
		}
	}
}

//#if UseColorImage
LOCALPROC SetUpBW2ColorTabl(void)
{
	int i;
	int k;
	uint32_t v;
	uint32_t * p4;

	p4 = (uint32_t *)ScalingTabl;
	for (i = 0; i < 256; ++i) {
		for (k = 8; --k >= 0; ) {
			if (0 != ((i >> k) & 1)) {
				v = 0;
			} else {
				v = 0xFFFFFF;
			}
			*p4++ = v;
		}
	}
}


#if EnableMagnify && ! UseColorImage

#define ScrnMapr_DoMap UpdateScaledBWCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 0
#define ScrnMapr_Map ScalingTabl
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#endif


#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)

#define ScrnMapr_DoMap UpdateMappedColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map ScalingTabl

#include "HW/SCREEN/SCRNMAPR.h"

#endif


#if EnableMagnify && (0 != vMacScreenDepth) && (vMacScreenDepth < 4)

#define ScrnMapr_DoMap UpdateMappedScaledColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map ScalingTabl
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#endif


#if vMacScreenDepth >= 4

#define ScrnTrns_DoTrans UpdateTransColorCopy
#define ScrnTrns_Src GetCurDrawBuff()
#define ScrnTrns_Dst ScalingBuff
#define ScrnTrns_SrcDepth vMacScreenDepth
#define ScrnTrns_DstDepth 5

#include "HW/SCREEN/SCRNTRNS.h"

#endif

#if EnableMagnify && (vMacScreenDepth >= 4)

#define ScrnTrns_DoTrans UpdateTransScaledColorCopy
#define ScrnTrns_Src GetCurDrawBuff()
#define ScrnTrns_Dst ScalingBuff
#define ScrnTrns_SrcDepth vMacScreenDepth
#define ScrnTrns_DstDepth 5
#define ScrnTrns_Scale WindowScale

#include "HW/SCREEN/SCRNTRNS.h"

#endif


#if EnableMagnify && UseColorImage

#define ScrnMapr_DoMap UpdateMappedScaledBW2ColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map ScalingTabl
#define ScrnMapr_Scale WindowScale

#include "HW/SCREEN/SCRNMAPR.h"

#endif


#if UseColorImage

#define ScrnMapr_DoMap UpdateMappedBW2ColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map ScalingTabl

#include "HW/SCREEN/SCRNMAPR.h"

#endif


LOCALPROC HaveChangedScreenBuff(uint16_t top, uint16_t left,
	uint16_t bottom, uint16_t right)
{
	int XDest;
	int YDest;
	char *the_data;

#if VarFullScreen
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

	XDest = left;
	YDest = top;

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		XDest -= ViewHStart;
		YDest -= ViewVStart;
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
		XDest *= WindowScale;
		YDest *= WindowScale;
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		XDest += hOffset;
		YDest += vOffset;
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
#if UseColorImage
#if 0 != vMacScreenDepth
		if (UseColorMode) {
#if vMacScreenDepth < 4
			if (! ColorTransValid) {
				SetUpColorScalingTabl();
				ColorTransValid = true;
			}

			UpdateMappedScaledColorCopy(top, left, bottom, right);
#else
			UpdateTransScaledColorCopy(top, left, bottom, right);
#endif
		} else
#endif /* 0 != vMacScreenDepth */
		{
			if (! ColorTransValid) {
				SetUpBW2ColorScalingTabl();
				ColorTransValid = true;
			}

			UpdateMappedScaledBW2ColorCopy(top, left, bottom, right);
		}
#else /* ! UseColorImage */
		/* assume 0 == vMacScreenDepth */
		{
			if (! ColorTransValid) {
				SetUpScalingTabl();
				ColorTransValid = true;
			}

			UpdateScaledBWCopy(top, left, bottom, right);
		}
#endif /* UseColorImage */

		{
			char *saveData = Scaled_image->data;
			Scaled_image->data = (char *)ScalingBuff;

			XPutImage(x_display, main_wind, gc, Scaled_image,
				left * WindowScale, top * WindowScale,
				XDest, YDest,
				(right - left) * WindowScale,
				(bottom - top) * WindowScale);

			Scaled_image->data = saveData;
		}
	} else
#endif /* EnableMagnify */
	{
#if UseColorImage
#if 0 != vMacScreenDepth
		if (UseColorMode) {
#if vMacScreenDepth < 4

			if (! ColorTransValid) {
				SetUpColorTabl();
				ColorTransValid = true;
			}

			UpdateMappedColorCopy(top, left, bottom, right);

			the_data = (char *)ScalingBuff;
#else
			/*
				if vMacScreenDepth == 5 and MSBFirst, could
				copy directly with the_data = (char *)GetCurDrawBuff();
			*/
			UpdateTransColorCopy(top, left, bottom, right);

			the_data = (char *)ScalingBuff;
#endif
		} else
#endif /* 0 != vMacScreenDepth */
		{
			if (! ColorTransValid) {
				SetUpBW2ColorTabl();
				ColorTransValid = true;
			}

			UpdateMappedBW2ColorCopy(top, left, bottom, right);

			the_data = (char *)ScalingBuff;
		}
#else /* ! UseColorImage */
		{
			the_data = (char *)GetCurDrawBuff();
		}
#endif /* UseColorImage */

		{
			char *saveData = image->data;
			image->data = the_data;

			XPutImage(x_display, main_wind, gc, image,
				left, top, XDest, YDest,
				right - left, bottom - top);

			image->data = saveData;
		}
	}

#if MayFullScreen
label_exit:
	;
#endif
}

LOCALPROC DrawChangesAndClear(void)
{
	if (ScreenChangedBottom > ScreenChangedTop) {
		HaveChangedScreenBuff(ScreenChangedTop, ScreenChangedLeft,
			ScreenChangedBottom, ScreenChangedRight);
		ScreenClearChanges();
	}
}

/* --- mouse --- */

/* cursor hiding */

LOCALVAR bool HaveCursorHidden = false;
LOCALVAR bool WantCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		if (main_wind) {
			XUndefineCursor(x_display, main_wind);
		}
	}
}

LOCALVAR Cursor blankCursor = None;

LOCALFUNC bool CreateBlankCursor(Window rootwin)
/*
	adapted from X11_CreateNullCursor in context.x11.c
	in quakeforge 0.5.5, copyright Id Software, Inc.
	Zephaniah E. Hull, and Jeff Teunissen.
*/
{
	Pixmap cursormask;
	XGCValues xgc;
	GC gc;
	bool IsOk = false;

	cursormask = XCreatePixmap(x_display, rootwin, 1, 1, 1);
	if (None == cursormask) {
		WriteExtraErr("XCreatePixmap failed.");
	} else {
		xgc.function = GXclear;
		gc = XCreateGC(x_display, cursormask, GCFunction, &xgc);
		if (None == gc) {
			WriteExtraErr("XCreateGC failed.");
		} else {
			XFillRectangle(x_display, cursormask, gc, 0, 0, 1, 1);
			XFreeGC(x_display, gc);

			blankCursor = XCreatePixmapCursor(x_display, cursormask,
							cursormask, &x_black, &x_white, 0, 0);
			if (None == blankCursor) {
				WriteExtraErr("XCreatePixmapCursor failed.");
			} else {
				IsOk = true;
			}
		}

		XFreePixmap(x_display, cursormask);
		/*
			assuming that XCreatePixmapCursor doesn't think it
			owns the pixmaps passed to it. I've seen code that
			assumes this, and other code that seems to assume
			the opposite.
		*/
	}
	return IsOk;
}

/* cursor moving */

#if EnableMoveMouse
LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
	int NewMousePosh;
	int NewMousePosv;
	int root_x_return;
	int root_y_return;
	Window root_return;
	Window child_return;
	unsigned int mask_return;
	bool IsOk;
	int attempts = 0;

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

	do {
		XWarpPointer(x_display, None, main_wind, 0, 0, 0, 0, h, v);
		XQueryPointer(x_display, main_wind,
			&root_return, &child_return,
			&root_x_return, &root_y_return,
			&NewMousePosh, &NewMousePosv,
			&mask_return);
		IsOk = (h == NewMousePosh) && (v == NewMousePosv);
		++attempts;
	} while ((! IsOk) && (attempts < 10));
	return IsOk;
}
#endif

#if EnableFSMouseMotion
LOCALPROC StartSaveMouseMotion(void)
{
	if (! HaveMouseMotion) {
		if (MoveMouse(ViewHStart + (ViewHSize / 2),
			ViewVStart + (ViewVSize / 2)))
		{
			SavedMouseH = ViewHStart + (ViewHSize / 2);
			SavedMouseV = ViewVStart + (ViewVSize / 2);
			HaveMouseMotion = true;
		}
	}
}
#endif

#if EnableFSMouseMotion
LOCALPROC StopSaveMouseMotion(void)
{
	if (HaveMouseMotion) {
		(void) MoveMouse(CurMouseH, CurMouseV);
		HaveMouseMotion = false;
	}
}
#endif

/* cursor state */

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

LOCALPROC MousePositionNotify(int NewMousePosh, int NewMousePosv)
{
	bool ShouldHaveCursorHidden = true;

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePosh -= hOffset;
		NewMousePosv -= vOffset;
	}
#endif

#if EnableMagnify
	if (UseMagnify) {
		NewMousePosh /= WindowScale;
		NewMousePosv /= WindowScale;
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewMousePosh += ViewHStart;
		NewMousePosv += ViewVStart;
	}
#endif

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MousePositionSetDelta(NewMousePosh - SavedMouseH,
			NewMousePosv - SavedMouseV);
		SavedMouseH = NewMousePosh;
		SavedMouseV = NewMousePosv;
	} else
#endif
	{
		if (NewMousePosh < 0) {
			NewMousePosh = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosh >= vMacScreenWidth) {
			NewMousePosh = vMacScreenWidth - 1;
			ShouldHaveCursorHidden = false;
		}
		if (NewMousePosv < 0) {
			NewMousePosv = 0;
			ShouldHaveCursorHidden = false;
		} else if (NewMousePosv >= vMacScreenHeight) {
			NewMousePosv = vMacScreenHeight - 1;
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
		MousePositionSet(NewMousePosh, NewMousePosv);
	}

	WantCursorHidden = ShouldHaveCursorHidden;
}

LOCALPROC CheckMouseState(void)
{
	int NewMousePosh;
	int NewMousePosv;
	int root_x_return;
	int root_y_return;
	Window root_return;
	Window child_return;
	unsigned int mask_return;

	XQueryPointer(x_display, main_wind,
		&root_return, &child_return,
		&root_x_return, &root_y_return,
		&NewMousePosh, &NewMousePosv,
		&mask_return);
	MousePositionNotify(NewMousePosh, NewMousePosv);
}

/* --- keyboard input --- */

LOCALVAR KeyCode TheCapsLockCode;

LOCALVAR uint8_t KC2MKC[256];

LOCALPROC KC2MKCAssignOne(KeySym ks, uint8_t key)
{
	KeyCode code = XKeysymToKeycode(x_display, ks);
	if (code != NoSymbol) {
		KC2MKC[code] = key;
	}
#if 0
	fprintf(stderr, "%d %d %d\n", (int)ks, key, (int)code);
#endif
}

LOCALFUNC bool KC2MKCInit(void)
{
	int i;

	for (i = 0; i < 256; ++i) {
		KC2MKC[i] = MKC_None;
	}

#if 0 /* find Keysym for a code */
	for (i = 0; i < 64 * 1024; ++i) {
		KeyCode code = XKeysymToKeycode(x_display, i);
		if (115 == code) {
			fprintf(stderr, "i %d\n", i);
		}
	}
#endif

	/*
	start with redundant mappings, should get overwritten
	by main mappings but define them just in case
	*/

#ifdef XK_KP_Insert
	KC2MKCAssignOne(XK_KP_Insert, MKC_KP0);
#endif
#ifdef XK_KP_End
	KC2MKCAssignOne(XK_KP_End, MKC_KP1);
#endif
#ifdef XK_KP_Down
	KC2MKCAssignOne(XK_KP_Down, MKC_KP2);
#endif
#ifdef XK_KP_Next
	KC2MKCAssignOne(XK_KP_Next, MKC_KP3);
#endif
#ifdef XK_KP_Left
	KC2MKCAssignOne(XK_KP_Left, MKC_KP4);
#endif
#ifdef XK_KP_Begin
	KC2MKCAssignOne(XK_KP_Begin, MKC_KP5);
#endif
#ifdef XK_KP_Right
	KC2MKCAssignOne(XK_KP_Right, MKC_KP6);
#endif
#ifdef XK_KP_Home
	KC2MKCAssignOne(XK_KP_Home, MKC_KP7);
#endif
#ifdef XK_KP_Up
	KC2MKCAssignOne(XK_KP_Up, MKC_KP8);
#endif
#ifdef XK_KP_Prior
	KC2MKCAssignOne(XK_KP_Prior, MKC_KP9);
#endif
#ifdef XK_KP_Delete
	KC2MKCAssignOne(XK_KP_Delete, MKC_Decimal);
#endif

	KC2MKCAssignOne(XK_asciitilde, MKC_formac_Grave);
	KC2MKCAssignOne(XK_underscore, MKC_Minus);
	KC2MKCAssignOne(XK_plus, MKC_Equal);
	KC2MKCAssignOne(XK_braceleft, MKC_LeftBracket);
	KC2MKCAssignOne(XK_braceright, MKC_RightBracket);
	KC2MKCAssignOne(XK_bar, MKC_formac_BackSlash);
	KC2MKCAssignOne(XK_colon, MKC_SemiColon);
	KC2MKCAssignOne(XK_quotedbl, MKC_SingleQuote);
	KC2MKCAssignOne(XK_less, MKC_Comma);
	KC2MKCAssignOne(XK_greater, MKC_Period);
	KC2MKCAssignOne(XK_question, MKC_formac_Slash);

	KC2MKCAssignOne(XK_a, MKC_A);
	KC2MKCAssignOne(XK_b, MKC_B);
	KC2MKCAssignOne(XK_c, MKC_C);
	KC2MKCAssignOne(XK_d, MKC_D);
	KC2MKCAssignOne(XK_e, MKC_E);
	KC2MKCAssignOne(XK_f, MKC_F);
	KC2MKCAssignOne(XK_g, MKC_G);
	KC2MKCAssignOne(XK_h, MKC_H);
	KC2MKCAssignOne(XK_i, MKC_I);
	KC2MKCAssignOne(XK_j, MKC_J);
	KC2MKCAssignOne(XK_k, MKC_K);
	KC2MKCAssignOne(XK_l, MKC_L);
	KC2MKCAssignOne(XK_m, MKC_M);
	KC2MKCAssignOne(XK_n, MKC_N);
	KC2MKCAssignOne(XK_o, MKC_O);
	KC2MKCAssignOne(XK_p, MKC_P);
	KC2MKCAssignOne(XK_q, MKC_Q);
	KC2MKCAssignOne(XK_r, MKC_R);
	KC2MKCAssignOne(XK_s, MKC_S);
	KC2MKCAssignOne(XK_t, MKC_T);
	KC2MKCAssignOne(XK_u, MKC_U);
	KC2MKCAssignOne(XK_v, MKC_V);
	KC2MKCAssignOne(XK_w, MKC_W);
	KC2MKCAssignOne(XK_x, MKC_X);
	KC2MKCAssignOne(XK_y, MKC_Y);
	KC2MKCAssignOne(XK_z, MKC_Z);

	/*
	main mappings
	*/

	KC2MKCAssignOne(XK_F1, MKC_formac_F1);
	KC2MKCAssignOne(XK_F2, MKC_formac_F2);
	KC2MKCAssignOne(XK_F3, MKC_formac_F3);
	KC2MKCAssignOne(XK_F4, MKC_formac_F4);
	KC2MKCAssignOne(XK_F5, MKC_formac_F5);
	KC2MKCAssignOne(XK_F6, MKC_F6);
	KC2MKCAssignOne(XK_F7, MKC_F7);
	KC2MKCAssignOne(XK_F8, MKC_F8);
	KC2MKCAssignOne(XK_F9, MKC_F9);
	KC2MKCAssignOne(XK_F10, MKC_F10);
	KC2MKCAssignOne(XK_F11, MKC_F11);
	KC2MKCAssignOne(XK_F12, MKC_F12);

#ifdef XK_Delete
	KC2MKCAssignOne(XK_Delete, MKC_formac_ForwardDel);
#endif
#ifdef XK_Insert
	KC2MKCAssignOne(XK_Insert, MKC_formac_Help);
#endif
#ifdef XK_Help
	KC2MKCAssignOne(XK_Help, MKC_formac_Help);
#endif
#ifdef XK_Home
	KC2MKCAssignOne(XK_Home, MKC_formac_Home);
#endif
#ifdef XK_End
	KC2MKCAssignOne(XK_End, MKC_formac_End);
#endif

#ifdef XK_Page_Up
	KC2MKCAssignOne(XK_Page_Up, MKC_formac_PageUp);
#else
#ifdef XK_Prior
	KC2MKCAssignOne(XK_Prior, MKC_formac_PageUp);
#endif
#endif

#ifdef XK_Page_Down
	KC2MKCAssignOne(XK_Page_Down, MKC_formac_PageDown);
#else
#ifdef XK_Next
	KC2MKCAssignOne(XK_Next, MKC_formac_PageDown);
#endif
#endif

#ifdef XK_Print
	KC2MKCAssignOne(XK_Print, MKC_Print);
#endif
#ifdef XK_Scroll_Lock
	KC2MKCAssignOne(XK_Scroll_Lock, MKC_ScrollLock);
#endif
#ifdef XK_Pause
	KC2MKCAssignOne(XK_Pause, MKC_Pause);
#endif

	KC2MKCAssignOne(XK_KP_Add, MKC_KPAdd);
	KC2MKCAssignOne(XK_KP_Subtract, MKC_KPSubtract);
	KC2MKCAssignOne(XK_KP_Multiply, MKC_KPMultiply);
	KC2MKCAssignOne(XK_KP_Divide, MKC_KPDevide);
	KC2MKCAssignOne(XK_KP_Enter, MKC_formac_Enter);
	KC2MKCAssignOne(XK_KP_Equal, MKC_KPEqual);

	KC2MKCAssignOne(XK_KP_0, MKC_KP0);
	KC2MKCAssignOne(XK_KP_1, MKC_KP1);
	KC2MKCAssignOne(XK_KP_2, MKC_KP2);
	KC2MKCAssignOne(XK_KP_3, MKC_KP3);
	KC2MKCAssignOne(XK_KP_4, MKC_KP4);
	KC2MKCAssignOne(XK_KP_5, MKC_KP5);
	KC2MKCAssignOne(XK_KP_6, MKC_KP6);
	KC2MKCAssignOne(XK_KP_7, MKC_KP7);
	KC2MKCAssignOne(XK_KP_8, MKC_KP8);
	KC2MKCAssignOne(XK_KP_9, MKC_KP9);
	KC2MKCAssignOne(XK_KP_Decimal, MKC_Decimal);

	KC2MKCAssignOne(XK_Left, MKC_Left);
	KC2MKCAssignOne(XK_Right, MKC_Right);
	KC2MKCAssignOne(XK_Up, MKC_Up);
	KC2MKCAssignOne(XK_Down, MKC_Down);

	KC2MKCAssignOne(XK_grave, MKC_formac_Grave);
	KC2MKCAssignOne(XK_minus, MKC_Minus);
	KC2MKCAssignOne(XK_equal, MKC_Equal);
	KC2MKCAssignOne(XK_bracketleft, MKC_LeftBracket);
	KC2MKCAssignOne(XK_bracketright, MKC_RightBracket);
	KC2MKCAssignOne(XK_backslash, MKC_formac_BackSlash);
	KC2MKCAssignOne(XK_semicolon, MKC_SemiColon);
	KC2MKCAssignOne(XK_apostrophe, MKC_SingleQuote);
	KC2MKCAssignOne(XK_comma, MKC_Comma);
	KC2MKCAssignOne(XK_period, MKC_Period);
	KC2MKCAssignOne(XK_slash, MKC_formac_Slash);

	KC2MKCAssignOne(XK_Escape, MKC_formac_Escape);

	KC2MKCAssignOne(XK_Tab, MKC_Tab);
	KC2MKCAssignOne(XK_Return, MKC_Return);
	KC2MKCAssignOne(XK_space, MKC_Space);
	KC2MKCAssignOne(XK_BackSpace, MKC_BackSpace);

	KC2MKCAssignOne(XK_Caps_Lock, MKC_formac_CapsLock);
	KC2MKCAssignOne(XK_Num_Lock, MKC_Clear);

	KC2MKCAssignOne(XK_Meta_L, MKC_formac_Command);

	KC2MKCAssignOne(XK_Meta_R, MKC_formac_RCommand);

	KC2MKCAssignOne(XK_Mode_switch, MKC_formac_Option);
	KC2MKCAssignOne(XK_Menu, MKC_formac_Option);
	KC2MKCAssignOne(XK_Super_L, MKC_formac_Option);
	KC2MKCAssignOne(XK_Super_R, MKC_formac_ROption);
	KC2MKCAssignOne(XK_Hyper_L, MKC_formac_Option);
	KC2MKCAssignOne(XK_Hyper_R, MKC_formac_ROption);

	KC2MKCAssignOne(XK_F13, MKC_formac_Option);
		/*
			seen being used in Mandrake Linux 9.2
			for windows key
		*/

	KC2MKCAssignOne(XK_Shift_L, MKC_formac_Shift);
	KC2MKCAssignOne(XK_Shift_R, MKC_formac_RShift);

	KC2MKCAssignOne(XK_Alt_L, MKC_formac_Command);

	KC2MKCAssignOne(XK_Alt_R, MKC_formac_RCommand);

	KC2MKCAssignOne(XK_Control_L, MKC_formac_Control);

	KC2MKCAssignOne(XK_Control_R, MKC_formac_RControl);

	KC2MKCAssignOne(XK_1, MKC_1);
	KC2MKCAssignOne(XK_2, MKC_2);
	KC2MKCAssignOne(XK_3, MKC_3);
	KC2MKCAssignOne(XK_4, MKC_4);
	KC2MKCAssignOne(XK_5, MKC_5);
	KC2MKCAssignOne(XK_6, MKC_6);
	KC2MKCAssignOne(XK_7, MKC_7);
	KC2MKCAssignOne(XK_8, MKC_8);
	KC2MKCAssignOne(XK_9, MKC_9);
	KC2MKCAssignOne(XK_0, MKC_0);

	KC2MKCAssignOne(XK_A, MKC_A);
	KC2MKCAssignOne(XK_B, MKC_B);
	KC2MKCAssignOne(XK_C, MKC_C);
	KC2MKCAssignOne(XK_D, MKC_D);
	KC2MKCAssignOne(XK_E, MKC_E);
	KC2MKCAssignOne(XK_F, MKC_F);
	KC2MKCAssignOne(XK_G, MKC_G);
	KC2MKCAssignOne(XK_H, MKC_H);
	KC2MKCAssignOne(XK_I, MKC_I);
	KC2MKCAssignOne(XK_J, MKC_J);
	KC2MKCAssignOne(XK_K, MKC_K);
	KC2MKCAssignOne(XK_L, MKC_L);
	KC2MKCAssignOne(XK_M, MKC_M);
	KC2MKCAssignOne(XK_N, MKC_N);
	KC2MKCAssignOne(XK_O, MKC_O);
	KC2MKCAssignOne(XK_P, MKC_P);
	KC2MKCAssignOne(XK_Q, MKC_Q);
	KC2MKCAssignOne(XK_R, MKC_R);
	KC2MKCAssignOne(XK_S, MKC_S);
	KC2MKCAssignOne(XK_T, MKC_T);
	KC2MKCAssignOne(XK_U, MKC_U);
	KC2MKCAssignOne(XK_V, MKC_V);
	KC2MKCAssignOne(XK_W, MKC_W);
	KC2MKCAssignOne(XK_X, MKC_X);
	KC2MKCAssignOne(XK_Y, MKC_Y);
	KC2MKCAssignOne(XK_Z, MKC_Z);

	TheCapsLockCode = XKeysymToKeycode(x_display, XK_Caps_Lock);

	InitKeyCodes();

	return true;
}

LOCALPROC CheckTheCapsLock(void)
{
	int NewMousePosh;
	int NewMousePosv;
	int root_x_return;
	int root_y_return;
	Window root_return;
	Window child_return;
	unsigned int mask_return;

	XQueryPointer(x_display, main_wind,
		&root_return, &child_return,
		&root_x_return, &root_y_return,
		&NewMousePosh, &NewMousePosv,
		&mask_return);

	Keyboard_UpdateKeyMap2(MKC_formac_CapsLock,
		(mask_return & LockMask) != 0);
}

LOCALPROC DoKeyCode0(int i, bool down)
{
	uint8_t key = KC2MKC[i];
	if (MKC_None != key) {
		Keyboard_UpdateKeyMap2(key, down);
	}
}

LOCALPROC DoKeyCode(int i, bool down)
{
	if (i == TheCapsLockCode) {
		CheckTheCapsLock();
	} else if (i >= 0 && i < 256) {
		DoKeyCode0(i, down);
	}
}

#if MayFullScreen && GrabKeysFullScreen
LOCALVAR bool KeyboardIsGrabbed = false;
#endif

#if MayFullScreen && GrabKeysFullScreen
LOCALPROC GrabKeyboard(void)
{
	if (! KeyboardIsGrabbed) {
		(void) XGrabKeyboard(x_display, main_wind,
			False, GrabModeAsync, GrabModeAsync,
			CurrentTime);
		KeyboardIsGrabbed = true;
	}
}
#endif

#if MayFullScreen && GrabKeysFullScreen
LOCALPROC UnGrabKeyboard(void)
{
	if (KeyboardIsGrabbed && main_wind) {
		XUngrabKeyboard(x_display, CurrentTime);
		KeyboardIsGrabbed = false;
	}
}
#endif

LOCALVAR bool NoKeyRepeat = false;
LOCALVAR int SaveKeyRepeat;

LOCALPROC DisableKeyRepeat(void)
{
	XKeyboardState r;
	XKeyboardControl k;

	if ((! NoKeyRepeat) && (x_display != NULL)) {
		NoKeyRepeat = true;

		XGetKeyboardControl(x_display, &r);
		SaveKeyRepeat = r.global_auto_repeat;

		k.auto_repeat_mode = AutoRepeatModeOff;
		XChangeKeyboardControl(x_display, KBAutoRepeatMode, &k);
	}
}

LOCALPROC RestoreKeyRepeat(void)
{
	XKeyboardControl k;

	if (NoKeyRepeat && (x_display != NULL)) {
		NoKeyRepeat = false;

		k.auto_repeat_mode = SaveKeyRepeat;
		XChangeKeyboardControl(x_display, KBAutoRepeatMode, &k);
	}
}

LOCALVAR bool WantCmdOptOnReconnect = false;

LOCALPROC GetTheDownKeys(void)
{
	char keys_return[32];
	int i;
	int v;
	int j;

	XQueryKeymap(x_display, keys_return);

	for (i = 0; i < 32; ++i) {
		v = keys_return[i];
		for (j = 0; j < 8; ++j) {
			if (0 != ((1 << j) & v)) {
				int k = i * 8 + j;
				if (k != TheCapsLockCode) {
					DoKeyCode0(k, true);
				}
			}
		}
	}
}

LOCALPROC ReconnectKeyCodes3(void)
{
	CheckTheCapsLock();

	if (WantCmdOptOnReconnect) {
		WantCmdOptOnReconnect = false;

		GetTheDownKeys();
	}
}

LOCALPROC DisconnectKeyCodes3(void)
{
	DisconnectKeyCodes2();
	MouseButtonSet(false);
}

/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

LOCALVAR uint32_t TrueEmulatedTime = 0;

#include "UTIL/DATE2SEC.h"

#define TicksPerSecond 1000000

LOCALVAR bool HaveTimeDelta = false;
LOCALVAR uint32_t TimeDelta;

LOCALVAR uint32_t NewMacDateInSeconds;

LOCALVAR uint32_t LastTimeSec;
LOCALVAR uint32_t LastTimeUsec;

LOCALPROC GetCurrentTicks(void)
{
	struct timeval t;

	gettimeofday(&t, NULL);
	if (! HaveTimeDelta) {
		time_t Current_Time;
		struct tm *s;

		(void) time(&Current_Time);
		s = localtime(&Current_Time);
		TimeDelta = Date2MacSeconds(s->tm_sec, s->tm_min, s->tm_hour,
			s->tm_mday, 1 + s->tm_mon, 1900 + s->tm_year) - t.tv_sec;
#if 0 && AutoTimeZone /* how portable is this ? */
		CurMacDelta = ((uint32_t)(s->tm_gmtoff) & 0x00FFFFFF)
			| ((s->tm_isdst ? 0x80 : 0) << 24);
#endif
		HaveTimeDelta = true;
	}

	NewMacDateInSeconds = t.tv_sec + TimeDelta;
	LastTimeSec = (uint32_t)t.tv_sec;
	LastTimeUsec = (uint32_t)t.tv_usec;
}

#define InvTimeStep 16626 /* TicksPerSecond / 60.14742 */

LOCALVAR uint32_t NextTimeSec;
LOCALVAR uint32_t NextTimeUsec;

LOCALPROC IncrNextTime(void)
{
	NextTimeUsec += InvTimeStep;
	if (NextTimeUsec >= TicksPerSecond) {
		NextTimeUsec -= TicksPerSecond;
		NextTimeSec += 1;
	}
}

LOCALPROC InitNextTime(void)
{
	NextTimeSec = LastTimeSec;
	NextTimeUsec = LastTimeUsec;
	IncrNextTime();
}

LOCALPROC StartUpTimeAdjust(void)
{
	GetCurrentTicks();
	InitNextTime();
}

LOCALFUNC int32_t GetTimeDiff(void)
{
	return ((int32_t)(LastTimeSec - NextTimeSec)) * TicksPerSecond
		+ ((int32_t)(LastTimeUsec - NextTimeUsec));
}

LOCALPROC UpdateTrueEmulatedTime(void)
{
	int32_t TimeDiff;

	GetCurrentTicks();

	TimeDiff = GetTimeDiff();
	if (TimeDiff >= 0) {
		if (TimeDiff > 16 * InvTimeStep) {
			/* emulation interrupted, forget it */
			++TrueEmulatedTime;
			InitNextTime();

#if dbglog_TimeStuff
			dbglog_writelnNum("emulation interrupted",
				TrueEmulatedTime);
#endif
		} else {
			do {
				++TrueEmulatedTime;
				IncrNextTime();
				TimeDiff -= TicksPerSecond;
			} while (TimeDiff >= 0);
		}
	} else if (TimeDiff < - 16 * InvTimeStep) {
		/* clock goofed if ever get here, reset */
#if dbglog_TimeStuff
		dbglog_writeln("clock set back");
#endif

		InitNextTime();
	}
}

LOCALFUNC bool CheckDateTime(void)
{
	if (CurMacDateInSeconds != NewMacDateInSeconds) {
		CurMacDateInSeconds = NewMacDateInSeconds;
		return true;
	} else {
		return false;
	}
}

LOCALFUNC bool InitLocationDat(void)
{
	GetCurrentTicks();
	CurMacDateInSeconds = NewMacDateInSeconds;

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
LOCALVAR uint16_t ThePlayOffset;
LOCALVAR uint16_t TheFillOffset;
LOCALVAR uint16_t TheWriteOffset;
LOCALVAR uint16_t MinFilledSoundBuffs;

LOCALPROC Sound_Start0(void)
{
	/* Reset variables */
	ThePlayOffset = 0;
	TheFillOffset = 0;
	TheWriteOffset = 0;
	MinFilledSoundBuffs = kSoundBuffers;
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

LOCALFUNC bool Sound_EndWrite0(uint16_t actL)
{
	bool v;

	TheWriteOffset += actL;

	if (0 != (TheWriteOffset & kOneBuffMask)) {
		v = false;
	} else {
		/* just finished a block */

		TheFillOffset = TheWriteOffset;

		v = true;
	}

	return v;
}

LOCALPROC Sound_SecondNotify0(void)
{
	if (MinFilledSoundBuffs > DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too high");
#endif
		IncrNextTime();
	} else if (MinFilledSoundBuffs < DesiredMinFilledSoundBuffs) {
#if dbglog_SoundStuff
			dbglog_writeln("MinFilledSoundBuffs too low");
#endif
		++TrueEmulatedTime;
	}
	MinFilledSoundBuffs = kSoundBuffers;
}

#define SOUND_SAMPLERATE 22255 /* = round(7833600 * 2 / 704) */

//#include "SOUNDGLU.h"
#include "HW/SOUND/SGLUALSA.h"

#endif

/* --- basic dialogs --- */

LOCALPROC CheckSavedMacMsg(void)
{
	/* called only on quit, if error saved but not yet reported */

	if (nullpr != SavedBriefMsg) {
		char briefMsg0[ClStrMaxLength + 1];
		char longMsg0[ClStrMaxLength + 1];

		NativeStrFromCStr(briefMsg0, SavedBriefMsg);
		NativeStrFromCStr(longMsg0, SavedLongMsg);

		fprintf(stderr, "%s\n", briefMsg0);
		fprintf(stderr, "%s\n", longMsg0);

		SavedBriefMsg = nullpr;
	}
}

/* --- clipboard --- */

#if IncludeHostTextClipExchange
LOCALVAR uint8_t * ClipBuffer = NULL;
#endif

#if IncludeHostTextClipExchange
LOCALPROC FreeClipBuffer(void)
{
	if (ClipBuffer != NULL) {
		free(ClipBuffer);
		ClipBuffer = NULL;
	}
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEexport(tPbuf i)
{
	tMacErr err = mnvm_miscErr;

	FreeClipBuffer();
	if (MacRomanTextToNativePtr(i, false,
		&ClipBuffer))
	{
		XSetSelectionOwner(x_display, XA_CLIPBOARD,
			main_wind, CurrentTime);
		err = mnvm_noErr;
	}

	PbufDispose(i);

	return err;
}
#endif

#if IncludeHostTextClipExchange
LOCALFUNC bool WaitForClipboardSelection(XEvent *xevent)
{
	struct timespec rqt;
	struct timespec rmt;
	int i;

	for (i = 100; --i >= 0; ) {
		while (XCheckTypedWindowEvent(x_display, main_wind,
			SelectionNotify, xevent))
		{
			if (xevent->xselection.selection != XA_CLIPBOARD) {
				/*
					not what we were looking for. lose it.
					(and hope it wasn't too important).
				*/
				WriteExtraErr("Discarding unwanted SelectionNotify");
			} else {
				/* this is our event */
				return true;
			}
		}

		rqt.tv_sec = 0;
		rqt.tv_nsec = 10000000;
		(void) nanosleep(&rqt, &rmt);
	}
	return false;
}
#endif

#if IncludeHostTextClipExchange
LOCALPROC HTCEimport_do(void)
{
	Window w = XGetSelectionOwner(x_display, XA_CLIPBOARD);

	if (w == main_wind) {
		/* We own the clipboard, already have ClipBuffer */
	} else {
		FreeClipBuffer();
		if (w != None) {
			XEvent xevent;

			XDeleteProperty(x_display, main_wind,
				XA_MinivMac_Clip);
			XConvertSelection(x_display, XA_CLIPBOARD, XA_STRING,
				XA_MinivMac_Clip, main_wind, CurrentTime);

			if (WaitForClipboardSelection(&xevent)) {
				if (None == xevent.xselection.property) {
					/* oops, target not supported */
				} else {
					if (xevent.xselection.property
						!= XA_MinivMac_Clip)
					{
						/* not where we expected it */
					} else {
						Atom ret_type;
						int ret_format;
						unsigned long ret_item;
						unsigned long remain_byte;
						unsigned char *s = NULL;

						if ((Success != XGetWindowProperty(
							x_display, main_wind, XA_MinivMac_Clip,
							0, 65535, False, AnyPropertyType, &ret_type,
							&ret_format, &ret_item, &remain_byte, &s))
							|| (ret_type != XA_STRING)
							|| (ret_format != 8)
							|| (NULL == s))
						{
							WriteExtraErr(
								"XGetWindowProperty failed"
								" in HTCEimport_do");
						} else {
							ClipBuffer = (uint8_t *)malloc(ret_item + 1);
							if (NULL == ClipBuffer) {
								MacMsg(kStrOutOfMemTitle,
									kStrOutOfMemMessage, false);
							} else {
								MoveBytes((anyp)s, (anyp)ClipBuffer,
									ret_item);
								ClipBuffer[ret_item] = 0;
							}
							XFree(s);
						}
					}
					XDeleteProperty(x_display, main_wind,
						XA_MinivMac_Clip);
				}
			}
		}
	}
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEimport(tPbuf *r)
{
	HTCEimport_do();

	return NativeTextToMacRomanPbuf((char *)ClipBuffer, r);
}
#endif

#if IncludeHostTextClipExchange
LOCALFUNC bool HandleSelectionRequestClipboard(XEvent *theEvent)
{
	bool RequestFilled = false;

#if DbgEvents
	dbglog_writeln("Requested XA_CLIPBOARD");
#endif

	if (NULL == ClipBuffer) {
		/* our clipboard is empty */
	} else if (theEvent->xselectionrequest.target == XA_TARGETS) {
		Atom a[2];

		a[0] = XA_TARGETS;
		a[1] = XA_STRING;

		XChangeProperty(x_display,
			theEvent->xselectionrequest.requestor,
			theEvent->xselectionrequest.property,
			XA_TARGETS,
			32,
				/*
					most, but not all, other programs I've
					look at seem to use 8 here, but that
					can't be right. can it?
				*/
			PropModeReplace,
			(unsigned char *)a,
			sizeof(a) / sizeof(Atom));

		RequestFilled = true;
	} else if (theEvent->xselectionrequest.target == XA_STRING) {
		XChangeProperty(x_display,
			theEvent->xselectionrequest.requestor,
			theEvent->xselectionrequest.property,
			XA_STRING,
			8,
			PropModeReplace,
			(unsigned char *)ClipBuffer,
			strlen((char *)ClipBuffer));

		RequestFilled = true;
	}

	return RequestFilled;
}
#endif

/* --- drag and drop --- */

#if EnableDragDrop
LOCALPROC ActivateWind(Time time)
{
	if (NetSupportedContains(XA_NetActiveWindow)) {
		XEvent xevent;
		Window rootwin = XRootWindow(x_display,
			DefaultScreen(x_display));

		memset(&xevent, 0, sizeof (xevent));

		xevent.xany.type = ClientMessage;
		xevent.xclient.send_event = True;
		xevent.xclient.window = main_wind;
		xevent.xclient.message_type = XA_NetActiveWindow;
		xevent.xclient.format = 32;
		xevent.xclient.data.l[0] = 1;
		xevent.xclient.data.l[1]= time;

		if (0 == XSendEvent(x_display, rootwin, 0,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&xevent))
		{
			WriteExtraErr("XSendEvent failed in ActivateWind");
		}
	}

	XRaiseWindow(x_display, main_wind);
		/*
			In RedHat 7.1, _NET_ACTIVE_WINDOW supported,
			but XSendEvent of _NET_ACTIVE_WINDOW
			doesn't raise the window. So just always
			call XRaiseWindow. Hopefully calling
			XRaiseWindow won't do any harm on window
			managers where it isn't needed.
			(Such as in Ubuntu 5.10)
		*/
	XSetInputFocus(x_display, main_wind,
		RevertToPointerRoot, time);
		/* And call this always too, just in case */
}
#endif

#if EnableDragDrop
LOCALPROC ParseOneUri(char *s)
{
	/* printf("ParseOneUri %s\n", s); */
	if (('f' == s[0]) && ('i' == s[1]) && ('l' == s[2])
		&& ('e' == s[3]) && (':' == s[4]))
	{
		s += 5;
		if (('/' == s[0]) && ('/' == s[1])) {
			/* skip hostname */
			char c;

			s += 2;
			while ((c = *s) != '/') {
				if (0 == c) {
					return;
				}
				++s;
			}
		}
		(void) Sony_Insert1a(s, false);
	}
}
#endif

#if EnableDragDrop
LOCALFUNC int HexChar2Nib(char x)
{
	if ((x >= '0') && (x <= '9')) {
		return x - '0';
	} else if ((x >= 'A') && (x <= 'F')) {
		return x - 'A' + 10;
	} else if ((x >= 'a') && (x <= 'f')) {
		return x - 'a' + 10;
	} else {
		return -1;
	}
}
#endif

#if EnableDragDrop
LOCALPROC ParseUriList(char *s)
{
	char *p1 = s;
	char *p0 = s;
	char *p = s;
	char c;

	/* printf("ParseUriList %s\n", s); */
	while ((c = *p++) != 0) {
		if ('%' == c) {
			int a;
			int b;

			if (((a = HexChar2Nib(p[0])) >= 0) &&
				((b = HexChar2Nib(p[1])) >= 0))
			{
				p += 2;
				*p1++ = (a << 4) + b;
			} else {
				*p1++ = c;
			}
		} else if (('\n' == c) || ('\r' == c)) {
			*p1++ = 0;
			ParseOneUri(p0);
			p0 = p1;
		} else {
			*p1++ = c;
		}
	}
	*p1++ = 0;
	ParseOneUri(p0);
}
#endif

#if EnableDragDrop
LOCALVAR Window PendingDragWindow = None;
#endif

#if EnableDragDrop
LOCALPROC HandleSelectionNotifyDnd(XEvent *theEvent)
{
	bool DropOk = false;

#if DbgEvents
	dbglog_writeln("Got XA_DndSelection");
#endif

	if ((theEvent->xselection.property == XA_MinivMac_DndXchng)
		&& (theEvent->xselection.target == XA_UriList))
	{
		Atom ret_type;
		int ret_format;
		unsigned long ret_item;
		unsigned long remain_byte;
		unsigned char *s = NULL;

		if ((Success != XGetWindowProperty(x_display, main_wind,
			XA_MinivMac_DndXchng,
			0, 65535, False, XA_UriList, &ret_type, &ret_format,
			&ret_item, &remain_byte, &s))
			|| (NULL == s))
		{
			WriteExtraErr(
				"XGetWindowProperty failed in SelectionNotify");
		} else {
			ParseUriList((char *)s);
			DropOk = true;
			XFree(s);
		}
	} else {
		WriteExtraErr("Got Unknown SelectionNotify");
	}

	XDeleteProperty(x_display, main_wind,
		XA_MinivMac_DndXchng);

	if (PendingDragWindow != None) {
		XEvent xevent;

		memset(&xevent, 0, sizeof(xevent));

		xevent.xany.type = ClientMessage;
		xevent.xany.display = x_display;
		xevent.xclient.window = PendingDragWindow;
		xevent.xclient.message_type = XA_DndFinished;
		xevent.xclient.format = 32;

		xevent.xclient.data.l[0] = main_wind;
		if (DropOk) {
			xevent.xclient.data.l[1] = 1;
		}
		xevent.xclient.data.l[2] = XA_DndActionPrivate;

		if (0 == XSendEvent(x_display,
			PendingDragWindow, 0, 0, &xevent))
		{
			WriteExtraErr("XSendEvent failed in SelectionNotify");
		}
	}
	if (DropOk && gTrueBackgroundFlag) {
		ActivateWind(theEvent->xselection.time);

		WantCmdOptOnReconnect = true;
	}
}
#endif

#if EnableDragDrop
LOCALPROC HandleClientMessageDndPosition(XEvent *theEvent)
{
	XEvent xevent;
	int xr;
	int yr;
	unsigned int dr;
	unsigned int wr;
	unsigned int hr;
	unsigned int bwr;
	Window rr;
	Window srcwin = theEvent->xclient.data.l[0];

#if DbgEvents
	dbglog_writeln("Got XdndPosition");
#endif

	XGetGeometry(x_display, main_wind,
		&rr, &xr, &yr, &wr, &hr, &bwr, &dr);
	memset (&xevent, 0, sizeof(xevent));
	xevent.xany.type = ClientMessage;
	xevent.xany.display = x_display;
	xevent.xclient.window = srcwin;
	xevent.xclient.message_type = XA_DndStatus;
	xevent.xclient.format = 32;

	xevent.xclient.data.l[0] = theEvent->xclient.window;
		/* Target Window */
	xevent.xclient.data.l[1] = 1; /* Accept */
	xevent.xclient.data.l[2] = ((xr) << 16) | ((yr) & 0xFFFFUL);
	xevent.xclient.data.l[3] = ((wr) << 16) | ((hr) & 0xFFFFUL);
	xevent.xclient.data.l[4] = XA_DndActionPrivate; /* Action */

	if (0 == XSendEvent(x_display, srcwin, 0, 0, &xevent)) {
		WriteExtraErr(
			"XSendEvent failed in HandleClientMessageDndPosition");
	}
}
#endif

#if EnableDragDrop
LOCALPROC HandleClientMessageDndDrop(XEvent *theEvent)
{
	Time timestamp = theEvent->xclient.data.l[2];
	PendingDragWindow = (Window) theEvent->xclient.data.l[0];

#if DbgEvents
	dbglog_writeln("Got XdndDrop");
#endif

	XConvertSelection(x_display, XA_DndSelection, XA_UriList,
		XA_MinivMac_DndXchng, main_wind, timestamp);
}
#endif

#define UseMotionEvents 1

#if UseMotionEvents
LOCALVAR bool CaughtMouse = false;
#endif

#if MayNotFullScreen
LOCALVAR int SavedTransH;
LOCALVAR int SavedTransV;
#endif

/* --- event handling for main window --- */

LOCALPROC HandleTheEvent(XEvent *theEvent)
{
	if (theEvent->xany.display != x_display) {
		WriteExtraErr("Got event for some other display");
	} else switch(theEvent->type) {
		case KeyPress:
			if (theEvent->xkey.window != main_wind) {
				WriteExtraErr("Got KeyPress for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - KeyPress");
#endif

				MousePositionNotify(theEvent->xkey.x, theEvent->xkey.y);
				DoKeyCode(theEvent->xkey.keycode, true);
			}
			break;
		case KeyRelease:
			if (theEvent->xkey.window != main_wind) {
				WriteExtraErr("Got KeyRelease for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - KeyRelease");
#endif

				MousePositionNotify(theEvent->xkey.x, theEvent->xkey.y);
				DoKeyCode(theEvent->xkey.keycode, false);
			}
			break;
		case ButtonPress:
			/* any mouse button, we don't care which */
			if (theEvent->xbutton.window != main_wind) {
				WriteExtraErr("Got ButtonPress for some other window");
			} else {
				/*
					could check some modifiers, but don't bother for now
					Keyboard_UpdateKeyMap2(MKC_formac_CapsLock,
						(theEvent->xbutton.state & LockMask) != 0);
				*/
				MousePositionNotify(
					theEvent->xbutton.x, theEvent->xbutton.y);
				MouseButtonSet(true);
			}
			break;
		case ButtonRelease:
			/* any mouse button, we don't care which */
			if (theEvent->xbutton.window != main_wind) {
				WriteExtraErr(
					"Got ButtonRelease for some other window");
			} else {
				MousePositionNotify(
					theEvent->xbutton.x, theEvent->xbutton.y);
				MouseButtonSet(false);
			}
			break;
#if UseMotionEvents
		case MotionNotify:
			if (theEvent->xmotion.window != main_wind) {
				WriteExtraErr("Got MotionNotify for some other window");
			} else {
				MousePositionNotify(
					theEvent->xmotion.x, theEvent->xmotion.y);
			}
			break;
		case EnterNotify:
			if (theEvent->xcrossing.window != main_wind) {
				WriteExtraErr("Got EnterNotify for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - EnterNotify");
#endif

				CaughtMouse = true;
				MousePositionNotify(
					theEvent->xcrossing.x, theEvent->xcrossing.y);
			}
			break;
		case LeaveNotify:
			if (theEvent->xcrossing.window != main_wind) {
				WriteExtraErr("Got LeaveNotify for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - LeaveNotify");
#endif

				MousePositionNotify(
					theEvent->xcrossing.x, theEvent->xcrossing.y);
				CaughtMouse = false;
			}
			break;
#endif
		case Expose:
			if (theEvent->xexpose.window != main_wind) {
				WriteExtraErr(
					"Got SelectionRequest for some other window");
			} else {
				int x0 = theEvent->xexpose.x;
				int y0 = theEvent->xexpose.y;
				int x1 = x0 + theEvent->xexpose.width;
				int y1 = y0 + theEvent->xexpose.height;

#if 0 && DbgEvents
				dbglog_writeln("- event - Expose");
#endif

#if VarFullScreen
				if (UseFullScreen)
#endif
#if MayFullScreen
				{
					x0 -= hOffset;
					y0 -= vOffset;
					x1 -= hOffset;
					y1 -= vOffset;
				}
#endif

#if EnableMagnify
				if (UseMagnify) {
					x0 /= WindowScale;
					y0 /= WindowScale;
					x1 = (x1 + (WindowScale - 1)) / WindowScale;
					y1 = (y1 + (WindowScale - 1)) / WindowScale;
				}
#endif

#if VarFullScreen
				if (UseFullScreen)
#endif
#if MayFullScreen
				{
					x0 += ViewHStart;
					y0 += ViewVStart;
					x1 += ViewHStart;
					y1 += ViewVStart;
				}
#endif

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

				NeedFinishOpen1 = false;
			}
			break;
#if IncludeHostTextClipExchange
		case SelectionRequest:
			if (theEvent->xselectionrequest.owner != main_wind) {
				WriteExtraErr(
					"Got SelectionRequest for some other window");
			} else {
				XEvent xevent;
				bool RequestFilled = false;

#if DbgEvents
				dbglog_writeln("- event - SelectionRequest");
				WriteDbgAtom("selection",
					theEvent->xselectionrequest.selection);
				WriteDbgAtom("target",
					theEvent->xselectionrequest.target);
				WriteDbgAtom("property",
					theEvent->xselectionrequest.property);
#endif

				if (theEvent->xselectionrequest.selection ==
					XA_CLIPBOARD)
				{
					RequestFilled =
						HandleSelectionRequestClipboard(theEvent);
				}


				memset(&xevent, 0, sizeof(xevent));
				xevent.xselection.type = SelectionNotify;
				xevent.xselection.display = x_display;
				xevent.xselection.requestor =
					theEvent->xselectionrequest.requestor;
				xevent.xselection.selection =
					theEvent->xselectionrequest.selection;
				xevent.xselection.target =
					theEvent->xselectionrequest.target;
				xevent.xselection.property = (! RequestFilled) ? None
					: theEvent->xselectionrequest.property ;
				xevent.xselection.time =
					theEvent->xselectionrequest.time;

				if (0 == XSendEvent(x_display,
					xevent.xselection.requestor, False, 0, &xevent))
				{
					WriteExtraErr(
						"XSendEvent failed in SelectionRequest");
				}
			}
			break;
		case SelectionClear:
			if (theEvent->xselectionclear.window != main_wind) {
				WriteExtraErr(
					"Got SelectionClear for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - SelectionClear");
				WriteDbgAtom("selection",
					theEvent->xselectionclear.selection);
#endif

				if (theEvent->xselectionclear.selection ==
					XA_CLIPBOARD)
				{
					FreeClipBuffer();
				}
			}
			break;
#endif
#if EnableDragDrop
		case SelectionNotify:
			if (theEvent->xselection.requestor != main_wind) {
				WriteExtraErr(
					"Got SelectionNotify for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - SelectionNotify");
				WriteDbgAtom("selection",
					theEvent->xselection.selection);
				WriteDbgAtom("target", theEvent->xselection.target);
				WriteDbgAtom("property", theEvent->xselection.property);
#endif

				if (theEvent->xselection.selection == XA_DndSelection)
				{
					HandleSelectionNotifyDnd(theEvent);
				} else {
					WriteExtraErr(
						"Got Unknown selection in SelectionNotify");
				}
			}
			break;
#endif
		case ClientMessage:
			if (theEvent->xclient.window != main_wind) {
				WriteExtraErr(
					"Got ClientMessage for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - ClientMessage");
				WriteDbgAtom("message_type",
					theEvent->xclient.message_type);
#endif

#if EnableDragDrop
				if (theEvent->xclient.message_type == XA_DndEnter) {
					/* printf("Got XdndEnter\n"); */
				} else if (theEvent->xclient.message_type ==
					XA_DndLeave)
				{
					/* printf("Got XdndLeave\n"); */
				} else if (theEvent->xclient.message_type ==
					XA_DndPosition)
				{
					HandleClientMessageDndPosition(theEvent);
				} else if (theEvent->xclient.message_type ==
					XA_DndDrop)
				{
					HandleClientMessageDndDrop(theEvent);
				} else
#endif
				{
					if ((32 == theEvent->xclient.format) &&
						(theEvent->xclient.data.l[0] == XA_DeleteW))
					{
						/*
							I would think that should test that
								WM_PROTOCOLS == message_type
							but none of the other programs I looked
							at did.
						*/
						RequestMacOff = true;
					}
				}
			}
			break;
		case FocusIn:
			if (theEvent->xfocus.window != main_wind) {
				WriteExtraErr("Got FocusIn for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - FocusIn");
#endif

				gTrueBackgroundFlag = false;
#if UseMotionEvents
				CheckMouseState();
					/*
						Doesn't help on x11 for OS X,
						can't get new mouse position
						in any fashion until mouse moves.
					*/
#endif
			}
			break;
		case FocusOut:
			if (theEvent->xfocus.window != main_wind) {
				WriteExtraErr("Got FocusOut for some other window");
			} else {
#if DbgEvents
				dbglog_writeln("- event - FocusOut");
#endif

				gTrueBackgroundFlag = true;
			}
			break;
		default:
			break;
	}
}

/* --- main window creation and disposal --- */

LOCALVAR int argc;
LOCALVAR char **argv;

LOCALVAR char *display_name = NULL;

LOCALFUNC bool Screen_Init(void)
{
	Window rootwin;
	int screen;
	Colormap Xcmap;
	Visual *Xvisual;

	x_display = XOpenDisplay(display_name);
	if (NULL == x_display) {
		fprintf(stderr, "Cannot connect to X server.\n");
		return false;
	}

	screen = DefaultScreen(x_display);

	rootwin = XRootWindow(x_display, screen);

	Xcmap = DefaultColormap(x_display, screen);

	Xvisual = DefaultVisual(x_display, screen);

	LoadXA();

	XParseColor(x_display, Xcmap, "#000000", &x_black);
	if (! XAllocColor(x_display, Xcmap, &x_black)) {
		WriteExtraErr("XParseColor black fails");
	}
	XParseColor(x_display, Xcmap, "#ffffff", &x_white);
	if (! XAllocColor(x_display, Xcmap, &x_white)) {
		WriteExtraErr("XParseColor white fails");
	}

	if (! CreateBlankCursor(rootwin)) {
		return false;
	}

#if ! UseColorImage
	image = XCreateImage(x_display, Xvisual, 1, XYBitmap, 0,
		NULL /* (char *)image_Mem1 */,
		vMacScreenWidth, vMacScreenHeight, 32,
		vMacScreenMonoByteWidth);
	if (NULL == image) {
		fprintf(stderr, "XCreateImage failed.\n");
		return false;
	}

#if 0
	fprintf(stderr, "bitmap_bit_order = %d\n",
		(int)image->bitmap_bit_order);
	fprintf(stderr, "byte_order = %d\n", (int)image->byte_order);
#endif

	image->bitmap_bit_order = MSBFirst;
	image->byte_order = MSBFirst;
#endif

#if UseColorImage
	image = XCreateImage(x_display, Xvisual, 24, ZPixmap, 0,
		NULL /* (char *)image_Mem1 */,
		vMacScreenWidth, vMacScreenHeight, 32,
			4 * (uint32_t)vMacScreenWidth);
	if (NULL == image) {
		fprintf(stderr, "XCreateImage Color failed.\n");
		return false;
	}

#if 0
	fprintf(stderr, "DefaultDepth = %d\n",
		(int)DefaultDepth(x_display, screen));

	fprintf(stderr, "MSBFirst = %d\n", (int)MSBFirst);
	fprintf(stderr, "LSBFirst = %d\n", (int)LSBFirst);

	fprintf(stderr, "bitmap_bit_order = %d\n",
		(int)image->bitmap_bit_order);
	fprintf(stderr, "byte_order = %d\n",
		(int)image->byte_order);
	fprintf(stderr, "bitmap_unit = %d\n",
		(int)image->bitmap_unit);
	fprintf(stderr, "bits_per_pixel = %d\n",
		(int)image->bits_per_pixel);
	fprintf(stderr, "red_mask = %d\n",
		(int)image->red_mask);
	fprintf(stderr, "green_mask = %d\n",
		(int)image->green_mask);
	fprintf(stderr, "blue_mask = %d\n",
		(int)image->blue_mask);
#endif

#endif /* UseColorImage */

#if EnableMagnify && (! UseColorImage)
	Scaled_image = XCreateImage(x_display, Xvisual,
		1, XYBitmap, 0,
		NULL /* (char *)image_Mem1 */,
		vMacScreenWidth * WindowScale,
		vMacScreenHeight * WindowScale,
		32, vMacScreenMonoByteWidth * WindowScale);
	if (NULL == Scaled_image) {
		fprintf(stderr, "XCreateImage failed.\n");
		return false;
	}

	Scaled_image->bitmap_bit_order = MSBFirst;
	Scaled_image->byte_order = MSBFirst;
#endif

#if EnableMagnify && UseColorImage
	Scaled_image = XCreateImage(x_display, Xvisual,
		24, ZPixmap, 0,
		NULL /* (char *)image_Mem1 */,
		vMacScreenWidth * WindowScale,
		vMacScreenHeight * WindowScale,
		32, 4 * (uint32_t)vMacScreenWidth * WindowScale);
	if (NULL == Scaled_image) {
		fprintf(stderr, "XCreateImage Scaled failed.\n");
		return false;
	}
#endif

#if 0 != vMacScreenDepth
	ColorModeWorks = true;
#endif

	DisableKeyRepeat();

	return true;
}

LOCALPROC CloseMainWindow(void)
{
	if (gc != NULL) {
		XFreeGC(x_display, gc);
		gc = NULL;
	}
	if (main_wind) {
		XDestroyWindow(x_display, main_wind);
		main_wind = 0;
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
LOCALVAR int WinPositionWinsH[kNumMagStates];
LOCALVAR int WinPositionWinsV[kNumMagStates];
#endif

#if EnableRecreateW
LOCALPROC ZapWState(void)
{
	main_wind = 0;
	gc = NULL;
}
#endif

LOCALFUNC bool CreateMainWindow(void)
{
	Window rootwin;
	int screen;
	int xr;
	int yr;
	unsigned int dr;
	unsigned int wr;
	unsigned int hr;
	unsigned int bwr;
	Window rr;
	int leftPos;
	int topPos;
#if MayNotFullScreen
	int WinIndx;
#endif
#if EnableDragDrop
	long int xdnd_version = 5;
#endif
	int NewWindowHeight = vMacScreenHeight;
	int NewWindowWidth = vMacScreenWidth;

	/* Get connection to X Server */
	screen = DefaultScreen(x_display);

	rootwin = XRootWindow(x_display, screen);

	XGetGeometry(x_display, rootwin,
		&rr, &xr, &yr, &wr, &hr, &bwr, &dr);

#if EnableMagnify
	if (UseMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}
#endif

	if (wr > NewWindowWidth) {
		leftPos = (wr - NewWindowWidth) / 2;
	} else {
		leftPos = 0;
	}
	if (hr > NewWindowHeight) {
		topPos = (hr - NewWindowHeight) / 2;
	} else {
		topPos = 0;
	}

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		ViewHSize = wr;
		ViewVSize = hr;
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

#if VarFullScreen
	if (! UseFullScreen)
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
			WinPositionWinsH[WinIndx] = leftPos;
			WinPositionWinsV[WinIndx] = topPos;
			HavePositionWins[WinIndx] = true;
		} else {
			leftPos = WinPositionWinsH[WinIndx];
			topPos = WinPositionWinsV[WinIndx];
		}
	}
#endif

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		XSetWindowAttributes xattr;
		xattr.override_redirect = True;
		xattr.background_pixel = x_black.pixel;
		xattr.border_pixel = x_white.pixel;

		main_wind = XCreateWindow(x_display, rr,
			0, 0, wr, hr, 0,
			CopyFromParent, /* depth */
			InputOutput, /* class */
			CopyFromParent, /* visual */
			CWOverrideRedirect | CWBackPixel | CWBorderPixel,
				/* valuemask */
			&xattr /* attributes */);
	}
#endif
#if VarFullScreen
	else
#endif
#if MayNotFullScreen
	{
		main_wind = XCreateSimpleWindow(x_display, rootwin,
			leftPos,
			topPos,
			NewWindowWidth, NewWindowHeight, 4,
			x_white.pixel,
			x_black.pixel);
	}
#endif

	if (! main_wind) {
		WriteExtraErr("XCreateSimpleWindow failed.");
		return false;
	} else {
		char *win_name =
			(NULL != n_arg) ? n_arg : (
#ifdef CanGetAppPath
			(NULL != app_name) ? app_name :
#endif
			kStrAppName);
		XSelectInput(x_display, main_wind,
			ExposureMask | KeyPressMask | KeyReleaseMask
			| ButtonPressMask | ButtonReleaseMask
#if UseMotionEvents
			| PointerMotionMask | EnterWindowMask | LeaveWindowMask
#endif
			| FocusChangeMask);

		XStoreName(x_display, main_wind, win_name);
		XSetIconName(x_display, main_wind, win_name);

		{
			XClassHint *hints = XAllocClassHint();
			if (hints) {
				hints->res_name = kStrAppName;
				hints->res_class = kStrAppName;
				XSetClassHint(x_display, main_wind, hints);
				XFree(hints);
			}
		}

		{
			XWMHints *hints = XAllocWMHints();
			if (hints) {
				hints->input = True;
				hints->initial_state = NormalState;
				hints->flags = InputHint | StateHint;
				XSetWMHints(x_display, main_wind, hints);
				XFree(hints);
			}

		}

		XSetCommand(x_display, main_wind, argv, argc);

		/* let us handle a click on the close box */
		XSetWMProtocols(x_display, main_wind, &XA_DeleteW, 1);

#if EnableDragDrop
		XChangeProperty (x_display, main_wind, XA_DndAware,
			XA_ATOM, 32, PropModeReplace,
			(unsigned char *) &xdnd_version, 1);
#endif

		gc = XCreateGC(x_display, main_wind, 0, NULL);
		if (NULL == gc) {
			WriteExtraErr("XCreateGC failed.");
			return false;
		}
		XSetState(x_display, gc, x_black.pixel, x_white.pixel,
			GXcopy, AllPlanes);

#if VarFullScreen
		if (! UseFullScreen)
#endif
#if MayNotFullScreen
		{
			XSizeHints *hints = XAllocSizeHints();
			if (hints) {
				hints->min_width = NewWindowWidth;
				hints->max_width = NewWindowWidth;
				hints->min_height = NewWindowHeight;
				hints->max_height = NewWindowHeight;

				/*
					Try again to say where the window ought to go.
					I've seen this described as obsolete, but it
					seems to work on all x implementations tried
					so far, and nothing else does.
				*/
				hints->x = leftPos;
				hints->y = topPos;
				hints->width = NewWindowWidth;
				hints->height = NewWindowHeight;

				hints->flags = PMinSize | PMaxSize | PPosition | PSize;
				XSetWMNormalHints(x_display, main_wind, hints);
				XFree(hints);
			}
		}
#endif

#if VarFullScreen
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			hOffset = leftPos;
			vOffset = topPos;
		}
#endif

		DisconnectKeyCodes3();
			/* since will lose keystrokes to old window */

#if MayNotFullScreen
		CurWinIndx = WinIndx;
#endif

		XMapRaised(x_display, main_wind);

#if 0
		XSync(x_display, 0);
#endif

#if 0
		/*
			This helps in Red Hat 9 to get the new window
			activated, and I've seen other programs
			do similar things.
		*/
		/*
			In current scheme, haven't closed old window
			yet. If old window full screen, never receive
			expose event for new one.
		*/
		{
			XEvent event;

			do {
				XNextEvent(x_display, &event);
				HandleTheEvent(&event);
			} while (! ((Expose == event.type)
				&& (event.xexpose.window == main_wind)));
		}
#endif

		NeedFinishOpen1 = true;
		NeedFinishOpen2 = true;

		return true;
	}
}

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if MayFullScreen
LOCALPROC GrabTheMachine(void)
{
#if EnableFSMouseMotion
	StartSaveMouseMotion();
#endif
#if GrabKeysFullScreen
	GrabKeyboard();
#endif
}
#endif

#if MayFullScreen
LOCALPROC UngrabMachine(void)
{
#if EnableFSMouseMotion
	StopSaveMouseMotion();
#endif
#if GrabKeysFullScreen
	UnGrabKeyboard();
#endif
}
#endif

#if EnableRecreateW
struct WState {
	Window f_main_wind;
	GC f_gc;
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
};
typedef struct WState WState;
#endif

#if EnableRecreateW
LOCALPROC GetWState(WState *r)
{
	r->f_main_wind = main_wind;
	r->f_gc = gc;
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
}
#endif

#if EnableRecreateW
LOCALPROC SetWState(WState *r)
{
	main_wind = r->f_main_wind;
	gc = r->f_gc;
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
}
#endif

#if EnableRecreateW
LOCALVAR bool WantRestoreCursPos = false;
LOCALVAR uint16_t RestoreMouseH;
LOCALVAR uint16_t RestoreMouseV;
#endif

#if EnableRecreateW
LOCALFUNC bool ReCreateMainWindow(void)
{
	WState old_state;
	WState new_state;
#if IncludeHostTextClipExchange
	bool OwnClipboard = false;
#endif

	if (HaveCursorHidden) {
		WantRestoreCursPos = true;
		RestoreMouseH = CurMouseH;
		RestoreMouseV = CurMouseV;
	}

	ForceShowCursor(); /* hide/show cursor api is per window */

#if MayNotFullScreen
#if VarFullScreen
	if (! UseFullScreen)
#endif
	if (main_wind)
	if (! NeedFinishOpen2)
	{
		/* save old position */
		int xr;
		int yr;
		unsigned int dr;
		unsigned int wr;
		unsigned int hr;
		unsigned int bwr;
		Window rr;
		Window rr2;

		/* Get connection to X Server */
		int screen = DefaultScreen(x_display);

		Window rootwin = XRootWindow(x_display, screen);

		XGetGeometry(x_display, rootwin,
			&rr, &xr, &yr, &wr, &hr, &bwr, &dr);

		/*
			Couldn't reliably find out where window
			is now, due to what seem to be some
			broken X implementations, and so instead
			track how far window has moved.
		*/
		XSync(x_display, 0);
		if (XTranslateCoordinates(x_display, main_wind, rootwin,
			0, 0, &xr, &yr, &rr2))
		{
			int newposh =
				WinPositionWinsH[CurWinIndx] + (xr - SavedTransH);
			int newposv =
				WinPositionWinsV[CurWinIndx] + (yr - SavedTransV);
			if ((newposv > 0) && (newposv < hr) && (newposh < wr)) {
				WinPositionWinsH[CurWinIndx] = newposh;
				WinPositionWinsV[CurWinIndx] = newposv;
				SavedTransH = xr;
				SavedTransV = yr;
			}
		}
	}
#endif

#if MayFullScreen
	if (GrabMachine) {
		GrabMachine = false;
		UngrabMachine();
	}
#endif

	GetWState(&old_state);
	ZapWState();

#if EnableMagnify
	UseMagnify = WantMagnify;
#endif
#if VarFullScreen
	UseFullScreen = WantFullScreen;
#endif

	ColorTransValid = false;

	if (! CreateMainWindow()) {
		CloseMainWindow();
		SetWState(&old_state);

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

#if IncludeHostTextClipExchange
		if (main_wind) {
			if (XGetSelectionOwner(x_display, XA_CLIPBOARD) ==
				main_wind)
			{
				OwnClipboard = true;
			}
		}
#endif

		CloseMainWindow();

		SetWState(&new_state);

#if IncludeHostTextClipExchange
		if (OwnClipboard) {
			XSetSelectionOwner(x_display, XA_CLIPBOARD,
				main_wind, CurrentTime);
		}
#endif
	}

	return true;
}
#endif

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
				Window rootwin;
				int xr;
				int yr;
				unsigned int dr;
				unsigned int wr;
				unsigned int hr;
				unsigned int bwr;
				Window rr;

				rootwin =
					XRootWindow(x_display, DefaultScreen(x_display));
				XGetGeometry(x_display, rootwin,
					&rr, &xr, &yr, &wr, &hr, &bwr, &dr);
				if ((wr >= vMacScreenWidth * WindowScale)
					&& (hr >= vMacScreenHeight * WindowScale)
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

/* --- SavedTasks --- */

LOCALPROC LeaveBackground(void)
{
	ReconnectKeyCodes3();
	DisableKeyRepeat();
}

LOCALPROC EnterBackground(void)
{
	RestoreKeyRepeat();
	DisconnectKeyCodes3();

	ForceShowCursor();
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

	if (NeedFinishOpen2 && ! NeedFinishOpen1) {
		NeedFinishOpen2 = false;

#if VarFullScreen
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			XSetInputFocus(x_display, main_wind,
				RevertToPointerRoot, CurrentTime);
		}
#endif
#if VarFullScreen
		else
#endif
#if MayNotFullScreen
		{
			Window rr;
			int screen = DefaultScreen(x_display);
			Window rootwin = XRootWindow(x_display, screen);
#if 0
			/*
				This doesn't work right in Red Hat 6, and may not
				be needed anymore, now that using PPosition hint.
			*/
			XMoveWindow(x_display, main_wind,
				leftPos, topPos);
				/*
					Needed after XMapRaised, because some window
					managers will apparently ignore where the
					window was asked to be put.
				*/
#endif

			XSync(x_display, 0);
				/*
					apparently, XTranslateCoordinates can be inaccurate
					without this
				*/
			XTranslateCoordinates(x_display, main_wind, rootwin,
				0, 0, &SavedTransH, &SavedTransV, &rr);
		}
#endif

#if EnableRecreateW
		if (WantRestoreCursPos) {
#if EnableFSMouseMotion
			if (! HaveMouseMotion)
#endif
			{
				(void) MoveMouse(RestoreMouseH, RestoreMouseV);
				WantCursorHidden = true;
			}
			WantRestoreCursPos = false;
		}
#endif
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
		(gBackgroundFlag && ! RunInBackground)))
	{
		CurSpeedStopped = ! CurSpeedStopped;
		if (CurSpeedStopped) {
			EnterSpeedStopped();
		} else {
			LeaveSpeedStopped();
		}
	}

#if MayFullScreen
	if (gTrueBackgroundFlag
#if VarFullScreen
		&& WantFullScreen
#endif
		)
	{
		/*
			Since often get here on Ubuntu Linux 5.10
			running on a slow machine (emulated) when
			attempt to enter full screen, don't abort
			full screen, but try to fix it.
		*/
#if 0
		ToggleWantFullScreen();
#else
		XRaiseWindow(x_display, main_wind);
		XSetInputFocus(x_display, main_wind,
			RevertToPointerRoot, CurrentTime);
#endif
	}
#endif

#if EnableRecreateW
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
		if (GrabMachine) {
			GrabTheMachine();
		} else {
			UngrabMachine();
		}
	}
#endif

#if IncludeSonyNew
	if (vSonyNewDiskWanted) {
#if IncludeSonyNameNew
		if (vSonyNewDiskName != NotAPbuf) {
			uint8_t * NewDiskNameDat;
			if (MacRomanTextToNativePtr(vSonyNewDiskName, true,
				&NewDiskNameDat))
			{
				MakeNewDisk(vSonyNewDiskSize, (char *)NewDiskNameDat);
				free(NewDiskNameDat);
			}
			PbufDispose(vSonyNewDiskName);
			vSonyNewDiskName = NotAPbuf;
		} else
#endif
		{
			MakeNewDiskAtDefault(vSonyNewDiskSize);
		}
		vSonyNewDiskWanted = false;
			/* must be done after may have gotten disk */
	}
#endif

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

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
		if (HaveCursorHidden) {
			XDefineCursor(x_display, main_wind, blankCursor);
		} else {
			XUndefineCursor(x_display, main_wind);
		}
	}
}

/* --- command line parsing --- */

LOCALFUNC bool ScanCommandLine(void)
{
	char *pa;
	int i = 1;

label_retry:
	if (i < argc) {
		pa = argv[i++];
		if ('-' == pa[0]) {
			if ((0 == strcmp(pa, "--display"))
				|| (0 == strcmp(pa, "-display")))
			{
				if (i < argc) {
					display_name = argv[i++];
					goto label_retry;
				}
			} else
			if ((0 == strcmp(pa, "--rom"))
				|| (0 == strcmp(pa, "-r")))
			{
				if (i < argc) {
					rom_path = argv[i++];
					goto label_retry;
				}
			} else
			if (0 == strcmp(pa, "-n"))
			{
				if (i < argc) {
					n_arg = argv[i++];
					goto label_retry;
				}
			} else
			if (0 == strcmp(pa, "-d"))
			{
				if (i < argc) {
					d_arg = argv[i++];
					goto label_retry;
				}
			} else
#ifndef UsingAlsa
#define UsingAlsa 0
#endif

#if UsingAlsa
			if ((0 == strcmp(pa, "--alsadev"))
				|| (0 == strcmp(pa, "-alsadev")))
			{
				if (i < argc) {
					alsadev_name = argv[i++];
					goto label_retry;
				}
			} else
#endif
#if 0
			if (0 == strcmp(pa, "-l")) {
				SpeedValue = 0;
				goto label_retry;
			} else
#endif
			{
				MacMsg(kStrBadArgTitle, kStrBadArgMessage, false);
			}
		} else {
			(void) Sony_Insert1(pa, false);
			goto label_retry;
		}
	}

	return true;
}

/* --- main program flow --- */

GLOBALOSGLUPROC DoneWithDrawingForTick(void)
{
#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		AutoScrollScreen();
	}
#endif
	DrawChangesAndClear();
	XFlush(x_display);
}

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

LOCALPROC WaitForTheNextEvent(void)
{
	XEvent event;

	XNextEvent(x_display, &event);
	HandleTheEvent(&event);
}

LOCALPROC CheckForSystemEvents(void)
{
	int i = 10;

	while ((XEventsQueued(x_display, QueuedAfterReading) > 0)
		&& (--i >= 0))
	{
		WaitForTheNextEvent();
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
		struct timespec rqt;
		struct timespec rmt;

		int32_t TimeDiff = GetTimeDiff();
		if (TimeDiff < 0) {
			rqt.tv_sec = 0;
			rqt.tv_nsec = (- TimeDiff) * 1000;
			(void) nanosleep(&rqt, &rmt);
		}
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

LOCALPROC ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
}

LOCALPROC ReserveAllocAll(void)
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
#if WantScalingBuff
	ReserveAllocOneBlock(&ScalingBuff,
		ScalingBuffsz, 5, false);
#endif
#if WantScalingTabl
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

LOCALPROC UnallocMemory(void)
{
	if (nullpr != ReserveAllocBigBlock) {
		free((char *)ReserveAllocBigBlock);
	}
}

#ifdef HaveAppPathLink
LOCALFUNC bool ReadLink_Alloc(char *path, char **r)
{
	/*
		This should work to find size:

		struct stat r;

		if (lstat(path, &r) != -1) {
			r = r.st_size;
			IsOk = true;
		}

		But observed to return 0 in Ubuntu 10.04 x86-64
	*/

	char *s;
	int sz;
	char *p;
	bool IsOk = false;
	size_t s_alloc = 256;

label_retry:
	s = (char *)malloc(s_alloc);
	if (NULL == s) {
		fprintf(stderr, "malloc failed.\n");
	} else {
		sz = readlink(path, s, s_alloc);
		if ((sz < 0) || (sz >= s_alloc)) {
			free(s);
			if (sz == s_alloc) {
				s_alloc <<= 1;
				goto label_retry;
			} else {
				fprintf(stderr, "readlink failed.\n");
			}
		} else {
			/* ok */
			p = (char *)malloc(sz + 1);
			if (NULL == p) {
				fprintf(stderr, "malloc failed.\n");
			} else {
				(void) memcpy(p, s, sz);
				p[sz] = 0;
				*r = p;
				IsOk = true;
			}
			free(s);
		}
	}

	return IsOk;
}
#endif

#ifdef HaveSysctlPath
LOCALFUNC bool ReadKernProcPathname(char **r)
{
	size_t s_alloc;
	char *s;
	int mib[] = {
		CTL_KERN,
		KERN_PROC,
		KERN_PROC_PATHNAME,
		-1
	};
	bool IsOk = false;

	if (0 != sysctl(mib, sizeof(mib) / sizeof(int),
		NULL, &s_alloc, NULL, 0))
	{
		fprintf(stderr, "sysctl failed.\n");
	} else {
		s = (char *)malloc(s_alloc);
		if (NULL == s) {
			fprintf(stderr, "malloc failed.\n");
		} else {
			if (0 != sysctl(mib, sizeof(mib) / sizeof(int),
				s, &s_alloc, NULL, 0))
			{
				fprintf(stderr, "sysctl 2 failed.\n");
			} else {
				*r = s;
				IsOk = true;
			}
			if (! IsOk) {
				free(s);
			}
		}
	}

	return IsOk;
}
#endif

#ifdef CanGetAppPath
LOCALFUNC bool Path2ParentAndName(char *path,
	char **parent, char **name)
{
	bool IsOk = false;

	char *t = strrchr(path, '/');
	if (NULL == t) {
		fprintf(stderr, "no directory.\n");
	} else {
		int par_sz = t - path;
		char *par = (char *)malloc(par_sz + 1);
		if (NULL == par) {
			fprintf(stderr, "malloc failed.\n");
		} else {
			(void) memcpy(par, path, par_sz);
			par[par_sz] = 0;
			{
				int s_sz = strlen(path);
				int child_sz = s_sz - par_sz - 1;
				char *child = (char *)malloc(child_sz + 1);
				if (NULL == child) {
					fprintf(stderr, "malloc failed.\n");
				} else {
					(void) memcpy(child, t + 1, child_sz);
					child[child_sz] = 0;

					*name = child;
					IsOk = true;
					/* free(child); */
				}
			}
			if (! IsOk) {
				free(par);
			} else {
				*parent = par;
			}
		}
	}

	return IsOk;
}
#endif

#ifdef CanGetAppPath
LOCALFUNC bool InitWhereAmI(void)
{
	char *s;

	if (!
#ifdef HaveAppPathLink
		ReadLink_Alloc(TheAppPathLink, &s)
#endif
#ifdef HaveSysctlPath
		ReadKernProcPathname(&s)
#endif
		)
	{
		fprintf(stderr, "InitWhereAmI fails.\n");
	} else {
		if (! Path2ParentAndName(s, &app_parent, &app_name)) {
			fprintf(stderr, "Path2ParentAndName fails.\n");
		} else {
			/* ok */
			/*
				fprintf(stderr, "parent = %s.\n", app_parent);
				fprintf(stderr, "name = %s.\n", app_name);
			*/
		}

		free(s);
	}

	return true; /* keep going regardless */
}
#endif

#ifdef CanGetAppPath
LOCALPROC UninitWhereAmI(void)
{
	MayFree(app_parent);
	MayFree(app_name);
}
#endif

LOCALFUNC bool InitOSGLU(void)
{
	if (AllocMemory())
#ifdef CanGetAppPath
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
	if (Screen_Init())
	if (CreateMainWindow())
	if (KC2MKCInit())
	if (WaitForRom())
	{
		return true;
	}
	return false;
}

LOCALPROC UnInitOSGLU(void)
{
	if (MacMsgDisplayed) {
		MacMsgDisplayOff();
	}

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
#if IncludeHostTextClipExchange
	FreeClipBuffer();
#endif
#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	ForceShowCursor();
	if (blankCursor != None) {
		XFreeCursor(x_display, blankCursor);
	}

	if (image != NULL) {
		XDestroyImage(image);
	}
#if EnableMagnify
	if (Scaled_image != NULL) {
		XDestroyImage(Scaled_image);
	}
#endif

	CloseMainWindow();
	if (x_display != NULL) {
		XCloseDisplay(x_display);
	}

#if dbglog_HAVE
	dbglog_close();
#endif

#ifdef CanGetAppPath
	UninitWhereAmI();
#endif
	UnallocMemory();

	CheckSavedMacMsg();
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
