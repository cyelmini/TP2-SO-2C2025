#include <stdlib.h>
#include "../include/lib.h"
#include "../include/doubleLinkedList.h"
#include "../../include/scheduler.h"
#include "../../include/process.h"
#include "../../include/memoryManagement.h"

schedulerADT scheduler = NULL;
static int created = 0;
static void idle();

static schedulerADT getScheduler(){
	return scheduler;
} 

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

	createProcess((uint64_t)idle, argsIdle, 1, MIN_PRIORITY, fileDescriptors, 0);
}

uint64_t schedule(uint64_t prevRSP){
	if(created == 0){
		return prevRSP;
	}
	schedulerADT scheduler = getScheduler();
	if(scheduler == NULL){
		return prevRSP;
	}
	scheduler->quantums--; 
	
	if(scheduler->processQty == 0 || scheduler->quantums > 0){
		return prevRSP;
	}
	
	if(scheduler->currentPid == NO_PROCESS){
		scheduler->currentProcess = getFirstData(scheduler->readyProcess);
		if(scheduler->currentProcess == NULL){
			return prevRSP;
		}
		scheduler->currentPid = scheduler->currentProcess->pid;
		scheduler->quantums = scheduler->currentProcess->priority;
		scheduler->currentProcess->status = RUNNING;
		return scheduler->currentProcess->stackPos;
	}

	if(scheduler->currentProcess != NULL){
		scheduler->currentProcess->stackPos = prevRSP;
		if(scheduler->currentProcess->status == RUNNING){
			scheduler->currentProcess->status = READY;
			addNode(scheduler->readyProcess, scheduler->currentProcess);
		} else if (scheduler->currentProcess->status == TERMINATED){
			freeProcess(scheduler->currentProcess);
		}
	}

	ProcessContext * firstProcess = getFirstData(scheduler->readyProcess);
	if(firstProcess == NULL){
		ProcessContext * process = findProcess(IDLE_PID); 
		if(process == NULL){
			return prevRSP;
		} else {
			scheduler->currentProcess = process;
			return process->stackPos;
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
	if(scheduler == NULL){
		return -1;
	}

	if(scheduler->processQty >= MAX_PROCESS){
		return -1;
	}

	ProcessContext *newProcess = (ProcessContext *)mm_alloc(sizeof(ProcessContext));
	if(newProcess == NULL){
		return -1;
	}

	if(initializeProcess(newProcess, scheduler->nextPid, args, argc, priority, rip, ground, fileDescriptors) == -1){
		freeProcess(newProcess);
		return -1;
	}

	addNode(scheduler->processList, newProcess);
	if(newProcess->pid > 1){
		addNode(scheduler->blockedProcess, newProcess);
	} else {
		addNode(scheduler->readyProcess, newProcess);
	}

	scheduler->nextPid++;
	scheduler->processQty++;
	return newProcess->pid;
}

int16_t getPid(){
	schedulerADT scheduler = getScheduler();
    return scheduler->currentPid;
}

ProcessInfo *ps(uint16_t *processQty){

	schedulerADT scheduler = getScheduler();
	if (scheduler->processList == NULL) {
		*processQty = 0;
		return NULL;
	}

	ProcessInfo * array = (ProcessInfo *) mm_alloc(sizeof(ProcessInfo) * scheduler->processQty);
	if(array == NULL){
		*processQty = 0;
		return NULL;
	}

	toBegin(scheduler->processList);
	ProcessContext * aux;
	int i = 0;

	while(hasNext(scheduler->processList)){
		aux = nextInList(scheduler->processList);
		array[i].pid = aux->pid;
		array[i].priority = aux->priority;
		array[i].ground = aux->ground;
		array[i].stackPos = aux->stackPos;
		array[i].stackBase = aux->stackBase;
		array[i].status = aux->status;

		if(aux->name != NULL){
			array[i].name = (char *)mm_alloc(my_strlen(aux->name) + 1);
			if(array[i].name == NULL){
				mm_free(array[i].name);
				mm_free(array);
				*processQty = 0;
				return NULL;
			}
			my_strcpy(array[i].name, aux->name);
		} else {
			array[i].name = NULL;
		}
		i++;
	}
	*processQty = scheduler->processQty;
	return array;
}

ProcessContext *findProcess(int16_t pid){
	schedulerADT scheduler = getScheduler();
	toBegin(scheduler->processList);
	while(hasNext(scheduler->processList)){
		ProcessContext * aux = nextInList(scheduler->processList);
		if(aux->pid == pid){
			return aux;
		}
	}
	return NULL;
}

int64_t setReadyProcess(int16_t pid){
	return 0;
}

int64_t blockProcess(int16_t pid){
	return 0;
}

void yield(){
	return;
}

int64_t killCurrentProcess(){
	return 0;
}

int64_t killProcess(int16_t pid){
	return 0;
}

int64_t killForegroundProcess(){
	return 0;
}

int64_t getFD(int64_t fd){
	return 0;
}

int16_t copyProcess(ProcessInfo *dest, ProcessContext *src) {
	return 0;
} 

 static void idle() {
	while (1) {
		_hlt();
	}
}