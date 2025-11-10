#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include "doubleLinkedList.h"
#include <stdint.h>

#define NUM_SEMS 128

typedef struct semaphore_t {
	doubleLinkedListADT waitQueue;
	uint32_t counter;
	uint8_t spinlock;
	uint8_t active;
} semaphore_t;

typedef struct SemaphoreCDT *SemaphoreADT;

typedef struct SemaphoreCDT {
	semaphore_t semaphores[NUM_SEMS];
} SemaphoreCDT;

/**
 * @brief Inicializa el gestor de semáforos
 * @return Handle al gestor de semáforos o NULL en caso de error
 */
SemaphoreADT initSemaphoreManager();

/**
 * @brief Crea un nuevo semáforo
 * @param id Identificador único del semáforo
 * @param initialValue Valor inicial del contador del semáforo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sem_create(int id, uint32_t initialValue);

/**
 * @brief Abre un semáforo existente
 * @param id Identificador del semáforo a abrir
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sem_open(int id);

/**
 * @brief Destruye un semáforo
 * @param id Identificador del semáforo a destruir
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sem_destroy(int id);

/**
 * @brief Realiza la operación wait en un semáforo
 * @param id Identificador del semáforo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sem_wait(int id);

/**
 * @brief Realiza la operación post en un semáforo
 * @param id Identificador del semáforo
 * @return 0 en caso de éxito, -1 en caso de error
 */
int sem_post(int id);

/**
 * @brief Adquiere un spinlock
 * @param lock Puntero al spinlock a adquirir
 */
extern void acquire(uint8_t *lock);

/**
 * @brief Libera un spinlock
 * @param lock Puntero al spinlock a liberar
 */
extern void release(uint8_t *lock);

#endif
