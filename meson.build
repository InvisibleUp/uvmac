# Project definition
project('microvmac', 'c')

# Configuration data
conf = configuration_data({
	# verify ROM checksum on startup?
	'RomStartCheckSum': 1,
	# Enable drag/drop of disk images?
	'EnableDragDrop': 1,
	# Allow user to choose location for new disks?
	'SaveDialogEnable': 1,
	# Start in full-screen mode?
	'WantInitFullScreen': 0,
	# Full screen is an option?
	'MayFullScreen': 1,
	# Windowed is an option?
	'MayNotFullScreen': 1,
	# Start magnified?
	'WantInitMagnify': 0,
	# Magnificaton power
	'WindowScale': 2,
	# Allow background running by default?
	'WantInitRunInBackground': 1,
	# Initial clock multiplied (0 = 1x)
	'WantInitSpeedValue': 0,
	# Enable interrupt key? (on prog. switch)
	'WantEnblCtrlInt': 1,
	# Enable reset key? (on prog. switch)
	'WantEnblCtrlRst': 1,
	# Emulated control key toggle via Ctrl+K?
	'WantEnblCrtlKtg': 1,
	# Use Ctrl+num to insert "./disk#.dsk"
	'NeedRequestIthDisk': 0,
	# Disk open dialog on Ctrl+O
	'NeedRequestInsertDisk': 1,
	# Special > More Commands tutorial message
	'NeedDoMoreCommandsMsg': 1,
	# Enable about dialog
	'NeedDoAboutMsg': 1,
	# Enable Control Mode (options menu)
	'UseControlKeys': 1,
	# Include international characters for Control Mode
	'NeedIntlChars': 0,
	# Force keyboard to match Mac layout on Windows
	'ItnlKyBdFix': 1,
	# Version information
	'kStrAppName': '"micro vMac"',
	'kAppVariationStr': '"0.37.0"',
	'kStrCopyrightYear': '"2020"',
	'kMaintainerName': '"InvisibleUp"',
	'kStrHomePage': '"https://github.com/invisibleup/minivmac/"',
	'kBldOpts': '"obsolete"'
})
configure_file(
	output: 'CNFGRAPI.h',
	configuration: conf
)

# Dependencies
lSDL2 = dependency('SDL2')

# Hardware libraries
# Some are temporarily disabled while I get CMake up and running

HW_SRC = {
	'ADB':  [
		'src/HW/ADB/ADBEMDEV.c'
	],
	'DISK': [
		'src/HW/DISK/IWMEMDEV.c',
		'src/HW/DISK/SONYEMDV.c',
		'src/PATCHES/SONYDRV.c'
    ],
    'KBRD': [
		'src/HW/KBRD/KBRDEMDV.c',
	],
	'M68K': [
#		'src/HW/M68K/DISAM68K.c',
		'src/HW/M68K/M68KITAB.c',
		'src/HW/M68K/MINEM68K.c',
	],
	'MOUSE': [
		'src/HW/MOUSE/MOUSEMDV.c',
	],
	'POWERMAN': [
		'src/HW/POWERMAN/PMUEMDEV.c',
	],
	'RTC': [
		'src/HW/RTC/RTCEMDEV.c',
	],
	'SCC': [
		'src/HW/SCC/SCCEMDEV.c',
	],
	'SCREEN': [
		'src/HW/SCREEN/SCRNEMDV.c',
		'src/PATCHES/SCRNHACK.c',
	],
	'SCSI': [
		'src/HW/SCSI/SCSIEMDV.c',
	],
	'SOUND': [
#		'src/HW/SOUND/ASCEMDEV.c',
		'src/HW/SOUND/SNDEMDEV.c',
	],
	'VIA1': [
		'src/HW/VIA/VIAEMDEV.c',
	],
	'VIA2': [
		'src/HW/VIA/VIA2EMDV.c',
	],
	'VIDCARD': [
		'src/HW/VIDCARD/VIDEMDEV.c',
	],
}

MAC_SRC_COMMON = HW_SRC['DISK'] + HW_SRC['M68K'] + HW_SRC['RTC'] + HW_SRC['SOUND'] + HW_SRC['VIA1']

# Macintosh definitions
MAC_SRC = {
	'Plus': 
		MAC_SRC_COMMON +
		HW_SRC['SCREEN'] +
		HW_SRC['MOUSE'] +
		HW_SRC['KBRD'],
	'II': 
		MAC_SRC_COMMON +
		HW_SRC['VIDCARD'] +
		HW_SRC['ADB'],
}

# User interface definitions
UI_SRC = {
	'WIN32': [
		'src/UI/COMOSGLU.c',
		'src/UI/CONTROLM.c',
		'src/UI/WIN32/DBGLOG.c',
		'src/UI/WIN32/INTLKBRD.c',
		'src/UI/WIN32/KEYBOARD.c',
		'src/UI/WIN32/OSGLUWIN.c',
		'src/UI/WIN32/SOUND.c',
		'src/UI/WIN32/TIMEDATE.c',
		'rsrc/WIN32/main.rc',
	],
	'SDL2': [
		'src/UI/COMOSGLU.c',
		'src/UI/CONTROLM.c',
		'src/UI/SDL2/OSGLUSD2.c'
	]
}

EMU_SRC = [
	'src/PROGMAIN.c',
	'src/GLOBGLUE.c',
	'src/PATCHES/ROMEMDEV.c',
	'src/UTIL/DATE2SEC.c',
	'src/incbin/incbin.c',
]

EMU_INC = include_directories([
	'cfg/',
	'src/',
	'src/HW/ADB',
	'src/HW/DISK',
	'src/HW/KBRD',
	'src/HW/M68K',
	'src/HW/MOUSE',
	'src/HW/POWERMAN',
	'src/HW/RTC',
	'src/HW/SCC',
	'src/HW/SCREEN',
	'src/HW/SCSI',
	'src/HW/SOUND',
	'src/HW/VIA',
	'src/HW/VIDCARD',
	'src/LANG',
	'src/UI',
	'src/UI/SDL2',
#	'src/UI/WIN32',
	'src/UTIL',
	'src/incbin',
])

## Final compiled program definition
# Just gonna do an SDL2 Mac Plus for now
executable(
	'microvmac',
	sources: MAC_SRC['Plus'] + UI_SRC['SDL2'] + EMU_SRC,
	dependencies: [lSDL2],
	include_directories: EMU_INC,
)

