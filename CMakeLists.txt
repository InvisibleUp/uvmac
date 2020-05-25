cmake_minimum_required(VERSION 3.1)

if(${CMAKE_VERSION} VERSION_LESS 3.15)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
    cmake_policy(VERSION 3.15)
endif()

project(
    microvmac
    VERSION 0.37.0
    DESCRIPTION "68k Macintosh emulator"
    LANGUAGES C
)

# For now, only target WIN32; just to make sure everything still works
#set(TARGET_PLATFORM AUTO CACHE STRING "Set platform to compile for")
#set_property(CACHE TARGET_PLATFORM PROPERTY STRINGS WINDOWS SDL2)

# Git submodule libraries
add_library(incbin INTERFACE)
target_include_directories(incbin INTERFACE src/incbin/)

# Hardware libraries
# Some are temporarily disabled while I get CMake up and running

#add_library(
#    hw_adb OBJECT
#    src/HW/ADB/ADBEMDEV.c
#)
#target_include_directories(hw_adb PRIVATE src/ cfg/)
#target_include_directories(hw_adb PRIVATE src/HW/ADB/)
#target_compile_definitions(hw_adb PUBLIC EmADB=1)

add_library(
    hw_disk OBJECT
    src/HW/DISK/IWMEMDEV.c
    src/HW/DISK/SONYEMDV.c
)
target_include_directories(hw_disk PRIVATE src/ cfg/)
target_include_directories(hw_disk PRIVATE src/HW/DISK/)

add_library(
    hw_kbrd OBJECT
    src/HW/KBRD/KBRDEMDV.c
)
target_include_directories(hw_kbrd PRIVATE src/ cfg/)
target_include_directories(hw_kbrd PRIVATE src/HW/KBRD/)
target_compile_definitions(hw_kbrd PUBLIC EmClassicKbrd=1)

add_library(
    hw_m68k OBJECT
#    src/HW/M68K/DISAM68K.c
    src/HW/M68K/M68KITAB.c
    src/HW/M68K/MINEM68K.c
)
target_include_directories(hw_m68k PRIVATE src/ cfg/)
target_include_directories(hw_m68k PUBLIC src/HW/M68K/)

add_library(
    hw_mouse OBJECT
    src/HW/MOUSE/MOUSEMDV.c
)
target_include_directories(hw_mouse PRIVATE src/ cfg/)
target_include_directories(hw_mouse PRIVATE src/HW/MOUSE/)

#add_library(
#    hw_powerman OBJECT
#    src/HW/POWERMAN/PMUEMDEV.c
#)
#target_include_directories(hw_powerman PRIVATE src/ cfg/)
#target_include_directories(hw_powerman PRIVATE src/HW/POWERMAN/)
#target_compile_definitions(hw_powerman PUBLIC EmPMU=1)

add_library(
    hw_rtc OBJECT
    src/HW/RTC/RTCEMDEV.c
)
target_include_directories(hw_rtc PRIVATE src/ cfg/)
target_include_directories(hw_rtc PRIVATE src/HW/RTC/)
target_compile_definitions(hw_rtc PUBLIC EmRtc=1)

add_library(
    hw_scc OBJECT
    src/HW/SCC/SCCEMDEV.c
)
target_include_directories(hw_scc PRIVATE src/ cfg/)
target_include_directories(hw_scc PRIVATE src/HW/SCC/)

add_library(
    hw_screen OBJECT
    src/HW/SCREEN/SCRNEMDV.c
)
target_include_directories(hw_screen PRIVATE src/ cfg/)
target_include_directories(hw_screen PRIVATE src/HW/SCREEN/)

add_library(
    hw_scsi OBJECT
    src/HW/SCSI/SCSIEMDV.c
)
target_include_directories(hw_scsi PRIVATE src/ cfg/)
target_include_directories(hw_scsi PRIVATE src/HW/SCSI/)

add_library(
    hw_sound OBJECT
#    src/HW/SOUND/ASCEMDEV.c
    src/HW/SOUND/SNDEMDEV.c
)
target_include_directories(hw_sound PRIVATE src/ cfg/)
target_include_directories(hw_sound PRIVATE src/HW/SOUND/)

add_library(
    hw_via1 OBJECT
    src/HW/VIA/VIAEMDEV.c
)
target_include_directories(hw_via1 PRIVATE src/ cfg/)
target_include_directories(hw_via1 PRIVATE src/HW/VIA/)

#add_library(
#    hw_via2 OBJECT
#    src/HW/VIA/VIA2EMDV.c
#)
#target_include_directories(hw_via2 PRIVATE src/ cfg/)
#target_include_directories(hw_via2 PRIVATE src/HW/VIA/)
#target_compile_definitions(hw_via2 PUBLIC EmVIA2=1)

#add_library(
#    hw_vidcard OBJECT
#    src/HW/VIDCARD/VIDEMDEV.c
#)
#target_include_directories(hw_vidcard PRIVATE src/ cfg/)
#target_include_directories(hw_vidcard PRIVATE src/HW/VIA/)
#target_compile_definitions(hw_vidcard PUBLIC EmVidCard=1)
#target_compile_definitions(hw_vidcard PUBLIC IncludeVidMem=1)

# Macintosh definitions
add_library(tgt_macbase INTERFACE) # things every Mac has
target_link_libraries(
    tgt_macbase INTERFACE
    hw_disk
    hw_m68k
    hw_rtc
    hw_sound
    hw_via1
)

add_library(tgt_macplus INTERFACE) # Macintosh Plus
target_link_libraries(
    tgt_macplus INTERFACE
    tgt_macbase
    hw_screen
    hw_mouse
    hw_kbrd
)
target_compile_definitions(tgt_macplus INTERFACE CurEmMd=kEmMd_Plus)

#add_library(tgt_macii INTERFACE) # Macintosh II
#target_link_libraries(tgt_macii INTERFACE tgt_macbase hw_vidcard hw_adb)
#target_compile_definitions(tgt_macii INTERFACE CurEmMd=kEmMd_Plus)

# User interface definitions

add_library(
    ui_win32 OBJECT
    src/UI/COMOSGLU.c
    src/UI/CONTROLM.c
    src/UI/WIN32/DBGLOG.c
    src/UI/WIN32/INTLKBRD.c
    src/UI/WIN32/KEYBOARD.c
    src/UI/WIN32/OSGLUWIN.c
    src/UI/WIN32/SOUND.c
    src/UI/WIN32/TIMEDATE.c
    src/UI/WIN32/main.rc
)
target_include_directories(ui_win32 PRIVATE cfg/ src/)
target_include_directories(ui_win32 PRIVATE src/UI/ src/UI/WIN32/)
configure_file(src/UI/WIN32/ICONAPPW.ico ICONAPPW.ico COPYONLY)
configure_file(src/UI/WIN32/ICONDSKW.ico ICONDSKW.ico COPYONLY)
configure_file(src/UI/WIN32/ICONROMW.ico ICONROMW.ico COPYONLY)
target_link_libraries(ui_win32 PUBLIC winmm ole32 uuid)

#add_library(
#    ui_sdl2 OBJECT
#    src/UI/COMOSGLU.c
#    src/UI/CONTROLM.c
#    src/UI/SDL2/OSGLUSD2.c
#)
#target_include_directories(ui_sdl2 PUBLIC src/UI/ src/UI/SDL2/)

## Final compiled program definition

add_executable(
    microvmac WIN32
    src/PROGMAIN.c
    src/GLOBGLUE.c
    src/PATCHES/ROMEMDEV.c
    src/UTIL/DATE2SEC.c
)
target_include_directories(microvmac PRIVATE cfg/ src/)
set_target_properties(microvmac PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED True)
#target_link_libraries(microvmac INTERFACE tgt_macplus) # temporary
target_link_libraries(microvmac PRIVATE incbin ui_win32)
configure_file(src/PATCHES/SONY.bin SONY.bin COPYONLY)

# temporary, because "INTERFACE" isn't working how I expect it to
target_link_libraries(
    microvmac PRIVATE
    hw_disk
    hw_m68k
    hw_rtc
    hw_sound
    hw_via1
    hw_screen
    hw_mouse
    hw_kbrd
    hw_scc
    hw_scsi
)
target_compile_definitions(microvmac PUBLIC CurEmMd=kEmMd_Plus)
