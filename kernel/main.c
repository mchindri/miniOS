#include "main.h"
#include "screen.h"
#include "interrupts.h"

void KernelMain()
{
	 //__magic();    // break into BOCHS
	
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
	consolePrint("Starting keyboard test [Pres ESC to continue]\n");
	wait(2000);
	testKeyboard();
	consolePrint("KeyBoard test done\n");

	// TODO!!! Implement a simple console
	clearConsole();
	consolePrint("Starting console test [Pres ESC to continue]\n");
	runConsole();

	// TODO!!! read disk sectors using PIO mode ATA
	consolePrint("Starting disk test\n");
	wait(2000);
	testDisk();
	consolePrint("Disk test done\n");
	runConsole(); //Acting as a wait ESC
	//__magic();
	// TODO!!! Memory management: virtual, physical and heap memory allocators

	consolePrint("Starting allocation test\n");
	wait(2000);
	test_memory_allocation();
	consolePrint("Allocation test done\n");
	runConsole(); //Acting as a wait ESC

	while (1)
		runConsole();
}