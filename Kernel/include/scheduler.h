#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "process.h"
#include <stdint.h>

#define MAX_PROCESS 30

#define NUM_PRIORITIES 5
#define HIGHEST_PRIORITY 1
#define LOWEST_PRIORITY 5
#define MIN_QUANTUMS 1
#define RR_QUANTUM_TICKS 2
#define AGING_THRESHOLD_TICKS 100
#define PRIORITY_RESET_TICKS 1000

#define NO_PROCESS -1
#define IDLE_PID 0
#define SHELL_PID 1

#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct schedulerCDT {
	doubleLinkedListADT processList;
	doubleLinkedListADT readyQueues[NUM_PRIORITIES];
	doubleLinkedListADT blockedProcess;

	int16_t currentPid;

	ProcessContext *currentProcess;

	uint16_t processQty;
	int quantums;
	uint64_t globalTicks;
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

void createScheduler();

uint64_t schedule(uint64_t prevRSP);

int16_t createProcess(uint64_t rip, char **args, int argc, uint8_t priority, int16_t fileDescriptors[], char ground);

int64_t setReadyProcess(int16_t pid);

int64_t blockProcess(int16_t pid);

ProcessContext *findProcess(int16_t pid);

void yield();

int64_t killCurrentProcess();

int64_t killProcess(int16_t pid);

int64_t killForegroundProcess();

int16_t getPid();

int64_t getFd(int64_t fd);

ProcessInfo *ps(uint16_t *proccesQty);

int16_t copyProcess(ProcessInfo *dest, ProcessContext *src);

void priority_reset(ProcessContext *p, schedulerADT scheduler);

void aging(ProcessContext *p, schedulerADT scheduler);

uint64_t nextscheduled(uint64_t prevRSP, schedulerADT scheduler);

int changePriority(int16_t pid, uint8_t priority);

#endif