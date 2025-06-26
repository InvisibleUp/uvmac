cmake_minimum_required(VERSION 3.23)
cmake_policy(VERSION 3.23)

project(
    microvmac
    VERSION 0.37.0
    DESCRIPTION "68k Macintosh emulator"
    LANGUAGES C
)

find_package(sdl2 CONFIG REQUIRED)

# Main executable
add_executable(
    microvmac WIN32
    src/PROGMAIN.c
    src/GLOBGLUE.c
    src/PATCHES/ROMEMDEV.c
    src/UTIL/DATE2SEC.c
    src/CFGMAN.c
    src/LANG/INTLCHAR.c
)
set_target_properties(microvmac PROPERTIES C_STANDARD 17 C_STANDARD_REQUIRED True)
target_compile_definitions(microvmac PUBLIC CurEmMd=kEmMd_Plus)
configure_file(cfg/CNFGRAPI.in cfg/CNFGRAPI.h)
include_directories(src/ cfg/ lib/ ${CMAKE_CURRENT_BINARY_DIR}/cfg/)

# Git submodule libraries
add_library(tomlc99 OBJECT lib/tomlc99/toml.c)
target_include_directories(tomlc99 PRIVATE lib/tomlc99)
target_link_libraries(microvmac PRIVATE tomlc99)

# Hardware libraries
# Some are temporarily disabled while I get CMake up and running

add_library(
    hw_adb OBJECT
    src/HW/ADB/ADBEMDEV.c
)
target_compile_definitions(hw_adb PUBLIC EmADB=1)

add_library(
    hw_disk OBJECT
    src/HW/DISK/IWMEMDEV.c
    src/HW/DISK/SONYEMDV.c
    src/PATCHES/SONYDRV.c
)
target_include_directories(hw_disk PRIVATE src/HW/DISK/)

add_library(
    hw_kbrd OBJECT
    src/HW/KBRD/KBRDEMDV.c
)
target_include_directories(hw_kbrd PRIVATE src/HW/KBRD/)
target_compile_definitions(hw_kbrd PUBLIC EmClassicKbrd=1)

add_library(
    hw_m68k OBJECT
    src/HW/M68K/M68KITAB.c
    src/HW/M68K/MINEM68K.c
)
target_include_directories(hw_m68k PUBLIC src/HW/M68K/)

add_library(
    hw_mouse OBJECT
    src/HW/MOUSE/MOUSEMDV.c
)
target_include_directories(hw_mouse PRIVATE src/HW/MOUSE/)

add_library(
    hw_powerman OBJECT
    src/HW/POWERMAN/PMUEMDEV.c
)
target_include_directories(hw_powerman PRIVATE src/HW/POWERMAN/)
target_compile_definitions(hw_powerman PUBLIC EmPMU=1)

target_include_directories(microvmac PRIVATE src/HW/RAM/RAMADDR.h) # no associated .c file

add_library(
    hw_rom OBJECT
    src/HW/ROM/ROMEMDEV.c
)
target_include_directories(hw_powerman PRIVATE src/HW/ROM/)

add_library(
    hw_rtc OBJECT
    src/HW/RTC/RTCEMDEV.c
)
target_include_directories(hw_rtc PRIVATE src/HW/RTC/)
target_compile_definitions(hw_rtc PUBLIC EmRtc=1)

add_library(
    hw_scc OBJECT
    src/HW/SCC/SCCEMDEV.c
)
target_include_directories(hw_scc PRIVATE src/HW/SCC/)

add_library(
    hw_screen OBJECT
    src/HW/SCREEN/SCRNEMDV.c
    src/PATCHES/SCRNHACK.c
)
target_include_directories(hw_screen PRIVATE src/HW/SCREEN/)

add_library(
    hw_scsi OBJECT
    src/HW/SCSI/SCSIEMDV.c
)
target_include_directories(hw_scsi PRIVATE src/HW/SCSI/)

add_library(
    hw_sound OBJECT
    src/HW/SOUND/SNDEMDEV.c
)
target_include_directories(hw_sound PRIVATE src/HW/SOUND/)

add_library(
    hw_via1 OBJECT
    src/HW/VIA/VIAEMDEV.c
)
target_include_directories(hw_via1 PRIVATE src/HW/VIA/)

add_library(
    hw_via2 OBJECT
    src/HW/VIA/VIA2EMDV.c
)
target_include_directories(hw_via2 PRIVATE src/HW/VIA/)
target_compile_definitions(hw_via2 PUBLIC EmVIA2=1)

add_library(
    hw_vidcard OBJECT
    src/HW/VIDCARD/VIDEMDEV.c
)
target_include_directories(hw_vidcard PRIVATE src/HW/VIA/)
target_compile_definitions(hw_vidcard PUBLIC EmVidCard=1)
target_compile_definitions(hw_vidcard PUBLIC IncludeVidMem=1)

# User interface definitions

add_library(
    ui_sdl2 OBJECT
    src/UI/COMOSGLU.c
	src/UI/CONTROLM.c
	src/UI/SDL2/OSGLUSD2.c
	#'src/UI/SDL2/CLIPBRD.c
	src/UI/SDL2/DBGLOG.c
	src/UI/SDL2/DRIVES.c
	src/UI/SDL2/INTL.c
	src/UI/SDL2/KEYBOARD.c
	src/UI/SDL2/MOUSE.c
	src/UI/SDL2/ROM.c
	src/UI/SDL2/SOUND.c
	src/UI/SDL2/TIMEDATE.c
	src/UI/SDL2/VIDEO.c
)
target_include_directories(ui_sdl2 PRIVATE src/ cfg/ ${CMAKE_CURRENT_BINARY_DIR}/cfg/ )
target_include_directories(ui_sdl2 PRIVATE src/UI/ src/UI/SDL2/)
target_link_libraries(ui_sdl2 SDL2::SDL2 SDL2::SDL2main)

# Win32 resource file

if (WIN32)
    target_sources(
        microvmac PRIVATE
        rsrc/WIN32/main.rc
    )
    target_include_directories(microvmac PRIVATE rsrc/WIN32)
    target_link_libraries(microvmac PRIVATE winmm ole32 uuid)
    if (MSVC)
        # if not specified, Visual Studio will attempt to add its own manifest,
        # resulting in a compilation error
        target_link_options(microvmac PRIVATE "/manifest:NO")
    endif()
endif()

# Macintosh definitions
set(
    tgt_macbase
    hw_disk
    hw_m68k
    hw_rom
    hw_rtc
    hw_sound
    hw_scc
    hw_via1
    ui_sdl2
)

set(
    tgt_macplus
    ${tgt_macbase}
    hw_screen
    hw_mouse
    hw_kbrd
    hw_scsi
)

set(
    tgt_macii
    ${tgt_macbase}
    hw_vidcard
    hw_adb
)

# TODO: allow selecting emulated machine and options
target_link_libraries(microvmac PRIVATE ${tgt_macplus}) # temporary
target_compile_definitions(microvmac INTERFACE CurEmMd=kEmMd_Plus)