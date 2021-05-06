#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi_dev.h"
#include "getopt.h"
#include "mpu401.h"
#include "delay.h"

static int mpubase = 0x330;

static char *dev_optstr = "p:";

int mididev_init(void) {
	if(mpu401_rst(mpubase) == -1) {
		fprintf(stderr, "ERROR: MPU401 not answering at port 0x%X.\n", mpubase);
		return -1;
	}
	mpu401_uart(mpubase);
	
	/* 
	 * Wait for 100ms, some intelligent MPU-401s seem to need this.
	 * (should fix github.com/gmcn42/mt32-pi-control/issues/2)
	 */
	delay_ms(100); 
	return 0;
}

int mididev_deinit(void) {
	mpu401_rst_nowait(mpubase);
	return 0;
}

int mididev_send_bytes(const unsigned char *buf, int len) {
	mpu401_send_bytes(mpubase, buf, len);
	return 0;
}

void mididev_print_usage(void) {
	printf("-p [ADDR]: Set the port address of the MPU401 interface. Default: 330.\n");
}

void mididev_add_optstr(char *optstr) {
	int main_len = strlen(optstr);
	strcpy(optstr+main_len, dev_optstr);
}

int mididev_parse_arg(int c, const char *optarg) {
	if(c=='p') {
		mpubase = (int)strtol(optarg, NULL, 16);
		if(!mpubase)
			return -1;
		return 0;
	}
	return -1;
}

