#ifndef _SHARED_H
#define _SHARED_H

#include <stdint.h>

/* Tipo para identificador de procesos en userland */
typedef int16_t pid_t;

/*
 * Información general del estado del heap.
 */
typedef struct mem {
	uint64_t size;
	uint64_t used;
	uint64_t free;
} mem_t;

/*
 * Información de un proceso dado.
 */
typedef struct ProcessInfo {
	char *name;
	uint8_t priority;
	char ground;
	uint8_t status;
	int16_t pid;

	uint64_t stackBase;
	uint64_t stackPos;
} ProcessInfo;

#endif