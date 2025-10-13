#include "../include/doubleLinkedList.h"
#include "../../include/scheduler.h"
#include "../../include/process.h"
#include "../../include/memoryManagement.h"

#include <lib.h>

schedulerADT scheduler = NULL;
static int created = 0;

void createScheduler() {

	scheduler = (schedulerADT) malloc(sizeof(schedulerCDT));
	
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

	//.        IDLE.       // --> proceso al pedo que salva a la cpu de ejecutar basura si no hay ningun proceso READY
	char *argsIdle[1] = {"idle"};

	int16_t fileDescriptors[] = {-1, -1, STDERR};
	//createProcess((uint64_t) idle, argsIdle, 1, 1, fileDescriptors, 1);
}


/* VER DONDE SE LLAMA AL SCHEDULE EN ASM !!!!!!!! */

uint64_t schedule(uint64_t prevRSP){
	if(created == 0){
		return prevRSP;
	}
	schedulerADT scheduler = getScheduler();
	if(scheduler == NULL){
		return -1;
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
		scheduler->currentProcess->state = RUNNING;
		return scheduler->currentProcess->stackPos;
	}

	if(scheduler->currentProcess != NULL){
		scheduler->currentProcess->stackPos = prevRSP;
		if(scheduler->currentProcess->state == RUNNING){
			scheduler->currentProcess->state = READY;
			addNode(scheduler->readyProcess, scheduler->currentProcess);
		} else if (scheduler->currentProcess->state == TERMINATED){
			freeProcess(scheduler->currentProcess);
		}
	}

	ProcessContext * firstProcess = getFirstData(scheduler->readyProcess);
	if(firstProcess == NULL){
		ProcessContext * process = findProcess(IDLE_PID); // ver si hay que implementar idle
		if(process == NULL){
			return prevRSP;
		} else {
			scheduler->currentProcess = process;
			return process->stackPos;
		}
	}

	scheduler->currentPid = firstProcess->pid;
	scheduler->quantums = scheduler->currentProcess->priority;
	scheduler->currentProcess = firstProcess;
	scheduler->currentProcess->state = RUNNING; 
	return scheduler->currentProcess->stackPos;
}

int16_t createProcess(uint64_t rip, char **args, int argc, uint8_t priority, int16_t fileDescriptors[], int ground) {
	schedulerADT scheduler = getScheduler();
	if(scheduler == NULL){
		return -1;
	}

	if(scheduler->processQty >= MAX_PROCESS){
		return -1;
	}

	ProcessContext *newProcess = (ProcessContext *)mm_alloc(sizeof(ProcessContext));
	if(newProcess = NULL){
		return -1;
	}

	//if(initializeProcess(newProcess, scheduler->nextPid, args, argc, priority, rip) == -1)
}

int64_t setReadyProcess(int16_t pid){

}

int64_t blockProcess(int16_t pid){

}

ProcessContext *findProcess(int16_t pid){
	
}

void yield(){
	
}

int64_t killCurrentProcess(){

}

int64_t killProcess(int16_t pid){
	
}

int64_t killForegroundProcess(){

}

int16_t getPid(){

}

int64_t getFD(int64_t fd){
	
}


ProcessInfo *ps(uint16_t *proccesQty) {

}

int16_t copyProcess(ProcessInfo *dest, ProcessContext *src) {
	
}

static getScheduler(){
	return scheduler;
}