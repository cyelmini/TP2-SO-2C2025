#include <stdio.h>
#include <stdint.h>

/*Listar todos los procesos: nombre, ID, prioridad, stack y base pointer, foreground y
cualquier otra variable que consideren necesaria.*/

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} ProcessState;

typedef struct ProcessContext{
    char *name;
    uint8_t priority;
	int16_t pid;
    int16_t parentPid;
	uint64_t stackBase;
	uint64_t stackPos;
    ProcessState state;
    uint8_t foreground;

}ProcessContext;