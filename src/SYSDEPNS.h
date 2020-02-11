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

#ifdef SYSDEPNS_H
#error "header already included"
#else
#define SYSDEPNS_H
#endif

#include "CNFGGLOB.h"


typedef uint8_t *ui3p;
typedef uint16_t *ui4p;
typedef uint32_t *ui5p;

/*
	Largest efficiently supported
	representation types. uimr should be
	large enough to hold number of elements
	of any array we will deal with.
*/
typedef uint32_t uimr;
typedef int32_t simr;

#define blnr uint8_t
#define trueblnr 1
#define falseblnr 0

#define nullpr ((void *) 0)

#define anyp ui3p

/* pascal string, single byte characters */
#define ps3p ui3p

#ifndef MayInline
#define MayInline
#endif

#ifndef my_reg_call
#define my_reg_call
#endif

#ifndef my_osglu_call
#define my_osglu_call
#endif

#define LOCALVAR static
#ifdef AllFiles
#define GLOBALVAR LOCALVAR
#define EXPORTVAR(t, v)
#else
#define GLOBALVAR
#define EXPORTVAR(t, v) extern t v;
#endif

#define LOCALFUNC static
#define FORWARDFUNC LOCALFUNC
#ifdef AllFiles
#define GLOBALFUNC LOCALFUNC
#define EXPORTFUNC LOCALFUNC
#else
#define GLOBALFUNC
#define EXPORTFUNC extern
#endif
#define IMPORTFUNC EXPORTFUNC
#define TYPEDEFFUNC typedef

#define LOCALPROC LOCALFUNC void
#define GLOBALPROC GLOBALFUNC void
#define EXPORTPROC EXPORTFUNC void
#define IMPORTPROC IMPORTFUNC void
#define FORWARDPROC FORWARDFUNC void
#define TYPEDEFPROC TYPEDEFFUNC void

#define LOCALINLINEFUNC static MayInline
#define LOCALINLINEPROC LOCALINLINEFUNC void

#define LOCALFUNCUSEDONCE LOCALINLINEFUNC
#define LOCALPROCUSEDONCE LOCALINLINEPROC

#define GLOBALOSGLUFUNC GLOBALFUNC my_osglu_call
#define EXPORTOSGLUFUNC EXPORTFUNC my_osglu_call
#define GLOBALOSGLUPROC GLOBALFUNC my_osglu_call void
#define EXPORTOSGLUPROC EXPORTFUNC my_osglu_call void
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

#ifndef uint8_tr
#define uint8_tr uint8_t
#endif

#ifndef uint16_tr
#define uint16_tr uint16_t
#endif

#ifndef int32_tr
#define int32_tr int32_t
#endif

#ifndef my_align_8
#define my_align_8
#endif

#ifndef my_cond_rare
#define my_cond_rare(x) (x)
#endif

#ifndef Have_ASR
#define Have_ASR 0
#endif

#ifndef HaveSwapUi5r
#define HaveSwapUi5r 0
#endif
