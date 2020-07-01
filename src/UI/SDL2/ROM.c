#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SYSDEPNS.h" 
#include "UI/MYOSGLUE.h"
#include "UI/COMOSGLU.h"

/* --- ROM --- */

LOCALVAR char *rom_path = NULL;

#if CanGetAppPath
LOCALFUNC MacErr_t LoadMacRomFromPrefDir(void)
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
LOCALFUNC MacErr_t LoadMacRomFromAppPar(void)
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
