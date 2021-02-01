/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs, Atari ST,
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek 
 *
 */

#include <stdio.h>

int mididev_init(void) {
	return 0;
}

int mididev_deinit(void) {
	return 0;
}


int mididev_send_bytes(const unsigned char *buf, int len) {
	if(len==0) return 0;	
	
	if((buf[0] == 0xF0 && buf[len-1] == 0xF7) || len==3) {
		for(int i=0; i<len; i++) {
			fprintf(stderr, "%02X ", buf[i]);
		}
		fprintf(stderr, "\n");
	} else {
		fprintf(stderr, "Error: Non-SysEx message with more than 3 bytes.\n");
		return -1;
	}
	
	//fprintf(stderr, "len:%d end:%02X\n", len, buf[len-1]); 
	//	for(int i=0; i<len; i++) {
	//		fprintf(stderr, "%02X ", buf[i]);
	//	}
//fprintf(stderr, "\n");

	return 0;
}

void mididev_print_usage(void) {}

void mididev_add_optstr(char *optstr) {}

int mididev_parse_arg(int c, const char *optarg) {
	return 0;
}

