#include "main.h"

#define TIMER_COUNTER_REGISTER			0x40
#define TIMER_CONTROL_REGISTER			0x43

#define bcp		0
#define op_mode	1
#define rw_mode	4
#define select	6

#define TIMER_FREQ	1193181
#define OUT_FREQ	100 //100 Hz -> 10 ms Teorie: Practice 1ms

QWORD g_ms;

void initTimer() {
	WORD timer_count = TIMER_FREQ / OUT_FREQ;
	BYTE cmd = 0;
	cmd |= 0 << bcp;
	cmd |= 2 << op_mode;
	cmd |= 3 << rw_mode;
	cmd |= 0 << select;

	__outbyte(TIMER_CONTROL_REGISTER, cmd);
	__outbyte(TIMER_COUNTER_REGISTER, (BYTE)timer_count);
	__outbyte(TIMER_COUNTER_REGISTER, timer_count >> 8);
	g_ms = 0;
}


void wait(QWORD ms)
{
	QWORD init = g_ms;
	while (g_ms - init < ms);
}

//#define PRINT_SECONDS

void timerHandler() {
	g_ms++;

#ifdef PRINT_SECONDS
	if (g_ms % 1000 == 0)
		PrintNb(g_ms / 1000, 10);
#endif
	//__iret_PIC1(); //return ;
}
