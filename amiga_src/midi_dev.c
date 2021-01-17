#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* FIXME: Kludge to get CAMD includes working with -mcrt=nix13 */
#include <exec/types.h>
typedef ULONG Tag;
#define TAG_USER   ((ULONG)(1L<<31))
#define TAG_END   (0L)

#include <midi/mididefs.h>
#include <proto/camd.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "midi_dev.h"

struct Library* CamdBase = NULL;
static struct MidiNode* midi_node = NULL;
static struct MidiLink* midi_link = NULL;

static char *dev_optstr = "Sl:";
static bool use_direct_serial = false;
static char camd_out[32] = "out.0";

int mididev_init(void) {
	if(use_direct_serial) {
		printf("Using direct serial port access\n");
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
	if(use_direct_serial) {
		for(short i=0; i<len; i++) {
			printf("%02X ", buf[i]);
		}
		printf("\n");
		return 0;
	}
	
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
	printf("-S: Use direct serial port access instead of camd.library\n");
	printf("-l: The camd output location to connect to (Default: out.0)\n");
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

