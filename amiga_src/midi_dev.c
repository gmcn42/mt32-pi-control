#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "midi_dev.h"
#include "getopt.h"

static char *dev_optstr = "S";
static bool use_direct_serial = false;

int mididev_init(void) {
	printf((use_direct_serial) ? "Using direct serial access.\n" : "Using camd.library\n");	
	return 0;
}

int mididev_deinit(void) {
	return 0;
}

int mididev_send_bytes(const unsigned char *buf, int len) {
	for(short i=0; i<len; i++) {
		printf("%02X ", buf[i]);
	}
	return 0;
}

void mididev_print_usage(void) {
	printf("-S: Use direct serial port access instead of camd.library\n");
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
	return -1;
}

