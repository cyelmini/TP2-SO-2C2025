#ifndef _LIB_H
#define _LIB_H

#include <stdint.h>

/**
 * @brief  Escribe 'character' en una direccion de memoria 'length' veces
 * @param  destination: Lugar a escribir
 * @param  character: Elemento a copiar
 * @param  length: Cantidad de veces que se desea escrbir el valor
 */
void *memset(void *destination, int32_t character, uint64_t length);

/**
 * @brief Copia a 'destination' 'length' caracteres de source
 * @param  destination: Direccion de origen
 * @param  source: Fuente
 * @param  length: Cantidad de bytes a copiar
 */
void *memcpy(void *destination, const void *source, uint64_t length);

/**
 * @brief Convierte un entero a string
 * @note  Actualmente no trabaja con negativos
 * @param  n: Numero a convertir
 * @param  buffer: Direccion donde se quiere copiar la respuesta
 * @param  base: Base numerica
 * @return Longitud del string generado
 */
int itoa(uint64_t n, char *buffer, int base);

/**
 * @brief Convierte un string a entero base 10
 * @param  s: String a convertir
 * @param  end: Puntero a la ultima direccion del string
 * @return Entero a partir del string
 */
int strtoi(char *s, char **end);

/**
 * @brief Devuelve el scancode de la ultima tecla presionada
 * @return Scancode
 */
char getKeyPressed();

/**
 * @brief Permite obtener el fabricante del cpu
 * @param  result: Puntero donde se quiere guardar el fabricante del cpu
 * @return Direccion donde se escribio el fabricante
 */
char *cpuVendor(char *result);

/**
 * @brief Devuelve la hora actual en formato de 24hs
 * @param  h: Horas
 * @param  m: Minutos
 * @param  s: Segundos
 */
void getTime(uint8_t *h, uint8_t *m, uint8_t *s);

/**
 * @brief Reproduce un sonido en el speaker
 * @param  freq: Frecuencia del sonido
 */
void startSound(uint64_t freq);

/**
 * @brief Apaga el speaker
 */
void stopSound();

/**
 * @brief Realiza un snapshot de los registros en un momento arbitrario
 */
void saveRegisters();

/**
 * @brief  Detiene la ejecucion del programa hasta que se produzca una interrupcion
 */
void _hlt();

/**
 * @brief Determina si dos cadenas de caracteres son iguales o diferentes
 * @param s1: Cadena de caracteres
 * @param s2: Cadena de caracteres
 * @return Numero positivo si s1 > s2, 0 si son iguales y numero negativo si s1 < s2
 */
int strcmp(const char *s1, const char *s2);

/**
 * @brief Copia los caracteres de la cadena origin en la cadena dest hasta que aparezca un caracter limit
 * @param dest: Cadena de destino
 * @param origin: Cadena de origen
 * @param limit: Caracter de corte
 * @return Longitud de la cadena de destino
 */
int strcpychar(char *dest, const char *origin, char limit);

/**
 * @brief Copia toda la cadena origin en dest, el limite es un '\0'
 * @param dest: Cadena de destino
 * @param origin: Cadena de origen
 * @return Londitud de la cadena de destino
 */
int my_strcpy(char *dest, const char *origin);

/**
 * @brief Cuenta la cantidad de caracteres de una cadena
 * @param s: cadena
 * @return Londitud de la cadena de destino
 */
int my_strlen(const char *s);

extern void callTimerTick();

#endif
