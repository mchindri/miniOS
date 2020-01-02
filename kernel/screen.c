#include "screen.h"

static PSCREEN gVideo = (PSCREEN)(0x000B8000);

void itoa(QWORD n, char* buffer, int base);

void CursorMove(int row, int col)
{
    unsigned short location = (row * MAX_COLUMNS) + col;       /* Short is a 16bit type , the formula is used here*/

    //Cursor Low port
    __outbyte(0x3D4, 0x0F);                                    //Sending the cursor low byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)(location & 0xFF));

    //Cursor High port
    __outbyte(0x3D4, 0x0E);                                    //Sending the cursor high byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)((location >> 8) & 0xFF)); //Char is a 8bit type
}

void CursorPosition(int pos)
{
    int line, col;

    if (pos > MAX_OFFSET)
    {
        pos = pos % MAX_OFFSET;
    }

    line = pos / MAX_COLUMNS;
    col = pos % MAX_COLUMNS;

    CursorMove(line, col);
}

void HelloBoot()
{
    int i, len;
	char boot[] = "Hello Boot! Greetings from C...";

	len = 0;
	while (boot[len] != 0)
	{
		len++;
	}

	for (i = 0; (i < len) && (i < MAX_OFFSET); i++)
	{
		gVideo[i].color = 10;
		gVideo[i].c = boot[i];
	}
    CursorPosition(i);
}

void Print(char*str)
{
    ClearScreen();
    int i, len;
    len = 0;
    while (str[len] != 0)
    {
        len++;
    }

    for (i = 0; i < len; i++)
    {
        gVideo[i % MAX_OFFSET].color = 10;
        gVideo[i % MAX_OFFSET].c = str[i];
    }
    CursorPosition(i % MAX_OFFSET);
}

void ClearScreen()
{
    int i;

    for (i = 0; i < MAX_OFFSET; i++)
    {
        gVideo[i].color = 10;
        gVideo[i].c = ' ';
    }

    CursorMove(0, 0);
}

void PrintNb(QWORD nb, int base) {
	char buff[20];
    char buff2[20];
    itoa(nb, buff, base);
    //REvers nb
    char* p;
    for (p = buff; *p; p++);
    p--;
    char *q = buff2;
    while (p >= buff)
        *(q++) = *(p--);
    *q = 0;
    Print(buff2);
    //__magic();
}

void itoa(QWORD n, char* buffer , int base) {
    //int n = abs(value);
	int i = 0;
	while (n)
	{
		int r = n % base;

		if (r >= 10)
			buffer[i++] = 65 + (r - 10);
		else
			buffer[i++] = 48 + r;

		n = n / base;
	}

	// if number is 0
	if (i == 0)
		buffer[i++] = '0';

	// If base is 10 and value is negative, the resulting string 
	// is preceded with a minus sign (-)
	// With any other base, value is always considered unsigned
	//if (value < 0 && base == 10)
	//	buffer[i++] = '-';

	buffer[i] = '\0'; // null terminate string
}