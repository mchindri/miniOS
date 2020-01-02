#include "main.h"

#define MASTER_BUS_SELECT	(0<<4)
#define LBA_BIT				(1<<6)
#define RESERVED_SET		(0xA0)

#define DATA_REGISTER			0x1F0
#define ERROR_REGISTER			0x1F1
#define FEATURE_REGISTER		0x1F1
#define COUNT_REGISTER			0x1F2
#define LBA_LO_REGISTER			0x1F3
#define LBA_MI_REGISTER			0x1F4
#define LBA_HI_REGISTER			0x1F5
#define DRIVER_REGISTER			0x1F6
#define CMD_REGISTER			0x1F7
#define STATUS_REGISTER			0x1F7

#define DEVICE_CTRL_REG			0x3F6
#define nIEN_PIN				(1 << 1)

#define ERR_BIT					(1 << 0)
#define DRQ_BIT					(1 << 3)
#define DF_BIT					(1 << 5)
#define RDY_BIT					(1 << 6)
#define BSY_BIT					(1 << 7)



void initDisk() {
	__outbyte(DEVICE_CTRL_REG, nIEN_PIN); //Disable interrupts
}


int myPool(WORD *buff, BYTE actualCount) {
	BYTE status;
	status = __inbyte(STATUS_REGISTER);//400ns delay
	status = __inbyte(STATUS_REGISTER);
	status = __inbyte(STATUS_REGISTER);
	status = __inbyte(STATUS_REGISTER);
	status = __inbyte(STATUS_REGISTER);
	while (1) {
		if ((status & ERR_BIT) == ERR_BIT || (status & DF_BIT) == DF_BIT) {
			PrintNb(status, 16);
			__magic();
			status = __inbyte(ERROR_REGISTER);
			PrintNb(status, 16);
			__magic();
			return -1;
		}
		if ((status & BSY_BIT) == 0 && (status & DRQ_BIT) == DRQ_BIT)
			return 0;
		status = __inbyte(STATUS_REGISTER);
	}
	return 0;
}

int readDiskSector(DWORD lba, BYTE count, WORD *buff) {
	__outbyte(FEATURE_REGISTER, 0x0);
	__outbyte(COUNT_REGISTER, count);
	__outbyte(LBA_LO_REGISTER, (BYTE)lba);
	__outbyte(LBA_MI_REGISTER, (BYTE)(lba >> 8));
	__outbyte(LBA_HI_REGISTER, (BYTE)(lba >> 16));
	BYTE cmd;
	cmd = RESERVED_SET | MASTER_BUS_SELECT | LBA_BIT | ((lba >> 24) & 0x0f);
	__outbyte(DRIVER_REGISTER, cmd);
	cmd = 0x20; //Read sectors
	__outbyte(CMD_REGISTER , cmd);
	//Poll
	BYTE actualCount = 0;
	int j = 0;
	while (actualCount < count) {
		if (0 != myPool(buff, actualCount))
			return -1;
		for (int i = 0; i < 256; i++)
			buff[j++] = __inword(DATA_REGISTER);
		actualCount++;
	}
	return 0;
}

void testDisk() {
	initDisk();
	BYTE buff[1025];
	memset(buff, 0, sizeof(buff));
	//__magic();
	if (0 != readDiskSector(0, 2, (WORD*)buff))
		consolePrint("Error\n");
	else {
		//buff[100] = 0;
		consolePrint(buff);
		consolePrint("\n");
	}
}