#include "main.h"


//For testing scrolling
#define LINES       25
#define COLUMNS     80
#define MAX_POS     (LINES * COLUMNS)//25 lines * 80 chars

#pragma pack(push)
#pragma pack(1)
typedef struct _CONSOLE
{
	char c;
	BYTE color;
}CONSOLE, * PCONSOLE;
#pragma pack(pop)

static PCONSOLE gConsole = (PCONSOLE)(0x000B8000);

#define TAB			'\t'
#define ENDL		'\n'
#define BACK_SPACE	'\b'

int currentPos = 0;

void updatePosition();
void putConsoleChar(char C);
void scrollConsole();

void cursorMove(int row, int col)
{
    unsigned short location = (row * COLUMNS) + col;       /* Short is a 16bit type , the formula is used here*/

    //Cursor Low port
    __outbyte(0x3D4, 0x0F);                                    //Sending the cursor low byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)(location & 0xFF));

    //Cursor High port
    __outbyte(0x3D4, 0x0E);                                    //Sending the cursor high byte to the VGA Controller
    __outbyte(0x3D5, (unsigned char)((location >> 8) & 0xFF)); //Char is a 8bit type
}

void updatePosition()
{
    int line, col;
    currentPos++;
    if (currentPos >= MAX_POS)
    {
        currentPos = (LINES - 1) * COLUMNS;
        scrollConsole();
    }

    line = currentPos / COLUMNS;
    col = currentPos % COLUMNS;

    cursorMove(line, col);
}


void putConsoleChar(char C) {
    gConsole[currentPos].color = 10;
    gConsole[currentPos].c = C;
    updatePosition();
}

void deleteChar() {
    if (currentPos == 0)
        return;
    currentPos--;
    putConsoleChar(' ');
    currentPos -= 2;
    updatePosition();
}

void newLine() {
    do {
        putConsoleChar(' ');
    } while (currentPos % COLUMNS != 0);
}

void scrollConsole() {
    int last_line_offset = (LINES - 1) * COLUMNS;
    memcpy(gConsole, gConsole + COLUMNS,
        last_line_offset * sizeof(CONSOLE));
    for (int i = (LINES - 1) * COLUMNS; i < MAX_POS; i++){
        gConsole[i].color = 10;
        gConsole[i].c = ' ';
    }
}


void clearConsole()
{
    int i;

    for (i = 0; i < MAX_POS; i++)
    {
        gConsole[i].color = 10;
        gConsole[i].c = ' ';
    }
    cursorMove(0, 0);
    currentPos = 0;
}

//25*80 console
void consoleWrite(char ch)
{
	switch (ch) {
	case TAB:
        putConsoleChar(' ');
        putConsoleChar(' ');
        putConsoleChar(' ');
        putConsoleChar(' ');
		break;
	case ENDL:
        newLine();
		break;
	case BACK_SPACE:
        deleteChar();
		break;
	default:
        putConsoleChar(ch);
		break;
	}
}

void runConsole() {
	char ch;
	while (1) {
		ch = getLastAscii();
        if (ch == '\r')
            break;
		if (ch != 0)
			consoleWrite(ch);
		wait(10);
	}
    clearConsole();
}

void consolePrint(char* str) {
    while (*str)
        consoleWrite(*(str++));
}
