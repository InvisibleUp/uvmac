/*
	M68KITAB.h

	Copyright (C) 2007, Paul C. Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

#ifndef M68KITAB_H
#define M68KITAB_H

enum {
	// 68000
	kIKindTst,
	kIKindCmpB,
	kIKindCmpW,
	kIKindCmpL,
	kIKindBccB,
	kIKindBccW,
	kIKindBraB,
	kIKindBraW,
	kIKindDBcc,
	kIKindDBF,
	kIKindSwap,
	kIKindMoveL,
	kIKindMoveW,
	kIKindMoveB,
	kIKindMoveAL,
	kIKindMoveAW,
	kIKindMoveQ,
	kIKindAddB,
	kIKindAddW,
	kIKindAddL,
	kIKindSubB,
	kIKindSubW,
	kIKindSubL,
	kIKindLea,
	kIKindPEA,
	kIKindA,
	kIKindBsrB,
	kIKindBsrW,
	kIKindJsr,
	kIKindLinkA6,
	kIKindMOVEMRmML,
	kIKindMOVEMApRL,
	kIKindUnlkA6,
	kIKindRts,
	kIKindJmp,
	kIKindClr,
	kIKindAddA,
	kIKindAddQA,
	kIKindSubA,
	kIKindSubQA,
	kIKindCmpA,
	kIKindAddXB,
	kIKindAddXW,
	kIKindAddXL,
	kIKindSubXB,
	kIKindSubXW,
	kIKindSubXL,
	kIKindAslB,
	kIKindAslW,
	kIKindAslL,
	kIKindAsrB,
	kIKindAsrW,
	kIKindAsrL,
	kIKindLslB,
	kIKindLslW,
	kIKindLslL,
	kIKindLsrB,
	kIKindLsrW,
	kIKindLsrL,
	kIKindRxlB,
	kIKindRxlW,
	kIKindRxlL,
	kIKindRxrB,
	kIKindRxrW,
	kIKindRxrL,
	kIKindRolB,
	kIKindRolW,
	kIKindRolL,
	kIKindRorB,
	kIKindRorW,
	kIKindRorL,
	kIKindBTstB,
	kIKindBChgB,
	kIKindBClrB,
	kIKindBSetB,
	kIKindBTstL,
	kIKindBChgL,
	kIKindBClrL,
	kIKindBSetL,
	kIKindAndI,
	kIKindAndEaD,
	kIKindAndDEa,
	kIKindOrI,
	kIKindOrDEa,
	kIKindOrEaD,
	kIKindEor,
	kIKindEorI,
	kIKindNot,
	kIKindScc,
	kIKindNegXB,
	kIKindNegXW,
	kIKindNegXL,
	kIKindNegB,
	kIKindNegW,
	kIKindNegL,
	kIKindEXTW,
	kIKindEXTL,
	kIKindMulU,
	kIKindMulS,
	kIKindDivU,
	kIKindDivS,
	kIKindExg,
	kIKindMoveEaCCR,
	kIKindMoveSREa,
	kIKindMoveEaSR,
	kIKindOrISR,
	kIKindAndISR,
	kIKindEorISR,
	kIKindOrICCR,
	kIKindAndICCR,
	kIKindEorICCR,
	kIKindMOVEMApRW,
	kIKindMOVEMRmMW,
	kIKindMOVEMrmW,
	kIKindMOVEMrmL,
	kIKindMOVEMmrW,
	kIKindMOVEMmrL,
	kIKindAbcd,
	kIKindSbcd,
	kIKindNbcd,
	kIKindRte,
	kIKindNop,
	kIKindMoveP0,
	kIKindMoveP1,
	kIKindMoveP2,
	kIKindMoveP3,
	kIKindIllegal,
	kIKindChkW,
	kIKindTrap,
	kIKindTrapV,
	kIKindRtr,
	kIKindLink,
	kIKindUnlk,
	kIKindMoveRUSP,
	kIKindMoveUSPR,
	kIKindTas,
	kIKindFdflt,
	kIKindStop,
	kIKindReset,
	// 68020
	kIKindCallMorRtm,
	kIKindBraL,
	kIKindBccL,
	kIKindBsrL,
	kIKindEXTBL,
	kIKindTRAPcc,
	kIKindChkL,
	kIKindBkpt,
	kIKindDivL,
	kIKindMulL,
	kIKindRtd,
	kIKindMoveCCREa,
	kIKindMoveCEa,
	kIKindMoveEaC,
	kIKindLinkL,
	kIKindPack,
	kIKindUnpk,
	kIKindCHK2orCMP2,
	kIKindCAS2,
	kIKindCAS,
	kIKindMoveS,
	kIKindBitField,
	// MMU
	kIKindMMU,
	// FPU
	kIKindFPUmd60,
	kIKindFPUDBcc,
	kIKindFPUTrapcc,
	kIKindFPUScc,
	kIKindFPUFBccW,
	kIKindFPUFBccL,
	kIKindFPUSave,
	kIKindFPURestore,
	kIKindFPUdflt,

	kNumIKinds
};

enum {
	kAMdRegB,
	kAMdRegW,
	kAMdRegL,
	kAMdIndirectB,
	kAMdIndirectW,
	kAMdIndirectL,
	kAMdAPosIncB,
	kAMdAPosIncW,
	kAMdAPosIncL,
	kAMdAPosInc7B,
	kAMdAPreDecB,
	kAMdAPreDecW,
	kAMdAPreDecL,
	kAMdAPreDec7B,
	kAMdADispB,
	kAMdADispW,
	kAMdADispL,
	kAMdAIndexB,
	kAMdAIndexW,
	kAMdAIndexL,
	kAMdAbsWB,
	kAMdAbsWW,
	kAMdAbsWL,
	kAMdAbsLB,
	kAMdAbsLW,
	kAMdAbsLL,
	kAMdPCDispB,
	kAMdPCDispW,
	kAMdPCDispL,
	kAMdPCIndexB,
	kAMdPCIndexW,
	kAMdPCIndexL,
	kAMdImmedB,
	kAMdImmedW,
	kAMdImmedL,
	kAMdDat4,

	kNumAMds
};

struct DecOpXR {
	/* expected size : 4 bytes */
	uint16_t MainClas;
	uint16_t Cycles;
};
typedef struct DecOpXR DecOpXR;

struct DecArgR {
	/* expected size : 2 bytes */
	uint8_t AMd;
	uint8_t ArgDat;
};
typedef struct DecArgR DecArgR;

struct DecOpYR {
	/* expected size : 4 bytes */
	DecArgR v[2];
};
typedef struct DecOpYR DecOpYR;

struct DecOpR {
	/* expected size : 8 bytes */
	DecOpXR x;
	DecOpYR y;
} align_8;
typedef struct DecOpR DecOpR;

#define GetDcoCycles(p) ((p)->x.Cycles)

#define SetDcoMainClas(p, xx) ((p)->x.MainClas = (xx))
#define SetDcoCycles(p, xx) ((p)->x.Cycles = (xx))

extern void M68KITAB_setup(DecOpR *p);

#endif