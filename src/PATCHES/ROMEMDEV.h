/*
	PATCHES/ROMEMDEV.h

	Copyright (C) 2003 Philip Cummins, Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifndef ROMEMDEV_H
#define ROMEMDEV_H


#ifndef DisableRomCheck
#define DisableRomCheck 1
#endif
#ifndef DisableRamTest
#define DisableRamTest 1
#endif

#define kVidMem_Base 0x00540000


extern bool ROM_Init(void);


#endif
