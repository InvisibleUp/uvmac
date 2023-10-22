/*
	SYSDEPNS.h

	Copyright (C) 2006 Bernd Schmidt, Philip Cummins, Paul C. Pratt

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
	SYStem DEPeNdencies.
*/

#ifndef SYSDEPNS_H
#define SYSDEPNS_H

#include <stdint.h>
#include <stdbool.h>
#include "CNFGGLOB.h"

/*
	Largest efficiently supported
	representation types. uimr should be
	large enough to hold number of elements
	of any array we will deal with.
*/
typedef uint32_t uimr;
typedef int32_t simr;

#define nullpr ((void *) 0)

#define anyp uint8_t *

/* pascal string, single byte characters */
#define ps3p uint8_t *

#ifndef reg_call
#define reg_call
#endif

#ifndef osglu_call
#define osglu_call
#endif

#define GLOBALOSGLUFUNC  osglu_call
#define EXPORTOSGLUFUNC extern osglu_call
#define GLOBALOSGLUPROC  osglu_call void
#define EXPORTOSGLUPROC extern osglu_call void
	/*
		For functions in operating system glue that
		are called by rest of program.
	*/

/*
	best type for uint16_t that is probably in register
	(when compiler messes up otherwise)
*/

#ifndef BigEndianUnaligned
#define BigEndianUnaligned 0
#endif

#ifndef LittleEndianUnaligned
#define LittleEndianUnaligned 0
#endif

#ifndef align_8
#define align_8
#endif

#ifndef cond_rare
#define cond_rare(x) (x)
#endif

#ifndef Have_ASR
#define Have_ASR 0
#endif

#ifndef HaveSwapUi5r
#define HaveSwapUi5r 0
#endif

#endif