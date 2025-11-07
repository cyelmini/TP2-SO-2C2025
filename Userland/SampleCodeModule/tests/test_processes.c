#include "../include/syscalls.h"
#include "../include/test_util.h"
#include <stdio.h>

enum State { RUNNING, BLOCKED, KILLED };

typedef struct P_rq {
	int32_t pid;
	enum State state;
} p_rq;

int64_t test_processes(uint64_t argc, char *argv[]) {
	uint8_t rq;
	uint8_t alive = 0;
	uint8_t action;
	uint64_t max_processes;
	char *argvAux[] = {"endless_loop"};
	int16_t fileDescriptors[] = {STDIN, STDOUT, STDERR}; // stdin, stdout, stderr

	if (argc != 1)
		return -1;
	
	if ((max_processes = satoi(argv[0])) <= 0)
		return -1;

	p_rq p_rqs[max_processes];

	while (1) {
		for (rq = 0; rq < max_processes; rq++) {
			p_rqs[rq].pid = sys_createProcess((uint64_t) endless_loop, argvAux, 1, 1, 1, fileDescriptors);
			
			if (p_rqs[rq].pid == -1) {
				printf("test_processes: ERROR creating process\n");
				return -1;
			}
			else {
				p_rqs[rq].state = RUNNING;
				alive++;
			}
		}

		// Randomly kills, blocks or unblocks processes until every one has been killed
		while (alive > 0) {
			for (rq = 0; rq < max_processes; rq++) {
				action = getUniform(100) % 2;

				switch (action) {
					case 0:
						if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
							if (sys_killProcess(p_rqs[rq].pid) == -1) {
								printf("test_processes: ERROR killing process\n");
								return -1;
							}
							p_rqs[rq].state = KILLED;
							alive--;
						}
						break;

					case 1:
						if (p_rqs[rq].state == RUNNING) {
							if (sys_blockProcess(p_rqs[rq].pid) == -1) {
								printf("test_processes: ERROR blocking process\n");
								return -1;
							}
							p_rqs[rq].state = BLOCKED;
						}
						break;
				}
			}

			// Randomly unblocks processes
			for (rq = 0; rq < max_processes; rq++)
				if (p_rqs[rq].state == BLOCKED && getUniform(100) % 2) {
					if (sys_setReadyProcess(p_rqs[rq].pid) == -1) {
						printf("test_processes: ERROR unblocking process\n");
						return -1;
					}
					p_rqs[rq].state = RUNNING;
				}
		}
	}
	return 0;
}