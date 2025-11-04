#include "../../include/keyboard.h"
#include "../../include/lib.h"
#include "../../include/time.h"
#include "../../include/video.h"
#include "../../include/scheduler.h"
#include "../../include/semaphore.h"
#include <stdint.h>

#define BUFFER_CAPACITY 10 /* Longitud maxima del vector _buffer */
#define HOTKEY 29		   /* Scancode para el snapshot de registros (Ctrl) */
#define LCTRL 29		   /* Left Control scancode */
#define LSHIFT 42		   /* Left Shift scancode */
#define C_KEY 0x2E		   /* C key scancode */
#define D_KEY 0x20		   /* D key scancode */
#define RELEASED 0x80	   /* Mascara para detectar tecla liberada */
#define SHIFTED 0x80	   /* Flag para indicar que se uso shift */
#define EOF (-1)		   /* End of File character */

static uint8_t _bufferStart = 0;			   /* Indice del comienzo de la cola */
static char _bufferSize = 0;				   /* Longitud de la cola */
static uint8_t _buffer[BUFFER_CAPACITY] = {0}; /* Vector ciclico que guarda las teclas
												* que se van leyendo del teclado */
static uint8_t _ctrl = 0;					   /* Flag para detectar si ctrl esta presionado */
static uint8_t _shift = 0;					   /* Flag para detectar si shift esta presionado */

static const char charHexMap[256] =			   /* Mapa de scancode a ASCII */
	{0,	  0,   '1', '2', '3', '4', '5',	 '6', '7', '8', '9', '0', '-', '=', '\b', ' ', 'q', 'w', 'e',  'r', 't', 'y',
	 'u', 'i', 'o', 'p', '[', ']', '\n', 0,	  'a', 's', 'd', 'f', 'g', 'h', 'j',  'k', 'l', ';', '\'', 0,	0,	 '\\',
	 'z', 'x', 'c', 'v', 'b', 'n', 'm',	 ',', '.', '/', 0,	 '*', 0,   ' ', 0,	  0,   0,	0,	 0,	   0};

static const char charHexMapShift[] =		   /* Mapa de scancode con Shift a ASCII */
	{0,	  0,   '!', '@', '#', '$', '%',	 '^', '&', '*', '(', ')', '_', '+', '\b', ' ', 'Q', 'W', 'E',  'R', 'T', 'Y',
	 'U', 'I', 'O', 'P', '{', '}', '\n', 0,	  'A', 'S', 'D', 'F', 'G', 'H', 'J',  'K', 'L', ':', '"',  0,	0,	 '|',
	 'Z', 'X', 'C', 'V', 'B', 'N', 'M',	 '<', '>', '?', 0,	 '*', 0,   ' ', 0,	  0,   0,	0,	 0,	   0};

static void writeKey(char key);

/**
 * @brief  Obtiene el indice del elemento en la cola dado un corrimiento
 * @param  offset: corrimiento
 * @return Indice del elemento en la cola
 */
static int getBufferIndex(int offset) {
	return (_bufferStart + offset) % (BUFFER_CAPACITY);
}

void initializeKeyboardDriver() {
	sem_create(KEYBOARD_SEM_ID, 0);
}

void keyboardHandler() {
	uint8_t key = getKeyPressed();
	
	if (!(key & RELEASED)) {
		if (key == LCTRL) {
			_ctrl = 1;
		} else if (key == LSHIFT) {
			_shift = 1;
		} else if (_ctrl) {
			if (key == C_KEY) {
				_bufferStart = _bufferSize = 0;
				killForegroundProcess();
			} else if (key == D_KEY && _bufferSize < BUFFER_CAPACITY - 1) {
				print("^D");
				printNewline();
				writeKey(EOF);
			} else if (key == HOTKEY) {
				saveRegisters();
			}
		} else if (_bufferSize < BUFFER_CAPACITY - 1) {
			if (_shift) {
				key = SHIFTED | key;
			}
			writeKey(key);
		}
	} else {
		if (key == (LCTRL | RELEASED)) {
			_ctrl = 0;
		} else if (key == (LSHIFT | RELEASED)) {
			_shift = 0;
		}
	}
}

char getScancode() {
	if (_bufferSize > 0) {
		char c = _buffer[getBufferIndex(0)];
		_bufferStart = getBufferIndex(1);
		_bufferSize--;
		return c;
	}
	return 0;
}

char getAscii() {
	int scanCode;
	
	sem_wait(KEYBOARD_SEM_ID);
	
	scanCode = getScancode();
	
	if (scanCode == EOF) {
		return EOF;
	}
	if (SHIFTED & scanCode) {
		scanCode &= 0x7F; 
		return charHexMapShift[(int) scanCode];
	}
	return charHexMap[(int) scanCode];
}

static void writeKey(char key) {
	if (((key & 0x7F) < sizeof(charHexMap) && charHexMap[key & 0x7F] != 0) || (int)key == EOF) {
		_buffer[getBufferIndex(_bufferSize)] = key;
		_bufferSize++;
		sem_post(KEYBOARD_SEM_ID);
	}
}