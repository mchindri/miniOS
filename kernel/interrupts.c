#include "main.h"
#include "screen.h"

#pragma pack(push,1)
typedef struct {
	WORD offset_1; // offset bits 0..15
	WORD selector; // a code segment selector in GDT or LDT
	BYTE ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	BYTE type_attr; // type and attributes
	WORD offset_2; // offset bits 16..31
	DWORD offset_3; // offset bits 32..63
	DWORD zero;     // reserved
}IDT_desc;

typedef struct{
	WORD    size;
	QWORD   offset;
}IDTr_desc;
#pragma pack(pop)

IDT_desc IDT[256];

void setInterruptRutines() {
	IDT_desc defautlDesc = {
		.offset_1 = (WORD)__defaultInterruptHandler,
		.selector = 40,
		.ist = 0,
		/*
		1b P: Present -> 1
		2b DPL: Privilege 0
		1b z : 0
		4b Gate type: Interrupt
		*/
		.type_attr = 0x8E, //0b1000110
		.offset_2 = (WORD)((QWORD)__defaultInterruptHandler >> 16),
		.offset_3 = (DWORD)((QWORD)__defaultInterruptHandler >> 32),
		.zero = 0
	};

	IDT_desc* p;
	/* FILL IDT with default handler */
	/*WORD i, f;
	i = 0x9;
	f = 0x15;
	f--;
	for (p = IDT + f; p >= IDT + i; *(p--) = defautlDesc);*/
	//for (p = IDT + 255; p >= IDT; *(p--) = defautlDesc);
	//for (p = IDT + 255; p >= IDT; memset(p--, 0, sizeof(*p)));
	
	//p = IDT + 0x8;
	//memset(p, 0, sizeof(IDT_desc));


	/* Define a default handler for test interrupts*/
	p = IDT + 60;
	*p = defautlDesc;

	IDT_desc timerDesc = defautlDesc;
	timerDesc.offset_1 = (WORD)__timerHandler;
	timerDesc.offset_2 = (WORD)((QWORD)__timerHandler >> 16);
	timerDesc.offset_3 = (DWORD)((QWORD)__timerHandler >> 32);

	p = IDT + 32; //Timer entry
	*p = timerDesc;

	IDT_desc keyDesc = defautlDesc;
	keyDesc.offset_1 = (WORD)__keyHandler;
	keyDesc.offset_2 = (WORD)((QWORD)__keyHandler >> 16);
	keyDesc.offset_3 = (DWORD)((QWORD)__keyHandler >> 32);

	p = IDT + 33; //Keyboard entry
	*p = keyDesc;

	/* Load de Interrupt description table to the IDTR*/
	IDTr_desc desc;
	desc.offset = (QWORD)IDT;
	desc.size = 256 * 16 - 1;
	__lidt(&desc);
}

void defaultInterruptHandler() {
	Print("Default interrupt");
}
