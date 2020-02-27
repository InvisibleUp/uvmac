# Makefile
# Currently only builds Mac Plus emulator for Windows x64

mk_COptionsCommon = -Wall -Wmissing-prototypes -Wno-uninitialized -Wundef -Wstrict-prototypes -Icfg/ -Isrc/
mk_COptionsOSGLU = $(mk_COptionsCommon) -O2
mk_COptions = $(mk_COptionsCommon) -O2

.PHONY: build clean

build : minivmac.exe

SrcFiles = \
	src/PROGMAIN.c \
	src/UI/WIN32/OSGLUWIN.c \
	src/GLOBGLUE.c \
	src/HW/M68K/M68KITAB.c \
	src/HW/M68K/MINEM68K.c \
	src/HW/VIA/VIAEMDEV.c \
	src/HW/DISK/IWMEMDEV.c \
	src/HW/SCC/SCCEMDEV.c \
	src/HW/RTC/RTCEMDEV.c \
	src/PATCHES/ROMEMDEV.c \
	src/HW/SCSI/SCSIEMDV.c \
	src/HW/DISK/SONYEMDV.c \
	src/HW/SCREEN/SCRNEMDV.c \
	src/HW/MOUSE/MOUSEMDV.c \
	src/HW/KBRD/KBRDEMDV.c \
	src/HW/SOUND/SNDEMDEV.c \
	src/UTIL/DATE2SEC.c \

minivmac.exe :
	mkdir -p "bld/"
	windres -i "src/UI/WIN32/main.rc" --input-format=rc -o "bld/main.res" -O coff  --include-dir "src/"
	gcc -o "minivmac.exe" $(SrcFiles) "bld/main.res" $(mk_COptions) \
		-mwindows -lwinmm -lole32 -luuid 

clean :
	rm -r "bld/"
	rm "minivmac.exe"
