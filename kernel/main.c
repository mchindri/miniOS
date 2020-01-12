#include "main.h"
#include "screen.h"
#include "interrupts.h"

void KernelMain()
{
	// __magic();    // break into BOCHS
	
	__enableSSE();  // only for demo; in the future will be called from __init.asm

	ClearScreen();

	HelloBoot();
	// TODO!!! PBIC programming; see http://www.osdever.net/tutorials/view/programming-the-pic
	initPic();

	// TODO!!! define interrupt routines and dump trap frame
	setInterruptRutines();
	__sti();
	//__launchInterrupt();
	// TODO!!! Timer programming
	initTimer();
	// TODO!!! Keyboard programming
	initDisk();

	init_memory();
	// TODO!!! Implement a simple console
	clearConsole();
	while (1)
		runConsole();
}