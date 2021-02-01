#include <time.h>
#include <errno.h>

#include "delay.h"

void delay_ms(unsigned int delay) {
	struct timespec ts, tr;
	ts.tv_sec = (time_t)(delay/1000);
	ts.tv_nsec = (long) ((delay%1000)*1000000);
	while((clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, &tr))==EINTR) {
		ts = tr;
	}
}

