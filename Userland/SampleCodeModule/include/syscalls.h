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

/**
 * @brief Crea un nuevo proceso
 * @param rip Dirección de instrucción de entrada (función a ejecutar)
 * @param args Vector de argumentos para el proceso
 * @param argc Cantidad de argumentos
 * @param priority Prioridad inicial del proceso
 * @param ground 1 = foreground, 0 = background
 * @param fileDescriptors Array con descriptores [stdin, stdout, stderr]
 * @return Identificador del proceso creado (pid) o un código de error
 */
uint64_t sys_createProcess(uint64_t rip, char **args, int argc, uint8_t priority, char ground,
						   int16_t fileDescriptors[]);

/**
 * @brief Obtiene el PID del proceso actual
 * @return PID del proceso que llama
 */
uint64_t sys_getPid();

/**
 * @brief Obtiene información de los procesos del sistema
 * @param processQty Puntero donde se escribirá la cantidad de procesos devueltos
 * @return Puntero a un arreglo de ProcessInfo con la información; NULL si no hay datos
 */
ProcessInfo *sys_processInfo(uint16_t *processQty);

/**
 * @brief Mata un proceso dado su PID
 * @param pid Identificador del proceso a terminar
 * @return Código de resultado (ej. 0 OK, <0 error)
 */
int64_t sys_killProcess(int16_t pid);

/**
 * @brief Cambia la prioridad de un proceso
 * @param pid Identificador del proceso
 * @param priority Nueva prioridad
 * @return Código de resultado (0 OK, <0 error)
 */
int sys_changePriority(int16_t pid, uint8_t priority);

/**
 * @brief Bloquea un proceso (cambia su estado a bloqueado)
 * @param pid Identificador del proceso a bloquear
 * @return Código de resultado (0 OK, <0 error)
 */
int64_t sys_blockProcess(int16_t pid);

/**
 * @brief Marca un proceso como listo (set ready)
 * @param pid Identificador del proceso
 * @return Código de resultado (0 OK, <0 error)
 */
int64_t sys_setReadyProcess(int16_t pid);

/**
 * @brief Cede voluntariamente la CPU al scheduler
 */
void sys_yield();

/**
 * @brief Espera a que un proceso termine
 * @param pid Identificador del proceso a esperar
 * @return Código de resultado (0 OK, <0 error)
 */
int sys_waitProcess(int16_t pid);

/**
 * @brief Termina el proceso actual de forma controlada
 */
void sys_exit();

<<<<<<< Updated upstream
void sys_sleep(uint32_t s);
=======
/**
 * @brief Crea e inicializa un semáforo con un valor inicial
 * @param id Identificador del semáforo (0 a 299)
 * @param initialValue Valor inicial del contador del semáforo
 * @return 0 si éxito, -1 si error
 */
int sys_sem_create(int id, uint32_t initialValue);

/**
 * @brief Verifica si un semáforo está abierto/activo
 * @param id Identificador del semáforo
 * @return 0 si está abierto, -1 si no existe
 */
int sys_sem_open(int id);

/**
 * @brief Operación wait (P) sobre un semáforo - decrementa contador o bloquea
 * @param id Identificador del semáforo
 * @return 0 si éxito, -1 si error
 */
int sys_sem_wait(int id);

/**
 * @brief Operación post (V) sobre un semáforo - incrementa contador o despierta proceso
 * @param id Identificador del semáforo
 * @return 0 si éxito, -1 si error
 */
int sys_sem_post(int id);

/**
 * @brief Destruye un semáforo y libera sus recursos
 * @param id Identificador del semáforo
 * @return 0 si éxito, -1 si error
 */
int sys_sem_destroy(int id);

/**
 * @brief Crea un nuevo pipe para comunicación entre procesos
 * @return File descriptor del pipe creado, o -1 si error
 */
int sys_pipe_create();

/**
 * @brief Lee datos de un pipe (bloqueante hasta que haya datos)
 * @param pipeId Identificador del pipe
 * @param buffer Buffer donde escribir los datos leídos
 * @param count Cantidad de bytes a leer
 * @return Cantidad de bytes leídos, o -1 si error
 */
int sys_pipe_read(int pipeId, char *buffer, int count);

/**
 * @brief Escribe datos en un pipe (bloqueante si no hay espacio)
 * @param pipeId Identificador del pipe
 * @param buffer Buffer con los datos a escribir
 * @param count Cantidad de bytes a escribir
 * @return Cantidad de bytes escritos, o -1 si error
 */
int sys_pipe_write(int pipeId, const char *buffer, int count);

/**
 * @brief Cierra un pipe y libera sus recursos
 * @param pipeId Identificador del pipe
 * @return 0 si éxito, -1 si error
 */
int sys_pipe_close(int pipeId);
>>>>>>> Stashed changes

#endif