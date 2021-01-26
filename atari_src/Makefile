PROGRAM_NAME := MT32-PI.TTP

CXX = m68k-atari-mint-g++
CC = m68k-atari-mint-gcc
AR = m68k-atari-mint-ar
AS = m68k-atari-mint-as

VPATH = .:../common_src
INCLUDE = -I../common_src
DEFINE = -DPROGRAM_NAME='"$(PROGRAM_NAME)"'

CFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -Wall -Werror -Wno-pointer-sign -std=c99 $(DEFINE)
CXXFLAGS = $(INCLUDE) -Os -s -m68000 -fomit-frame-pointer -Wall -Werror $(DEFINE)

OBJ = mt32-pi.o midi_dev.o getopt.o

%.o : %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o : %.C
	$(CC) -c -o $@ $< $(CFLAGS)


$(PROGRAM_NAME) : $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean :
	rm -f $(OBJ) $(PROGRAM_NAME)

.PHONY: dist
dist: $(PROGRAM_NAME) 
	cp -v $(PROGRAM_NAME) ../atari_bin/
	cp -v *.bmp ../atari_bin/

