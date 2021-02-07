/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs, Atari ST,
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek 
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "windows.h"

#include "midi_dev.h"

int output_port = -1;
HMIDIOUT midi_handle = NULL;
static volatile bool port_busy = false;
static char *dev_optstr = "lp:";

static void list_midi_outputs(void);
void CALLBACK midi_out_cb(
		HMIDIOUT mh,
		UINT wMsg,
		DWORD_PTR dwInst,
		DWORD_PTR dwp1,
		DWORD_PTR dwp2
		);

int mididev_init(void) {
	if(output_port == -1) {
		fprintf(stderr, "ERROR: You must specify the MIDI output port number with \"-p PORT\". You can list available ports with \"-l\".\n");
		return -1;
	}

	if(midiOutOpen(&midi_handle, output_port, (DWORD)(void*)midi_out_cb, 0, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
		fprintf(stderr, "ERROR: Could not open MIDI output port %d.\n", output_port);
		return -1;
	}
	return 0;
}

int mididev_deinit(void) {
	midiOutClose(midi_handle);
	return 0;
}

int mididev_send_bytes(const unsigned char *buf, int len) {
	if(len==0) return 0;
	
	
	if(buf[0] == 0xF0 && buf[len-1] == 0xF7) {
		MIDIHDR mheader;
		char *tempbuf = (char *) malloc(len);
		if(tempbuf == NULL) {
			fprintf(stderr, "Error allocating memory for MIDI output buffer.\n");
			return -1;
		}
		memcpy(tempbuf, buf, len);
		mheader.lpData = tempbuf;
		mheader.dwBufferLength = len;
		mheader.dwFlags = 0;
		mheader.dwBytesRecorded = len;
		if(midiOutPrepareHeader(midi_handle, &mheader, sizeof(mheader)) != MMSYSERR_NOERROR) {
			free(tempbuf);
			fprintf(stderr, "ERROR: Failed to prepare MIDI header.\n");
			return -1;
		}

		port_busy = true;
		if(midiOutLongMsg(midi_handle, &mheader, sizeof(mheader)) != MMSYSERR_NOERROR) {
			midiOutUnprepareHeader(midi_handle, &mheader, sizeof(mheader));
			free(tempbuf);
			fprintf(stderr, "ERROR: Failed to send SysEx message.\n");
			return -1;
		}

		// Wait for message to be fully sent. port_busy is reset via callback
		while(port_busy) {
			Sleep(10);
		}

		midiOutUnprepareHeader(midi_handle, &mheader, sizeof(mheader));
		free(tempbuf);
		return 0;
	}

	if(len==2) {
		DWORD out = ((DWORD)buf[1])<<8 | (DWORD)buf[0];
		if(midiOutShortMsg(midi_handle, out) != MMSYSERR_NOERROR) {
			fprintf(stderr, "Error sending MIDI message: %02X %02X\n", buf[0], buf[1]);
			return -1;
		}
		return 0;
	}

	if(len==3) {
		DWORD out =  ((DWORD)buf[2])<<16 | ((DWORD)buf[1])<<8 | (DWORD)buf[0];
		if(midiOutShortMsg(midi_handle, out) != MMSYSERR_NOERROR) {
			fprintf(stderr, "Error sending MIDI message: %02X %02X %02X\n", buf[0], buf[1], buf[2]);
			return -1;
		}
		return 0;
	}

	fprintf(stderr, "ERROR: Invalid MIDI message.\n");
	return -1;
}

void mididev_print_usage(void) {
	printf("-p PORT : Set MIDI output port number (*MANDATORY*).\n");
	printf("-l : List available MIDI output ports and exit.\n");
}

void mididev_add_optstr(char *optstr) {
	int main_len = strlen(optstr);
	strcpy(optstr+main_len, dev_optstr);
}

int mididev_parse_arg(int c, const char *optarg) {
	if(c=='l') {
		list_midi_outputs();
		exit(EXIT_SUCCESS);
	}

	if(c=='p') {
		output_port = (int) strtoul(optarg, NULL, 10);
		return 0;
	}

	fprintf(stderr, "ERROR: Unknown option \'%c\'.\n", c);
	return -1;
}

static void list_midi_outputs(void) {
	UINT n_outs = 0;
	MIDIOUTCAPS c;

	n_outs = midiOutGetNumDevs();
	if (!n_outs) {
		fprintf(stderr, "ERROR: No MIDI output ports found.\n");
		return;
	}

	printf("MIDI output ports:\n");
	for(UINT i=0; i < n_outs; i++) {
		midiOutGetDevCaps(i, &c, sizeof(MIDIOUTCAPS));
		fprintf(stderr, "%d: %s\n", i, c.szPname);
	}
}

void CALLBACK midi_out_cb( HMIDIOUT mh, UINT wMsg,
		DWORD_PTR dwInst, DWORD_PTR dwp1, DWORD_PTR dwp2 ) {
	if(wMsg == MOM_DONE) {
		port_busy = false;
	}
}

