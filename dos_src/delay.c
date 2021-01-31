/*
 * mt32-pi-control
 *
 * An mt32-pi control program for DOS PCs
 * and Amiga computers
 *
 * dos_src/delay.c - non-stdlib delay function
 * using the 8254 timer in DOS PCs
 *
 * Copyright (C) 2021 Andreas Zdziarstek
 */

#include <dos.h>
#include <conio.h>

#include "delay.h"

static void interrupt (*orig_isr)(void);
static volatile unsigned int ticks = 0, doscall_counter = 0;

/* interrupt handler for Timer 0 */
static void interrupt tim_0_isr(void) {
	ticks++;
	doscall_counter++;

	/* Call original DOS handler about every 55ms (approx 18.2Hz) */
	if (doscall_counter >= 55) {
		doscall_counter = 0;
		_chain_intr(orig_isr);
	} else {
		/* if not, signal EOI to the PIC */
		outp(0x20, 0x20);
	}
}

/* Hardware timer delay funtion, busy-waits for
 * delay milliseconds using the 8254 timer 0
 */
void delay_ms(unsigned int delay) {
	/* Save the address of the original 18.2Hz DOS
	 * Timer ISR
	 */
	orig_isr = _dos_getvect(0x08);

	//reset ticks and doscall counter
	ticks = 0;
	doscall_counter = 0;

	/* Disable interrupts */
	_disable();

	/* Install our new timer ISR */
	_dos_setvect(0x08, tim_0_isr);

	/* Set Timer 0 prescaler and start counting*/
	outp(0x43, 0x36);   
	outp(0x40, 0xA8);  // Prescaler 0x04A8 == 1192
	outp(0x40, 0x04);  // 1192/1193180Hz is approx 1ms 
	// We could also do 1193 but 1192 gives a better
	// approximation to the 18.2Hz DOS interrupt.
	// Don't really know if that's important though.

	/* Enable interrupts */
	_enable();

	// Busy-wait delay ms
	while(ticks < delay);

	// Disable interrupts
	_disable();
	// Restore the original timer 0 ISR
	_dos_setvect(0x08, orig_isr);

	// Restore Timer 0 configuration
	// 0x0000 is a special case that
	// gets interpreted as the highest
	// possible prescaler of 65536, yielding
	// the good old 18.2 Hz interrupt
	outp(0x43, 0x36);
	outp(0x40, 0x00);
	outp(0x40, 0x00);

	// Enable interrupts again and we're done,
	// hopefully without breaking anything :)
	_enable();
}

