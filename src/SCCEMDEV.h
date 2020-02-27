/*
	SCCEMDEV.h

	Copyright (C) 2004 Philip Cummins, Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifndef SCCEMDEV_H
#define SCCEMDEV_H

EXPORTPROC SCC_Reset(void);

EXPORTFUNC uint32_t SCC_Access(uint32_t Data, bool WriteMem, CPTR addr);

EXPORTFUNC bool SCC_InterruptsEnabled(void);

#if EmLocalTalk
EXPORTPROC LocalTalkTick(void);
EXPORTFUNC int InitLocalTalk(void);
#endif

#endif
