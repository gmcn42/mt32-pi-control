/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 *
 * parts of camd and serial code taken from
 * https://github.com/dwhinham/minisysex
 *
 * Copyright (c) 2020 Dale Whinham
 * Thanks! :)
 *
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

/* FIXME: Kludge to get CAMD includes working with -mcrt=nix13 */
#include <exec/types.h>
typedef ULONG Tag;
#define TAG_USER   ((ULONG)(1L<<31))
#define TAG_END   (0L)

#include <midi/mididefs.h>
#include <devices/serial.h>
#include <proto/alib.h>
#include <proto/camd.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "midi_dev.h"

#define MIDI_BAUD_RATE 31250L

// Serial stuff
static struct MsgPort* msg_port = NULL;
static struct IOExtSer* io_serial = NULL;

// Camd stuff
struct Library* CamdBase = NULL;
static struct MidiNode* midi_node = NULL;
static struct MidiLink* midi_link = NULL;

static char *dev_optstr = "Sl:";
static bool use_direct_serial = false;
static char camd_out[32] = "out.0";

int mididev_init(void) {
	if(use_direct_serial) {
		if (!(msg_port = CreatePort(NULL, 0L))) {
			fprintf(stderr, "Failed to create port.\n");
			return -1;
		}

		if (!(io_serial = (struct IOExtSer*) CreateExtIO(msg_port, sizeof(struct IOExtSer)))) {
			fprintf(stderr, "Failed to create I/O request.\n");
			return -1;
		}

		if (OpenDevice(SERIALNAME, 0L, (struct IORequest*) io_serial, 0L)) {
			fprintf(stderr, "Failed to open " SERIALNAME);
			return false;
		}

		io_serial->io_Baud = MIDI_BAUD_RATE;
		io_serial->io_SerFlags = SERF_RAD_BOOGIE | SERF_XDISABLED;
		io_serial->io_WriteLen = 8;
		io_serial->io_StopBits = 1;
		io_serial->IOSer.io_Command = SDCMD_SETPARAMS;
		DoIO((struct IORequest*) io_serial);

		return 0;
	}

	if (!(CamdBase = OpenLibrary("camd.library", 0L))) {
		fprintf(stderr, "Failed to open camd.library.\n");
		return -1;
	}

	if (!(midi_node = CreateMidi(MIDI_MsgQueue, 16, MIDI_SysExSize, 512L, TAG_END))) {
		fprintf(stderr, "Failed to create MIDI node.\n");
		return -1;
	}

	midi_link = AddMidiLink(
		midi_node,
		MLTYPE_Sender,
		MLINK_Name, (ULONG) "mt32-pi-ctl.out",
		MLINK_Location, (ULONG) camd_out,
		TAG_END
	);

	if(!midi_link) {
		fprintf(stderr, "Failed to create MIDI link to %s.\n", camd_out);
		return -1;
	}

	return 0;
}

int mididev_deinit(void) {
	if(use_direct_serial) {
		if (io_serial) {
			if (io_serial->IOSer.io_Device)
				CloseDevice((struct IORequest*) io_serial);

			DeleteExtIO((struct IORequest*) io_serial);
			io_serial = NULL;
		}

		if (msg_port) {
			DeletePort(msg_port);
			msg_port = NULL;
		}

		return 0;
	}

	if (midi_link) {
		RemoveMidiLink(midi_link);
		midi_link = NULL;
	}
	if (midi_node) {
		DeleteMidi(midi_node);
		midi_node = NULL;
	}
	return 0;
}

int mididev_send_bytes(const unsigned char *buf, int len) {
	if(len==0) return 0;
	
	// We're not doing any sanity checking on serial port writes
	// so if someone wants to send corrupt MIDI bytes
	// with -M for debugging or whatever they can do that
	if(use_direct_serial) {
		io_serial->IOSer.io_Length = (ULONG) len; 
		io_serial->IOSer.io_Data = (APTR) buf; 
		io_serial->IOSer.io_Command = CMD_WRITE; 
		DoIO((struct IORequest*) io_serial);
		return 0;
	}

	// CAMD however gets unhappy if we try to send weird stuff
	// so here we're doing minimal checking beforehand.
	if(buf[0] == MS_SysEx && buf[len-1] == MS_EOX) {
		PutSysEx(midi_link, (UBYTE*)buf);
	} else if (len==3) {
		MidiMsg midi_message = 
		{
			.mm_Status = buf[0],
			.mm_Data1 = buf[1],
			.mm_Data2 = buf[2],
			.mm_Port = 0
		};
		PutMidi(midi_link, midi_message.mm_Msg);
	} else {
		fprintf(stderr, "Error: Non-SysEx message with more than 3 bytes.\n");
		return -1;
	}

	return 0;
}

void mididev_print_usage(void) {
	printf("-S: Use direct serial port access instead of camd.library.\n");
	printf("-l: The camd output location to connect to. (Default: out.0)\n");
}

void mididev_add_optstr(char *optstr) {
	int main_len = strlen(optstr);
	strcpy(optstr+main_len, dev_optstr);
}

int mididev_parse_arg(int c, const char *optarg) {
	if(c=='S') {
		use_direct_serial = true;
		return 0;
	}
	if(c=='l') {
		if(strlen(optarg) > 31) {
			fprintf(stderr, "Sorry, the camd output location's name is too long.\n");
			return -1;
		}
		strcpy(camd_out, optarg);
		return 0;
	}
	return -1;
}

