/*
	GLOBGLUE.c

	Copyright (C) 2003 Bernd Schmidt, Philip Cummins, Paul C. Pratt

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
	GLOBal GLUE (or GLOB of GLUE)

	Holds the program together.

	Some code here adapted from "custom.c" in vMac by Philip Cummins,
	in turn descended from code in the Un*x Amiga Emulator by
	Bernd Schmidt.
	
	TODO: Try to yank as much out of this as we possibly can.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"

#include "UI/MYOSGLUE.h"
#include "UTIL/ENDIANAC.h"
#include "EMCONFIG.h"
#endif

#include "GLOBGLUE.h"
#include "HW/RAM/RAMADDR.h"

/*
	ReportAbnormalID unused 0x111D - 0x11FF
*/

/*
	ReportAbnormalID ranges unused 0x12xx - 0xFFxx
*/

// Global variables (temporary location?)
bool SpeedStopped = false;
bool RunInBackground = (WantInitRunInBackground != 0);
bool WantFullScreen = (WantInitFullScreen != 0);
bool WantMagnify = (WantInitMagnify != 0);
bool RequestInsertDisk = false;
uint8_t RequestIthDisk = 0;
bool ControlKeyPressed = false;


extern void m68k_reset(void);
extern void IWM_Reset(void);
extern void SCC_Reset(void);
extern void SCSI_Reset(void);
extern void VIA1_Reset(void);
#if EmVIA2
extern void VIA2_Reset(void);
#endif
extern void Sony_Reset(void);

extern void ExtnDisk_Access(CPTR p);
extern void ExtnSony_Access(CPTR p);
#if EmVidCard
extern void ExtnVideo_Access(CPTR p);
#endif

extern void Sony_SetQuitOnEject(void);

extern void m68k_IPLchangeNtfy(void);
extern void MINEM68K_Init(
	uint8_t *fIPL);

extern uint32_t GetCyclesRemaining(void);
extern void SetCyclesRemaining(uint32_t n);

extern void SetHeadATTel(ATTep p);
extern ATTep FindATTel(CPTR addr);

extern uint32_t SCSI_Access(uint32_t Data, bool WriteMem, CPTR addr);
extern uint32_t SCC_Access(uint32_t Data, bool WriteMem, CPTR addr);
extern uint32_t IWM_Access(uint32_t Data, bool WriteMem, CPTR addr);
extern uint32_t VIA1_Access(uint32_t Data, bool WriteMem, CPTR addr);
#if EmVIA2
extern uint32_t VIA2_Access(uint32_t Data, bool WriteMem, CPTR addr);
#endif
#if EmASC
extern uint32_t ASC_Access(uint32_t Data, bool WriteMem, CPTR addr);
#endif

extern uint8_t get_vm_byte(CPTR addr);
extern uint16_t get_vm_word(CPTR addr);
extern uint32_t get_vm_long(CPTR addr);

extern void put_vm_byte(CPTR addr, uint8_t b);
extern void put_vm_word(CPTR addr, uint16_t w);
extern void put_vm_long(CPTR addr, uint32_t l);

 uint32_t disk_icon_addr;

void customreset(void)
{
	IWM_Reset();
	SCC_Reset();
	SCSI_Reset();
	VIA1_Reset();
#if EmVIA2
	VIA2_Reset();
#endif
	Sony_Reset();
	Extn_Reset();
#if CurEmMd <= kEmMd_Plus
	WantMacReset = true;
	/*
		kludge, code in Finder appears
		to do RESET and not expect
		to come back. Maybe asserting
		the RESET somehow causes
		other hardware compenents to
		later reset the 68000.
	*/
#endif
}

 uint8_t * RAM = nullpr;

#if EmVidCard
 uint8_t * VidROM = nullpr;
#endif

#if IncludeVidMem
 uint8_t * VidMem = nullpr;
#endif

 uint8_t Wires[kNumWires];


#if WantDisasm
extern void m68k_WantDisasmContext(void);
#endif

#if WantDisasm
void dbglog_StartLine(void)
{
	m68k_WantDisasmContext();
	dbglog_writeCStr(" ");
}
#endif

#if dbglog_HAVE
void dbglog_WriteMemArrow(bool WriteMem)
{
	if (WriteMem) {
		dbglog_writeCStr(" <- ");
	} else {
		dbglog_writeCStr(" -> ");
	}
}
#endif

#if dbglog_HAVE
void dbglog_AddrAccess(char *s, uint32_t Data,
	bool WriteMem, uint32_t addr)
{
	dbglog_StartLine();
	dbglog_writeCStr(s);
	dbglog_writeCStr("[");
	dbglog_writeHex(addr);
	dbglog_writeCStr("]");
	dbglog_WriteMemArrow(WriteMem);
	dbglog_writeHex(Data);
	dbglog_writeReturn();
}
#endif

#if dbglog_HAVE
void dbglog_Access(char *s, uint32_t Data, bool WriteMem)
{
	dbglog_StartLine();
	dbglog_writeCStr(s);
	dbglog_WriteMemArrow(WriteMem);
	dbglog_writeHex(Data);
	dbglog_writeReturn();
}
#endif

#if dbglog_HAVE
void dbglog_WriteNote(char *s)
{
	dbglog_StartLine();
	dbglog_writeCStr(s);
	dbglog_writeReturn();
}
#endif

#if dbglog_HAVE
void dbglog_WriteSetBool(char *s, bool v)
{
	dbglog_StartLine();
	dbglog_writeCStr(s);
	dbglog_writeCStr(" <- ");
	if (v) {
		dbglog_writeCStr("1");
	} else {
		dbglog_writeCStr("0");
	}
	dbglog_writeReturn();
}
#endif

#if WantAbnormalReports
static bool GotOneAbnormal = false;
#endif

#ifndef ReportAbnormalInterrupt
#define ReportAbnormalInterrupt 0
#endif

#if WantAbnormalReports
void DoReportAbnormalID(uint16_t id
#if dbglog_HAVE
	, char *s
#endif
	)
{
#if dbglog_HAVE
	dbglog_StartLine();
	dbglog_writeCStr("*** abnormal : ");
	dbglog_writeCStr(s);
	dbglog_writeReturn();
#endif

	if (! GotOneAbnormal) {
		WarnMsgAbnormalID(id);
#if ReportAbnormalInterrupt
		SetInterruptButton(true);
#endif
		GotOneAbnormal = true;
	}
}
#endif



#if IncludeExtnPbufs
static MacErr_t PbufTransferVM(CPTR Buffera,
	tPbuf i, uint32_t offset, uint32_t count, bool IsWrite)
{
	MacErr_t result;
	uint32_t contig;
	uint8_t * Buffer;

label_1:
	if (0 == count) {
		result = mnvm_noErr;
	} else {
		Buffer = get_real_address0(count, ! IsWrite, Buffera, &contig);
		if (0 == contig) {
			result = mnvm_miscErr;
		} else {
			PbufTransfer(Buffer, i, offset, contig, IsWrite);
			offset += contig;
			Buffera += contig;
			count -= contig;
			goto label_1;
		}
	}

	return result;
}
#endif

/* extension mechanism */

#if IncludeExtnPbufs
#define kCmndPbufFeatures 1
#define kCmndPbufNew 2
#define kCmndPbufDispose 3
#define kCmndPbufGetSize 4
#define kCmndPbufTransfer 5
#endif

#if IncludeExtnPbufs
static void ExtnParamBuffers_Access(CPTR p)
{
	MacErr_t result = mnvm_controlErr;

	switch (get_vm_word(p + ExtnDat_commnd)) {
		case kCmndVersion:
			put_vm_word(p + ExtnDat_version, 1);
			result = mnvm_noErr;
			break;
		case kCmndPbufFeatures:
			put_vm_long(p + ExtnDat_params + 0, 0);
			result = mnvm_noErr;
			break;
		case kCmndPbufNew:
			{
				tPbuf Pbuf_No;
				uint32_t count = get_vm_long(p + ExtnDat_params + 4);
				/* reserved word at offset 2, should be zero */
				result = PbufNew(count, &Pbuf_No);
				put_vm_word(p + ExtnDat_params + 0, Pbuf_No);
			}
			break;
		case kCmndPbufDispose:
			{
				tPbuf Pbuf_No = get_vm_word(p + ExtnDat_params + 0);
				/* reserved word at offset 2, should be zero */
				result = CheckPbuf(Pbuf_No);
				if (mnvm_noErr == result) {
					PbufDispose(Pbuf_No);
				}
			}
			break;
		case kCmndPbufGetSize:
			{
				uint32_t Count;
				tPbuf Pbuf_No = get_vm_word(p + ExtnDat_params + 0);
				/* reserved word at offset 2, should be zero */

				result = PbufGetSize(Pbuf_No, &Count);
				if (mnvm_noErr == result) {
					put_vm_long(p + ExtnDat_params + 4, Count);
				}
			}
			break;
		case kCmndPbufTransfer:
			{
				uint32_t PbufCount;
				tPbuf Pbuf_No = get_vm_word(p + ExtnDat_params + 0);
				/* reserved word at offset 2, should be zero */
				uint32_t offset = get_vm_long(p + ExtnDat_params + 4);
				uint32_t count = get_vm_long(p + ExtnDat_params + 8);
				CPTR Buffera = get_vm_long(p + ExtnDat_params + 12);
				bool IsWrite =
					(get_vm_word(p + ExtnDat_params + 16) != 0);
				result = PbufGetSize(Pbuf_No, &PbufCount);
				if (mnvm_noErr == result) {
					uint32_t endoff = offset + count;
					if ((endoff < offset) /* overflow */
						|| (endoff > PbufCount))
					{
						result = mnvm_eofErr;
					} else {
						result = PbufTransferVM(Buffera,
							Pbuf_No, offset, count, IsWrite);
					}
				}
			}
			break;
	}

	put_vm_word(p + ExtnDat_result, result);
}
#endif

#if IncludeExtnHostTextClipExchange
#define kCmndHTCEFeatures 1
#define kCmndHTCEExport 2
#define kCmndHTCEImport 3
#endif

#if IncludeExtnHostTextClipExchange
static void ExtnHostTextClipExchange_Access(CPTR p)
{
	MacErr_t result = mnvm_controlErr;

	switch (get_vm_word(p + ExtnDat_commnd)) {
		case kCmndVersion:
			put_vm_word(p + ExtnDat_version, 1);
			result = mnvm_noErr;
			break;
		case kCmndHTCEFeatures:
			put_vm_long(p + ExtnDat_params + 0, 0);
			result = mnvm_noErr;
			break;
		case kCmndHTCEExport:
			{
				tPbuf Pbuf_No = get_vm_word(p + ExtnDat_params + 0);

				result = CheckPbuf(Pbuf_No);
				if (mnvm_noErr == result) {
					result = HTCEexport(Pbuf_No);
				}
			}
			break;
		case kCmndHTCEImport:
			{
				tPbuf Pbuf_No;
				result = HTCEimport(&Pbuf_No);
				put_vm_word(p + ExtnDat_params + 0, Pbuf_No);
			}
			break;
	}

	put_vm_word(p + ExtnDat_result, result);
}
#endif

#define kFindExtnExtension 0x64E1F58A
#define kDiskDriverExtension 0x4C9219E6
#if IncludeExtnPbufs
#define kHostParamBuffersExtension 0x314C87BF
#endif
#if IncludeExtnHostTextClipExchange
#define kHostClipExchangeExtension 0x27B130CA
#endif

#define kCmndFindExtnFind 1
#define kCmndFindExtnId2Code 2
#define kCmndFindExtnCount 3

#define kParamFindExtnTheExtn 8
#define kParamFindExtnTheId 12

static void ExtnFind_Access(CPTR p)
{
	MacErr_t result = mnvm_controlErr;

	switch (get_vm_word(p + ExtnDat_commnd)) {
		case kCmndVersion:
			put_vm_word(p + ExtnDat_version, 1);
			result = mnvm_noErr;
			break;
		case kCmndFindExtnFind:
			{
				uint32_t extn = get_vm_long(p + kParamFindExtnTheExtn);

				if (extn == kDiskDriverExtension) {
					put_vm_word(p + kParamFindExtnTheId, kExtnDisk);
					result = mnvm_noErr;
				} else
#if IncludeExtnPbufs
				if (extn == kHostParamBuffersExtension) {
					put_vm_word(p + kParamFindExtnTheId,
						kExtnParamBuffers);
					result = mnvm_noErr;
				} else
#endif
#if IncludeExtnHostTextClipExchange
				if (extn == kHostClipExchangeExtension) {
					put_vm_word(p + kParamFindExtnTheId,
						kExtnHostTextClipExchange);
					result = mnvm_noErr;
				} else
#endif
				if (extn == kFindExtnExtension) {
					put_vm_word(p + kParamFindExtnTheId,
						kExtnFindExtn);
					result = mnvm_noErr;
				} else
				{
					/* not found */
				}
			}
			break;
		case kCmndFindExtnId2Code:
			{
				uint16_t extn = get_vm_word(p + kParamFindExtnTheId);

				if (extn == kExtnDisk) {
					put_vm_long(p + kParamFindExtnTheExtn,
						kDiskDriverExtension);
					result = mnvm_noErr;
				} else
#if IncludeExtnPbufs
				if (extn == kExtnParamBuffers) {
					put_vm_long(p + kParamFindExtnTheExtn,
						kHostParamBuffersExtension);
					result = mnvm_noErr;
				} else
#endif
#if IncludeExtnHostTextClipExchange
				if (extn == kExtnHostTextClipExchange) {
					put_vm_long(p + kParamFindExtnTheExtn,
						kHostClipExchangeExtension);
					result = mnvm_noErr;
				} else
#endif
				if (extn == kExtnFindExtn) {
					put_vm_long(p + kParamFindExtnTheExtn,
						kFindExtnExtension);
					result = mnvm_noErr;
				} else
				{
					/* not found */
				}
			}
			break;
		case kCmndFindExtnCount:
			put_vm_word(p + kParamFindExtnTheId, kNumExtns);
			result = mnvm_noErr;
			break;
	}

	put_vm_word(p + ExtnDat_result, result);
}

#define kDSK_Params_Hi 0
#define kDSK_Params_Lo 1
#define kDSK_QuitOnEject 3 /* obsolete */

static uint16_t ParamAddrHi;

static void Extn_Access(uint32_t Data, CPTR addr)
{
	switch (addr) {
		case kDSK_Params_Hi:
			ParamAddrHi = Data;
			break;
		case kDSK_Params_Lo:
			{
				CPTR p = ParamAddrHi << 16 | Data;

				ParamAddrHi = (uint16_t) - 1;
				if (kcom_callcheck == get_vm_word(p + ExtnDat_checkval))
				{
					put_vm_word(p + ExtnDat_checkval, 0);

					switch (get_vm_word(p + ExtnDat_extension)) {
						case kExtnFindExtn:
							ExtnFind_Access(p);
							break;
#if EmVidCard
						case kExtnVideo:
							ExtnVideo_Access(p);
							break;
#endif
#if IncludeExtnPbufs
						case kExtnParamBuffers:
							ExtnParamBuffers_Access(p);
							break;
#endif
#if IncludeExtnHostTextClipExchange
						case kExtnHostTextClipExchange:
							ExtnHostTextClipExchange_Access(p);
							break;
#endif
						case kExtnDisk:
							ExtnDisk_Access(p);
							break;
						case kExtnSony:
							ExtnSony_Access(p);
							break;
						default:
							put_vm_word(p + ExtnDat_result,
								mnvm_controlErr);
							break;
					}
				}
			}
			break;
		case kDSK_QuitOnEject:
			/* obsolete, kept for compatibility */
			Sony_SetQuitOnEject();
			break;
	}
}

void Extn_Reset(void)
{
	ParamAddrHi = (uint16_t) - 1;
}

enum {
	kMMDV_VIA1,
#if EmVIA2
	kMMDV_VIA2,
#endif
	kMMDV_SCC,
	kMMDV_Extn,
#if EmASC
	kMMDV_ASC,
#endif
	kMMDV_SCSI,
	kMMDV_IWM,

	kNumMMDVs
};

enum {
#if CurEmMd >= kEmMd_SE
	kMAN_OverlayOff,
#endif

	kNumMANs
};


static ATTer ATTListA[MaxATTListN];
static uint16_t LastATTel;


static void AddToATTList(ATTep p)
{
	uint16_t NewLast = LastATTel + 1;
	if (NewLast >= MaxATTListN) {
		ReportAbnormalID(0x1101, "MaxATTListN not big enough");
	} else {
		ATTListA[LastATTel] = *p;
		LastATTel = NewLast;
	}
}

static void InitATTList(void)
{
	LastATTel = 0;
}

static void FinishATTList(void)
{
	{
		/* add guard */
		ATTer r;

		r.cmpmask = 0;
		r.cmpvalu = 0;
		r.usemask = 0;
		r.usebase = nullpr;
		r.Access = 0;
		AddToATTList(&r);
	}

	{
		uint16_t i = LastATTel;
		ATTep p = &ATTListA[LastATTel];
		ATTep h = nullpr;

		while (0 != i) {
			--i;
			--p;
			p->Next = h;
			h = p;
		}

#if 0 /* verify list. not for final version */
		{
			ATTep q1;
			ATTep q2;
			for (q1 = h; nullpr != q1->Next; q1 = q1->Next) {
				if ((q1->cmpvalu & ~ q1->cmpmask) != 0) {
					ReportAbnormalID(0x1102, "ATTListA bad entry");
				}
				for (q2 = q1->Next; nullpr != q2->Next; q2 = q2->Next) {
					uint32_t common_mask = (q1->cmpmask) & (q2->cmpmask);
					if ((q1->cmpvalu & common_mask) ==
						(q2->cmpvalu & common_mask))
					{
						ReportAbnormalID(0x1103, "ATTListA Conflict");
					}
				}
			}
		}
#endif

		SetHeadATTel(h);
	}
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
static void SetUp_RAM24(void)
{
	ATTer r;
	uint32_t bankbit = 0x00100000 << (((VIA2_iA7 << 1) | VIA2_iA6) << 1);

#if kRAMa_Size == kRAMb_Size
	if (kRAMa_Size == bankbit) {
		/* properly set up balanced RAM */
		r.cmpmask = 0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1);
		r.cmpvalu = 0;
		r.usemask = ((1 << kRAM_ln2Spc) - 1) & (kRAM_Size - 1);
		r.usebase = RAM;
		r.Access = kATTA_readwritereadymask;
		AddToATTList(&r);
	} else
#endif
	{
		bankbit &= 0x00FFFFFF; /* if too large, always use RAMa */

		if (0 != bankbit) {
#if kRAMb_Size != 0
			r.cmpmask = bankbit
				| (0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1));
			r.cmpvalu = bankbit;
			r.usemask = ((1 << kRAM_ln2Spc) - 1) & (kRAMb_Size - 1);
			r.usebase = kRAMa_Size + RAM;
			r.Access = kATTA_readwritereadymask;
			AddToATTList(&r);
#endif
		}

		{
			r.cmpmask = bankbit
				| (0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1));
			r.cmpvalu = 0;
			r.usemask = ((1 << kRAM_ln2Spc) - 1) & (kRAMa_Size - 1);
			r.usebase = RAM;
			r.Access = kATTA_readwritereadymask;
			AddToATTList(&r);
		}
	}
}
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
static void SetUp_io(void)
{
	ATTer r;

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_VIA1;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x2000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x2000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_VIA2;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x4000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x4000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_SCC;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x0C000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x0C000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_Extn;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x10000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x10000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_SCSI;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x14000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x14000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_ASC;
	AddToATTList(&r);

	if (Addr32) {
		r.cmpmask = 0xFF01E000;
		r.cmpvalu = 0x50000000 | 0x16000;
	} else {
		r.cmpmask = 0x00F1E000;
		r.cmpvalu = 0x00F00000 | 0x16000;
	}
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_IWM;
	AddToATTList(&r);

#if 0
		case 14:
			/*
				fail, nothing supposed to be here,
				but rom accesses it anyway
			*/
			{
				uint32_t addr2 = addr & 0x1FFFF;

				if ((addr2 != 0x1DA00) && (addr2 != 0x1DC00)) {
					ReportAbnormalID(0x1104, "another unknown access");
				}
			}
			get_fail_realblock(p);
			break;
#endif
}
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
static void SetUp_address24(void)
{
	ATTer r;

#if 0
	if (MemOverlay) {
		ReportAbnormalID(0x1105, "Overlay with 24 bit addressing");
	}
#endif

	if (MemOverlay) {
		r.cmpmask = Overlay_ROM_CmpZeroMask |
			(0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1));
		r.cmpvalu = kRAM_Base;
		r.usemask = kROM_Size - 1;
		r.usebase = ROM;
		r.Access = kATTA_readreadymask;
		AddToATTList(&r);
	} else {
		SetUp_RAM24();
	}

	r.cmpmask = kROM_cmpmask;
	r.cmpvalu = kROM_Base;
	r.usemask = kROM_Size - 1;
	r.usebase = ROM;
	r.Access = kATTA_readreadymask;
	AddToATTList(&r);

	r.cmpmask = 0x00FFFFFF & ~ (0x100000 - 1);
	r.cmpvalu = 0x900000;
	r.usemask = (kVidMemRAM_Size - 1) & (0x100000 - 1);
	r.usebase = VidMem;
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#if kVidMemRAM_Size >= 0x00200000
	r.cmpmask = 0x00FFFFFF & ~ (0x100000 - 1);
	r.cmpvalu = 0xA00000;
	r.usemask = (0x100000 - 1);
	r.usebase = VidMem + (1 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif
#if kVidMemRAM_Size >= 0x00400000
	r.cmpmask = 0x00FFFFFF & ~ (0x100000 - 1);
	r.cmpvalu = 0xB00000;
	r.usemask = (0x100000 - 1);
	r.usebase = VidMem + (2 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
	r.cmpmask = 0x00FFFFFF & ~ (0x100000 - 1);
	r.cmpvalu = 0xC00000;
	r.usemask = (0x100000 - 1);
	r.usebase = VidMem + (3 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif

	SetUp_io();
}
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
static void SetUp_address32(void)
{
	ATTer r;

	if (MemOverlay) {
		r.cmpmask = ~ ((1 << 30) - 1);
		r.cmpvalu = 0;
		r.usemask = kROM_Size - 1;
		r.usebase = ROM;
		r.Access = kATTA_readreadymask;
		AddToATTList(&r);
	} else {
		uint32_t bankbit =
			0x00100000 << (((VIA2_iA7 << 1) | VIA2_iA6) << 1);
#if kRAMa_Size == kRAMb_Size
		if (kRAMa_Size == bankbit) {
			/* properly set up balanced RAM */
			r.cmpmask = ~ ((1 << 30) - 1);
			r.cmpvalu = 0;
			r.usemask = kRAM_Size - 1;
			r.usebase = RAM;
			r.Access = kATTA_readwritereadymask;
			AddToATTList(&r);
		} else
#endif
		{
#if kRAMb_Size != 0
			r.cmpmask = bankbit | ~ ((1 << 30) - 1);
			r.cmpvalu = bankbit;
			r.usemask = kRAMb_Size - 1;
			r.usebase = kRAMa_Size + RAM;
			r.Access = kATTA_readwritereadymask;
			AddToATTList(&r);
#endif

			r.cmpmask = bankbit | ~ ((1 << 30) - 1);
			r.cmpvalu = 0;
			r.usemask = kRAMa_Size - 1;
			r.usebase = RAM;
			r.Access = kATTA_readwritereadymask;
			AddToATTList(&r);
		}
	}

	r.cmpmask = ~ ((1 << 28) - 1);
	r.cmpvalu = 0x40000000;
	r.usemask = kROM_Size - 1;
	r.usebase = ROM;
	r.Access = kATTA_readreadymask;
	AddToATTList(&r);

#if 0
	/* haven't persuaded emulated computer to look here yet. */
	/* NuBus super space */
	r.cmpmask = ~ ((1 << 28) - 1);
	r.cmpvalu = 0x90000000;
	r.usemask = kVidMemRAM_Size - 1;
	r.usebase = VidMem;
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif

	/* Standard NuBus space */
	r.cmpmask = ~ ((1 << 20) - 1);
	r.cmpvalu = 0xF9F00000;
	r.usemask = kVidROM_Size - 1;
	r.usebase = VidROM;
	r.Access = kATTA_readreadymask;
	AddToATTList(&r);
#if 0
	r.cmpmask = ~ 0x007FFFFF;
	r.cmpvalu = 0xF9000000;
	r.usemask = 0x007FFFFF & (kVidMemRAM_Size - 1);
	r.usebase = VidMem;
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif

	r.cmpmask = ~ 0x000FFFFF;
	r.cmpvalu = 0xF9900000;
	r.usemask = 0x000FFFFF & (kVidMemRAM_Size - 1);
	r.usebase = VidMem;
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
/* kludge to allow more than 1M of Video Memory */
#if kVidMemRAM_Size >= 0x00200000
	r.cmpmask = ~ 0x000FFFFF;
	r.cmpvalu = 0xF9A00000;
	r.usemask = 0x000FFFFF & (kVidMemRAM_Size - 1);
	r.usebase = VidMem + (1 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif
#if kVidMemRAM_Size >= 0x00400000
	r.cmpmask = ~ 0x000FFFFF;
	r.cmpvalu = 0xF9B00000;
	r.usemask = 0x000FFFFF & (kVidMemRAM_Size - 1);
	r.usebase = VidMem + (2 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
	r.cmpmask = ~ 0x000FFFFF;
	r.cmpvalu = 0xF9C00000;
	r.usemask = 0x000FFFFF & (kVidMemRAM_Size - 1);
	r.usebase = VidMem + (3 << 20);
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif

	SetUp_io();

#if 0
	if ((addr >= 0x58000000) && (addr < 0x58000004)) {
		/* test hardware. fail */
	}
#endif
}
#endif

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
static void SetUp_address(void)
{
	if (Addr32) {
		SetUp_address32();
	} else {
		SetUp_address24();
	}
}
#endif

/*
	unlike in the real Mac Plus, Mini vMac
	will allow misaligned memory access,
	since it is easier to allow it than
	it is to correctly simulate a bus error
	and back out of the current instruction.
*/

#ifndef ln2mtb
#define AddToATTListWithMTB AddToATTList
#else
static void AddToATTListWithMTB(ATTep p)
{
	/*
		Test of memory mapping system.
	*/
	ATTer r;

	r.Access = p->Access;
	r.cmpmask = p->cmpmask | (1 << ln2mtb);
	r.usemask = p->usemask & ~ (1 << ln2mtb);

	r.cmpvalu = p->cmpvalu + (1 << ln2mtb);
	r.usebase = p->usebase;
	AddToATTList(&r);

	r.cmpvalu = p->cmpvalu;
	r.usebase = p->usebase + (1 << ln2mtb);
	AddToATTList(&r);
}
#endif

#if (CurEmMd != kEmMd_II) && (CurEmMd != kEmMd_IIx)
static void SetUp_RAM24(void)
{
	ATTer r;

#if (0 == kRAMb_Size) || (kRAMa_Size == kRAMb_Size)
	r.cmpmask = 0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1);
	r.cmpvalu = kRAM_Base;
	r.usemask = kRAM_Size - 1;
	r.usebase = RAM;
	r.Access = kATTA_readwritereadymask;
	AddToATTListWithMTB(&r);
#else
	/* unbalanced memory */

#if 0 != (0x00FFFFFF & kRAMa_Size)
	/* condition should always be true if configuration file right */
	r.cmpmask = 0x00FFFFFF & (kRAMa_Size | ~ ((1 << kRAM_ln2Spc) - 1));
	r.cmpvalu = kRAM_Base + kRAMa_Size;
	r.usemask = kRAMb_Size - 1;
	r.usebase = kRAMa_Size + RAM;
	r.Access = kATTA_readwritereadymask;
	AddToATTListWithMTB(&r);
#endif

	r.cmpmask = 0x00FFFFFF & (kRAMa_Size | ~ ((1 << kRAM_ln2Spc) - 1));
	r.cmpvalu = kRAM_Base;
	r.usemask = kRAMa_Size - 1;
	r.usebase = RAM;
	r.Access = kATTA_readwritereadymask;
	AddToATTListWithMTB(&r);
#endif
}
#endif

#if (CurEmMd != kEmMd_II) && (CurEmMd != kEmMd_IIx)
static void SetUp_address(void)
{
	ATTer r;

	if (MemOverlay) {
		r.cmpmask = Overlay_ROM_CmpZeroMask |
			(0x00FFFFFF & ~ ((1 << kRAM_ln2Spc) - 1));
		r.cmpvalu = kRAM_Base;
		r.usemask = kROM_Size - 1;
		r.usebase = ROM;
		r.Access = kATTA_readreadymask;
		AddToATTListWithMTB(&r);
	} else {
		SetUp_RAM24();
	}

	r.cmpmask = kROM_cmpmask;
	r.cmpvalu = kROM_Base;
#if (CurEmMd >= kEmMd_SE)
	if (MemOverlay) {
		r.usebase = nullpr;
		r.Access = kATTA_ntfymask;
		r.Ntfy = kMAN_OverlayOff;
		AddToATTList(&r);
	} else
#endif
	{
		r.usemask = kROM_Size - 1;
		r.usebase = ROM;
		r.Access = kATTA_readreadymask;
		AddToATTListWithMTB(&r);
	}

	if (MemOverlay) {
		r.cmpmask = 0x00E00000;
		r.cmpvalu = kRAM_Overlay_Base;
#if (0 == kRAMb_Size) || (kRAMa_Size == kRAMb_Size)
		r.usemask = kRAM_Size - 1;
			/* note that cmpmask and usemask overlap for 4M */
		r.usebase = RAM;
		r.Access = kATTA_readwritereadymask;
#else
		/* unbalanced memory */
		r.usemask = kRAMb_Size - 1;
		r.usebase = kRAMa_Size + RAM;
		r.Access = kATTA_readwritereadymask;
#endif
		AddToATTListWithMTB(&r);
	}

#if IncludeVidMem
	r.cmpmask = 0x00FFFFFF & ~ ((1 << kVidMem_ln2Spc) - 1);
	r.cmpvalu = kVidMem_Base;
	r.usemask = kVidMemRAM_Size - 1;
	r.usebase = VidMem;
	r.Access = kATTA_readwritereadymask;
	AddToATTList(&r);
#endif

	r.cmpmask = 0x00FFFFFF & ~ ((1 << kVIA1_ln2Spc) - 1);
	r.cmpvalu = kVIA1_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_VIA1;
	AddToATTList(&r);

	r.cmpmask = 0x00FFFFFF & ~ ((1 << kSCC_ln2Spc) - 1);
	r.cmpvalu = kSCCRd_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_SCC;
	AddToATTList(&r);

	r.cmpmask = 0x00FFFFFF & ~ ((1 << kExtn_ln2Spc) - 1);
	r.cmpvalu = kExtn_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_Extn;
	AddToATTList(&r);

#if CurEmMd == kEmMd_PB100
	r.cmpmask = 0x00FFFFFF & ~ ((1 << kASC_ln2Spc) - 1);
	r.cmpvalu = kASC_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_ASC;
	AddToATTList(&r);
#endif

	r.cmpmask = 0x00FFFFFF & ~ ((1 << kSCSI_ln2Spc) - 1);
	r.cmpvalu = kSCSI_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_SCSI;
	AddToATTList(&r);

	r.cmpmask = 0x00FFFFFF & ~ ((1 << kIWM_ln2Spc) - 1);
	r.cmpvalu = kIWM_Block_Base;
	r.usebase = nullpr;
	r.Access = kATTA_mmdvmask;
	r.MMDV = kMMDV_IWM;
	AddToATTList(&r);
}
#endif

static void SetUpMemBanks(void)
{
	InitATTList();

	SetUp_address();

	FinishATTList();
}

#if 0
static void get_fail_realblock(ATTep p)
{
	p->cmpmask = 0;
	p->cmpvalu = 0xFFFFFFFF;
	p->usemask = 0;
	p->usebase = nullpr;
	p->Access = 0;
}
#endif

 uint32_t MMDV_Access(ATTep p, uint32_t Data,
	bool WriteMem, bool ByteSize, CPTR addr)
{
	switch (p->MMDV) {
		case kMMDV_VIA1:
			if (! ByteSize) {
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
				if (WriteMem && (addr == 0xF40006)) {
					/* for weirdness on shutdown in System 6 */
#if 0
					VIA1_Access((Data >> 8) & 0x00FF, WriteMem,
							(addr >> 9) & kVIA1_Mask);
					VIA1_Access((Data) & 0x00FF, WriteMem,
							(addr >> 9) & kVIA1_Mask);
#endif
				} else
#endif
				{
					ReportAbnormalID(0x1106, "access VIA1 word");
				}
			} else if ((addr & 1) != 0) {
				ReportAbnormalID(0x1107, "access VIA1 odd");
			} else {
#if CurEmMd != kEmMd_PB100
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
				if ((addr & 0x000001FE) != 0x00000000)
#else
				if ((addr & 0x000FE1FE) != 0x000FE1FE)
#endif
				{
					ReportAbnormalID(0x1108,
						"access VIA1 nonstandard address");
				}
#endif
				Data = VIA1_Access(Data, WriteMem,
					(addr >> 9) & kVIA1_Mask);
			}

			break;
#if EmVIA2
		case kMMDV_VIA2:
			if (! ByteSize) {
				if ((! WriteMem)
					&& ((0x3e00 == (addr & 0x1FFFF))
						|| (0x3e02 == (addr & 0x1FFFF))))
				{
					/* for weirdness at offset 0x71E in ROM */
					Data =
						(VIA2_Access(Data, WriteMem,
							(addr >> 9) & kVIA2_Mask) << 8)
						| VIA2_Access(Data, WriteMem,
							(addr >> 9) & kVIA2_Mask);

				} else {
					ReportAbnormalID(0x1109, "access VIA2 word");
				}
			} else if ((addr & 1) != 0) {
				if (0x3FFF == (addr & 0x1FFFF)) {
					/*
						for weirdness at offset 0x7C4 in ROM.
						looks like bug.
					*/
					Data = VIA2_Access(Data, WriteMem,
						(addr >> 9) & kVIA2_Mask);
				} else {
					ReportAbnormalID(0x110A, "access VIA2 odd");
				}
			} else {
				if ((addr & 0x000001FE) != 0x00000000) {
					ReportAbnormalID(0x110B,
						"access VIA2 nonstandard address");
				}
				Data = VIA2_Access(Data, WriteMem,
					(addr >> 9) & kVIA2_Mask);
			}
			break;
#endif
		case kMMDV_SCC:

#if (CurEmMd >= kEmMd_SE) \
	&& ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))

			if ((addr & 0x00100000) == 0) {
				ReportAbnormalID(0x110C,
					"access SCC unassigned address");
			} else
#endif
			if (! ByteSize) {
				ReportAbnormalID(0x110D, "Attemped Phase Adjust");
			} else
#if ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))
			if (WriteMem != ((addr & 1) != 0)) {
				if (WriteMem) {
#if CurEmMd >= kEmMd_512Ke
#if CurEmMd != kEmMd_PB100
					ReportAbnormalID(0x110E, "access SCC even/odd");
					/*
						This happens on boot with 64k ROM.
					*/
#endif
#endif
				} else {
					SCC_Reset();
				}
			} else
#endif
#if (CurEmMd != kEmMd_PB100) \
	&& ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))

			if (WriteMem != (addr >= kSCCWr_Block_Base)) {
				ReportAbnormalID(0x110F, "access SCC wr/rd base wrong");
			} else
#endif
			{
#if CurEmMd != kEmMd_PB100
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
				if ((addr & 0x1FF9) != 0x00000000)
#else
				if ((addr & 0x001FFFF8) != 0x001FFFF8)
#endif
				{
					ReportAbnormalID(0x1110,
						"access SCC nonstandard address");
				}
#endif
				Data = SCC_Access(Data, WriteMem,
					(addr >> 1) & kSCC_Mask);
			}
			break;
		case kMMDV_Extn:
			if (ByteSize) {
				ReportAbnormalID(0x1111, "access Sony byte");
			} else if ((addr & 1) != 0) {
				ReportAbnormalID(0x1112, "access Sony odd");
			} else if (! WriteMem) {
				ReportAbnormalID(0x1113, "access Sony read");
			} else {
				Extn_Access(Data, (addr >> 1) & 0x0F);
			}
			break;
#if EmASC
		case kMMDV_ASC:
			if (! ByteSize) {
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
				if (WriteMem) {
					(void) ASC_Access((Data >> 8) & 0x00FF,
						WriteMem, addr & kASC_Mask);
					Data = ASC_Access((Data) & 0x00FF,
						WriteMem, (addr + 1) & kASC_Mask);
				} else {
					Data =
						(ASC_Access((Data >> 8) & 0x00FF,
							WriteMem, addr & kASC_Mask) << 8)
						| ASC_Access((Data) & 0x00FF,
							WriteMem, (addr + 1) & kASC_Mask);
				}
#else
				ReportAbnormalID(0x1114, "access ASC word");
#endif
			} else {
				Data = ASC_Access(Data, WriteMem, addr & kASC_Mask);
			}
			break;
#endif
		case kMMDV_SCSI:
			if (! ByteSize) {
				ReportAbnormalID(0x1115, "access SCSI word");
			} else
#if ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))
			if (WriteMem != ((addr & 1) != 0)) {
				ReportAbnormalID(0x1116, "access SCSI even/odd");
			} else
#endif
			{
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
				if ((addr & 0x1F8F) != 0x00000000) {
					ReportAbnormalID(0x1117,
						"access SCSI nonstandard address");
				}
#endif
				Data = SCSI_Access(Data, WriteMem, (addr >> 4) & 0x07);
			}

			break;
		case kMMDV_IWM:
#if (CurEmMd >= kEmMd_SE) \
	&& ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))

			if ((addr & 0x00100000) == 0) {
				ReportAbnormalID(0x1118,
					"access IWM unassigned address");
			} else
#endif
			if (! ByteSize) {
#if ExtraAbnormalReports
				ReportAbnormalID(0x1119, "access IWM word");
				/*
					This happens when quitting 'Glider 3.1.2'.
					perhaps a bad handle is being disposed of.
				*/
#endif
			} else
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
			if ((addr & 1) != 0) {
				ReportAbnormalID(0x111A, "access IWM odd");
			} else
#else
			if ((addr & 1) == 0) {
				ReportAbnormalID(0x111B, "access IWM even");
			} else
#endif
			{
#if (CurEmMd != kEmMd_PB100) \
	&& ! ((CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx))

				if ((addr & 0x001FE1FF) != 0x001FE1FF) {
					ReportAbnormalID(0x111C,
						"access IWM nonstandard address");
				}
#endif
				Data = IWM_Access(Data, WriteMem,
					(addr >> 9) & kIWM_Mask);
			}

			break;
	}

	return Data;
}

 bool MemAccessNtfy(ATTep pT)
{
	bool v = false;

	switch (pT->Ntfy) {
#if CurEmMd >= kEmMd_SE
		case kMAN_OverlayOff:
			pT->Access = kATTA_readreadymask;

			MemOverlay = 0;
			SetUpMemBanks();

			v = true;

			break;
#endif
	}

	return v;
}

void MemOverlay_ChangeNtfy(void)
{
#if CurEmMd <= kEmMd_Plus
	SetUpMemBanks();
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	SetUpMemBanks();
#endif
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
void Addr32_ChangeNtfy(void)
{
	SetUpMemBanks();
}
#endif

static ATTep get_address_realblock1(bool WriteMem, CPTR addr)
{
	ATTep p;

Label_Retry:
	p = FindATTel(addr);
	if (0 != (p->Access &
		(WriteMem ? kATTA_writereadymask : kATTA_readreadymask)))
	{
		/* ok */
	} else {
		if (0 != (p->Access & kATTA_ntfymask)) {
			if (MemAccessNtfy(p)) {
				goto Label_Retry;
			}
		}
		p = nullpr; /* fail */
	}

	return p;
}

 uint8_t * get_real_address0(uint32_t L, bool WritableMem, CPTR addr,
	uint32_t *actL)
{
	uint32_t bankleft;
	uint8_t * p;
	ATTep q;

	q = get_address_realblock1(WritableMem, addr);
	if (nullpr == q) {
		*actL = 0;
		p = nullpr;
	} else {
		uint32_t m2 = q->usemask & ~ q->cmpmask;
		uint32_t m3 = m2 & ~ (m2 + 1);
		p = q->usebase + (addr & q->usemask);
		bankleft = (m3 + 1) - (addr & m3);
		if (bankleft >= L) {
			/* this block is big enough (by far the most common case) */
			*actL = L;
		} else {
			*actL = bankleft;
		}
	}

	return p;
}

 bool InterruptButton = false;

void SetInterruptButton(bool v)
{
	if (InterruptButton != v) {
		InterruptButton = v;
		VIAorSCCinterruptChngNtfy();
	}
}

static uint8_t CurIPL = 0;

void VIAorSCCinterruptChngNtfy(void)
{
#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
	uint8_t NewIPL;

	if (InterruptButton) {
		NewIPL = 7;
	} else if (SCCInterruptRequest) {
		NewIPL = 4;
	} else if (VIA2_InterruptRequest) {
		NewIPL = 2;
	} else if (VIA1_InterruptRequest) {
		NewIPL = 1;
	} else {
		NewIPL = 0;
	}
#else
	uint8_t VIAandNotSCC = VIA1_InterruptRequest
		& ~ SCCInterruptRequest;
	uint8_t NewIPL = VIAandNotSCC
		| (SCCInterruptRequest << 1)
		| (InterruptButton << 2);
#endif
	if (NewIPL != CurIPL) {
		CurIPL = NewIPL;
		m68k_IPLchangeNtfy();
	}
}

 bool AddrSpac_Init(void)
{
	int i;

	for (i = 0; i < kNumWires; i++) {
		Wires[i] = 1;
	}

	MINEM68K_Init(
		&CurIPL);
	return true;
}

void Memory_Reset(void)
{
	MemOverlay = 1;
	SetUpMemBanks();
}

#if (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
extern void PowerOff_ChangeNtfy(void);
void PowerOff_ChangeNtfy(void)
{
	if (! VIA2_iB2) {
		ForceMacOff = true;
	}
}
#endif

/* user event queue utilities */

#if HaveMasterEvtQLock
 uint16_t MasterEvtQLock = 0;
	/*
		Takes a few ticks to process button event because
		of debounce code of Mac. So have this mechanism
		to prevent processing further events meanwhile.
	*/
#endif

 bool FindKeyEvent(int *VirtualKey, bool *KeyDown)
{
	EvtQEl *p;

	if (
#if HaveMasterEvtQLock
		(0 == MasterEvtQLock) &&
#endif
		(nullpr != (p = EvtQOutP())))
	{
		if (EvtQElKindKey == p->kind) {
			*VirtualKey = p->u.press.key;
			*KeyDown = p->u.press.down;
			EvtQOutDone();
			return true;
		}
	}

	return false;
}

/* task management */

#ifdef _VIA_Debug
#include <stdio.h>
#endif

 uimr ICTactive;
 iCountt ICTwhen[kNumICTs];

void ICT_Zap(void)
{
	ICTactive = 0;
}

static void InsertICT(int taskid, iCountt when)
{
	ICTwhen[taskid] = when;
	ICTactive |= (1 << taskid);
}

 iCountt NextiCount = 0;

 iCountt GetCuriCount(void)
{
	return NextiCount - GetCyclesRemaining();
}

void ICT_add(int taskid, uint32_t n)
{
	/* n must be > 0 */
	int32_t x = GetCyclesRemaining();
	uint32_t when = NextiCount - x + n;

#ifdef _VIA_Debug
	fprintf(stderr, "ICT_add: %d, %d, %d\n", when, taskid, n);
#endif
	InsertICT(taskid, when);

	if (x > (int32_t)n) {
		SetCyclesRemaining(n);
		NextiCount = when;
	}
}
