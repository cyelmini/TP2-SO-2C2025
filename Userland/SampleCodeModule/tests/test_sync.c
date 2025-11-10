// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "../include/stdio.h"
#include "../include/syscalls.h"
#include "../include/test_util.h"
#include <stdint.h>

#define SEM_ID 100
#define TOTAL_PAIR_PROCESSES 2

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc) {
	uint64_t aux = *p;
	sys_yield(); // This makes the race condition highly probable
	aux += inc;
	*p = aux;
}

uint64_t my_process_inc(uint64_t argc, char *argv[]) {
	uint64_t n;
	int8_t inc;
	int8_t use_sem;

	if (argc != 3) {
		sys_exit();
		return -1;
	}

	if ((n = satoi(argv[0])) <= 0) {
		sys_exit();
		return -1;
	}
	if ((inc = satoi(argv[1])) == 0) {
		sys_exit();
		return -1;
	}
	if ((use_sem = satoi(argv[2])) < 0) {
		sys_exit();
		return -1;
	}

	if (use_sem) {
		if (sys_sem_open(SEM_ID) != 0) {
			printf("test_sync: ERROR opening semaphore\n");
			sys_exit();
			return -1;
		}
	}

	uint64_t i;
	for (i = 0; i < n; i++) {
		if (use_sem) {
			sys_sem_wait(SEM_ID);
		}
		slowInc(&global, inc);
		if (use_sem)
			sys_sem_post(SEM_ID);
	}

	sys_exit();
	return 0;
}

uint64_t test_sync(uint64_t argc, char *argv[]) { //{n, use_sem, 0}
	uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

	if (argc != 3)
		return -1;

	char *argvDec[] = {argv[1], "-1", argv[2], NULL};
	char *argvInc[] = {argv[1], "1", argv[2], NULL};

	global = 0;

	uint64_t use_sem = satoi(argv[2]);
	if (use_sem) {
		if (sys_sem_create(SEM_ID, 1) != 0) {
			printf("test_sync: ERROR creating semaphore\n");
			return -1;
		}
	}

	int16_t fds[] = {STDIN, STDOUT, STDERR};
	uint64_t i;
	for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		pids[i] = sys_createProcess((uint64_t) my_process_inc, argvDec, 3, 1, 0, fds);
		pids[i + TOTAL_PAIR_PROCESSES] = sys_createProcess((uint64_t) my_process_inc, argvInc, 3, 1, 0, fds);
	}

	for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		sys_waitProcess(pids[i]);
		sys_waitProcess(pids[i + TOTAL_PAIR_PROCESSES]);
	}

	printf("Final value: %d\n", global);

	if (use_sem) {
		sys_sem_destroy(SEM_ID);
	}

	sys_exit();
	return 0;
}
