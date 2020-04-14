/*
	Configuration options used by platform specific code.

	Hopefully, one day, we can remove this file entirely.
*/

// TODO: replace below with struct of variable options
#define RomFileName "vMac.ROM"
#define kCheckSumRom_Size 0x00020000
#define kRomCheckSum1 0x4D1EEEE1
#define kRomCheckSum2 0x4D1EEAE1
#define kRomCheckSum3 0x4D1F8172

// Keybindings
// TODO: Make these variable
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

// Options (please remove / make variable as many as possible!)
#define RomStartCheckSum        1  // Let emu verify ROM chksum on startup
#define EnableDragDrop          1  // Enable drag+drop of disk images
#define SaveDialogEnable        1  // Allow user to choose loc. for new disks
#define EnableAltKeysMode       0  // Vim-like keybindings. TODO: remove

#define WantInitFullScreen      0  // Start with full-screen mode on
#define MayFullScreen           1  // Full screen is an option
#define MayNotFullScreen        1  // Windowed is an option

#define WantInitMagnify         0  // Start magnified (boolean)
#define WindowScale             2  // Magnification power (TODO: make a var!!!)

#define WantInitRunInBackground 1  // Start running in background (boolean)
#define WantInitNotAutoSlow     0  // Deprecated/ TODO: remove
#define WantInitSpeedValue      0  // Initial clock multiplier (0 = 1x)
#define WantEnblCtrlInt         1  // Enable interrupt key (from prog. switch)
#define WantEnblCtrlRst         1  // Enable reset key (from prog. switch)
#define WantEnblCtrlKtg         1  // Emulated control key toggle via Ctrl+K

#define NeedRequestInsertDisk   1  // Disk open dialog on Ctrl+O
#define NeedDoMoreCommandsMsg   1  // Special > More Commands tutorial message
#define NeedDoAboutMsg          1  // About mini vMac message
#define UseControlKeys          1  // Enable Control Mode (options mode)
#define NeedIntlChars           0  // Include int'l chars for Control Mode
#define ItnlKyBdFix             1  // force keyboard to match Mac layout

/* version and other info to display to user */
#define kStrAppName "micro vMac"
#define kAppVariationStr "uvmac-0.37.0-wx64"
#define kStrCopyrightYear "2020"
#define kMaintainerName "InvisibleUp"
#define kStrHomePage "https://github.com/invisibleup/minivmac"

#define kBldOpts "obsolete"
