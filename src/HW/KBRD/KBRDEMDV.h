/*
	HW/KBRD/KBRDEMDV.h

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

#ifdef KBRDEMDV_H
#error "header already included"
#else
#define KBRDEMDV_H
#endif


extern void Kybd_DataLineChngNtfy(void);
extern void DoKybd_ReceiveEndCommand(void);
extern void DoKybd_ReceiveCommand(void);
extern void KeyBoard_Update(void);
