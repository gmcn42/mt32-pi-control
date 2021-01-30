/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs
 * and Amiga computers
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 */

#ifndef __DELAY_H__
#define __DELAY_H__

#ifdef USE_CUSTOM_DELAY
void delay_ms(unsigned int delay);
#else
#include <time.h>

static void delay_ms(unsigned int delay) {
	clock_t start = clock();
	while(clock() - start < (1000UL*(unsigned long)delay)/CLOCKS_PER_SEC);
}
#endif

#endif /* __DELAY_H__ */
