#include <osbind.h>

#include "delay.h"

#define TIM_A_MFP 13
#define TIM_A_XBIOS 0

#define MFP_ISR_RUNNING_REG *((volatile unsigned char*)0xFFFFFA0FL)
#define MFP_VECTOR_REG *((volatile unsigned char*)0xFFFFFA17L)

#define ISR_RUNNING_TIM_A 0x20
#define VECTREG_SW_EOI 0x08

volatile unsigned int ticks = 0;

static void __attribute__((interrupt)) tim_a_isr(void) {
	ticks++;
	// Notify MFP of end-of-interrupt bc we're in
	// sw eoi mode
	MFP_ISR_RUNNING_REG &= ~ISR_RUNNING_TIM_A;
}

static void tim_a_set_sei( void ) {
	// Set software end-of-interrupt mode
	MFP_VECTOR_REG |= VECTREG_SW_EOI;	
}

void delay_ms(unsigned int delay) {
	// Disable the timer int if it was enabled
	Jdisint(TIM_A_MFP);
	
	// reset elapsed ticks
	ticks = 0;

	// Initialize Timer A (for user progs)
	// Set Prescaler to 50 and counter start to 50
	// 50/(2.4576MHz) * 50 is approx 1ms
	// Set ISR to our handler
	Xbtimer(TIM_A_XBIOS, 4, 50, tim_a_isr);
	
	// NOt sure if this is 100% necessary or Xbtimer
	// always takes care of this. In any case, here
	// we set software EOI mode and enable the Timer A
	// interrupt
	Supexec(tim_a_set_sei);
	Jenabint(TIM_A_MFP);

	// Wait until delay ms have elapsed
	while(ticks < delay);
	// Disable the interrupt again
	Jdisint(TIM_A_MFP);
}

