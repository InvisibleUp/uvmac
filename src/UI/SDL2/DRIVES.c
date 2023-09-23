#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include "CNFGRAPI.h"
#include "EMCONFIG.h"
#include "SYSDEPNS.h"
#include "UTIL/ENDIANAC.h"
#include "UI/MYOSGLUE.h"
#include "UI/COMOSGLU.h"
#include "STRCONST.h"
#include "HW/ROM/ROMEMDEV.h"
#include "UI/CONTROLM.h"
#include "UI/SDL2/OSGLUSD2.h"

/* --- drives --- */

#define NotAfileRef NULL

FilePtr Drives[NumDrives]; /* open disk image files */

void InitDrives(void)
{
	/*
		This isn't really needed, Drives[i] and DriveNames[i]
		need not have valid values when not vSonyIsInserted[i].
	*/
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		Drives[i] = NotAfileRef;
	}
}

MacErr_t vSonyTransfer(bool IsWrite, uint8_t * Buffer,
	tDrive Drive_No, uint32_t Sony_Start, uint32_t Sony_Count,
	uint32_t *Sony_ActCount)
{
	MacErr_t err = mnvm_miscErr;
	FilePtr refnum = Drives[Drive_No];
	uint32_t NewSony_Count = 0;

	if (Seek(refnum, Sony_Start, SeekSet) >= 0) {
		if (IsWrite) {
			NewSony_Count = FileWrite(Buffer, 1, Sony_Count, refnum);
		} else {
			NewSony_Count = FileRead(Buffer, 1, Sony_Count, refnum);
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

MacErr_t vSonyGetSize(tDrive Drive_No, uint32_t *Sony_Count)
{
	MacErr_t err = mnvm_miscErr;
	FilePtr refnum = Drives[Drive_No];
	long v;

	if (Seek(refnum, 0, SeekEnd) >= 0) {
		v = FileTell(refnum);
		if (v >= 0) {
			*Sony_Count = v;
			err = mnvm_noErr;
		}
	}

	return err; /*& figure out what really to return &*/
}

MacErr_t vSonyEject0(tDrive Drive_No, bool deleteit)
{
	FilePtr refnum = Drives[Drive_No];

	DiskEjectedNotify(Drive_No);

	FileClose(refnum);
	Drives[Drive_No] = NotAfileRef; /* not really needed */

	return mnvm_noErr;
}

MacErr_t vSonyEject(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, false);
}

MacErr_t vSonyEjectDelete(tDrive Drive_No)
{
	return vSonyEject0(Drive_No, true);
}

// TODO: complete the stub
MacErr_t vSonyGetName (uint16_t Drive_No, uint16_t* r)
{
	memcpy("TEST\0", r, 5);
	return mnvm_noErr;
}


void UnInitDrives(void)
{
	tDrive i;

	for (i = 0; i < NumDrives; ++i) {
		if (vSonyIsInserted(i)) {
			(void) vSonyEject(i);
		}
	}
}

bool Sony_Insert0(FilePtr refnum, bool locked, char *drivepath)
{
	tDrive Drive_No;
	bool IsOk = false;

	if (! FirstFreeDisk(&Drive_No)) {
		MacMsg(kStrTooManyImagesTitle, kStrTooManyImagesMessage,
			false);
	} else {
		/* printf("Sony_Insert0 %d\n", (int)Drive_No); */

		{
			Drives[Drive_No] = refnum;
			DiskInsertNotify(Drive_No, locked);

			IsOk = true;
		}
	}

	if (! IsOk) {
		FileClose(refnum);
	}

	return IsOk;
}

bool Sony_Insert1(char *drivepath, bool silentfail)
{
	bool locked = false;
	/* printf("Sony_Insert1 %s\n", drivepath); */
	FilePtr refnum = FileOpen(drivepath, "rb+");
	if (NULL == refnum) {
		locked = true;
		refnum = FileOpen(drivepath, "rb");
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

bool Sony_Insert1a(char *drivepath, bool silentfail)
{
	bool v;

	if (! ROM_loaded) {
		v = (mnvm_noErr == LoadMacRomFrom(drivepath));
	} else {
		v = Sony_Insert1(drivepath, silentfail);
	}

	return v;
}

// TODO: implement stub function (or don't, lol)
bool Sony_Insert2(char *s)
{
	return false;
}

bool Sony_InsertIth(int i)
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

bool LoadInitialImages(void)
{
	if (! AnyDiskInserted()) {
		int i;

		for (i = 1; Sony_InsertIth(i); ++i) {
			/* stop on first error (including file not found) */
		}
	}

	return true;
}
