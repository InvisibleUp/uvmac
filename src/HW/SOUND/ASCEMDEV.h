/*
	HW/SOUND/ASCEMDEV.h

	Copyright (C) 2008 Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifndef ASCEMDEV_H
#define ASCEMDEV_H

extern uint32_t ASC_Access(uint32_t Data, bool WriteMem, CPTR addr);
extern void ASC_SubTick(int SubTick);

#endif
