#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SYSDEPNS.h" 
#include "ERRCODES.h"
#include "STRCONST.h"
#include "UI/MYOSGLUE.h"
#include "UI/COMOSGLU.h"
#include "UI/CONTROLM.h"
#include "UI/SDL2/OSGLUSD2.h"
#include "HW/ROM/ROMEMDEV.h"

/* --- ROM --- */

static char *rom_path = NULL;

#if CanGetAppPath
static MacErr_t LoadMacRomFromPrefDir(void)
{
	MacErr_t err;
	char *t = NULL;
	char *t2 = NULL;

	if (NULL == pref_dir) {
		err = mnvm_fnfErr;
	} else
	if (mnvm_noErr != (err =
		ChildPath(pref_dir, "mnvm_rom", &t)))
	{
		/* fail */
	} else
	if (mnvm_noErr != (err =
		ChildPath(t, RomFileName, &t2)))
	{
		/* fail */
	} else
	{
		err = LoadMacRomFrom(t2);
	}

	MayFree(t2);
	MayFree(t);

	return err;
}
#endif

#if CanGetAppPath
static MacErr_t LoadMacRomFromAppPar(void)
{
	MacErr_t err;
	char *d = (NULL == d_arg) ? app_parent : d_arg;
	char *t = NULL;

	if (NULL == d) {
		err = mnvm_fnfErr;
	} else
	if (mnvm_noErr != (err =
		ChildPath(d, RomFileName, &t)))
	{
		/* fail */
	} else
	{
		err = LoadMacRomFrom(t);
	}

	MayFree(t);

	return err;
}
#endif

bool LoadMacRom(void)
{
	MacErr_t err;

	if ((NULL == rom_path)
		|| (mnvm_fnfErr == (err = LoadMacRomFrom(rom_path))))
#if CanGetAppPath
	if (mnvm_fnfErr == (err = LoadMacRomFromAppPar()))
	if (mnvm_fnfErr == (err = LoadMacRomFromPrefDir()))
#endif
	if (mnvm_fnfErr == (err = LoadMacRomFrom(RomFileName)))
	{
	}

	return true; /* keep launching Mini vMac, regardless */
}

MacErr_t LoadMacRomFrom(char *path)
{
	MacErr_t err;
	FilePtr ROM_File;
	int File_Size;

	ROM_File = FileOpen(path, "rb");
	if (NULL == ROM_File) {
		err = mnvm_fnfErr;
	} else {
		File_Size = FileRead(ROM, 1, kROM_Size, ROM_File);
		if (File_Size != kROM_Size) {
#ifdef FileEof
			if (FileEof(ROM_File))
#else
			if (File_Size > 0)
#endif
			{
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
		FileClose(ROM_File);
	}

	return err;
}

