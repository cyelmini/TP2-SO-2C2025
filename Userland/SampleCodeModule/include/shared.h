#ifndef _SHARED_H
#define _SHARED_H

#include <stdint.h>

/*
 * Información general del estado del heap.
 */
typedef struct mem {
	uint64_t size;
	uint64_t used;
	uint64_t free;
} mem_t;

#endif