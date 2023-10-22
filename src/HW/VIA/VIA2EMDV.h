/*
	HW/VIA/VIA2EMDV.h

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

#ifndef VIA2EMDV_H
#define VIA2EMDV_H

extern void VIA2_Zap(void);
extern void VIA2_Reset(void);

extern uint32_t VIA2_Access(uint32_t Data, bool WriteMem, CPTR addr);

extern void VIA2_ExtraTimeBegin(void);
extern void VIA2_ExtraTimeEnd(void);
#ifdef VIA2_iCA1_PulseNtfy
extern void VIA2_iCA1_PulseNtfy(void);
#endif
#ifdef VIA2_iCA2_PulseNtfy
extern void VIA2_iCA2_PulseNtfy(void);
#endif
#ifdef VIA2_iCB1_PulseNtfy
extern void VIA2_iCB1_PulseNtfy(void);
#endif
#ifdef VIA2_iCB2_PulseNtfy
extern void VIA2_iCB2_PulseNtfy(void);
#endif
extern void VIA2_DoTimer1Check(void);
extern void VIA2_DoTimer2Check(void);

extern uint16_t VIA2_GetT1InvertTime(void);

extern void VIA2_ShiftInData(uint8_t v);
extern uint8_t VIA2_ShiftOutData(void);

#endif
