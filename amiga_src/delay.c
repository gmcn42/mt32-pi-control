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

#include <proto/dos.h>
#include <proto/exec.h>

#include "delay.h"

void delay_ms(unsigned int delay) {
	if(DOSBase) {
		//System-friendly delay, uses 50Hz ticks normally
		Delay(delay/(1000/TICKS_PER_SECOND) + 1);
	} else {
		// with mcrt=nix13 we should get a DOSBase through proto/dos.h so this should
		// (hopefully) never happen
		fprintf(stderr, "WARNING: Delay seems not to be working due to missing DOSBase.\n");
	}
}

