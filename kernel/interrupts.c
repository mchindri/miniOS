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

typedef void (*INTERRUP_HEADER)(void);

INTERRUP_HEADER interrupt_functions[] = {
__Divide_by_0,
__Single_step,
__Non_Maskable_Interrupt_Pin,
__Breakpoint,
__Overflow,
__Bounds_check,
__Undefined_OPCode,
__No_coprocessor,
__Double_Fault,
__Coprocessor_Segment_Overrun,
__Invalid_Task_State_Segment,
__Segment_Not_Present,
__Stack_Segment_Overrun,
__General_Protection_Fault,
__Page_Fault,
0, //Unasigned
__Coprocessor_error,
__Alignment_Check,
__Machine_Check
};

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

	/* Fill the interrupt with the defaut handler */
	for (int i = 0; i < 19; i++)
	{
		INTERRUP_HEADER fun = interrupt_functions[i];
		if (fun) {
			IDT_desc intDesc = defautlDesc;
			intDesc.offset_1 = (WORD)fun;
			intDesc.offset_2 = (WORD)((QWORD)fun >> 16);
			intDesc.offset_3 = (DWORD)((QWORD)fun >> 32);
			p = IDT + i; //Timer entry
			*p = intDesc;
		}
	}

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

void defaultInterruptHandler(QWORD intId) {
	//__magic();
	consolePrint("Default Interrupt ( ");
	PrintNb(intId, 10);
	consolePrint(" )\n");
}
