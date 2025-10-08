#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define CANT_FILE_DESCRIPTORS 3 
#define STACK_SIZE 4096 

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
    int ground;

    char **argv;
    int argc;
    CPUState registers;
    int16_t fileDescriptors[CANT_FILE_DESCRIPTORS];

    uint64_t quantumTicks;
} ProcessContext;

int initializeProcess(ProcessContext* process, int16_t pid, char **args, int argc, uint8_t priority, uint64_t rip);

void freeProcess(ProcessContext * pcb);

int waitProcess(int16_t pid);

int changeFileDescriptors(int16_t pid, int16_t fileDescriptors[]);

extern uint64_t setupStackframe(uint64_t stackBase, uint64_t code, int argc, char *args[]);

#endif // PROCESS_H