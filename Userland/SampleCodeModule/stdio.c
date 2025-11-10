// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include <stdarg.h>
#include <stdint.h>

#define CURSOR_FREQ 10 /* Frecuencia en Ticks del dibujo del cursor*/

/**
 * @brief Funcion auxiliar para printf y printfc
 * @note La cantidad de parametros no es fija
 * @param fmt Formato de lo que se desea escribir de STDOUT
 * @param args lista de argumentos
 */
static void vprintf(char *fmt, va_list args);

void putchar(char c) {
	sys_write(STDOUT, c);
}

void putcharErr(char c) {
	sys_write(STDERR, c);
}

void puts(const char *s) {
	while (*s)
		putchar(*s++);
}

void printErr(const char *s) {
	while (*s)
		putcharErr(*s++);
}

int getchar() {
	unsigned char c = sys_read(STDIN);
	// Si es 0xFF (255), es EOF (-1)
	if (c == 0xFF) {
		return -1;
	}
	return (int)c;
}

char getScanCode() {
	return sys_read(KBDIN);
}

void printf(char *fmt, ...) {
	va_list v;
	va_start(v, fmt);
	vprintf(fmt, v);
	va_end(v);
}

static void vprintf(char *fmt, va_list args) {
    char buffer[MAX_CHARS] = {0};
    char *fmtPtr = fmt;
    while (*fmtPtr) {
        if (*fmtPtr == '%') {
            fmtPtr++;
            int dx = strtoi(fmtPtr, &fmtPtr);
            int len;

            switch (*fmtPtr) {
                case 'c': {
                    putchar(va_arg(args, int));
                } break;

                case 'd': { // decimal (con signo, pero imprimimos igual con itoa)
                    len = itoa(va_arg(args, uint64_t), buffer, 10);
                    printNChars('0', dx - len);
                    puts(buffer);
                } break;

                case 'u': { // unsigned
                    len = itoa(va_arg(args, uint64_t), buffer, 10);
                    printNChars('0', dx - len);
                    puts(buffer);
                } break;

                case 'x': {
                    len = itoa(va_arg(args, uint64_t), buffer, 16);
                    printNChars('0', dx - len);
                    puts(buffer);
                } break;

                case 's': {
                    printNChars(' ', dx); 
                    puts((char *) va_arg(args, char *));
                } break;

                default: {
                    // si llega un especificador desconocido, imprimimos el carácter tal cual
                    putchar(*fmtPtr);
                } break;
            }
        } else {
            putchar(*fmtPtr);
        }
        fmtPtr++;
    }
}

void printfc(Color color, char *fmt, ...) {
	Color prevColor = sys_getFontColor();
	sys_setFontColor(color.r, color.g, color.b);
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	sys_setFontColor(prevColor.r, prevColor.g, prevColor.b);
}

void printNChars(char c, int n) {
	for (int i = 0; i < n; i++)
		putchar(c);
}

int scanf(char *fmt, ...) {
	va_list v;
	va_start(v, fmt);
	char c;
	int ticks = sys_getTicks();
	int cursorTicks = 0;
	char cursorDrawn = 0;
	char buffer[MAX_CHARS];
	uint64_t bIdx = 0;
	while ((c = getchar()) != '\n' && bIdx < MAX_CHARS - 1) {
		cursorTicks = sys_getTicks() - ticks;
		if (cursorTicks > CURSOR_FREQ) {
			ticks = sys_getTicks();
			cursorTicks = 0;
			if (cursorDrawn)
				putchar('\b');
			else
				putchar('_');
			cursorDrawn = !cursorDrawn;
		}
		if (c != 0) {
			if (cursorDrawn) {
				putchar('\b');
				cursorDrawn = !cursorDrawn;
			}
			if (c != '\b') {
				buffer[bIdx++] = c;
				putchar(c);
			}
			else if (bIdx > 0) {
				bIdx--;
				putchar(c);
			}
		}
	}
	if (cursorDrawn)
		putchar('\b');
	putchar('\n');
	buffer[bIdx] = 0;
	char *fmtPtr = fmt;
	char *end;
	bIdx = 0;

	int qtyParams = 0;
	while (*fmtPtr && buffer[bIdx] && bIdx < MAX_CHARS) {
		if (*fmtPtr == '%') {
			fmtPtr++;
			switch (*fmtPtr) {
				case 'c':
					*(char *) va_arg(v, char *) = buffer[bIdx];
					end = &buffer[bIdx] + 1;
					break;
				case 'd':
					*(int *) va_arg(v, int *) = strtoi(&buffer[bIdx], &end);
					break;
				case 'x':
					*(int *) va_arg(v, int *) = strtoh(&buffer[bIdx], &end);
					break;
				case 's':
					end = &buffer[bIdx] + strcpychar((char *) va_arg(v, char *), &buffer[bIdx], ' ');
					break;
			}
			bIdx += end - &buffer[bIdx];
			qtyParams++;
		}
		else if (*fmtPtr == buffer[bIdx]) {
			bIdx++;
		}
		else {
			printErr("Error!!!");
		}
		fmtPtr++;
	}
	buffer[bIdx - 1] = 0;
	va_end(v);
	return qtyParams;
}

int read_line(char *dst, int maxlen) {
    if (dst == NULL || maxlen <= 1) return 0;

    int len = 0;

    for (;;) {
        int c = getchar();

        if (c == 0) {                
            sys_yield();             
            continue;
        }

        if (c == '\n') {             
            putchar('\n');          
            break;
        }

        if (c == '\b') {             
            if (len > 0) {
                putchar('\b');
                putchar(' ');
                putchar('\b');
                len--;
            }
            continue;
        }

        if (len < maxlen - 1) {      
            dst[len++] = (char)c;
            putchar((char)c);        
        }
    }
    dst[len] = '\0';
    return len;
}

static char *_regNames[] = {"RAX", "RBX", "RCX", "RDX", "RBP", "RDI", "RSI", "R8",
							"R9",  "R10", "R11", "R12", "R13", "R14", "R15"};
void printRegisters(const uint64_t *rsp) {
	for (int i = 0; i < sizeof(_regNames) / sizeof(char *); i++)
		printf("%s: 0x%x\n", _regNames[i], rsp[sizeof(_regNames) / sizeof(char *) - i - 1]);
}