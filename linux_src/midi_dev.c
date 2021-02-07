/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs, Atari ST,
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek 
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include "midi_dev.h"

typedef enum {
	MIDI_OP_NOTEOFF = 0x80,
	MIDI_OP_NOTEON = 0x90,
	MIDI_OP_KEYPRESS = 0xA0,
	MIDI_OP_CTRL = 0xB0,
	MIDI_OP_PC = 0xC0,
	MIDI_OP_CC = 0xD0,
	MIDI_OP_PB = 0xE0,
	MIDI_OP_SYX = 0xF0
} midi_op_t;

static snd_seq_t *seq = NULL;
static int src_port = -1, dst_port = -1, dst_client = -1;

static char *dev_optstr = "p:";

int mididev_init(void) {
	if(dst_port == -1 || dst_client == -1) {
		fprintf(stderr, "ERROR: You must specify ALSA client and port using -p [CLIENT]:[PORT]\n");
		return -1;
	}
	
	if(snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		fprintf(stderr, "ERROR: Could not open ALSA sequencer.\n");
		return -1;
	}
	
	if( (src_port = snd_seq_create_simple_port(
					seq,
					"mt32-pi-ctl",
					SND_SEQ_PORT_CAP_READ,
					SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		fprintf(stderr, "ERROR: Could not create MIDI output.\n");
		snd_seq_close(seq);
		return -1;
	}
	
	if(snd_seq_connect_to(seq, src_port, dst_client, dst_port) < 0) {
		fprintf(stderr, "ERROR: Could not connect to MIDI port %d:%d.\n", dst_client, dst_port);
		mididev_deinit();
		return -1;
	}

	return 0;
}

int mididev_deinit(void) {
	snd_seq_delete_simple_port(seq, src_port);
	snd_seq_close(seq);
	return 0;
}

int mididev_send_bytes(const unsigned char *buf, int len) {
	if(len==0) return 0;

	snd_seq_event_t ev;
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_direct(&ev);
	snd_seq_ev_set_source(&ev, src_port);
	snd_seq_ev_set_dest(&ev, dst_client, dst_port);

	midi_op_t o = (midi_op_t)(buf[0] & 0xF0);
	
	unsigned char *tempbuf = NULL;

	switch(o) {
		case MIDI_OP_NOTEOFF:
			if(len != 3) goto handle_error;
			snd_seq_ev_set_noteoff(&ev, buf[0]&0x0F, buf[1], buf[2]);
			break;
		case MIDI_OP_NOTEON:
			if(len != 3) goto handle_error;
			snd_seq_ev_set_noteon(&ev, buf[0]&0x0F, buf[1], buf[2]);
			break;
		case MIDI_OP_KEYPRESS:
			if(len != 3) goto handle_error;
			snd_seq_ev_set_keypress(&ev, buf[0]&0x0F, buf[1], buf[2]);
			break;
		case MIDI_OP_CTRL:
			if(len != 3) goto handle_error;
			snd_seq_ev_set_controller(&ev, buf[0]&0x0F, buf[1], buf[2]);
			break;
		case MIDI_OP_PC:
			if(len != 2) goto handle_error;
			snd_seq_ev_set_pgmchange(&ev, buf[0]&0x0F, buf[1]);
			break;
		case MIDI_OP_CC:
			if(len != 2) goto handle_error;
			snd_seq_ev_set_chanpress(&ev, buf[0]&0x0F, buf[1]);
			break;
		case MIDI_OP_PB:
			if(len != 3) goto handle_error;
			unsigned char par = (buf[1] & 0x7F) + ((buf[2] & 0x7F)<<7);
			snd_seq_ev_set_pitchbend(&ev, buf[0]&0x0F, par);
			break;
		case MIDI_OP_SYX:
			if(buf[len-1] != 0xF7) goto handle_error;
			{
				tempbuf = malloc(len);
				if(tempbuf == NULL) {
					fprintf(stderr, "ERROR: Memory allocation error.\n");
					return -1;
				}
				memcpy(tempbuf, buf, len);
				snd_seq_ev_set_sysex(&ev, len, tempbuf);
			}
			break;
		default:
handle_error:
			fprintf(stderr, "ERROR: Unknown MIDI message.\n");
			return -1;
			break;
	}
	
	snd_seq_event_output(seq, &ev);
	snd_seq_drain_output(seq);

	if(tempbuf != NULL) free(tempbuf);
	return 0;
}

void mididev_print_usage(void) {
	printf("-p \"[CLIENT]:[PORT]\" : The ALSA MIDI client and port address to output to (*MANDATORY*).\n");
}

void mididev_add_optstr(char *optstr) {
	int main_len = strlen(optstr);
	strcpy(optstr+main_len, dev_optstr);
}

int mididev_parse_arg(int c, const char *optarg) {
	if(c != 'p') {
		fprintf(stderr, "ERROR: Unknown option \'%c\'.\n", c);
		return -1;
	}
	size_t len = strlen(optarg);
	char *temparg = (char*) malloc(len+1);
	if(temparg == NULL) {
		fprintf(stderr, "Error allocating memory.\n");
		return -1;
	}
	memcpy(temparg, optarg, len+1);

	char *firstarg = strtok(temparg, ":");
	if(firstarg == NULL) {
		return -1;
	}
	char *secondarg = firstarg + strlen(firstarg) + 1;
	if(secondarg - temparg >= len) {
		return -1;
	}
	dst_client = (int) strtol(firstarg, NULL, 10);
	dst_port = (int) strtol(secondarg, NULL, 10);
	free(temparg);
	return 0;
}

