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



char comenzi[][20] = {
    "sleep", //sleep second
    "read_disk", //read_disk lba count -> print content
    "all", //allocate_memory nb_bytes -> memory_address
    "wr", //write_memory memory_address number -> success or error
    "rd", //read_memory memory_address -> qword from memory
    "free",  //free memory_address
    "int", //int int_nb[0-15]
    "time",
    "print",
    "help"
};

char helpString[] =
"sleep second -> wait(seconds)\n"
"read_disk lba count -> print content\n"
"all nb_bytes -> memory_address\n"
"wr memory_address number -> write qword success or error\n"
"rd memory_address -> qword from memory\n"
"free memory_address -> free space\n"
"int int_nb -> launch int_nb interrupt\n"
"time -> show time 1/0\n"
"print -> print memory descriptors\n"
"help -> show help\n";


char command[1000];
char *index, *p1, *p2;
void fill_params() {
    char* p = command;
    while (*p && *p != ' ') p++;
    if (*p == 0) {
        p1 = p;
        p2 = p;
        return;
    }
    p++;
    p1 = p;
    while (*p && *p != ' ') p++;
    if (*p == 0) {
        p2 = p;
        return;
    }
    *p = 0;
    p++;
    p2 = p;
}

int my_strncmp(char* s1, char* s2) {
    int i = 0;
    while (s1[i]) {
        if (s2[i] == 0)
            return 1;
        if (s1[i] != s2[i])
            return 1;
        i++;
    }
    return 0;
}

int myAtoi(char* str)
{
    int res = 0; // Initialize result 

    // Iterate through all characters of input string and 
    // update result 
    for (int i = 0; str[i] != '\0'; ++i)
        res = res * 10 + str[i] - '0';

    // return result. 
    return res;
}

BYTE buff[10000];
void process_command() {
    int i, size = sizeof(comenzi)/sizeof(comenzi[0]);
    for (i = 0; i < size; i++) {
        if (0 == my_strncmp(comenzi[i], command)) {
            break;
        }
    }
    //PL(command);
    fill_params();
    index = command;
    /*
    consolePrint("[D] Param 1:");
    consolePrint(p1);
    consolePrint("\n[D] Param 2:");
    consolePrint(p2);
    consolePrint("\n");
    */
    QWORD lba, count, n;
    switch (i) {
    case 0://"sleep", //sleep second
        n = myAtoi(p1);
        STR("Waiting for ");
        NB(n); STR("sec\n");
        wait(n * 1000);
        break;
    case 1://"read_disk", //read_disk lba count -> print contentq
        lba = myAtoi(p1);
        count = myAtoi(p2);
        STR("Reading "); NB(count); STR(" sectors form address "); NB(lba); STR("\n");
        if (count * 512 > 10000)
        {
            STR("To many sector to read");
            break;
        }
        memset(buff, 0, sizeof(buff));
        if (0 != readDiskSector(lba, count, (WORD*)buff)) {
            STR("ERROR\n");
        }
        else {
            buff[count * 512] = 0;
            STR("From disk: \n");
            STR(buff);
            STR("\n");
        }
        break;
    case 2://"allocate_memory", //allocate_memory nb_bytes -> memory_address
        n = myAtoi(p1);
        lba = my_malloc(n);
        STR("Allocated "); NB(n); STR(" bytes at address "); NB(lba); STR("\n");
        break;
    case 3://"write_memory", //write_memory memory_address number -> success or error
        lba = myAtoi(p1);
        n = myAtoi(p2);
        if (writeQ(lba, n)) {
            STR("Written "); NB(n); STR(" to address "); NB(lba); STR("\n");
        }
        else
            STR("ERROR\n");
        break;
    case 4://"read_memory", //read_memory memory_address -> qword from memory
        lba = myAtoi(p1);
        if (readQ(lba, &n)) {
            STR("Read "); NB(n); STR(" from address "); NB(lba); STR("\n");
        }
        else
            STR("ERROR\n");
        break;
    case 5://"free"  //free memory_address
        lba = myAtoi(p1);
        my_free(lba);
        STR("Free "); NB(lba); STR("\n");
        break;
    case 6://"int" //int int_nb
        n = myAtoi(p1);
        __launchInterrupt(n);
        break;
    case 7://"time 1/0"
        n = myAtoi(p1);
        enableTimer(n);
        if (n)
            STR("Time enabled\n");
        else
            STR("Time disabled\n");
        break;
    case 8://"time"
        printDesc();
        break;
    case 9://"time"
        consolePrint(helpString);
        break;
    default:
        consolePrint("Command not found\n");
        break;
    }
}

void runConsole() {
	char ch;
    index = command;
    consolePrint("Welcome to console\n");
    consoleWrite('>');
	while (1) {
		ch = getLastAscii();
        if (ch == '\r')
            break;
        if (ch == BACK_SPACE) {
            *(index--) = 0;
            consoleWrite(ch);
            continue;
        }
        if (ch == '\n') {
            consoleWrite(ch);
            *index = 0;
            process_command();
            consoleWrite('>');
            continue;
        }
        if (ch != 0) {
            *(index++) = ch;
            consoleWrite(ch);
        }
		wait(10);
	}
    clearConsole();
}

void consolePrint(char* str) {
    while (*str)
        consoleWrite(*(str++));
}

void _printNb(QWORD nb, int base, char* buff2) {
    char buff[20];
    itoa(nb, buff, base);
    //REvers nb
    char* p;
    for (p = buff; *p; p++);
    p--;
    char* q = buff2;
    while (p >= buff)
        *(q++) = *(p--);
    *q = 0;
}

void PrintNb(QWORD nb, int base) {
    char buff2[20];
    _printNb(nb, base, buff2);
    consolePrint(buff2);
    //__magic();
}