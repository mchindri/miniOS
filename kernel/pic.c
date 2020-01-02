#define INIT_CMD 0x11
#define BASE_ADDRESS_IVT    32 //Befor 32 are free for use. Map form 32 to 40 for hardware interupts
#define IRQ2_BIT            (1 << 2) //Interrupt for mapping the slave PIC
#define IRQ2_NB             (2) //Interrupt for mapping the slave PIC
#define uPM                 (1) //Activate 80x86 mode

#define PIC1_CMD_AND_STATUS_REGISTER    0x20
#define PIC2_CMD_AND_STATUS_REGISTER    0xA0
#define PIC1_MASK_AND_DATA_REGISTER    0x21
#define PIC2_MASK_AND_DATA_REGISTER    0xA1


/*
IRQ0	Timer
IRQ1	Keyboard
IRQ2	Cascade for 8259A Slave controller //If enable some of the slave interrupts, this must be activateed
IRQ3	Serial port 2
IRQ4	Serial port 1
IRQ5	AT systems: Parallel Port 2. PS/2 systems: reserved
IRQ6	Diskette drive
IRQ7	Parallel Port 1
*/
#define PIC1_ACTIVE_INTERRUPTS          (0xFC)
/*
IRQ8	CMOS Real time clock
IRQ9	CGA vertical retrace
IRQ10	Reserved
IRQ11	Reserved
IRQ12	AT systems: reserved. PS/2: auxiliary device
IRQ13	FPU
IRQ14	Hard disk controller
IRQ15	Reserved
*/
#define PIC2_ACTIVE_INTERRUPTS          (0xFF)

void initPic() {

	/* Send init command to PIC1 and PIC2 (ICW 1)*/
	__outbyte(PIC1_CMD_AND_STATUS_REGISTER, INIT_CMD);
	__outbyte(PIC2_CMD_AND_STATUS_REGISTER, INIT_CMD);

	/* Set base address for interrupts (ICW 2) */
	__outbyte(PIC1_MASK_AND_DATA_REGISTER, BASE_ADDRESS_IVT);
	__outbyte(PIC2_MASK_AND_DATA_REGISTER, BASE_ADDRESS_IVT + 8);

	/* Configure IRQ communication lines between PICs (ICW 3) */
	__outbyte(PIC1_MASK_AND_DATA_REGISTER, IRQ2_BIT);
	__outbyte(PIC2_MASK_AND_DATA_REGISTER, IRQ2_NB);

	/* Enables 80x86 mode (ICW 4) */
	__outbyte(PIC1_MASK_AND_DATA_REGISTER, uPM);
	__outbyte(PIC2_MASK_AND_DATA_REGISTER, uPM);

	/* Set MASK for the interupts*/
	__outbyte(PIC1_MASK_AND_DATA_REGISTER, PIC1_ACTIVE_INTERRUPTS);
	__outbyte(PIC2_MASK_AND_DATA_REGISTER, PIC2_ACTIVE_INTERRUPTS);

}