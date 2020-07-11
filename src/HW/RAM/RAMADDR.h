#pragma once

/* map of address space */

#define kRAM_Base 0x00000000 /* when overlay off */
#if (CurEmMd == kEmMd_PB100)
#define kRAM_ln2Spc 23
#elif (CurEmMd == kEmMd_II) || (CurEmMd == kEmMd_IIx)
#define kRAM_ln2Spc 23
#else
#define kRAM_ln2Spc 22
#endif

#if IncludeVidMem
#if CurEmMd == kEmMd_PB100
#define kVidMem_Base 0x00FA0000
#define kVidMem_ln2Spc 16
#else
#define kVidMem_Base 0x00540000
#define kVidMem_ln2Spc 18
#endif
#endif

#if CurEmMd == kEmMd_PB100
#define kSCSI_Block_Base 0x00F90000
#define kSCSI_ln2Spc 16
#else
#define kSCSI_Block_Base 0x00580000
#define kSCSI_ln2Spc 19
#endif

#define kRAM_Overlay_Base 0x00600000 /* when overlay on */
#define kRAM_Overlay_Top  0x00800000

#if CurEmMd == kEmMd_PB100
#define kSCCRd_Block_Base 0x00FD0000
#define kSCC_ln2Spc 16
#else
#define kSCCRd_Block_Base 0x00800000
#define kSCC_ln2Spc 22
#endif

#if CurEmMd != kEmMd_PB100
#define kSCCWr_Block_Base 0x00A00000
#define kSCCWr_Block_Top  0x00C00000
#endif

#if CurEmMd == kEmMd_PB100
#define kIWM_Block_Base 0x00F60000
#define kIWM_ln2Spc 16
#else
#define kIWM_Block_Base 0x00C00000
#define kIWM_ln2Spc 21
#endif

#if CurEmMd == kEmMd_PB100
#define kVIA1_Block_Base 0x00F70000
#define kVIA1_ln2Spc 16
#else
#define kVIA1_Block_Base 0x00E80000
#define kVIA1_ln2Spc 19
#endif

#if CurEmMd == kEmMd_PB100
#define kASC_Block_Base 0x00FB0000
#define kASC_ln2Spc 16
#endif
#define kASC_Mask 0x00000FFF


/* implementation of read/write for everything but RAM and ROM */

#define kSCC_Mask 0x03

#define kVIA1_Mask 0x00000F
#if EmVIA2
#define kVIA2_Mask 0x00000F
#endif

#define kIWM_Mask 0x00000F /* Allocated Memory Bandwidth for IWM */

#if CurEmMd <= kEmMd_512Ke
#define ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_Plus
#if kROM_Size > 0x00020000
#define ROM_CmpZeroMask 0 /* For hacks like Mac ROM-inator */
#else
#define ROM_CmpZeroMask 0x00020000
#endif
#elif CurEmMd <= kEmMd_PB100
#define ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_IIx
#define ROM_CmpZeroMask 0
#else
#error "ROM_CmpZeroMask not defined"
#endif

#define kROM_cmpmask (0x00F00000 | ROM_CmpZeroMask)

#if CurEmMd <= kEmMd_512Ke
#define Overlay_ROM_CmpZeroMask 0x00100000
#elif CurEmMd <= kEmMd_Plus
#define Overlay_ROM_CmpZeroMask 0x00020000
#elif CurEmMd <= kEmMd_Classic
#define Overlay_ROM_CmpZeroMask 0x00300000
#elif CurEmMd <= kEmMd_PB100
#define Overlay_ROM_CmpZeroMask 0
#elif CurEmMd <= kEmMd_IIx
#define Overlay_ROM_CmpZeroMask 0
#else
#error "Overlay_ROM_CmpZeroMask not defined"
#endif
