# Makefile
# Currently only builds Mac Plus emulator for Windows x64

CC := gcc
CCFLAGS := -O2 -Wall -Wmissing-prototypes -Wno-uninitialized -Wundef -Wstrict-prototypes -Icfg/ -Isrc/

.PHONY: linux windows clean

SrcFiles := \
	src/PROGMAIN.c \
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

windows :
	mkdir -p "bld/"
	windres -i "src/UI/WIN32/main.rc" --input-format=rc -o "bld/main.res" -O coff  --include-dir "src/"
	$(CC) -o "microvmac.exe" $(SrcFiles) "src/UI/WIN32/OSGLUWIN.c" \
	"bld/main.res" $(CCFLAGS) -mwindows -lwinmm -lole32 -luuid 

linux : 
	mkdir -p "bld/"
	$(CC) -o "microvmac.exe" $(SrcFiles) "src/UI/UNIX/OSGLUXWN.c" $(CCFLAGS) -lX11 -ldl

clean :
	rm -r "bld/"
	rm "microvmac.exe"
