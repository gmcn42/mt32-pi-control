/*
 * mt32-pi control program for DOS PCs
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "midi_dev.h"
#include "getopt.h"
#include "delay.h"
#include "miniwift.h"

#ifndef PROGRAM_NAME
	#define PROGRAM_NAME "MT32-PI.EXE"
#endif

typedef enum {
	MODE_UNCHANGED,
	MODE_MUNT,
	MODE_FLUID
} mt32pi_mode_t;

typedef enum {
	ROM_UNCHANGED,
	ROM_OLD,
	ROM_NEW,
	ROM_CM32L
} mt32pi_romset_t;

static void str_to_sysex_disp_mt32(unsigned char *sysexbuf, const char *str);
static int str_to_sysex_disp_sc55(unsigned char *sysexbuf, const char *str);
static void bmp_to_sysex_disp_sc55(unsigned char *se_array, const char *fname, int negative);
static void strpic_to_sysex_disp_sc55(unsigned char *sysexbuf, const char *str, int negative);

static unsigned char roland_checksum(const unsigned char *buf, unsigned short len);

static void print_usage(void);

static unsigned char cmd_sysex[5] = {0xF0, 0x7D, 0x00, 0x00, 0xF7};
static unsigned char reboot_sysex[4] = {0xF0, 0x7D, 0x00, 0xF7};

static unsigned char gm_reset[6] = {0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7};
static unsigned char gs_reset[11] = {0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7};
static unsigned char mt32_reset[8] = {0xF0, 0x41, 0x10, 0x16, 0x12, 0x7F, 0x01, 0xF7};

static unsigned char custom_midi_buf[64];
static int custom_midi_len = 0;

static char sc55_bmp_fname[64] = {'\0'};
static char syx_fname[64] = {'\0'};

static unsigned char sysexbuf[74];
static char mt32_text[21] = {'\0'};
static char sc55_text[33] = {'\0'};
static char sc55_pictxt[9] = {'\0'};


int main(int argc, char *argv[]) {
	// static linkage because of C89 and struct
	// initialization for long_options[]
	static int reboot_flag = 0,
		mt32_rst_flag = 0,
		gs_rst_flag = 0,
		gm_rst_flag = 0,
		pic_negative_flag = 0,
		verbose = 0,
		soundfont = -1,
		mt32_rstereo = -1;
	static mt32pi_mode_t mode = MODE_UNCHANGED;
	static mt32pi_romset_t romset = ROM_UNCHANGED;
	#ifdef ARGV_NO_BASENAME
	argv[0] = PROGRAM_NAME;
	#endif

	int c;
	static struct option long_options[] =
		{
			{"verbose", no_argument, &verbose, 1},
			{"reboot", no_argument, &reboot_flag, 1},
			{"help", no_argument, 0, 'h'},
			{"mt32", no_argument, 0, 'm'},
			{"mt32-rstereo", required_argument, 0, 'S'},
			{"fluidsynth", no_argument, 0, 'g'},
			{"soundfont", required_argument, 0, 's'},
			{"mt32-reset", no_argument, &mt32_rst_flag, 1},
			{"gm-reset", no_argument, &gm_rst_flag, 1},
			{"gs-reset", no_argument, &gs_rst_flag, 1},
			{"mt32-txt", required_argument, 0, 't'},
			{"sc55-txt", required_argument, 0, 'T'},
			{"sc55-bmp", required_argument, 0, 'P'},
			{"sc55-btxt", required_argument, 0, 'X'},
			{"negative", required_argument, 0, 'N'},
			{"romset", required_argument, 0, 'b'},
			{"midi", required_argument, 0, 'M'},
			{"syx", required_argument, 0, 'Y'},
			{0, 0, 0, 0}
		};
	char optstr[32] = "hs:S:t:T:b:mgrvM:P:NY:X:";
	
	// The MIDI backend may add some more short options
	// Be sure that strlen(optstr) remains <32
	// Could also generate new string of arbitrary
	// but don't really wanna mess with malloc/free
	// for this bit
	mididev_add_optstr(optstr);

	opterr = 1;
	if(argc == 1) {
		print_usage();
		#ifndef NO_ERROR_ON_HELP
		return EXIT_FAILURE;
		#else
		return EXIT_SUCCESS;
		#endif
	}
	
	while(1) {
		
		c = getopt_long(argc, argv, optstr, long_options, NULL);
		
		// End of options
		if(c == -1)
			break;
			
		switch(c) {
			case 0:
				break;
			case 's':
				soundfont = (int)strtol(optarg, NULL, 10);
				break;
			case 'S':
				mt32_rstereo = (int)strtol(optarg, NULL, 10);
				break;
			case 't':
				strncpy(mt32_text, optarg, 20);
				break;
			case 'T':
				strncpy(sc55_text, optarg, 32);
				break;
			case 'b':
				if(strcmp(optarg, "old")==0) {
					romset = ROM_OLD;
				} else if(strcmp(optarg, "new")==0) {
					romset = ROM_NEW;
				} else if(strcmp(optarg, "cm32l")==0) {
					romset = ROM_CM32L;
				} else {
					fprintf(stderr, "%s is not a recognized romset.\n", optarg);
					return EXIT_FAILURE;
				}
				break;
			case 'm':
				mode = MODE_MUNT;
				break;
			case 'g':
				mode = MODE_FLUID;
				break;
			case 'r':
				reboot_flag = 1;
				break;
			case 'M':
				{
					unsigned char *mptr = custom_midi_buf;
					char *end;
					while(1) {
						errno = 0;
						*mptr = (unsigned char)strtoul(optarg, &end, 16);
						if(errno || end==optarg)
							break;
						custom_midi_len++;
						mptr++;
						if(mptr == custom_midi_buf+64 || *end == '\0')
							break;
						optarg = end;
					}
				}
				break;
			case 'P':
				if(strlen(optarg) > 63) {
					fprintf(stderr, "Sorry, the filename is too long!\n");
					return EXIT_FAILURE;
				}
				strncpy(sc55_bmp_fname, optarg, 64);
				break;
			case 'X':
				strncpy(sc55_pictxt, optarg, 8);
				break;
			case 'N':
				pic_negative_flag = 1;
				break;
			case 'Y':
				if(strlen(optarg) > 63) {
					fprintf(stderr, "Sorry, the filename is too long!\n");
					return EXIT_FAILURE;
				}
				strncpy(syx_fname, optarg, 64);
				break;
			case 'v':
				verbose = 1;
				break;
			case ':':
			case '?':
			case 'h':
				print_usage();
				#ifndef NO_ERROR_ON_HELP
				return EXIT_FAILURE;
				#else
				return EXIT_SUCCESS;
				#endif
			default:
				if(mididev_parse_arg(c, optarg)<0) {
					fprintf(stderr, "Error parsing argument \"%s\"\n", optarg);
					return EXIT_FAILURE;
				}
		}
		
	}
	
	// Reset/init the MIDI interface
	if(mididev_init() == -1) {
		fprintf(stderr, "Error initializing midi interface.\n");
		return EXIT_FAILURE;
	}
	
	// -r/--reboot
	if(reboot_flag) {
		if(verbose)
			fprintf(stderr, "Rebooting MT32-PI and waiting 5 seconds.\n");
		mididev_send_bytes(reboot_sysex, 4);
		delay_ms(5000);
	}
	
	// -m/--mt32 and -g/--fluidsynth
	if(mode==MODE_MUNT) {
		if(verbose)
			fprintf(stderr, "Switching to MT-32 mode.\n");
		cmd_sysex[2] = 0x03;
		cmd_sysex[3] = 0x00;
		mididev_send_bytes(cmd_sysex, 5);
	} else if(mode==MODE_FLUID) {
		if(verbose)
			fprintf(stderr, "Switching to FluidSynth mode.\n");
		cmd_sysex[2] = 0x03;
		cmd_sysex[3] = 0x01;
		mididev_send_bytes(cmd_sysex, 5);
	}
	
	// -b/--romset
	if(romset==ROM_OLD) {
		if(verbose)
			fprintf(stderr, "Switching to old MT-32 romset.\n");
		cmd_sysex[2] = 0x01;
		cmd_sysex[3] = 0x00;
		mididev_send_bytes(cmd_sysex, 5);
	} else if(romset==ROM_NEW) {
		if(verbose)
			fprintf(stderr, "Switching to new MT-32 romset.\n");
		cmd_sysex[2] = 0x01;
		cmd_sysex[3] = 0x01;
		mididev_send_bytes(cmd_sysex, 5);
	} else if(romset==ROM_CM32L) {
		if(verbose)
			fprintf(stderr, "Switching to CM-32L romset.\n");
		cmd_sysex[2] = 0x01;
		cmd_sysex[3] = 0x02;
		mididev_send_bytes(cmd_sysex, 5);
	}
	
	// -s/--soundfont
	if(soundfont > -1) {
		if(verbose)
			fprintf(stderr, "Switching SoundFont to #%d.\n", soundfont);
		cmd_sysex[2] = 0x02;
		cmd_sysex[3] = soundfont;
		mididev_send_bytes(cmd_sysex, 5);
	}
	
	// -S/--mt32-rstereo
	if(mt32_rstereo > -1) {
		if(verbose)
			fprintf(stderr, "%sabling MT-32 reversed stereo mode.\n", mt32_rstereo ? "En" : "Dis");
		cmd_sysex[2] = 0x04;
		cmd_sysex[3] = mt32_rstereo;
		mididev_send_bytes(cmd_sysex, 5);
	}

	// --mt32-reset
	if(mt32_rst_flag) {
		if(verbose)
			fprintf(stderr, "Sending MT-32 reset and waiting 100ms.\n");
		mididev_send_bytes(mt32_reset, 8);
		delay_ms(100);
	}
	
	// --gm-reset
	if(gm_rst_flag) {
		if(verbose)
			fprintf(stderr, "Sending GM reset and waiting 100ms.\n");
		mididev_send_bytes(gm_reset, 6);
		delay_ms(100);
	}
	
	// --gs-reset
	if(gs_rst_flag) {
		if(verbose)
			fprintf(stderr, "Sending GS reset and waiting 100ms.\n");
		mididev_send_bytes(gs_reset, 11);
		delay_ms(100);
	}

	// -t/--mt32-txt
	if(mt32_text[0] != '\0') {
		if(verbose)
			fprintf(stderr, "Displaying \"%s\" (MT-32 mode).\n", mt32_text);
		str_to_sysex_disp_mt32(sysexbuf, mt32_text);
		mididev_send_bytes(sysexbuf, 30);
	}
	
	// -T/--sc55-txt
	if(sc55_text[0] != '\0') {
		int se_len;
		if(verbose)
			fprintf(stderr, "Displaying \"%s\" (SC-55 mode).\n", sc55_text);
		se_len = str_to_sysex_disp_sc55(sysexbuf, sc55_text);
		mididev_send_bytes(sysexbuf, se_len);
	}
	
	// -M/--midi "C0 01 C0 DE"
	if(custom_midi_len > 0) {
		if(verbose)
			fprintf(stderr, "Sending %d custom MIDI bytes.\n", custom_midi_len);
		mididev_send_bytes(custom_midi_buf, custom_midi_len);
	}
	
	// -P/--sc55-bmp FILE.BMP and -N
	if(sc55_bmp_fname[0] != '\0') {
		if(verbose)
			fprintf(stderr, "Displaying %s.\n", sc55_bmp_fname);
		bmp_to_sysex_disp_sc55(sysexbuf, sc55_bmp_fname, pic_negative_flag);
		mididev_send_bytes(sysexbuf, 74);
	}
	
	// -X/--sc55-btxt
	if(sc55_pictxt[0] != '\0') {
		if(verbose)
			fprintf(stderr, "Displaying \"%s\" using bitmap mode.\n", sc55_pictxt);
		strpic_to_sysex_disp_sc55(sysexbuf, sc55_pictxt, pic_negative_flag);
		mididev_send_bytes(sysexbuf, 74);
	}
	
	// -Y/--syx FILE.SYX
	if(syx_fname[0] != '\0') {
		unsigned char fbuf[512];
		FILE *fh;
		if(verbose)
			fprintf(stderr, "Sending %s.\n", syx_fname);


		fh = fopen(syx_fname, "rb");
		if(fh == NULL) {
			fprintf(stderr, "ERROR: Can't open %s.\n", syx_fname);
			mididev_deinit();
			return EXIT_FAILURE;
		}
		
		// go through file
		while(1) {
			int i, found_start, found_end, start_index, end_index;
			int n;
			long int read_start = ftell(fh);
			n = fread(fbuf, 1, 512, fh);
			
			found_start = 0;
			found_end = 0;
			// search for next sysex in buffer
			start_index = 0;
			end_index = 0;
			for(i=0; i<n; i++) {
				if(found_start && found_end) break;
				
				if(fbuf[i] == 0xF0) {
					if(found_start) {
						fprintf(stderr, "ERROR: Malformed SysEx in %s.\n", syx_fname);
						mididev_deinit();
						return EXIT_FAILURE;
					}
					found_start = 1;
					start_index = i;
				}
				
				if(found_start && fbuf[i] == 0xF7) {
					if(found_end) {
						fprintf(stderr, "ERROR: Malformed SysEx in %s.\n", syx_fname);
						mididev_deinit();
						return EXIT_FAILURE;
					}
					found_end = 1;
					end_index = i;
				}
			}
			if(!found_start || !found_end) {
				fprintf(stderr, "ERROR: Malformed or too long SysEx in %s.\n", syx_fname);
				mididev_deinit();
				return EXIT_FAILURE;
			}
			mididev_send_bytes(fbuf+start_index, end_index - start_index+1);
			// Wait grace period
			delay_ms(100);
			if(read_start + end_index+1 == ftell(fh))
				break;
			fseek(fh, read_start + end_index+1, SEEK_SET);
		}
		fclose(fh);
	}
	
	mididev_deinit();

	return EXIT_SUCCESS;
}

static void str_to_sysex_disp_mt32(unsigned char *sysexbuf, const char *str) {
	const unsigned char prefix[6] = { 0xF0, 0x41, 0x10, 0x16, 0x12, 0x20 };
	unsigned char *ptr;
	memset(sysexbuf, '\0', 30);
	/* Copy SysEx start and display cmd into msg */ 
	memcpy(sysexbuf, prefix, 6);
	/* Copy string into msg */
	ptr = sysexbuf+8;
	while(*str != '\0') {
		*ptr++ = *(unsigned char*)str++;
	}
	/* Generate Roland checksum */
	sysexbuf[28] = roland_checksum(sysexbuf+5, 23);
	/* SysEx end */
	sysexbuf[29] = 0xF7;
}

static int str_to_sysex_disp_sc55(unsigned char *sysexbuf, const char *str) {
	const unsigned char prefix[6] = { 0xF0, 0x41, 0x10, 0x45, 0x12, 0x10 };
	unsigned char *ptr;
	unsigned int len;
	memset(sysexbuf, '\0', 42);
	/* Copy SysEx start and display cmd into msg */ 
	memcpy(sysexbuf, prefix, 6);
	/* Copy string into msg */
	ptr = sysexbuf+8;
	len = 0;
	while(*str != '\0') {
		*ptr++ = *(unsigned char*)str++;
		len++;
	}
	/* Generate Roland checksum */
	sysexbuf[8+len] = roland_checksum(sysexbuf+5, len+3);
	/* SysEx end */
	sysexbuf[9+len] = 0xF7;
	return(len+10);
}

static void bmp_to_sysex_disp_sc55(unsigned char *sysexbuf, const char *fname, int negative) {
	const unsigned char prefix[7] = { 0xF0, 0x41, 0x10, 0x45, 0x12, 0x10, 0x01 };
	int i;
	unsigned char fbuf[64];
	FILE *fh;
	int flen;
	unsigned char pix_offset;
	memset(sysexbuf, '\0', 74);
	/* Copy SysEx start and display cmd into msg */ 
	memcpy(sysexbuf, prefix, 7);
	fh = fopen(fname, "rb");
	if(fh == NULL) {
		fprintf(stderr, "ERROR: Can't open %s.\n", fname);
		abort();
	}
	// check if this is a BMP
	if(!fread(fbuf, 1, 2, fh)) {
		fprintf(stderr, "Error reading %s.\n", sc55_bmp_fname);
		fclose(fh);
		abort();
	}
	if(!(fbuf[0]=='B' && fbuf[1]=='M')) {
		fprintf(stderr, "ERROR: %s is not a valid BMP file.\n", sc55_bmp_fname);
		fclose(fh);
		abort();
	}
	
	// check offset of pixel array
	fseek(fh, 0xA, SEEK_SET);
	if(!fread(&pix_offset, 1, 1, fh)) {
		fprintf(stderr, "Error reading %s.\n", sc55_bmp_fname);
		fclose(fh);
		abort();
	}


	fseek(fh, 0, SEEK_END);
	flen = ftell(fh);
	if(flen - pix_offset < 64) {
		fprintf(stderr, "ERROR: %s seems to be too small in size.\n", sc55_bmp_fname);
		fclose(fh);
		abort();
	}
	// read pixel array
	fseek(fh, pix_offset, SEEK_SET);
	if(!fread(fbuf, 1, 64, fh)) {
		fprintf(stderr, "Error reading %s.\n", sc55_bmp_fname);
		fclose(fh);
		abort();
	}

	fclose(fh);
	
	// Perform bit-manipulation magic to
	// build the SysEx
	for(i=0; i<16; i++) {
		unsigned char *bmprow, *sysexrow;
		bmprow = &fbuf[i*4];
		sysexrow = &sysexbuf[(15-i)+8];
		sysexrow[0] = (bmprow[0]>>3) & 0x1F;
		sysexrow[16] = ((bmprow[0]&0x07)<<2) | ((bmprow[1]>>6)&0x03);
		sysexrow[32] = (bmprow[1]>>1)&0x1F;
		sysexrow[48] = (bmprow[1] & 0x01)<<4;
		if(negative) {
			sysexrow[0] = (~sysexrow[0])&0x1F;
			sysexrow[16] = (~sysexrow[16])&0x1F;
			sysexrow[32] = (~sysexrow[32])&0x1F;
			sysexrow[48] = (~sysexrow[48])&0x10;
		}
	}
	sysexbuf[72] = roland_checksum(sysexbuf+5, 67);
	sysexbuf[73] = 0xF7;
}

static void strpic_to_sysex_disp_sc55(unsigned char *sysexbuf, const char *str, int negative) {
	const unsigned char prefix[7] = { 0xF0, 0x41, 0x10, 0x45, 0x12, 0x10, 0x01 };
	int i, slen;
	unsigned char picbuf[32];
	memset(sysexbuf, '\0', 74);
	memset(picbuf, 0, sizeof(picbuf));
	/* Copy SysEx start and display cmd into msg */ 
	memcpy(sysexbuf, prefix, 7);
	slen = strlen(str);
	if(slen > 8) {
		slen = 8;
	}
	
	for(i=0; i<slen; i++) {
		unsigned char dest_lshift, src_rshift;
		unsigned char c, *destptr, j;
		if(str[i] < 32 || str[i] > 127) {
			continue;
		}
		c = str[i] - 32;
		src_rshift = (c & 1) ? 0 : 4;
		dest_lshift = (i & 1) ? 0 : 4;
		destptr = (i<4) ? &picbuf[i/2] : &picbuf[(i-4)/2 + 16];
		for(j=0; j<8; j++) {
			destptr[j*2] |= ( (miniwi_data[c/2][j]>>src_rshift ) & 0x0F ) <<dest_lshift;
		}
	}
	
	// Perform bit-manipulation magic to
	// build the SysEx
	for(i=0; i<16; i++) {
		unsigned char *bmprow, *sysexrow;
		bmprow = &picbuf[i*2];
		sysexrow = &sysexbuf[i+8];
		sysexrow[0] = (bmprow[0]>>3) & 0x1F;
		sysexrow[16] = ((bmprow[0]&0x07)<<2) | ((bmprow[1]>>6)&0x03);
		sysexrow[32] = (bmprow[1]>>1)&0x1F;
		sysexrow[48] = (bmprow[1] & 0x01)<<4;
		if(negative) {
			sysexrow[0] = (~sysexrow[0])&0x1F;
			sysexrow[16] = (~sysexrow[16])&0x1F;
			sysexrow[32] = (~sysexrow[32])&0x1F;
			sysexrow[48] = (~sysexrow[48])&0x10;
		}
	}
	sysexbuf[72] = roland_checksum(sysexbuf+5, 67);
	sysexbuf[73] = 0xF7;
}

static unsigned char roland_checksum(const unsigned char *buf, unsigned short len) {
	const unsigned char *ptr;
	unsigned short ck_acc = 0;
	for(ptr=buf; ptr<buf+len; ptr++) {
		ck_acc = (ck_acc + (unsigned short)*ptr) & 0x7F;
	}
	return(0x80 - ck_acc);
}

static void print_usage(void) {
	printf("USAGE: " PROGRAM_NAME " [OPTIONS]\n");
	printf("OPTIONS:\n");
	printf( "--help: Print this info.\n"
			"-v/--verbose: Be verbose about what is going on.\n"
			"-r/--reboot: Reboot the Pi. Will block for a few secs to give it time.\n");
	mididev_print_usage();
	#ifdef ATARI_ST_QUOTING
	printf("-m/--mt32: Switch mt32-pi to MT-32 mode.\n"
			"-g/--fluidsynth: Switch mt32-pi to FluidSynth mode.\n"
			"-b/--romset [old, new, cm32l]: Switch MT-32 romset.\n"
			"-s/--soundfont [NUMBER]: Set FluidSynth SoundFont.\n" 
			"-S/--mt32-rstereo [0, 1]: Enable/disable MT-32 reversed stereo.\n"
			"--mt32-reset: Send an MT-32 reset SysEx message.\n"
			"--gm-reset: Send a GM reset SysEx message.\n"
			"--gs-reset: Send a GS reset SysEx message.\n"
			"\'-tSome text\': Send an MT-32 text display SysEx.\n"
			"\'-TSome text\': Send an SC-55 text display SysEx.\n"
			"-P/--sc55-bmp FILE.BMP: Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)\n"
			"\'-X/--sc55-btxt SomeText\': Display a string on the screen as a Bitmap. (SC-55)\n"
			"-N/--negative: Reverse color. Use with \'-P/--sc55-bmp\' or \'-X/--sc55-btxt\'.\n"
			"\'-M C0 01 C0 DE\': Send a list of custom MIDI bytes.\n"
			"-Y/--syx file.syx : Send the contents of a SYX-file.\n"
			"NOTE: If you use quotes in arguments, put those before unquoted ones.\n" );
	#else
	printf("-m/--mt32: Switch mt32-pi to MT-32 mode.\n"
			"-g/--fluidsynth: Switch mt32-pi to FluidSynth mode.\n"
			"-b/--romset [old, new, cm32l]: Switch MT-32 romset.\n"
			"-s/--soundfont [NUMBER]: Set FluidSynth SoundFont.\n" 
			"-S/--mt32-rstereo [0, 1]: Enable/disable MT-32 reversed stereo.\n"
			"--mt32-reset: Send an MT-32 reset SysEx message.\n"
			"--gm-reset: Send a GM reset SysEx message.\n"
			"--gs-reset: Send a GS reset SysEx message.\n"
			"-t/--mt32-txt \"Some text\": Send an MT-32 text display SysEx.\n"
			"-T/--sc55-txt \"Some text\": Send an SC-55 text display SysEx.\n"
			"-P/--sc55-bmp FILE.BMP: Display a 16x16 1bpp BMP on the screen. (SC-55 SysEx)\n"
			"-X/--sc55-btxt \"SomeText\": Display a string on the screen as a Bitmap. (SC-55)\n"
			"-N/--negative: Reverse color. Use with \'-P/--sc55-bmp\' or \'-X/--sc55-btxt\'.\n"
			"-M/--midi \"C0 01 C0 DE\": Send a list of custom MIDI bytes.\n"
			"-Y/--syx file.syx : Send the contents of a SYX-file.\n" );
	#endif
}

