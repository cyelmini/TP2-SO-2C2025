#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "process.h"
#include <stdint.h>

#define MAX_PROCESS 30
#define MIN_QUANTUMS 1
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10

#define NO_PROCESS -1
#define IDLE_PID 0
#define SHELL_PID 1

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct schedulerCDT {
	doubleLinkedListADT processList;
	doubleLinkedListADT readyProcess;
	doubleLinkedListADT blockedProcess;

	int16_t currentPid;

	ProcessContext *currentProcess;

	uint16_t processQty;
	int quantums;
} schedulerCDT;

typedef struct schedulerCDT *schedulerADT;

typedef struct ProcessInfo {
	char *name;
	uint8_t priority;
	char ground;
	uint8_t status;
	int16_t pid;

	uint64_t stackBase;
	uint64_t stackPos;
} ProcessInfo;

/**
 * @brief Inicializa el planificador de procesos
 */
void createScheduler();

/**
 * @brief Realiza el cambio de contexto entre procesos
 * @param prevRSP El valor del RSP del proceso anterior
 * @return El valor del RSP del próximo proceso a ejecutar
 */
uint64_t schedule(uint64_t prevRSP);

/**
 * @brief Crea un nuevo proceso
 * @param rip Dirección de inicio del código del proceso
 * @param args Argumentos del proceso
 * @param argc Cantidad de argumentos
 * @param priority Prioridad del proceso
 * @param fileDescriptors Array de descriptores de archivo
 * @param ground Indica si es proceso de foreground (1) o background (0)
 * @return PID del proceso creado o -1 en caso de error
 */
int16_t createProcess(uint64_t rip, char **args, int argc, uint8_t priority, int16_t fileDescriptors[], char ground);

/**
 * @brief Marca un proceso como listo para ejecutar
 * @param pid ID del proceso a marcar como listo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int64_t setReadyProcess(int16_t pid);

/**
 * @brief Bloquea un proceso
 * @param pid ID del proceso a bloquear
 * @return 0 en caso de éxito, -1 en caso de error
 */
int64_t blockProcess(int16_t pid);

/**
 * @brief Busca un proceso por su PID
 * @param pid ID del proceso a buscar
 * @return Puntero al contexto del proceso o NULL si no se encuentra
 */
ProcessContext *findProcess(int16_t pid);

/**
 * @brief Cede voluntariamente el procesador
 */
void yield();

/**
 * @brief Termina el proceso actual
 * @return 0 en caso de éxito, -1 en caso de error
 */
int64_t killCurrentProcess();

/**
 * @brief Termina un proceso específico
 * @param pid ID del proceso a terminar
 * @return 0 en caso de éxito, -1 en caso de error
 */
int64_t killProcess(int16_t pid);

/**
 * @brief Termina el proceso en foreground actual
 * @return 0 en caso de éxito, -1 en caso de error
 */
int64_t killForegroundProcess();

/**
 * @brief Obtiene el PID del proceso actual
 * @return PID del proceso actual
 */
int16_t getPid();

/**
 * @brief Obtiene el descriptor de archivo correspondiente
 * @param fd Número de descriptor de archivo
 * @return Descriptor de archivo o -1 en caso de error
 */
int64_t getFd(int64_t fd);

/**
 * @brief Obtiene información de todos los procesos
 * @param proccesQty Puntero donde se guardará la cantidad de procesos
 * @return Array de estructuras ProcessInfo con la información de los procesos
 */
ProcessInfo *ps(uint16_t *proccesQty);

/**
 * @brief Copia la información de un proceso a una estructura ProcessInfo
 * @param dest Estructura destino
 * @param src Proceso origen
 * @return 0 en caso de éxito, -1 en caso de error
 */
int16_t copyProcess(ProcessInfo *dest, ProcessContext *src);

#endif