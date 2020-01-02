#include "main.h"
#include "keyboard.h"
#include "screen.h"

#define KEYBOARD_ENCODER				0x60
#define KEYBOARD_CONTROLLER				0x64
#define KEY_IN_BUFF_STATUS_BIT			(1 << 0)
#define KEY_OUT_BUFF_STATUS_BIT			(1 << 1)

#define KYBRD_ENC_CMD_SET_LED			0xED

#define BRAKE_CODE_BIT					(1 << 7)

#define INVALID_SCANCODE				0

static enum KEYCODE _kkybrd_scancode_std[] = {

	//! key			scancode
	KEY_UNKNOWN,	//0
	KEY_ESCAPE,		//1
	KEY_1,			//2
	KEY_2,			//3
	KEY_3,			//4
	KEY_4,			//5
	KEY_5,			//6
	KEY_6,			//7
	KEY_7,			//8
	KEY_8,			//9
	KEY_9,			//0xa
	KEY_0,			//0xb
	KEY_MINUS,		//0xc
	KEY_EQUAL,		//0xd
	KEY_BACKSPACE,	//0xe
	KEY_TAB,		//0xf
	KEY_Q,			//0x10
	KEY_W,			//0x11
	KEY_E,			//0x12
	KEY_R,			//0x13
	KEY_T,			//0x14
	KEY_Y,			//0x15
	KEY_U,			//0x16
	KEY_I,			//0x17
	KEY_O,			//0x18
	KEY_P,			//0x19
	KEY_LEFTBRACKET,//0x1a
	KEY_RIGHTBRACKET,//0x1b
	KEY_RETURN,		//0x1c
	KEY_LCTRL,		//0x1d
	KEY_A,			//0x1e
	KEY_S,			//0x1f
	KEY_D,			//0x20
	KEY_F,			//0x21
	KEY_G,			//0x22
	KEY_H,			//0x23
	KEY_J,			//0x24
	KEY_K,			//0x25
	KEY_L,			//0x26
	KEY_SEMICOLON,	//0x27
	KEY_QUOTE,		//0x28
	KEY_GRAVE,		//0x29
	KEY_LSHIFT,		//0x2a
	KEY_BACKSLASH,	//0x2b
	KEY_Z,			//0x2c
	KEY_X,			//0x2d
	KEY_C,			//0x2e
	KEY_V,			//0x2f
	KEY_B,			//0x30
	KEY_N,			//0x31
	KEY_M,			//0x32
	KEY_COMMA,		//0x33
	KEY_DOT,		//0x34
	KEY_SLASH,		//0x35
	KEY_RSHIFT,		//0x36
	KEY_KP_ASTERISK,//0x37
	KEY_RALT,		//0x38
	KEY_SPACE,		//0x39
	KEY_CAPSLOCK,	//0x3a
	KEY_F1,			//0x3b
	KEY_F2,			//0x3c
	KEY_F3,			//0x3d
	KEY_F4,			//0x3e
	KEY_F5,			//0x3f
	KEY_F6,			//0x40
	KEY_F7,			//0x41
	KEY_F8,			//0x42
	KEY_F9,			//0x43
	KEY_F10,		//0x44
	KEY_KP_NUMLOCK,	//0x45
	KEY_SCROLLLOCK,	//0x46
	KEY_HOME,		//0x47
	KEY_KP_8,		//0x48	//keypad up arrow
	KEY_PAGEUP,		//0x49
	KEY_KP_2,		//0x50	//keypad down arrow
	KEY_KP_3,		//0x51	//keypad page down
	KEY_KP_0,		//0x52	//keypad insert key
	KEY_KP_DECIMAL,	//0x53	//keypad delete key
	KEY_UNKNOWN,	//0x54
	KEY_UNKNOWN,	//0x55
	KEY_UNKNOWN,	//0x56
	KEY_F11,		//0x57
	KEY_F12			//0x58
};

BYTE lastCode = INVALID_SCANCODE;
BYTE _shift, _alt, _ctrl;
BYTE _numlock, _capslock, _scrolllock;

void keyHandler() {
	BYTE code;
	if (!(__inbyte(KEYBOARD_CONTROLLER) & KEY_OUT_BUFF_STATUS_BIT)) {
		code = __inbyte(KEYBOARD_ENCODER);
		if (code & BRAKE_CODE_BIT) {	//test bit 7
			//! covert the break code into its make code equivelant
			code -= 0x80;
			//! grab the key
			int key = _kkybrd_scancode_std[code];
			//! test if a special key has been released & set it
			switch (key) {
				case KEY_LCTRL:
				case KEY_RCTRL:
					_ctrl = 0;
					break;
				case KEY_LSHIFT:
				case KEY_RSHIFT:
					_shift = 0;
					break;
				case KEY_LALT:
				case KEY_RALT:
					_alt = 0;
					break;
			}
		}
		else {
			//! this is a make code - set the scan code
			lastCode = code;
			//! grab the key
			int key = _kkybrd_scancode_std[code];
			//! test if user is holding down any special keys & set it
			switch (key) {
			case KEY_LCTRL:
			case KEY_RCTRL:
				_ctrl = 1;
				break;

			case KEY_LSHIFT:
			case KEY_RSHIFT:
				_shift = 1;
				break;

			case KEY_LALT:
			case KEY_RALT:
				_alt = 1;
				break;

			case KEY_CAPSLOCK:
				_capslock = (_capslock) ? 0 : 1;
				break;
			case KEY_KP_NUMLOCK:
				_numlock = (_numlock) ? 0 : 1;
				break;

			case KEY_SCROLLLOCK:
				_scrolllock = (_scrolllock) ? 0 : 1;
				break;
			}
		}
		//PrintNb((BYTE)code, 16);
	}
}

enum KEYCODE getLastKey() {
	if (lastCode == INVALID_SCANCODE)
		return KEY_UNKNOWN;
	enum KEYCODE key = _kkybrd_scancode_std[lastCode];
	lastCode = INVALID_SCANCODE;
	return key;
}

char kkybrd_key_to_ascii(enum KEYCODE key) {
	if (key == KEY_UNKNOWN)
		return 0;
	//! insure key is an ascii character
	if (isascii(key)) {
		//! if shift key is down or caps lock is on, make the key uppercase
		if (_shift || _capslock)
			if (key >= 'a' && key <= 'z')
				key -= 32;
		if (_shift && !_capslock)
			if (key >= '0' && key <= '9')
				switch (key) {

				case '0':
					key = KEY_RIGHTPARENTHESIS;
					break;
				case '1':
					key = KEY_EXCLAMATION;
					break;
				case '2':
					key = KEY_AT;
					break;
				case '3':
					key = KEY_HASH;
					break;
				case '4':
					key = KEY_EURO;
					break;
				case '5':
					key = KEY_PERCENT;
					break;
				case '6':
					key = KEY_CARRET;
					break;
				case '7':
					key = KEY_AMPERSAND;
					break;
				case '8':
					key = KEY_ASTERISK;
					break;
				case '9':
					key = KEY_LEFTPARENTHESIS;
					break;
				}
			else {

				switch (key) {
				case KEY_COMMA:
					key = KEY_LESS;
					break;

				case KEY_DOT:
					key = KEY_GREATER;
					break;

				case KEY_SLASH:
					key = KEY_QUESTION;
					break;

				case KEY_SEMICOLON:
					key = KEY_COLON;
					break;

				case KEY_QUOTE:
					key = KEY_QUOTEDOUBLE;
					break;

				case KEY_LEFTBRACKET:
					key = KEY_LEFTCURL;
					break;

				case KEY_RIGHTBRACKET:
					key = KEY_RIGHTCURL;
					break;

				case KEY_GRAVE:
					key = KEY_TILDE;
					break;

				case KEY_MINUS:
					key = KEY_UNDERSCORE;
					break;

				case KEY_EQUAL:
					key = KEY_PLUS;
					break;

				case KEY_BACKSLASH:
					key = KEY_BAR;
					break;
				}
			}

		//! return the key
		return key;
	}

	//! scan code != a valid ascii char so no convertion is possible
	return 0;
}

char getLastAscii() {
	//__magic();
	return (kkybrd_key_to_ascii(getLastKey()));
}

void testKeyboard() {
	char str[2] = { 0, 0 }, ch;
	while (1) {
		//__magic();
		ch = getLastAscii();
		if (ch == '\r')
			break;
		if (ch != 0) {
			//__magic();
			str[0] = ch;
			Print(str);
		}
		wait(10);
	}
}