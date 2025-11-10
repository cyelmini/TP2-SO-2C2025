// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "../../include/memoryManagement.h"
#include "../../include/process.h"
#include "../../include/scheduler.h"
#include "../../include/semaphore.h"
#include <stddef.h>

static SemaphoreADT semManager = NULL;
static int isValidSemId(int id);

SemaphoreADT initSemaphoreManager() {
	semManager = mm_alloc(sizeof(SemaphoreCDT));
	if (semManager == NULL) {
		return NULL;
	}

	for (int i = 0; i < NUM_SEMS; i++) {
		semManager->semaphores[i].counter = 0;
		semManager->semaphores[i].spinlock = 0;
		semManager->semaphores[i].active = 0;
		semManager->semaphores[i].waitQueue = NULL;
	}
	return semManager;
}

int sem_create(int id, uint32_t initialValue) {
	if (isValidSemId(id) == -1) {
		return -1;
	}

	if (!semManager->semaphores[id].active) {
		semManager->semaphores[id].counter = initialValue;
		semManager->semaphores[id].spinlock = 0;
		semManager->semaphores[id].active = 1;
		semManager->semaphores[id].waitQueue = createDoubleLinkedListADT();

		if (semManager->semaphores[id].waitQueue == NULL) {
			return -1;
		}

		return 0;
	}
	return -1;
}

int sem_open(int id) {
	if (isValidSemId(id) == -1) {
		return -1;
	}
	if (semManager->semaphores[id].active) {
		return 0;
	}
	return -1;
}

int sem_destroy(int id) {
	if (isValidSemId(id) == -1) {
		return -1;
	}

	if (!semManager->semaphores[id].active) {
		return -1;
	}

	semaphore_t *sem = &semManager->semaphores[id];

	freeLinkedListADT(sem->waitQueue);
	sem->active = 0;
	sem->counter = 0;
	sem->spinlock = 0;
	sem->waitQueue = NULL;

	return 0;
}

int sem_wait(int id) {
	if (isValidSemId(id) == -1) {
		return -1;
	}

	semaphore_t *sem = &semManager->semaphores[id];

	acquire(&sem->spinlock);

	if (sem->counter > 0) { // todavia pueden entrar procesos
		sem->counter--;
		release(&sem->spinlock);
		return 0;
	}

	// si el contador está en 0 hay que bloquear el proceso
	int16_t currentPid = getPid();
	int16_t *pid = (int16_t *) mm_alloc(sizeof(int16_t));
	if (pid == NULL) {
		release(&sem->spinlock);
		return -1;
	}

	*pid = currentPid;

	addNode(sem->waitQueue, (void *) pid);

	release(&sem->spinlock);

	blockProcess(currentPid);

	return 0;
}

int sem_post(int id) {
	if (isValidSemId(id) == -1) {
		return -1;
	}

	semaphore_t *sem = &semManager->semaphores[id];

	acquire(&sem->spinlock);

	// cuando un proceso hace sem_wait y no hay recursos, se guarda su PID
	// en la cola y el proceso se bloquea. Es posible que mientras espera
	// ese proceso termine por otra razón -> hay que borrarlos de la waiting list

	toBegin(sem->waitQueue);
	while (hasNext(sem->waitQueue)) {
		int16_t *pid = (int16_t *) nextInList(sem->waitQueue);
		ProcessContext *proc = findProcess(*pid);

		if (proc == NULL || proc->status == TERMINATED) {
			removeNode(sem->waitQueue, (void *) pid);
			mm_free(pid);
		}
	}

	// si hay procesos en la cola, se saca al primero y se lo despierta
	// (no se incrementa el contador)

	if (!isEmpty(sem->waitQueue)) {
		int16_t *pid = (int16_t *) getFirstData(sem->waitQueue);
		removeNode(sem->waitQueue, (void *) pid);

		if (pid != NULL) {
			int16_t waitingPid = *pid;
			mm_free(pid);

			setReadyProcess(waitingPid);
			release(&sem->spinlock);
			return 0;
		}
	}

	// si no hay procesos esperando, incrementar contador
	sem->counter++;
	release(&sem->spinlock);
	return 0;
}

static int isValidSemId(int id) {
	if (id < 0 || id >= NUM_SEMS) {
		return -1;
	}
	return 0;
}
