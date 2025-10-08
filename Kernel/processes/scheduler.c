#include <stdint.h>
#include <../include/doubleLinkedList.h>
#include <scheduler.h>
#include <../include/process.h>
#include <lib.h>

#define MAX_PROCESS 30 //nidea tenemos que poner un numero real
#define MIN_QUANTUMS 1

typedef struct schedulerCDT {
	doubleLinkedListADT processList;
	doubleLinkedListADT readyProcess;
	doubleLinkedListADT blockedProcess;
	int16_t currentPid;
	int16_t nextPid;
	ProcessContext *currentProcess;
	uint16_t processQty;
	int quantums;
    uint64_t globalTicks; // para aging de prioridad
} schedulerCDT;

typedef struct schedulerCDT *schedulerADT;

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

	//.        IDLE.       // --> proceso al pedo que salva a la cpu de ejecutar basura si no hay ningun proceso READY
	char *argsIdle[1] = {"idle"};
	int16_t fileDescriptors[] = {-1, -1, STDERR};
	//createProcess((uint64_t) idle, argsIdle, 1, 1, fileDescriptors, 1);
}

int16_t createProcess(){
    if (scheduler->processQty > MAX_PROCESS)
		return -1;

}