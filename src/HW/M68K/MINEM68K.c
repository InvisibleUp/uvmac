/*
	MINEM68K.c

	Copyright (C) 2009 Bernd Schmidt, Paul C. Pratt

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
	EMulator of 68K cpu with GENeric c code (not assembly)

	This code descends from a simple 68000 emulator that I
	(Paul C. Pratt) wrote long ago. That emulator ran on a 680x0,
	and used the cpu it ran on to do some of the work. This
	descendent fills in those holes with code from the
	Un*x Amiga Emulator by Bernd Schmidt, as found being used in vMac.

	This emulator is about 10 times smaller than the UAE,
	at the cost of being 2 to 3 times slower.

	FPU Emulation added 9/12/2009 by Ross Martin
		(this code now located in "FPCPEMDV.h")
*/

#ifndef AllFiles
#include "SYSDEPNS.h"

#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#include "GLOBGLUE.h"

#include "HW/M68K/M68KITAB.h"

#if WantDisasm
#include "HW/M68K/DISAM68K.h"
#endif
#endif

#include "HW/M68K/MINEM68K.h"

/*
	ReportAbnormalID unused 0x0123 - 0x01FF
*/

#ifndef DisableLazyFlagAll
#define DisableLazyFlagAll 0
#endif
	/*
		useful for debugging, to tell if an observed bug is
		being cause by lazy flag evaluation stuff.
		Can also disable parts of it individually:
	*/

#ifndef ForceFlagsEval
#if DisableLazyFlagAll
#define ForceFlagsEval 1
#else
#define ForceFlagsEval 0
#endif
#endif

#ifndef UseLazyZ
#if DisableLazyFlagAll || ForceFlagsEval
#define UseLazyZ 0
#else
#define UseLazyZ 1
#endif
#endif

#ifndef UseLazyCC
#if DisableLazyFlagAll
#define UseLazyCC 0
#else
#define UseLazyCC 1
#endif
#endif


typedef unsigned char flagtype; /* must be 0 or 1, not boolean */


/* Memory Address Translation Cache record */

struct MATCr {
	uint32_t cmpmask;
	uint32_t cmpvalu;
	uint32_t usemask;
	uint8_t * usebase;
};
typedef struct MATCr MATCr;
typedef MATCr *MATCp;

#ifndef USE_PCLIMIT
#define USE_PCLIMIT 1
#endif

#define AKMemory 0
#define AKRegister 1

union ArgAddrT {
	uint32_t mem;
	uint32_t *rga;
};
typedef union ArgAddrT ArgAddrT;

enum {
	kLazyFlagsDefault,
	kLazyFlagsTstB,
	kLazyFlagsTstW,
	kLazyFlagsTstL,
	kLazyFlagsCmpB,
	kLazyFlagsCmpW,
	kLazyFlagsCmpL,
	kLazyFlagsSubB,
	kLazyFlagsSubW,
	kLazyFlagsSubL,
	kLazyFlagsAddB,
	kLazyFlagsAddW,
	kLazyFlagsAddL,
	kLazyFlagsNegB,
	kLazyFlagsNegW,
	kLazyFlagsNegL,
	kLazyFlagsAsrB,
	kLazyFlagsAsrW,
	kLazyFlagsAsrL,
	kLazyFlagsAslB,
	kLazyFlagsAslW,
	kLazyFlagsAslL,
#if UseLazyZ
	kLazyFlagsZSet,
#endif

	kNumLazyFlagsKinds
};

typedef void (reg_call *ArgSetDstP)(uint32_t f);

#define FasterAlignedL 0
	/*
		If most long memory access is long aligned,
		this should be faster. But on the Mac, this
		doesn't seem to be the case, so an
		unpredictable branch slows it down.
	*/

#ifndef HaveGlbReg
#define HaveGlbReg 0
#endif

LOCALVAR struct regstruct
{
	uint32_t regs[16]; /* Data and Address registers */

	uint8_t * pc_p;
	uint8_t * pc_pHi;
	int32_t MaxCyclesToGo;

#if WantCloserCyc
	DecOpR *CurDecOp;
#endif
	DecOpYR CurDecOpY;

	uint8_t LazyFlagKind;
	uint8_t LazyXFlagKind;
#if UseLazyZ
	uint8_t LazyFlagZSavedKind;
#endif
	uint32_t LazyFlagArgSrc;
	uint32_t LazyFlagArgDst;
	uint32_t LazyXFlagArgSrc;
	uint32_t LazyXFlagArgDst;

	ArgAddrT ArgAddr;
	ArgSetDstP ArgSetDst;
	uint32_t SrcVal;

	uint8_t * pc_pLo;
	uint32_t pc; /* Program Counter */

	MATCr MATCrdB;
	MATCr MATCwrB;
	MATCr MATCrdW;
	MATCr MATCwrW;
#if FasterAlignedL
	MATCr MATCrdL;
	MATCr MATCwrL;
#endif
	ATTep HeadATTel;

	int32_t MoreCyclesToGo;
	int32_t ResidualCycles;
	uint8_t fakeword[2];

	/* Status Register */
	uint32_t intmask; /* bits 10-8 : interrupt priority mask */
	flagtype t1; /* bit 15: Trace mode 1 */
#if Use68020
	flagtype t0; /* bit 14: Trace mode 0 */
#endif
	flagtype s; /* bit 13: Supervisor or user privilege level */
#if Use68020
	flagtype m; /* bit 12: Master or interrupt mode */
#endif

	flagtype x; /* bit 4: eXtend */
	flagtype n; /* bit 3: Negative */
	flagtype z; /* bit 2: Zero */
	flagtype v; /* bit 1: oVerflow */
	flagtype c; /* bit 0: Carry */

#if EmMMU | EmFPU
	uint32_t ArgKind;
#endif

	bool TracePending;
	bool ExternalInterruptPending;
#if 0
	bool ResetPending;
#endif
	uint8_t *fIPL;
#ifdef r_regs
	struct regstruct *save_regs;
#endif

	CPTR usp; /* User Stack Pointer */
	CPTR isp; /* Interrupt Stack Pointer */
#if Use68020
	CPTR msp; /* Master Stack Pointer */
	uint32_t sfc; /* Source Function Code register */
	uint32_t dfc; /* Destination Function Code register */
	uint32_t vbr; /* Vector Base Register */
	uint32_t cacr; /* Cache Control Register */
		/*
			bit 0 : Enable Cache
			bit 1 : Freeze Cache
			bit 2 : Clear Entry In Cache (write only)
			bit 3 : Clear Cache (write only)
		*/
	uint32_t caar; /* Cache Address Register */
#endif

#define disp_table_sz (256 * 256)
#if SmallGlobals
	DecOpR *disp_table;
#else
	DecOpR disp_table[disp_table_sz];
#endif
} regs;

#define uint32_t_MSBisSet(x) (((int32_t)(x)) < 0)

#define Bool2Bit(x) ((x) ? 1 : 0)


#ifdef r_regs
register struct regstruct *g_regs asm (r_regs);
#define V_regs (*g_regs)
#else
#define V_regs regs
#endif

#ifdef r_pc_p
register uint8_t * g_pc_p asm (r_pc_p);
#define V_pc_p g_pc_p
#else
#define V_pc_p V_regs.pc_p
#endif

#ifdef r_MaxCyclesToGo
register int32_t g_MaxCyclesToGo asm (r_MaxCyclesToGo);
#define V_MaxCyclesToGo g_MaxCyclesToGo
#else
#define V_MaxCyclesToGo V_regs.MaxCyclesToGo
#endif

#ifdef r_pc_pHi
register uint8_t * g_pc_pHi asm (r_pc_pHi);
#define V_pc_pHi g_pc_pHi
#else
#define V_pc_pHi V_regs.pc_pHi
#endif

#define ZFLG V_regs.z
#define NFLG V_regs.n
#define CFLG V_regs.c
#define VFLG V_regs.v
#define XFLG V_regs.x

#define m68k_dreg(num) (V_regs.regs[(num)])
#define m68k_areg(num) (V_regs.regs[(num) + 8])


#ifndef WantDumpTable
#define WantDumpTable 0
#endif

#if WantDumpTable
LOCALVAR uint32_t DumpTable[kNumIKinds];
#endif

#if USE_PCLIMIT
FORWARDPROC Recalc_PC_Block(void);
FORWARDFUNC uint32_t reg_call Recalc_PC_BlockReturnUi5r(uint32_t v);
#endif

LOCALINLINEFUNC uint16_t nextiword(void)
/* NOT sign extended */
{
	uint16_t r = do_get_mem_word(V_pc_p);
	V_pc_p += 2;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		Recalc_PC_Block();
	}
#endif

	return r;
}

LOCALINLINEFUNC uint32_t nextiSByte(void)
{
	uint32_t r = uint32_t_FromSByte(do_get_mem_byte(V_pc_p + 1));
	V_pc_p += 2;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		return Recalc_PC_BlockReturnUi5r(r);
	}
#endif

	return r;
}

LOCALINLINEFUNC uint32_t nextiSWord(void)
/* NOT sign extended */
{
	uint32_t r = uint32_t_FromSWord(do_get_mem_word(V_pc_p));
	V_pc_p += 2;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		return Recalc_PC_BlockReturnUi5r(r);
	}
#endif

	return r;
}

FORWARDFUNC uint32_t nextilong_ext(void);

LOCALINLINEFUNC uint32_t nextilong(void)
{
	uint32_t r = do_get_mem_long(V_pc_p);
	V_pc_p += 4;

#if USE_PCLIMIT
	/* could be two words in different blocks */
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		r = nextilong_ext();
	}
#endif

	return r;
}

LOCALINLINEPROC BackupPC(void)
{
	V_pc_p -= 2;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p < V_regs.pc_pLo)) {
		Recalc_PC_Block();
	}
#endif
}

LOCALINLINEFUNC CPTR m68k_getpc(void)
{
	return V_regs.pc + (V_pc_p - V_regs.pc_pLo);
}


FORWARDPROC DoCodeTst(void);
FORWARDPROC DoCodeCmpB(void);
FORWARDPROC DoCodeCmpW(void);
FORWARDPROC DoCodeCmpL(void);
FORWARDPROC DoCodeBccB(void);
FORWARDPROC DoCodeBccW(void);
FORWARDPROC DoCodeBraB(void);
FORWARDPROC DoCodeBraW(void);
FORWARDPROC DoCodeDBcc(void);
FORWARDPROC DoCodeDBF(void);
FORWARDPROC DoCodeSwap(void);
FORWARDPROC DoCodeMoveL(void);
FORWARDPROC DoCodeMoveW(void);
FORWARDPROC DoCodeMoveB(void);
FORWARDPROC DoCodeMoveA(void);
FORWARDPROC DoCodeMoveQ(void);
FORWARDPROC DoCodeAddB(void);
FORWARDPROC DoCodeAddW(void);
FORWARDPROC DoCodeAddL(void);
FORWARDPROC DoCodeSubB(void);
FORWARDPROC DoCodeSubW(void);
FORWARDPROC DoCodeSubL(void);
FORWARDPROC DoCodeLea(void);
FORWARDPROC DoCodePEA(void);
FORWARDPROC DoCodeA(void);
FORWARDPROC DoCodeBsrB(void);
FORWARDPROC DoCodeBsrW(void);
FORWARDPROC DoCodeJsr(void);
FORWARDPROC DoCodeLinkA6(void);
FORWARDPROC DoCodeMOVEMRmML(void);
FORWARDPROC DoCodeMOVEMApRL(void);
FORWARDPROC DoCodeUnlkA6(void);
FORWARDPROC DoCodeRts(void);
FORWARDPROC DoCodeJmp(void);
FORWARDPROC DoCodeClr(void);
FORWARDPROC DoCodeAddA(void);
FORWARDPROC DoCodeSubA(void);
FORWARDPROC DoCodeCmpA(void);
FORWARDPROC DoCodeAddXB(void);
FORWARDPROC DoCodeAddXW(void);
FORWARDPROC DoCodeAddXL(void);
FORWARDPROC DoCodeSubXB(void);
FORWARDPROC DoCodeSubXW(void);
FORWARDPROC DoCodeSubXL(void);
FORWARDPROC DoCodeAslB(void);
FORWARDPROC DoCodeAslW(void);
FORWARDPROC DoCodeAslL(void);
FORWARDPROC DoCodeAsrB(void);
FORWARDPROC DoCodeAsrW(void);
FORWARDPROC DoCodeAsrL(void);
FORWARDPROC DoCodeLslB(void);
FORWARDPROC DoCodeLslW(void);
FORWARDPROC DoCodeLslL(void);
FORWARDPROC DoCodeLsrB(void);
FORWARDPROC DoCodeLsrW(void);
FORWARDPROC DoCodeLsrL(void);
FORWARDPROC DoCodeRxlB(void);
FORWARDPROC DoCodeRxlW(void);
FORWARDPROC DoCodeRxlL(void);
FORWARDPROC DoCodeRxrB(void);
FORWARDPROC DoCodeRxrW(void);
FORWARDPROC DoCodeRxrL(void);
FORWARDPROC DoCodeRolB(void);
FORWARDPROC DoCodeRolW(void);
FORWARDPROC DoCodeRolL(void);
FORWARDPROC DoCodeRorB(void);
FORWARDPROC DoCodeRorW(void);
FORWARDPROC DoCodeRorL(void);
FORWARDPROC DoCodeBTstB(void);
FORWARDPROC DoCodeBChgB(void);
FORWARDPROC DoCodeBClrB(void);
FORWARDPROC DoCodeBSetB(void);
FORWARDPROC DoCodeBTstL(void);
FORWARDPROC DoCodeBChgL(void);
FORWARDPROC DoCodeBClrL(void);
FORWARDPROC DoCodeBSetL(void);
FORWARDPROC DoCodeAnd(void);
FORWARDPROC DoCodeOr(void);
FORWARDPROC DoCodeEor(void);
FORWARDPROC DoCodeNot(void);
FORWARDPROC DoCodeScc(void);
FORWARDPROC DoCodeNegXB(void);
FORWARDPROC DoCodeNegXW(void);
FORWARDPROC DoCodeNegXL(void);
FORWARDPROC DoCodeNegB(void);
FORWARDPROC DoCodeNegW(void);
FORWARDPROC DoCodeNegL(void);
FORWARDPROC DoCodeEXTW(void);
FORWARDPROC DoCodeEXTL(void);
FORWARDPROC DoCodeMulU(void);
FORWARDPROC DoCodeMulS(void);
FORWARDPROC DoCodeDivU(void);
FORWARDPROC DoCodeDivS(void);
FORWARDPROC DoCodeExg(void);
FORWARDPROC DoCodeMoveEaCR(void);
FORWARDPROC DoCodeMoveSREa(void);
FORWARDPROC DoCodeMoveEaSR(void);
FORWARDPROC DoCodeOrISR(void);
FORWARDPROC DoCodeAndISR(void);
FORWARDPROC DoCodeEorISR(void);
FORWARDPROC DoCodeOrICCR(void);
FORWARDPROC DoCodeAndICCR(void);
FORWARDPROC DoCodeEorICCR(void);
FORWARDPROC DoCodeMOVEMApRW(void);
FORWARDPROC DoCodeMOVEMRmMW(void);
FORWARDPROC DoCodeMOVEMrmW(void);
FORWARDPROC DoCodeMOVEMrmL(void);
FORWARDPROC DoCodeMOVEMmrW(void);
FORWARDPROC DoCodeMOVEMmrL(void);
FORWARDPROC DoCodeAbcd(void);
FORWARDPROC DoCodeSbcd(void);
FORWARDPROC DoCodeNbcd(void);
FORWARDPROC DoCodeRte(void);
FORWARDPROC DoCodeNop(void);
FORWARDPROC DoCodeMoveP0(void);
FORWARDPROC DoCodeMoveP1(void);
FORWARDPROC DoCodeMoveP2(void);
FORWARDPROC DoCodeMoveP3(void);
FORWARDPROC op_illg(void);
FORWARDPROC DoCodeChk(void);
FORWARDPROC DoCodeTrap(void);
FORWARDPROC DoCodeTrapV(void);
FORWARDPROC DoCodeRtr(void);
FORWARDPROC DoCodeLink(void);
FORWARDPROC DoCodeUnlk(void);
FORWARDPROC DoCodeMoveRUSP(void);
FORWARDPROC DoCodeMoveUSPR(void);
FORWARDPROC DoCodeTas(void);
FORWARDPROC DoCodeFdefault(void);
FORWARDPROC DoCodeStop(void);
FORWARDPROC DoCodeReset(void);

#if Use68020
FORWARDPROC DoCodeCallMorRtm(void);
FORWARDPROC DoCodeBraL(void);
FORWARDPROC DoCodeBccL(void);
FORWARDPROC DoCodeBsrL(void);
FORWARDPROC DoCodeEXTBL(void);
FORWARDPROC DoCodeTRAPcc(void);
FORWARDPROC DoCodeBkpt(void);
FORWARDPROC DoCodeDivL(void);
FORWARDPROC DoCodeMulL(void);
FORWARDPROC DoCodeRtd(void);
FORWARDPROC DoCodeMoveCCREa(void);
FORWARDPROC DoMoveFromControl(void);
FORWARDPROC DoMoveToControl(void);
FORWARDPROC DoCodeLinkL(void);
FORWARDPROC DoCodePack(void);
FORWARDPROC DoCodeUnpk(void);
FORWARDPROC DoCHK2orCMP2(void);
FORWARDPROC DoCAS2(void);
FORWARDPROC DoCAS(void);
FORWARDPROC DoMOVES(void);
FORWARDPROC DoBitField(void);
#endif

#if EmMMU
FORWARDPROC DoCodeMMU(void);
#endif

#if EmFPU
FORWARDPROC DoCodeFPU_md60(void);
FORWARDPROC DoCodeFPU_DBcc(void);
FORWARDPROC DoCodeFPU_Trapcc(void);
FORWARDPROC DoCodeFPU_Scc(void);
FORWARDPROC DoCodeFPU_FBccW(void);
FORWARDPROC DoCodeFPU_FBccL(void);
FORWARDPROC DoCodeFPU_Save(void);
FORWARDPROC DoCodeFPU_Restore(void);
FORWARDPROC DoCodeFPU_dflt(void);
#endif

typedef void (*func_pointer_t)(void);

LOCALVAR const func_pointer_t OpDispatch[kNumIKinds + 1] = {
	DoCodeTst /* kIKindTst */,
	DoCodeCmpB /* kIKindCmpB */,
	DoCodeCmpW /* kIKindCmpW */,
	DoCodeCmpL /* kIKindCmpL */,
	DoCodeBccB /* kIKindBccB */,
	DoCodeBccW /* kIKindBccW */,
	DoCodeBraB /* kIKindBraB */,
	DoCodeBraW /* kIKindBraW */,
	DoCodeDBcc /* kIKindDBcc */,
	DoCodeDBF /* kIKindDBF */,
	DoCodeSwap /* kIKindSwap */,
	DoCodeMoveL /* kIKindMoveL */,
	DoCodeMoveW /* kIKindMoveW */,
	DoCodeMoveB /* kIKindMoveB */,
	DoCodeMoveA /* kIKindMoveAL */,
	DoCodeMoveA /* kIKindMoveAW */,
	DoCodeMoveQ /* kIKindMoveQ */,
	DoCodeAddB /* kIKindAddB */,
	DoCodeAddW /* kIKindAddW */,
	DoCodeAddL /* kIKindAddL */,
	DoCodeSubB /* kIKindSubB */,
	DoCodeSubW /* kIKindSubW */,
	DoCodeSubL /* kIKindSubL */,
	DoCodeLea /* kIKindLea */,
	DoCodePEA /* kIKindPEA */,
	DoCodeA /* kIKindA */,
	DoCodeBsrB /* kIKindBsrB */,
	DoCodeBsrW /* kIKindBsrW */,
	DoCodeJsr /* kIKindJsr */,
	DoCodeLinkA6 /* kIKindLinkA6 */,
	DoCodeMOVEMRmML /* kIKindMOVEMRmML */,
	DoCodeMOVEMApRL /* kIKindMOVEMApRL */,
	DoCodeUnlkA6 /* kIKindUnlkA6 */,
	DoCodeRts /* kIKindRts */,
	DoCodeJmp /* kIKindJmp */,
	DoCodeClr /* kIKindClr */,
	DoCodeAddA /* kIKindAddA */,
	DoCodeAddA /* kIKindAddQA */,
	DoCodeSubA /* kIKindSubA */,
	DoCodeSubA /* kIKindSubQA */,
	DoCodeCmpA /* kIKindCmpA */,
	DoCodeAddXB /* kIKindAddXB */,
	DoCodeAddXW /* kIKindAddXW */,
	DoCodeAddXL /* kIKindAddXL */,
	DoCodeSubXB /* kIKindSubXB */,
	DoCodeSubXW /* kIKindSubXW */,
	DoCodeSubXL /* kIKindSubXL */,
	DoCodeAslB /* kIKindAslB */,
	DoCodeAslW /* kIKindAslW */,
	DoCodeAslL /* kIKindAslL */,
	DoCodeAsrB /* kIKindAsrB */,
	DoCodeAsrW /* kIKindAsrW */,
	DoCodeAsrL /* kIKindAsrL */,
	DoCodeLslB /* kIKindLslB */,
	DoCodeLslW /* kIKindLslW */,
	DoCodeLslL /* kIKindLslL */,
	DoCodeLsrB /* kIKindLsrB */,
	DoCodeLsrW /* kIKindLsrW */,
	DoCodeLsrL /* kIKindLsrL */,
	DoCodeRxlB /* kIKindRxlB */,
	DoCodeRxlW /* kIKindRxlW */,
	DoCodeRxlL /* kIKindRxlL */,
	DoCodeRxrB /* kIKindRxrB */,
	DoCodeRxrW /* kIKindRxrW */,
	DoCodeRxrL /* kIKindRxrL */,
	DoCodeRolB /* kIKindRolB */,
	DoCodeRolW /* kIKindRolW */,
	DoCodeRolL /* kIKindRolL */,
	DoCodeRorB /* kIKindRorB */,
	DoCodeRorW /* kIKindRorW */,
	DoCodeRorL /* kIKindRorL */,
	DoCodeBTstB /* kIKindBTstB */,
	DoCodeBChgB /* kIKindBChgB */,
	DoCodeBClrB /* kIKindBClrB */,
	DoCodeBSetB /* kIKindBSetB */,
	DoCodeBTstL /* kIKindBTstL */,
	DoCodeBChgL /* kIKindBChgL */,
	DoCodeBClrL /* kIKindBClrL */,
	DoCodeBSetL /* kIKindBSetL */,
	DoCodeAnd /* kIKindAndI */,
	DoCodeAnd /* kIKindAndEaD */,
	DoCodeAnd /* kIKindAndDEa */,
	DoCodeOr /* kIKindOrI */,
	DoCodeOr /* kIKindOrDEa */,
	DoCodeOr /* kIKindOrEaD */,
	DoCodeEor /* kIKindEor */,
	DoCodeEor /* kIKindEorI */,
	DoCodeNot /* kIKindNot */,
	DoCodeScc /* kIKindScc */,
	DoCodeNegXB /* kIKindNegXB */,
	DoCodeNegXW /* kIKindNegXW */,
	DoCodeNegXL /* kIKindNegXL */,
	DoCodeNegB /* kIKindNegB */,
	DoCodeNegW /* kIKindNegW */,
	DoCodeNegL /* kIKindNegL */,
	DoCodeEXTW /* kIKindEXTW */,
	DoCodeEXTL /* kIKindEXTL */,
	DoCodeMulU /* kIKindMulU */,
	DoCodeMulS /* kIKindMulS */,
	DoCodeDivU /* kIKindDivU */,
	DoCodeDivS /* kIKindDivS */,
	DoCodeExg /* kIKindExg */,
	DoCodeMoveEaCR /* kIKindMoveEaCCR */,
	DoCodeMoveSREa /* kIKindMoveSREa */,
	DoCodeMoveEaSR /* kIKindMoveEaSR */,
	DoCodeOrISR /* kIKindOrISR */,
	DoCodeAndISR /* kIKindAndISR */,
	DoCodeEorISR /* kIKindEorISR */,
	DoCodeOrICCR /* kIKindOrICCR */,
	DoCodeAndICCR /* kIKindAndICCR */,
	DoCodeEorICCR /* kIKindEorICCR */,
	DoCodeMOVEMApRW /* kIKindMOVEMApRW */,
	DoCodeMOVEMRmMW /* kIKindMOVEMRmMW */,
	DoCodeMOVEMrmW /* kIKindMOVEMrmW */,
	DoCodeMOVEMrmL /* kIKindMOVEMrmL */,
	DoCodeMOVEMmrW /* kIKindMOVEMmrW */,
	DoCodeMOVEMmrL /* kIKindMOVEMmrL */,
	DoCodeAbcd /* kIKindAbcd */,
	DoCodeSbcd /* kIKindSbcd */,
	DoCodeNbcd /* kIKindNbcd */,
	DoCodeRte /* kIKindRte */,
	DoCodeNop /* kIKindNop */,
	DoCodeMoveP0 /* kIKindMoveP0 */,
	DoCodeMoveP1 /* kIKindMoveP1 */,
	DoCodeMoveP2 /* kIKindMoveP2 */,
	DoCodeMoveP3 /* kIKindMoveP3 */,
	op_illg /* kIKindIllegal */,
	DoCodeChk /* kIKindChkW */,
	DoCodeTrap /* kIKindTrap */,
	DoCodeTrapV /* kIKindTrapV */,
	DoCodeRtr /* kIKindRtr */,
	DoCodeLink /* kIKindLink */,
	DoCodeUnlk /* kIKindUnlk */,
	DoCodeMoveRUSP /* kIKindMoveRUSP */,
	DoCodeMoveUSPR /* kIKindMoveUSPR */,
	DoCodeTas /* kIKindTas */,
	DoCodeFdefault /* kIKindFdflt */,
	DoCodeStop /* kIKindStop */,
	DoCodeReset /* kIKindReset */,

#if Use68020
	DoCodeCallMorRtm /* kIKindCallMorRtm */,
	DoCodeBraL /* kIKindBraL */,
	DoCodeBccL /* kIKindBccL */,
	DoCodeBsrL /* kIKindBsrL */,
	DoCodeEXTBL /* kIKindEXTBL */,
	DoCodeTRAPcc /* kIKindTRAPcc */,
	DoCodeChk /* kIKindChkL */,
	DoCodeBkpt /* kIKindBkpt */,
	DoCodeDivL /* kIKindDivL */,
	DoCodeMulL /* kIKindMulL */,
	DoCodeRtd /* kIKindRtd */,
	DoCodeMoveCCREa /* kIKindMoveCCREa */,
	DoMoveFromControl /* kIKindMoveCEa */,
	DoMoveToControl /* kIKindMoveEaC */,
	DoCodeLinkL /* kIKindLinkL */,
	DoCodePack /* kIKindPack */,
	DoCodeUnpk /* kIKindUnpk */,
	DoCHK2orCMP2 /* kIKindCHK2orCMP2 */,
	DoCAS2 /* kIKindCAS2 */,
	DoCAS /* kIKindCAS */,
	DoMOVES /* kIKindMoveS */,
	DoBitField /* kIKindBitField */,
#endif
#if EmMMU
	DoCodeMMU /* kIKindMMU */,
#endif
#if EmFPU
	DoCodeFPU_md60 /* kIKindFPUmd60 */,
	DoCodeFPU_DBcc /* kIKindFPUDBcc */,
	DoCodeFPU_Trapcc /* kIKindFPUTrapcc */,
	DoCodeFPU_Scc /* kIKindFPUScc */,
	DoCodeFPU_FBccW /* kIKindFPUFBccW */,
	DoCodeFPU_FBccL /* kIKindFPUFBccL */,
	DoCodeFPU_Save /* kIKindFPUSave */,
	DoCodeFPU_Restore /* kIKindFPURestore */,
	DoCodeFPU_dflt /* kIKindFPUdflt */,
#endif

	0
};

#ifndef WantBreakPoint
#define WantBreakPoint 0
#endif

#if WantBreakPoint

#define BreakPointAddress 0xD198

LOCALPROC BreakPointAction(void)
{
	dbglog_StartLine();
	dbglog_writeCStr("breakpoint A0=");
	dbglog_writeHex(m68k_areg(0));
	dbglog_writeCStr(" A1=");
	dbglog_writeHex(m68k_areg(1));
	dbglog_writeReturn();
}

#endif

LOCALINLINEPROC DecodeNextInstruction(func_pointer_t *d, uint16_t *Cycles,
	DecOpYR *y)
{
	uint32_t opcode;
	DecOpR *p;
	uint16_t MainClas;

	opcode = nextiword();

	p = &V_regs.disp_table[opcode];

#if WantCloserCyc
	V_regs.CurDecOp = p;
#endif
	MainClas = p->x.MainClas;
	*Cycles = p->x.Cycles;
	*y = p->y;
#if WantDumpTable
	DumpTable[MainClas] ++;
#endif
	*d = OpDispatch[MainClas];
}

LOCALINLINEPROC UnDecodeNextInstruction(uint16_t Cycles)
{
	V_MaxCyclesToGo += Cycles;

	BackupPC();

#if WantDumpTable
	{
		uint32_t opcode = do_get_mem_word(V_pc_p);
		DecOpR *p = &V_regs.disp_table[opcode];
		uint16_t MainClas = p->x.MainClas;

		DumpTable[MainClas] --;
	}
#endif
}

LOCALPROC m68k_go_MaxCycles(void)
{
	uint16_t Cycles;
	DecOpYR y;
	func_pointer_t d;

	/*
		Main loop of emulator.

		Always execute at least one instruction,
		even if V_regs.MaxInstructionsToGo == 0.
		Needed for trace flag to work.
	*/

	DecodeNextInstruction(&d, &Cycles, &y);

	V_MaxCyclesToGo -= Cycles;

	do {
		V_regs.CurDecOpY = y;

#if WantDisasm || WantBreakPoint
		{
			CPTR pc = m68k_getpc() - 2;
#if WantDisasm
			DisasmOneOrSave(pc);
#endif
#if WantBreakPoint
			if (BreakPointAddress == pc) {
				BreakPointAction();
			}
#endif
		}
#endif

		d();

		DecodeNextInstruction(&d, &Cycles, &y);

	} while (((int32_t)(V_MaxCyclesToGo -= Cycles)) > 0);

	/* abort instruction that have started to decode */

	UnDecodeNextInstruction(Cycles);
}

FORWARDFUNC uint32_t reg_call get_byte_ext(CPTR addr);

LOCALFUNC uint32_t reg_call get_byte(CPTR addr)
{
	uint8_t * m = (addr & V_regs.MATCrdB.usemask) + V_regs.MATCrdB.usebase;

	if ((addr & V_regs.MATCrdB.cmpmask) == V_regs.MATCrdB.cmpvalu) {
		return uint32_t_FromSByte(*m);
	} else {
		return get_byte_ext(addr);
	}
}

FORWARDPROC reg_call put_byte_ext(CPTR addr, uint32_t b);

LOCALPROC reg_call put_byte(CPTR addr, uint32_t b)
{
	uint8_t * m = (addr & V_regs.MATCwrB.usemask) + V_regs.MATCwrB.usebase;
	if ((addr & V_regs.MATCwrB.cmpmask) == V_regs.MATCwrB.cmpvalu) {
		*m = b;
	} else {
		put_byte_ext(addr, b);
	}
}

FORWARDFUNC uint32_t reg_call get_word_ext(CPTR addr);

LOCALFUNC uint32_t reg_call get_word(CPTR addr)
{
	uint8_t * m = (addr & V_regs.MATCrdW.usemask) + V_regs.MATCrdW.usebase;
	if ((addr & V_regs.MATCrdW.cmpmask) == V_regs.MATCrdW.cmpvalu) {
		return uint32_t_FromSWord(do_get_mem_word(m));
	} else {
		return get_word_ext(addr);
	}
}

FORWARDPROC reg_call put_word_ext(CPTR addr, uint32_t w);

LOCALPROC reg_call put_word(CPTR addr, uint32_t w)
{
	uint8_t * m = (addr & V_regs.MATCwrW.usemask) + V_regs.MATCwrW.usebase;
	if ((addr & V_regs.MATCwrW.cmpmask) == V_regs.MATCwrW.cmpvalu) {
		do_put_mem_word(m, w);
	} else {
		put_word_ext(addr, w);
	}
}

FORWARDFUNC uint32_t reg_call get_long_misaligned_ext(CPTR addr);

LOCALFUNC uint32_t reg_call get_long_misaligned(CPTR addr)
{
	CPTR addr2 = addr + 2;
	uint8_t * m = (addr & V_regs.MATCrdW.usemask) + V_regs.MATCrdW.usebase;
	uint8_t * m2 = (addr2 & V_regs.MATCrdW.usemask) + V_regs.MATCrdW.usebase;
	if (((addr & V_regs.MATCrdW.cmpmask) == V_regs.MATCrdW.cmpvalu)
		&& ((addr2 & V_regs.MATCrdW.cmpmask) == V_regs.MATCrdW.cmpvalu))
	{
		uint32_t hi = do_get_mem_word(m);
		uint32_t lo = do_get_mem_word(m2);
		uint32_t Data = ((hi << 16) & 0xFFFF0000)
			| (lo & 0x0000FFFF);

		return uint32_t_FromSLong(Data);
	} else {
		return get_long_misaligned_ext(addr);
	}
}

#if FasterAlignedL
FORWARDFUNC uint32_t reg_call get_long_ext(CPTR addr);
#endif

#if FasterAlignedL
LOCALFUNC uint32_t reg_call get_long(CPTR addr)
{
	if (0 == (addr & 0x03)) {
		uint8_t * m = (addr & V_regs.MATCrdL.usemask)
			+ V_regs.MATCrdL.usebase;
		if ((addr & V_regs.MATCrdL.cmpmask) == V_regs.MATCrdL.cmpvalu) {
			return uint32_t_FromSLong(do_get_mem_long(m));
		} else {
			return get_long_ext(addr);
		}
	} else {
		return get_long_misaligned(addr);
	}
}
#else
#define get_long get_long_misaligned
#endif

FORWARDPROC reg_call put_long_misaligned_ext(CPTR addr, uint32_t l);

LOCALPROC reg_call put_long_misaligned(CPTR addr, uint32_t l)
{
	CPTR addr2 = addr + 2;
	uint8_t * m = (addr & V_regs.MATCwrW.usemask) + V_regs.MATCwrW.usebase;
	uint8_t * m2 = (addr2 & V_regs.MATCwrW.usemask) + V_regs.MATCwrW.usebase;
	if (((addr & V_regs.MATCwrW.cmpmask) == V_regs.MATCwrW.cmpvalu)
		&& ((addr2 & V_regs.MATCwrW.cmpmask) == V_regs.MATCwrW.cmpvalu))
	{
		do_put_mem_word(m, l >> 16);
		do_put_mem_word(m2, l);
	} else {
		put_long_misaligned_ext(addr, l);
	}
}

#if FasterAlignedL
FORWARDPROC reg_call put_long_ext(CPTR addr, uint32_t l);
#endif

#if FasterAlignedL
LOCALPROC reg_call put_long(CPTR addr, uint32_t l)
{
	if (0 == (addr & 0x03)) {
		uint8_t * m = (addr & V_regs.MATCwrL.usemask)
			+ V_regs.MATCwrL.usebase;
		if ((addr & V_regs.MATCwrL.cmpmask) == V_regs.MATCwrL.cmpvalu) {
			do_put_mem_long(m, l);
		} else {
			put_long_ext(addr, l);
		}
	} else {
		put_long_misaligned(addr, l);
	}
}
#else
#define put_long put_long_misaligned
#endif

LOCALFUNC uint32_t reg_call get_disp_ea(uint32_t base)
{
	uint16_t dp = nextiword();
	int regno = (dp >> 12) & 0x0F;
	int32_t regd = V_regs.regs[regno];
	if ((dp & 0x0800) == 0) {
		regd = (int32_t)(int16_t)regd;
	}
#if Use68020
	regd <<= (dp >> 9) & 3;
#if ExtraAbnormalReports
	if (((dp >> 9) & 3) != 0) {
		/* ReportAbnormal("Have scale in Extension Word"); */
		/* apparently can happen in Sys 7.5.5 boot on 68000 */
	}
#endif
	if (dp & 0x0100) {
		if ((dp & 0x80) != 0) {
			base = 0;
			/* ReportAbnormal("Extension Word: suppress base"); */
			/* used by Sys 7.5.5 boot */
		}
		if ((dp & 0x40) != 0) {
			regd = 0;
			/* ReportAbnormal("Extension Word: suppress regd"); */
			/* used by Mac II boot */
		}

		switch ((dp >> 4) & 0x03) {
			case 0:
				/* reserved */
				ReportAbnormalID(0x0101, "Extension Word: dp reserved");
				break;
			case 1:
				/* no displacement */
				/* ReportAbnormal("Extension Word: no displacement"); */
				/* used by Sys 7.5.5 boot */
				break;
			case 2:
				base += nextiSWord();
				/*
					ReportAbnormal("Extension Word: word displacement");
				*/
				/* used by Sys 7.5.5 boot */
				break;
			case 3:
				base += nextilong();
				/*
					ReportAbnormal("Extension Word: long displacement");
				*/
				/* used by Mac II boot from system 6.0.8? */
				break;
		}

		if ((dp & 0x03) == 0) {
			base += regd;
			if ((dp & 0x04) != 0) {
				ReportAbnormalID(0x0102,
					"Extension Word: reserved dp form");
			}
			/* ReportAbnormal("Extension Word: noindex"); */
			/* used by Sys 7.5.5 boot */
		} else {
			if ((dp & 0x04) != 0) {
				base = get_long(base);
				base += regd;
				/* ReportAbnormal("Extension Word: postindex"); */
				/* used by Sys 7.5.5 boot */
			} else {
				base += regd;
				base = get_long(base);
				/* ReportAbnormal("Extension Word: preindex"); */
				/* used by Sys 7.5.5 boot */
			}
			switch (dp & 0x03) {
				case 1:
					/* null outer displacement */
					/*
						ReportAbnormal(
							"Extension Word: null outer displacement");
					*/
					/* used by Sys 7.5.5 boot */
					break;
				case 2:
					base += nextiSWord();
					/*
						ReportAbnormal(
							"Extension Word: word outer displacement");
					*/
					/* used by Mac II boot from system 6.0.8? */
					break;
				case 3:
					base += nextilong();
					/*
						ReportAbnormal(
							"Extension Word: long outer displacement");
					*/
					/* used by Mac II boot from system 6.0.8? */
					break;
			}
		}

		return base;
	} else
#endif
	{
		return base + (int8_t)(dp) + regd;
	}
}

LOCALFUNC uint32_t reg_call DecodeAddr_Indirect(uint8_t ArgDat)
{
	return V_regs.regs[ArgDat];
}

LOCALFUNC uint32_t reg_call DecodeAddr_APosIncB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 1;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_APosIncW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_APosIncL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 4;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_APreDecB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 1;

	*p = a;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_APreDecW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_APreDecL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 4;

	*p = a;

	return a;
}

LOCALFUNC uint32_t reg_call DecodeAddr_ADisp(uint8_t ArgDat)
{
	return V_regs.regs[ArgDat] + nextiSWord();
}

LOCALFUNC uint32_t reg_call DecodeAddr_AIndex(uint8_t ArgDat)
{
	return get_disp_ea(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeAddr_AbsW(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return nextiSWord();
}

LOCALFUNC uint32_t reg_call DecodeAddr_AbsL(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return nextilong();
}

LOCALFUNC uint32_t reg_call DecodeAddr_PCDisp(uint8_t ArgDat)
{
	CPTR pc = m68k_getpc();

	UnusedParam(ArgDat);
	return pc + nextiSWord();
}

LOCALFUNC uint32_t reg_call DecodeAddr_PCIndex(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return get_disp_ea(m68k_getpc());
}

typedef uint32_t (reg_call *DecodeAddrP)(uint8_t ArgDat);

LOCALVAR const DecodeAddrP DecodeAddrDispatch[kNumAMds] = {
	(DecodeAddrP)nullpr /* kAMdRegB */,
	(DecodeAddrP)nullpr /* kAMdRegW */,
	(DecodeAddrP)nullpr /* kAMdRegL */,
	DecodeAddr_Indirect /* kAMdIndirectB */,
	DecodeAddr_Indirect /* kAMdIndirectW */,
	DecodeAddr_Indirect /* kAMdIndirectL */,
	DecodeAddr_APosIncB /* kAMdAPosIncB */,
	DecodeAddr_APosIncW /* kAMdAPosIncW */,
	DecodeAddr_APosIncL /* kAMdAPosIncL */,
	DecodeAddr_APosIncW /* kAMdAPosInc7B */,
	DecodeAddr_APreDecB /* kAMdAPreDecB */,
	DecodeAddr_APreDecW /* kAMdAPreDecW */,
	DecodeAddr_APreDecL /* kAMdAPreDecL */,
	DecodeAddr_APreDecW /* kAMdAPreDec7B */,
	DecodeAddr_ADisp /* kAMdADispB */,
	DecodeAddr_ADisp /* kAMdADispW */,
	DecodeAddr_ADisp /* kAMdADispL */,
	DecodeAddr_AIndex /* kAMdAIndexB */,
	DecodeAddr_AIndex /* kAMdAIndexW */,
	DecodeAddr_AIndex /* kAMdAIndexL */,
	DecodeAddr_AbsW /* kAMdAbsWB */,
	DecodeAddr_AbsW /* kAMdAbsWW */,
	DecodeAddr_AbsW /* kAMdAbsWL */,
	DecodeAddr_AbsL /* kAMdAbsLB */,
	DecodeAddr_AbsL /* kAMdAbsLW */,
	DecodeAddr_AbsL /* kAMdAbsLL */,
	DecodeAddr_PCDisp /* kAMdPCDispB */,
	DecodeAddr_PCDisp /* kAMdPCDispW */,
	DecodeAddr_PCDisp /* kAMdPCDispL */,
	DecodeAddr_PCIndex /* kAMdPCIndexB */,
	DecodeAddr_PCIndex /* kAMdPCIndexW */,
	DecodeAddr_PCIndex /* kAMdPCIndexL */,
	(DecodeAddrP)nullpr /* kAMdImmedB */,
	(DecodeAddrP)nullpr /* kAMdImmedW */,
	(DecodeAddrP)nullpr /* kAMdImmedL */,
	(DecodeAddrP)nullpr /* kAMdDat4 */
};

LOCALINLINEFUNC uint32_t DecodeAddrSrcDst(DecArgR *f)
{
	return (DecodeAddrDispatch[f->AMd])(f->ArgDat);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_RegB(uint8_t ArgDat)
{
	return uint32_t_FromSByte(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_RegW(uint8_t ArgDat)
{
	return uint32_t_FromSWord(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_RegL(uint8_t ArgDat)
{
	return uint32_t_FromSLong(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_IndirectB(uint8_t ArgDat)
{
	return get_byte(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_IndirectW(uint8_t ArgDat)
{
	return get_word(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_IndirectL(uint8_t ArgDat)
{
	return get_long(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APosIncB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 1;

	return get_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APosIncW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	return get_word(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APosIncL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 4;

	return get_long(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APosInc7B(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	return get_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APreDecB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 1;

	*p = a;

	return get_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APreDecW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	return get_word(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APreDecL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 4;

	*p = a;

	return get_long(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_APreDec7B(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	return get_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ADispB(uint8_t ArgDat)
{
	return get_byte(DecodeAddr_ADisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ADispW(uint8_t ArgDat)
{
	return get_word(DecodeAddr_ADisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ADispL(uint8_t ArgDat)
{
	return get_long(DecodeAddr_ADisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AIndexB(uint8_t ArgDat)
{
	return get_byte(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AIndexW(uint8_t ArgDat)
{
	return get_word(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AIndexL(uint8_t ArgDat)
{
	return get_long(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsWB(uint8_t ArgDat)
{
	return get_byte(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsWW(uint8_t ArgDat)
{
	return get_word(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsWL(uint8_t ArgDat)
{
	return get_long(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsLB(uint8_t ArgDat)
{
	return get_byte(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsLW(uint8_t ArgDat)
{
	return get_word(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_AbsLL(uint8_t ArgDat)
{
	return get_long(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCDispB(uint8_t ArgDat)
{
	return get_byte(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCDispW(uint8_t ArgDat)
{
	return get_word(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCDispL(uint8_t ArgDat)
{
	return get_long(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCIndexB(uint8_t ArgDat)
{
	return get_byte(DecodeAddr_PCIndex(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCIndexW(uint8_t ArgDat)
{
	return get_word(DecodeAddr_PCIndex(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_PCIndexL(uint8_t ArgDat)
{
	return get_long(DecodeAddr_PCIndex(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ImmedB(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return nextiSByte();
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ImmedW(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return nextiSWord();
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_ImmedL(uint8_t ArgDat)
{
	UnusedParam(ArgDat);
	return uint32_t_FromSLong(nextilong());
}

LOCALFUNC uint32_t reg_call DecodeGetSrcDst_Dat4(uint8_t ArgDat)
{
	return ArgDat;
}

typedef uint32_t (reg_call *DecodeGetSrcDstP)(uint8_t ArgDat);

LOCALVAR const DecodeGetSrcDstP DecodeGetSrcDstDispatch[kNumAMds] = {
	DecodeGetSrcDst_RegB /* kAMdRegB */,
	DecodeGetSrcDst_RegW /* kAMdRegW */,
	DecodeGetSrcDst_RegL /* kAMdRegL */,
	DecodeGetSrcDst_IndirectB /* kAMdIndirectB */,
	DecodeGetSrcDst_IndirectW /* kAMdIndirectW */,
	DecodeGetSrcDst_IndirectL /* kAMdIndirectL */,
	DecodeGetSrcDst_APosIncB /* kAMdAPosIncB */,
	DecodeGetSrcDst_APosIncW /* kAMdAPosIncW */,
	DecodeGetSrcDst_APosIncL /* kAMdAPosIncL */,
	DecodeGetSrcDst_APosInc7B /* kAMdAPosInc7B */,
	DecodeGetSrcDst_APreDecB /* kAMdAPreDecB */,
	DecodeGetSrcDst_APreDecW /* kAMdAPreDecW */,
	DecodeGetSrcDst_APreDecL /* kAMdAPreDecL */,
	DecodeGetSrcDst_APreDec7B /* kAMdAPreDec7B */,
	DecodeGetSrcDst_ADispB /* kAMdADispB */,
	DecodeGetSrcDst_ADispW /* kAMdADispW */,
	DecodeGetSrcDst_ADispL /* kAMdADispL */,
	DecodeGetSrcDst_AIndexB /* kAMdAIndexB */,
	DecodeGetSrcDst_AIndexW /* kAMdAIndexW */,
	DecodeGetSrcDst_AIndexL /* kAMdAIndexL */,
	DecodeGetSrcDst_AbsWB /* kAMdAbsWB */,
	DecodeGetSrcDst_AbsWW /* kAMdAbsWW */,
	DecodeGetSrcDst_AbsWL /* kAMdAbsWL */,
	DecodeGetSrcDst_AbsLB /* kAMdAbsLB */,
	DecodeGetSrcDst_AbsLW /* kAMdAbsLW */,
	DecodeGetSrcDst_AbsLL /* kAMdAbsLL */,
	DecodeGetSrcDst_PCDispB /* kAMdPCDispB */,
	DecodeGetSrcDst_PCDispW /* kAMdPCDispW */,
	DecodeGetSrcDst_PCDispL /* kAMdPCDispL */,
	DecodeGetSrcDst_PCIndexB /* kAMdPCIndexB */,
	DecodeGetSrcDst_PCIndexW /* kAMdPCIndexW */,
	DecodeGetSrcDst_PCIndexL /* kAMdPCIndexL */,
	DecodeGetSrcDst_ImmedB /* kAMdImmedB */,
	DecodeGetSrcDst_ImmedW /* kAMdImmedW */,
	DecodeGetSrcDst_ImmedL /* kAMdImmedL */,
	DecodeGetSrcDst_Dat4 /* kAMdDat4 */
};

LOCALINLINEFUNC uint32_t DecodeGetSrcDst(DecArgR *f)
{
	return (DecodeGetSrcDstDispatch[f->AMd])(f->ArgDat);
}

LOCALPROC reg_call DecodeSetSrcDst_RegB(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];

#if LittleEndianUnaligned
	*(uint8_t *)p = v;
#else
	*p = (*p & ~ 0xff) | ((v) & 0xff);
#endif
}

LOCALPROC reg_call DecodeSetSrcDst_RegW(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];

#if LittleEndianUnaligned
	*(uint16_t *)p = v;
#else
	*p = (*p & ~ 0xffff) | ((v) & 0xffff);
#endif
}

LOCALPROC reg_call DecodeSetSrcDst_RegL(uint32_t v, uint8_t ArgDat)
{
	V_regs.regs[ArgDat] = v;
}

LOCALPROC reg_call DecodeSetSrcDst_IndirectB(uint32_t v, uint8_t ArgDat)
{
	put_byte(V_regs.regs[ArgDat], v);
}

LOCALPROC reg_call DecodeSetSrcDst_IndirectW(uint32_t v, uint8_t ArgDat)
{
	put_word(V_regs.regs[ArgDat], v);
}

LOCALPROC reg_call DecodeSetSrcDst_IndirectL(uint32_t v, uint8_t ArgDat)
{
	put_long(V_regs.regs[ArgDat], v);
}

LOCALPROC reg_call DecodeSetSrcDst_APosIncB(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 1;

	put_byte(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APosIncW(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	put_word(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APosIncL(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 4;

	put_long(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APosInc7B(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	put_byte(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APreDecB(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 1;

	*p = a;

	put_byte(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APreDecW(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	put_word(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APreDecL(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 4;

	*p = a;

	put_long(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_APreDec7B(uint32_t v, uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	put_byte(a, v);
}

LOCALPROC reg_call DecodeSetSrcDst_ADispB(uint32_t v, uint8_t ArgDat)
{
	put_byte(V_regs.regs[ArgDat]
		+ nextiSWord(), v);
}

LOCALPROC reg_call DecodeSetSrcDst_ADispW(uint32_t v, uint8_t ArgDat)
{
	put_word(V_regs.regs[ArgDat]
		+ nextiSWord(), v);
}

LOCALPROC reg_call DecodeSetSrcDst_ADispL(uint32_t v, uint8_t ArgDat)
{
	put_long(V_regs.regs[ArgDat]
		+ nextiSWord(), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AIndexB(uint32_t v, uint8_t ArgDat)
{
	put_byte(get_disp_ea(V_regs.regs[ArgDat]), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AIndexW(uint32_t v, uint8_t ArgDat)
{
	put_word(get_disp_ea(V_regs.regs[ArgDat]), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AIndexL(uint32_t v, uint8_t ArgDat)
{
	put_long(get_disp_ea(V_regs.regs[ArgDat]), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsWB(uint32_t v, uint8_t ArgDat)
{
	put_byte(DecodeAddr_AbsW(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsWW(uint32_t v, uint8_t ArgDat)
{
	put_word(DecodeAddr_AbsW(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsWL(uint32_t v, uint8_t ArgDat)
{
	put_long(DecodeAddr_AbsW(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsLB(uint32_t v, uint8_t ArgDat)
{
	put_byte(DecodeAddr_AbsL(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsLW(uint32_t v, uint8_t ArgDat)
{
	put_word(DecodeAddr_AbsL(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_AbsLL(uint32_t v, uint8_t ArgDat)
{
	put_long(DecodeAddr_AbsL(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCDispB(uint32_t v, uint8_t ArgDat)
{
	put_byte(DecodeAddr_PCDisp(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCDispW(uint32_t v, uint8_t ArgDat)
{
	put_word(DecodeAddr_PCDisp(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCDispL(uint32_t v, uint8_t ArgDat)
{
	put_long(DecodeAddr_PCDisp(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCIndexB(uint32_t v, uint8_t ArgDat)
{
	put_byte(DecodeAddr_PCIndex(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCIndexW(uint32_t v, uint8_t ArgDat)
{
	put_word(DecodeAddr_PCIndex(ArgDat), v);
}

LOCALPROC reg_call DecodeSetSrcDst_PCIndexL(uint32_t v, uint8_t ArgDat)
{
	put_long(DecodeAddr_PCIndex(ArgDat), v);
}

typedef void (reg_call *DecodeSetSrcDstP)(uint32_t v, uint8_t ArgDat);

LOCALVAR const DecodeSetSrcDstP DecodeSetSrcDstDispatch[kNumAMds] = {
	DecodeSetSrcDst_RegB /* kAMdRegB */,
	DecodeSetSrcDst_RegW /* kAMdRegW */,
	DecodeSetSrcDst_RegL /* kAMdRegL */,
	DecodeSetSrcDst_IndirectB /* kAMdIndirectB */,
	DecodeSetSrcDst_IndirectW /* kAMdIndirectW */,
	DecodeSetSrcDst_IndirectL /* kAMdIndirectL*/,
	DecodeSetSrcDst_APosIncB /* kAMdAPosIncB */,
	DecodeSetSrcDst_APosIncW /* kAMdAPosIncW */,
	DecodeSetSrcDst_APosIncL /* kAMdAPosIncL */,
	DecodeSetSrcDst_APosInc7B /* kAMdAPosInc7B */,
	DecodeSetSrcDst_APreDecB /* kAMdAPreDecB */,
	DecodeSetSrcDst_APreDecW /* kAMdAPreDecW */,
	DecodeSetSrcDst_APreDecL /* kAMdAPreDecL */,
	DecodeSetSrcDst_APreDec7B /* kAMdAPreDec7B */,
	DecodeSetSrcDst_ADispB /* kAMdADispB */,
	DecodeSetSrcDst_ADispW /* kAMdADispW */,
	DecodeSetSrcDst_ADispL /* kAMdADispL */,
	DecodeSetSrcDst_AIndexB /* kAMdAIndexB */,
	DecodeSetSrcDst_AIndexW /* kAMdAIndexW */,
	DecodeSetSrcDst_AIndexL /* kAMdAIndexL */,
	DecodeSetSrcDst_AbsWB /* kAMdAbsWB */,
	DecodeSetSrcDst_AbsWW /* kAMdAbsWW */,
	DecodeSetSrcDst_AbsWL /* kAMdAbsWL */,
	DecodeSetSrcDst_AbsLB /* kAMdAbsLB */,
	DecodeSetSrcDst_AbsLW /* kAMdAbsLW */,
	DecodeSetSrcDst_AbsLL /* kAMdAbsLL */,
	DecodeSetSrcDst_PCDispB /* kAMdPCDispB */,
	DecodeSetSrcDst_PCDispW /* kAMdPCDispW */,
	DecodeSetSrcDst_PCDispL /* kAMdPCDispL */,
	DecodeSetSrcDst_PCIndexB /* kAMdPCIndexB */,
	DecodeSetSrcDst_PCIndexW /* kAMdPCIndexW */,
	DecodeSetSrcDst_PCIndexL /* kAMdPCIndexL */,
	(DecodeSetSrcDstP)nullpr /* kAMdImmedB */,
	(DecodeSetSrcDstP)nullpr /* kAMdImmedW */,
	(DecodeSetSrcDstP)nullpr /* kAMdImmedL */,
	(DecodeSetSrcDstP)nullpr /* kAMdDat4 */
};

LOCALINLINEPROC DecodeSetSrcDst(uint32_t v, DecArgR *f)
{
	(DecodeSetSrcDstDispatch[f->AMd])(v, f->ArgDat);
}

LOCALPROC reg_call ArgSetDstRegBValue(uint32_t v)
{
	uint32_t *p = V_regs.ArgAddr.rga;

#if LittleEndianUnaligned
	*(uint8_t *)p = v;
#else
	*p = (*p & ~ 0xff) | ((v) & 0xff);
#endif
}

LOCALPROC reg_call ArgSetDstRegWValue(uint32_t v)
{
	uint32_t *p = V_regs.ArgAddr.rga;

#if LittleEndianUnaligned
	*(uint16_t *)p = v;
#else
	*p = (*p & ~ 0xffff) | ((v) & 0xffff);
#endif
}

LOCALPROC reg_call ArgSetDstRegLValue(uint32_t v)
{
	uint32_t *p = V_regs.ArgAddr.rga;

	*p = v;
}

LOCALPROC reg_call ArgSetDstMemBValue(uint32_t v)
{
	put_byte(V_regs.ArgAddr.mem, v);
}

LOCALPROC reg_call ArgSetDstMemWValue(uint32_t v)
{
	put_word(V_regs.ArgAddr.mem, v);
}

LOCALPROC reg_call ArgSetDstMemLValue(uint32_t v)
{
	put_long(V_regs.ArgAddr.mem, v);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_RegB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];

	V_regs.ArgAddr.rga = p;
	V_regs.ArgSetDst = ArgSetDstRegBValue;

	return uint32_t_FromSByte(*p);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_RegW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];

	V_regs.ArgAddr.rga = p;
	V_regs.ArgSetDst = ArgSetDstRegWValue;

	return uint32_t_FromSWord(*p);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_RegL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];

	V_regs.ArgAddr.rga = p;
	V_regs.ArgSetDst = ArgSetDstRegLValue;

	return uint32_t_FromSLong(*p);
}

LOCALFUNC uint32_t reg_call getarg_byte(uint32_t a)
{
	V_regs.ArgAddr.mem = a;
	V_regs.ArgSetDst = ArgSetDstMemBValue;

	return get_byte(a);
}

LOCALFUNC uint32_t reg_call getarg_word(uint32_t a)
{
	V_regs.ArgAddr.mem = a;
	V_regs.ArgSetDst = ArgSetDstMemWValue;

	return get_word(a);
}

LOCALFUNC uint32_t reg_call getarg_long(uint32_t a)
{
	V_regs.ArgAddr.mem = a;
	V_regs.ArgSetDst = ArgSetDstMemLValue;

	return get_long(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_IndirectB(uint8_t ArgDat)
{
	return getarg_byte(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_IndirectW(uint8_t ArgDat)
{
	return getarg_word(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_IndirectL(uint8_t ArgDat)
{
	return getarg_long(V_regs.regs[ArgDat]);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APosIncB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 1;

	return getarg_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APosIncW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	return getarg_word(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APosIncL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 4;

	return getarg_long(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APosInc7B(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p;

	*p = a + 2;

	return getarg_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APreDecB(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 1;

	*p = a;

	return getarg_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APreDecW(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	return getarg_word(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APreDecL(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 4;

	*p = a;

	return getarg_long(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_APreDec7B(uint8_t ArgDat)
{
	uint32_t *p = &V_regs.regs[ArgDat];
	uint32_t a = *p - 2;

	*p = a;

	return getarg_byte(a);
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_ADispB(uint8_t ArgDat)
{
	return getarg_byte(V_regs.regs[ArgDat]
		+ nextiSWord());
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_ADispW(uint8_t ArgDat)
{
	return getarg_word(V_regs.regs[ArgDat]
		+ nextiSWord());
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_ADispL(uint8_t ArgDat)
{
	return getarg_long(V_regs.regs[ArgDat]
		+ nextiSWord());
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AIndexB(uint8_t ArgDat)
{
	return getarg_byte(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AIndexW(uint8_t ArgDat)
{
	return getarg_word(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AIndexL(uint8_t ArgDat)
{
	return getarg_long(get_disp_ea(V_regs.regs[ArgDat]));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsWB(uint8_t ArgDat)
{
	return getarg_byte(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsWW(uint8_t ArgDat)
{
	return getarg_word(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsWL(uint8_t ArgDat)
{
	return getarg_long(DecodeAddr_AbsW(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsLB(uint8_t ArgDat)
{
	return getarg_byte(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsLW(uint8_t ArgDat)
{
	return getarg_word(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_AbsLL(uint8_t ArgDat)
{
	return getarg_long(DecodeAddr_AbsL(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCDispB(uint8_t ArgDat)
{
	return getarg_byte(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCDispW(uint8_t ArgDat)
{
	return getarg_word(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCDispL(uint8_t ArgDat)
{
	return getarg_long(DecodeAddr_PCDisp(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCIndexB(uint8_t ArgDat)
{
	return getarg_byte(DecodeAddr_PCIndex(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCIndexW(uint8_t ArgDat)
{
	return getarg_word(DecodeAddr_PCIndex(ArgDat));
}

LOCALFUNC uint32_t reg_call DecodeGetSetSrcDst_PCIndexL(uint8_t ArgDat)
{
	return getarg_long(DecodeAddr_PCIndex(ArgDat));
}

typedef uint32_t (reg_call *DecodeGetSetSrcDstP)(uint8_t ArgDat);

LOCALVAR const DecodeGetSetSrcDstP
	DecodeGetSetSrcDstDispatch[kNumAMds] =
{
	DecodeGetSetSrcDst_RegB /* kAMdRegB */,
	DecodeGetSetSrcDst_RegW /* kAMdRegW */,
	DecodeGetSetSrcDst_RegL /* kAMdRegL */,
	DecodeGetSetSrcDst_IndirectB /* kAMdIndirectB */,
	DecodeGetSetSrcDst_IndirectW /* kAMdIndirectW */,
	DecodeGetSetSrcDst_IndirectL /* kAMdIndirectL*/,
	DecodeGetSetSrcDst_APosIncB /* kAMdAPosIncB */,
	DecodeGetSetSrcDst_APosIncW /* kAMdAPosIncW */,
	DecodeGetSetSrcDst_APosIncL /* kAMdAPosIncL */,
	DecodeGetSetSrcDst_APosInc7B /* kAMdAPosInc7B */,
	DecodeGetSetSrcDst_APreDecB /* kAMdAPreDecB */,
	DecodeGetSetSrcDst_APreDecW /* kAMdAPreDecW */,
	DecodeGetSetSrcDst_APreDecL /* kAMdAPreDecL */,
	DecodeGetSetSrcDst_APreDec7B /* kAMdAPreDec7B */,
	DecodeGetSetSrcDst_ADispB /* kAMdADispB */,
	DecodeGetSetSrcDst_ADispW /* kAMdADispW */,
	DecodeGetSetSrcDst_ADispL /* kAMdADispL */,
	DecodeGetSetSrcDst_AIndexB /* kAMdAIndexB */,
	DecodeGetSetSrcDst_AIndexW /* kAMdAIndexW */,
	DecodeGetSetSrcDst_AIndexL /* kAMdAIndexL */,
	DecodeGetSetSrcDst_AbsWB /* kAMdAbsWB */,
	DecodeGetSetSrcDst_AbsWW /* kAMdAbsWW */,
	DecodeGetSetSrcDst_AbsWL /* kAMdAbsWL */,
	DecodeGetSetSrcDst_AbsLB /* kAMdAbsLB */,
	DecodeGetSetSrcDst_AbsLW /* kAMdAbsLW */,
	DecodeGetSetSrcDst_AbsLL /* kAMdAbsLL */,
	DecodeGetSetSrcDst_PCDispB /* kAMdPCDispB */,
	DecodeGetSetSrcDst_PCDispW /* kAMdPCDispW */,
	DecodeGetSetSrcDst_PCDispL /* kAMdPCDispL */,
	DecodeGetSetSrcDst_PCIndexB /* kAMdPCIndexB */,
	DecodeGetSetSrcDst_PCIndexW /* kAMdPCIndexW */,
	DecodeGetSetSrcDst_PCIndexL /* kAMdPCIndexL */,
	(DecodeGetSetSrcDstP)nullpr /* kAMdImmedB */,
	(DecodeGetSetSrcDstP)nullpr /* kAMdImmedW */,
	(DecodeGetSetSrcDstP)nullpr /* kAMdImmedL */,
	(DecodeGetSetSrcDstP)nullpr /* kAMdDat4 */
};

LOCALINLINEFUNC uint32_t DecodeGetSetSrcDst(DecArgR *f)
{
	return (DecodeGetSetSrcDstDispatch[f->AMd])(f->ArgDat);
}


LOCALINLINEFUNC uint32_t DecodeDst(void)
{
	return DecodeAddrSrcDst(&V_regs.CurDecOpY.v[1]);
}

LOCALINLINEFUNC uint32_t DecodeGetSetDstValue(void)
{
	return DecodeGetSetSrcDst(&V_regs.CurDecOpY.v[1]);
}

LOCALINLINEPROC ArgSetDstValue(uint32_t v)
{
	V_regs.ArgSetDst(v);
}

LOCALINLINEPROC DecodeSetDstValue(uint32_t v)
{
	DecodeSetSrcDst(v, &V_regs.CurDecOpY.v[1]);
}

LOCALINLINEFUNC uint32_t DecodeGetSrcValue(void)
{
	return DecodeGetSrcDst(&V_regs.CurDecOpY.v[0]);
}

LOCALINLINEFUNC uint32_t DecodeGetDstValue(void)
{
	return DecodeGetSrcDst(&V_regs.CurDecOpY.v[1]);
}

LOCALINLINEFUNC uint32_t DecodeGetSrcSetDstValue(void)
{
	V_regs.SrcVal = DecodeGetSrcValue();

	return DecodeGetSetDstValue();
}

LOCALINLINEFUNC uint32_t DecodeGetSrcGetDstValue(void)
{
	V_regs.SrcVal = DecodeGetSrcValue();

	return DecodeGetDstValue();
}


typedef void (*cond_actP)(void);

LOCALPROC reg_call cctrue_T(cond_actP t_act, cond_actP f_act)
{
	UnusedParam(f_act);
	t_act();
}

LOCALPROC reg_call cctrue_F(cond_actP t_act, cond_actP f_act)
{
	UnusedParam(t_act);
	f_act();
}

LOCALPROC reg_call cctrue_HI(cond_actP t_act, cond_actP f_act)
{
	if (0 == (CFLG | ZFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_LS(cond_actP t_act, cond_actP f_act)
{
	if (0 != (CFLG | ZFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CC(cond_actP t_act, cond_actP f_act)
{
	if (0 == (CFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CS(cond_actP t_act, cond_actP f_act)
{
	if (0 != (CFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_NE(cond_actP t_act, cond_actP f_act)
{
	if (0 == (ZFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_EQ(cond_actP t_act, cond_actP f_act)
{
	if (0 != (ZFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_VC(cond_actP t_act, cond_actP f_act)
{
	if (0 == (VFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_VS(cond_actP t_act, cond_actP f_act)
{
	if (0 != (VFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_PL(cond_actP t_act, cond_actP f_act)
{
	if (0 == (NFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_MI(cond_actP t_act, cond_actP f_act)
{
	if (0 != (NFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_GE(cond_actP t_act, cond_actP f_act)
{
	if (0 == (NFLG ^ VFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_LT(cond_actP t_act, cond_actP f_act)
{
	if (0 != (NFLG ^ VFLG)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_GT(cond_actP t_act, cond_actP f_act)
{
	if (0 == (ZFLG | (NFLG ^ VFLG))) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_LE(cond_actP t_act, cond_actP f_act)
{
	if (0 != (ZFLG | (NFLG ^ VFLG))) {
		t_act();
	} else {
		f_act();
	}
}

#if Have_ASR
#define Ui5rASR(x, s) ((uint32_t)(((int32_t)(x)) >> (s)))
#else
LOCALFUNC uint32_t Ui5rASR(uint32_t x, uint32_t s)
{
	uint32_t v;

	if (uint32_t_MSBisSet(x)) {
		v = ~ ((~ x) >> s);
	} else {
		v = x >> s;
	}

	return v;
}
#endif

#if UseLazyCC

LOCALPROC reg_call cctrue_TstL_HI(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) > ((uint32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_LS(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) <= ((uint32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

#if 0 /* always true */
LOCALPROC reg_call cctrue_TstL_CC(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) >= ((uint32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}
#endif

#if 0 /* always false */
LOCALPROC reg_call cctrue_TstL_CS(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) < ((uint32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}
#endif

LOCALPROC reg_call cctrue_TstL_NE(cond_actP t_act, cond_actP f_act)
{
	if (V_regs.LazyFlagArgDst != 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_EQ(cond_actP t_act, cond_actP f_act)
{
	if (V_regs.LazyFlagArgDst == 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_PL(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)(V_regs.LazyFlagArgDst)) >= 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_MI(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)(V_regs.LazyFlagArgDst)) < 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_GE(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) >= ((int32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_LT(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) < ((int32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_GT(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) > ((int32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_TstL_LE(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) <= ((int32_t)0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_HI(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) > ((uint8_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_LS(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) <= ((uint8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_CC(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) >= ((uint8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_CS(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) < ((uint8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_NE(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) != ((uint8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_EQ(cond_actP t_act, cond_actP f_act)
{
	if (((uint8_t)V_regs.LazyFlagArgDst) == ((uint8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_PL(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) >= 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_MI(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) < 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_GE(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)V_regs.LazyFlagArgDst) >= ((int8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_LT(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)V_regs.LazyFlagArgDst) < ((int8_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_GT(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)V_regs.LazyFlagArgDst) > ((int8_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpB_LE(cond_actP t_act, cond_actP f_act)
{
	if (((int8_t)V_regs.LazyFlagArgDst) <= ((int8_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_HI(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) > ((uint16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_LS(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) <= ((uint16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_CC(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) >= ((uint16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_CS(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) < ((uint16_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_NE(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) != ((uint16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_EQ(cond_actP t_act, cond_actP f_act)
{
	if (((uint16_t)V_regs.LazyFlagArgDst) == ((uint16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_PL(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) >= 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_MI(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) < 0) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_GE(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)V_regs.LazyFlagArgDst) >= ((int16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_LT(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)V_regs.LazyFlagArgDst) < ((int16_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_GT(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)V_regs.LazyFlagArgDst) > ((int16_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpW_LE(cond_actP t_act, cond_actP f_act)
{
	if (((int16_t)V_regs.LazyFlagArgDst) <= ((int16_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_HI(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) > ((uint32_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_LS(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) <= ((uint32_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_CC(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) >= ((uint32_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_CS(cond_actP t_act, cond_actP f_act)
{
	if (((uint32_t)V_regs.LazyFlagArgDst) < ((uint32_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_NE(cond_actP t_act, cond_actP f_act)
{
	if (V_regs.LazyFlagArgDst != V_regs.LazyFlagArgSrc) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_EQ(cond_actP t_act, cond_actP f_act)
{
	if (V_regs.LazyFlagArgDst == V_regs.LazyFlagArgSrc) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_PL(cond_actP t_act, cond_actP f_act)
{
	if ((((int32_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) >= 0))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_MI(cond_actP t_act, cond_actP f_act)
{
	if ((((int32_t)(V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc)) < 0)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_GE(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) >= ((int32_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_LT(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) < ((int32_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_GT(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) > ((int32_t)V_regs.LazyFlagArgSrc)) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_CmpL_LE(cond_actP t_act, cond_actP f_act)
{
	if (((int32_t)V_regs.LazyFlagArgDst) <= ((int32_t)V_regs.LazyFlagArgSrc))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_Asr_CC(cond_actP t_act, cond_actP f_act)
{
	if (0 ==
		((V_regs.LazyFlagArgDst >> (V_regs.LazyFlagArgSrc - 1)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_Asr_CS(cond_actP t_act, cond_actP f_act)
{
	if (0 !=
		((V_regs.LazyFlagArgDst >> (V_regs.LazyFlagArgSrc - 1)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslB_CC(cond_actP t_act, cond_actP f_act)
{
	if (0 ==
		((V_regs.LazyFlagArgDst >> (8 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslB_CS(cond_actP t_act, cond_actP f_act)
{
	if (0 !=
		((V_regs.LazyFlagArgDst >> (8 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslB_VC(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSByte(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) == V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslB_VS(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSByte(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) != V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslW_CC(cond_actP t_act, cond_actP f_act)
{
	if (0 ==
		((V_regs.LazyFlagArgDst >> (16 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslW_CS(cond_actP t_act, cond_actP f_act)
{
	if (0 !=
		((V_regs.LazyFlagArgDst >> (16 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslW_VC(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSWord(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) == V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslW_VS(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSWord(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) != V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslL_CC(cond_actP t_act, cond_actP f_act)
{
	if (0 ==
		((V_regs.LazyFlagArgDst >> (32 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslL_CS(cond_actP t_act, cond_actP f_act)
{
	if (0 !=
		((V_regs.LazyFlagArgDst >> (32 - V_regs.LazyFlagArgSrc)) & 1))
	{
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslL_VC(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSLong(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) == V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

LOCALPROC reg_call cctrue_AslL_VS(cond_actP t_act, cond_actP f_act)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = uint32_t_FromSLong(V_regs.LazyFlagArgDst << cnt);

	if (Ui5rASR(dst, cnt) != V_regs.LazyFlagArgDst) {
		t_act();
	} else {
		f_act();
	}
}

FORWARDPROC reg_call cctrue_Dflt(cond_actP t_act, cond_actP f_act);

#endif /* UseLazyCC */

#if UseLazyCC
#define CCdispSz (16 * kNumLazyFlagsKinds)
#else
#define CCdispSz kNumLazyFlagsKinds
#endif

typedef void (reg_call *cctrueP)(cond_actP t_act, cond_actP f_act);

LOCALVAR const cctrueP cctrueDispatch[CCdispSz + 1] = {
	cctrue_T /* kLazyFlagsDefault T */,
	cctrue_F /* kLazyFlagsDefault F */,
	cctrue_HI /* kLazyFlagsDefault HI */,
	cctrue_LS /* kLazyFlagsDefault LS */,
	cctrue_CC /* kLazyFlagsDefault CC */,
	cctrue_CS /* kLazyFlagsDefault CS */,
	cctrue_NE /* kLazyFlagsDefault NE */,
	cctrue_EQ /* kLazyFlagsDefault EQ */,
	cctrue_VC /* kLazyFlagsDefault VC */,
	cctrue_VS /* kLazyFlagsDefault VS */,
	cctrue_PL /* kLazyFlagsDefault PL */,
	cctrue_MI /* kLazyFlagsDefault MI */,
	cctrue_GE /* kLazyFlagsDefault GE */,
	cctrue_LT /* kLazyFlagsDefault LT */,
	cctrue_GT /* kLazyFlagsDefault GT */,
	cctrue_LE /* kLazyFlagsDefault LE */,

#if UseLazyCC
	cctrue_T /* kLazyFlagsTstB T */,
	cctrue_F /* kLazyFlagsTstB F */,
	cctrue_Dflt /* kLazyFlagsTstB HI */,
	cctrue_Dflt /* kLazyFlagsTstB LS */,
	cctrue_Dflt /* kLazyFlagsTstB CC */,
	cctrue_Dflt /* kLazyFlagsTstB CS */,
	cctrue_Dflt /* kLazyFlagsTstB NE */,
	cctrue_Dflt /* kLazyFlagsTstB EQ */,
	cctrue_Dflt /* kLazyFlagsTstB VC */,
	cctrue_Dflt /* kLazyFlagsTstB VS */,
	cctrue_Dflt /* kLazyFlagsTstB PL */,
	cctrue_Dflt /* kLazyFlagsTstB MI */,
	cctrue_Dflt /* kLazyFlagsTstB GE */,
	cctrue_Dflt /* kLazyFlagsTstB LT */,
	cctrue_Dflt /* kLazyFlagsTstB GT */,
	cctrue_Dflt /* kLazyFlagsTstB LE */,

	cctrue_T /* kLazyFlagsTstW T */,
	cctrue_F /* kLazyFlagsTstW F */,
	cctrue_Dflt /* kLazyFlagsTstW HI */,
	cctrue_Dflt /* kLazyFlagsTstW LS */,
	cctrue_Dflt /* kLazyFlagsTstW CC */,
	cctrue_Dflt /* kLazyFlagsTstW CS */,
	cctrue_Dflt /* kLazyFlagsTstW NE */,
	cctrue_Dflt /* kLazyFlagsTstW EQ */,
	cctrue_Dflt /* kLazyFlagsTstW VC */,
	cctrue_Dflt /* kLazyFlagsTstW VS */,
	cctrue_Dflt /* kLazyFlagsTstW PL */,
	cctrue_Dflt /* kLazyFlagsTstW MI */,
	cctrue_Dflt /* kLazyFlagsTstW GE */,
	cctrue_Dflt /* kLazyFlagsTstW LT */,
	cctrue_Dflt /* kLazyFlagsTstW GT */,
	cctrue_Dflt /* kLazyFlagsTstW LE */,

	cctrue_T /* kLazyFlagsTstL T */,
	cctrue_F /* kLazyFlagsTstL F */,
	cctrue_TstL_HI /* kLazyFlagsTstL HI */,
	cctrue_TstL_LS /* kLazyFlagsTstL LS */,
	cctrue_T /* cctrue_TstL_CC */ /* kLazyFlagsTstL CC */,
	cctrue_F /* cctrue_TstL_CS */ /* kLazyFlagsTstL CS */,
	cctrue_TstL_NE /* kLazyFlagsTstL NE */,
	cctrue_TstL_EQ /* kLazyFlagsTstL EQ */,
	cctrue_T /* cctrue_Dflt */ /* kLazyFlagsTstL VC */,
	cctrue_F /* cctrue_Dflt */ /* kLazyFlagsTstL VS */,
	cctrue_TstL_PL /* kLazyFlagsTstL PL */,
	cctrue_TstL_MI /* kLazyFlagsTstL MI */,
	cctrue_TstL_GE /* kLazyFlagsTstL GE */,
	cctrue_TstL_LT /* kLazyFlagsTstL LT */,
	cctrue_TstL_GT /* kLazyFlagsTstL GT */,
	cctrue_TstL_LE /* kLazyFlagsTstL LE */,

	cctrue_T /* kLazyFlagsCmpB T */,
	cctrue_F /* kLazyFlagsCmpB F */,
	cctrue_CmpB_HI /* kLazyFlagsCmpB HI */,
	cctrue_CmpB_LS /* kLazyFlagsCmpB LS */,
	cctrue_CmpB_CC /* kLazyFlagsCmpB CC */,
	cctrue_CmpB_CS /* kLazyFlagsCmpB CS */,
	cctrue_CmpB_NE /* kLazyFlagsCmpB NE */,
	cctrue_CmpB_EQ /* kLazyFlagsCmpB EQ */,
	cctrue_Dflt /* kLazyFlagsCmpB VC */,
	cctrue_Dflt /* kLazyFlagsCmpB VS */,
	cctrue_CmpB_PL /* kLazyFlagsCmpB PL */,
	cctrue_CmpB_MI /* kLazyFlagsCmpB MI */,
	cctrue_CmpB_GE /* kLazyFlagsCmpB GE */,
	cctrue_CmpB_LT /* kLazyFlagsCmpB LT */,
	cctrue_CmpB_GT /* kLazyFlagsCmpB GT */,
	cctrue_CmpB_LE /* kLazyFlagsCmpB LE */,

	cctrue_T /* kLazyFlagsCmpW T */,
	cctrue_F /* kLazyFlagsCmpW F */,
	cctrue_CmpW_HI /* kLazyFlagsCmpW HI */,
	cctrue_CmpW_LS /* kLazyFlagsCmpW LS */,
	cctrue_CmpW_CC /* kLazyFlagsCmpW CC */,
	cctrue_CmpW_CS /* kLazyFlagsCmpW CS */,
	cctrue_CmpW_NE /* kLazyFlagsCmpW NE */,
	cctrue_CmpW_EQ /* kLazyFlagsCmpW EQ */,
	cctrue_Dflt /* kLazyFlagsCmpW VC */,
	cctrue_Dflt /* kLazyFlagsCmpW VS */,
	cctrue_CmpW_PL /* kLazyFlagsCmpW PL */,
	cctrue_CmpW_MI /* kLazyFlagsCmpW MI */,
	cctrue_CmpW_GE /* kLazyFlagsCmpW GE */,
	cctrue_CmpW_LT /* kLazyFlagsCmpW LT */,
	cctrue_CmpW_GT /* kLazyFlagsCmpW GT */,
	cctrue_CmpW_LE /* kLazyFlagsCmpW LE */,

	cctrue_T /* kLazyFlagsCmpL T */,
	cctrue_F /* kLazyFlagsCmpL F */,
	cctrue_CmpL_HI /* kLazyFlagsCmpL HI */,
	cctrue_CmpL_LS /* kLazyFlagsCmpL LS */,
	cctrue_CmpL_CC /* kLazyFlagsCmpL CC */,
	cctrue_CmpL_CS /* kLazyFlagsCmpL CS */,
	cctrue_CmpL_NE /* kLazyFlagsCmpL NE */,
	cctrue_CmpL_EQ /* kLazyFlagsCmpL EQ */,
	cctrue_Dflt /* kLazyFlagsCmpL VC */,
	cctrue_Dflt /* kLazyFlagsCmpL VS */,
	cctrue_CmpL_PL /* kLazyFlagsCmpL PL */,
	cctrue_CmpL_MI /* kLazyFlagsCmpL MI */,
	cctrue_CmpL_GE /* kLazyFlagsCmpL GE */,
	cctrue_CmpL_LT /* kLazyFlagsCmpL LT */,
	cctrue_CmpL_GT /* kLazyFlagsCmpL GT */,
	cctrue_CmpL_LE /* kLazyFlagsCmpL LE */,

	cctrue_T /* kLazyFlagsSubB T */,
	cctrue_F /* kLazyFlagsSubB F */,
	cctrue_CmpB_HI /* kLazyFlagsSubB HI */,
	cctrue_CmpB_LS /* kLazyFlagsSubB LS */,
	cctrue_CmpB_CC /* kLazyFlagsSubB CC */,
	cctrue_CmpB_CS /* kLazyFlagsSubB CS */,
	cctrue_CmpB_NE /* kLazyFlagsSubB NE */,
	cctrue_CmpB_EQ /* kLazyFlagsSubB EQ */,
	cctrue_Dflt /* kLazyFlagsSubB VC */,
	cctrue_Dflt /* kLazyFlagsSubB VS */,
	cctrue_CmpB_PL /* kLazyFlagsSubB PL */,
	cctrue_CmpB_MI /* kLazyFlagsSubB MI */,
	cctrue_CmpB_GE /* kLazyFlagsSubB GE */,
	cctrue_CmpB_LT /* kLazyFlagsSubB LT */,
	cctrue_CmpB_GT /* kLazyFlagsSubB GT */,
	cctrue_CmpB_LE /* kLazyFlagsSubB LE */,

	cctrue_T /* kLazyFlagsSubW T */,
	cctrue_F /* kLazyFlagsSubW F */,
	cctrue_CmpW_HI /* kLazyFlagsSubW HI */,
	cctrue_CmpW_LS /* kLazyFlagsSubW LS */,
	cctrue_CmpW_CC /* kLazyFlagsSubW CC */,
	cctrue_CmpW_CS /* kLazyFlagsSubW CS */,
	cctrue_CmpW_NE /* kLazyFlagsSubW NE */,
	cctrue_CmpW_EQ /* kLazyFlagsSubW EQ */,
	cctrue_Dflt /* kLazyFlagsSubW VC */,
	cctrue_Dflt /* kLazyFlagsSubW VS */,
	cctrue_CmpW_PL /* kLazyFlagsSubW PL */,
	cctrue_CmpW_MI /* kLazyFlagsSubW MI */,
	cctrue_CmpW_GE /* kLazyFlagsSubW GE */,
	cctrue_CmpW_LT /* kLazyFlagsSubW LT */,
	cctrue_CmpW_GT /* kLazyFlagsSubW GT */,
	cctrue_CmpW_LE /* kLazyFlagsSubW LE */,

	cctrue_T /* kLazyFlagsSubL T */,
	cctrue_F /* kLazyFlagsSubL F */,
	cctrue_CmpL_HI /* kLazyFlagsSubL HI */,
	cctrue_CmpL_LS /* kLazyFlagsSubL LS */,
	cctrue_CmpL_CC /* kLazyFlagsSubL CC */,
	cctrue_CmpL_CS /* kLazyFlagsSubL CS */,
	cctrue_CmpL_NE /* kLazyFlagsSubL NE */,
	cctrue_CmpL_EQ /* kLazyFlagsSubL EQ */,
	cctrue_Dflt /* kLazyFlagsSubL VC */,
	cctrue_Dflt /* kLazyFlagsSubL VS */,
	cctrue_CmpL_PL /* kLazyFlagsSubL PL */,
	cctrue_CmpL_MI /* kLazyFlagsSubL MI */,
	cctrue_CmpL_GE /* kLazyFlagsSubL GE */,
	cctrue_CmpL_LT /* kLazyFlagsSubL LT */,
	cctrue_CmpL_GT /* kLazyFlagsSubL GT */,
	cctrue_CmpL_LE /* kLazyFlagsSubL LE */,

	cctrue_T /* kLazyFlagsAddB T */,
	cctrue_F /* kLazyFlagsAddB F */,
	cctrue_Dflt /* kLazyFlagsAddB HI */,
	cctrue_Dflt /* kLazyFlagsAddB LS */,
	cctrue_Dflt /* kLazyFlagsAddB CC */,
	cctrue_Dflt /* kLazyFlagsAddB CS */,
	cctrue_Dflt /* kLazyFlagsAddB NE */,
	cctrue_Dflt /* kLazyFlagsAddB EQ */,
	cctrue_Dflt /* kLazyFlagsAddB VC */,
	cctrue_Dflt /* kLazyFlagsAddB VS */,
	cctrue_Dflt /* kLazyFlagsAddB PL */,
	cctrue_Dflt /* kLazyFlagsAddB MI */,
	cctrue_Dflt /* kLazyFlagsAddB GE */,
	cctrue_Dflt /* kLazyFlagsAddB LT */,
	cctrue_Dflt /* kLazyFlagsAddB GT */,
	cctrue_Dflt /* kLazyFlagsAddB LE */,

	cctrue_T /* kLazyFlagsAddW T */,
	cctrue_F /* kLazyFlagsAddW F */,
	cctrue_Dflt /* kLazyFlagsAddW HI */,
	cctrue_Dflt /* kLazyFlagsAddW LS */,
	cctrue_Dflt /* kLazyFlagsAddW CC */,
	cctrue_Dflt /* kLazyFlagsAddW CS */,
	cctrue_Dflt /* kLazyFlagsAddW NE */,
	cctrue_Dflt /* kLazyFlagsAddW EQ */,
	cctrue_Dflt /* kLazyFlagsAddW VC */,
	cctrue_Dflt /* kLazyFlagsAddW VS */,
	cctrue_Dflt /* kLazyFlagsAddW PL */,
	cctrue_Dflt /* kLazyFlagsAddW MI */,
	cctrue_Dflt /* kLazyFlagsAddW GE */,
	cctrue_Dflt /* kLazyFlagsAddW LT */,
	cctrue_Dflt /* kLazyFlagsAddW GT */,
	cctrue_Dflt /* kLazyFlagsAddW LE */,

	cctrue_T /* kLazyFlagsAddL T */,
	cctrue_F /* kLazyFlagsAddL F */,
	cctrue_Dflt /* kLazyFlagsAddL HI */,
	cctrue_Dflt /* kLazyFlagsAddL LS */,
	cctrue_Dflt /* kLazyFlagsAddL CC */,
	cctrue_Dflt /* kLazyFlagsAddL CS */,
	cctrue_Dflt /* kLazyFlagsAddL NE */,
	cctrue_Dflt /* kLazyFlagsAddL EQ */,
	cctrue_Dflt /* kLazyFlagsAddL VC */,
	cctrue_Dflt /* kLazyFlagsAddL VS */,
	cctrue_Dflt /* kLazyFlagsAddL PL */,
	cctrue_Dflt /* kLazyFlagsAddL MI */,
	cctrue_Dflt /* kLazyFlagsAddL GE */,
	cctrue_Dflt /* kLazyFlagsAddL LT */,
	cctrue_Dflt /* kLazyFlagsAddL GT */,
	cctrue_Dflt /* kLazyFlagsAddL LE */,

	cctrue_T /* kLazyFlagsNegB T */,
	cctrue_F /* kLazyFlagsNegB F */,
	cctrue_Dflt /* kLazyFlagsNegB HI */,
	cctrue_Dflt /* kLazyFlagsNegB LS */,
	cctrue_Dflt /* kLazyFlagsNegB CC */,
	cctrue_Dflt /* kLazyFlagsNegB CS */,
	cctrue_Dflt /* kLazyFlagsNegB NE */,
	cctrue_Dflt /* kLazyFlagsNegB EQ */,
	cctrue_Dflt /* kLazyFlagsNegB VC */,
	cctrue_Dflt /* kLazyFlagsNegB VS */,
	cctrue_Dflt /* kLazyFlagsNegB PL */,
	cctrue_Dflt /* kLazyFlagsNegB MI */,
	cctrue_Dflt /* kLazyFlagsNegB GE */,
	cctrue_Dflt /* kLazyFlagsNegB LT */,
	cctrue_Dflt /* kLazyFlagsNegB GT */,
	cctrue_Dflt /* kLazyFlagsNegB LE */,

	cctrue_T /* kLazyFlagsNegW T */,
	cctrue_F /* kLazyFlagsNegW F */,
	cctrue_Dflt /* kLazyFlagsNegW HI */,
	cctrue_Dflt /* kLazyFlagsNegW LS */,
	cctrue_Dflt /* kLazyFlagsNegW CC */,
	cctrue_Dflt /* kLazyFlagsNegW CS */,
	cctrue_Dflt /* kLazyFlagsNegW NE */,
	cctrue_Dflt /* kLazyFlagsNegW EQ */,
	cctrue_Dflt /* kLazyFlagsNegW VC */,
	cctrue_Dflt /* kLazyFlagsNegW VS */,
	cctrue_Dflt /* kLazyFlagsNegW PL */,
	cctrue_Dflt /* kLazyFlagsNegW MI */,
	cctrue_Dflt /* kLazyFlagsNegW GE */,
	cctrue_Dflt /* kLazyFlagsNegW LT */,
	cctrue_Dflt /* kLazyFlagsNegW GT */,
	cctrue_Dflt /* kLazyFlagsNegW LE */,

	cctrue_T /* kLazyFlagsNegL T */,
	cctrue_F /* kLazyFlagsNegL F */,
	cctrue_Dflt /* kLazyFlagsNegL HI */,
	cctrue_Dflt /* kLazyFlagsNegL LS */,
	cctrue_Dflt /* kLazyFlagsNegL CC */,
	cctrue_Dflt /* kLazyFlagsNegL CS */,
	cctrue_Dflt /* kLazyFlagsNegL NE */,
	cctrue_Dflt /* kLazyFlagsNegL EQ */,
	cctrue_Dflt /* kLazyFlagsNegL VC */,
	cctrue_Dflt /* kLazyFlagsNegL VS */,
	cctrue_Dflt /* kLazyFlagsNegL PL */,
	cctrue_Dflt /* kLazyFlagsNegL MI */,
	cctrue_Dflt /* kLazyFlagsNegL GE */,
	cctrue_Dflt /* kLazyFlagsNegL LT */,
	cctrue_Dflt /* kLazyFlagsNegL GT */,
	cctrue_Dflt /* kLazyFlagsNegL LE */,

	cctrue_T /* kLazyFlagsAsrB T */,
	cctrue_F /* kLazyFlagsAsrB F */,
	cctrue_Dflt /* kLazyFlagsAsrB HI */,
	cctrue_Dflt /* kLazyFlagsAsrB LS */,
	cctrue_Asr_CC /* kLazyFlagsAsrB CC */,
	cctrue_Asr_CS /* kLazyFlagsAsrB CS */,
	cctrue_Dflt /* kLazyFlagsAsrB NE */,
	cctrue_Dflt /* kLazyFlagsAsrB EQ */,
	cctrue_Dflt /* kLazyFlagsAsrB VC */,
	cctrue_Dflt /* kLazyFlagsAsrB VS */,
	cctrue_Dflt /* kLazyFlagsAsrB PL */,
	cctrue_Dflt /* kLazyFlagsAsrB MI */,
	cctrue_Dflt /* kLazyFlagsAsrB GE */,
	cctrue_Dflt /* kLazyFlagsAsrB LT */,
	cctrue_Dflt /* kLazyFlagsAsrB GT */,
	cctrue_Dflt /* kLazyFlagsAsrB LE */,

	cctrue_T /* kLazyFlagsAsrW T */,
	cctrue_F /* kLazyFlagsAsrW F */,
	cctrue_Dflt /* kLazyFlagsAsrW HI */,
	cctrue_Dflt /* kLazyFlagsAsrW LS */,
	cctrue_Asr_CC /* kLazyFlagsAsrW CC */,
	cctrue_Asr_CS /* kLazyFlagsAsrW CS */,
	cctrue_Dflt /* kLazyFlagsAsrW NE */,
	cctrue_Dflt /* kLazyFlagsAsrW EQ */,
	cctrue_Dflt /* kLazyFlagsAsrW VC */,
	cctrue_Dflt /* kLazyFlagsAsrW VS */,
	cctrue_Dflt /* kLazyFlagsAsrW PL */,
	cctrue_Dflt /* kLazyFlagsAsrW MI */,
	cctrue_Dflt /* kLazyFlagsAsrW GE */,
	cctrue_Dflt /* kLazyFlagsAsrW LT */,
	cctrue_Dflt /* kLazyFlagsAsrW GT */,
	cctrue_Dflt /* kLazyFlagsAsrW LE */,

	cctrue_T /* kLazyFlagsAsrL T */,
	cctrue_F /* kLazyFlagsAsrL F */,
	cctrue_Dflt /* kLazyFlagsAsrL HI */,
	cctrue_Dflt /* kLazyFlagsAsrL LS */,
	cctrue_Asr_CC /* kLazyFlagsAsrL CC */,
	cctrue_Asr_CS /* kLazyFlagsAsrL CS */,
	cctrue_Dflt /* kLazyFlagsAsrL NE */,
	cctrue_Dflt /* kLazyFlagsAsrL EQ */,
	cctrue_Dflt /* kLazyFlagsAsrL VC */,
	cctrue_Dflt /* kLazyFlagsAsrL VS */,
	cctrue_Dflt /* kLazyFlagsAsrL PL */,
	cctrue_Dflt /* kLazyFlagsAsrL MI */,
	cctrue_Dflt /* kLazyFlagsAsrL GE */,
	cctrue_Dflt /* kLazyFlagsAsrL LT */,
	cctrue_Dflt /* kLazyFlagsAsrL GT */,
	cctrue_Dflt /* kLazyFlagsAsrL LE */,

	cctrue_T /* kLazyFlagsAslB T */,
	cctrue_F /* kLazyFlagsAslB F */,
	cctrue_Dflt /* kLazyFlagsAslB HI */,
	cctrue_Dflt /* kLazyFlagsAslB LS */,
	cctrue_AslB_CC /* kLazyFlagsAslB CC */,
	cctrue_AslB_CS /* kLazyFlagsAslB CS */,
	cctrue_Dflt /* kLazyFlagsAslB NE */,
	cctrue_Dflt /* kLazyFlagsAslB EQ */,
	cctrue_AslB_VC /* kLazyFlagsAslB VC */,
	cctrue_AslB_VS /* kLazyFlagsAslB VS */,
	cctrue_Dflt /* kLazyFlagsAslB PL */,
	cctrue_Dflt /* kLazyFlagsAslB MI */,
	cctrue_Dflt /* kLazyFlagsAslB GE */,
	cctrue_Dflt /* kLazyFlagsAslB LT */,
	cctrue_Dflt /* kLazyFlagsAslB GT */,
	cctrue_Dflt /* kLazyFlagsAslB LE */,

	cctrue_T /* kLazyFlagsAslW T */,
	cctrue_F /* kLazyFlagsAslW F */,
	cctrue_Dflt /* kLazyFlagsAslW HI */,
	cctrue_Dflt /* kLazyFlagsAslW LS */,
	cctrue_AslW_CC /* kLazyFlagsAslW CC */,
	cctrue_AslW_CS /* kLazyFlagsAslW CS */,
	cctrue_Dflt /* kLazyFlagsAslW NE */,
	cctrue_Dflt /* kLazyFlagsAslW EQ */,
	cctrue_AslW_VC /* kLazyFlagsAslW VC */,
	cctrue_AslW_VS /* kLazyFlagsAslW VS */,
	cctrue_Dflt /* kLazyFlagsAslW PL */,
	cctrue_Dflt /* kLazyFlagsAslW MI */,
	cctrue_Dflt /* kLazyFlagsAslW GE */,
	cctrue_Dflt /* kLazyFlagsAslW LT */,
	cctrue_Dflt /* kLazyFlagsAslW GT */,
	cctrue_Dflt /* kLazyFlagsAslW LE */,

	cctrue_T /* kLazyFlagsAslL T */,
	cctrue_F /* kLazyFlagsAslL F */,
	cctrue_Dflt /* kLazyFlagsAslL HI */,
	cctrue_Dflt /* kLazyFlagsAslL LS */,
	cctrue_AslL_CC /* kLazyFlagsAslL CC */,
	cctrue_AslL_CS /* kLazyFlagsAslL CS */,
	cctrue_Dflt /* kLazyFlagsAslL NE */,
	cctrue_Dflt /* kLazyFlagsAslL EQ */,
	cctrue_AslL_VC /* kLazyFlagsAslL VC */,
	cctrue_AslL_VS /* kLazyFlagsAslL VS */,
	cctrue_Dflt /* kLazyFlagsAslL PL */,
	cctrue_Dflt /* kLazyFlagsAslL MI */,
	cctrue_Dflt /* kLazyFlagsAslL GE */,
	cctrue_Dflt /* kLazyFlagsAslL LT */,
	cctrue_Dflt /* kLazyFlagsAslL GT */,
	cctrue_Dflt /* kLazyFlagsAslL LE */,

#if UseLazyZ
	cctrue_T /* kLazyFlagsZSet T */,
	cctrue_F /* kLazyFlagsZSet F */,
	cctrue_Dflt /* kLazyFlagsZSet HI */,
	cctrue_Dflt /* kLazyFlagsZSet LS */,
	cctrue_Dflt /* kLazyFlagsZSet CC */,
	cctrue_Dflt /* kLazyFlagsZSet CS */,
	cctrue_NE /* kLazyFlagsZSet NE */,
	cctrue_EQ /* kLazyFlagsZSet EQ */,
	cctrue_Dflt /* kLazyFlagsZSet VC */,
	cctrue_Dflt /* kLazyFlagsZSet VS */,
	cctrue_Dflt /* kLazyFlagsZSet PL */,
	cctrue_Dflt /* kLazyFlagsZSet MI */,
	cctrue_Dflt /* kLazyFlagsZSet GE */,
	cctrue_Dflt /* kLazyFlagsZSet LT */,
	cctrue_Dflt /* kLazyFlagsZSet GT */,
	cctrue_Dflt /* kLazyFlagsZSet LE */,
#endif
#endif /* UseLazyCC */

	0
};

#if UseLazyCC
LOCALINLINEPROC cctrue(cond_actP t_act, cond_actP f_act)
{
	(cctrueDispatch[V_regs.LazyFlagKind * 16
		+ V_regs.CurDecOpY.v[0].ArgDat])(t_act, f_act);
}
#endif


LOCALPROC NeedDefaultLazyXFlagSubB(void)
{
	XFLG = Bool2Bit(((uint8_t)V_regs.LazyXFlagArgDst)
		< ((uint8_t)V_regs.LazyXFlagArgSrc));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagSubW(void)
{
	XFLG = Bool2Bit(((uint16_t)V_regs.LazyXFlagArgDst)
		< ((uint16_t)V_regs.LazyXFlagArgSrc));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagSubL(void)
{
	XFLG = Bool2Bit(((uint32_t)V_regs.LazyXFlagArgDst)
		< ((uint32_t)V_regs.LazyXFlagArgSrc));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAddB(void)
{
	uint8_t src = (uint8_t)V_regs.LazyXFlagArgSrc;
	uint8_t dst = (uint8_t)V_regs.LazyXFlagArgDst;
	uint8_t result = dst + src;

	XFLG = Bool2Bit(result < src);
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAddW(void)
{
	uint16_t src = (uint16_t)V_regs.LazyXFlagArgSrc;
	uint16_t dst = (uint16_t)V_regs.LazyXFlagArgDst;
	uint16_t result = dst + src;

	XFLG = Bool2Bit(result < src);
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAddL(void)
{
	uint32_t src = (uint32_t)V_regs.LazyXFlagArgSrc;
	uint32_t dst = (uint32_t)V_regs.LazyXFlagArgDst;
	uint32_t result = dst + src;

	XFLG = Bool2Bit(result < src);
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagNegB(void)
{
	XFLG = Bool2Bit(((uint8_t)0)
		< ((uint8_t)V_regs.LazyXFlagArgDst));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagNegW(void)
{
	XFLG = Bool2Bit(((uint16_t)0)
		< ((uint16_t)V_regs.LazyXFlagArgDst));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagNegL(void)
{
	XFLG = Bool2Bit(((uint32_t)0)
		< ((uint32_t)V_regs.LazyXFlagArgDst));
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAsr(void)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;

	XFLG = ((dst >> (cnt - 1)) & 1);

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAslB(void)
{
	XFLG = (V_regs.LazyFlagArgDst >> (8 - V_regs.LazyFlagArgSrc)) & 1;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAslW(void)
{
	XFLG = (V_regs.LazyFlagArgDst >> (16 - V_regs.LazyFlagArgSrc)) & 1;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagAslL(void)
{
	XFLG = (V_regs.LazyFlagArgDst >> (32 - V_regs.LazyFlagArgSrc)) & 1;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyXFlagDefault(void)
{
}

typedef void (*NeedLazyFlagP)(void);

LOCALVAR const NeedLazyFlagP
	NeedLazyXFlagDispatch[kNumLazyFlagsKinds + 1] =
{
	NeedDefaultLazyXFlagDefault /* kLazyFlagsDefault */,
	0 /* kLazyFlagsTstB */,
	0 /* kLazyFlagsTstW */,
	0 /* kLazyFlagsTstL */,
	0 /* kLazyFlagsCmpB */,
	0 /* kLazyFlagsCmpW */,
	0 /* kLazyFlagsCmpL */,
	NeedDefaultLazyXFlagSubB /* kLazyFlagsSubB */,
	NeedDefaultLazyXFlagSubW /* kLazyFlagsSubW */,
	NeedDefaultLazyXFlagSubL /* kLazyFlagsSubL */,
	NeedDefaultLazyXFlagAddB /* kLazyFlagsAddB */,
	NeedDefaultLazyXFlagAddW /* kLazyFlagsAddW */,
	NeedDefaultLazyXFlagAddL /* kLazyFlagsAddL */,
	NeedDefaultLazyXFlagNegB /* kLazyFlagsNegB */,
	NeedDefaultLazyXFlagNegW /* kLazyFlagsNegW */,
	NeedDefaultLazyXFlagNegL /* kLazyFlagsNegL */,
	NeedDefaultLazyXFlagAsr  /* kLazyFlagsAsrB */,
	NeedDefaultLazyXFlagAsr  /* kLazyFlagsAsrW */,
	NeedDefaultLazyXFlagAsr  /* kLazyFlagsAsrL */,
	NeedDefaultLazyXFlagAslB /* kLazyFlagsAslB */,
	NeedDefaultLazyXFlagAslW /* kLazyFlagsAslW */,
	NeedDefaultLazyXFlagAslL /* kLazyFlagsAslL */,
#if UseLazyZ
	0 /* kLazyFlagsZSet */,
#endif

	0
};

LOCALPROC NeedDefaultLazyXFlag(void)
{
#if ForceFlagsEval
	if (kLazyFlagsDefault != V_regs.LazyXFlagKind) {
		ReportAbnormalID(0x0103,
			"not kLazyFlagsDefault in NeedDefaultLazyXFlag");
	}
#else
	(NeedLazyXFlagDispatch[V_regs.LazyXFlagKind])();
#endif
}

LOCALPROC NeedDefaultLazyFlagsTstL(void)
{
	uint32_t dst = V_regs.LazyFlagArgDst;

	VFLG = CFLG = 0;
	ZFLG = Bool2Bit(dst == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(dst));

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	NeedDefaultLazyXFlag();
}

LOCALPROC NeedDefaultLazyFlagsCmpB(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result0 = dst - src;
	uint32_t result1 = uint32_t_FromUByte(dst)
		- uint32_t_FromUByte(src);
	uint32_t result = uint32_t_FromSByte(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));
	VFLG = (((result0 >> 1) ^ result0) >> 7) & 1;
	CFLG = (result1 >> 8) & 1;

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	NeedDefaultLazyXFlag();
}

LOCALPROC NeedDefaultLazyFlagsCmpW(void)
{
	uint32_t result0 = V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc;
	uint32_t result = uint32_t_FromSWord(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));

	VFLG = (((result0 >> 1) ^ result0) >> 15) & 1;
	{
		uint32_t result1 = uint32_t_FromUWord(V_regs.LazyFlagArgDst)
			- uint32_t_FromUWord(V_regs.LazyFlagArgSrc);

		CFLG = (result1 >> 16) & 1;
	}

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	NeedDefaultLazyXFlag();
}

LOCALPROC NeedDefaultLazyFlagsCmpL(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSLong(dst - src);

	ZFLG = Bool2Bit(result == 0);

	{
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(src));
		flagtype flgo = Bool2Bit(uint32_t_MSBisSet(dst)) ^ 1;
		flagtype flgsando = flgs & flgo;
		flagtype flgsoro = flgs | flgo;

		NFLG = flgn;
		VFLG = ((flgn | flgsoro) ^ 1) | (flgn & flgsando);
		CFLG = flgsando | (flgn & flgsoro);
	}

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	NeedDefaultLazyXFlag();
}

LOCALPROC NeedDefaultLazyFlagsSubB(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result0 = dst - src;
	uint32_t result1 = uint32_t_FromUByte(dst)
		- uint32_t_FromUByte(src);
	uint32_t result = uint32_t_FromSByte(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));
	VFLG = (((result0 >> 1) ^ result0) >> 7) & 1;
	CFLG = (result1 >> 8) & 1;

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsSubW(void)
{
	uint32_t result0 = V_regs.LazyFlagArgDst - V_regs.LazyFlagArgSrc;
	uint32_t result = uint32_t_FromSWord(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));

	VFLG = (((result0 >> 1) ^ result0) >> 15) & 1;
	{
		uint32_t result1 = uint32_t_FromUWord(V_regs.LazyFlagArgDst)
			- uint32_t_FromUWord(V_regs.LazyFlagArgSrc);

		CFLG = (result1 >> 16) & 1;
	}

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsSubL(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSLong(dst - src);

	ZFLG = Bool2Bit(result == 0);

	{
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(src));
		flagtype flgo = Bool2Bit(uint32_t_MSBisSet(dst)) ^ 1;
		flagtype flgsando = flgs & flgo;
		flagtype flgsoro = flgs | flgo;

		NFLG = flgn;
		VFLG = ((flgn | flgsoro) ^ 1) | (flgn & flgsando);
		CFLG = flgsando | (flgn & flgsoro);
	}

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsAddB(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result0 = dst + src;
	uint32_t result1 = uint32_t_FromUByte(dst)
		+ uint32_t_FromUByte(src);
	uint32_t result = uint32_t_FromSByte(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));
	VFLG = (((result0 >> 1) ^ result0) >> 7) & 1;
	CFLG = (result1 >> 8);

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsAddW(void)
{
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result0 = dst + src;
	uint32_t result1 = uint32_t_FromUWord(dst)
		+ uint32_t_FromUWord(src);
	uint32_t result = uint32_t_FromSWord(result0);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));
	VFLG = (((result0 >> 1) ^ result0) >> 15) & 1;
	CFLG = (result1 >> 16);

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

#if 0
LOCALPROC NeedDefaultLazyFlagsAddCommon(uint32_t result)
{
	ZFLG = Bool2Bit(result == 0);
	{
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(V_regs.LazyFlagArgSrc));
		flagtype flgo = Bool2Bit(uint32_t_MSBisSet(V_regs.LazyFlagArgDst));
		flagtype flgsando = flgs & flgo;
		flagtype flgsoro = flgs | flgo;

		NFLG = flgn;
		flgn ^= 1;
		VFLG = ((flgn | flgsoro) ^ 1) | (flgn & flgsando);
		CFLG = flgsando | (flgn & flgsoro);
	}

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}
#endif

LOCALPROC NeedDefaultLazyFlagsAddL(void)
{
#if 1
	uint32_t src = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSLong(dst + src);

	ZFLG = Bool2Bit(result == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(result));

	{
		uint32_t result1;
		uint32_t result0;
		uint32_t MidCarry = (uint32_t_FromUWord(dst)
			+ uint32_t_FromUWord(src)) >> 16;

		dst >>= 16;
		src >>= 16;

		result1 = uint32_t_FromUWord(dst)
			+ uint32_t_FromUWord(src)
			+ MidCarry;
		CFLG = (result1 >> 16);
		result0 = uint32_t_FromSWord(dst)
			+ uint32_t_FromSWord(src)
			+ MidCarry;
		VFLG = (((result0 >> 1) ^ result0) >> 15) & 1;
	}

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
#else
	uint32_t result = uint32_t_FromSLong(V_regs.LazyFlagArgDst
		+ V_regs.LazyFlagArgSrc);

	NeedDefaultLazyFlagsAddCommon(result);
#endif
}

LOCALPROC NeedDefaultLazyFlagsNegCommon(uint32_t dstvalue, uint32_t result)
{
	flagtype flgs = Bool2Bit(uint32_t_MSBisSet(dstvalue));
	flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));

	ZFLG = Bool2Bit(result == 0);
	NFLG = flgn;
	VFLG = flgs & flgn;
	CFLG = flgs | flgn;

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsNegB(void)
{
	uint32_t dstvalue = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSByte(0 - dstvalue);

	NeedDefaultLazyFlagsNegCommon(dstvalue, result);
}

LOCALPROC NeedDefaultLazyFlagsNegW(void)
{
	uint32_t dstvalue = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSWord(0 - dstvalue);

	NeedDefaultLazyFlagsNegCommon(dstvalue, result);
}

LOCALPROC NeedDefaultLazyFlagsNegL(void)
{
	uint32_t dstvalue = V_regs.LazyFlagArgDst;
	uint32_t result = uint32_t_FromSLong(0 - dstvalue);

	NeedDefaultLazyFlagsNegCommon(dstvalue, result);
}

LOCALPROC NeedDefaultLazyFlagsAsr(void)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;

	NFLG = Bool2Bit(uint32_t_MSBisSet(dst));
	VFLG = 0;

	CFLG = ((dst >> (cnt - 1)) & 1);
	dst = Ui5rASR(dst, cnt);
	ZFLG = Bool2Bit(dst == 0);

	XFLG = CFLG;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsAslB(void)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t dstvalue0 = dst;
	uint32_t comparevalue;

	dst = dst << (cnt - 1);
	dst = uint32_t_FromSByte(dst);
	CFLG = Bool2Bit(uint32_t_MSBisSet(dst));
	dst = dst << 1;
	dst = uint32_t_FromSByte(dst);
	comparevalue = Ui5rASR(dst, cnt);
	VFLG = Bool2Bit(comparevalue != dstvalue0);
	ZFLG = Bool2Bit(dst == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(dst));

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsAslW(void)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t dstvalue0 = dst;
	uint32_t comparevalue;

	dst = dst << (cnt - 1);
	dst = uint32_t_FromSWord(dst);
	CFLG = Bool2Bit(uint32_t_MSBisSet(dst));
	dst = dst << 1;
	dst = uint32_t_FromSWord(dst);
	comparevalue = Ui5rASR(dst, cnt);
	VFLG = Bool2Bit(comparevalue != dstvalue0);
	ZFLG = Bool2Bit(dst == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(dst));

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

LOCALPROC NeedDefaultLazyFlagsAslL(void)
{
	uint32_t cnt = V_regs.LazyFlagArgSrc;
	uint32_t dst = V_regs.LazyFlagArgDst;
	uint32_t dstvalue0 = dst;
	uint32_t comparevalue;

	dst = dst << (cnt - 1);
	dst = uint32_t_FromSLong(dst);
	CFLG = Bool2Bit(uint32_t_MSBisSet(dst));
	dst = dst << 1;
	dst = uint32_t_FromSLong(dst);
	comparevalue = Ui5rASR(dst, cnt);
	VFLG = Bool2Bit(comparevalue != dstvalue0);
	ZFLG = Bool2Bit(dst == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(dst));

	XFLG = CFLG;
	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}

#if UseLazyZ
FORWARDPROC NeedDefaultLazyFlagsZSet(void);
#endif

LOCALVAR const NeedLazyFlagP
	NeedLazyFlagDispatch[kNumLazyFlagsKinds + 1] =
{
	NeedDefaultLazyXFlag /* kLazyFlagsDefault */,
	0 /* kLazyFlagsTstB */,
	0 /* kLazyFlagsTstW */,
	NeedDefaultLazyFlagsTstL /* kLazyFlagsTstL */,
	NeedDefaultLazyFlagsCmpB /* kLazyFlagsCmpB */,
	NeedDefaultLazyFlagsCmpW /* kLazyFlagsCmpW */,
	NeedDefaultLazyFlagsCmpL /* kLazyFlagsCmpL */,
	NeedDefaultLazyFlagsSubB /* kLazyFlagsSubB */,
	NeedDefaultLazyFlagsSubW /* kLazyFlagsSubW */,
	NeedDefaultLazyFlagsSubL /* kLazyFlagsSubL */,
	NeedDefaultLazyFlagsAddB /* kLazyFlagsAddB */,
	NeedDefaultLazyFlagsAddW /* kLazyFlagsAddW */,
	NeedDefaultLazyFlagsAddL /* kLazyFlagsAddL */,
	NeedDefaultLazyFlagsNegB /* kLazyFlagsNegB */,
	NeedDefaultLazyFlagsNegW /* kLazyFlagsNegW */,
	NeedDefaultLazyFlagsNegL /* kLazyFlagsNegL */,
	NeedDefaultLazyFlagsAsr  /* kLazyFlagsAsrB */,
	NeedDefaultLazyFlagsAsr  /* kLazyFlagsAsrW */,
	NeedDefaultLazyFlagsAsr  /* kLazyFlagsAsrL */,
	NeedDefaultLazyFlagsAslB /* kLazyFlagsAslB */,
	NeedDefaultLazyFlagsAslW /* kLazyFlagsAslW */,
	NeedDefaultLazyFlagsAslL /* kLazyFlagsAslL */,
#if UseLazyZ
	NeedDefaultLazyFlagsZSet /* kLazyFlagsZSet */,
#endif

	0
};

LOCALPROC NeedDefaultLazyAllFlags0(void)
{
	(NeedLazyFlagDispatch[V_regs.LazyFlagKind])();
}

#if ForceFlagsEval
LOCALPROC NeedDefaultLazyAllFlags(void)
{
	if (kLazyFlagsDefault != V_regs.LazyFlagKind) {
		ReportAbnormalID(0x0104,
			"not kLazyFlagsDefault in NeedDefaultLazyAllFlags");
#if dbglog_HAVE
		dbglog_writelnNum("LazyFlagKind", V_regs.LazyFlagKind);
#endif
	}
}
#else
#define NeedDefaultLazyAllFlags NeedDefaultLazyAllFlags0
#endif

#if ForceFlagsEval
#define HaveSetUpFlags NeedDefaultLazyAllFlags0
#else
#define HaveSetUpFlags()
#endif

#if UseLazyZ
LOCALPROC NeedDefaultLazyFlagsZSet(void)
{
	flagtype SaveZFLG = ZFLG;

	V_regs.LazyFlagKind = V_regs.LazyFlagZSavedKind;
	NeedDefaultLazyAllFlags();

	ZFLG = SaveZFLG;
}
#endif

#if UseLazyCC
LOCALPROC reg_call cctrue_Dflt(cond_actP t_act, cond_actP f_act)
{
	NeedDefaultLazyAllFlags();
	cctrue(t_act, f_act);
}
#endif

#if ! UseLazyCC
LOCALINLINEPROC cctrue(cond_actP t_act, cond_actP f_act)
{
	NeedDefaultLazyAllFlags();
	(cctrueDispatch[V_regs.CurDecOpY.v[0].ArgDat])(t_act, f_act);
}
#endif


#define LOCALIPROC LOCALPROC /* LOCALPROCUSEDONCE */

LOCALIPROC DoCodeCmpB(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsCmpB;
	V_regs.LazyFlagArgSrc = V_regs.SrcVal;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();
}

LOCALIPROC DoCodeCmpW(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsCmpW;
	V_regs.LazyFlagArgSrc = V_regs.SrcVal;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();
}

LOCALIPROC DoCodeCmpL(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsCmpL;
	V_regs.LazyFlagArgSrc = V_regs.SrcVal;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();
}

LOCALIPROC DoCodeMoveL(void)
{
	uint32_t src = DecodeGetSrcValue();

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = src;

	HaveSetUpFlags();

	DecodeSetDstValue(src);
}

LOCALIPROC DoCodeMoveW(void)
{
	uint32_t src = DecodeGetSrcValue();

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = src;

	HaveSetUpFlags();

	DecodeSetDstValue(src);
}

LOCALIPROC DoCodeMoveB(void)
{
	uint32_t src = DecodeGetSrcValue();

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = src;

	HaveSetUpFlags();

	DecodeSetDstValue(src);
}

LOCALIPROC DoCodeTst(void)
{
	/* Tst 01001010ssmmmrrr */

	uint32_t srcvalue = DecodeGetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = srcvalue;

	HaveSetUpFlags();
}

LOCALIPROC DoCodeBraB(void)
{
	int32_t offset = (int32_t)(int8_t)(uint8_t)(V_regs.CurDecOpY.v[1].ArgDat);
	uint8_t * s = V_pc_p + offset;

	V_pc_p = s;

#if USE_PCLIMIT
	if (cond_rare(s >= V_pc_pHi)
		|| cond_rare(s < V_regs.pc_pLo))
	{
		Recalc_PC_Block();
	}
#endif
}

LOCALIPROC DoCodeBraW(void)
{
	int32_t offset = (int32_t)(int16_t)(uint16_t)do_get_mem_word(V_pc_p);
		/* note that pc not incremented here */
	uint8_t * s = V_pc_p + offset;

	V_pc_p = s;

#if USE_PCLIMIT
	if (cond_rare(s >= V_pc_pHi)
		|| cond_rare(s < V_regs.pc_pLo))
	{
		Recalc_PC_Block();
	}
#endif
}

#if WantCloserCyc
LOCALPROC DoCodeBccB_t(void)
{
	V_MaxCyclesToGo -= (10 * kCycleScale + 2 * RdAvgXtraCyc);
	DoCodeBraB();
}
#else
#define DoCodeBccB_t DoCodeBraB
#endif

LOCALPROC DoCodeBccB_f(void)
{
#if WantCloserCyc
	V_MaxCyclesToGo -= (8 * kCycleScale + RdAvgXtraCyc);
#endif
		/* do nothing */
}

LOCALIPROC DoCodeBccB(void)
{
	/* Bcc 0110ccccnnnnnnnn */
	cctrue(DoCodeBccB_t, DoCodeBccB_f);
}

LOCALPROC SkipiWord(void)
{
	V_pc_p += 2;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		Recalc_PC_Block();
	}
#endif
}

#if WantCloserCyc
LOCALPROC DoCodeBccW_t(void)
{
	V_MaxCyclesToGo -= (10 * kCycleScale + 2 * RdAvgXtraCyc);
	DoCodeBraW();
}
#else
#define DoCodeBccW_t DoCodeBraW
#endif

#if WantCloserCyc
LOCALPROC DoCodeBccW_f(void)
{
	V_MaxCyclesToGo -= (12 * kCycleScale + 2 * RdAvgXtraCyc);
	SkipiWord();
}
#else
#define DoCodeBccW_f SkipiWord
#endif

LOCALIPROC DoCodeBccW(void)
{
	/* Bcc 0110ccccnnnnnnnn */
	cctrue(DoCodeBccW_t, DoCodeBccW_f);
}


LOCALIPROC DoCodeDBF(void)
{
	/* DBcc 0101cccc11001ddd */

	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = uint32_t_FromSWord(*dstp);

	--dstvalue;
#if LittleEndianUnaligned
	*(uint16_t *)dstp = dstvalue;
#else
	*dstp = (*dstp & ~ 0xffff) | ((dstvalue) & 0xffff);
#endif

	if ((int32_t)dstvalue == -1) {
#if WantCloserCyc
		V_MaxCyclesToGo -= (14 * kCycleScale + 3 * RdAvgXtraCyc);
#endif
		SkipiWord();
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (10 * kCycleScale + 2 * RdAvgXtraCyc);
#endif
		DoCodeBraW();
	}
}

#if WantCloserCyc
LOCALPROC DoCodeDBcc_t(void)
{
	V_MaxCyclesToGo -= (12 * kCycleScale + 2 * RdAvgXtraCyc);
	SkipiWord();
}
#else
#define DoCodeDBcc_t SkipiWord
#endif

LOCALIPROC DoCodeDBcc(void)
{
	/* DBcc 0101cccc11001ddd */

	cctrue(DoCodeDBcc_t, DoCodeDBF);
}

LOCALIPROC DoCodeSwap(void)
{
	/* Swap 0100100001000rrr */
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t src = *dstp;
	uint32_t dst = uint32_t_FromSLong(((src >> 16) & 0xFFFF)
		| ((src & 0xFFFF) << 16));

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dst;

	HaveSetUpFlags();

	*dstp = dst;
}

LOCALIPROC DoCodeMoveA(void) /* MOVE */
{
	uint32_t src = DecodeGetSrcValue();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;

	m68k_areg(dstreg) = src;
}

LOCALIPROC DoCodeMoveQ(void)
{
	/* MoveQ 0111ddd0nnnnnnnn */
	uint32_t src = uint32_t_FromSByte(V_regs.CurDecOpY.v[0].ArgDat);
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = src;

	HaveSetUpFlags();

	m68k_dreg(dstreg) = src;
}

LOCALIPROC DoCodeAddB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSByte(dstvalue + srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsAddB;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsAddB;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeAddW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSWord(dstvalue + srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsAddW;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsAddW;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeAddL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSLong(dstvalue + srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsAddL;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsAddL;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeSubB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSByte(dstvalue - srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsSubB;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsSubB;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeSubW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSWord(dstvalue - srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsSubW;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsSubW;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeSubL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;
	uint32_t result = uint32_t_FromSLong(dstvalue - srcvalue);

	V_regs.LazyFlagKind = kLazyFlagsSubL;
	V_regs.LazyFlagArgSrc = srcvalue;
	V_regs.LazyFlagArgDst = dstvalue;

	V_regs.LazyXFlagKind = kLazyFlagsSubL;
	V_regs.LazyXFlagArgSrc = srcvalue;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeLea(void)
{
	/* Lea 0100aaa111mmmrrr */
	uint32_t DstAddr = DecodeDst();
	uint32_t dstreg = V_regs.CurDecOpY.v[0].ArgDat;

	m68k_areg(dstreg) = DstAddr;
}

LOCALIPROC DoCodePEA(void)
{
	/* PEA 0100100001mmmrrr */
	uint32_t DstAddr = DecodeDst();

	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), DstAddr);
}

LOCALIPROC DoCodeBsrB(void)
{
	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), m68k_getpc());
	DoCodeBraB();
}

LOCALIPROC DoCodeBsrW(void)
{
	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), m68k_getpc() + 2);
	DoCodeBraW();
}

#define m68k_logExceptions (dbglog_HAVE && 0)


#ifndef WantDumpAJump
#define WantDumpAJump 0
#endif

#if WantDumpAJump
LOCALPROCUSEDONCE DumpAJump(CPTR toaddr)
{
	CPTR fromaddr = m68k_getpc();
	if ((toaddr > fromaddr) || (toaddr < V_regs.pc))
	{
		dbglog_writeHex(fromaddr);
		dbglog_writeCStr(",");
		dbglog_writeHex(toaddr);
		dbglog_writeReturn();
	}
}
#endif

LOCALPROC reg_call m68k_setpc(CPTR newpc)
{
#if WantDumpAJump
	DumpAJump(newpc);
#endif

#if 0
	if (newpc == 0xBD50 /* 401AB4 */) {
		/* Debugger(); */
		/* Exception(5); */ /* try and get macsbug */
	}
#endif

	V_pc_p = V_regs.pc_pLo + (newpc - V_regs.pc);
	if (cond_rare(V_pc_p >= V_pc_pHi)
		|| cond_rare(V_pc_p < V_regs.pc_pLo))
	{
		Recalc_PC_Block();
	}
}

LOCALIPROC DoCodeJsr(void)
{
	/* Jsr 0100111010mmmrrr */
	uint32_t DstAddr = DecodeDst();

	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), m68k_getpc());
	m68k_setpc(DstAddr);
}

LOCALIPROC DoCodeLinkA6(void)
{
	CPTR stackp = m68k_areg(7);
	stackp -= 4;
	put_long(stackp, m68k_areg(6));
	m68k_areg(6) = stackp;
	m68k_areg(7) = stackp + nextiSWord();
}

LOCALIPROC DoCodeUnlkA6(void)
{
	uint32_t src = m68k_areg(6);
	m68k_areg(6) = get_long(src);
	m68k_areg(7) = src + 4;
}

LOCALIPROC DoCodeRts(void)
{
	/* Rts 0100111001110101 */
	uint32_t NewPC = get_long(m68k_areg(7));
	m68k_areg(7) += 4;
	m68k_setpc(NewPC);
}

LOCALIPROC DoCodeJmp(void)
{
	/* JMP 0100111011mmmrrr */
	uint32_t DstAddr = DecodeDst();

	m68k_setpc(DstAddr);
}

LOCALIPROC DoCodeClr(void)
{
	/* Clr 01000010ssmmmrrr */

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = 0;

	HaveSetUpFlags();

	DecodeSetDstValue(0);
}

LOCALIPROC DoCodeAddA(void)
{
	/* ADDA 1101dddm11mmmrrr */
	uint32_t dstvalue = DecodeGetSrcSetDstValue();

	ArgSetDstValue(dstvalue + V_regs.SrcVal);
}

LOCALIPROC DoCodeSubA(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();

	ArgSetDstValue(dstvalue - V_regs.SrcVal);
}

LOCALIPROC DoCodeCmpA(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsCmpL;
	V_regs.LazyFlagArgSrc = V_regs.SrcVal;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();
}

LOCALFUNC uint16_t m68k_getCR(void)
{
	NeedDefaultLazyAllFlags();

	return (XFLG << 4) | (NFLG << 3) | (ZFLG << 2)
		| (VFLG << 1) | CFLG;
}

LOCALPROC reg_call m68k_setCR(uint16_t newcr)
{
	XFLG = (newcr >> 4) & 1;
	NFLG = (newcr >> 3) & 1;
	ZFLG = (newcr >> 2) & 1;
	VFLG = (newcr >> 1) & 1;
	CFLG = newcr & 1;

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;
}


LOCALFUNC uint16_t m68k_getSR(void)
{
	return m68k_getCR()
			| (V_regs.t1 << 15)
#if Use68020
			| (V_regs.t0 << 14)
#endif
			| (V_regs.s << 13)
#if Use68020
			| (V_regs.m << 12)
#endif
			| (V_regs.intmask << 8);
}

LOCALPROC NeedToGetOut(void)
{
	if (V_MaxCyclesToGo <= 0) {
		/*
			already have gotten out, and exception processing has
			caused another exception, such as because a bad
			stack pointer pointing to a memory mapped device.
		*/
	} else {
		V_regs.MoreCyclesToGo += V_MaxCyclesToGo;
			/* not counting the current instruction */
		V_MaxCyclesToGo = 0;
	}
}

LOCALPROC SetExternalInterruptPending(void)
{
	V_regs.ExternalInterruptPending = true;
	NeedToGetOut();
}

LOCALPROC reg_call m68k_setSR(uint16_t newsr)
{
	CPTR *pnewstk;
	CPTR *poldstk = (V_regs.s != 0) ? (
#if Use68020
		(V_regs.m != 0) ? &V_regs.msp :
#endif
		&V_regs.isp) : &V_regs.usp;
	uint32_t oldintmask = V_regs.intmask;

	V_regs.t1 = (newsr >> 15) & 1;
#if Use68020
	V_regs.t0 = (newsr >> 14) & 1;
	if (V_regs.t0 != 0) {
		ReportAbnormalID(0x0105, "t0 flag set in m68k_setSR");
	}
#endif
	V_regs.s = (newsr >> 13) & 1;
#if Use68020
	V_regs.m = (newsr >> 12) & 1;
	if (V_regs.m != 0) {
		ReportAbnormalID(0x0106, "m flag set in m68k_setSR");
	}
#endif
	V_regs.intmask = (newsr >> 8) & 7;

	pnewstk = (V_regs.s != 0) ? (
#if Use68020
		(V_regs.m != 0) ? &V_regs.msp :
#endif
		&V_regs.isp) : &V_regs.usp;

	if (poldstk != pnewstk) {
		*poldstk = m68k_areg(7);
		m68k_areg(7) = *pnewstk;
	}

	if (V_regs.intmask != oldintmask) {
		SetExternalInterruptPending();
	}

	if (V_regs.t1 != 0) {
		NeedToGetOut();
	} else {
		/* V_regs.TracePending = false; */
	}

	m68k_setCR(newsr);
}

LOCALPROC reg_call ExceptionTo(CPTR newpc
#if Use68020
	, int nr
#endif
	)
{
	uint16_t saveSR = m68k_getSR();

	if (0 == V_regs.s) {
		V_regs.usp = m68k_areg(7);
		m68k_areg(7) =
#if Use68020
			(V_regs.m != 0) ? V_regs.msp :
#endif
			V_regs.isp;
		V_regs.s = 1;
	}
#if Use68020
	switch (nr) {
		case 5: /* Zero Divide */
		case 6: /* CHK, CHK2 */
		case 7: /* cpTRAPcc, TRAPCcc, TRAPv */
		case 9: /* Trace */
			m68k_areg(7) -= 4;
			put_long(m68k_areg(7), m68k_getpc());
			m68k_areg(7) -= 2;
			put_word(m68k_areg(7), 0x2000 + nr * 4);
			break;
		default:
			m68k_areg(7) -= 2;
			put_word(m68k_areg(7), nr * 4);
			break;
	}
	/* if V_regs.m should make throw away stack frame */
#endif
	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), m68k_getpc());
	m68k_areg(7) -= 2;
	put_word(m68k_areg(7), saveSR);
	m68k_setpc(newpc);
	V_regs.t1 = 0;
#if Use68020
	V_regs.t0 = 0;
	V_regs.m = 0;
#endif
	V_regs.TracePending = false;
}

LOCALPROC reg_call Exception(int nr)
{
	ExceptionTo(get_long(4 * nr
#if Use68020
		+ V_regs.vbr
#endif
		)
#if Use68020
		, nr
#endif
		);
}


LOCALIPROC DoCodeA(void)
{
	BackupPC();
	Exception(0xA);
}

LOCALFUNC uint16_t nextiword_nm(void)
/* NOT sign extended */
{
	return nextiword();
}

LOCALIPROC DoCodeMOVEMRmML(void)
{
	/* MOVEM reg to mem 01001000111100rrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t p = *dstp;

#if Use68020
	{
		int n = 0;

		for (z = 0; z < 16; ++z) {
			if ((regmask & (1 << z)) != 0) {
				n++;
			}
		}
		*dstp = p - n * 4;
	}
#endif
	for (z = 16; --z >= 0; ) {
		if ((regmask & (1 << (15 - z))) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -= (8 * kCycleScale + 2 * WrAvgXtraCyc);
#endif
			p -= 4;
			put_long(p, V_regs.regs[z]);
		}
	}
#if ! Use68020
	*dstp = p;
#endif
}

LOCALIPROC DoCodeMOVEMApRL(void)
{
	/* MOVEM mem to reg 01001100111011rrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t p = *dstp;

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -= (8 * kCycleScale + 2 * RdAvgXtraCyc);
#endif
			V_regs.regs[z] = get_long(p);
			p += 4;
		}
	}
	*dstp = p;
}

LOCALPROC reg_call SetCCRforAddX(uint32_t dstvalue, uint32_t srcvalue,
	uint32_t result)
{
	ZFLG &= Bool2Bit(result == 0);

	{
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(srcvalue));
		flagtype flgo = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		flagtype flgsando = flgs & flgo;
		flagtype flgsoro = flgs | flgo;
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));

		NFLG = flgn;
		flgn ^= 1;
		VFLG = ((flgn | flgsoro) ^ 1) | (flgn & flgsando);
		XFLG = CFLG = flgsando | (flgn & flgsoro);
	}

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeAddXB(void)
{
	NeedDefaultLazyAllFlags();

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSByte(XFLG + dstvalue + srcvalue);

		SetCCRforAddX(dstvalue, srcvalue, result);
	}
}

LOCALIPROC DoCodeAddXW(void)
{
	if ((kLazyFlagsDefault != V_regs.LazyFlagKind)
		|| (kLazyFlagsDefault != V_regs.LazyXFlagKind))
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSWord(XFLG + dstvalue + srcvalue);

		SetCCRforAddX(dstvalue, srcvalue, result);
	}
}

LOCALIPROC DoCodeAddXL(void)
{
	if (kLazyFlagsAddL == V_regs.LazyFlagKind) {
		uint32_t src = V_regs.LazyFlagArgSrc;
		uint32_t dst = V_regs.LazyFlagArgDst;
		uint32_t result = uint32_t_FromULong(dst + src);

		ZFLG = Bool2Bit(result == 0);
		XFLG = Bool2Bit(result < src);

		V_regs.LazyFlagKind = kLazyFlagsDefault;
		V_regs.LazyXFlagKind = kLazyFlagsDefault;
	} else
	if ((kLazyFlagsDefault == V_regs.LazyFlagKind)
		&& (kLazyFlagsDefault == V_regs.LazyXFlagKind))
	{
		/* ok */
	} else
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSLong(XFLG + dstvalue + srcvalue);

		SetCCRforAddX(dstvalue, srcvalue, result);
	}
}

LOCALPROC reg_call SetCCRforSubX(uint32_t dstvalue, uint32_t srcvalue,
	uint32_t result)
{
	ZFLG &= Bool2Bit(result == 0);

	{
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(srcvalue));
		flagtype flgo = Bool2Bit(uint32_t_MSBisSet(dstvalue)) ^ 1;
		flagtype flgsando = flgs & flgo;
		flagtype flgsoro = flgs | flgo;
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));

		NFLG = flgn;
		VFLG = ((flgn | flgsoro) ^ 1) | (flgn & flgsando);
		XFLG = CFLG = flgsando | (flgn & flgsoro);
	}

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeSubXB(void)
{
	NeedDefaultLazyAllFlags();

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSByte(dstvalue - srcvalue - XFLG);

		SetCCRforSubX(dstvalue, srcvalue, result);
	}
}

LOCALIPROC DoCodeSubXW(void)
{
	if ((kLazyFlagsDefault != V_regs.LazyFlagKind)
		|| (kLazyFlagsDefault != V_regs.LazyXFlagKind))
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSWord(dstvalue - srcvalue - XFLG);

		SetCCRforSubX(dstvalue, srcvalue, result);
	}
}

LOCALIPROC DoCodeSubXL(void)
{
	if (kLazyFlagsSubL == V_regs.LazyFlagKind) {
		uint32_t src = V_regs.LazyFlagArgSrc;
		uint32_t dst = V_regs.LazyFlagArgDst;
		uint32_t result = uint32_t_FromSLong(dst - src);

		ZFLG = Bool2Bit(result == 0);
		XFLG = Bool2Bit(((uint32_t)dst) < ((uint32_t)src));

		V_regs.LazyFlagKind = kLazyFlagsDefault;
		V_regs.LazyXFlagKind = kLazyFlagsDefault;
	} else
	if ((kLazyFlagsDefault == V_regs.LazyFlagKind)
		&& (kLazyFlagsDefault == V_regs.LazyXFlagKind))
	{
		/* ok */
	} else
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSrcSetDstValue();
		uint32_t srcvalue = V_regs.SrcVal;
		uint32_t result = uint32_t_FromSLong(dstvalue - srcvalue - XFLG);

		SetCCRforSubX(dstvalue, srcvalue, result);
	}
}

LOCALPROC reg_call DoCodeNullShift(uint32_t dstvalue)
{
	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(dstvalue);
}

LOCALPROC DoCodeOverAsl(uint32_t dstvalue)
{
	XFLG = CFLG = 0;
	VFLG = Bool2Bit(0 != dstvalue);
	ZFLG = 1;
	NFLG = 0;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;

	ArgSetDstValue(0);
}

LOCALPROC reg_call DoCodeMaxAsr(uint32_t dstvalue)
{
	XFLG = CFLG = dstvalue & 1;
	VFLG = Bool2Bit(0 != dstvalue);
	ZFLG = 1;
	NFLG = 0;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;

	ArgSetDstValue(0);
}

LOCALIPROC DoCodeAslB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 8) {
			if (cnt == 8) {
				DoCodeMaxAsr(dstvalue);
			} else {
				DoCodeOverAsl(dstvalue);
			}
		} else {
			uint32_t result = uint32_t_FromSByte(dstvalue << cnt);

			V_regs.LazyFlagKind = kLazyFlagsAslB;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAslB;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALIPROC DoCodeAslW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 16) {
			if (cnt == 16) {
				DoCodeMaxAsr(dstvalue);
			} else {
				DoCodeOverAsl(dstvalue);
			}
		} else {
			uint32_t result = uint32_t_FromSWord(dstvalue << cnt);

			V_regs.LazyFlagKind = kLazyFlagsAslW;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAslW;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALIPROC DoCodeAslL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 32) {
			if (cnt == 32) {
				DoCodeMaxAsr(dstvalue);
			} else {
				DoCodeOverAsl(dstvalue);
			}
		} else {
			uint32_t result = uint32_t_FromSLong(dstvalue << cnt);

			V_regs.LazyFlagKind = kLazyFlagsAslL;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAslL;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALPROC DoCodeOverShift(void)
{
	XFLG = CFLG = 0;
	ZFLG = 1;
	NFLG = 0;
	VFLG = 0;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;

	ArgSetDstValue(0);
}

LOCALPROC DoCodeOverShiftN(void)
{
	NFLG = 1;
	VFLG = 0;
	CFLG = 1;
	XFLG = CFLG;
	ZFLG = 0;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;

	ArgSetDstValue(~ 0);
}

LOCALPROC DoCodeOverAShift(uint32_t dstvalue)
{
	if (uint32_t_MSBisSet(dstvalue)) {
		DoCodeOverShiftN();
	} else {
		DoCodeOverShift();
	}
}

LOCALIPROC DoCodeAsrB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 8) {
			DoCodeOverAShift(dstvalue);
		} else {
			uint32_t result = Ui5rASR(dstvalue, cnt);

			V_regs.LazyFlagKind = kLazyFlagsAsrB;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAsrB;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALIPROC DoCodeAsrW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 16) {
			DoCodeOverAShift(dstvalue);
		} else {
			uint32_t result = Ui5rASR(dstvalue, cnt);

			V_regs.LazyFlagKind = kLazyFlagsAsrW;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAsrW;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALIPROC DoCodeAsrL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 32) {
			DoCodeOverAShift(dstvalue);
		} else {
			uint32_t result = Ui5rASR(dstvalue, cnt);

			V_regs.LazyFlagKind = kLazyFlagsAsrL;
			V_regs.LazyFlagArgSrc = cnt;
			V_regs.LazyFlagArgDst = dstvalue;

			V_regs.LazyXFlagKind = kLazyFlagsAsrL;
			V_regs.LazyXFlagArgSrc = cnt;
			V_regs.LazyXFlagArgDst = dstvalue;

			HaveSetUpFlags();

			ArgSetDstValue(result);
		}
	}
}

LOCALPROC reg_call DoCodeMaxLslShift(uint32_t dstvalue)
{
	XFLG = CFLG = dstvalue & 1;
	ZFLG = 1;
	NFLG = 0;
	VFLG = 0;

	V_regs.LazyXFlagKind = kLazyFlagsDefault;
	V_regs.LazyFlagKind = kLazyFlagsDefault;

	ArgSetDstValue(0);
}

LOCALIPROC DoCodeLslB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 8) {
			if (cnt == 8) {
				DoCodeMaxLslShift(dstvalue);
			} else {
				DoCodeOverShift();
			}
		} else {
			CFLG = (dstvalue >> (8 - cnt)) & 1;
			dstvalue = dstvalue << cnt;
			dstvalue = uint32_t_FromSByte(dstvalue);

			ZFLG = Bool2Bit(dstvalue == 0);
			NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			VFLG = 0;
			XFLG = CFLG;
			V_regs.LazyXFlagKind = kLazyFlagsDefault;
			V_regs.LazyFlagKind = kLazyFlagsDefault;

			ArgSetDstValue(dstvalue);
		}
	}
}

LOCALIPROC DoCodeLslW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 16) {
			if (cnt == 16) {
				DoCodeMaxLslShift(dstvalue);
			} else {
				DoCodeOverShift();
			}
		} else {
			CFLG = (dstvalue >> (16 - cnt)) & 1;
			dstvalue = dstvalue << cnt;
			dstvalue = uint32_t_FromSWord(dstvalue);

			ZFLG = Bool2Bit(dstvalue == 0);
			NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			VFLG = 0;
			XFLG = CFLG;
			V_regs.LazyXFlagKind = kLazyFlagsDefault;
			V_regs.LazyFlagKind = kLazyFlagsDefault;

			ArgSetDstValue(dstvalue);
		}
	}
}

LOCALIPROC DoCodeLslL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		if (cnt >= 32) {
			if (cnt == 32) {
				DoCodeMaxLslShift(dstvalue);
			} else {
				DoCodeOverShift();
			}
		} else {
			CFLG = (dstvalue >> (32 - cnt)) & 1;
			dstvalue = dstvalue << cnt;
			dstvalue = uint32_t_FromSLong(dstvalue);

			ZFLG = Bool2Bit(dstvalue == 0);
			NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			VFLG = 0;
			XFLG = CFLG;
			V_regs.LazyXFlagKind = kLazyFlagsDefault;
			V_regs.LazyFlagKind = kLazyFlagsDefault;

			ArgSetDstValue(dstvalue);
		}
	}
}

LOCALIPROC DoCodeLsrB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else if (cnt > 32) {
		DoCodeOverShift();
	} else {
		dstvalue = uint32_t_FromUByte(dstvalue);
		dstvalue = dstvalue >> (cnt - 1);
		CFLG = XFLG = (dstvalue & 1);
		dstvalue = dstvalue >> 1;
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = 0 /* Bool2Bit(uint32_t_MSBisSet(dstvalue)) */;
			/* if cnt != 0, always false */
		VFLG = 0;
		V_regs.LazyXFlagKind = kLazyFlagsDefault;
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeLsrW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else if (cnt > 32) {
		DoCodeOverShift();
	} else {
		dstvalue = uint32_t_FromUWord(dstvalue);
		dstvalue = dstvalue >> (cnt - 1);
		CFLG = XFLG = (dstvalue & 1);
		dstvalue = dstvalue >> 1;
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = 0 /* Bool2Bit(uint32_t_MSBisSet(dstvalue)) */;
			/* if cnt != 0, always false */
		VFLG = 0;
		V_regs.LazyXFlagKind = kLazyFlagsDefault;
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeLsrL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else if (cnt > 32) {
		DoCodeOverShift();
	} else {
		dstvalue = uint32_t_FromULong(dstvalue);
		dstvalue = dstvalue >> (cnt - 1);
		CFLG = XFLG = (dstvalue & 1);
		dstvalue = dstvalue >> 1;
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = 0 /* Bool2Bit(uint32_t_MSBisSet(dstvalue)) */;
			/* if cnt != 0, always false */
		VFLG = 0;
		V_regs.LazyXFlagKind = kLazyFlagsDefault;
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALFUNC uint32_t DecodeGetSrcSetDstValueDfltFlags_nm(void)
{
	NeedDefaultLazyAllFlags();

	return DecodeGetSrcSetDstValue();
}

LOCALPROC reg_call DoCodeNullXShift(uint32_t dstvalue)
{
	CFLG = XFLG;

	ZFLG = Bool2Bit(dstvalue == 0);
	NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
	VFLG = 0;

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeRxlB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		for (; cnt; --cnt) {
			CFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			dstvalue = (dstvalue << 1) | XFLG;
			dstvalue = uint32_t_FromSByte(dstvalue);
			XFLG = CFLG;
		}

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRxlW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		for (; cnt; --cnt) {
			CFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			dstvalue = (dstvalue << 1) | XFLG;
			dstvalue = uint32_t_FromSWord(dstvalue);
			XFLG = CFLG;
		}

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRxlL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		for (; cnt; --cnt) {
			CFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
			dstvalue = (dstvalue << 1) | XFLG;
			dstvalue = uint32_t_FromSLong(dstvalue);
			XFLG = CFLG;
		}

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRxrB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		dstvalue = uint32_t_FromUByte(dstvalue);
		for (; cnt; --cnt) {
			CFLG = dstvalue & 1;
			dstvalue = (dstvalue >> 1) | (((uint32_t)XFLG) << 7);
			XFLG = CFLG;
		}
		dstvalue = uint32_t_FromSByte(dstvalue);

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRxrW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		dstvalue = uint32_t_FromUWord(dstvalue);
		for (; cnt; --cnt) {
			CFLG = dstvalue & 1;
			dstvalue = (dstvalue >> 1) | (((uint32_t)XFLG) << 15);
			XFLG = CFLG;
		}
		dstvalue = uint32_t_FromSWord(dstvalue);

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRxrL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t cnt = V_regs.SrcVal & 63;

	if (0 == cnt) {
		DoCodeNullXShift(dstvalue);
	} else {
#if WantCloserCyc
		V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

		dstvalue = uint32_t_FromULong(dstvalue);
		for (; cnt; --cnt) {
			CFLG = dstvalue & 1;
			dstvalue = (dstvalue >> 1) | (((uint32_t)XFLG) << 31);
			XFLG = CFLG;
		}
		dstvalue = uint32_t_FromSLong(dstvalue);

		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRolB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 7;
		if (0 != cnt) {
			uint8_t dst = (uint8_t)dstvalue;

			dst = (dst >> (8 - cnt))
					| ((dst & ((1 << (8 - cnt)) - 1))
						<< cnt);

			dstvalue = (uint32_t)(int32_t)(int8_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = (dstvalue & 1);
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRolW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 15;
		if (0 != cnt) {
			uint16_t dst = (uint16_t)dstvalue;

			dst = (dst >> (16 - cnt))
					| ((dst & ((1 << (16 - cnt)) - 1))
						<< cnt);

			dstvalue = (uint32_t)(int32_t)(int16_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = (dstvalue & 1);
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRolL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 31;
		if (0 != cnt) {
			uint32_t dst = (uint32_t)dstvalue;

			dst = (dst >> (32 - cnt))
					| ((dst & ((1 << (32 - cnt)) - 1))
						<< cnt);

			dstvalue = (uint32_t)(int32_t)(int32_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = (dstvalue & 1);
		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRorB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 7;
		if (0 != cnt) {
			uint8_t dst = (uint8_t)dstvalue;

			dst = (dst >> cnt)
					| ((dst & ((1 << cnt) - 1))
						<< (8 - cnt));

			dstvalue = (uint32_t)(int32_t)(int8_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = NFLG;

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRorW(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 15;
		if (0 != cnt) {
			uint16_t dst = (uint16_t)dstvalue;

			dst = (dst >> cnt)
					| ((dst & ((1 << cnt) - 1))
						<< (16 - cnt));

			dstvalue = (uint32_t)(int32_t)(int16_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = NFLG;

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}

LOCALIPROC DoCodeRorL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValue();
	uint32_t cnt = V_regs.SrcVal & 63;

#if WantCloserCyc
	V_MaxCyclesToGo -= (cnt * 2 * kCycleScale);
#endif

	if (0 == cnt) {
		DoCodeNullShift(dstvalue);
	} else {
		cnt &= 31;
		if (0 != cnt) {
			uint32_t dst = (uint32_t)dstvalue;

			dst = (dst >> cnt)
					| ((dst & ((1 << cnt) - 1))
						<< (32 - cnt));

			dstvalue = (uint32_t)(int32_t)(int32_t)dst;
		}
		ZFLG = Bool2Bit(dstvalue == 0);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = 0;
		CFLG = NFLG;

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		ArgSetDstValue(dstvalue);
	}
}


#if UseLazyZ
LOCALPROC WillSetZFLG(void)
{
	if (kLazyFlagsZSet == V_regs.LazyFlagKind) {
		/* ok */
	} else if (kLazyFlagsDefault == V_regs.LazyFlagKind) {
		/* also ok */
	} else {
		V_regs.LazyFlagZSavedKind = V_regs.LazyFlagKind;
		V_regs.LazyFlagKind = kLazyFlagsZSet;
	}
}
#else
#define WillSetZFLG NeedDefaultLazyAllFlags
#endif

LOCALINLINEFUNC uint32_t DecodeGetSrcGetDstValueSetZ(void)
{
	WillSetZFLG();

	return DecodeGetSrcSetDstValue();
}

LOCALIPROC DoCodeBTstB(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 7;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;
}

LOCALIPROC DoCodeBTstL(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 31;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;
}

LOCALINLINEFUNC uint32_t DecodeGetSrcSetDstValueSetZ(void)
{
	WillSetZFLG();

	return DecodeGetSrcSetDstValue();
}

LOCALIPROC DoCodeBChgB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 7;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue ^= (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeBChgL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 31;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue ^= (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeBClrB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 7;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue &= ~ (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeBClrL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 31;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue &= ~ (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeBSetB(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 7;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue |= (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeBSetL(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueSetZ();
	uint32_t srcvalue = V_regs.SrcVal & 31;

	ZFLG = ((dstvalue >> srcvalue) ^ 1) & 1;

	dstvalue |= (1 << srcvalue);
	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeAnd(void)
{
	/* DoBinOpAnd(DecodeI_xxxxxxxxssmmmrrr()); */
	uint32_t dstvalue = DecodeGetSrcSetDstValue();

	dstvalue &= V_regs.SrcVal;
		/*
			don't need to extend, since excess high
			bits all the same as desired high bit.
		*/

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeOr(void)
{
	/* DoBinOr(DecodeI_xxxxxxxxssmmmrrr()); */
	uint32_t dstvalue = DecodeGetSrcSetDstValue();

	dstvalue |= V_regs.SrcVal;
		/*
			don't need to extend, since excess high
			bits all the same as desired high bit.
		*/

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeEor(void)
{
	/* Eor 1011ddd1ssmmmrrr */
	/* DoBinOpEor(DecodeDEa_xxxxdddxssmmmrrr()); */
	uint32_t dstvalue = DecodeGetSrcSetDstValue();

	dstvalue ^= V_regs.SrcVal;
		/*
			don't need to extend, since excess high
			bits all the same as desired high bit.
		*/

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeNot(void)
{
	/* Not 01000110ssmmmrrr */
	uint32_t dstvalue = DecodeGetSetDstValue();

	dstvalue = ~ dstvalue;

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(dstvalue);
}

LOCALPROC DoCodeScc_t(void)
{
#if WantCloserCyc
	if (kAMdRegB == V_regs.CurDecOpY.v[1].AMd) {
		V_MaxCyclesToGo -= (2 * kCycleScale);
	}
#endif
	DecodeSetDstValue(0xff);
}

LOCALPROC DoCodeScc_f(void)
{
	DecodeSetDstValue(0);
}

LOCALIPROC DoCodeScc(void)
{
	/* Scc 0101cccc11mmmrrr */
	cctrue(DoCodeScc_t, DoCodeScc_f);
}

LOCALIPROC DoCodeEXTL(void)
{
	/* EXT.L */
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = uint32_t_FromSWord(*dstp);

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	*dstp = dstvalue;
}

LOCALIPROC DoCodeEXTW(void)
{
	/* EXT.W */
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = uint32_t_FromSByte(*dstp);

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

#if LittleEndianUnaligned
	*(uint16_t *)dstp = dstvalue;
#else
	*dstp = (*dstp & ~ 0xffff) | (dstvalue & 0xffff);
#endif
}

LOCALIPROC DoCodeNegB(void)
{
	uint32_t dstvalue = DecodeGetSetDstValue();
	uint32_t result = uint32_t_FromSByte(0 - dstvalue);

	V_regs.LazyFlagKind = kLazyFlagsNegB;
	V_regs.LazyFlagArgDst = dstvalue;
	V_regs.LazyXFlagKind = kLazyFlagsNegB;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeNegW(void)
{
	uint32_t dstvalue = DecodeGetSetDstValue();
	uint32_t result = uint32_t_FromSWord(0 - dstvalue);

	V_regs.LazyFlagKind = kLazyFlagsNegW;
	V_regs.LazyFlagArgDst = dstvalue;
	V_regs.LazyXFlagKind = kLazyFlagsNegW;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeNegL(void)
{
	uint32_t dstvalue = DecodeGetSetDstValue();
	uint32_t result = uint32_t_FromSLong(0 - dstvalue);

	V_regs.LazyFlagKind = kLazyFlagsNegL;
	V_regs.LazyFlagArgDst = dstvalue;
	V_regs.LazyXFlagKind = kLazyFlagsNegL;
	V_regs.LazyXFlagArgDst = dstvalue;

	HaveSetUpFlags();

	ArgSetDstValue(result);
}

LOCALPROC reg_call SetCCRforNegX(uint32_t dstvalue, uint32_t result)
{
	ZFLG &= Bool2Bit(result == 0);

	{
		flagtype flgs = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		flagtype flgn = Bool2Bit(uint32_t_MSBisSet(result));

		NFLG = flgn;
		VFLG = flgs & flgn;
		XFLG = CFLG = flgs | flgn;
	}

	ArgSetDstValue(result);
}

LOCALIPROC DoCodeNegXB(void)
{
	NeedDefaultLazyAllFlags();

	{
		uint32_t dstvalue = DecodeGetSetDstValue();
		uint32_t result = uint32_t_FromSByte(0 - (XFLG + dstvalue));

		SetCCRforNegX(dstvalue, result);
	}
}

LOCALIPROC DoCodeNegXW(void)
{
	if ((kLazyFlagsDefault != V_regs.LazyFlagKind)
		|| (kLazyFlagsDefault != V_regs.LazyXFlagKind))
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSetDstValue();
		uint32_t result = uint32_t_FromSWord(0 - (XFLG + dstvalue));

		SetCCRforNegX(dstvalue, result);
	}
}

LOCALIPROC DoCodeNegXL(void)
{
	if (kLazyFlagsNegL == V_regs.LazyFlagKind) {
		NeedDefaultLazyFlagsNegL();
	} else
	if ((kLazyFlagsDefault == V_regs.LazyFlagKind)
		&& (kLazyFlagsDefault == V_regs.LazyXFlagKind))
	{
		/* ok */
	} else
	{
		NeedDefaultLazyAllFlags();
	}

	{
		uint32_t dstvalue = DecodeGetSetDstValue();
		uint32_t result = uint32_t_FromSLong(0 - (XFLG + dstvalue));

		SetCCRforNegX(dstvalue, result);
	}
}

LOCALIPROC DoCodeMulU(void)
{
	/* MulU 1100ddd011mmmrrr */
	uint32_t srcvalue = DecodeGetSrcValue();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = *dstp;

	dstvalue = uint32_t_FromSLong(uint32_t_FromUWord(dstvalue)
		* uint32_t_FromUWord(srcvalue));
#if WantCloserCyc
	{
		uint32_t v = srcvalue;

		while (v != 0) {
			if ((v & 1) != 0) {
				V_MaxCyclesToGo -= (2 * kCycleScale);
			}
			v >>= 1;
		}
	}
#endif

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	*dstp = dstvalue;
}

LOCALIPROC DoCodeMulS(void)
{
	/* MulS 1100ddd111mmmrrr */
	uint32_t srcvalue = DecodeGetSrcValue();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = *dstp;

	dstvalue = uint32_t_FromSLong((int32_t)(int16_t)dstvalue
		* (int32_t)(int16_t)srcvalue);
#if WantCloserCyc
	{
		uint32_t v = (srcvalue << 1);

		while (v != 0) {
			if ((v & 1) != ((v >> 1) & 1)) {
				V_MaxCyclesToGo -= (2 * kCycleScale);
			}
			v >>= 1;
		}
	}
#endif

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	*dstp = dstvalue;
}

LOCALIPROC DoCodeDivU(void)
{
	/* DivU 1000ddd011mmmrrr */
	uint32_t srcvalue = DecodeGetSrcValue();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = *dstp;

	if (srcvalue == 0) {
#if WantCloserCyc
		V_MaxCyclesToGo -=
			(38 * kCycleScale + 3 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		Exception(5);
#if m68k_logExceptions
		dbglog_WriteNote("*** zero devide exception");
#endif
	} else {
		uint32_t newv = (uint32_t)dstvalue / (uint32_t)(uint16_t)srcvalue;
		uint32_t rem = (uint32_t)dstvalue % (uint32_t)(uint16_t)srcvalue;
#if WantCloserCyc
		V_MaxCyclesToGo -= (133 * kCycleScale);
#endif
		if (newv > 0xffff) {
			NeedDefaultLazyAllFlags();

			VFLG = NFLG = 1;
			CFLG = 0;
		} else {
			VFLG = CFLG = 0;
			ZFLG = Bool2Bit(((int16_t)(newv)) == 0);
			NFLG = Bool2Bit(((int16_t)(newv)) < 0);

			V_regs.LazyFlagKind = kLazyFlagsDefault;

			newv = (newv & 0xffff) | ((uint32_t)rem << 16);
			dstvalue = newv;
		}
	}

	*dstp = dstvalue;
}

LOCALIPROC DoCodeDivS(void)
{
	/* DivS 1000ddd111mmmrrr */
	uint32_t srcvalue = DecodeGetSrcValue();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = *dstp;

	if (srcvalue == 0) {
#if WantCloserCyc
		V_MaxCyclesToGo -=
			(38 * kCycleScale + 3 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		Exception(5);
#if m68k_logExceptions
		dbglog_WriteNote("*** zero devide exception");
#endif
	} else {
		int32_t newv = (int32_t)dstvalue / (int32_t)(int16_t)srcvalue;
		uint16_t rem = (int32_t)dstvalue % (int32_t)(int16_t)srcvalue;
#if WantCloserCyc
		V_MaxCyclesToGo -= (150 * kCycleScale);
#endif
		if (((newv & 0xffff8000) != 0) &&
			((newv & 0xffff8000) != 0xffff8000))
		{
			NeedDefaultLazyAllFlags();

			VFLG = NFLG = 1;
			CFLG = 0;
		} else {
			if (((int16_t)rem < 0) != ((int32_t)dstvalue < 0)) {
				rem = - rem;
			}
			VFLG = CFLG = 0;
			ZFLG = Bool2Bit(((int16_t)(newv)) == 0);
			NFLG = Bool2Bit(((int16_t)(newv)) < 0);

			V_regs.LazyFlagKind = kLazyFlagsDefault;

			newv = (newv & 0xffff) | ((uint32_t)rem << 16);
			dstvalue = newv;
		}
	}

	*dstp = dstvalue;
}

LOCALIPROC DoCodeExg(void)
{
	/* Exg dd 1100ddd101000rrr, opsize = 4 */
	/* Exg aa 1100ddd101001rrr, opsize = 4 */
	/* Exg da 1100ddd110001rrr, opsize = 4 */

	uint32_t srcreg = V_regs.CurDecOpY.v[0].ArgDat;
	uint32_t *srcp = &V_regs.regs[srcreg];
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t srcvalue = *srcp;

	*srcp = *dstp;
	*dstp = srcvalue;
}

LOCALIPROC DoCodeMoveEaCR(void)
{
	/* 0100010011mmmrrr */
	m68k_setCR(DecodeGetDstValue());
}

LOCALPROC DoPrivilegeViolation(void)
{
#if WantCloserCyc
	V_MaxCyclesToGo += GetDcoCycles(V_regs.CurDecOp);
	V_MaxCyclesToGo -=
		(34 * kCycleScale + 4 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
	BackupPC();
	Exception(8);
#if m68k_logExceptions
	dbglog_WriteNote("*** Privilege Violation exception");
#endif
}

LOCALIPROC DoCodeMoveSREa(void)
{
	/* Move from SR 0100000011mmmrrr */
#if Use68020
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else
#endif
	{
		DecodeSetDstValue(m68k_getSR());
	}
}

LOCALIPROC DoCodeMoveEaSR(void)
{
	/* 0100011011mmmrrr */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		m68k_setSR(DecodeGetDstValue());
	}
}

LOCALIPROC DoCodeOrISR(void)
{
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		V_regs.SrcVal = nextiword_nm();

		m68k_setSR(m68k_getSR() | V_regs.SrcVal);
	}
}

LOCALIPROC DoCodeAndISR(void)
{
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		V_regs.SrcVal = nextiword_nm();

		m68k_setSR(m68k_getSR() & V_regs.SrcVal);
	}
}

LOCALIPROC DoCodeEorISR(void)
{
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		V_regs.SrcVal = nextiword_nm();

		m68k_setSR(m68k_getSR() ^ V_regs.SrcVal);
	}
}

LOCALIPROC DoCodeOrICCR(void)
{
	V_regs.SrcVal = nextiword_nm();

	m68k_setCR(m68k_getCR() | V_regs.SrcVal);
}

LOCALIPROC DoCodeAndICCR(void)
{
	V_regs.SrcVal = nextiword_nm();

	m68k_setCR(m68k_getCR() & V_regs.SrcVal);
}

LOCALIPROC DoCodeEorICCR(void)
{
	V_regs.SrcVal = nextiword_nm();

	m68k_setCR(m68k_getCR() ^ V_regs.SrcVal);
}

LOCALIPROC DoCodeMOVEMApRW(void)
{
	/* MOVEM mem to reg 01001100110011rrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t p = *dstp;

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -= (4 * kCycleScale + RdAvgXtraCyc);
#endif
			V_regs.regs[z] = get_word(p);
			p += 2;
		}
	}
	*dstp = p;
}

LOCALIPROC DoCodeMOVEMRmMW(void)
{
	/* MOVEM reg to mem 01001000110100rrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t p = *dstp;

#if Use68020
	{
		int n = 0;

		for (z = 0; z < 16; ++z) {
			if ((regmask & (1 << z)) != 0) {
				n++;
			}
		}
		*dstp = p - n * 2;
	}
#endif
	for (z = 16; --z >= 0; ) {
		if ((regmask & (1 << (15 - z))) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -= (4 * kCycleScale + WrAvgXtraCyc);
#endif
			p -= 2;
			put_word(p, V_regs.regs[z]);
		}
	}
#if ! Use68020
	*dstp = p;
#endif
}

LOCALIPROC DoCodeMOVEMrmW(void)
{
	/* MOVEM reg to mem 010010001ssmmmrrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t p = DecodeDst();

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -=
				(4 * kCycleScale + WrAvgXtraCyc);
#endif
			put_word(p, V_regs.regs[z]);
			p += 2;
		}
	}
}

LOCALIPROC DoCodeMOVEMrmL(void)
{
	/* MOVEM reg to mem 010010001ssmmmrrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t p = DecodeDst();

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -=
				(8 * kCycleScale + 2 * WrAvgXtraCyc);
#endif
			put_long(p, V_regs.regs[z]);
			p += 4;
		}
	}
}

LOCALIPROC DoCodeMOVEMmrW(void)
{
	/* MOVEM mem to reg 0100110011smmmrrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t p = DecodeDst();

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -=
				(4 * kCycleScale + RdAvgXtraCyc);
#endif
			V_regs.regs[z] = get_word(p);
			p += 2;
		}
	}
}

LOCALIPROC DoCodeMOVEMmrL(void)
{
	/* MOVEM mem to reg 0100110011smmmrrr */
	int16_t z;
	uint32_t regmask = nextiword_nm();
	uint32_t p = DecodeDst();

	for (z = 0; z < 16; ++z) {
		if ((regmask & (1 << z)) != 0) {
#if WantCloserCyc
			V_MaxCyclesToGo -=
				(8 * kCycleScale + 2 * RdAvgXtraCyc);
#endif
			V_regs.regs[z] = get_long(p);
			p += 4;
		}
	}
}

LOCALIPROC DoCodeAbcd(void)
{
	/* ABCD r 1100ddd100000rrr */
	/* ABCD m 1100ddd100001rrr */

	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t srcvalue = V_regs.SrcVal;

	{
		/* if (V_regs.opsize != 1) a bug */
		int flgs = uint32_t_MSBisSet(srcvalue);
		int flgo = uint32_t_MSBisSet(dstvalue);
		uint16_t newv_lo =
			(srcvalue & 0xF) + (dstvalue & 0xF) + XFLG;
		uint16_t newv_hi = (srcvalue & 0xF0) + (dstvalue & 0xF0);
		uint16_t newv;

		if (newv_lo > 9) {
			newv_lo += 6;
		}
		newv = newv_hi + newv_lo;
		CFLG = XFLG = Bool2Bit((newv & 0x1F0) > 0x90);
		if (CFLG != 0) {
			newv += 0x60;
		}
		dstvalue = uint32_t_FromSByte(newv);
		if (dstvalue != 0) {
			ZFLG = 0;
		}
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = Bool2Bit((flgs != flgo) && ((NFLG != 0) != flgo));
		/*
			but according to my reference book,
			VFLG is Undefined for ABCD
		*/
	}

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeSbcd(void)
{
	uint32_t dstvalue = DecodeGetSrcSetDstValueDfltFlags_nm();
	uint32_t srcvalue = V_regs.SrcVal;

	{
		int flgs = uint32_t_MSBisSet(srcvalue);
		int flgo = uint32_t_MSBisSet(dstvalue);
		uint16_t newv_lo =
			(dstvalue & 0xF) - (srcvalue & 0xF) - XFLG;
		uint16_t newv_hi = (dstvalue & 0xF0) - (srcvalue & 0xF0);
		uint16_t newv;

		if (newv_lo > 9) {
			newv_lo -= 6;
			newv_hi -= 0x10;
		}
		newv = newv_hi + (newv_lo & 0xF);
		CFLG = XFLG = Bool2Bit((newv_hi & 0x1F0) > 0x90);
		if (CFLG != 0) {
			newv -= 0x60;
		}
		dstvalue = uint32_t_FromSByte(newv);
		if (dstvalue != 0) {
			ZFLG = 0;
		}
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		VFLG = Bool2Bit((flgs != flgo) && ((NFLG != 0) != flgo));
		/*
			but according to my reference book,
			VFLG is Undefined for SBCD
		*/
	}

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeNbcd(void)
{
	/* Nbcd 0100100000mmmrrr */
	uint32_t dstvalue = DecodeGetSetDstValue();

	NeedDefaultLazyAllFlags();

	{
		uint16_t newv_lo = - (dstvalue & 0xF) - XFLG;
		uint16_t newv_hi = - (dstvalue & 0xF0);
		uint16_t newv;

		if (newv_lo > 9) {
			newv_lo -= 6;
			newv_hi -= 0x10;
		}
		newv = newv_hi + (newv_lo & 0xF);
		CFLG = XFLG = Bool2Bit((newv_hi & 0x1F0) > 0x90);
		if (CFLG != 0) {
			newv -= 0x60;
		}

		dstvalue = uint32_t_FromSByte(newv);
		NFLG = Bool2Bit(uint32_t_MSBisSet(dstvalue));
		if (dstvalue != 0) {
			ZFLG = 0;
		}
	}

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeRte(void)
{
	/* Rte 0100111001110011 */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint32_t NewPC;
		CPTR stackp = m68k_areg(7);
		uint32_t NewSR = get_word(stackp);
		stackp += 2;
		NewPC = get_long(stackp);
		stackp += 4;

#if Use68020
		{
			uint16_t format = get_word(stackp);
			stackp += 2;

			switch ((format >> 12) & 0x0F) {
				case 0:
					/* ReportAbnormal("rte stack frame format 0"); */
					break;
				case 1:
					ReportAbnormalID(0x0107,
						"rte stack frame format 1");
					NewPC = m68k_getpc() - 2;
						/* rerun instruction */
					break;
				case 2:
					ReportAbnormalID(0x0108,
						"rte stack frame format 2");
					stackp += 4;
					break;
				case 9:
					ReportAbnormalID(0x0109,
						"rte stack frame format 9");
					stackp += 12;
					break;
				case 10:
					ReportAbnormalID(0x010A,
						"rte stack frame format 10");
					stackp += 24;
					break;
				case 11:
					ReportAbnormalID(0x010B,
						"rte stack frame format 11");
					stackp += 84;
					break;
				default:
					ReportAbnormalID(0x010C,
						"unknown rte stack frame format");
					Exception(14);
					return;
					break;
			}
		}
#endif
		m68k_areg(7) = stackp;
		m68k_setSR(NewSR);
		m68k_setpc(NewPC);
	}
}

LOCALIPROC DoCodeNop(void)
{
	/* Nop 0100111001110001 */
}

LOCALIPROC DoCodeMoveP0(void)
{
	/* MoveP 0000ddd1mm001aaa */
	uint32_t srcreg = V_regs.CurDecOpY.v[0].ArgDat;
	uint32_t *srcp = &V_regs.regs[srcreg];
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];

	uint32_t Displacement = nextiword_nm();
		/* shouldn't this sign extend ? */
	CPTR memp = *srcp + Displacement;

	uint16_t val = ((get_byte(memp) & 0x00FF) << 8)
		| (get_byte(memp + 2) & 0x00FF);

	*dstp =
		(*dstp & ~ 0xffff) | (val & 0xffff);

#if 0
	if ((Displacement & 0x00008000) != 0) {
		/* **** for testing only **** */
		BackupPC();
		op_illg();
	}
#endif
}

LOCALIPROC DoCodeMoveP1(void)
{
	/* MoveP 0000ddd1mm001aaa */
	uint32_t srcreg = V_regs.CurDecOpY.v[0].ArgDat;
	uint32_t *srcp = &V_regs.regs[srcreg];
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];

	uint32_t Displacement = nextiword_nm();
		/* shouldn't this sign extend ? */
	CPTR memp = *srcp + Displacement;

	uint32_t val = ((get_byte(memp) & 0x00FF) << 24)
		| ((get_byte(memp + 2) & 0x00FF) << 16)
		| ((get_byte(memp + 4) & 0x00FF) << 8)
		| (get_byte(memp + 6) & 0x00FF);

	*dstp = val;
}

LOCALIPROC DoCodeMoveP2(void)
{
	/* MoveP 0000ddd1mm001aaa */
	uint32_t srcreg = V_regs.CurDecOpY.v[0].ArgDat;
	uint32_t *srcp = &V_regs.regs[srcreg];
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];

	uint32_t Displacement = nextiword_nm();
		/* shouldn't this sign extend ? */
	CPTR memp = *srcp + Displacement;

	int16_t val = *dstp;

	put_byte(memp, val >> 8);
	put_byte(memp + 2, val);
}

LOCALIPROC DoCodeMoveP3(void)
{
	/* MoveP 0000ddd1mm001aaa */
	uint32_t srcreg = V_regs.CurDecOpY.v[0].ArgDat;
	uint32_t *srcp = &V_regs.regs[srcreg];
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];

	uint32_t Displacement = nextiword_nm();
		/* shouldn't this sign extend ? */
	CPTR memp = *srcp + Displacement;

	int32_t val = *dstp;

	put_byte(memp, val >> 24);
	put_byte(memp + 2, val >> 16);
	put_byte(memp + 4, val >> 8);
	put_byte(memp + 6, val);
}

LOCALPROC op_illg(void)
{
	BackupPC();
	Exception(4);
#if m68k_logExceptions
	dbglog_WriteNote("*** illegal instruction exception");
#endif
}

LOCALIPROC DoCodeChk(void)
{
	uint32_t dstvalue = DecodeGetSrcGetDstValue();
	uint32_t srcvalue = V_regs.SrcVal;

	if (uint32_t_MSBisSet(srcvalue)) {
		NeedDefaultLazyAllFlags();

#if WantCloserCyc
		V_MaxCyclesToGo -=
			(30 * kCycleScale + 3 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		NFLG = 1;
		Exception(6);
	} else if (((int32_t)srcvalue) > ((int32_t)dstvalue)) {
		NeedDefaultLazyAllFlags();

#if WantCloserCyc
		V_MaxCyclesToGo -=
			(30 * kCycleScale + 3 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		NFLG = 0;
		Exception(6);
	}
}

LOCALIPROC DoCodeTrap(void)
{
	/* Trap 010011100100vvvv */
	Exception(V_regs.CurDecOpY.v[1].ArgDat);
}

LOCALIPROC DoCodeTrapV(void)
{
	/* TrapV 0100111001110110 */
	NeedDefaultLazyAllFlags();

	if (VFLG != 0) {
#if WantCloserCyc
		V_MaxCyclesToGo += GetDcoCycles(V_regs.CurDecOp);
		V_MaxCyclesToGo -=
			(34 * kCycleScale + 4 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		Exception(7);
	}
}

LOCALIPROC DoCodeRtr(void)
{
	/* Rtr 0100111001110111 */
	uint32_t NewPC;
	CPTR stackp = m68k_areg(7);
	uint32_t NewCR = get_word(stackp);
	stackp += 2;
	NewPC = get_long(stackp);
	stackp += 4;
	m68k_areg(7) = stackp;
	m68k_setCR(NewCR);
	m68k_setpc(NewPC);
}

LOCALIPROC DoCodeLink(void)
{
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	CPTR stackp = m68k_areg(7);

	stackp -= 4;
	m68k_areg(7) = stackp; /* only matters if dstreg == 7 + 8 */
	put_long(stackp, *dstp);
	*dstp = stackp;
	m68k_areg(7) += uint32_t_FromSWord(nextiword_nm());
}

LOCALIPROC DoCodeUnlk(void)
{
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];

	if (dstreg != 7 + 8) {
		uint32_t src = *dstp;
		*dstp = get_long(src);
		m68k_areg(7) = src + 4;
	} else {
		/* wouldn't expect this to happen */
		m68k_areg(7) = get_long(m68k_areg(7)) + 4;
	}
}

LOCALIPROC DoCodeMoveRUSP(void)
{
	/* MOVE USP 0100111001100aaa */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
		uint32_t *dstp = &V_regs.regs[dstreg];

		V_regs.usp = *dstp;
	}
}

LOCALIPROC DoCodeMoveUSPR(void)
{
	/* MOVE USP 0100111001101aaa */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
		uint32_t *dstp = &V_regs.regs[dstreg];

		*dstp = V_regs.usp;
	}
}

LOCALIPROC DoCodeTas(void)
{
	/* Tas 0100101011mmmrrr */
	uint32_t dstvalue = DecodeGetSetDstValue();

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	dstvalue |= 0x80;

	ArgSetDstValue(dstvalue);
}

LOCALIPROC DoCodeFdefault(void)
{
	BackupPC();
	Exception(0xB);
}

LOCALPROC m68k_setstopped(void)
{
	/* not implemented. doesn't seemed to be used on Mac Plus */
	Exception(4); /* fake an illegal instruction */
#if m68k_logExceptions
	dbglog_WriteNote("*** set stopped");
#endif
}

LOCALIPROC DoCodeStop(void)
{
	/* Stop 0100111001110010 */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		m68k_setSR(nextiword_nm());
		m68k_setstopped();
	}
}

FORWARDPROC local_customreset(void);

LOCALIPROC DoCodeReset(void)
{
	/* Reset 0100111001100000 */
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		local_customreset();
	}
}

#if Use68020
LOCALIPROC DoCodeCallMorRtm(void)
{
	/* CALLM or RTM 0000011011mmmrrr */
	ReportAbnormalID(0x010D, "CALLM or RTM instruction");
}
#endif

#if Use68020
LOCALIPROC DoCodeMoveCCREa(void)
{
	/* Move from CCR 0100001011mmmrrr */
	DecodeSetDstValue(m68k_getCR());
}
#endif

#if Use68020
LOCALIPROC DoCodeBraL(void)
{
	/* Bra 0110ccccnnnnnnnn */
	int32_t offset = ((int32_t)(uint32_t)nextilong()) - 4;
	uint8_t * s = V_pc_p + offset;

	V_pc_p = s;

#if USE_PCLIMIT
	if (cond_rare(s >= V_pc_pHi)
		|| cond_rare(s < V_regs.pc_pLo))
	{
		Recalc_PC_Block();
	}
#endif
}
#endif

#if Use68020
LOCALPROC SkipiLong(void)
{
	V_pc_p += 4;

#if USE_PCLIMIT
	if (cond_rare(V_pc_p >= V_pc_pHi)) {
		Recalc_PC_Block();
	}
#endif
}
#endif

#if Use68020
LOCALIPROC DoCodeBccL(void)
{
	/* Bcc 0110ccccnnnnnnnn */
	cctrue(DoCodeBraL, SkipiLong);
}
#endif

#if Use68020
LOCALIPROC DoCodeBsrL(void)
{
	int32_t offset = ((int32_t)(uint32_t)nextilong()) - 4;
	uint8_t * s = V_pc_p + offset;

	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), m68k_getpc());
	V_pc_p = s;

#if USE_PCLIMIT
	if (cond_rare(s >= V_pc_pHi)
		|| cond_rare(s < V_regs.pc_pLo))
	{
		Recalc_PC_Block();
	}
#endif

	/* ReportAbnormal("long branch in DoCode6"); */
	/* Used by various Apps */
}
#endif

#if Use68020
LOCALIPROC DoCodeEXTBL(void)
{
	/* EXTB.L */
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	uint32_t dstvalue = uint32_t_FromSByte(*dstp);

	V_regs.LazyFlagKind = kLazyFlagsTstL;
	V_regs.LazyFlagArgDst = dstvalue;

	HaveSetUpFlags();

	*dstp = dstvalue;
}
#endif

#if Use68020
LOCALPROC DoCHK2orCMP2(void)
{
	/* CHK2 or CMP2 00000ss011mmmrrr */
	uint32_t regv;
	uint32_t lower;
	uint32_t upper;
	uint32_t extra = nextiword_nm();
	uint32_t DstAddr = DecodeDst();
	uint32_t srcreg = (extra >> 12) & 0x0F;
	uint32_t *srcp = &V_regs.regs[srcreg];

	/* ReportAbnormal("CHK2 or CMP2 instruction"); */
	switch (V_regs.CurDecOpY.v[0].ArgDat) {
		case 1:
			if ((extra & 0x8000) == 0) {
				regv = uint32_t_FromSByte(*srcp);
			} else {
				regv = uint32_t_FromSLong(*srcp);
			}
			lower = get_byte(DstAddr);
			upper = get_byte(DstAddr + 1);
			break;
		case 2:
			if ((extra & 0x8000) == 0) {
				regv = uint32_t_FromSWord(*srcp);
			} else {
				regv = uint32_t_FromSLong(*srcp);
			}
			lower = get_word(DstAddr);
			upper = get_word(DstAddr + 2);
			break;
		default:
#if ExtraAbnormalReports
			if (4 != V_regs.CurDecOpY.v[0].ArgDat) {
				ReportAbnormalID(0x010E,
					"illegal opsize in CHK2 or CMP2");
			}
#endif
			if ((extra & 0x8000) == 0) {
				regv = uint32_t_FromSLong(*srcp);
			} else {
				regv = uint32_t_FromSLong(*srcp);
			}
			lower = get_long(DstAddr);
			upper = get_long(DstAddr + 4);
			break;
	}

	NeedDefaultLazyAllFlags();

	ZFLG = Bool2Bit((upper == regv) || (lower == regv));
	CFLG = Bool2Bit((((int32_t)lower) <= ((int32_t)upper))
			? (((int32_t)regv) < ((int32_t)lower)
				|| ((int32_t)regv) > ((int32_t)upper))
			: (((int32_t)regv) > ((int32_t)upper)
				|| ((int32_t)regv) < ((int32_t)lower)));

	if ((extra & 0x800) && (CFLG != 0)) {
		Exception(6);
	}
}
#endif

#if Use68020
LOCALPROC DoCAS(void)
{
	/* CAS 00001ss011mmmrrr */
	uint32_t srcvalue;
	uint32_t dstvalue;

	uint16_t src = nextiword_nm();
	int ru = (src >> 6) & 7;
	int rc = src & 7;

	ReportAbnormalID(0x010F, "CAS instruction");
	switch (V_regs.CurDecOpY.v[0].ArgDat) {
		case 1:
			srcvalue = uint32_t_FromSByte(V_regs.regs[rc]);
			break;
		case 2:
			srcvalue = uint32_t_FromSWord(V_regs.regs[rc]);
			break;
		default:
#if ExtraAbnormalReports
			if (4 != V_regs.CurDecOpY.v[0].ArgDat) {
				ReportAbnormalID(0x0110, "illegal opsize in DoCAS");
			}
#endif
			srcvalue = uint32_t_FromSLong(V_regs.regs[rc]);
			break;
	}
	dstvalue = DecodeGetSetDstValue();

	{
		int flgs = ((int32_t)srcvalue) < 0;
		int flgo = ((int32_t)dstvalue) < 0;
		uint32_t newv = dstvalue - srcvalue;
		if (V_regs.CurDecOpY.v[0].ArgDat == 1) {
			newv = uint32_t_FromSByte(newv);
		} else if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
			newv = uint32_t_FromSWord(newv);
		} else {
			newv = uint32_t_FromSLong(newv);
		}
		ZFLG = Bool2Bit(((int32_t)newv) == 0);
		NFLG = Bool2Bit(((int32_t)newv) < 0);
		VFLG = Bool2Bit((flgs != flgo) && ((NFLG != 0) != flgo));
		CFLG = Bool2Bit(
			(flgs && ! flgo) || ((NFLG != 0) && ((! flgo) || flgs)));

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		if (ZFLG != 0) {
			ArgSetDstValue(m68k_dreg(ru));
		} else {
			V_regs.ArgAddr.rga = &V_regs.regs[rc];

			if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
				*V_regs.ArgAddr.rga =
					(*V_regs.ArgAddr.rga & ~ 0xffff)
						| ((dstvalue) & 0xffff);
			} else if (V_regs.CurDecOpY.v[0].ArgDat < 2) {
				*V_regs.ArgAddr.rga =
					(*V_regs.ArgAddr.rga & ~ 0xff)
						| ((dstvalue) & 0xff);
			} else {
				*V_regs.ArgAddr.rga = dstvalue;
			}
		}
	}
}
#endif

#if Use68020
LOCALPROC DoCAS2(void)
{
	/* CAS2 00001ss011111100 */
	uint32_t extra = nextilong();
	int dc2 = extra & 7;
	int du2 = (extra >> 6) & 7;
	int dc1 = (extra >> 16) & 7;
	int du1 = (extra >> 22) & 7;
	CPTR rn1 = V_regs.regs[(extra >> 28) & 0x0F];
	CPTR rn2 = V_regs.regs[(extra >> 12) & 0x0F];
	int32_t src = m68k_dreg(dc1);
	int32_t dst1;
	int32_t dst2;

	ReportAbnormalID(0x0111, "DoCAS2 instruction");
	if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
		dst1 = get_word(rn1);
		dst2 = get_word(rn2);
		src = (int32_t)(int16_t)src;
	} else {
		dst1 = get_long(rn1);
		dst2 = get_long(rn2);
	}
	{
		int flgs = src < 0;
		int flgo = dst1 < 0;
		int32_t newv = dst1 - src;
		if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
			newv = (uint16_t)newv;
		}
		ZFLG = Bool2Bit(newv == 0);
		NFLG = Bool2Bit(newv < 0);
		VFLG = Bool2Bit((flgs != flgo) && ((NFLG != 0) != flgo));
		CFLG = Bool2Bit(
			(flgs && ! flgo) || ((NFLG != 0) && ((! flgo) || flgs)));

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		if (ZFLG != 0) {
			src = m68k_dreg(dc2);
			if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
				src = (int32_t)(int16_t)src;
			}
			flgs = src < 0;
			flgo = dst2 < 0;
			newv = dst2 - src;
			if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
				newv = (uint16_t)newv;
			}
			ZFLG = Bool2Bit(newv == 0);
			NFLG = Bool2Bit(newv < 0);
			VFLG = Bool2Bit((flgs != flgo) && ((NFLG != 0) != flgo));
			CFLG = Bool2Bit((flgs && ! flgo)
				|| ((NFLG != 0) && ((! flgo) || flgs)));

			V_regs.LazyFlagKind = kLazyFlagsDefault;
			if (ZFLG != 0) {
				if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
					put_word(rn1, m68k_dreg(du1));
					put_word(rn2, m68k_dreg(du2));
				} else {
					put_word(rn1, m68k_dreg(du1));
					put_word(rn2, m68k_dreg(du2));
				}
			}
		}
	}
	if (ZFLG == 0) {
		if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
			m68k_dreg(du1) =
				(m68k_dreg(du1) & ~ 0xffff) | ((uint32_t)dst1 & 0xffff);
			m68k_dreg(du2) =
				(m68k_dreg(du2) & ~ 0xffff) | ((uint32_t)dst2 & 0xffff);
		} else {
			m68k_dreg(du1) = dst1;
			m68k_dreg(du2) = dst2;
		}
	}
}
#endif

#if Use68020
LOCALPROC DoMOVES(void)
{
	/* MoveS 00001110ssmmmrrr */
	ReportAbnormalID(0x0112, "MoveS instruction");
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint16_t extra = nextiword_nm();
		if (extra & 0x0800) {
			uint32_t src = V_regs.regs[(extra >> 12) & 0x0F];
			DecodeSetDstValue(src);
		} else {
			uint32_t srcvalue = DecodeGetDstValue();
			uint32_t rr = (extra >> 12) & 7;
			if (extra & 0x8000) {
				m68k_areg(rr) = srcvalue;
			} else {
				V_regs.ArgAddr.rga = &V_regs.regs[rr];

				if (V_regs.CurDecOpY.v[0].ArgDat == 2) {
					*V_regs.ArgAddr.rga =
						(*V_regs.ArgAddr.rga & ~ 0xffff)
							| ((srcvalue) & 0xffff);
				} else if (V_regs.CurDecOpY.v[0].ArgDat < 2) {
					*V_regs.ArgAddr.rga =
						(*V_regs.ArgAddr.rga & ~ 0xff)
							| ((srcvalue) & 0xff);
				} else {
					*V_regs.ArgAddr.rga = srcvalue;
				}
			}
		}
	}
}
#endif

#define uint32_t_lo(x) ((x) & 0x0000FFFF)
#define uint32_t_hi(x) (((x) >> 16) & 0x0000FFFF)

#if Use68020
struct uint64_t0 {
	uint32_t hi;
	uint32_t lo;
};
typedef struct uint64_t0 uint64_t0;
#endif

#if Use68020
LOCALPROC Ui6r_Negate(uint64_t0 *v)
{
	v->hi = ~ v->hi;
	v->lo = - v->lo;
	if (v->lo == 0) {
		v->hi++;
	}
}
#endif

#if Use68020
LOCALFUNC bool reg_call Ui6r_IsZero(uint64_t0 *v)
{
	return (v->hi == 0) && (v->lo == 0);
}
#endif

#if Use68020
LOCALFUNC bool reg_call Ui6r_IsNeg(uint64_t0 *v)
{
	return ((int32_t)v->hi) < 0;
}
#endif

#if Use68020
LOCALPROC mul_unsigned(uint32_t src1, uint32_t src2, uint64_t0 *dst)
{
	uint32_t src1_lo = uint32_t_lo(src1);
	uint32_t src2_lo = uint32_t_lo(src2);
	uint32_t src1_hi = uint32_t_hi(src1);
	uint32_t src2_hi = uint32_t_hi(src2);

	uint32_t r0 = src1_lo * src2_lo;
	uint32_t r1 = src1_hi * src2_lo;
	uint32_t r2 = src1_lo * src2_hi;
	uint32_t r3 = src1_hi * src2_hi;

	uint32_t ra1 = uint32_t_hi(r0) + uint32_t_lo(r1) + uint32_t_lo(r2);

	dst->lo = (uint32_t_lo(ra1) << 16) | uint32_t_lo(r0);
	dst->hi = uint32_t_hi(ra1) + uint32_t_hi(r1) + uint32_t_hi(r2) + r3;
}
#endif

#if Use68020
LOCALFUNC bool div_unsigned(uint64_t0 *src, uint32_t div,
	uint32_t *quot, uint32_t *rem)
{
	int i;
	uint32_t q = 0;
	uint32_t cbit = 0;
	uint32_t src_hi = src->hi;
	uint32_t src_lo = src->lo;

	if (div <= src_hi) {
		return true;
	}
	for (i = 0 ; i < 32 ; i++) {
		cbit = src_hi & 0x80000000ul;
		src_hi <<= 1;
		if (src_lo & 0x80000000ul) {
			src_hi++;
		}
		src_lo <<= 1;
		q = q << 1;
		if (cbit || div <= src_hi) {
			q |= 1;
			src_hi -= div;
		}
	}
	*quot = q;
	*rem = src_hi;
	return false;
}
#endif

#if Use68020
LOCALIPROC DoCodeMulL(void)
{
	/* MULU 0100110000mmmrrr 0rrr0s0000000rrr */
	/* MULS 0100110000mmmrrr 0rrr1s0000000rrr */
	uint64_t0 dst;
	uint16_t extra = nextiword();
	uint32_t r2 = (extra >> 12) & 7;
	uint32_t dstvalue = m68k_dreg(r2);
	uint32_t srcvalue = DecodeGetDstValue();

	if (extra & 0x800) {
		/* MULS.L - signed */

		int32_t src1 = (int32_t)srcvalue;
		int32_t src2 = (int32_t)dstvalue;
		bool s1 = src1 < 0;
		bool s2 = src2 < 0;
		bool sr = s1 != s2;

		/* ReportAbnormal("MULS.L"); */
		/* used by Sys 7.5.5 boot extensions */
		if (s1) {
			src1 = - src1;
		}
		if (s2) {
			src2 = - src2;
		}
		mul_unsigned((uint32_t)src1, (uint32_t)src2, &dst);
		if (sr) {
			Ui6r_Negate(&dst);
		}
		VFLG = CFLG = 0;
		ZFLG = Bool2Bit(Ui6r_IsZero(&dst));
		NFLG = Bool2Bit(Ui6r_IsNeg(&dst));

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		if (extra & 0x400) {
			m68k_dreg(extra & 7) = dst.hi;
		} else {
			if ((dst.lo & 0x80000000) != 0) {
				if ((dst.hi & 0xffffffff) != 0xffffffff) {
					VFLG = 1;
				}
			} else {
				if (dst.hi != 0) {
					VFLG = 1;
				}
			}
		}
	} else {
		/* MULU.L - unsigned */

		/* ReportAbnormal("MULU.U"); */
		/* Used by various Apps */

		mul_unsigned(srcvalue, dstvalue, &dst);

		VFLG = CFLG = 0;
		ZFLG = Bool2Bit(Ui6r_IsZero(&dst));
		NFLG = Bool2Bit(Ui6r_IsNeg(&dst));

		V_regs.LazyFlagKind = kLazyFlagsDefault;

		if (extra & 0x400) {
			m68k_dreg(extra & 7) = dst.hi;
		} else {
			if (dst.hi != 0) {
				VFLG = 1;
			}
		}
	}
	m68k_dreg(r2) = dst.lo;
}
#endif

#if Use68020
LOCALIPROC DoCodeDivL(void)
{
	/* DIVU 0100110001mmmrrr 0rrr0s0000000rrr */
	/* DIVS 0100110001mmmrrr 0rrr1s0000000rrr */
	/* ReportAbnormal("DIVS/DIVU long"); */
	uint64_t0 v2;
	uint32_t quot;
	uint32_t rem;
	uint16_t extra = nextiword();
	uint32_t rDr = extra & 7;
	uint32_t rDq = (extra >> 12) & 7;
	uint32_t src = (uint32_t)(int32_t)DecodeGetDstValue();

	if (src == 0) {
		Exception(5);
#if m68k_logExceptions
		dbglog_WriteNote("*** zero devide exception");
#endif
		return;
	}
	if (0 != (extra & 0x0800)) {
		/* signed variant */
		bool sr;
		bool s2;
		bool s1 = ((int32_t)src < 0);

		v2.lo = (int32_t)m68k_dreg(rDq);
		if (extra & 0x0400) {
			v2.hi = (int32_t)m68k_dreg(rDr);
		} else {
			v2.hi = ((int32_t)v2.lo) < 0 ? -1 : 0;
		}
		s2 = Ui6r_IsNeg(&v2);
		sr = (s1 != s2);
		if (s2) {
			Ui6r_Negate(&v2);
		}
		if (s1) {
			src = - src;
		}
		if (div_unsigned(&v2, src, &quot, &rem)
			|| (sr ? quot > 0x80000000 : quot > 0x7fffffff))
		{
			NeedDefaultLazyAllFlags();

			VFLG = NFLG = 1;
			CFLG = 0;
		} else {
			if (sr) {
				quot = - quot;
			}
			if (((int32_t)rem < 0) != s2) {
				rem = - rem;
			}
			VFLG = CFLG = 0;
			ZFLG = Bool2Bit(((int32_t)quot) == 0);
			NFLG = Bool2Bit(((int32_t)quot) < 0);

			V_regs.LazyFlagKind = kLazyFlagsDefault;

			m68k_dreg(rDr) = rem;
			m68k_dreg(rDq) = quot;
		}
	} else {
		/* unsigned */

		v2.lo = (uint32_t)m68k_dreg(rDq);
		if (extra & 0x400) {
			v2.hi = (uint32_t)m68k_dreg(rDr);
		} else {
			v2.hi = 0;
		}
		if (div_unsigned(&v2, src, &quot, &rem)) {
			NeedDefaultLazyAllFlags();

			VFLG = NFLG = 1;
			CFLG = 0;
		} else {
			VFLG = CFLG = 0;
			ZFLG = Bool2Bit(((int32_t)quot) == 0);
			NFLG = Bool2Bit(((int32_t)quot) < 0);

			V_regs.LazyFlagKind = kLazyFlagsDefault;

			m68k_dreg(rDr) = rem;
			m68k_dreg(rDq) = quot;
		}
	}
}
#endif

#if Use68020
LOCALIPROC DoMoveToControl(void)
{
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint16_t src = nextiword_nm();
		int regno = (src >> 12) & 0x0F;
		uint32_t v = V_regs.regs[regno];

		switch (src & 0x0FFF) {
			case 0x0000:
				V_regs.sfc = v & 7;
				/* ReportAbnormal("DoMoveToControl: sfc"); */
				/* happens on entering macsbug */
				break;
			case 0x0001:
				V_regs.dfc = v & 7;
				/* ReportAbnormal("DoMoveToControl: dfc"); */
				break;
			case 0x0002:
				V_regs.cacr = v & 0x3;
				/* ReportAbnormal("DoMoveToControl: cacr"); */
				/* used by Sys 7.5.5 boot */
				break;
			case 0x0800:
				V_regs.usp = v;
				ReportAbnormalID(0x0113, "DoMoveToControl: usp");
				break;
			case 0x0801:
				V_regs.vbr = v;
				/* ReportAbnormal("DoMoveToControl: vbr"); */
				/* happens on entering macsbug */
				break;
			case 0x0802:
				V_regs.caar = v &0xfc;
				/* ReportAbnormal("DoMoveToControl: caar"); */
				/* happens on entering macsbug */
				break;
			case 0x0803:
				V_regs.msp = v;
				if (V_regs.m == 1) {
					m68k_areg(7) = V_regs.msp;
				}
				/* ReportAbnormal("DoMoveToControl: msp"); */
				/* happens on entering macsbug */
				break;
			case 0x0804:
				V_regs.isp = v;
				if (V_regs.m == 0) {
					m68k_areg(7) = V_regs.isp;
				}
				ReportAbnormalID(0x0114, "DoMoveToControl: isp");
				break;
			default:
				op_illg();
				ReportAbnormalID(0x0115,
					"DoMoveToControl: unknown reg");
				break;
		}
	}
}
#endif

#if Use68020
LOCALIPROC DoMoveFromControl(void)
{
	if (0 == V_regs.s) {
		DoPrivilegeViolation();
	} else {
		uint32_t v;
		uint16_t src = nextiword_nm();
		int regno = (src >> 12) & 0x0F;

		switch (src & 0x0FFF) {
			case 0x0000:
				v = V_regs.sfc;
				/* ReportAbnormal("DoMoveFromControl: sfc"); */
				/* happens on entering macsbug */
				break;
			case 0x0001:
				v = V_regs.dfc;
				/* ReportAbnormal("DoMoveFromControl: dfc"); */
				/* happens on entering macsbug */
				break;
			case 0x0002:
				v = V_regs.cacr;
				/* ReportAbnormal("DoMoveFromControl: cacr"); */
				/* used by Sys 7.5.5 boot */
				break;
			case 0x0800:
				v = V_regs.usp;
				ReportAbnormalID(0x0116, "DoMoveFromControl: usp");
				break;
			case 0x0801:
				v = V_regs.vbr;
				/* ReportAbnormal("DoMoveFromControl: vbr"); */
				/* happens on entering macsbug */
				break;
			case 0x0802:
				v = V_regs.caar;
				/* ReportAbnormal("DoMoveFromControl: caar"); */
				/* happens on entering macsbug */
				break;
			case 0x0803:
				v = (V_regs.m == 1)
					? m68k_areg(7)
					: V_regs.msp;
				/* ReportAbnormal("DoMoveFromControl: msp"); */
				/* happens on entering macsbug */
				break;
			case 0x0804:
				v = (V_regs.m == 0)
					? m68k_areg(7)
					: V_regs.isp;
				ReportAbnormalID(0x0117, "DoMoveFromControl: isp");
				break;
			default:
				v = 0;
				ReportAbnormalID(0x0118,
					"DoMoveFromControl: unknown reg");
				op_illg();
				break;
		}
		V_regs.regs[regno] = v;
	}
}
#endif

#if Use68020
LOCALIPROC DoCodeBkpt(void)
{
	/* BKPT 0100100001001rrr */
	ReportAbnormalID(0x0119, "BKPT instruction");
	op_illg();
}
#endif

#if Use68020
LOCALIPROC DoCodeRtd(void)
{
	/* Rtd 0100111001110100 */
	uint32_t NewPC = get_long(m68k_areg(7));
	int32_t offs = nextiSWord();
	/* ReportAbnormal("RTD"); */
	/* used by Sys 7.5.5 boot */
	m68k_areg(7) += (4 + offs);
	m68k_setpc(NewPC);
}
#endif

#if Use68020
LOCALIPROC DoCodeLinkL(void)
{
	/* Link.L 0100100000001rrr */

	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint32_t *dstp = &V_regs.regs[dstreg];
	CPTR stackp = m68k_areg(7);

	ReportAbnormalID(0x011A, "Link.L");

	stackp -= 4;
	m68k_areg(7) = stackp; /* only matters if dstreg == 7 + 8 */
	put_long(stackp, *dstp);
	*dstp = stackp;
	m68k_areg(7) += (int32_t)nextilong();
}
#endif

#if Use68020
LOCALPROC DoCodeTRAPcc_t(void)
{
	ReportAbnormalID(0x011B, "TRAPcc trapping");
	Exception(7);
	/* pc pushed onto stack wrong */
}
#endif

#if Use68020
LOCALPROC DoCodeTRAPcc_f(void)
{
}
#endif

#if Use68020
LOCALIPROC DoCodeTRAPcc(void)
{
	/* TRAPcc 0101cccc11111sss */
	/* ReportAbnormal("TRAPcc"); */
	switch (V_regs.CurDecOpY.v[1].ArgDat) {
		case 2:
			ReportAbnormalID(0x011C, "TRAPcc word data");
			SkipiWord();
			break;
		case 3:
			ReportAbnormalID(0x011D, "TRAPcc long data");
			SkipiLong();
			break;
		case 4:
			/* ReportAbnormal("TRAPcc no data"); */
			/* no optional data */
			break;
		default:
			ReportAbnormalID(0x011E, "TRAPcc illegal format");
			op_illg();
			break;
	}
	cctrue(DoCodeTRAPcc_t, DoCodeTRAPcc_f);
}
#endif

#if Use68020
LOCALIPROC DoCodePack(void)
{
	uint32_t offs = nextiSWord();
	uint32_t val = DecodeGetSrcValue();

	ReportAbnormalID(0x011F, "PACK");

	val += offs;
	val = ((val >> 4) & 0xf0) | (val & 0xf);

	DecodeSetDstValue(val);
}
#endif

#if Use68020
LOCALIPROC DoCodeUnpk(void)
{
	uint32_t offs = nextiSWord();
	uint32_t val = DecodeGetSrcValue();

	ReportAbnormalID(0x0120, "UNPK");

	val = (((val & 0xF0) << 4) | (val & 0x0F)) + offs;

	DecodeSetDstValue(val);
}
#endif

#if Use68020
LOCALIPROC DoBitField(void)
{
	uint32_t tmp;
	uint32_t newtmp;
	CPTR dsta;
	uint32_t bf0;
	uint8_t bf1;
	uint32_t dstreg = V_regs.CurDecOpY.v[1].ArgDat;
	uint16_t extra = nextiword();
	uint32_t offset = ((extra & 0x0800) != 0)
		? m68k_dreg((extra >> 6) & 7)
		: ((extra >> 6) & 0x1f);
	uint32_t width = ((extra & 0x0020) != 0)
		? m68k_dreg(extra & 7)
		: extra;
	uint8_t bfa[5];
	uint32_t offwid;

	/* ReportAbnormal("Bit Field operator"); */
	/* width = ((width - 1) & 0x1f) + 1; */ /* 0 -> 32 */
	width &= 0x001F; /* except width == 0 really means 32 */
	if (V_regs.CurDecOpY.v[0].AMd == 0) {
		bf0 = m68k_dreg(dstreg);
		offset &= 0x1f;
		tmp = bf0;
		if (0 != offset) {
			tmp = (tmp << offset) | (tmp >> (32 - offset));
		}
	} else {
		/*
			V_regs.ArgKind == AKMemory,
			otherwise illegal and don't get here
		*/
		dsta = DecodeDst();
		dsta += Ui5rASR(offset, 3);
		offset &= 7;
		offwid = offset + ((width == 0) ? 32 : width);

		/* if (offwid > 0) */ {
			bf1 = get_byte(dsta);
			bfa[0] = bf1;
			tmp = ((uint32_t)bf1) << (24 + offset);
		}
		if (offwid > 8) {
			bf1 = get_byte(dsta + 1);
			bfa[1] = bf1;
			tmp |= ((uint32_t)bf1) << (16 + offset);
		}
		if (offwid > 16) {
			bf1 = get_byte(dsta + 2);
			bfa[2] = bf1;
			tmp |= ((uint32_t)bf1) << (8 + offset);
		}
		if (offwid > 24) {
			bf1 = get_byte(dsta + 3);
			bfa[3] = bf1;
			tmp |= ((uint32_t)bf1) << (offset);
		}
		if (offwid > 32) {
			bf1 = get_byte(dsta + 4);
			bfa[4] = bf1;
			tmp |= ((uint32_t)bf1) >> (8 - offset);
		}
	}

	NFLG = Bool2Bit(((int32_t)tmp) < 0);
	if (width != 0) {
		tmp >>= (32 - width);
	}
	ZFLG = tmp == 0;
	VFLG = 0;
	CFLG = 0;

	V_regs.LazyFlagKind = kLazyFlagsDefault;

	newtmp = tmp;

	switch (V_regs.CurDecOpY.v[0].ArgDat) {
		case 0: /* BFTST */
			/* do nothing */
			break;
		case 1: /* BFEXTU */
			m68k_dreg((extra >> 12) & 7) = tmp;
			break;
		case 2: /* BFCHG */
			newtmp = ~ newtmp;
			if (width != 0) {
				newtmp &= ((1 << width) - 1);
			}
			break;
		case 3: /* BFEXTS */
			if (NFLG != 0) {
				m68k_dreg((extra >> 12) & 7) = tmp
					| ((width == 0) ? 0 : (-1 << width));
			} else {
				m68k_dreg((extra >> 12) & 7) = tmp;
			}
			break;
		case 4: /* BFCLR */
			newtmp = 0;
			break;
		case 5: /* BFFFO */
			{
				uint32_t mask = 1 << ((width == 0) ? 31 : (width - 1));
				uint32_t i = offset;

				while ((0 != mask) && (0 == (tmp & mask))) {
					mask >>= 1;
					i++;
				}
				m68k_dreg((extra >> 12) & 7) = i;
			}
			break;
		case 6: /* BFSET */
			newtmp = (width == 0) ? ~ 0 : ((1 << width) - 1);
			break;
		case 7: /* BFINS */
			newtmp = m68k_dreg((extra >> 12) & 7);
			if (width != 0) {
				newtmp &= ((1 << width) - 1);
			}
			break;
	}

	if (newtmp != tmp) {

		if (width != 0) {
			newtmp <<= (32 - width);
		}

		if (V_regs.CurDecOpY.v[0].AMd == 0) {
			uint32_t mask = ~ 0;

			if (width != 0) {
				mask <<= (32 - width);
			}

			if (0 != offset) {
				newtmp = (newtmp >> offset) | (newtmp << (32 - offset));
				mask = (mask >> offset) | (mask << (32 - offset));
			}

			bf0 = (bf0 & ~ mask) | (newtmp);
			m68k_dreg(dstreg) = bf0;
		} else {

			/* if (offwid > 0) */ {
				uint8_t mask = ~ (0xFF >> offset);

				bf1 = newtmp >> (24 + offset);
				if (offwid < 8) {
					mask |= (0xFF >> offwid);
				}
				if (mask != 0) {
					bf1 |= bfa[0] & mask;
				}
				put_byte(dsta + 0, bf1);
			}
			if (offwid > 8) {
				bf1 = newtmp >> (16 + offset);
				if (offwid < 16) {
					bf1 |= (bfa[1] & (0xFF >> (offwid - 8)));
				}
				put_byte(dsta + 1, bf1);
			}
			if (offwid > 16) {
				bf1 = newtmp >> (8 + offset);
				if (offwid < 24) {
					bf1 |= (bfa[2] & (0xFF >> (offwid - 16)));
				}
				put_byte(dsta + 2, bf1);
			}
			if (offwid > 24) {
				bf1 = newtmp >> (offset);
				if (offwid < 32) {
					bf1 |= (bfa[3] & (0xFF >> (offwid - 24)));
				}
				put_byte(dsta + 3, bf1);
			}
			if (offwid > 32) {
				bf1 = newtmp << (8 - offset);
				bf1 |= (bfa[4] & (0xFF >> (offwid - 32)));
				put_byte(dsta + 4, bf1);
			}
		}
	}
}
#endif

#if EmMMU | EmFPU
LOCALFUNC bool DecodeModeRegister(uint32_t sz)
{
	bool IsOk;
	uint16_t Dat = V_regs.CurDecOpY.v[0].ArgDat;
	uint16_t themode = (Dat >> 3) & 7;
	uint16_t thereg = Dat & 7;

	switch (themode) {
		case 0 :
			V_regs.ArgKind = AKRegister;
			V_regs.ArgAddr.rga = &V_regs.regs[thereg];
			IsOk = true;
			break;
		case 1 :
			V_regs.ArgKind = AKRegister;
			V_regs.ArgAddr.rga = &V_regs.regs[thereg + 8];
			IsOk = true;
			break;
		case 2 :
			V_regs.ArgKind = AKMemory;
			V_regs.ArgAddr.mem = m68k_areg(thereg);
			IsOk = true;
			break;
		case 3 :
			V_regs.ArgKind = AKMemory;
			V_regs.ArgAddr.mem = m68k_areg(thereg);
			if ((thereg == 7) && (sz == 1)) {
				m68k_areg(thereg) += 2;
			} else {
				m68k_areg(thereg) += sz;
			}
			IsOk = true;
			break;
		case 4 :
			V_regs.ArgKind = AKMemory;
			if ((thereg == 7) && (sz == 1)) {
				m68k_areg(thereg) -= 2;
			} else {
				m68k_areg(thereg) -= sz;
			}
			V_regs.ArgAddr.mem = m68k_areg(thereg);
			IsOk = true;
			break;
		case 5 :
			V_regs.ArgKind = AKMemory;
			V_regs.ArgAddr.mem = m68k_areg(thereg)
				+ nextiSWord();
			IsOk = true;
			break;
		case 6 :
			V_regs.ArgKind = AKMemory;
			V_regs.ArgAddr.mem = get_disp_ea(m68k_areg(thereg));
			IsOk = true;
			break;
		case 7 :
			switch (thereg) {
				case 0 :
					V_regs.ArgKind = AKMemory;
					V_regs.ArgAddr.mem = nextiSWord();
					IsOk = true;
					break;
				case 1 :
					V_regs.ArgKind = AKMemory;
					V_regs.ArgAddr.mem = nextilong();
					IsOk = true;
					break;
				case 2 :
					V_regs.ArgKind = AKMemory;
					V_regs.ArgAddr.mem = m68k_getpc();
					V_regs.ArgAddr.mem += nextiSWord();
					IsOk = true;
					break;
				case 3 :
					V_regs.ArgKind = AKMemory;
					V_regs.ArgAddr.mem = get_disp_ea(m68k_getpc());
					IsOk = true;
					break;
				case 4 :
					V_regs.ArgKind = AKMemory;
					V_regs.ArgAddr.mem = m68k_getpc();
					if (sz == 1) {
						++V_regs.ArgAddr.mem;
					}
					m68k_setpc(V_regs.ArgAddr.mem + sz);
					IsOk = true;
					break;
				default:
					IsOk = false;
					break;
			}
			break;
		default:
			IsOk = false;
			break;
	}

	return IsOk;
}
#endif

#if EmMMU | EmFPU
LOCALFUNC uint32_t GetArgValueL(void)
{
	uint32_t v;

	if (AKMemory == V_regs.ArgKind) {
		v = get_long(V_regs.ArgAddr.mem);
	} else {
		/* must be AKRegister */
		v = uint32_t_FromSLong(*V_regs.ArgAddr.rga);
	}

	return v;
}
#endif

#if EmMMU | EmFPU
LOCALFUNC uint32_t GetArgValueW(void)
{
	uint32_t v;

	if (AKMemory == V_regs.ArgKind) {
		v = get_word(V_regs.ArgAddr.mem);
	} else {
		/* must be AKRegister */
		v = uint32_t_FromSWord(*V_regs.ArgAddr.rga);
	}

	return v;
}
#endif

#if EmMMU | EmFPU
LOCALFUNC uint32_t GetArgValueB(void)
{
	uint32_t v;

	if (AKMemory == V_regs.ArgKind) {
		v = get_byte(V_regs.ArgAddr.mem);
	} else {
		/* must be AKRegister */
		v = uint32_t_FromSByte(*V_regs.ArgAddr.rga);
	}

	return v;
}
#endif

#if EmMMU | EmFPU
LOCALPROC SetArgValueL(uint32_t v)
{
	if (AKMemory == V_regs.ArgKind) {
		put_long(V_regs.ArgAddr.mem, v);
	} else {
		/* must be AKRegister */
		*V_regs.ArgAddr.rga = v;
	}
}
#endif

#if EmMMU | EmFPU
LOCALPROC SetArgValueW(uint32_t v)
{
	if (AKMemory == V_regs.ArgKind) {
		put_word(V_regs.ArgAddr.mem, v);
	} else {
		/* must be AKRegister */
		*V_regs.ArgAddr.rga =
			(*V_regs.ArgAddr.rga & ~ 0xffff) | ((v) & 0xffff);
	}
}
#endif

#if EmMMU | EmFPU
LOCALPROC SetArgValueB(uint32_t v)
{
	if (AKMemory == V_regs.ArgKind) {
		put_byte(V_regs.ArgAddr.mem, v);
	} else {
		/* must be AKRegister */
		*V_regs.ArgAddr.rga =
			(*V_regs.ArgAddr.rga & ~ 0xff) | ((v) & 0xff);
	}
}
#endif


#if EmMMU
LOCALIPROC DoCodeMMU(void)
{
	/*
		Emulate enough of MMU for System 7.5.5 universal
		to boot on Mac Plus 68020. There is one
		spurious "PMOVE TC, (A0)".
		And implement a few more PMOVE operations seen
		when running Disk Copy 6.3.3 and MacsBug.
	*/
	uint16_t opcode = ((uint16_t)(V_regs.CurDecOpY.v[0].AMd) << 8)
		| V_regs.CurDecOpY.v[0].ArgDat;
	if (opcode == 0xF010) {
		uint16_t ew = (int)nextiword_nm();
		if (ew == 0x4200) {
			/* PMOVE TC, (A0) */
			/* fprintf(stderr, "0xF010 0x4200\n"); */
			if (DecodeModeRegister(4)) {
				SetArgValueL(0);
				return;
			}
		} else if ((ew == 0x4E00) || (ew == 0x4A00)) {
			/* PMOVE CRP, (A0) and PMOVE SRP, (A0) */
			/* fprintf(stderr, "0xF010 %x\n", ew); */
			if (DecodeModeRegister(4)) {
				SetArgValueL(0x7FFF0001);
				V_regs.ArgAddr.mem += 4;
				SetArgValueL(0);
				return;
			}
		} else if (ew == 0x6200) {
			/* PMOVE MMUSR, (A0) */
			/* fprintf(stderr, "0xF010 %x\n", ew); */
			if (DecodeModeRegister(2)) {
				SetArgValueW(0);
				return;
			}
		}
		/* fprintf(stderr, "extensions %x\n", ew); */
		BackupPC();
	}
	/* fprintf(stderr, "opcode %x\n", (int)opcode); */
	ReportAbnormalID(0x0121, "MMU op");
	DoCodeFdefault();
}
#endif

#if EmFPU

#include "FPMATHEM.h"
#include "FPCPEMDV.h"

#endif

#if HaveGlbReg
LOCALPROC Em_Swap(void)
{
#ifdef r_pc_p
	{
		uint8_t * t = g_pc_p;
		g_pc_p = regs.pc_p;
		regs.pc_p = t;
	}
#endif
#ifdef r_MaxCyclesToGo
	{
		int32_t t = g_MaxCyclesToGo;
		g_MaxCyclesToGo = regs.MaxCyclesToGo;
		regs.MaxCyclesToGo = t;
	}
#endif
#ifdef r_pc_pHi
	{
		uint8_t * t = g_pc_pHi;
		g_pc_pHi = regs.pc_pHi;
		regs.pc_pHi = t;
	}
#endif
#ifdef r_regs
	{
		struct regstruct *t = g_regs;
		g_regs = regs.save_regs;
		regs.save_regs = t;
	}
#endif
}
#endif

#if HaveGlbReg
#define Em_Enter Em_Swap
#else
#define Em_Enter()
#endif

#if HaveGlbReg
#define Em_Exit Em_Swap
#else
#define Em_Exit()
#endif

#if HaveGlbReg
LOCALFUNC bool LocalMemAccessNtfy(ATTep pT)
{
	bool v;

	Em_Exit();
	v = MemAccessNtfy(pT);
	Em_Enter();

	return v;
}
#else
#define LocalMemAccessNtfy MemAccessNtfy
#endif

#if HaveGlbReg
LOCALFUNC uint32_t LocalMMDV_Access(ATTep p, uint32_t Data,
	bool WriteMem, bool ByteSize, CPTR addr)
{
	uint32_t v;

	Em_Exit();
	v = MMDV_Access(p, Data, WriteMem, ByteSize, addr);
	Em_Enter();

	return v;
}
#else
#define LocalMMDV_Access MMDV_Access
#endif

LOCALPROC local_customreset(void)
{
	Em_Exit();
	customreset();
	Em_Enter();
}

LOCALFUNC ATTep LocalFindATTel(CPTR addr)
{
	ATTep prev;
	ATTep p;

	p = V_regs.HeadATTel;
	if ((addr & p->cmpmask) != p->cmpvalu) {
		do {
			prev = p;
			p = p->Next;
		} while ((addr & p->cmpmask) != p->cmpvalu);

		{
			ATTep next = p->Next;

			if (nullpr == next) {
				/* don't move the end guard */
			} else {
				/* move to first */
				prev->Next = next;
				p->Next = V_regs.HeadATTel;
				V_regs.HeadATTel = p;
			}
		}
	}

	return p;
}

LOCALPROC SetUpMATC(
	MATCp CurMATC,
	ATTep p)
{
	CurMATC->cmpmask = p->cmpmask;
	CurMATC->usemask = p->usemask;
	CurMATC->cmpvalu = p->cmpvalu;
	CurMATC->usebase = p->usebase;
}

LOCALFUNC uint32_t reg_call get_byte_ext(CPTR addr)
{
	ATTep p;
	uint8_t * m;
	uint32_t AccFlags;
	uint32_t Data;

Label_Retry:
	p = LocalFindATTel(addr);
	AccFlags = p->Access;

	if (0 != (AccFlags & kATTA_readreadymask)) {
		SetUpMATC(&V_regs.MATCrdB, p);
		m = p->usebase + (addr & p->usemask);

		Data = *m;
	} else if (0 != (AccFlags & kATTA_mmdvmask)) {
		Data = LocalMMDV_Access(p, 0, false, true, addr);
	} else if (0 != (AccFlags & kATTA_ntfymask)) {
		if (LocalMemAccessNtfy(p)) {
			goto Label_Retry;
		} else {
			Data = 0; /* fail */
		}
	} else {
		Data = 0; /* fail */
	}

	return uint32_t_FromSByte(Data);
}

LOCALPROC reg_call put_byte_ext(CPTR addr, uint32_t b)
{
	ATTep p;
	uint8_t * m;
	uint32_t AccFlags;

Label_Retry:
	p = LocalFindATTel(addr);
	AccFlags = p->Access;

	if (0 != (AccFlags & kATTA_writereadymask)) {
		SetUpMATC(&V_regs.MATCwrB, p);
		m = p->usebase + (addr & p->usemask);
		*m = b;
	} else if (0 != (AccFlags & kATTA_mmdvmask)) {
		(void) LocalMMDV_Access(p, b & 0x00FF,
			true, true, addr);
	} else if (0 != (AccFlags & kATTA_ntfymask)) {
		if (LocalMemAccessNtfy(p)) {
			goto Label_Retry;
		} else {
			/* fail */
		}
	} else {
		/* fail */
	}
}

LOCALFUNC uint32_t reg_call get_word_ext(CPTR addr)
{
	uint32_t Data;

	if (0 != (addr & 0x01)) {
		uint32_t hi = get_byte(addr);
		uint32_t lo = get_byte(addr + 1);
		Data = ((hi << 8) & 0x0000FF00)
			| (lo & 0x000000FF);
	} else {
		ATTep p;
		uint8_t * m;
		uint32_t AccFlags;

Label_Retry:
		p = LocalFindATTel(addr);
		AccFlags = p->Access;

		if (0 != (AccFlags & kATTA_readreadymask)) {
			SetUpMATC(&V_regs.MATCrdW, p);
			V_regs.MATCrdW.cmpmask |= 0x01;
			m = p->usebase + (addr & p->usemask);
			Data = do_get_mem_word(m);
		} else if (0 != (AccFlags & kATTA_mmdvmask)) {
			Data = LocalMMDV_Access(p, 0, false, false, addr);
		} else if (0 != (AccFlags & kATTA_ntfymask)) {
			if (LocalMemAccessNtfy(p)) {
				goto Label_Retry;
			} else {
				Data = 0; /* fail */
			}
		} else {
			Data = 0; /* fail */
		}
	}

	return uint32_t_FromSWord(Data);
}

LOCALPROC reg_call put_word_ext(CPTR addr, uint32_t w)
{
	if (0 != (addr & 0x01)) {
		put_byte(addr, w >> 8);
		put_byte(addr + 1, w);
	} else {
		ATTep p;
		uint8_t * m;
		uint32_t AccFlags;

Label_Retry:
		p = LocalFindATTel(addr);
		AccFlags = p->Access;

		if (0 != (AccFlags & kATTA_writereadymask)) {
			SetUpMATC(&V_regs.MATCwrW, p);
			V_regs.MATCwrW.cmpmask |= 0x01;
			m = p->usebase + (addr & p->usemask);
			do_put_mem_word(m, w);
		} else if (0 != (AccFlags & kATTA_mmdvmask)) {
			(void) LocalMMDV_Access(p, w & 0x0000FFFF,
				true, false, addr);
		} else if (0 != (AccFlags & kATTA_ntfymask)) {
			if (LocalMemAccessNtfy(p)) {
				goto Label_Retry;
			} else {
				/* fail */
			}
		} else {
			/* fail */
		}
	}
}

LOCALFUNC uint32_t reg_call get_long_misaligned_ext(CPTR addr)
{
	uint32_t hi = get_word(addr);
	uint32_t lo = get_word(addr + 2);
	uint32_t Data = ((hi << 16) & 0xFFFF0000)
		| (lo & 0x0000FFFF);

	return uint32_t_FromSLong(Data);
}

LOCALPROC reg_call put_long_misaligned_ext(CPTR addr, uint32_t l)
{
	put_word(addr, l >> 16);
	put_word(addr + 2, l);
}

#if FasterAlignedL
LOCALFUNC uint32_t reg_call get_long_ext(CPTR addr)
{
	uint32_t Data;

	if (0 != (addr & 0x03)) {
		uint32_t hi = get_word(addr);
		uint32_t lo = get_word(addr + 2);
		Data = ((hi << 16) & 0xFFFF0000)
			| (lo & 0x0000FFFF);
	} else {
		ATTep p;
		uint8_t * m;
		uint32_t AccFlags;

Label_Retry:
		p = LocalFindATTel(addr);
		AccFlags = p->Access;

		if (0 != (AccFlags & kATTA_readreadymask)) {
			SetUpMATC(&V_regs.MATCrdL, p);
			V_regs.MATCrdL.cmpmask |= 0x03;
			m = p->usebase + (addr & p->usemask);
			Data = do_get_mem_long(m);
		} else if (0 != (AccFlags & kATTA_mmdvmask)) {
			uint32_t hi = LocalMMDV_Access(p, 0,
				false, false, addr);
			uint32_t lo = LocalMMDV_Access(p, 0,
				false, false, addr + 2);
			Data = ((hi << 16) & 0xFFFF0000)
				| (lo & 0x0000FFFF);
		} else if (0 != (AccFlags & kATTA_ntfymask)) {
			if (LocalMemAccessNtfy(p)) {
				goto Label_Retry;
			} else {
				Data = 0; /* fail */
			}
		} else {
			Data = 0; /* fail */
		}
	}

	return uint32_t_FromSLong(Data);
}
#endif

#if FasterAlignedL
LOCALPROC reg_call put_long_ext(CPTR addr, uint32_t l)
{
	if (0 != (addr & 0x03)) {
		put_word(addr, l >> 16);
		put_word(addr + 2, l);
	} else {
		ATTep p;
		uint8_t * m;
		uint32_t AccFlags;

Label_Retry:
		p = LocalFindATTel(addr);
		AccFlags = p->Access;

		if (0 != (AccFlags & kATTA_writereadymask)) {
			SetUpMATC(&V_regs.MATCwrL, p);
			V_regs.MATCwrL.cmpmask |= 0x03;
			m = p->usebase + (addr & p->usemask);
			do_put_mem_long(m, l);
		} else if (0 != (AccFlags & kATTA_mmdvmask)) {
			(void) LocalMMDV_Access(p, (l >> 16) & 0x0000FFFF,
				true, false, addr);
			(void) LocalMMDV_Access(p, l & 0x0000FFFF,
				true, false, addr + 2);
		} else if (0 != (AccFlags & kATTA_ntfymask)) {
			if (LocalMemAccessNtfy(p)) {
				goto Label_Retry;
			} else {
				/* fail */
			}
		} else {
			/* fail */
		}
	}
}
#endif

LOCALPROC Recalc_PC_Block(void)
{
	ATTep p;
	CPTR curpc = m68k_getpc();

Label_Retry:
	p = LocalFindATTel(curpc);
	if (cond_rare(0 == (p->Access & kATTA_readreadymask))) {
		if (0 != (p->Access & kATTA_ntfymask)) {
			if (LocalMemAccessNtfy(p)) {
				goto Label_Retry;
			}
		}
		/* in trouble if get here */
#if ExtraAbnormalReports
		ReportAbnormalID(0x0122, "Recalc_PC_Block fails");
			/* happens on Restart */
#endif

		V_regs.pc_pLo = V_regs.fakeword;
		V_pc_p = V_regs.pc_pLo;
		V_pc_pHi = V_regs.pc_pLo + 2;
		V_regs.pc = curpc;
	} else {
		uint32_t m2 = p->usemask & ~ p->cmpmask;
		m2 = m2 & ~ (m2 + 1);

		V_pc_p = p->usebase + (curpc & p->usemask);
		V_regs.pc_pLo = V_pc_p - (curpc & m2);
		V_pc_pHi = V_regs.pc_pLo + m2 + 1;
		V_regs.pc = curpc - (V_pc_p - V_regs.pc_pLo);
	}
}

LOCALFUNC uint32_t reg_call Recalc_PC_BlockReturnUi5r(uint32_t v)
{
	/*
		Used to prevent compiler from saving
		register on the stack in calling
		functions, when Recalc_PC_Block isn't being called.
	*/
	Recalc_PC_Block();

	return v;
}

LOCALFUNC uint32_t nextilong_ext(void)
{
	uint32_t r;

	V_pc_p -= 4;

	{
		uint32_t hi = nextiword();
		uint32_t lo = nextiword();
		r = ((hi << 16) & 0xFFFF0000)
			| (lo & 0x0000FFFF);
	}

	return r;
}

LOCALPROC DoCheckExternalInterruptPending(void)
{
	uint8_t level = *V_regs.fIPL;
	if ((level > V_regs.intmask) || (level == 7)) {
#if WantCloserCyc
		V_MaxCyclesToGo -=
			(44 * kCycleScale + 5 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
		Exception(24 + level);
		V_regs.intmask = level;
	}
}

LOCALPROC do_trace(void)
{
	V_regs.TracePending = true;
	NeedToGetOut();
}

GLOBALPROC m68k_go_nCycles(uint32_t n)
{
	Em_Enter();
	V_MaxCyclesToGo += (n + V_regs.ResidualCycles);
	while (V_MaxCyclesToGo > 0) {

#if 0
		if (V_regs.ResetPending) {
			m68k_DoReset();
		}
#endif
		if (V_regs.TracePending) {
#if WantCloserCyc
			V_MaxCyclesToGo -= (34 * kCycleScale
				+ 4 * RdAvgXtraCyc + 3 * WrAvgXtraCyc);
#endif
			Exception(9);
		}
		if (V_regs.ExternalInterruptPending) {
			V_regs.ExternalInterruptPending = false;
			DoCheckExternalInterruptPending();
		}
		if (V_regs.t1 != 0) {
			do_trace();
		}
		m68k_go_MaxCycles();
		V_MaxCyclesToGo += V_regs.MoreCyclesToGo;
		V_regs.MoreCyclesToGo = 0;
	}

	V_regs.ResidualCycles = V_MaxCyclesToGo;
	V_MaxCyclesToGo = 0;
	Em_Exit();
}

GLOBALFUNC int32_t GetCyclesRemaining(void)
{
	int32_t v;

	Em_Enter();
	v = V_regs.MoreCyclesToGo + V_MaxCyclesToGo;
	Em_Exit();

	return v;
}

GLOBALPROC SetCyclesRemaining(int32_t n)
{
	Em_Enter();

	if (V_MaxCyclesToGo >= n) {
		V_regs.MoreCyclesToGo = 0;
		V_MaxCyclesToGo = n;
	} else {
		V_regs.MoreCyclesToGo = n - V_MaxCyclesToGo;
	}

	Em_Exit();
}

GLOBALFUNC ATTep FindATTel(CPTR addr)
{
	ATTep v;

	Em_Enter();
	v = LocalFindATTel(addr);
	Em_Exit();

	return v;
}

GLOBALFUNC uint8_t get_vm_byte(CPTR addr)
{
	uint8_t v;

	Em_Enter();
	v = (uint8_t) get_byte(addr);
	Em_Exit();

	return v;
}

GLOBALFUNC uint16_t get_vm_word(CPTR addr)
{
	uint16_t v;

	Em_Enter();
	v = (uint16_t) get_word(addr);
	Em_Exit();

	return v;
}

GLOBALFUNC uint32_t get_vm_long(CPTR addr)
{
	uint32_t v;

	Em_Enter();
	v = (uint32_t) get_long(addr);
	Em_Exit();

	return v;
}

GLOBALPROC put_vm_byte(CPTR addr, uint8_t b)
{
	Em_Enter();
	put_byte(addr, uint32_t_FromSByte(b));
	Em_Exit();
}

GLOBALPROC put_vm_word(CPTR addr, uint16_t w)
{
	Em_Enter();
	put_word(addr, uint32_t_FromSWord(w));
	Em_Exit();
}

GLOBALPROC put_vm_long(CPTR addr, uint32_t l)
{
	Em_Enter();
	put_long(addr, uint32_t_FromSLong(l));
	Em_Exit();
}

GLOBALPROC SetHeadATTel(ATTep p)
{
	Em_Enter();

	V_regs.MATCrdB.cmpmask = 0;
	V_regs.MATCrdB.cmpvalu = 0xFFFFFFFF;
	V_regs.MATCwrB.cmpmask = 0;
	V_regs.MATCwrB.cmpvalu = 0xFFFFFFFF;
	V_regs.MATCrdW.cmpmask = 0;
	V_regs.MATCrdW.cmpvalu = 0xFFFFFFFF;
	V_regs.MATCwrW.cmpmask = 0;
	V_regs.MATCwrW.cmpvalu = 0xFFFFFFFF;
#if FasterAlignedL
	V_regs.MATCrdL.cmpmask = 0;
	V_regs.MATCrdL.cmpvalu = 0xFFFFFFFF;
	V_regs.MATCwrL.cmpmask = 0;
	V_regs.MATCwrL.cmpvalu = 0xFFFFFFFF;
#endif
	/* force Recalc_PC_Block soon */
		V_regs.pc = m68k_getpc();
		V_regs.pc_pLo = V_pc_p;
		V_pc_pHi = V_regs.pc_pLo + 2;
	V_regs.HeadATTel = p;

	Em_Exit();
}

GLOBALPROC DiskInsertedPsuedoException(CPTR newpc, uint32_t data)
{
	Em_Enter();
	ExceptionTo(newpc
#if Use68020
		, 0
#endif
		);
	m68k_areg(7) -= 4;
	put_long(m68k_areg(7), data);
	Em_Exit();
}

GLOBALPROC m68k_IPLchangeNtfy(void)
{
	Em_Enter();
	{
		uint8_t level = *V_regs.fIPL;

		if ((level > V_regs.intmask) || (level == 7)) {
			SetExternalInterruptPending();
		}
	}
	Em_Exit();
}

#if WantDumpTable
LOCALPROC InitDumpTable(void)
{
	int32_t i;

	for (i = 0; i < kNumIKinds; ++i) {
		DumpTable[i] = 0;
	}
}

LOCALPROC DumpATable(uint32_t *p, uint32_t n)
{
	int32_t i;

	for (i = 0; i < n; ++i) {
		dbglog_writeNum(p[i]);
		dbglog_writeReturn();
	}
}

EXPORTPROC DoDumpTable(void);
GLOBALPROC DoDumpTable(void)
{
	DumpATable(DumpTable, kNumIKinds);
}
#endif

GLOBALPROC m68k_reset(void)
{
	Em_Enter();

#if WantDumpTable
	InitDumpTable();
#endif
	V_MaxCyclesToGo = 0;
	V_regs.MoreCyclesToGo = 0;
	V_regs.ResidualCycles = 0;
	V_pc_p = (uint8_t *)nullpr;
	V_pc_pHi = (uint8_t *)nullpr;
	V_regs.pc_pLo = (uint8_t *)nullpr;

	do_put_mem_word(V_regs.fakeword, 0x4AFC);
		/* illegal instruction opcode */

#if 0
	V_regs.ResetPending = true;
	NeedToGetOut();
#else
/* Sets the MC68000 reset jump vector... */
	m68k_setpc(get_long(0x00000004));

/* Sets the initial stack vector... */
	m68k_areg(7) = get_long(0x00000000);

	V_regs.s = 1;
#if Use68020
	V_regs.m = 0;
	V_regs.t0 = 0;
#endif
	V_regs.t1 = 0;
	ZFLG = CFLG = NFLG = VFLG = 0;

	V_regs.LazyFlagKind = kLazyFlagsDefault;
	V_regs.LazyXFlagKind = kLazyFlagsDefault;

	V_regs.ExternalInterruptPending = false;
	V_regs.TracePending = false;
	V_regs.intmask = 7;

#if Use68020
	V_regs.sfc = 0;
	V_regs.dfc = 0;
	V_regs.vbr = 0;
	V_regs.cacr = 0;
	V_regs.caar = 0;
#endif
#endif

	Em_Exit();
}

#if SmallGlobals
GLOBALPROC MINEM68K_ReserveAlloc(void)
{
	ReserveAllocOneBlock((uint8_t * *)&regs.disp_table,
		disp_table_sz * 8, 6, false);
}
#endif

GLOBALPROC MINEM68K_Init(
	uint8_t *fIPL)
{
	regs.fIPL = fIPL;
#ifdef r_regs
	regs.save_regs = &regs;
#endif

	M68KITAB_setup(regs.disp_table);
}
