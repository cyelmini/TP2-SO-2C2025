#include "../../include/scheduler.h"
#include "../../include/memoryManagement.h"
#include "../../include/process.h"
#include "../../include/video.h"
#include "../include/doubleLinkedList.h"
#include "../include/lib.h"
#include <stdlib.h>

schedulerADT scheduler = NULL;
static int created = 0;

static schedulerADT getScheduler();
static void idle();
static ProcessContext *pipedTo(int16_t fd);
static int16_t pipedFd(int16_t *fds);
static int64_t kill(schedulerADT scheduler, ProcessContext *process);

void createScheduler() {
	scheduler = (schedulerADT) mm_alloc(sizeof(schedulerCDT));

	if (scheduler == NULL) {
		return;
	}

	scheduler->processList = createDoubleLinkedListADT();
	scheduler->readyProcess = createDoubleLinkedListADT();
	scheduler->blockedProcess = createDoubleLinkedListADT();
	scheduler->currentPid = -1;
	scheduler->nextPid = 0;
	scheduler->currentProcess = NULL;
	scheduler->processQty = 0;
	scheduler->quantums = MIN_QUANTUMS;
	scheduler->globalTicks = 0;

	created = 1;

	char *argsIdle[1] = {"idle"};

	int16_t fileDescriptors[] = {-1, -1, STDERR};

	createProcess((uint64_t) idle, argsIdle, 1, MIN_PRIORITY, fileDescriptors, 1);
}

uint64_t schedule(uint64_t prevRSP) {
	if (created == 0) {
		return prevRSP;
	}
	schedulerADT scheduler = getScheduler();
	if (scheduler == NULL) {
		return prevRSP;
	}
	scheduler->quantums--;

	if (scheduler->processQty == 0 || scheduler->quantums > 0) {
		return prevRSP;
	}

	if (scheduler->currentPid == NO_PROCESS) {
		scheduler->currentProcess = getFirstData(scheduler->readyProcess);
		if (scheduler->currentProcess == NULL) {
			return prevRSP;
		}
		scheduler->currentPid = scheduler->currentProcess->pid;
		scheduler->quantums = scheduler->currentProcess->priority;
		scheduler->currentProcess->status = RUNNING;
		return scheduler->currentProcess->stackPos;
	}

	if (scheduler->currentProcess != NULL) {
		scheduler->currentProcess->stackPos = prevRSP;
		if (scheduler->currentProcess->status == RUNNING) {
			scheduler->currentProcess->status = READY;
			addNode(scheduler->readyProcess, scheduler->currentProcess);
		} else if (scheduler->currentProcess->status == TERMINATED) {
			freeProcess(scheduler->currentProcess);
			scheduler->currentProcess = NULL;
            scheduler->currentPid = NO_PROCESS;
		}
	}

	ProcessContext *firstProcess = getFirstData(scheduler->readyProcess);
	if (firstProcess == NULL) {
		ProcessContext *idle = findProcess(IDLE_PID);
		if (idle == NULL) {
			return prevRSP;
		} else {
			scheduler->currentProcess = idle;
			scheduler->currentPid = idle->pid;
			scheduler->quantums = idle->priority;
			idle->status = RUNNING;
			return idle->stackPos;
		}
	}

	scheduler->currentProcess = firstProcess;
	scheduler->currentPid = scheduler->currentProcess->pid;
	scheduler->quantums = scheduler->currentProcess->priority;
	scheduler->currentProcess->status = RUNNING;
	return scheduler->currentProcess->stackPos;
}

int16_t createProcess(uint64_t rip, char **args, int argc, uint8_t priority, int16_t fileDescriptors[], char ground) {
	schedulerADT scheduler = getScheduler();
	if (scheduler == NULL) {
		return -1;
	}

	if (scheduler->processQty > MAX_PROCESS) {
		return -1;
	}

	ProcessContext *newProcess = (ProcessContext *) mm_alloc(sizeof(ProcessContext));
	if (newProcess == NULL) {
		return -1;
	}

	if (initializeProcess(newProcess, scheduler->nextPid, args, argc, priority, rip, ground, fileDescriptors) == -1) {
		freeProcess(newProcess);
		return -1;
	}

	addNode(scheduler->processList, newProcess);
	if (newProcess->status == READY) {
		addNode(scheduler->readyProcess, newProcess);
	} else if (newProcess->status == BLOCKED) {
		addNode(scheduler->blockedProcess, newProcess);
	}

	scheduler->nextPid++;
	scheduler->processQty++;
	return newProcess->pid;
}

int16_t getPid() {
	schedulerADT scheduler = getScheduler();
	return scheduler->currentPid;
}

ProcessInfo *ps(uint16_t *processQty) {
	schedulerADT scheduler = getScheduler();
	if (scheduler->processList == NULL) {
		*processQty = 0;
		return NULL;
	}

	ProcessInfo *array = (ProcessInfo *) mm_alloc(sizeof(ProcessInfo) * scheduler->processQty);
	if (array == NULL) {
		*processQty = 0;
		return NULL;
	}

	toBegin(scheduler->processList);
	ProcessContext *aux;
	int i = 0;

	while (hasNext(scheduler->processList)) {
		aux = nextInList(scheduler->processList);
		array[i].pid = aux->pid;
		array[i].priority = aux->priority;
		array[i].ground = aux->ground;
		array[i].stackPos = aux->stackPos;
		array[i].stackBase = aux->stackBase;
		array[i].status = aux->status;

		if (aux->name != NULL) {
			array[i].name = (char *) mm_alloc(my_strlen(aux->name) + 1);
			if (array[i].name == NULL) {
				mm_free(array[i].name);
				mm_free(array);
				*processQty = 0;
				return NULL;
			}
			my_strcpy(array[i].name, aux->name);
		}
		else {
			array[i].name = NULL;
		}
		i++;
	}
	*processQty = scheduler->processQty;
	return array;
}

ProcessContext *findProcess(int16_t pid) {
	schedulerADT scheduler = getScheduler();
	toBegin(scheduler->processList);
	while (hasNext(scheduler->processList)) {
		ProcessContext *aux = nextInList(scheduler->processList);
		if (aux->pid == pid) {
			return aux;
		}
	}
	return NULL;
}

int64_t setReadyProcess(int16_t pid) {
	schedulerADT scheduler = getScheduler();
	ProcessContext *process = findProcess(pid);
	if (process == NULL) {
		return -1;
	}
	if (process->status == BLOCKED) {
		if (removeNode(scheduler->blockedProcess, process) == NULL) {
			return -1;
		}
		if (addNode(scheduler->readyProcess, process) == NULL) {
			return -1;
		}
		process->status = READY;
	}
	return 0;
}

int64_t blockProcess(int16_t pid) {
	schedulerADT scheduler = getScheduler();
	ProcessContext *process = findProcess(pid);
	if (process == NULL) {
		return -1;
	}
	if (process->status == RUNNING || process->status == READY) {
		if (process->status == READY) {
			if (removeNode(scheduler->readyProcess, process) == NULL)
				return -1;
		}
		if (addNode(scheduler->blockedProcess, process) == NULL)
			return -1;

		process->status = BLOCKED;
	}

	if (getPid() == pid)
		yield();

	return 0;
}

void yield() {
	schedulerADT scheduler = getScheduler();
	scheduler->quantums = 0;
	callTimerTick();
}

int64_t killCurrentProcess() {
	schedulerADT scheduler = getScheduler();
	return killProcess(scheduler->currentProcess->pid);
}

int64_t killProcess(int16_t pid) {
	schedulerADT scheduler = getScheduler();
	ProcessContext *process = findProcess(pid);
	if (process == NULL) {
		return -1;
	}

	int16_t fd = pipedFd(process->fileDescriptors);
	if (kill(scheduler, process) == -1) {
		return -1;
	}
	if (fd != -1) {
		ProcessContext *aux = pipedTo(fd);
		if (aux != NULL) {
			return kill(scheduler, aux);
		}
	}
	return 0;
}

// ground == 0 -->foreground
int64_t killForegroundProcess() {
	schedulerADT scheduler = getScheduler();
	if (scheduler->currentProcess == NULL) {
		return -1;
	}
	if (scheduler->currentProcess->ground == 0 && scheduler->currentProcess->pid != SHELL_PID) {
		printf("^C\n");
		return killCurrentProcess();
	}
	else {
		ProcessContext *aux;
		toBegin(scheduler->processList);
		while (hasNext(scheduler->processList)) {
			aux = nextInList(scheduler->processList);
			if (aux->ground == 0 && aux->pid != SHELL_PID) {
				printf("^C\n");
				return kill(scheduler, aux);
			}
		}
	}
	// si no habia foreground que no sea la shell
	return 0;
}

int64_t getFd(int64_t fd) {
	schedulerADT scheduler = getScheduler();
	ProcessContext *process = scheduler->currentProcess;
	return process->fileDescriptors[fd];
}

int16_t copyProcess(ProcessInfo *dest, ProcessContext *src) {
	dest->pid = src->pid;
	dest->stackBase = src->stackBase;
	dest->stackPos = src->stackPos;
	dest->priority = src->priority;
	dest->ground = src->ground;
	dest->status = src->status;

	if (src->name != NULL) {
		dest->name = mm_alloc(my_strlen(src->name) + 1);
		if (dest->name == NULL) {
			return -1;
		}
		my_strcpy(dest->name, src->name);
	}
	else {
		dest->name = NULL;
	}

	return 0;
}

static schedulerADT getScheduler() {
	return scheduler;
}

static void idle() {
	while (1) {
		_hlt();
	}
}

static ProcessContext *pipedTo(int16_t fd) {
	schedulerADT scheduler = getScheduler();
	ProcessContext *aux;
	toBegin(scheduler->processList);
	while (hasNext(scheduler->processList)) {
		aux = nextInList(scheduler->processList);
		if (aux->fileDescriptors[STDIN] == fd || aux->fileDescriptors[STDOUT] == fd) {
			return aux;
		}
	}
	return NULL;
}

static int16_t pipedFd(int16_t *fds) {
	if (fds[STDIN] != STDIN) {
		return fds[STDIN];
	}
	else if (fds[STDOUT] != STDOUT) {
		return fds[STDOUT];
	}
	return -1;
}

static int64_t kill(schedulerADT scheduler, ProcessContext *process) {
	if (process->status == READY) {
		if (removeNode(scheduler->readyProcess, process) == NULL) {
			return -1;
		}
	}
	else if (process->status == BLOCKED) {
		if (removeNode(scheduler->blockedProcess, process) == NULL) {
			return -1;
		}
	}

	toBegin(process->waitingList);
	ProcessContext *aux;
	while (hasNext(process->waitingList)) {
		aux = nextInList(process->waitingList);
		if(aux != NULL) {
			setReadyProcess(aux->pid);
		}
	}
	if (removeNode(scheduler->processList, process) == NULL) {
		return -1;
	}
	process->status = TERMINATED;
	scheduler->processQty--;

	if(scheduler->currentProcess != process){
		freeProcess(process);
	}

	return 0;
}