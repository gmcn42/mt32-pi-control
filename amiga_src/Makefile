PROGRAM_NAME := mt32-pi-ctl

PREFIX := /opt/amiga/bin

CXX = $(PREFIX)/m68k-amigaos-g++
CC =  $(PREFIX)/m68k-amigaos-gcc
AR =  $(PREFIX)/m68k-amigaos-ar
AS =  $(PREFIX)/m68k-amigaos-as

VPATH = .:../common_src
INCLUDE = -I../common_src

CFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -mregparm -Wall -Werror -Wno-pointer-sign -std=gnu99 

CXXFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -mcrt=nix13 -msmall-code -fbaserel -mregparm -Wall -Werror

OBJ = mt32-pi.o midi_dev.o getopt.o version.o

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o : %.C
	$(CC) -c -o $@ $< $(CFLAGS)


$(PROGRAM_NAME) : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean :
	rm -f $(OBJ) $(PROGRAM_NAME)
