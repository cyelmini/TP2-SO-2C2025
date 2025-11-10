// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "../../include/process.h"
#include "../../include/lib.h"
#include "../../include/memoryManagement.h"
#include "../../include/scheduler.h"
#include <stdint.h>
#include <stdio.h>

static void freeArgv(char **argv, int argc);
static char **allocArgv(char **argv, int argc);

int initializeProcess(ProcessContext *process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip,
					  char ground, int16_t fileDescriptors[]) {
	process->stackBase = 0;
	process->stackPos = 0;
	process->argv = NULL;
	process->argc = argc;
	process->name = NULL;
	process->priority = priority;
	process->pid = pid;
	process->rip = rip;
	process->ground = ground;
	process->waitingList = NULL;
	process->status = READY;

	process->stackBase = (uint64_t) mm_alloc(STACK_SIZE);
	if (process->stackBase == 0) {
		return -1;
	}
	process->stackBase += STACK_SIZE;

	process->argv = allocArgv(args, argc);
	if (process->argv == NULL) {
		mm_free((void *) (process->stackBase - STACK_SIZE));
		process->stackBase = 0;
		return -1;
	}

	const char *name = "process";
	if (argc > 0 && args != NULL && args[0] != NULL) {
		name = args[0];
	}

	process->name = mm_alloc(my_strlen(name) + 1);
	if (process->name == NULL) {
		freeArgv(process->argv, process->argc);
		process->argv = NULL;
		mm_free((void *) (process->stackBase - STACK_SIZE));
		process->stackBase = 0;
		return -1;
	}
	my_strcpy(process->name, name);
	process->stackPos = setupStackFrame(process->stackBase, process->rip, argc, process->argv);

	for (int i = 0; i < CANT_FILE_DESCRIPTORS; i++) {
		process->fileDescriptors[i] = (fileDescriptors != NULL) ? fileDescriptors[i] : i;
	}

	process->waitingList = createDoubleLinkedListADT();
	if (process->waitingList == NULL) {
		mm_free(process->name);
		process->name = NULL;
		freeArgv(process->argv, process->argc);
		process->argv = NULL;
		mm_free((void *) (process->stackBase - STACK_SIZE));
		process->stackBase = 0;
		return -1;
	}

	return 0;
}

void freeProcess(ProcessContext *pcb) {
	if (pcb == NULL) {
		return;
	}

	if (pcb->argv != NULL) {
		freeArgv(pcb->argv, pcb->argc);
		pcb->argv = NULL;
	}

	if (pcb->name != NULL) {
		mm_free(pcb->name);
		pcb->name = NULL;
	}

	if (pcb->stackBase >= STACK_SIZE) {
		mm_free((void *) (pcb->stackBase - STACK_SIZE));
		pcb->stackBase = 0;
	}

	if (pcb->waitingList != NULL) {
		freeLinkedListADT(pcb->waitingList);
		pcb->waitingList = NULL;
	}

	mm_free(pcb);
}

int waitProcess(int16_t pid) {
	ProcessContext *pcb = findProcess(pid);
	int16_t currentPid = getPid();

	// si falla o intenta esperarse a si mismo
	if (pcb == NULL || currentPid == pid) {
		return -1;
	}

	ProcessContext *currentProcess = findProcess(currentPid);
	addNode(pcb->waitingList, currentProcess);
	blockProcess(currentPid);
	return 0;
}

int changeFileDescriptors(int16_t pid, int16_t fileDescriptors[]) {
	ProcessContext *process = findProcess(pid);

	if (process == NULL) {
		return -1;
	}

	for (int i = 0; i < CANT_FILE_DESCRIPTORS; i++) {
		process->fileDescriptors[i] = fileDescriptors[i];
	}

	return 0;
}

static void freeArgv(char **argv, int argc) {
	if (argv == NULL) {
		return;
	}

	for (int i = 0; i < argc; i++) {
		if (argv[i] != NULL) {
			mm_free(argv[i]);
		}
	}

	mm_free(argv);
}

static char **allocArgv(char **argv, int argc) {
	if (argc <= 0) {
		char **emptyArgv = mm_alloc(sizeof(char *));
		if (emptyArgv != NULL) {
			emptyArgv[0] = NULL;
		}
		return emptyArgv;
	}

	if (argv == NULL) {
		return NULL;
	}

	char **newArgv = mm_alloc((argc + 1) * sizeof(char *));
	if (newArgv == NULL) {
		return NULL;
	}
	for (int i = 0; i < argc; i++) {
		if (argv[i] == NULL) {
			for (int j = 0; j < i; j++) {
				mm_free(newArgv[j]);
			}
			mm_free(newArgv);
			return NULL;
		}

		size_t len = my_strlen(argv[i]);
		newArgv[i] = mm_alloc(len + 1);

		if (newArgv[i] == NULL) {
			for (int j = 0; j < i; j++) {
				mm_free(newArgv[j]);
			}
			mm_free(newArgv);
			return NULL;
		}

		my_strcpy(newArgv[i], argv[i]);
	}
	newArgv[argc] = NULL;
	return newArgv;
}

int changePriority(int16_t pid, uint8_t priority) {
	if (priority > MAX_PRIORITY || priority < MIN_PRIORITY) {
		return -1;
	}

	ProcessContext *process = findProcess(pid);
	if (process == NULL) {
		return -1;
	}
	process->priority = priority;
	return priority;
}