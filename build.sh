#! /bin/bash

my_project_d=./bin/

# ----- make output directory -----

my_obj_d="${my_project_d}bld/"
if test ! -d "${my_obj_d}" ; then
	mkdir "${my_obj_d}"
fi


# ----- Dummy -----

DestFile="${my_obj_d}dummy.txt"
printf "" > "${DestFile}"

printf "%s\n" 'This file is here because some archive extraction' >> "${DestFile}"
printf "%s\n" 'software will not create an empty directory.' >> "${DestFile}"


# ----- make configuration folder -----

my_config_d="${my_project_d}cfg/"
if test ! -d "${my_config_d}" ; then
	mkdir "${my_config_d}"
fi


# ----- C Configuration file -----

DestFile="${my_config_d}CNFGGLOB.h"
printf "" > "${DestFile}"

printf "%s\n" '/*' >> "${DestFile}"
printf "%s\n" '	Configuration options used by both platform specific' >> "${DestFile}"
printf "%s\n" '	and platform independent code.' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	This file is automatically generated by the build system,' >> "${DestFile}"
printf "%s\n" '	which tries to know what options are valid in what' >> "${DestFile}"
printf "%s\n" '	combinations. Avoid changing this file manually unless' >> "${DestFile}"
printf "%s\n" '	you know what you'\''re doing.' >> "${DestFile}"
printf "%s\n" '*/' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* adapt to current compiler/host processor */' >> "${DestFile}"
printf "%s\n" '#ifdef __i386__' >> "${DestFile}"
printf "%s\n" '#error "source is configured for 64 bit compiler"' >> "${DestFile}"
printf "%s\n" '#endif' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define MayInline inline __attribute__((always_inline))' >> "${DestFile}"
printf "%s\n" '#define MayNotInline __attribute__((noinline))' >> "${DestFile}"
printf "%s\n" '#define SmallGlobals 0' >> "${DestFile}"
printf "%s\n" '#define cIncludeUnused 0' >> "${DestFile}"
printf "%s\n" '#define UnusedParam(p) (void) p' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* --- integer types ---- */' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef unsigned char ui3b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealui3b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef signed char si3b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealsi3b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef unsigned short ui4b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealui4b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef short si4b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealsi4b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef unsigned int ui5b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealui5b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef int si5b;' >> "${DestFile}"
printf "%s\n" '#define HaveRealsi5b 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define HaveRealui6b 0' >> "${DestFile}"
printf "%s\n" '#define HaveRealsi6b 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* --- integer representation types ---- */' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef ui3b ui3r;' >> "${DestFile}"
printf "%s\n" '#define ui3beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef si3b si3r;' >> "${DestFile}"
printf "%s\n" '#define si3beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef ui4b ui4r;' >> "${DestFile}"
printf "%s\n" '#define ui4beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef si4b si4r;' >> "${DestFile}"
printf "%s\n" '#define si4beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef ui5b ui5r;' >> "${DestFile}"
printf "%s\n" '#define ui5beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'typedef si5b si5r;' >> "${DestFile}"
printf "%s\n" '#define si5beqr 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* capabilities provided by platform specific code */' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define MySoundEnabled 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define MySoundRecenterSilence 0' >> "${DestFile}"
printf "%s\n" '#define kLn2SoundSampSz 3' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define dbglog_HAVE 0' >> "${DestFile}"
printf "%s\n" '#define WantAbnormalReports 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define NumDrives 6' >> "${DestFile}"
printf "%s\n" '#define IncludeSonyRawMode 1' >> "${DestFile}"
printf "%s\n" '#define IncludeSonyGetName 1' >> "${DestFile}"
printf "%s\n" '#define IncludeSonyNew 1' >> "${DestFile}"
printf "%s\n" '#define IncludeSonyNameNew 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define vMacScreenHeight 342' >> "${DestFile}"
printf "%s\n" '#define vMacScreenWidth 512' >> "${DestFile}"
printf "%s\n" '#define vMacScreenDepth 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kROM_Size 0x00020000' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define IncludePbufs 1' >> "${DestFile}"
printf "%s\n" '#define NumPbufs 4' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define EnableMouseMotion 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define IncludeHostTextClipExchange 1' >> "${DestFile}"
printf "%s\n" '#define EnableAutoSlow 1' >> "${DestFile}"
printf "%s\n" '#define EmLocalTalk 0' >> "${DestFile}"
printf "%s\n" '#define AutoLocation 1' >> "${DestFile}"
printf "%s\n" '#define AutoTimeZone 1' >> "${DestFile}"


# ----- C API Configuration file -----

DestFile="${my_config_d}CNFGRAPI.h"
printf "" > "${DestFile}"

printf "%s\n" '/*' >> "${DestFile}"
printf "%s\n" '	Configuration options used by platform specific code.' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	This file is automatically generated by the build system,' >> "${DestFile}"
printf "%s\n" '	which tries to know what options are valid in what' >> "${DestFile}"
printf "%s\n" '	combinations. Avoid changing this file manually unless' >> "${DestFile}"
printf "%s\n" '	you know what you'\''re doing.' >> "${DestFile}"
printf "%s\n" '*/' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#include <windows.h>' >> "${DestFile}"
printf "%s\n" '#include <time.h>' >> "${DestFile}"
printf "%s\n" '#include <shlobj.h>' >> "${DestFile}"
printf "%s\n" '#include <tchar.h>' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define RomFileName "vMac.ROM"' >> "${DestFile}"
printf "%s\n" '#define kCheckSumRom_Size 0x00020000' >> "${DestFile}"
printf "%s\n" '#define kRomCheckSum1 0x4D1EEEE1' >> "${DestFile}"
printf "%s\n" '#define kRomCheckSum2 0x4D1EEAE1' >> "${DestFile}"
printf "%s\n" '#define kRomCheckSum3 0x4D1F8172' >> "${DestFile}"
printf "%s\n" '#define RomStartCheckSum 1' >> "${DestFile}"
printf "%s\n" '#define EnableDragDrop 1' >> "${DestFile}"
printf "%s\n" '#define SaveDialogEnable 1' >> "${DestFile}"
printf "%s\n" '#define EnableAltKeysMode 0' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Control MKC_CM' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Command MKC_Command' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Option MKC_Option' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Shift MKC_Shift' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_CapsLock MKC_CapsLock' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Escape MKC_Escape' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_BackSlash MKC_BackSlash' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Slash MKC_Slash' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Grave MKC_Grave' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Enter MKC_Enter' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_PageUp MKC_PageUp' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_PageDown MKC_PageDown' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Home MKC_Home' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_End MKC_End' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_Help MKC_Help' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_ForwardDel MKC_ForwardDel' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_F1 MKC_Option' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_F2 MKC_Command' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_F3 MKC_F3' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_F4 MKC_F4' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_F5 MKC_F5' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_RControl MKC_CM' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_RCommand MKC_Command' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_ROption MKC_Option' >> "${DestFile}"
printf "%s\n" '#define MKC_formac_RShift MKC_Shift' >> "${DestFile}"
printf "%s\n" '#define MKC_UnMappedKey  MKC_Control' >> "${DestFile}"
printf "%s\n" '#define VarFullScreen 1' >> "${DestFile}"
printf "%s\n" '#define WantInitFullScreen 0' >> "${DestFile}"
printf "%s\n" '#define MayFullScreen 1' >> "${DestFile}"
printf "%s\n" '#define MayNotFullScreen 1' >> "${DestFile}"
printf "%s\n" '#define WantInitMagnify 0' >> "${DestFile}"
printf "%s\n" '#define EnableMagnify 1' >> "${DestFile}"
printf "%s\n" '#define MyWindowScale 2' >> "${DestFile}"
printf "%s\n" '#define WantInitRunInBackground 0' >> "${DestFile}"
printf "%s\n" '#define WantInitNotAutoSlow 0' >> "${DestFile}"
printf "%s\n" '#define WantInitSpeedValue 3' >> "${DestFile}"
printf "%s\n" '#define WantEnblCtrlInt 1' >> "${DestFile}"
printf "%s\n" '#define WantEnblCtrlRst 1' >> "${DestFile}"
printf "%s\n" '#define WantEnblCtrlKtg 1' >> "${DestFile}"
printf "%s\n" '#define NeedRequestInsertDisk 1' >> "${DestFile}"
printf "%s\n" '#define NeedDoMoreCommandsMsg 1' >> "${DestFile}"
printf "%s\n" '#define NeedDoAboutMsg 1' >> "${DestFile}"
printf "%s\n" '#define UseControlKeys 1' >> "${DestFile}"
printf "%s\n" '#define UseActvCode 0' >> "${DestFile}"
printf "%s\n" '#define EnableDemoMsg 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* version and other info to display to user */' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define NeedIntlChars 0' >> "${DestFile}"
printf "%s\n" '#define ItnlKyBdFix 1' >> "${DestFile}"
printf "%s\n" '#define kStrAppName "Mini vMac"' >> "${DestFile}"
printf "%s\n" '#define kAppVariationStr "minivmac-36.04-wx64"' >> "${DestFile}"
printf "%s\n" '#define kStrCopyrightYear "2018"' >> "${DestFile}"
printf "%s\n" '#define kMaintainerName "unknown"' >> "${DestFile}"
printf "%s\n" '#define kStrHomePage "(unknown)"' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kBldOpts "-br 36 -t wx64"' >> "${DestFile}"


# ----- C Platform Independent Configuration file -----

DestFile="${my_config_d}EMCONFIG.h"
printf "" > "${DestFile}"

printf "%s\n" '/*' >> "${DestFile}"
printf "%s\n" '	Configuration options used by platform independent code.' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	This file is automatically generated by the build system,' >> "${DestFile}"
printf "%s\n" '	which tries to know what options are valid in what' >> "${DestFile}"
printf "%s\n" '	combinations. Avoid changing this file manually unless' >> "${DestFile}"
printf "%s\n" '	you know what you'\''re doing.' >> "${DestFile}"
printf "%s\n" '*/' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define EmClassicKbrd 1' >> "${DestFile}"
printf "%s\n" '#define EmADB 0' >> "${DestFile}"
printf "%s\n" '#define EmRTC 1' >> "${DestFile}"
printf "%s\n" '#define EmPMU 0' >> "${DestFile}"
printf "%s\n" '#define EmVIA2 0' >> "${DestFile}"
printf "%s\n" '#define Use68020 0' >> "${DestFile}"
printf "%s\n" '#define EmFPU 0' >> "${DestFile}"
printf "%s\n" '#define EmMMU 0' >> "${DestFile}"
printf "%s\n" '#define EmASC 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define CurEmMd kEmMd_Plus' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kMyClockMult 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define WantCycByPriOp 1' >> "${DestFile}"
printf "%s\n" '#define WantCloserCyc 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kRAMa_Size 0x00200000' >> "${DestFile}"
printf "%s\n" '#define kRAMb_Size 0x00200000' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define IncludeVidMem 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define EmVidCard 0' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define MaxATTListN 16' >> "${DestFile}"
printf "%s\n" '#define IncludeExtnPbufs 1' >> "${DestFile}"
printf "%s\n" '#define IncludeExtnHostTextClipExchange 1' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define Sony_SupportDC42 1' >> "${DestFile}"
printf "%s\n" '#define Sony_SupportTags 0' >> "${DestFile}"
printf "%s\n" '#define Sony_WantChecksumsUpdated 0' >> "${DestFile}"
printf "%s\n" '#define Sony_VerifyChecksums 0' >> "${DestFile}"
printf "%s\n" '#define CaretBlinkTime 0x03' >> "${DestFile}"
printf "%s\n" '#define SpeakerVol 0x07' >> "${DestFile}"
printf "%s\n" '#define DoubleClickTime 0x05' >> "${DestFile}"
printf "%s\n" '#define MenuBlink 0x03' >> "${DestFile}"
printf "%s\n" '#define AutoKeyThresh 0x06' >> "${DestFile}"
printf "%s\n" '#define AutoKeyRate 0x03' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* the Wire variables are 1/0, not true/false */' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'enum {' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA0_SoundVolb0,' >> "${DestFile}"
printf "%s\n" '#define SoundVolb0 (Wires[Wire_VIA1_iA0_SoundVolb0])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA0 (Wires[Wire_VIA1_iA0_SoundVolb0])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA1_SoundVolb1,' >> "${DestFile}"
printf "%s\n" '#define SoundVolb1 (Wires[Wire_VIA1_iA1_SoundVolb1])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA1 (Wires[Wire_VIA1_iA1_SoundVolb1])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA2_SoundVolb2,' >> "${DestFile}"
printf "%s\n" '#define SoundVolb2 (Wires[Wire_VIA1_iA2_SoundVolb2])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA2 (Wires[Wire_VIA1_iA2_SoundVolb2])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA4_MemOverlay,' >> "${DestFile}"
printf "%s\n" '#define MemOverlay (Wires[Wire_VIA1_iA4_MemOverlay])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA4 (Wires[Wire_VIA1_iA4_MemOverlay])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA4_ChangeNtfy MemOverlay_ChangeNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA6_SCRNvPage2,' >> "${DestFile}"
printf "%s\n" '#define SCRNvPage2 (Wires[Wire_VIA1_iA6_SCRNvPage2])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA6 (Wires[Wire_VIA1_iA6_SCRNvPage2])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA5_IWMvSel,' >> "${DestFile}"
printf "%s\n" '#define IWMvSel (Wires[Wire_VIA1_iA5_IWMvSel])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA5 (Wires[Wire_VIA1_iA5_IWMvSel])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA7_SCCwaitrq,' >> "${DestFile}"
printf "%s\n" '#define SCCwaitrq (Wires[Wire_VIA1_iA7_SCCwaitrq])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA7 (Wires[Wire_VIA1_iA7_SCCwaitrq])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB0_RTCdataLine,' >> "${DestFile}"
printf "%s\n" '#define RTCdataLine (Wires[Wire_VIA1_iB0_RTCdataLine])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB0 (Wires[Wire_VIA1_iB0_RTCdataLine])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB0_ChangeNtfy RTCdataLine_ChangeNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB1_RTCclock,' >> "${DestFile}"
printf "%s\n" '#define RTCclock (Wires[Wire_VIA1_iB1_RTCclock])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB1 (Wires[Wire_VIA1_iB1_RTCclock])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB1_ChangeNtfy RTCclock_ChangeNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB2_RTCunEnabled,' >> "${DestFile}"
printf "%s\n" '#define RTCunEnabled (Wires[Wire_VIA1_iB2_RTCunEnabled])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB2 (Wires[Wire_VIA1_iB2_RTCunEnabled])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB2_ChangeNtfy RTCunEnabled_ChangeNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iA3_SoundBuffer,' >> "${DestFile}"
printf "%s\n" '#define SoundBuffer (Wires[Wire_VIA1_iA3_SoundBuffer])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iA3 (Wires[Wire_VIA1_iA3_SoundBuffer])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB3_MouseBtnUp,' >> "${DestFile}"
printf "%s\n" '#define MouseBtnUp (Wires[Wire_VIA1_iB3_MouseBtnUp])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB3 (Wires[Wire_VIA1_iB3_MouseBtnUp])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB4_MouseX2,' >> "${DestFile}"
printf "%s\n" '#define MouseX2 (Wires[Wire_VIA1_iB4_MouseX2])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB4 (Wires[Wire_VIA1_iB4_MouseX2])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB5_MouseY2,' >> "${DestFile}"
printf "%s\n" '#define MouseY2 (Wires[Wire_VIA1_iB5_MouseY2])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB5 (Wires[Wire_VIA1_iB5_MouseY2])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iCB2_KybdDat,' >> "${DestFile}"
printf "%s\n" '#define VIA1_iCB2 (Wires[Wire_VIA1_iCB2_KybdDat])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iCB2_ChangeNtfy Kybd_DataLineChngNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB6_SCRNbeamInVid,' >> "${DestFile}"
printf "%s\n" '#define SCRNbeamInVid (Wires[Wire_VIA1_iB6_SCRNbeamInVid])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB6 (Wires[Wire_VIA1_iB6_SCRNbeamInVid])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_iB7_SoundDisable,' >> "${DestFile}"
printf "%s\n" '#define SoundDisable (Wires[Wire_VIA1_iB7_SoundDisable])' >> "${DestFile}"
printf "%s\n" '#define VIA1_iB7 (Wires[Wire_VIA1_iB7_SoundDisable])' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_VIA1_InterruptRequest,' >> "${DestFile}"
printf "%s\n" '#define VIA1_InterruptRequest (Wires[Wire_VIA1_InterruptRequest])' >> "${DestFile}"
printf "%s\n" '#define VIA1_interruptChngNtfy VIAorSCCinterruptChngNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	Wire_SCCInterruptRequest,' >> "${DestFile}"
printf "%s\n" '#define SCCInterruptRequest (Wires[Wire_SCCInterruptRequest])' >> "${DestFile}"
printf "%s\n" '#define SCCinterruptChngNtfy VIAorSCCinterruptChngNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '	kNumWires' >> "${DestFile}"
printf "%s\n" '};' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '/* VIA configuration */' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORA_FloatVal 0xFF' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORB_FloatVal 0xFF' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORA_CanIn 0x80' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORA_CanOut 0x7F' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORB_CanIn 0x79' >> "${DestFile}"
printf "%s\n" '#define VIA1_ORB_CanOut 0x87' >> "${DestFile}"
printf "%s\n" '#define VIA1_IER_Never0 (1 << 1)' >> "${DestFile}"
printf "%s\n" '#define VIA1_IER_Never1 ((1 << 3) | (1 << 4))' >> "${DestFile}"
printf "%s\n" '#define VIA1_CB2modesAllowed 0x01' >> "${DestFile}"
printf "%s\n" '#define VIA1_CA2modesAllowed 0x01' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define Mouse_Enabled SCC_InterruptsEnabled' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define VIA1_iCA1_PulseNtfy VIA1_iCA1_Sixtieth_PulseNtfy' >> "${DestFile}"
printf "%s\n" '#define Sixtieth_PulseNtfy VIA1_iCA1_Sixtieth_PulseNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define VIA1_iCA2_PulseNtfy VIA1_iCA2_RTC_OneSecond_PulseNtfy' >> "${DestFile}"
printf "%s\n" '#define RTC_OneSecond_PulseNtfy VIA1_iCA2_RTC_OneSecond_PulseNtfy' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define GetSoundInvertTime VIA1_GetT1InvertTime' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define KYBD_ShiftInData VIA1_ShiftOutData' >> "${DestFile}"
printf "%s\n" '#define KYBD_ShiftOutData VIA1_ShiftInData' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kExtn_Block_Base 0x00F40000' >> "${DestFile}"
printf "%s\n" '#define kExtn_ln2Spc 5' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define kROM_Base 0x00400000' >> "${DestFile}"
printf "%s\n" '#define kROM_ln2Spc 20' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '#define WantDisasm 0' >> "${DestFile}"
printf "%s\n" '#define ExtraAbnormalReports 0' >> "${DestFile}"


# ----- Language Configuration file -----

DestFile="${my_config_d}STRCONST.h"
printf "" > "${DestFile}"

printf "%s\n" '#include "STRCNENG.h"' >> "${DestFile}"


# ----- Resource Configuration file -----

DestFile="${my_config_d}main.rc"
printf "" > "${DestFile}"

printf "%s\n" '256                     ICON    DISCARDABLE     "ICONAPPW.ico"' >> "${DestFile}"


# ----- Make file -----

DestFile="${my_project_d}Makefile"
printf "" > "${DestFile}"

printf "%s\n" '# make file generated by gryphel build system' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'mk_COptionsCommon = -c -Wall -Wmissing-prototypes -Wno-uninitialized -Wundef -Wstrict-prototypes -Icfg/ -Isrc/' >> "${DestFile}"
printf "%s\n" 'mk_COptionsOSGLU = $(mk_COptionsCommon) -Os' >> "${DestFile}"
printf "%s\n" 'mk_COptions = $(mk_COptionsCommon) -Os' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" '.PHONY: TheDefaultOutput clean' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'TheDefaultOutput : minivmac.exe' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'bld/OSGLUWIN.o : src/OSGLUWIN.c src/STRCNENG.h cfg/STRCONST.h src/INTLCHAR.h src/COMOSGLU.h src/CONTROLM.h cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/OSGLUWIN.c" -o "bld/OSGLUWIN.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/GLOBGLUE.o : src/GLOBGLUE.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/GLOBGLUE.c" -o "bld/GLOBGLUE.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/M68KITAB.o : src/M68KITAB.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/M68KITAB.c" -o "bld/M68KITAB.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/MINEM68K.o : src/MINEM68K.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/MINEM68K.c" -o "bld/MINEM68K.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/VIAEMDEV.o : src/VIAEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/VIAEMDEV.c" -o "bld/VIAEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/IWMEMDEV.o : src/IWMEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/IWMEMDEV.c" -o "bld/IWMEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/SCCEMDEV.o : src/SCCEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/SCCEMDEV.c" -o "bld/SCCEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/RTCEMDEV.o : src/RTCEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/RTCEMDEV.c" -o "bld/RTCEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/ROMEMDEV.o : src/ROMEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/ROMEMDEV.c" -o "bld/ROMEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/SCSIEMDV.o : src/SCSIEMDV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/SCSIEMDV.c" -o "bld/SCSIEMDV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/SONYEMDV.o : src/SONYEMDV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/SONYEMDV.c" -o "bld/SONYEMDV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/SCRNEMDV.o : src/SCRNEMDV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/SCRNEMDV.c" -o "bld/SCRNEMDV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/MOUSEMDV.o : src/MOUSEMDV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/MOUSEMDV.c" -o "bld/MOUSEMDV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/KBRDEMDV.o : src/KBRDEMDV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/KBRDEMDV.c" -o "bld/KBRDEMDV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/SNDEMDEV.o : src/SNDEMDEV.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/SNDEMDEV.c" -o "bld/SNDEMDEV.o" $(mk_COptions)' >> "${DestFile}"
printf "%s\n" 'bld/PROGMAIN.o : src/PROGMAIN.c cfg/CNFGGLOB.h' >> "${DestFile}"
printf "%s\n" '	gcc "src/PROGMAIN.c" -o "bld/PROGMAIN.o" $(mk_COptions)' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'ObjFiles = \' >> "${DestFile}"
printf "%s\n" '	bld/MINEM68K.o \' >> "${DestFile}"
printf "%s\n" '	bld/OSGLUWIN.o \' >> "${DestFile}"
printf "%s\n" '	bld/GLOBGLUE.o \' >> "${DestFile}"
printf "%s\n" '	bld/M68KITAB.o \' >> "${DestFile}"
printf "%s\n" '	bld/VIAEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/IWMEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/SCCEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/RTCEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/ROMEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/SCSIEMDV.o \' >> "${DestFile}"
printf "%s\n" '	bld/SONYEMDV.o \' >> "${DestFile}"
printf "%s\n" '	bld/SCRNEMDV.o \' >> "${DestFile}"
printf "%s\n" '	bld/MOUSEMDV.o \' >> "${DestFile}"
printf "%s\n" '	bld/KBRDEMDV.o \' >> "${DestFile}"
printf "%s\n" '	bld/SNDEMDEV.o \' >> "${DestFile}"
printf "%s\n" '	bld/PROGMAIN.o \' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'bld/: cfg/main.r' >> "${DestFile}"
printf "%s\n" '	windres.exe -i "cfg/main.r" --input-format=rc -o "bld/" -O coff  --include-dir SRC' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'minivmac.exe : $(ObjFiles) bld/' >> "${DestFile}"
printf "%s\n" '	gcc \' >> "${DestFile}"
printf "%s\n" '		-o "minivmac.exe" \' >> "${DestFile}"
printf "%s\n" '		$(ObjFiles) "bld/" -mwindows -lwinmm -lole32 -luuid' >> "${DestFile}"
printf "\n" >> "${DestFile}"
printf "%s\n" 'clean :' >> "${DestFile}"
printf "%s\n" '	rm -f $(ObjFiles)' >> "${DestFile}"
printf "%s\n" '	rm -f "bld/"' >> "${DestFile}"
printf "%s\n" '	rm -f "minivmac.exe"' >> "${DestFile}"