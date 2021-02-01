PROGRAM_NAME := mt32-pi-ctl

PREFIX := /opt/amiga/bin

CXX = $(PREFIX)/m68k-amigaos-g++
CC =  $(PREFIX)/m68k-amigaos-gcc
AR =  $(PREFIX)/m68k-amigaos-ar
AS =  $(PREFIX)/m68k-amigaos-as

VPATH = .:../common_src
INCLUDE = -I../common_src
DEFINE = -DPROGRAM_NAME='"$(PROGRAM_NAME)"' -DREPLACE_GETOPT

#CFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -mregparm -Wall -Werror -Wno-pointer-sign -std=gnu99 

CFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -Wall -Werror -Wno-pointer-sign -std=gnu99 $(DEFINE)

CXXFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -Wall -Werror $(DEFINE)

LIBS = -lamiga

OBJ = mt32-pi.o midi_dev.o getopt.o version.o

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
	cp -v $(PROGRAM_NAME) ../amiga_bin/
	cp -v *.bmp ../amiga_bin/

