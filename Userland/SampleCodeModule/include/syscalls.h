#ifndef _SYSCALLS_H
#define _SYSCALLS_H

#include "color.h"
#include "shared.h"
#include "stddef.h"
#include "stdint.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/**
 * @brief Escribe a partir del descriptor recibido un caracter
 * @param fd: FileDescriptor (STDOUT | STDERR)
 * @param c: Caracter a escribir
 */
void sys_write(int fd, char c);

/**
 * @brief Lee un byte a partir del descriptor recibido
 * @param fd: FileDescriptor (STDIN | KBDIN)
 * @return Byte leido
 */
uint8_t sys_read(int fd);

/**
 * @brief Devuelve la hora expresada en segundos
 * @return Hora expresada en segundos
 */
uint32_t sys_getSeconds();
uint32_t sys_getMinutes();
uint32_t sys_getHours();

/**
 * @brief Pone todos los pixeles de la pantalla en negro y limpia el buffer de video
 */
void sys_clear(void);

/**
 * @brief
 * @param regarr: Vector donde se llena la informacion de los registros
 * @return Puntero a la informacion de los registros
 */
uint64_t *sys_getInfoReg(uint64_t *regarr);

/**
 * @brief Cambia el tamaño de la fuente
 * @param size: (1|2|3)
 */
void sys_setFontSize(uint8_t size);

/**
 * @brief Devuelve las dimensiones de la pantalla
 * @return 32 bits menos significativos el ancho, 32 el alto
 */
uint32_t sys_getScreenResolution();

/**
 * @brief Devuelve la cantidad de ticks actuales
 * @return Cantidad de ticks
 */
uint64_t sys_getTicks();

/**
 * @brief Llena un vector con 32 bytes de informacion a partir de una direccion de memoria en hexa
 * @param pos: Direccion de memoria a partir de la cual se llena el vector
 * @param vec: Vector en el cual se llena la informacion
 */
void sys_getMemory(uint64_t pos, uint8_t *vec);

/**
 * @brief Ejecuta una excepcion de Invalid Opcode Exception
 */
void sys_kaboom();

/**
 * @brief Establece un color de fuente
 * @param r: Color rojo
 * @param g: Color verde
 * @param b: Color azul
 */
void sys_setFontColor(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Retorna el color de fuente que se esta usando actualmente
 * @return Color
 */
Color sys_getFontColor();

/**
 * @brief Reserva memoria dinámica dentro del heap administrado
 *
 * @param size Cantidad de bytes a reservar
 * @return void* Puntero al bloque asignado, o NULL si no hay espacio
 */
void *sys_mm_alloc(size_t size);

/**
 * @brief Libera un bloque previamente asignado
 *
 * @param ptr Puntero devuelto por mm_alloc(), ignora NULL
 */
void sys_mm_free(void *const restrict ptr);

/**
 * @brief Obtiene informacion del heap administrado
 *
 * @param info Puntero donde se escribirá la información del heap
 */
void sys_mm_info(mem_t *info);

uint64_t sys_createProcess(uint64_t rip, char **args, int argc, uint8_t priority, char ground, int16_t fileDescriptors[]);

uint64_t sys_getPid();

ProcessInfo sys_processInfo(uint16_t *processQty);

int64_t sys_killProcess(int16_t pid);

int sys_changePriority(int16_t pid);

int64_t sys_blockProcess(int16_t pid);

int64_t sys_setReadyProcess(int16_t pid);

void sys_yield();

int sys_waitProcess(int16_t pid);

#endif