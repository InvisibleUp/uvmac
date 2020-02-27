/*
	OSGLUCCO.m

	Copyright (C) 2012 Paul C. Pratt, SDL by Sam Lantinga and others

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
	Operating System GLUe for mac os CoCOa

	All operating system dependent code for the
	Mac OS Cocoa should go here.

	Originally derived from Cocoa port of SDL Library
	by Sam Lantinga (but little trace of that remains).
*/

#include "CNFGRAPI.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"

#include "UI/MYOSGLUE.h"

#include "STRCONST.h"

/* --- adapting to API/ABI version differences --- */


#ifndef MAC_OS_X_VERSION_10_5
#define MAC_OS_X_VERSION_10_5 1050
#endif

#ifndef MAC_OS_X_VERSION_10_6
#define MAC_OS_X_VERSION_10_6 1060
#endif


#ifndef WantGraphicsSwitching
#define WantGraphicsSwitching 0
#endif

#if MAC_OS_X_VERSION_10_5 > MAC_OS_X_VERSION_MAX_ALLOWED

typedef unsigned long NSUInteger;
typedef long NSInteger;

typedef struct __CFError * CFErrorRef;

#if WantGraphicsSwitching
#define NSOpenGLPFAAllowOfflineRenderers \
	(NSOpenGLPixelFormatAttribute)96
#endif

#endif

#if MAC_OS_X_VERSION_10_6 > MAC_OS_X_VERSION_MAX_ALLOWED

@protocol NSWindowDelegate <NSObject> @end
@protocol NSApplicationDelegate <NSObject> @end

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

#if MayFullScreen

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


typedef Boolean (*CFURLCopyResourcePropertyForKeyProcPtr) (
	CFURLRef    url,
	CFStringRef key,
	void        *propertyValueTypeRefPtr,
	CFErrorRef  *error
	);
LOCALVAR CFURLCopyResourcePropertyForKeyProcPtr
	CFURLCopyResourcePropertyForKey = NULL;
LOCALVAR bool DidCFURLCopyResourcePropertyForKey = false;

LOCALFUNC bool HaveCFURLCopyResourcePropertyForKey(void)
{
	if (! DidCFURLCopyResourcePropertyForKey) {
		if (HaveApplicationServicesBun()) {
			CFURLCopyResourcePropertyForKey =
				(CFURLCopyResourcePropertyForKeyProcPtr)
				CFBundleGetFunctionPointerForName(
					AppServBunRef,
					CFSTR("CFURLCopyResourcePropertyForKey"));
		}
		DidCFURLCopyResourcePropertyForKey = true;
	}
	return (CFURLCopyResourcePropertyForKey != NULL);
}


LOCALVAR const CFStringRef *kCFURLIsAliasFileKey
	= NULL;
LOCALVAR bool DidkCFURLIsAliasFileKey = false;

LOCALFUNC bool HavekCFURLIsAliasFileKey(void)
{
	if (! DidkCFURLIsAliasFileKey) {
		if (HaveApplicationServicesBun()) {
			kCFURLIsAliasFileKey =
				(const CFStringRef *)
				CFBundleGetDataPointerForName(
					AppServBunRef, CFSTR("kCFURLIsAliasFileKey"));
		}
		DidkCFURLIsAliasFileKey = true;
	}
	return (kCFURLIsAliasFileKey != NULL);
}


LOCALVAR const CFStringRef *kCFURLIsSymbolicLinkKey
	= NULL;
LOCALVAR bool DidkCFURLIsSymbolicLinkKey = false;

LOCALFUNC bool HavekCFURLIsSymbolicLinkKey(void)
{
	if (! DidkCFURLIsSymbolicLinkKey) {
		if (HaveApplicationServicesBun()) {
			kCFURLIsSymbolicLinkKey =
				(const CFStringRef *)
				CFBundleGetDataPointerForName(
					AppServBunRef, CFSTR("kCFURLIsSymbolicLinkKey"));
		}
		DidkCFURLIsSymbolicLinkKey = true;
	}
	return (kCFURLIsSymbolicLinkKey != NULL);
}


typedef CFDataRef (*CFURLCreateBookmarkDataFromFileProcPtr) (
	CFAllocatorRef allocator, CFURLRef fileURL, CFErrorRef *errorRef);
LOCALVAR CFURLCreateBookmarkDataFromFileProcPtr
	CFURLCreateBookmarkDataFromFile = NULL;
LOCALVAR bool DidCFURLCreateBookmarkDataFromFile = false;

LOCALFUNC bool HaveCFURLCreateBookmarkDataFromFile(void)
{
	if (! DidCFURLCreateBookmarkDataFromFile) {
		if (HaveApplicationServicesBun()) {
			CFURLCreateBookmarkDataFromFile =
				(CFURLCreateBookmarkDataFromFileProcPtr)
				CFBundleGetFunctionPointerForName(AppServBunRef,
					CFSTR("CFURLCreateBookmarkDataFromFile"));
		}
		DidCFURLCreateBookmarkDataFromFile = true;
	}
	return (CFURLCreateBookmarkDataFromFile != NULL);
}


typedef CFOptionFlags CFURLBookmarkResolutionOptions;

typedef CFURLRef (*CFURLCreateByResolvingBookmarkDataProcPtr) (
	CFAllocatorRef allocator, CFDataRef bookmark,
	CFURLBookmarkResolutionOptions options, CFURLRef relativeToURL,
	CFArrayRef resourcePropertiesToInclude,
	Boolean* isStale, CFErrorRef* error);
LOCALVAR CFURLCreateByResolvingBookmarkDataProcPtr
	CFURLCreateByResolvingBookmarkData = NULL;
LOCALVAR bool DidCFURLCreateByResolvingBookmarkData = false;

LOCALFUNC bool HaveCFURLCreateByResolvingBookmarkData(void)
{
	if (! DidCFURLCreateByResolvingBookmarkData) {
		if (HaveApplicationServicesBun()) {
			CFURLCreateByResolvingBookmarkData =
				(CFURLCreateByResolvingBookmarkDataProcPtr)
				CFBundleGetFunctionPointerForName(AppServBunRef,
					CFSTR("CFURLCreateByResolvingBookmarkData"));
		}
		DidCFURLCreateByResolvingBookmarkData = true;
	}
	return (CFURLCreateByResolvingBookmarkData != NULL);
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


/* --- some simple utilities --- */

GLOBALOSGLUPROC MoveBytes(anyp srcPtr, anyp destPtr, int32_t byteCount)
{
	(void) memcpy((char *)destPtr, (char *)srcPtr, byteCount);
}

/* --- control mode and internationalization --- */

#define NeedCell2UnicodeMap 1

#include "LANG/INTLCHAR.h"

/* --- sending debugging info to file --- */

LOCALVAR NSString *myAppName = nil;
LOCALVAR NSString *DataPath = nil;

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
	NSString *myLogPath = [DataPath
		stringByAppendingPathComponent: @"dbglog.txt"];
	const char *path = [myLogPath fileSystemRepresentation];

	dbglog_File = fopen(path, "w");
	return (NULL != dbglog_File);
#endif
}

LOCALPROC dbglog_write0(char *s, uimr L)
{
#if dbglog_ToStdErr
	(void) fwrite(s, 1, L, stderr);
#else
	if (NULL != dbglog_File) {
		(void) fwrite(s, 1, L, dbglog_File);
	}
#endif
}

LOCALPROC dbglog_close0(void)
{
#if ! dbglog_ToStdErr
	if (NULL != dbglog_File) {
		fclose(dbglog_File);
		dbglog_File = NULL;
	}
#endif
}

#endif

/* --- information about the environment --- */

#define WantColorTransValid 1

#include "UI/COMOSGLU.h"

#define WantKeyboard_RemapMac 1

#include "UTILS/PBUFSTDC.h"

#include "UI/CONTROLM.h"

/* --- text translation --- */

LOCALPROC UniCharStrFromSubstCStr(int *L, unichar *x, char *s)
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

LOCALFUNC NSString * NSStringCreateFromSubstCStr(char *s)
{
	int L;
	unichar x[ClStrMaxLength];

	UniCharStrFromSubstCStr(&L, x, s);

	return [NSString stringWithCharacters:x length:L];
}

#if IncludeSonyNameNew
LOCALFUNC bool MacRomanFileNameToNSString(tPbuf i,
	NSString **r)
{
	uint8_t * p;
	void *Buffer = PbufDat[i];
	uint32_t L = PbufSize[i];

	p = (uint8_t *)malloc(L /* + 1 */);
	if (p != NULL) {
		NSData *d;
		uint8_t *p0 = (uint8_t *)Buffer;
		uint8_t *p1 = (uint8_t *)p;

		if (L > 0) {
			uint32_t j = L;

			do {
				uint8_t x = *p0++;
				if (x < 32) {
					x = '-';
				} else if (x >= 128) {
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
			} while (--j > 0);

			if ('.' == p[0]) {
				p[0] = '-';
			}
		}

#if 0
		*p1 = 0;
		*r = [NSString stringWithCString:(char *)p
			encoding:NSMacOSRomanStringEncoding];
			/* only as of OS X 10.4 */
		free(p);
#endif

		d = [[NSData alloc] initWithBytesNoCopy:p length:L];

		*r = [[[NSString alloc]
			initWithData:d encoding:NSMacOSRomanStringEncoding]
			autorelease];

		[d release];

		return true;
	}

	return false;
}
#endif

#if IncludeSonyGetName || IncludeHostTextClipExchange
LOCALFUNC tMacErr NSStringToRomanPbuf(NSString *string, tPbuf *r)
{
	tMacErr v = mnvm_miscErr;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
#if 0
	const char *s = [s0
		cStringUsingEncoding: NSMacOSRomanStringEncoding];
	uint32_t L = strlen(s);
		/* only as of OS X 10.4 */
#endif
#if 0
	NSData *d0 = [string dataUsingEncoding: NSMacOSRomanStringEncoding];
#endif
	NSData *d0 = [string dataUsingEncoding: NSMacOSRomanStringEncoding
		allowLossyConversion: YES];
	const void *s = [d0 bytes];
	NSUInteger L = [d0 length];

	if (NULL == s) {
		v = mnvm_miscErr;
	} else {
		uint8_t * p = (uint8_t *)malloc(L);

		if (NULL == p) {
			v = mnvm_miscErr;
		} else {
			/* memcpy((char *)p, s, L); */
			uint8_t *p0 = (uint8_t *)s;
			uint8_t *p1 = (uint8_t *)p;
			int i;

			for (i = L; --i >= 0; ) {
				uint8_t v = *p0++;
				if (10 == v) {
					v = 13;
				}
				*p1++ = v;
			}

			v = PbufNewFromPtr(p, L, r);
		}
	}

	[pool release];

	return v;
}
#endif

/* --- drives --- */

LOCALFUNC bool FindNamedChildPath(NSString *parentPath,
	char *ChildName, NSString **childPath)
{
	bool v = false;

#if 0
	NSString *ss = [NSString stringWithCString:s
		encoding:NSASCIIStringEncoding];
		/* only as of OS X 10.4 */
#endif
#if 0
	NSData *d = [NSData dataWithBytes: ChildName
		length: strlen(ChildName)];
	NSString *ss = [[[NSString alloc]
		initWithData:d encoding:NSASCIIStringEncoding]
		autorelease];
#endif
	NSString *ss = NSStringCreateFromSubstCStr(ChildName);
	if (nil != ss) {
		NSString *r = [parentPath stringByAppendingPathComponent: ss];
		if (nil != r) {
			*childPath = r;
			v = true;
		}
	}

	return v;
}

LOCALFUNC NSString *ResolveAlias(NSString *filePath,
	Boolean *targetIsFolder)
{
	NSString *resolvedPath = nil;
	CFURLRef url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
		(CFStringRef)filePath, kCFURLPOSIXPathStyle, NO);


	if (url != NULL) {
		if (HaveCFURLCopyResourcePropertyForKey()
			&& HavekCFURLIsAliasFileKey()
			&& HavekCFURLIsSymbolicLinkKey()
			&& HaveCFURLCreateBookmarkDataFromFile()
			&& HaveCFURLCreateByResolvingBookmarkData())
		{
			BOOL isDir;
			Boolean isStale;
			CFBooleanRef is_alias_file = NULL;
			CFBooleanRef is_symbolic_link = NULL;
			CFDataRef bookmark = NULL;
			CFURLRef resolvedurl = NULL;

			if (CFURLCopyResourcePropertyForKey(url,
				*kCFURLIsAliasFileKey, &is_alias_file, NULL))
			if (CFBooleanGetValue(is_alias_file))
			if (CFURLCopyResourcePropertyForKey(url,
				*kCFURLIsSymbolicLinkKey, &is_symbolic_link, NULL))
			if (! CFBooleanGetValue(is_symbolic_link))
			if (NULL != (bookmark = CFURLCreateBookmarkDataFromFile(
				kCFAllocatorDefault, url, NULL)))
			if (NULL != (resolvedurl =
				CFURLCreateByResolvingBookmarkData(
					kCFAllocatorDefault,
					bookmark,
					0 /* CFURLBookmarkResolutionOptions options */,
					NULL /* relativeToURL */,
					NULL /* resourcePropertiesToInclude */,
					&isStale,
					NULL /* error */)))
			if (nil != (resolvedPath =
				(NSString *)CFURLCopyFileSystemPath(
					resolvedurl, kCFURLPOSIXPathStyle)))
			{
				if ([[NSFileManager defaultManager]
					fileExistsAtPath: resolvedPath isDirectory: &isDir])
				{
					*targetIsFolder = isDir;
				} else
				{
					*targetIsFolder = FALSE;
				}

				[resolvedPath autorelease];
			}

			if (NULL != resolvedurl) {
				CFRelease(resolvedurl);
			}
			if (NULL != bookmark) {
				CFRelease(bookmark);
			}
			if (NULL != is_alias_file) {
				CFRelease(is_alias_file);
			}
			if (NULL != is_symbolic_link) {
				CFRelease(is_symbolic_link);
			}
		} else {
			FSRef fsRef;
			Boolean wasAliased;

			if (CFURLGetFSRef(url, &fsRef)) {
				/*
					FSResolveAliasFile deprecated in 10.8
				*/

				if ((FSResolveAliasFile(&fsRef,
					TRUE /*resolveAliasChains*/,
					targetIsFolder, &wasAliased) == noErr)
					&& wasAliased)
				{
					CFURLRef resolvedurl =
						CFURLCreateFromFSRef(kCFAllocatorDefault,
							&fsRef);
					if (resolvedurl != NULL) {
						resolvedPath =
							(NSString *)CFURLCopyFileSystemPath(
								resolvedurl, kCFURLPOSIXPathStyle);
						[resolvedPath autorelease];
						CFRelease(resolvedurl);
					}
				}
			}
		}

		CFRelease(url);
	}

	return resolvedPath;
}

LOCALFUNC bool FindNamedChildDirPath(NSString *parentPath,
	char *ChildName, NSString **childPath)
{
	NSString *r;
	BOOL isDir;
	Boolean isDirectory;
	bool v = false;

	if (FindNamedChildPath(parentPath, ChildName, &r))
	if ([[NSFileManager defaultManager]
		fileExistsAtPath:r isDirectory: &isDir])
	{
		if (isDir) {
			*childPath = r;
			v = true;
		} else {
			NSString *RslvPath = ResolveAlias(r, &isDirectory);
			if (nil != RslvPath) {
				if (isDirectory) {
					*childPath = RslvPath;
					v = true;
				}
			}
		}
	}

	return v;
}

LOCALFUNC bool FindNamedChildFilePath(NSString *parentPath,
	char *ChildName, NSString **childPath)
{
	NSString *r;
	BOOL isDir;
	Boolean isDirectory;
	bool v = false;

	if (FindNamedChildPath(parentPath, ChildName, &r))
	if ([[NSFileManager defaultManager]
		fileExistsAtPath:r isDirectory: &isDir])
	{
		if (! isDir) {
			NSString *RslvPath = ResolveAlias(r, &isDirectory);
			if (nil != RslvPath) {
				if (! isDirectory) {
					*childPath = RslvPath;
					v = true;
				}
			} else {
				*childPath = r;
				v = true;
			}
		}
	}

	return v;
}


#define NotAfileRef NULL

LOCALVAR FILE *Drives[NumDrives]; /* open disk image files */
#if IncludeSonyGetName || IncludeSonyNew
LOCALVAR NSString *DriveNames[NumDrives];
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
		DriveNames[i] = nil;
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

#if 0
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
		if (EWOULDBLOCK == errno) {
			/* already locked */
			MacMsg(kStrImageInUseTitle, kStrImageInUseMessage,
				false);
		} else
		{
			/*
				Failed for other reasons, such as unsupported
				for this volume.
				Don't prevent opening.
			*/
			IsOk = true;
		}
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
#if 0
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
		NSString *filePath = DriveNames[Drive_No];
		if (NULL != filePath) {
			if (deleteit) {
				NSAutoreleasePool *pool =
					[[NSAutoreleasePool alloc] init];
				const char *s = [filePath fileSystemRepresentation];
				remove(s);
				[pool release];
			}
			[filePath release];
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
	tMacErr v = mnvm_miscErr;
	NSString *filePath = DriveNames[Drive_No];
	if (NULL != filePath) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSString *s0 = [filePath lastPathComponent];
		v = NSStringToRomanPbuf(s0, r);

		[pool release];
	}

	return v;
}
#endif

LOCALFUNC bool Sony_Insert0(FILE *refnum, bool locked,
	NSString *filePath)
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
			DriveNames[Drive_No] = [filePath retain];
#endif

			IsOk = true;
		}
	}

	if (! IsOk) {
		fclose(refnum);
	}

	return IsOk;
}

LOCALFUNC bool Sony_Insert1(NSString *filePath, bool silentfail)
{
	/* const char *drivepath = [filePath UTF8String]; */
	const char *drivepath = [filePath fileSystemRepresentation];
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
		return Sony_Insert0(refnum, locked, filePath);
	}
	return false;
}

LOCALFUNC bool Sony_Insert2(char *s)
{
	NSString *sPath;

	if (! FindNamedChildFilePath(DataPath, s, &sPath)) {
		return false;
	} else {
		return Sony_Insert1(sPath, true);
	}
}

LOCALFUNC tMacErr LoadMacRomPath(NSString *RomPath)
{
	FILE *ROM_File;
	int File_Size;
	tMacErr err = mnvm_fnfErr;
	const char *path = [RomPath fileSystemRepresentation];

	ROM_File = fopen(path, "rb");
	if (NULL != ROM_File) {
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

LOCALFUNC bool Sony_Insert1a(NSString *filePath)
{
	bool v;

	if (! ROM_loaded) {
		v = (mnvm_noErr == LoadMacRomPath(filePath));
	} else {
		v = Sony_Insert1(filePath, false);
	}

	return v;
}

LOCALPROC Sony_ResolveInsert(NSString *filePath)
{
	Boolean isDirectory;
	NSString *RslvPath = ResolveAlias(filePath, &isDirectory);
	if (nil != RslvPath) {
		if (! isDirectory) {
			(void) Sony_Insert1a(RslvPath);
		}
	} else {
		(void) Sony_Insert1a(filePath);
	}
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
LOCALPROC MakeNewDisk0(uint32_t L, NSString *sPath)
{
	bool IsOk = false;
	const char *drivepath = [sPath fileSystemRepresentation];
	FILE *refnum = fopen(drivepath, "wb+");
	if (NULL == refnum) {
		MacMsg(kStrOpenFailTitle, kStrOpenFailMessage, false);
	} else {
		if (WriteZero(refnum, L)) {
			IsOk = Sony_Insert0(refnum, false, sPath);
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

/* --- ROM --- */

LOCALFUNC tMacErr LoadMacRomFrom(NSString *parentPath)
{
	NSString *RomPath;
	tMacErr err = mnvm_fnfErr;

	if (FindNamedChildFilePath(parentPath, RomFileName, &RomPath)) {
		err = LoadMacRomPath(RomPath);
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromAppDir(void)
{
	return LoadMacRomFrom(DataPath);
}

LOCALFUNC tMacErr LoadMacRomFromPrefDir(void)
{
	NSString *PrefsPath;
	NSString *GryphelPath;
	NSString *RomsPath;
	tMacErr err = mnvm_fnfErr;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(
		NSLibraryDirectory, NSUserDomainMask, YES);
	if ((nil != paths) && ([paths count] > 0))
	{
		NSString *LibPath = [paths objectAtIndex:0];
		if (FindNamedChildDirPath(LibPath, "Preferences", &PrefsPath))
		if (FindNamedChildDirPath(PrefsPath, "Gryphel", &GryphelPath))
		if (FindNamedChildDirPath(GryphelPath, "mnvm_rom", &RomsPath))
		{
			err = LoadMacRomFrom(RomsPath);
		}
	}

	return err;
}

LOCALFUNC tMacErr LoadMacRomFromGlobalDir(void)
{
	NSString *GryphelPath;
	NSString *RomsPath;
	tMacErr err = mnvm_fnfErr;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(
		NSApplicationSupportDirectory, NSLocalDomainMask, NO);
	if ((nil != paths) && ([paths count] > 0))
	{
		NSString *LibPath = [paths objectAtIndex:0];
		if (FindNamedChildDirPath(LibPath, "Gryphel", &GryphelPath))
		if (FindNamedChildDirPath(GryphelPath, "mnvm_rom", &RomsPath))
		{
			err = LoadMacRomFrom(RomsPath);
		}
	}

	return err;
}

LOCALFUNC bool LoadMacRom(void)
{
	tMacErr err;

	if (mnvm_fnfErr == (err = LoadMacRomFromAppDir()))
	if (mnvm_fnfErr == (err = LoadMacRomFromPrefDir()))
	if (mnvm_fnfErr == (err = LoadMacRomFromGlobalDir()))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}


#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEexport(tPbuf i)
{
	void *Buffer;
	uint32_t L;
	tMacErr err = mnvm_miscErr;

	PbufKillToPtr(&Buffer, &L, i);

	if (L > 0) {
		int j;
		uint8_t *p = (uint8_t *)Buffer;

		for (j = L; --j >= 0; ) {
			uint8_t v = *p;
			if (13 == v) {
				*p = 10;
			}
			++p;
		}
	}

	{
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSData *d = [[NSData alloc]
			initWithBytesNoCopy: Buffer length: L];
		/* NSData *d = [NSData dataWithBytes: Buffer length: L]; */
		NSString *ss = [[[NSString alloc]
			initWithData:d encoding:NSMacOSRomanStringEncoding]
			autorelease];
		NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
		NSArray *newTypes =
			[NSArray arrayWithObject: NSStringPboardType];

		(void) [pasteboard declareTypes: newTypes owner: nil];
		if ([pasteboard setString: ss forType: NSStringPboardType]) {
			err = mnvm_noErr;
		}

		[d release];

		[pool release];
	}

	return err;
}
#endif

#if IncludeHostTextClipExchange
GLOBALOSGLUFUNC tMacErr HTCEimport(tPbuf *r)
{
	tMacErr err = mnvm_miscErr;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
	NSArray *supportedTypes = [NSArray
		arrayWithObject: NSStringPboardType];
	NSString *available = [pasteboard
		availableTypeFromArray: supportedTypes];

	if (nil != available) {
		NSString *string = [pasteboard
			stringForType: NSStringPboardType];
		if (nil != string) {
			err = NSStringToRomanPbuf(string, r);
		}
	}

	[pool release];

	return err;
}
#endif


#if EmLocalTalk

#include "UTIL/BPFILTER.h"

#endif


#define UseCGContextDrawImage 0

LOCALVAR NSWindow *Window = nil;
LOCALVAR NSView *NSview = nil;
#if UseCGContextDrawImage
LOCALVAR NSGraphicsContext *NSgfxContext = nil;
LOCALVAR CGContextRef CGcontext = nil;
LOCALVAR void *Pixels = NULL;
LOCALVAR uint16_t Pitch;
LOCALVAR uint8_t BytesPerPixel;
#endif

LOCALVAR NSOpenGLContext *NSOpnGLCntxt = nil;
LOCALVAR short GLhOffset;
LOCALVAR short GLvOffset;
	/* OpenGL coordinates of upper left point of drawing area */


LOCALPROC HideCursor(void)
{
	[NSCursor hide];
}

LOCALPROC ShowCursor(void)
{
	if (nil != Window) {
		[Window invalidateCursorRectsForView:
			NSview];
	}
#if 0
	[cursor->nscursor performSelectorOnMainThread: @selector(set)
		withObject: nil waitUntilDone: NO];
#endif
#if 0
	[[NSCursor arrowCursor] set];
#endif
	[NSCursor unhide];
}

#if EnableMoveMouse
LOCALFUNC CGPoint QZ_PrivateSDLToCG(NSPoint *p)
{
	CGPoint cgp;

	*p = [NSview convertPoint: *p toView: nil];
	p->y = [NSview frame].size.height - p->y;
	*p = [Window convertBaseToScreen: *p];

	cgp.x = p->x;
	cgp.y = CGDisplayPixelsHigh(kCGDirectMainDisplay)
		- p->y;

	return cgp;
}
#endif

LOCALPROC QZ_GetMouseLocation(NSPoint *p)
{
	/* incorrect while window is being dragged */

	*p = [NSEvent mouseLocation]; /* global coordinates */
	if (nil != Window) {
		*p = [Window convertScreenToBase: *p];
	}
	*p = [NSview convertPoint: *p fromView: nil];
	p->y = [NSview frame].size.height - p->y;
}

/* --- keyboard --- */

LOCALVAR NSUInteger CurrentMods = 0;

/*
	Apple documentation says:
	"The lower 16 bits of the modifier flags are reserved
	for device-dependent bits."

	observed to be:
*/
#define _NSLShiftKeyMask   0x0002
#define _NSRShiftKeyMask   0x0004
#define _NSLControlKeyMask 0x0001
#define _NSRControlKeyMask 0x2000
#define _NSLCommandKeyMask 0x0008
#define _NSRCommandKeyMask 0x0010
#define _NSLOptionKeyMask  0x0020
#define _NSROptionKeyMask  0x0040
/*
	Avoid using the above unless it is
	really needed.
*/

LOCALPROC UpdateKeyboardModifiers(NSUInteger newMods)
{
	NSUInteger changeMask = CurrentMods ^ newMods;

	if (0 != changeMask) {
		if (0 != (changeMask & NSAlphaShiftKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_CapsLock,
				0 != (newMods & NSAlphaShiftKeyMask));
		}

#if MKC_formac_RShift == MKC_formac_Shift
		if (0 != (changeMask & NSShiftKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Shift,
				0 != (newMods & NSShiftKeyMask));
		}
#else
		if (0 != (changeMask & _NSLShiftKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Shift,
				0 != (newMods & _NSLShiftKeyMask));
		}
		if (0 != (changeMask & _NSRShiftKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_RShift,
				0 != (newMods & _NSRShiftKeyMask));
		}
#endif

#if MKC_formac_RControl == MKC_formac_Control
		if (0 != (changeMask & NSControlKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Control,
				0 != (newMods & NSControlKeyMask));
		}
#else
		if (0 != (changeMask & _NSLControlKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Control,
				0 != (newMods & _NSLControlKeyMask));
		}
		if (0 != (changeMask & _NSRControlKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_RControl,
				0 != (newMods & _NSRControlKeyMask));
		}
#endif

#if MKC_formac_RCommand == MKC_formac_Command
		if (0 != (changeMask & NSCommandKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Command,
				0 != (newMods & NSCommandKeyMask));
		}
#else
		if (0 != (changeMask & _NSLCommandKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Command,
				0 != (newMods & _NSLCommandKeyMask));
		}
		if (0 != (changeMask & _NSRCommandKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_RCommand,
				0 != (newMods & _NSRCommandKeyMask));
		}
#endif

#if MKC_formac_ROption == MKC_formac_Option
		if (0 != (changeMask & NSAlternateKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Option,
				0 != (newMods & NSAlternateKeyMask));
		}
#else
		if (0 != (changeMask & _NSLOptionKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_Option,
				0 != (newMods & _NSLOptionKeyMask));
		}
		if (0 != (changeMask & _NSROptionKeyMask)) {
			Keyboard_UpdateKeyMap2(MKC_formac_ROption,
				0 != (newMods & _NSROptionKeyMask));
		}
#endif

		CurrentMods = newMods;
	}
}

/* --- mouse --- */

/* cursor hiding */

LOCALVAR bool WantCursorHidden = false;

#if MayFullScreen
LOCALVAR short hOffset;
	/* number of pixels to left of drawing area in window */
LOCALVAR short vOffset;
	/* number of pixels above drawing area in window */
#endif

#if MayFullScreen
LOCALVAR bool GrabMachine = false;
#endif

#if VarFullScreen
LOCALVAR bool UseFullScreen = (0 != WantInitFullScreen);
#endif

#if EnableMagnify
LOCALVAR bool UseMagnify = (0 != WantInitMagnify);
#endif

LOCALVAR bool gBackgroundFlag = false;
LOCALVAR bool CurSpeedStopped = true;

#if EnableMagnify
#define MaxScale WindowScale
#else
#define MaxScale 1
#endif

LOCALVAR bool HaveCursorHidden = false;

LOCALPROC ForceShowCursor(void)
{
	if (HaveCursorHidden) {
		HaveCursorHidden = false;
		ShowCursor();
	}
}

/* cursor moving */

#if EnableMoveMouse
LOCALFUNC bool MoveMouse(int16_t h, int16_t v)
{
	NSPoint p;
	CGPoint cgp;

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

	p = NSMakePoint(h, v);
	cgp = QZ_PrivateSDLToCG(&p);

	/*
		this is the magic call that fixes cursor "freezing"
		after warp
	*/
	CGAssociateMouseAndMouseCursorPosition(0);
	CGWarpMouseCursorPosition(cgp);
	CGAssociateMouseAndMouseCursorPosition(1);

#if 0
	if (noErr != CGSetLocalEventsSuppressionInterval(0.0)) {
		/* don't use MacMsg which can call MoveMouse */
	}
	if (noErr != CGWarpMouseCursorPosition(cgp)) {
		/* don't use MacMsg which can call MoveMouse */
	}
#endif

	return true;
}
#endif

#if EnableFSMouseMotion
LOCALPROC AdjustMouseMotionGrab(void)
{
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

/* cursor state */

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
	/*
		incorrect while window is being dragged
		so only call when needed.
	*/
	NSPoint p;

	QZ_GetMouseLocation(&p);
	MousePositionNotify((int) p.x, (int) p.y);
}

LOCALVAR bool gTrueBackgroundFlag = false;


LOCALVAR uint8_t * ScalingBuff = nullpr;

LOCALVAR uint8_t * CLUT_final;

#define CLUT_finalsz1 (256 * 8)

#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)

#define CLUT_finalClrSz (256 << (5 - vMacScreenDepth))

#define CLUT_finalsz ((CLUT_finalClrSz > CLUT_finalsz1) \
	? CLUT_finalClrSz : CLUT_finalsz1)

#else
#define CLUT_finalsz CLUT_finalsz1
#endif


#define ScrnMapr_DoMap UpdateBWLuminanceCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth 0
#define ScrnMapr_DstDepth 3
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"


#if (0 != vMacScreenDepth) && (vMacScreenDepth < 4)

#define ScrnMapr_DoMap UpdateMappedColorCopy
#define ScrnMapr_Src GetCurDrawBuff()
#define ScrnMapr_Dst ScalingBuff
#define ScrnMapr_SrcDepth vMacScreenDepth
#define ScrnMapr_DstDepth 5
#define ScrnMapr_Map CLUT_final

#include "HW/SCREEN/SCRNMAPR.h"

#endif

#if vMacScreenDepth >= 4

#define ScrnTrns_DoTrans UpdateTransColorCopy
#define ScrnTrns_Src GetCurDrawBuff()
#define ScrnTrns_Dst ScalingBuff
#define ScrnTrns_SrcDepth vMacScreenDepth
#define ScrnTrns_DstDepth 5
#define ScrnTrns_DstZLo 1

#include "HW/SCREEN/SCRNTRNS.h"

#endif

LOCALPROC UpdateLuminanceCopy(int16_t top, int16_t left,
	int16_t bottom, int16_t right)
{
	int i;

#if 0 != vMacScreenDepth
	if (UseColorMode) {

#if vMacScreenDepth < 4

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

#else
		UpdateTransColorCopy(top, left, bottom, right);
#endif

	} else
#endif
	{
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

LOCALPROC DrawWithOpenGL(uint16_t top, uint16_t left, uint16_t bottom, uint16_t right)
{
	if (nil == NSOpnGLCntxt) {
		/* oops */
	} else {
		int16_t top2;
		int16_t left2;

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

		top2 = top;
		left2 = left;

#if VarFullScreen
		if (UseFullScreen)
#endif
#if MayFullScreen
		{
			left2 -= ViewHStart;
			top2 -= ViewVStart;
		}
#endif

#if EnableMagnify
		if (UseMagnify) {
			top2 *= WindowScale;
			left2 *= WindowScale;
		}
#endif

		[NSOpnGLCntxt makeCurrentContext];

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

		glFlush();
	}

#if MayFullScreen
label_exit:
	;
#endif
}

#if UseCGContextDrawImage
LOCALPROC SDL_UpdateRect(int32_t x, int32_t y, uint32_t w, uint32_t h)
{
	if ([Window isMiniaturized]) {

		/* Do nothing if miniaturized */

	} else {
		NSGraphicsContext *ctx = [NSGraphicsContext currentContext];
		if (ctx != NSgfxContext) {
			/* uhoh, you might be rendering from another thread... */
			[NSGraphicsContext
				setCurrentContext: NSgfxContext];
			ctx = NSgfxContext;
		}
		CGContextRef cgc = (CGContextRef) [ctx graphicsPort];
		CGContextFlush(CGcontext);
		CGImageRef image = CGBitmapContextCreateImage(
			CGcontext);
		CGRect rectangle = CGRectMake(0, 0,
			[NSview frame].size.width,
			[NSview frame].size.height);

		CGContextDrawImage(cgc, rectangle, image);
		CGImageRelease(image);
		CGContextFlush(cgc);
	}
}
#endif

/* --- time, date, location --- */

#define dbglog_TimeStuff (0 && dbglog_HAVE)

LOCALVAR uint32_t TrueEmulatedTime = 0;

LOCALVAR NSTimeInterval LatestTime;
LOCALVAR NSTimeInterval NextTickChangeTime;

#define TickDuration (1.0 / 60.14742)

LOCALVAR uint32_t NewMacDateInSeconds;

LOCALVAR bool EmulationWasInterrupted = false;

LOCALPROC UpdateTrueEmulatedTime(void)
{
	NSTimeInterval TimeDiff;

	LatestTime = [NSDate timeIntervalSinceReferenceDate];
	TimeDiff = LatestTime - NextTickChangeTime;

	if (TimeDiff >= 0.0) {
		if (TimeDiff > 16 * TickDuration) {
			/* emulation interrupted, forget it */
			++TrueEmulatedTime;
			NextTickChangeTime = LatestTime + TickDuration;

			EmulationWasInterrupted = true;
#if dbglog_TimeStuff
			dbglog_writelnNum("emulation interrupted",
				TrueEmulatedTime);
#endif
		} else {
			do {
#if 0 && dbglog_TimeStuff
				dbglog_writeln("got next tick");
#endif
				++TrueEmulatedTime;
				TimeDiff -= TickDuration;
				NextTickChangeTime += TickDuration;
			} while (TimeDiff >= 0.0);
		}
	} else if (TimeDiff < (-16 * TickDuration)) {
		/* clock set back, reset */
#if dbglog_TimeStuff
		dbglog_writeln("clock set back");
#endif

		NextTickChangeTime = LatestTime + TickDuration;
	}
}


LOCALVAR uint32_t DateDelta;

LOCALFUNC bool CheckDateTime(void)
{
	NewMacDateInSeconds = ((uint32_t)LatestTime) + DateDelta;
	if (CurMacDateInSeconds != NewMacDateInSeconds) {
		CurMacDateInSeconds = NewMacDateInSeconds;
		return true;
	} else {
		return false;
	}
}

LOCALPROC StartUpTimeAdjust(void)
{
	LatestTime = [NSDate timeIntervalSinceReferenceDate];
	NextTickChangeTime = LatestTime;
}

LOCALFUNC bool InitLocationDat(void)
{
	NSTimeZone *Zone = [NSTimeZone localTimeZone];
	uint32_t TzOffSet = (uint32_t)[Zone secondsFromGMT];
#if AutoTimeZone
	BOOL isdst = [Zone isDaylightSavingTime];
#endif

	DateDelta = TzOffSet - 1233815296;
	LatestTime = [NSDate timeIntervalSinceReferenceDate];
	NewMacDateInSeconds = ((uint32_t)LatestTime) + DateDelta;
	CurMacDateInSeconds = NewMacDateInSeconds;
#if AutoTimeZone
	CurMacDelta = (TzOffSet & 0x00FFFFFF)
		| ((isdst ? 0x80 : 0) << 24);
#endif

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
static volatile uint16_t ThePlayOffset;
static volatile uint16_t TheFillOffset;
static volatile uint16_t MinFilledSoundBuffs;
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

typedef uint16_t trSoundTemp;

#define kCenterTempSound 0x8000

#define AudioStepVal 0x0040

#if 3 == kLn2SoundSampSz
#define ConvertTempSoundSampleFromNative(v) ((v) << 8)
#elif 4 == kLn2SoundSampSz
#define ConvertTempSoundSampleFromNative(v) ((v) + kCenterSound)
#else
#error "unsupported kLn2SoundSampSz"
#endif

#if 3 == kLn2SoundSampSz
#define ConvertTempSoundSampleToNative(v) ((v) >> 8)
#elif 4 == kLn2SoundSampSz
#define ConvertTempSoundSampleToNative(v) ((v) - kCenterSound)
#else
#error "unsupported kLn2SoundSampSz"
#endif

LOCALPROC SoundRampTo(trSoundTemp *last_val, trSoundTemp dst_val,
	tpSoundSamp *stream, int *len)
{
	trSoundTemp diff;
	tpSoundSamp p = *stream;
	int n = *len;
	trSoundTemp v1 = *last_val;

	while ((v1 != dst_val) && (0 != n)) {
		if (v1 > dst_val) {
			diff = v1 - dst_val;
			if (diff > AudioStepVal) {
				v1 -= AudioStepVal;
			} else {
				v1 = dst_val;
			}
		} else {
			diff = dst_val - v1;
			if (diff > AudioStepVal) {
				v1 += AudioStepVal;
			} else {
				v1 = dst_val;
			}
		}

		--n;
		*p++ = ConvertTempSoundSampleToNative(v1);
	}

	*stream = p;
	*len = n;
	*last_val = v1;
}

struct SoundR {
	tpSoundSamp fTheSoundBuffer;
	volatile uint16_t (*fPlayOffset);
	volatile uint16_t (*fFillOffset);
	volatile uint16_t (*fMinFilledSoundBuffs);

	volatile trSoundTemp lastv;

	bool enabled;
	bool wantplaying;
	bool HaveStartedPlaying;

	AudioUnit outputAudioUnit;
};
typedef struct SoundR SoundR;

LOCALPROC audio_callback(void *udata, void *stream, int len)
{
	uint16_t ToPlayLen;
	uint16_t FilledSoundBuffs;
	int i;
	SoundR *datp = (SoundR *)udata;
	tpSoundSamp CurSoundBuffer = datp->fTheSoundBuffer;
	uint16_t CurPlayOffset = *datp->fPlayOffset;
	trSoundTemp v0 = datp->lastv;
	trSoundTemp v1 = v0;
	tpSoundSamp dst = (tpSoundSamp)stream;

#if kLn2SoundSampSz > 3
	len >>= (kLn2SoundSampSz - 3);
#endif

#if dbglog_SoundStuff
	dbglog_writeln("Enter audio_callback");
	dbglog_writelnNum("len", len);
#endif

label_retry:
	ToPlayLen = *datp->fFillOffset - CurPlayOffset;
	FilledSoundBuffs = ToPlayLen >> kLnOneBuffLen;

	if (! datp->wantplaying) {
#if dbglog_SoundStuff
		dbglog_writeln("playing end transistion");
#endif

		SoundRampTo(&v1, kCenterTempSound, &dst, &len);

		ToPlayLen = 0;
	} else if (! datp->HaveStartedPlaying) {
#if dbglog_SoundStuff
		dbglog_writeln("playing start block");
#endif

		if ((ToPlayLen >> kLnOneBuffLen) < 8) {
			ToPlayLen = 0;
		} else {
			tpSoundSamp p = datp->fTheSoundBuffer
				+ (CurPlayOffset & kAllBuffMask);
			trSoundTemp v2 = ConvertTempSoundSampleFromNative(*p);

#if dbglog_SoundStuff
			dbglog_writeln("have enough samples to start");
#endif

			SoundRampTo(&v1, v2, &dst, &len);

			if (v1 == v2) {
#if dbglog_SoundStuff
				dbglog_writeln("finished start transition");
#endif

				datp->HaveStartedPlaying = true;
			}
		}
	}

	if (0 == len) {
		/* done */

		if (FilledSoundBuffs < *datp->fMinFilledSoundBuffs) {
			*datp->fMinFilledSoundBuffs = FilledSoundBuffs;
		}
	} else if (0 == ToPlayLen) {

#if dbglog_SoundStuff
		dbglog_writeln("under run");
#endif

		for (i = 0; i < len; ++i) {
			*dst++ = ConvertTempSoundSampleToNative(v1);
		}
		*datp->fMinFilledSoundBuffs = 0;
	} else {
		uint16_t PlayBuffContig = kAllBuffLen
			- (CurPlayOffset & kAllBuffMask);
		tpSoundSamp p = CurSoundBuffer
			+ (CurPlayOffset & kAllBuffMask);

		if (ToPlayLen > PlayBuffContig) {
			ToPlayLen = PlayBuffContig;
		}
		if (ToPlayLen > len) {
			ToPlayLen = len;
		}

		for (i = 0; i < ToPlayLen; ++i) {
			*dst++ = *p++;
		}
		v1 = ConvertTempSoundSampleFromNative(p[-1]);

		CurPlayOffset += ToPlayLen;
		len -= ToPlayLen;

		*datp->fPlayOffset = CurPlayOffset;

		goto label_retry;
	}

	datp->lastv = v1;
}

LOCALFUNC OSStatus audioCallback(
	void                       *inRefCon,
	AudioUnitRenderActionFlags *ioActionFlags,
	const AudioTimeStamp       *inTimeStamp,
	UInt32                     inBusNumber,
	UInt32                     inNumberFrames,
	AudioBufferList            *ioData)
{
	AudioBuffer *abuf;
	UInt32 i;
	UInt32 n = ioData->mNumberBuffers;

#if dbglog_SoundStuff
	dbglog_writeln("Enter audioCallback");
	dbglog_writelnNum("mNumberBuffers", n);
#endif

	for (i = 0; i < n; i++) {
		abuf = &ioData->mBuffers[i];
		audio_callback(inRefCon,
			abuf->mData, abuf->mDataByteSize);
	}

	return 0;
}

LOCALVAR SoundR cur_audio;

LOCALPROC ZapAudioVars(void)
{
	memset(&cur_audio, 0, sizeof(SoundR));
}

LOCALPROC Sound_Stop(void)
{
#if dbglog_SoundStuff
	dbglog_writeln("enter Sound_Stop");
#endif

	if (cur_audio.wantplaying) {
		OSStatus result;
		uint16_t retry_limit = 50; /* half of a second */

		cur_audio.wantplaying = false;

label_retry:
		if (kCenterTempSound == cur_audio.lastv) {
#if dbglog_SoundStuff
			dbglog_writeln("reached kCenterTempSound");
#endif

			/* done */
		} else if (0 == --retry_limit) {
#if dbglog_SoundStuff
			dbglog_writeln("retry limit reached");
#endif
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

		if (noErr != (result = AudioOutputUnitStop(
			cur_audio.outputAudioUnit)))
		{
#if dbglog_HAVE
			dbglog_writeln("AudioOutputUnitStop fails");
#endif
		}
	}

#if dbglog_SoundStuff
	dbglog_writeln("leave Sound_Stop");
#endif
}

LOCALPROC Sound_Start(void)
{
	OSStatus result;

	if ((! cur_audio.wantplaying) && cur_audio.enabled) {
#if dbglog_SoundStuff
		dbglog_writeln("enter Sound_Start");
#endif

		Sound_Start0();
		cur_audio.lastv = kCenterTempSound;
		cur_audio.HaveStartedPlaying = false;
		cur_audio.wantplaying = true;

		if (noErr != (result = AudioOutputUnitStart(
			cur_audio.outputAudioUnit)))
		{
#if dbglog_HAVE
			dbglog_writeln("AudioOutputUnitStart fails");
#endif
			cur_audio.wantplaying = false;
		}

#if dbglog_SoundStuff
		dbglog_writeln("leave Sound_Start");
#endif
	}
}

LOCALPROC Sound_UnInit(void)
{
	if (cur_audio.enabled) {
		OSStatus result;
		struct AURenderCallbackStruct callback;

		cur_audio.enabled = false;

		/* Remove the input callback */
		callback.inputProc = 0;
		callback.inputProcRefCon = 0;

		if (noErr != (result = AudioUnitSetProperty(
			cur_audio.outputAudioUnit,
			kAudioUnitProperty_SetRenderCallback,
			kAudioUnitScope_Input,
			0,
			&callback,
			sizeof(callback))))
		{
#if dbglog_HAVE
			dbglog_writeln("AudioUnitSetProperty fails"
				"(kAudioUnitProperty_SetRenderCallback)");
#endif
		}

		if (noErr != (result = CloseComponent(
			cur_audio.outputAudioUnit)))
		{
#if dbglog_HAVE
			dbglog_writeln("CloseComponent fails in Sound_UnInit");
#endif
		}
	}
}

#define SOUND_SAMPLERATE 22255 /* = round(7833600 * 2 / 704) */

LOCALFUNC bool Sound_Init(void)
{
	OSStatus result = noErr;
	Component comp;
	ComponentDescription desc;
	struct AURenderCallbackStruct callback;
	AudioStreamBasicDescription requestedDesc;

	cur_audio.fTheSoundBuffer = TheSoundBuffer;
	cur_audio.fPlayOffset = &ThePlayOffset;
	cur_audio.fFillOffset = &TheFillOffset;
	cur_audio.fMinFilledSoundBuffs = &MinFilledSoundBuffs;
	cur_audio.wantplaying = false;

	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_DefaultOutput;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;
	desc.componentFlags = 0;
	desc.componentFlagsMask = 0;


	requestedDesc.mFormatID = kAudioFormatLinearPCM;
	requestedDesc.mFormatFlags = kLinearPCMFormatFlagIsPacked
#if 3 != kLn2SoundSampSz
		| kLinearPCMFormatFlagIsSignedInteger
#endif
		;
	requestedDesc.mChannelsPerFrame = 1;
	requestedDesc.mSampleRate = SOUND_SAMPLERATE;

	requestedDesc.mBitsPerChannel = (1 << kLn2SoundSampSz);
#if 0
	requestedDesc.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
#endif
#if 0
	requestedDesc.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
#endif

	requestedDesc.mFramesPerPacket = 1;
	requestedDesc.mBytesPerFrame = (requestedDesc.mBitsPerChannel
		* requestedDesc.mChannelsPerFrame) >> 3;
	requestedDesc.mBytesPerPacket = requestedDesc.mBytesPerFrame
		* requestedDesc.mFramesPerPacket;


	callback.inputProc = audioCallback;
	callback.inputProcRefCon = &cur_audio;

	if (NULL == (comp = FindNextComponent(NULL, &desc)))
	{
#if dbglog_HAVE
		dbglog_writeln("Failed to start CoreAudio: "
			"FindNextComponent returned NULL");
#endif
	} else

	if (noErr != (result = OpenAComponent(
		comp, &cur_audio.outputAudioUnit)))
	{
#if dbglog_HAVE
		dbglog_writeln("Failed to start CoreAudio: OpenAComponent");
#endif
	} else

	if (noErr != (result = AudioUnitInitialize(
		cur_audio.outputAudioUnit)))
	{
#if dbglog_HAVE
		dbglog_writeln(
			"Failed to start CoreAudio: AudioUnitInitialize");
#endif
	} else

	if (noErr != (result = AudioUnitSetProperty(
		cur_audio.outputAudioUnit,
		kAudioUnitProperty_StreamFormat,
		kAudioUnitScope_Input,
		0,
		&requestedDesc,
		sizeof(requestedDesc))))
	{
#if dbglog_HAVE
		dbglog_writeln("Failed to start CoreAudio: "
			"AudioUnitSetProperty(kAudioUnitProperty_StreamFormat)");
#endif
	} else

	if (noErr != (result = AudioUnitSetProperty(
		cur_audio.outputAudioUnit,
		kAudioUnitProperty_SetRenderCallback,
		kAudioUnitScope_Input,
		0,
		&callback,
		sizeof(callback))))
	{
#if dbglog_HAVE
		dbglog_writeln("Failed to start CoreAudio: "
			"AudioUnitSetProperty(kAudioUnitProperty_SetInputCallback)"
			);
#endif
	} else

	{
		cur_audio.enabled = true;

		Sound_Start();
			/*
				This should be taken care of by LeaveSpeedStopped,
				but since takes a while to get going properly,
				start early.
			*/
	}

	return true; /* keep going, even if no sound */
}

GLOBALOSGLUPROC Sound_EndWrite(uint16_t actL)
{
	if (Sound_EndWrite0(actL)) {
	}
}

LOCALPROC Sound_SecondNotify(void)
{
	if (cur_audio.enabled) {
		Sound_SecondNotify0();
	}
}

#endif

LOCALPROC FinishSubMenu(NSMenu *theMenu, NSMenu *parentMenu,
	NSString *sTitle)
{
	NSMenuItem *menuItem = [[NSMenuItem alloc]
		initWithTitle: sTitle
		action: nil
		keyEquivalent: @""];

	[menuItem setSubmenu: theMenu];
	[parentMenu addItem: menuItem];
	[menuItem release];
}

LOCALFUNC NSMenu *setApplicationMenu(NSMenu *mainMenu)
{
	NSMenuItem *menuItem;
	NSString *sAppName = NSStringCreateFromSubstCStr("^p");
		/* doesn't matter though, OS X replaces this */
	NSString *sAbout =
		NSStringCreateFromSubstCStr(kStrMenuItemAbout);
	NSString *sHide =
		NSStringCreateFromSubstCStr(kStrAppMenuItemHide);
	NSString *sHideOthers =
		NSStringCreateFromSubstCStr(kStrAppMenuItemHideOthers);
	NSString *sShowAll =
		NSStringCreateFromSubstCStr(kStrAppMenuItemShowAll);
	NSString *sQuit =
		NSStringCreateFromSubstCStr(kStrAppMenuItemQuit);

	NSMenu *appleMenu = [[NSMenu alloc] initWithTitle: sAppName];

	/* Add menu items */
	menuItem = [appleMenu addItemWithTitle: sAbout
		action: @selector(performApplicationAbout:)
		keyEquivalent: @"a"];
	[menuItem setKeyEquivalentModifierMask: NSControlKeyMask];

	[appleMenu addItem:[NSMenuItem separatorItem]];

	[appleMenu addItemWithTitle: sHide
		action: @selector(hide:) keyEquivalent: @""];

	[appleMenu
		addItemWithTitle: sHideOthers
		action: @selector(hideOtherApplications:)
		keyEquivalent: @""];

	[appleMenu addItemWithTitle: sShowAll
		action: @selector(unhideAllApplications:)
		keyEquivalent: @""];

	[appleMenu addItem: [NSMenuItem separatorItem]];

	menuItem = [appleMenu addItemWithTitle: sQuit
		action: @selector(terminate:) keyEquivalent: @"q"];
	[menuItem setKeyEquivalentModifierMask: NSControlKeyMask];

	FinishSubMenu(appleMenu, mainMenu, sAppName);

	[appleMenu release];

	return appleMenu;
}

/* Create File menu */
LOCALPROC setupFileMenu(NSMenu *mainMenu)
{
	NSMenu *fileMenu;
	NSMenuItem *menuItem;
	NSString *sFile =
		NSStringCreateFromSubstCStr(kStrMenuFile);
	NSString *sOpen =
		NSStringCreateFromSubstCStr(kStrMenuItemOpen ";ll");

	fileMenu = [[NSMenu alloc] initWithTitle: sFile];

	menuItem = [fileMenu
		addItemWithTitle: sOpen
		action: @selector(performFileOpen:)
		keyEquivalent: @"o"];
	[menuItem setKeyEquivalentModifierMask: NSControlKeyMask];

	FinishSubMenu(fileMenu, mainMenu, sFile);

	[fileMenu release];
}

/* Create Special menu */
LOCALPROC setupSpecialMenu(NSMenu *mainMenu)
{
	NSMenu *specialMenu;
	NSString *sSpecial =
		NSStringCreateFromSubstCStr(kStrMenuSpecial);
	NSString *sMore =
		NSStringCreateFromSubstCStr(kStrMenuItemMore ";ll");

	specialMenu = [[NSMenu alloc] initWithTitle: sSpecial];

	[specialMenu
		addItemWithTitle: sMore
		action: @selector(performSpecialMoreCommands:)
		keyEquivalent: @""];

	FinishSubMenu(specialMenu, mainMenu, sSpecial);

	[specialMenu release];
}

LOCALPROC MenuSetup(void)
{
	NSMenu *mainMenu = [[NSMenu alloc] init];
	NSMenu *appleMenu = setApplicationMenu(mainMenu);

	setupFileMenu(mainMenu);
	setupSpecialMenu(mainMenu);

	[NSApp setMainMenu: mainMenu];

	/*
		Tell the application object that this is now
		the application menu, if this unsupported
		call actually exists. Doesn't seem to
		be needed anyway, at least in OS X 10.7
	*/
	if([NSApp respondsToSelector:@selector(setAppleMenu:)]) {
		/* [NSApp setAppleMenu: appleMenu]; */
		[NSApp performSelector: @selector(setAppleMenu:)
			withObject:appleMenu];
	}

	[mainMenu release];
}



/* --- video out --- */


#if ! UseCGContextDrawImage
LOCALPROC HaveChangedScreenBuff(uint16_t top, uint16_t left,
	uint16_t bottom, uint16_t right)
{
	if ([NSview lockFocusIfCanDraw]) {
		DrawWithOpenGL(top, left, bottom, right);
		[NSview unlockFocus];
	}
}
#else
LOCALPROC HaveChangedScreenBuff(uint16_t top, uint16_t left,
	uint16_t bottom, uint16_t right)
{
	int i;
	int j;
	uint8_t *the_data = (uint8_t *)GetCurDrawBuff();
	uint8_t *p;
	uint32_t color;
	uint32_t black_color = 0;
		/* SDL_MapRGB(cur_video.format, 0, 0, 0) */
	uint32_t white_color = 0;
		/* SDL_MapRGB(cur_video.format, 255, 255, 255) */

	switch (BytesPerPixel) {
		case 2: /* (1)-5-5-5 RGB */
#if 0
			rmask = 0x7C00;
			gmask = 0x03E0;
			bmask = 0x001F;
#endif
			break;
		case 4:
#if LittleEndianUnaligned
#if 0
			rmask = 0x0000FF00;
			gmask = 0x00FF0000;
			bmask = 0xFF000000;
#endif
			black_color = 0x000000FF;
			white_color = 0xFFFFFFFF;
#else
#if 0
			rmask = 0x00FF0000;
			gmask = 0x0000FF00;
			bmask = 0x000000FF;
#endif
			black_color = 0xFF000000;
			white_color = 0xFFFFFFFF;
#endif
			break;
	}

#if EnableMagnify
	if (UseMagnify) {
		for (i = top * WindowScale; i < bottom * WindowScale; ++i) {
			for (j = left * WindowScale;
				j < right * WindowScale; ++j)
			{
				p = the_data + (((i / WindowScale) * vMacScreenWidth
					+ (j / WindowScale)) / 8);
				if (0 != (*p & (1 << ((~ (j / WindowScale)) & 0x7))))
				{
					color = black_color;
				} else {
					color = white_color;
				}
				switch (BytesPerPixel) {
					case 2: { /* Probably 15-bpp or 16-bpp */
						uint16_t *bufp;

						bufp = (uint16_t *)Pixels
							+ i * Pitch / 2 + j;
						*bufp = color;
					}
					break;

					case 4: { /* Probably 32-bpp */
						uint32_t *bufp;

						bufp = (uint32_t *)Pixels
							+ i * Pitch / 4 + j;
						*bufp = color;
					}
					break;
				}
			}
		}
	} else
#endif
	{
		for (i = top; i < bottom; ++i) {
			for (j = left; j < right; ++j) {
				p = the_data + ((i * vMacScreenWidth + j) / 8);
				if (0 != (*p & (1 << ((~ j) & 0x7)))) {
					color = black_color;
				} else {
					color = white_color;
				}
				switch (BytesPerPixel) {
					case 2: { /* Probably 15-bpp or 16-bpp */
						uint16_t *bufp;

						bufp = (uint16_t *)Pixels
							+ i * Pitch / 2 + j;
						*bufp = color;
					}
					break;
					case 4: { /* Probably 32-bpp */
						uint32_t *bufp;

						bufp = (uint32_t *)Pixels
							+ i * Pitch / 4 + j;
						*bufp = color;
					}
					break;
				}
			}
		}
	}

#if EnableMagnify
	if (UseMagnify) {
		SDL_UpdateRect(left * WindowScale,
			top * WindowScale,
			(right - left) * WindowScale,
			(bottom - top) * WindowScale);
	} else
#endif
	{
		SDL_UpdateRect(left, top,
			right - left, bottom - top);
	}
}
#endif

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

/* --- keyboard input --- */

LOCALPROC DisableKeyRepeat(void)
{
}

LOCALPROC RestoreKeyRepeat(void)
{
}

LOCALPROC ReconnectKeyCodes3(void)
{
}

LOCALPROC DisconnectKeyCodes3(void)
{
	DisconnectKeyCodes2();
	MouseButtonSet(false);
}

/* --- basic dialogs --- */

LOCALPROC CheckSavedMacMsg(void)
{
	/* called only on quit, if error saved but not yet reported */

	if (nullpr != SavedBriefMsg) {
		NSString *briefMsg0 =
			NSStringCreateFromSubstCStr(SavedBriefMsg);
		NSString *longMsg0 =
			NSStringCreateFromSubstCStr(SavedLongMsg);
		NSString *quitMsg0 =
			NSStringCreateFromSubstCStr(kStrCmdQuit);

		(void) NSRunAlertPanel(briefMsg0, @"%@", quitMsg0, nil, nil,
			longMsg0);

		SavedBriefMsg = nullpr;
	}
}

/* --- hide/show menubar --- */

enum {
	NSApplicationPresentationDefault                    = 0,
	NSApplicationPresentationAutoHideDock               = (1 <<  0),
	NSApplicationPresentationHideDock                   = (1 <<  1),
	NSApplicationPresentationAutoHideMenuBar            = (1 <<  2),
	NSApplicationPresentationHideMenuBar                = (1 <<  3),
	NSApplicationPresentationDisableAppleMenu           = (1 <<  4),
	NSApplicationPresentationDisableProcessSwitching    = (1 <<  5),
	NSApplicationPresentationDisableForceQuit           = (1 <<  6),
	NSApplicationPresentationDisableSessionTermination  = (1 <<  7),
	NSApplicationPresentationDisableHideApplication     = (1 <<  8),
	NSApplicationPresentationDisableMenuBarTransparency = (1 <<  9),
	NSApplicationPresentationFullScreen                 = (1 << 10),
	NSApplicationPresentationAutoHideToolbar            = (1 << 11)
};
typedef NSUInteger NSApplicationPresentationOptions;

@interface NSApplication : NSObject
- (void)setPresentationOptions:
	(NSApplicationPresentationOptions)newOptions;
@end


#if MayFullScreen
LOCALPROC _HideMenuBar(void)
{
	if ([NSApp respondsToSelector:@selector(setPresentationOptions:)]) {
		[((NSApplication *)NSApp) setPresentationOptions:
			NSApplicationPresentationHideDock
			| NSApplicationPresentationHideMenuBar
#if GrabKeysFullScreen
			| NSApplicationPresentationDisableProcessSwitching
#if GrabKeysMaxFullScreen /* dangerous !! */
			| NSApplicationPresentationDisableForceQuit
			| NSApplicationPresentationDisableSessionTermination
#endif
#endif
			];
	} else
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
	} else
	{
	}
}
#endif

#if MayFullScreen
LOCALPROC _ShowMenuBar(void)
{
	if ([NSApp respondsToSelector:@selector(setPresentationOptions:)]) {
		[((NSApplication *)NSApp) setPresentationOptions:
			NSApplicationPresentationDefault];
	} else
	if (HaveSetSystemUIMode()) {
		(void) SetSystemUIMode(kUIModeNormal,
			0);
	} else
	{
	}
}
#endif

/* --- event handling for main window --- */

LOCALPROC BeginDialog(void)
{
	DisconnectKeyCodes3();
	ForceShowCursor();
}

LOCALPROC EndDialog(void)
{
	[Window makeKeyWindow];
	EmulationWasInterrupted = true;
}

LOCALPROC InsertADisk0(void)
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];

	[panel setAllowsMultipleSelection: YES];

	BeginDialog();

	if (NSOKButton == [panel runModal]) {
		int i;
		NSArray *a = [panel URLs];
		int n = [a count];

		for (i = 0; i < n; ++i) {
			NSURL *fileURL = [a objectAtIndex: i];
			NSString* filePath = [fileURL path];
			(void) Sony_Insert1a(filePath);
		}
	}

	EndDialog();
}

/* --- main window creation and disposal --- */

LOCALFUNC bool Screen_Init(void)
{
#if 0
	if (noErr != Gestalt(gestaltSystemVersion,
		&cur_video.system_version))
	{
		cur_video.system_version = 0;
	}
#endif

#if 0
#define CGMainDisplayID CGMainDisplayID
	CGDirectDisplayID CurMainDisplayID = CGMainDisplayID();

	cur_video.width = (uint32_t) CGDisplayPixelsWide(CurMainDisplayID);
	cur_video.height = (uint32_t) CGDisplayPixelsHigh(CurMainDisplayID);
#endif

	InitKeyCodes();

	return true;
}

#if MayFullScreen
LOCALPROC AdjustMachineGrab(void)
{
#if EnableFSMouseMotion
	AdjustMouseMotionGrab();
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

LOCALPROC AdjustGLforSize(int h, int v)
{
	[NSOpnGLCntxt makeCurrentContext];

	glClearColor (0.0, 0.0, 0.0, 1.0);

#if 1
	glViewport(0, 0, h, v);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, h, 0, v, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
#endif

	glColor3f(0.0, 0.0, 0.0);
#if EnableMagnify
	if (UseMagnify) {
		glPixelZoom(WindowScale, - WindowScale);
	} else
#endif
	{
		glPixelZoom(1, -1);
	}
	glPixelStorei(GL_UNPACK_ROW_LENGTH, vMacScreenWidth);

	glClear(GL_COLOR_BUFFER_BIT);

	[NSOpenGLContext clearCurrentContext];

	ScreenChangedAll();
}

LOCALVAR bool WantScreensChangedCheck = false;

LOCALPROC UpdateOpenGLContext(void)
{
	if (nil != NSOpnGLCntxt) {
		[NSOpnGLCntxt makeCurrentContext];
		[NSOpnGLCntxt update];
	}
}

LOCALPROC CloseOpenGLContext(void)
{
	if (nil != NSOpnGLCntxt) {

		[NSOpenGLContext clearCurrentContext];
		/*
			Only because DrawWithOpenGL doesn't
			bother to do this. No one
			uses the current context
			without settting it first.
		*/
	}
}

LOCALFUNC bool GetOpnGLCntxt(void)
{
	bool v = false;

	if (nil == NSOpnGLCntxt) {
		NSRect NewWinRect = [NSview frame];
		NSOpenGLPixelFormat *fmt;

#if WantGraphicsSwitching
		{
			NSOpenGLPixelFormatAttribute attr0[] = {
				NSOpenGLPFAAllowOfflineRenderers,
				0};

			fmt =
				[[NSOpenGLPixelFormat alloc] initWithAttributes:attr0];
		}
		if (nil != fmt) {
			/* ok */
		} else
#endif
		{
			NSOpenGLPixelFormatAttribute attr[] = {
				0};

			fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
			if (nil == fmt) {
#if dbglog_HAVE
				dbglog_writeln("Could not create fmt");
#endif
				goto label_exit;
			}
		}

		NSOpnGLCntxt = [[NSOpenGLContext alloc]
			initWithFormat:fmt shareContext:nil];

		[fmt release];

		if (nil == NSOpnGLCntxt) {
#if dbglog_HAVE
			dbglog_writeln("Could not create NSOpnGLCntxt");
#endif
			goto label_exit;
		}

		/* fprintf(stderr, "%s\n", "Got OpenGL context"); */

		[NSOpnGLCntxt setView: NSview];
		[NSOpnGLCntxt update];

		AdjustGLforSize(NewWinRect.size.width,
			NewWinRect.size.height);

#if 0 != vMacScreenDepth
		ColorModeWorks = true;
#endif
	}
	v = true;

label_exit:
	return v;
}

typedef NSUInteger (*modifierFlagsProcPtr)
	(id self, SEL cmd);

/* Subclass of NSWindow to fix genie effect and support resize events */
@interface ClassWindow : NSWindow
@end

@implementation ClassWindow

#if MayFullScreen
- (BOOL)canBecomeKeyWindow
{
	return
#if VarFullScreen
		(! UseFullScreen) ? [super canBecomeKeyWindow] :
#endif
		YES;
}
#endif

#if MayFullScreen
- (BOOL)canBecomeMainWindow
{
	return
#if VarFullScreen
		(! UseFullScreen) ? [super canBecomeMainWindow] :
#endif
		YES;
}
#endif

#if MayFullScreen
- (NSRect)constrainFrameRect:(NSRect)frameRect
	toScreen:(NSScreen *)screen
{
#if VarFullScreen
	if (! UseFullScreen) {
		return [super constrainFrameRect:frameRect toScreen:screen];
	} else
#endif
	{
		return frameRect;
	}
}
#endif

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
	/* NSPasteboard *pboard = [sender draggingPasteboard]; */
	NSDragOperation sourceDragMask =
		[sender draggingSourceOperationMask];
	NSDragOperation v = NSDragOperationNone;

	if (0 != (sourceDragMask & NSDragOperationGeneric)) {
		return NSDragOperationGeneric;
	}

	return v;
}

- (void)draggingExited:(id <NSDraggingInfo>)sender
{
	/* remove hilighting */
}

- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender
{
	return YES;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
	BOOL v = NO;
	NSPasteboard *pboard = [sender draggingPasteboard];
	/*
		NSDragOperation sourceDragMask =
			[sender draggingSourceOperationMask];
	*/

	if ([[pboard types] containsObject:NSFilenamesPboardType]) {
		int i;
		NSArray *file_names =
			[pboard propertyListForType: NSFilenamesPboardType];
		int n = [file_names count];

		for (i = 0; i < n; ++i) {
			NSString *filePath = [file_names objectAtIndex:i];
			Sony_ResolveInsert(filePath);
		}
		v = YES;
	} else if ([[pboard types] containsObject: NSURLPboardType]) {
		NSURL *fileURL = [NSURL URLFromPasteboard: pboard];
		NSString* filePath = [fileURL path];
		Sony_ResolveInsert(filePath);
		v = YES;
	}

	if (v && gTrueBackgroundFlag) {
		{
			SEL sel = @selector(modifierFlags);

			if ([NSEvent respondsToSelector:sel]) {
				modifierFlagsProcPtr imp = (modifierFlagsProcPtr)
					[NSEvent methodForSelector:sel];

				UpdateKeyboardModifiers(imp([NSEvent class], sel));
			}
		}

		[NSApp activateIgnoringOtherApps: YES];
	}

	return v;
}

- (void) concludeDragOperation:(id <NSDraggingInfo>)the_sender
{
}

@end

@interface ClassWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation ClassWindowDelegate

- (BOOL)windowShouldClose:(id)sender
{
	RequestMacOff = true;
	return NO;
}

- (void)windowDidBecomeKey:(NSNotification *)aNotification
{
	gTrueBackgroundFlag = false;
}

- (void)windowDidResignKey:(NSNotification *)aNotification
{
	gTrueBackgroundFlag = true;
}

@end

@interface ClassView : NSView
@end

@implementation ClassView

- (void)resetCursorRects
{
	[self addCursorRect: [self visibleRect]
		cursor: [NSCursor arrowCursor]];
}

- (BOOL)isOpaque
{
	return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
	/*
		Called upon makeKeyAndOrderFront. Create our
		OpenGL context here, because can't do so
		before makeKeyAndOrderFront.
		And if create after then our content won't
		be drawn initially, resulting in flicker.
	*/
	if (GetOpnGLCntxt()) {
		DrawWithOpenGL(0, 0, vMacScreenHeight, vMacScreenWidth);
	}
}

@end

#if UseCGContextDrawImage
/* absent in 10.3.9.  */
CG_EXTERN CGImageRef CGBitmapContextCreateImage(CGContextRef);
#endif


LOCALVAR ClassWindowDelegate *WinDelegate = nil;

LOCALPROC CloseMainWindow(void)
{
	if (nil != WinDelegate) {
		[WinDelegate release];
		WinDelegate = nil;
	}

	if (nil != Window) {
		[Window close];
		Window = nil;
	}

	if (nil != NSview) {
		[NSview release];
		NSview = nil;
	}

#if UseCGContextDrawImage
	if (nil != CGcontext) {
		CGContextFlush(CGcontext);
		CGContextRelease(CGcontext);
		CGcontext = nil;
	}

	if (NULL != Pixels) {
		free(Pixels);
		Pixels = NULL;
	}
#endif

	if (nil != NSOpnGLCntxt) {
		[NSOpnGLCntxt release];
		NSOpnGLCntxt = nil;
	}
}

LOCALPROC QZ_SetCaption(void)
{
#if 0
	NSString *string =
		[[NSString alloc] initWithUTF8String: kStrAppName];
#endif
	[Window setTitle: myAppName /* string */];
#if 0
	[string release];
#endif
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
LOCALVAR NSPoint WinPositionWins[kNumMagStates];
#endif

LOCALVAR NSRect SavedScrnBounds;

LOCALFUNC bool CreateMainWindow(void)
{
#if UseCGContextDrawImage
	CGColorSpaceRef cgColorspace;
#endif
	unsigned int style;
	NSRect MainScrnBounds;
	NSRect AllScrnBounds;
	NSRect NewWinRect;
	NSPoint botleftPos;
	int NewWindowHeight = vMacScreenHeight;
	int NewWindowWidth = vMacScreenWidth;
	bool v = false;

#if VarFullScreen
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

	MainScrnBounds = [[NSScreen mainScreen] frame];
	SavedScrnBounds = MainScrnBounds;
	{
		int i;
		NSArray *screens = [NSScreen screens];
		int n = [screens count];

		AllScrnBounds = MainScrnBounds;
		for (i = 0; i < n; ++i) {
			AllScrnBounds = NSUnionRect(AllScrnBounds,
				[[screens objectAtIndex:i] frame]);
		}
	}

#if EnableMagnify
	if (UseMagnify) {
		NewWindowHeight *= WindowScale;
		NewWindowWidth *= WindowScale;
	}
#endif

	botleftPos.x = MainScrnBounds.origin.x
		+ floor((MainScrnBounds.size.width
			- NewWindowWidth) / 2);
	botleftPos.y = MainScrnBounds.origin.y
		+ floor((MainScrnBounds.size.height
			- NewWindowHeight) / 2);
	if (botleftPos.x < MainScrnBounds.origin.x) {
		botleftPos.x = MainScrnBounds.origin.x;
	}
	if (botleftPos.y < MainScrnBounds.origin.y) {
		botleftPos.y = MainScrnBounds.origin.y;
	}

#if VarFullScreen
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		ViewHSize = MainScrnBounds.size.width;
		ViewVSize = MainScrnBounds.size.height;
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
	if (UseFullScreen)
#endif
#if MayFullScreen
	{
		NewWinRect = AllScrnBounds;

		GLhOffset = botleftPos.x - AllScrnBounds.origin.x;
		GLvOffset = (botleftPos.y - AllScrnBounds.origin.y)
			+ ((NewWindowHeight < MainScrnBounds.size.height)
				? NewWindowHeight : MainScrnBounds.size.height);

		hOffset = GLhOffset;
		vOffset = AllScrnBounds.size.height - GLvOffset;

		style = NSBorderlessWindowMask;
	}
#endif
#if VarFullScreen
	else
#endif
#if MayNotFullScreen
	{
		int WinIndx;

#if EnableMagnify
		if (UseMagnify) {
			WinIndx = kMagStateMagnifgy;
		} else
#endif
		{
			WinIndx = kMagStateNormal;
		}

		if (! HavePositionWins[WinIndx]) {
			WinPositionWins[WinIndx].x = botleftPos.x;
			WinPositionWins[WinIndx].y = botleftPos.y;
			HavePositionWins[WinIndx] = true;
			NewWinRect = NSMakeRect(botleftPos.x, botleftPos.y,
				NewWindowWidth, NewWindowHeight);
		} else {
			NewWinRect = NSMakeRect(WinPositionWins[WinIndx].x,
				WinPositionWins[WinIndx].y,
				NewWindowWidth, NewWindowHeight);
		}

		GLhOffset = 0;
		GLvOffset = NewWindowHeight;

		style = NSTitledWindowMask
			| NSMiniaturizableWindowMask | NSClosableWindowMask;

		CurWinIndx = WinIndx;
	}
#endif

	/* Manually create a window, avoids having a nib file resource */
	Window = [[ClassWindow alloc]
		initWithContentRect: NewWinRect
		styleMask: style
		backing: NSBackingStoreBuffered
		defer: YES];

	if (nil == Window) {
#if dbglog_HAVE
		dbglog_writeln("Could not create the Cocoa window");
#endif
		goto label_exit;
	}

	/* [Window setReleasedWhenClosed: YES]; */
		/*
			no need to set current_video as it's the
			default for NSWindows
		*/
	QZ_SetCaption();
	[Window setAcceptsMouseMovedEvents: YES];
	[Window setViewsNeedDisplay: NO];

	[Window registerForDraggedTypes:
		[NSArray arrayWithObjects:
			NSURLPboardType, NSFilenamesPboardType, nil]];

	WinDelegate = [[ClassWindowDelegate alloc] init];
	if (nil == WinDelegate) {
#if dbglog_HAVE
		dbglog_writeln("Could not create NSview");
#endif
		goto label_exit;
	}
	[Window setDelegate: WinDelegate];

	NSview = [[ClassView alloc] init];
	if (nil == NSview) {
#if dbglog_HAVE
		dbglog_writeln("Could not create NSview");
#endif
		goto label_exit;
	}
	[Window setContentView: NSview];

	[Window makeKeyAndOrderFront: nil];

	/* just in case drawRect didn't get called */
	if (! GetOpnGLCntxt()) {
#if dbglog_HAVE
		dbglog_writeln("Could not GetOpnGLCntxt");
#endif
		goto label_exit;
	}

#if UseCGContextDrawImage
	Pitch = 4 * NewWindowWidth;
	Pixels = malloc(NewWindowHeight * Pitch);

	cgColorspace = CGColorSpaceCreateDeviceRGB();
	CGcontext = CGBitmapContextCreate(Pixels,
		NewWindowWidth, NewWindowHeight,
		8, Pitch, cgColorspace,
		kCGImageAlphaNoneSkipFirst);
	CGColorSpaceRelease(cgColorspace);

	NSgfxContext = [NSGraphicsContext
		graphicsContextWithWindow: Window];
	[NSGraphicsContext setCurrentContext: NSgfxContext];

	BytesPerPixel = 4;
#endif

	v = true;

label_exit:

	return v;
}

#if EnableRecreateW
LOCALPROC ZapWState(void)
{
	Window = nil;
	NSview = nil;
	WinDelegate = nil;
#if UseCGContextDrawImage
	NSgfxContext = nil;
	CGcontext = nil;
	Pixels = NULL;
#endif
	NSOpnGLCntxt = nil;
}
#endif

#if EnableRecreateW
struct WState {
#if MayFullScreen
	uint16_t f_ViewHSize;
	uint16_t f_ViewVSize;
	uint16_t f_ViewHStart;
	uint16_t f_ViewVStart;
	short f_hOffset;
	short f_vOffset;
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
	NSWindow *f_Window;
	NSView *f_NSview;
	ClassWindowDelegate *f_WinDelegate;
#if UseCGContextDrawImage
	NSGraphicsContext *f_NSgfxContext;
	CGContextRef f_CGcontext;
	void *f_Pixels;
	uint16_t f_Pitch;
	uint8_t f_BytesPerPixel;
#endif
	NSOpenGLContext *f_NSOpnGLCntxt;
	short f_GLhOffset;
	short f_GLvOffset;
};
typedef struct WState WState;
#endif

#if EnableRecreateW
LOCALPROC GetWState(WState *r)
{
#if MayFullScreen
	r->f_ViewHSize = ViewHSize;
	r->f_ViewVSize = ViewVSize;
	r->f_ViewHStart = ViewHStart;
	r->f_ViewVStart = ViewVStart;
	r->f_hOffset = hOffset;
	r->f_vOffset = vOffset;
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
	r->f_Window = Window;
	r->f_NSview = NSview;
	r->f_WinDelegate = WinDelegate;
#if UseCGContextDrawImage
	r->f_NSgfxContext = NSgfxContext;
	r->f_CGcontext = CGcontext;
	r->f_Pixels = Pixels;
	r->f_Pitch = Pitch;
	r->f_BytesPerPixel = BytesPerPixel;
#endif
	r->f_NSOpnGLCntxt = NSOpnGLCntxt;
	r->f_GLhOffset = GLhOffset;
	r->f_GLvOffset = GLvOffset;
}
#endif

#if EnableRecreateW
LOCALPROC SetWState(WState *r)
{
#if MayFullScreen
	ViewHSize = r->f_ViewHSize;
	ViewVSize = r->f_ViewVSize;
	ViewHStart = r->f_ViewHStart;
	ViewVStart = r->f_ViewVStart;
	hOffset = r->f_hOffset;
	vOffset = r->f_vOffset;
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
	Window = r->f_Window;
	NSview = r->f_NSview;
	WinDelegate = r->f_WinDelegate;
#if UseCGContextDrawImage
	NSgfxContext = r->f_NSgfxContext;
	CGcontext = r->f_CGcontext;
	Pixels = r->f_Pixels;
	Pitch = r->f_Pitch;
	BytesPerPixel = r->f_BytesPerPixel;
#endif
	NSOpnGLCntxt = r->f_NSOpnGLCntxt;
	GLhOffset = r->f_GLhOffset;
	GLvOffset = r->f_GLvOffset;
}
#endif

#if EnableRecreateW
LOCALPROC ReCreateMainWindow(void)
{
	WState old_state;
	WState new_state;
	bool HadCursorHidden = HaveCursorHidden;

#if VarFullScreen
	if (! UseFullScreen)
#endif
#if MayNotFullScreen
	{
		/* save old position */
		NSRect r =
			[NSWindow contentRectForFrameRect: [Window frame]
				styleMask: [Window styleMask]];
		WinPositionWins[CurWinIndx] = r.origin;
	}
#endif

#if MayFullScreen
	if (GrabMachine) {
		GrabMachine = false;
		UngrabMachine();
	}
#endif

	CloseOpenGLContext();

	GetWState(&old_state);

	ZapWState();

#if EnableMagnify
	UseMagnify = WantMagnify;
#endif
#if VarFullScreen
	UseFullScreen = WantFullScreen;
#endif

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

	} else {
		GetWState(&new_state);
		SetWState(&old_state);
		CloseMainWindow();
		SetWState(&new_state);

		if (HadCursorHidden) {
			(void) MoveMouse(CurMouseH, CurMouseV);
		}
	}
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
				NSRect MainScrnBounds = [[NSScreen mainScreen] frame];

				if ((MainScrnBounds.size.width
						>= vMacScreenWidth * WindowScale)
					&& (MainScrnBounds.size.height
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

/* --- SavedTasks --- */

LOCALPROC LeaveBackground(void)
{
	ReconnectKeyCodes3();
	DisableKeyRepeat();
	EmulationWasInterrupted = true;
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

#if IncludeSonyNew && ! SaveDialogEnable
LOCALFUNC bool FindOrMakeNamedChildDirPath(NSString *parentPath,
	char *ChildName, NSString **childPath)
{
	NSString *r;
	BOOL isDir;
	Boolean isDirectory;
	NSFileManager *fm = [NSFileManager defaultManager];
	bool v = false;

	if (FindNamedChildPath(parentPath, ChildName, &r)) {
		if ([fm fileExistsAtPath:r isDirectory: &isDir])
		{
			if (isDir) {
				*childPath = r;
				v = true;
			} else {
				NSString *RslvPath = ResolveAlias(r, &isDirectory);
				if (nil != RslvPath) {
					if (isDirectory) {
						*childPath = RslvPath;
						v = true;
					}
				}
			}
		} else {
			if ([fm respondsToSelector:@selector(
createDirectoryAtPath:withIntermediateDirectories:attributes:error:
				)])
			{
				if ([fm
					createDirectoryAtPath:r
					withIntermediateDirectories:NO
					attributes:nil
					error:nil])
				{
					*childPath = r;
					v = true;
				}
			} else
			if ([fm respondsToSelector:
				@selector(createDirectoryAtPath:attributes:)])
			{
				if ([fm
					createDirectoryAtPath:r
					attributes:nil])
				{
					*childPath = r;
					v = true;
				}
			} else
			{
				/* fail */
			}
		}
	}

	return v;
}
#endif

@interface NSSavePanel : NSObject
- (NSInteger)runModalForDirectory:(NSString *)path
	file:(NSString *)filename;
- (void)setNameFieldStringValue:(NSString *)value;
@end

#if IncludeSonyNew
LOCALPROC MakeNewDisk(uint32_t L, NSString *drivename)
{
#if SaveDialogEnable
	NSInteger result = NSCancelButton;
	NSSavePanel *panel = [NSSavePanel savePanel];

	BeginDialog();

	if ([panel respondsToSelector:@selector(setNameFieldStringValue:)])
	{
#if 0
		[panel setNameFieldStringValue: drivename];
			/* available as of OS X 10.6 */
#endif
#if 0
		[panel performSelector:@selector(setNameFieldStringValue:)
			withObject: drivename];
#endif
		[((NSSavePanel *)panel)
			setNameFieldStringValue: drivename];

		result = [panel runModal];
	} else
	if ([panel
		respondsToSelector: @selector(runModalForDirectory:file:)])
	{
#if 0
		result = [panel runModalForDirectory: nil file: drivename];
			/*
				compiler warns deprecated. To avoid warning, and
				to still work if removed from SDK, use NSInvocation.
			*/
#endif
#if 0
		NSString *sDirName = nil;
		SEL sel = @selector(runModalForDirectory:file:);
		NSInvocation* invoc =
			[NSInvocation invocationWithMethodSignature:
				[panel methodSignatureForSelector: sel]];
		[invoc setTarget:panel];
		[invoc setSelector:sel];
		[invoc setArgument:&sDirName atIndex:2];
		[invoc setArgument:&drivename atIndex:3];
		[invoc invoke];
		[invoc getReturnValue: &result];
#endif
		/* an easier way ? seems to work */
		result = [((NSSavePanel *)panel)
			runModalForDirectory: nil file: drivename];
	} else
	{
		/* fail */
	}

	EndDialog();

	if (NSOKButton == result) {
		NSString* filePath = [[panel URL] path];
		MakeNewDisk0(L, filePath);
	}
#else /* SaveDialogEnable */
	NSString *sPath;

	if (FindOrMakeNamedChildDirPath(DataPath, "out", &sPath)) {
		NSString *filePath =
			[sPath stringByAppendingPathComponent: drivename];
		MakeNewDisk0(L, filePath);
	}
#endif /* SaveDialogEnable */
}
#endif

#if IncludeSonyNew
LOCALPROC MakeNewDiskAtDefault(uint32_t L)
{
	MakeNewDisk(L, @"untitled.dsk");
}
#endif

LOCALPROC CheckForSavedTasks(void)
{
	if (EvtQNeedRecover) {
		EvtQNeedRecover = false;

		/* attempt cleanup, EvtQNeedRecover may get set again */
		EvtQTryRecoverFromFull();
	}

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

	if (EmulationWasInterrupted) {
		EmulationWasInterrupted = false;

		if (! gTrueBackgroundFlag) {
			CheckMouseState();
		}
	}

#if EnableFSMouseMotion
	if (HaveMouseMotion) {
		MouseConstrain();
	}
#endif

#if VarFullScreen
	if (gTrueBackgroundFlag && WantFullScreen) {
		ToggleWantFullScreen();
	}
#endif

	if (WantScreensChangedCheck) {
		WantScreensChangedCheck = false;

		UpdateOpenGLContext();

#if VarFullScreen
		/*
			triggered on enter full screen for some
			reason in OS X 10.11. so check against
			saved rect.
		*/
		if ((WantFullScreen)
			&& (! NSEqualRects(SavedScrnBounds,
				[[NSScreen mainScreen] frame])))
		{
			ToggleWantFullScreen();
		}
#endif
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

	if ((nullpr != SavedBriefMsg) & ! MacMsgDisplayed) {
		MacMsgDisplayOn();
	}

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
		ReCreateMainWindow();
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

#if IncludeSonyNew
	if (vSonyNewDiskWanted) {
#if IncludeSonyNameNew
		if (vSonyNewDiskName != NotAPbuf) {
			NSString *sNewDiskName;
			if (MacRomanFileNameToNSString(vSonyNewDiskName,
				&sNewDiskName))
			{
				MakeNewDisk(vSonyNewDiskSize, sNewDiskName);
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

	if (NeedWholeScreenDraw) {
		NeedWholeScreenDraw = false;
		ScreenChangedAll();
	}

	if (! gTrueBackgroundFlag) {
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

	if (HaveCursorHidden != (
#if MayNotFullScreen
		(WantCursorHidden
#if VarFullScreen
			|| UseFullScreen
#endif
		) &&
#endif
		! (gTrueBackgroundFlag || CurSpeedStopped)))
	{
		HaveCursorHidden = ! HaveCursorHidden;
		if (HaveCursorHidden) {
			HideCursor();
		} else {
			ShowCursor();
		}
	}

#if 1
	/*
		Check if actual cursor visibility is what it should be.
		If move mouse to dock then cursor is made visible, but then
		if move directly to our window, cursor is not hidden again.
	*/
	if (HaveCGCursorIsVisible()) {
		/* but only in OS X 10.3 and later */
		/* deprecated in cocoa, but no alternative (?) */
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
#endif
}

/* --- main program flow --- */

GLOBALOSGLUFUNC bool ExtraTimeNotOver(void)
{
	UpdateTrueEmulatedTime();
	return TrueEmulatedTime == OnTrueTime;
}

LOCALPROC ProcessEventModifiers(NSEvent *event)
{
	NSUInteger newMods = [event modifierFlags];

	UpdateKeyboardModifiers(newMods);
}

LOCALPROC ProcessEventLocation(NSEvent *event)
{
	NSPoint p = [event locationInWindow];
	NSWindow *w = [event window];

	if (w != Window) {
		if (nil != w) {
			p = [w convertBaseToScreen: p];
		}
		p = [Window convertScreenToBase: p];
	}
	p = [NSview convertPoint: p fromView: nil];
	p.y = [NSview frame].size.height - p.y;
	MousePositionNotify((int) p.x, (int) p.y);
}

LOCALPROC ProcessKeyEvent(bool down, NSEvent *event)
{
	uint8_t scancode = [event keyCode];

	ProcessEventModifiers(event);
	Keyboard_UpdateKeyMap2(Keyboard_RemapMac(scancode), down);
}

LOCALPROC ProcessOneSystemEvent(NSEvent *event)
{
	switch ([event type]) {
		case NSLeftMouseDown:
		case NSRightMouseDown:
		case NSOtherMouseDown:
			/*
				int button = QZ_OtherMouseButtonToSDL(
					[event buttonNumber]);
			*/
			ProcessEventLocation(event);
			ProcessEventModifiers(event);
			if (([event window] == Window)
				&& (! gTrueBackgroundFlag)
#if MayNotFullScreen
				&& (WantCursorHidden
#if VarFullScreen
				|| UseFullScreen
#endif
				)
#endif
				)
			{
				MouseButtonSet(true);
			} else {
				/* doesn't belong to us */
				[NSApp sendEvent: event];
			}
			break;

		case NSLeftMouseUp:
		case NSRightMouseUp:
		case NSOtherMouseUp:
			/*
				int button = QZ_OtherMouseButtonToSDL(
					[event buttonNumber]);
			*/
			ProcessEventLocation(event);
			ProcessEventModifiers(event);
			if (! MouseButtonState) {
				/* doesn't belong to us */
				[NSApp sendEvent: event];
			} else {
				MouseButtonSet(false);
			}
			break;

		case NSMouseMoved:
			{
				ProcessEventLocation(event);
				ProcessEventModifiers(event);
			}
			break;
		case NSLeftMouseDragged:
		case NSRightMouseDragged:
		case NSOtherMouseDragged:
			if (! MouseButtonState) {
				/* doesn't belong to us ? */
				[NSApp sendEvent: event];
			} else {
				ProcessEventLocation(event);
				ProcessEventModifiers(event);
			}
			break;
		case NSKeyUp:
			ProcessKeyEvent(false, event);
			break;
		case NSKeyDown:
			ProcessKeyEvent(true, event);
			break;
		case NSFlagsChanged:
			ProcessEventModifiers(event);
			break;
		/* case NSScrollWheel: */
		/* case NSSystemDefined: */
		/* case NSAppKitDefined: */
		/* case NSApplicationDefined: */
		/* case NSPeriodic: */
		/* case NSCursorUpdate: */
		default:
			[NSApp sendEvent: event];
	}
}

GLOBALOSGLUPROC WaitForNextTick(void)
{
	NSDate *TheUntil;
	int i;
	NSEvent *event;
	NSAutoreleasePool *pool;

	pool = [[NSAutoreleasePool alloc] init];

	NSDate *TheDistantFuture = [NSDate distantFuture];
	NSDate *TheDistantPast = [NSDate distantPast];
#if 0
	NSDate *TheNextTick = [NSDate
		dateWithTimeIntervalSinceReferenceDate: NextTickChangeTime];
#endif

	TheUntil = TheDistantPast;

label_retry:

	i = 32;
	while ((--i >= 0) && (nil != (event =
		[NSApp nextEventMatchingMask: NSAnyEventMask
			untilDate: TheUntil
			inMode: NSDefaultRunLoopMode
			dequeue: YES])))
	{
		ProcessOneSystemEvent(event);
		TheUntil = TheDistantPast;
	}

	CheckForSavedTasks();

	if (ForceMacOff) {
		goto label_exit;
	}

	if (CurSpeedStopped) {
		DoneWithDrawingForTick();
		TheUntil = TheDistantFuture;
		goto label_retry;
	}

	if (ExtraTimeNotOver()) {
#if 1
#if 0 && EnableAutoSlow
		if ((QuietSubTicks >= 16384)
			&& (QuietTime >= 34)
			&& ! WantNotAutoSlow)
		{
			TheUntil = [NSDate
				dateWithTimeIntervalSinceReferenceDate:
					(NextTickChangeTime + 0.50)];
		} else
#endif
		{
			NSTimeInterval inTimeout =
				NextTickChangeTime - LatestTime;
			if (inTimeout > 0.0) {
				struct timespec rqt;
				struct timespec rmt;

				rqt.tv_sec = 0;
				rqt.tv_nsec = inTimeout * 1000000000.0;
				(void) nanosleep(&rqt, &rmt);
			}
			TheUntil = TheDistantPast;
		}
#else
		/*
			This has higher overhead.
		*/
		TheUntil = TheNextTick;
#endif
		goto label_retry;
	}

#if 0
	if (! gTrueBackgroundFlag) {
		CheckMouseState();
	}
#endif

	if (CheckDateTime()) {
#if SoundEnabled
		Sound_SecondNotify();
#endif
	}

	OnTrueTime = TrueEmulatedTime;

#if dbglog_TimeStuff
	dbglog_writelnNum("WaitForNextTick, OnTrueTime", OnTrueTime);
#endif

label_exit:
	[pool release];
}

typedef Boolean (*SecTranslocateIsTranslocatedURL_t)(
	CFURLRef path, bool *isTranslocated, CFErrorRef * error);
typedef CFURLRef (*SecTranslocateCreateOriginalPathForURL_t)(
	CFURLRef translocatedPath, CFErrorRef * error);

LOCALFUNC bool setupWorkingDirectory(void)
{
	NSString *myAppDir;
	NSString *contentsPath;
	NSString *dataPath;
	NSBundle *myBundle = [NSBundle mainBundle];
	NSString *myAppPath = [myBundle bundlePath];

#if WantUnTranslocate
	{
		bool isTranslocated;
		void *sec_handle = NULL;
		SecTranslocateIsTranslocatedURL_t
			mySecTranslocateIsTranslocatedURL = NULL;
		CFURLRef url = NULL;
		SecTranslocateCreateOriginalPathForURL_t
			mySecTranslocateCreateOriginalPathForURL = NULL;
		CFURLRef untranslocatedURL = NULL;
		NSString *realAppPath = NULL;

		if (NULL == (sec_handle = dlopen(
			"/System/Library/Frameworks/Security.framework/Security",
			RTLD_LAZY)))
		{
			/* fail */
		} else
		if (NULL == (mySecTranslocateIsTranslocatedURL =
			dlsym(sec_handle, "SecTranslocateIsTranslocatedURL")))
		{
			/* fail */
		} else
		if (NULL == (url =
			CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
				(CFStringRef)myAppPath, kCFURLPOSIXPathStyle, NO)))
		{
			/* fail */
		} else
		if (! mySecTranslocateIsTranslocatedURL(url, &isTranslocated,
			NULL))
		{
			/* fail */
		} else
		if (! isTranslocated) {
			/* done */
		} else
		if (NULL == (mySecTranslocateCreateOriginalPathForURL =
			dlsym(sec_handle,
				"SecTranslocateCreateOriginalPathForURL")))
		{
			/* fail */
		} else
		if (NULL == (untranslocatedURL =
			mySecTranslocateCreateOriginalPathForURL(url, NULL)))
		{
			/* fail */
		} else
		if (NULL == (realAppPath =
			(NSString *)CFURLCopyFileSystemPath(
				untranslocatedURL, kCFURLPOSIXPathStyle)))
		{
			/* fail */
		} else
		{
			myAppPath = realAppPath;
		}

		if (NULL != realAppPath) {
			[realAppPath autorelease];
		}
		if (NULL != untranslocatedURL) {
			CFRelease(untranslocatedURL);
		}
		if (NULL != url) {
			CFRelease(url);
		}
		if (NULL != sec_handle) {
			if (0 != dlclose(sec_handle)) {
				/* dbglog_writeln("dlclose  failed"); */
			}
		}
	}
#endif /* WantUnTranslocate */

	myAppDir = [myAppPath stringByDeletingLastPathComponent];
	myAppName = [[[myAppPath lastPathComponent]
		stringByDeletingPathExtension] retain];

	DataPath = myAppDir;
	if (FindNamedChildDirPath(myAppPath, "Contents", &contentsPath))
	if (FindNamedChildDirPath(contentsPath, "mnvm_dat", &dataPath))
	{
		DataPath = dataPath;
	}
	[DataPath retain];

	return true;
}

@interface ClassApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation ClassApplicationDelegate

- (BOOL)application:(NSApplication *)theApplication
	openFile:(NSString *)filename
{
	(void) Sony_Insert1a(filename);

	return TRUE;
}

- (void) applicationDidFinishLaunching: (NSNotification *) note
{
	[NSApp stop: nil]; /* stop immediately */

	{
		/*
			doesn't actually stop until an event, so make one.
			(As suggested by Michiel de Hoon in
			http://www.cocoabuilder.com/ post.)
		*/
		NSEvent* event = [NSEvent
			otherEventWithType: NSApplicationDefined
			location: NSMakePoint(0, 0)
			modifierFlags: 0
			timestamp: 0.0
			windowNumber: 0
			context: nil
			subtype: 0
			data1: 0
			data2: 0];
		[NSApp postEvent: event atStart: true];
	}
}

- (void)applicationDidChangeScreenParameters:
	(NSNotification *)aNotification
{
	WantScreensChangedCheck = true;
}

- (IBAction)performSpecialMoreCommands:(id)sender
{
	DoMoreCommandsMsg();
}

- (IBAction)performFileOpen:(id)sender
{
	RequestInsertDisk = true;
}

- (IBAction)performApplicationAbout:(id)sender
{
	DoAboutMsg();
}

@end

LOCALVAR ClassApplicationDelegate *ApplicationDelegate = nil;

LOCALFUNC bool InitCocoaStuff(void)
{
	NSApplication *NSApp = [NSApplication sharedApplication];
		/*
			in Xcode 6.2, NSApp isn't the same as NSApp,
			breaks NSApp setDelegate
		*/

	MenuSetup();

	ApplicationDelegate = [[ClassApplicationDelegate alloc] init];
	[NSApp setDelegate: ApplicationDelegate];

#if 0
	[NSApp finishLaunching];
#endif
		/*
			If use finishLaunching, after
			Hide Mini vMac command, activating from
			Dock doesn't bring our window forward.
			Using "run" instead fixes this.
			As suggested by Hugues De Keyzer in
			http://forums.libsdl.org/ post.
			SDL 2.0 doesn't use this
			technique. Was another solution found?
		*/

	[NSApp run];
		/*
			our applicationDidFinishLaunching forces
			immediate halt.
		*/

	return true;
}

LOCALPROC UnInitCocoaStuff(void)
{
	if (nil != ApplicationDelegate) {
		[ApplicationDelegate release];
	}
	if (nil != myAppName) {
		[myAppName release];
	}
	if (nil != DataPath) {
		[DataPath release];
	}
}

/* --- platform independent code can be thought of as going here --- */

#include "PROGMAIN.h"

LOCALPROC ZapOSGLUVars(void)
{
	InitDrives();
	ZapWinStateVars();
#if SoundEnabled
	ZapAudioVars();
#endif
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
#if 0 /* for testing start up error reporting */
	MacMsg(kStrOutOfMemTitle, kStrOutOfMemMessage, true);
	return false;
#else
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
#endif
}

LOCALPROC UnallocMemory(void)
{
	if (nullpr != ReserveAllocBigBlock) {
		free((char *) ReserveAllocBigBlock);
	}
}

LOCALFUNC bool InitOSGLU(void)
{
	bool IsOk = false;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	if (AllocMemory())
	if (setupWorkingDirectory())
#if dbglog_HAVE
	if (dbglog_open())
#endif
#if SoundEnabled
	if (Sound_Init())
		/* takes a while to stabilize, do as soon as possible */
#endif
	if (LoadMacRom())
	if (LoadInitialImages())
	if (InitCocoaStuff())
		/*
			Can get openFile call backs here
			for initial files.
			So must load ROM, disk1.dsk, etc first.
		*/
#if EmLocalTalk
	if (InitLocalTalk())
#endif
	if (InitLocationDat())
	if (Screen_Init())
	if (CreateMainWindow())
	if (WaitForRom())
	{
		IsOk = true;
	}

	[pool release];

	return IsOk;
}

#if dbglog_HAVE && 0
IMPORTPROC DumpRTC(void);
#endif

LOCALPROC UnInitOSGLU(void)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

#if dbglog_HAVE && 0
	DumpRTC();
#endif

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
#if IncludePbufs
	UnInitPbufs();
#endif
	UnInitDrives();

	ForceShowCursor();

#if dbglog_HAVE
	dbglog_close();
#endif

	CheckSavedMacMsg();

	CloseOpenGLContext();
	CloseMainWindow();

#if MayFullScreen
	_ShowMenuBar();
#endif

	UnInitCocoaStuff();

	UnallocMemory();

	[pool release];
}

int main(int argc, char **argv)
{
	ZapOSGLUVars();

	if (InitOSGLU()) {
		ProgramMain();
	}
	UnInitOSGLU();

	return 0;
}
