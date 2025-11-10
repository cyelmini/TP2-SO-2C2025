#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define KEYBOARD_SEM_ID 0 /* ID del semáforo para sincronización de I/O del teclado */

/* Inicializa el driver del teclado (crea el semáforo de I/O) */
void initializeKeyboardDriver();

/* Handler de interrupciones de teclado */
void keyboardHandler();

/* Devuelve el valor ascii del ultimo caracter en el buffer de teclado */
char getAscii();

/* Devuelve el scancode del ultimo caracter en el buffer de teclado */
char getScancode();

#endif
