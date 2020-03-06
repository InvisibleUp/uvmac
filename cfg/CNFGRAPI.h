/*
	Configuration options used by platform specific code.

	Hopefully, one day, we can remove this file entirely.
*/

#define RomFileName "vMac.ROM"
#define kCheckSumRom_Size 0x00020000
#define kRomCheckSum1 0x4D1EEEE1
#define kRomCheckSum2 0x4D1EEAE1
#define kRomCheckSum3 0x4D1F8172
#define RomStartCheckSum 1
#define EnableDragDrop 1
#define SaveDialogEnable 1
#define EnableAltKeysMode 0
#define MKC_formac_Control MKC_CM
#define MKC_formac_Command MKC_Command
#define MKC_formac_Option MKC_Option
#define MKC_formac_Shift MKC_Shift
#define MKC_formac_CapsLock MKC_CapsLock
#define MKC_formac_Escape MKC_Escape
#define MKC_formac_BackSlash MKC_BackSlash
#define MKC_formac_Slash MKC_Slash
#define MKC_formac_Grave MKC_Grave
#define MKC_formac_Enter MKC_Enter
#define MKC_formac_PageUp MKC_PageUp
#define MKC_formac_PageDown MKC_PageDown
#define MKC_formac_Home MKC_Home
#define MKC_formac_End MKC_End
#define MKC_formac_Help MKC_Help
#define MKC_formac_ForwardDel MKC_ForwardDel
#define MKC_formac_F1 MKC_Option
#define MKC_formac_F2 MKC_Command
#define MKC_formac_F3 MKC_F3
#define MKC_formac_F4 MKC_F4
#define MKC_formac_F5 MKC_F5
#define MKC_formac_RControl MKC_CM
#define MKC_formac_RCommand MKC_Command
#define MKC_formac_ROption MKC_Option
#define MKC_formac_RShift MKC_Shift
#define MKC_UnMappedKey  MKC_Control
#define VarFullScreen 1
#define WantInitFullScreen 0
#define MayFullScreen 1
#define MayNotFullScreen 1
#define WantInitMagnify 0
#define EnableMagnify 1
#define WindowScale 2
#define WantInitRunInBackground 1
#define WantInitNotAutoSlow 0
#define WantInitSpeedValue 0
#define WantEnblCtrlInt 1
#define WantEnblCtrlRst 1
#define WantEnblCtrlKtg 1
#define NeedRequestInsertDisk 1
#define NeedDoMoreCommandsMsg 1
#define NeedDoAboutMsg 1
#define UseControlKeys 1

/* version and other info to display to user */

#define NeedIntlChars 0
#define ItnlKyBdFix 1
#define kStrAppName "Mini vMac (Viz)"
#define kAppVariationStr "minivmac-viz-36.04-wx64"
#define kStrCopyrightYear "2020"
#define kMaintainerName "InvisibleUp"
#define kStrHomePage "https://github.com/invisibleup/minivmac"

#define kBldOpts "-br 36 -t wx64"
