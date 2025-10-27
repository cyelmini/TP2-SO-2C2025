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

	uint64_t quantumTicks;

	doubleLinkedListADT waitingList;
} ProcessContext;

int initializeProcess(ProcessContext *process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip,
					  char ground, int16_t fileDescriptors[]);

void freeProcess(ProcessContext *pcb);

int waitProcess(int16_t pid);

int changeFileDescriptors(int16_t pid, int16_t fileDescriptors[]);

int changePriority(int16_t pid, uint8_t priority);

extern uint64_t setupStackFrame(uint64_t stackBase, uint64_t code, int argc, char *args[]);

#endif // PROCESS_H