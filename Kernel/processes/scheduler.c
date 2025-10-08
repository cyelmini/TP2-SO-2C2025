#include <stdint.h>
#include <doubleLinkedList.h>
#include <memoryManager.h>
#include <scheduler.h>
#include <process.h>
#include <lib.h>

#define MAX_PROCESS 20 //nidea tenemos que poner un numero real
#define MIN_QUANTUM 1

typedef struct schedulerCDT {
	doubleLinkedListADT processList;
	doubleLinkedListADT readyProcess;
	doubleLinkedListADT blockedProcess;
	int16_t currentPid;
	int16_t nextPid;
	PCB *currentProcess;
	uint16_t processQty;
	int quantums;
    uint64_t globalTicks; // para aging de prioridad
} schedulerCDT;

schedulerADT scheduler = NULL;

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

	//faltaria aca hacer el createProcess del primer proceso
}

int16_t createProcess(){
    if (scheduler->processQty > MAX_PROCESS)
		return -1;

}