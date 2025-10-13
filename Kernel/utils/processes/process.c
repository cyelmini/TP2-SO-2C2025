#include <stdio.h>
#include <stdint.h>
#include "../../include/process.h"
#include "../../include/memoryManagement.h"

/*Listar todos los procesos: nombre, ID, prioridad, stack y base pointer, foreground y
cualquier otra variable que consideren necesaria.*/

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

typedef struct CPUState {
    uint64_t rip;
    uint64_t rsp;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
} CPUState;

typedef struct ProcessContext{
    char *name;
    uint8_t priority;
	int16_t pid;
    int16_t parentPid;

	uint64_t stackBase;
	uint64_t stackPos;

    ProcessState state;
    int ground; // 0 1

    char **argv;
	int argc;
    CPUState registers;
    int16_t fileDescriptors[CANT_FILE_DESCRIPTORS];

    uint64_t quantumTicks;
} ProcessContext;


int initializeProcess(ProcessContext* process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip, int ground, int16_t fileDescriptors[]){  
    //.        STACK Y ARGS        //
                        // me paro en el final ya que el stack crece hacia abajo
    process->stackBase = (uint64_t) mm_alloc(STACK_SIZE) + STACK_SIZE; 
                        //si malloc fallo devuelve NULL(0)...
    if (process->stackBase - STACK_SIZE == 0) {
		return -1;
	}
    process->argc = argc;
    process->argv = allocArgv(process, args, argc);
    if (process->argv == NULL) {
		//FREE
		return -1;
	}
    process->registers.rip = rip;
    process->stackPos = setupStackFrame(process->stackBase, process->registers.rip, argc, process->argv);
    
    //.       NOMBRE.       //
    process->name = mm_alloc(strlen(args[0]) + 1);
    if (process->name == NULL) {
		return -1;
        // FREE
	}
    strcpy(process->name, args[0]);

    //.     PRIORIDAD  Y PID   //
    process->priority = priority;
    process->pid = pid;
    if (process->pid > 1) {
		process->state = BLOCKED;
	}
	else {
		process->state = READY;
	}

    //.       FD Y GROUND.   //
    for (int i = 0; i < CANT_FILE_DESCRIPTORS; i++) {
		process->fileDescriptors[i] = fileDescriptors[i];
	}
	process->ground = ground;

	return 0;
}

static char **allocArgv(ProcessContext *pc, char **argv, int argc) {
	char **newArgv = mm_alloc((argc + 1) * sizeof(char *));
	if (newArgv == NULL) {
		return NULL;
	}
	for (int i = 0; i < argc; i++) {
		newArgv[i] = mm_alloc(strlen(argv[i]) + 1);

		if (newArgv[i] == NULL) {
			for (int j = 0; j < i; j++) {
				mm_free(newArgv[j]);
			}
			mm_free(newArgv);
			return NULL;
		}

		strcpy(newArgv[i], argv[i]);
	}
	newArgv[argc] = NULL;
	return newArgv;
}

void freeProcess(ProcessContext * pcb) {
    freeArgv(pcb, pcb->argv, pcb->argc);
    mm_free(pcb->name);
    mm_free((void *)(pcb->stackBase - STACK_SIZE));
    mm_free(pcb);
}

int waitProcess(int16_t pid) {
    
}

int changeFileDescriptors(int16_t pid, int16_t fileDescriptors[]) {
    
}