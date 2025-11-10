#ifndef PROCESS_H
#define PROCESS_H

#include "doubleLinkedList.h"
#include <stdint.h>

#define CANT_FILE_DESCRIPTORS 3
#define STACK_SIZE 4096

typedef enum { READY, RUNNING, BLOCKED, TERMINATED } ProcessState;

typedef struct ProcessContext {
	char *name;
	uint8_t priority;
	int16_t pid;
	int16_t parentPid;

	uint64_t stackBase;
	uint64_t stackPos;

	ProcessState status;
	char ground; // 0 1

	char **argv;
	int argc;
	uint64_t rip;
	int16_t fileDescriptors[CANT_FILE_DESCRIPTORS];

	doubleLinkedListADT waitingList;
} ProcessContext;

/**
 * @brief Inicializa un nuevo proceso con los parámetros especificados
 * @param process Puntero a la estructura ProcessContext a inicializar
 * @param pid ID del proceso
 * @param args Argumentos del proceso
 * @param argc Cantidad de argumentos
 * @param priority Prioridad del proceso
 * @param rip Dirección de inicio del código del proceso
 * @param ground Indica si es proceso de foreground (1) o background (0)
 * @param fileDescriptors Array con los descriptores de archivo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int initializeProcess(ProcessContext *process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip,
					  char ground, int16_t fileDescriptors[]);

/**
 * @brief Libera los recursos asociados a un proceso
 * @param pcb Puntero al Process Control Block a liberar
 */
void freeProcess(ProcessContext *pcb);

/**
 * @brief Bloquea el proceso actual hasta que el proceso especificado termine
 * @param pid ID del proceso a esperar
 * @return 0 en caso de éxito, -1 en caso de error
 */
int waitProcess(int16_t pid);

/**
 * @brief Cambia los descriptores de archivo de un proceso
 * @param pid ID del proceso a modificar
 * @param fileDescriptors Nuevos descriptores de archivo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int changeFileDescriptors(int16_t pid, int16_t fileDescriptors[]);

/**
 * @brief Cambia la prioridad de un proceso
 * @param pid ID del proceso a modificar
 * @param priority Nueva prioridad
 * @return 0 en caso de éxito, -1 en caso de error
 */
int changePriority(int16_t pid, uint8_t priority);

/**
 * @brief Configura el frame de la pila para un nuevo proceso
 * @param stackBase Dirección base de la pila
 * @param code Dirección del código a ejecutar
 * @param argc Cantidad de argumentos
 * @param args Array de argumentos
 * @return Dirección del tope de la pila configurada
 */
extern uint64_t setupStackFrame(uint64_t stackBase, uint64_t code, int argc, char *args[]);

#endif // PROCESS_H