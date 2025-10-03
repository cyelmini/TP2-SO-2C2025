#include "../include/loader.h"
#include "../include/stdio.h"
#include "../include/syscalls.h"
#include "../include/test_utils.h"

#define MAX_BLOCKS 128

typedef struct mm_rq {
	void *address;
	uint32_t size;
} mm_rq;

uint64_t test_mm(uint64_t argc, char *argv[]) {
	mm_rq mm_rqs[MAX_BLOCKS];
	uint8_t rq;
	uint32_t total;
	uint64_t max_memory;

	if (argc != 1) {
		printf("ERROR: cantidad de argumentos\n");
		return -1;
	}

	max_memory = satoi(argv[0]);
	if (max_memory <= 0) {
		printf("ERROR: argumento invalido\n");
		return -1;
	}

	printf("Iniciando test con maximo %d bytes\n", max_memory);
	printf("Solicitando bloques de memoria...\n");
	rq = 0;
	total = 0;
    int iteracion = 0;
	while (rq < MAX_BLOCKS && total < max_memory) {
		mm_rqs[rq].size = getUniform(max_memory - total - 1) + 1;
		mm_rqs[rq].address = mm_alloc(mm_rqs[rq].size);

		if (mm_rqs[rq].address != NULL) {
			total += mm_rqs[rq].size;
			rq++;
		}
        iteracion++;
	}

	printf("Inicializando bloques...\n");
	uint32_t i;
	for (i = 0; i < rq; i++) {
		if (mm_rqs[i].address) {
			memset(mm_rqs[i].address, i % 256, mm_rqs[i].size);
		}
	}

	printf("Chequeando bloques...\n");
	for (i = 0; i < rq; i++) {
		if (mm_rqs[i].address) {
			if (!memcheck(mm_rqs[i].address, i, mm_rqs[i].size)) {
				return -1;
			}
		}
	}

	printf("Liberando bloques...\n");
	for (i = 0; i < rq; i++) {
		if (mm_rqs[i].address) {
			mm_free(mm_rqs[i].address);
		}
	}
	return 0;
}