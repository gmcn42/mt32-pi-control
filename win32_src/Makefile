PROGRAM_NAME := mt32-pi-ctl.exe

CXX = i686-w64-mingw32-g++
CC =  i686-w64-mingw32-gcc
AR =  i686-w64-mingw32-ar
AS =  i686-w64-mingw32-as

VPATH = .:../common_src
INCLUDE = -I../common_src
DEFINE = -DPROGRAM_NAME='"$(PROGRAM_NAME)"' -DUSE_CUSTOM_DELAY

#CFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -mregparm -Wall -Werror -Wno-pointer-sign -std=gnu99 

CFLAGS = $(INCLUDE) -O2 -ffunction-sections -fdata-sections -static-libgcc -static-libstdc++ -Wl,--gc-sections -Wall -Werror -Wno-pointer-sign -std=gnu99 $(DEFINE)

CXXFLAGS = $(INCLUDE) -O2 -ffunction-sections -fdata-sections -static-libgcc -static-libstdc++ -Wl,--gc-sections -Wall -Werror $(DEFINE)

LIBS = -lwinmm

OBJ = mt32-pi.o midi_dev.o delay.o

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o : %.C
	$(CC) -c -o $@ $< $(CFLAGS)


$(PROGRAM_NAME) : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean :
	rm -f $(OBJ) $(PROGRAM_NAME)

.PHONY: dist
dist: $(PROGRAM_NAME) 
	cp -v $(PROGRAM_NAME) ../win32_bin/
	cp -v *.bmp ../win32_bin/

