#ifndef _MAIN_H_
#define _MAIN_H_

#include <intrin.h>
#include "String.h"

#define BREAK(str) do{\
		Print(str);\
		__magic();}while(0)




#define STR(x) consolePrint((x))
#define NB(x) PrintNb((x), 10)
#define PL(str) {STR(#str); STR(": "); consolePrint(str); consolePrint("\n");}

void printDesc();

//
// default types
//
typedef unsigned __int8     BYTE, *PBYTE;
typedef unsigned __int16    WORD, *PWORD;
typedef unsigned __int32    DWORD, *PDWORD;
typedef unsigned __int64    QWORD, *PQWORD;
typedef signed __int8       INT8;
typedef signed __int16      INT16;
typedef signed __int32      INT32;
typedef signed __int64      INT64;

//
// exported functions from __init.asm
//
void __cli(void);
void __sti(void);
void __magic(void);         // MAGIC breakpoint into BOCHS (XCHG BX,BX)
void __enableSSE(void);
void __launchInterrupt(QWORD nb);


void initTimer();
void initPic();
void setInterruptRutines();
void __defaultInterruptHandler();

void __Divide_by_0();
void __Single_step();
void __Non_Maskable_Interrupt_Pin();
void __Breakpoint();
void __Overflow();
void __Bounds_check();
void __Undefined_OPCode();
void __No_coprocessor();
void __Double_Fault();
void __Coprocessor_Segment_Overrun();
void __Invalid_Task_State_Segment();
void __Segment_Not_Present();
void __Stack_Segment_Overrun();
void __General_Protection_Fault();
void __Page_Fault();
void __Coprocessor_error();
void __Alignment_Check();
void __Machine_Check();


void __timerHandler(void);
void __keyHandler(void);

void __iret_PIC1();
void __iret_PIC2();
void __iret();

void wait(QWORD ms);

void runConsole();
void consolePrint(char* str);
void clearConsole();
void PrintNb(QWORD nb, int base);

//Private
void timerHandler();
void keyHandler();
void keyHandler();

char getLastAscii();


void testDisk();
void initDisk();
int readDiskSector(DWORD lba, BYTE count, WORD* buff);

void test_memory_allocation();

QWORD my_malloc(QWORD n);
void my_free(QWORD addr);
int  writeQ(QWORD addr, QWORD n);
int  readQ(QWORD addr, QWORD *n);

void PrintRight(char* str);
void _printNb(QWORD nb, int base, char* str);
void enableTimer(int status);
int my_strncmp(char* s1, char* s2);

void init_memory();

#endif // _MAIN_H_