#include "windows.h"
#include "delay.h"

void delay_ms(unsigned int delay) {
	Sleep(delay);
}

