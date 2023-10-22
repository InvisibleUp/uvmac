/*
	HW/RTC/RTCEMDEV.h

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

#ifndef RTCEMDEV_H
#define RTCEMDEV_H

extern bool RTC_Init(void);
extern void RTC_Interrupt(void);

extern void RTCunEnabled_ChangeNtfy(void);
extern void RTCclock_ChangeNtfy(void);
extern void RTCdataLine_ChangeNtfy(void);

#endif