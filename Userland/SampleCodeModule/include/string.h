#ifndef _STRING_H
#define _STRING_H

#include <stdint.h>

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
 * @brief Copia toda la cadena origin en dest (analogo a strcpychar pero limit es un '\0')
 * @param dest: Cadena de destino
 * @param origin: Cadena de origen
 * @return Londitud de la cadena de destino
 */
int strcpy(char *dest, const char *origin);

/**
 * @brief Calcula la longitud de una cadena de caracteres
 * @param str: Cadena de caracteres
 * @return Longitud de la cadena (sin incluir el caracter nulo '\0')
 */
int strlen(const char *str);

/*
 * Busca el primer caracter 'c' en la cadena 's' y devuelve
 * un puntero a su posicion o NULL si no se encuentra.
 */
char *find_char(const char *s, char c);

int str_in_list(const char *needle, char *list[], int list_len);

uint32_t str_to_uint32(char *str);

#endif
