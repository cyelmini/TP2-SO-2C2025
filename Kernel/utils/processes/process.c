#include "../../include/process.h"
#include "../../include/lib.h"
#include "../../include/memoryManagement.h"
#include "../../include/scheduler.h"
#include <stdint.h>
#include <stdio.h>

static void freeArgv(ProcessContext *pcb, char **argv, int argc);
static char **allocArgv(ProcessContext *pc, char **argv, int argc);

int initializeProcess(ProcessContext *process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip,
					  char ground, int16_t fileDescriptors[]) {
	//.        STACK Y ARGS        //
	// me paro en el final ya que el stack crece hacia abajo
	process->stackBase = (uint64_t) mm_alloc(STACK_SIZE) + STACK_SIZE;
	// si malloc fallo devuelve NULL(0)...
	if (process->stackBase - STACK_SIZE == 0) {
		return -1;
	}
	process->argc = argc;
	process->argv = allocArgv(process, args, argc);
	if (process->argv == NULL) {
		mm_free((void *) (process->stackBase - STACK_SIZE));
		return -1;
	}
	process->rip = rip;
	process->stackPos = setupStackFrame(process->stackBase, process->rip, argc, process->argv);

	if (argc <= 0 || args == NULL || args[0] == NULL) {
		mm_free((void *) (process->stackBase - STACK_SIZE));
		return -1;
	}
	//.       NOMBRE.       //
	process->name = mm_alloc(my_strlen(args[0]) + 1);
	if (process->name == NULL) {
		freeArgv(process, process->argv, process->argc);
		mm_free((void *) (process->stackBase - STACK_SIZE));
		return -1;
	}
	my_strcpy(process->name, args[0]);

	//.     PRIORIDAD  Y PID   //
	process->priority = priority;
	process->pid = pid;
	if (process->pid > 1) {
		process->status = BLOCKED;
	}
	else {
		process->status = READY;
	}

	//.       FD Y GROUND.   //
	for (int i = 0; i < CANT_FILE_DESCRIPTORS; i++) {
		process->fileDescriptors[i] = fileDescriptors[i];
	}
	process->ground = ground;

	//.     WAITINGLIST.      //
	process->waitingList = createDoubleLinkedListADT();
	if (process->waitingList == NULL) {
		mm_free(process->name);
		freeArgv(process, process->argv, process->argc);
		mm_free((void *) (process->stackBase - STACK_SIZE));
		return -1;
	}

	return 0;
}

void freeProcess(ProcessContext *pcb) {
	freeArgv(pcb, pcb->argv, pcb->argc);
	mm_free(pcb->name);
	mm_free((void *) (pcb->stackBase - STACK_SIZE));
	freeLinkedListADT(pcb->waitingList);
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

static void freeArgv(ProcessContext *pcb, char **argv, int argc) {
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

static char **allocArgv(ProcessContext *pc, char **argv, int argc) {
	char **newArgv = mm_alloc((argc + 1) * sizeof(char *));
	if (newArgv == NULL) {
		return NULL;
	}
	for (int i = 0; i < argc; i++) {
		newArgv[i] = mm_alloc(my_strlen(argv[i]) + 1);

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
		return .1;
	}

	ProcessContext *process = findProcess(pid);
	if (process == NULL) {
		return -1;
	}
	process->priority = priority;
	return priority;
}