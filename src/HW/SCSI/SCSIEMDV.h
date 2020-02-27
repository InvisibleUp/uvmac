/*
	HW/SCSI/SCSIEMDV.h

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

#ifndef SCSIEMDV_H
#define SCSIEMDV_H

EXPORTPROC SCSI_Reset(void);

EXPORTFUNC uint32_t SCSI_Access(uint32_t Data, bool WriteMem, CPTR addr);

#endif
