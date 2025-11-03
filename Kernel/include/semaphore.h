#ifndef SEMAPHORES_H
#define SEMAPHORES_H

#include <stdint.h>
#include "doubleLinkedList.h"

#define NUM_SEMS 128            

typedef struct semaphore_t {
    doubleLinkedListADT waitQueue; // cola de procesos en espera
    uint32_t counter;            // cuantos procesos pueden entrar hasta que el semaforo se bloquee
    uint8_t spinlock;            // spinlock para operaciones atómicas
    uint8_t active;              // 0 = no existe o fue destruido, 1 = habilitado para uso 
} semaphore_t;

typedef struct SemaphoreCDT *SemaphoreADT;

typedef struct SemaphoreCDT {
    semaphore_t semaphores[NUM_SEMS]; // vector de semaforos 
} SemaphoreCDT;

SemaphoreADT initSemaphoreManager();

int sem_create(int id, uint32_t initialValue);

int sem_open(int id);

int sem_destroy(int id);

int sem_wait(int id);

int sem_post(int id);

extern void acquire(uint8_t *lock);

extern void release(uint8_t *lock);

#endif
