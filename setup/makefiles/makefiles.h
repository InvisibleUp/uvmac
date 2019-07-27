#ifndef _MAKEFILES_H
#define _MAKEFILES_H

#include "../config.h"

/* Include the corresponsing platform-specific file */

#if gbk_ide_mpw == cur_ide
#include "mpw.h"
#endif

#if gbk_ide_mw8 == cur_ide
#include "metrowerks8.h"
#endif

#if gbk_ide_mvc == cur_ide
#include "minivmac.h"
#endif

#if (gbk_ide_bgc == cur_ide) \
	|| (gbk_ide_cyg == cur_ide) \
	|| (gbk_ide_mgw == cur_ide) \
	|| (gbk_ide_dkp == cur_ide) \
	|| (gbk_ide_dvc == cur_ide) \
	|| (gbk_ide_xcd == cur_ide)
#include "gcc.h"
#endif

#if gbk_ide_snc == cur_ide
#include "sun.h"
#endif

#if gbk_ide_msv == cur_ide
#include "msvc.h"
#endif

#if gbk_ide_lcc == cur_ide
#include "lcc.h"
#endif

#if gbk_ide_dvc == cur_ide
#include "bloodshed.h"
#endif

#if gbk_ide_xcd == cur_ide
#include "xcode.h"
#endif

#if gbk_ide_dmc == cur_ide
#include "digitalmars.h"
#endif

#if gbk_ide_plc == cur_ide
#include "pelles.h"
#endif

#if gbk_ide_ccc == cur_ide
#include "generic.h"
#endif

#endif
