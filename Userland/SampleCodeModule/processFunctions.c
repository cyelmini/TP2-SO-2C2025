// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "include/processFunctions.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/tests.h"
#include "include/shared.h"
#include <stdint.h>

static uint64_t clear();
static uint64_t ps();
static uint64_t loop(int argc, char **argv);
static uint64_t cat();
static uint64_t wc();
static uint64_t is_vowel(char c);
static uint64_t filter();
static uint64_t run_test_mm(int argc, char **argv);
static uint64_t run_test_processes(int argc, char **argv);
static uint64_t run_test_priority(int argc, char **argv);
static uint64_t run_test_sync(int argc, char **argv);

/* ------------------------ Funciones de aplicaciones de User Space ------------------------ */

/* ------------------------ CLEAR ------------------------ */

pid_t handle_clear(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	/* create process using provided argv/argc */
	pid_t pid = sys_createProcess((uint64_t) clear, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0 ;
}

static uint64_t clear() {
	sys_clear();
	sys_exit();
	return 0;
}

/* ------------------------ PS ------------------------ */

pid_t handle_ps(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	pid_t pid = sys_createProcess((uint64_t) ps, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t ps() {
    uint16_t qty = 0;
    ProcessInfo *list = sys_processInfo(&qty);
    if (!list) {
        sys_exit();
        return 0;
    }
	
    for (uint16_t i = 0; i < qty; i++) {
   		printf("PID:%d  NAME:%s  STATUS:%d  PRIO:%d\n",
           		(int) list[i].pid,
           		list[i].name ? list[i].name : "(null)",
           		(int) list[i].status,
        		(int) list[i].priority);

        if (list[i].name) {
            sys_mm_free(list[i].name);
        }
    }
	
    sys_mm_free(list);
    sys_exit();
    return 0;
}

/* ------------------------ LOOP ------------------------ */

pid_t handle_loop(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	pid_t pid = sys_createProcess((uint64_t) loop, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t loop(int argc, char **argv) {
	uint32_t interval = 1;
	
	if (argc > 1) {
		uint32_t v = str_to_uint32(argv[1]);
		if(v > 0){
			interval = v;
		}
	}

	uint64_t mypid = sys_getPid();
	
	while (1) {
		printf("[loop] Hola! soy PID=%d\n", (int) mypid);
		sys_sleep(interval);
	}
	return 0;
}

/* ------------------------ CAT ------------------------ */

pid_t handle_cat(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	pid_t pid = (pid_t) sys_createProcess((uint64_t) cat, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t cat() {
	int c;
	while((c = getchar()) != EOF) {
		if(c != 0){
			putchar(c);
		}
	}
	sys_exit();
	return 0;
}

/* ------------------------ WC ------------------------ */

pid_t handle_wc(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	pid_t pid = (pid_t) sys_createProcess((uint64_t) wc, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t wc() {
	int lines = 1;
    int c;
    
   while((c = getchar()) != EOF) {
		if (c != 0) {
			if (c == '\n') {
				lines++;
			}
			putchar(c);
		}
    }
    printf("La cantidad de lineas es: %d\n", lines);

	sys_exit();
	return 0;
}

/* ------------------------ FILTER ------------------------ */

static uint64_t is_vowel(char c) {
	return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
	        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

pid_t handle_filter(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	pid_t pid = (pid_t) sys_createProcess((uint64_t) filter, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t filter() {
	int c;

	while((c = getchar()) != EOF){
		if(!is_vowel(c)){
			putchar(c);
		}
	}
	
	sys_exit();
	return 0;
}

/* ------------------------ TEST_MM ------------------------ */

pid_t handle_test_mm(char **argv, int argc, int ground, int stdin, int stdout) {
	if (argc < 2 || !argv[1]) {
		printf("Uso: testmem <max_memory>\n");
		return -1;
	}

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	pid_t pid = (pid_t) sys_createProcess((uint64_t) run_test_mm, argv, argc, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t run_test_mm(int argc, char **argv) {
	printf("[test_mm] Iniciando test del administrador de memoria...\n");

	if (argc > 0 && argv[1] && *argv[1])
		printf("Tamanio maximo de prueba: %s bytes\n", argv[1]);

	uint64_t result = test_mm(argc, argv);

	if (result == 0)
		printf("[test_mm] Finalizado exitosamente.\n");
	else
		printf("[test_mm] Fallo con codigo: %d\n", (int) result);

	sys_exit();
	return 0;
}

/* ------------------------ TEST_PROCESSES ------------------------ */

pid_t handle_test_processes(char **argv, int argc, int ground, int stdin, int stdout) {
	int16_t fds[] = { stdin, stdout, STDERR };
	uint8_t priority = 1;

	pid_t pid = sys_createProcess((uint64_t) run_test_processes, argv, argc > 0 ? argc : 1, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t run_test_processes(int argc, char **argv) {
	printf("[test_processes] Creando, bloqueando y matando procesos dummy...\n");
	uint64_t result = test_processes(argc, argv);

	if (result == 0)
		printf("[test_processes] Completado exitosamente.\n");
	else
		printf("[test_processes] Fallo con codigo: %d\n", (int) result);

	sys_exit();
	return 0;
}

/* ------------------------ TEST_PRIORITY ------------------------ */

pid_t handle_test_priority(char **argv, int argc, int ground, int stdin, int stdout) {
	if (argc < 2 || !argv[1]) {
		printf("Uso: testprio <max_value>\n");
		return -1;
	}

	int16_t fds[] = { stdin, stdout, STDERR };
	uint8_t priority = 1;

	pid_t pid = (pid_t) sys_createProcess((uint64_t) run_test_priority, argv, argc, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t run_test_priority(int argc, char **argv) {
	printf("[test_priority] Iniciando prueba de scheduling por prioridad...\n");
	uint64_t result = test_prio(argc, argv);

	if (result == 0) {
		printf("[test_priority] Test completado exitosamente.\n");
	} else {
		printf("[test_priority] Fallo con codigo: %d\n", (int) result);
	}
	sys_exit();
	return 0;
}

/* ------------------------ TEST_SYNC ------------------------ */

pid_t handle_test_sync(char **argv, int argc, int ground, int stdin, int stdout) {
	if (argc < 3) {
		printf("Uso: testsync <iteraciones> <usar_sem>\n");
		return -1;
	}

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	pid_t pid = (pid_t) sys_createProcess((uint64_t) run_test_sync, argv, argc, priority, (char)(!ground), fds);
	return ground ? pid : 0;
}

static uint64_t run_test_sync(int argc, char **argv) {
	printf("[test_sync] Iniciando prueba de sincronizacion con semaforos...\n");
	
	if (argc > 1 && argv[1] && *argv[1]) {
		printf("Iteraciones: %s\n", argv[1]);
	}
	if (argc > 2 && argv[2] && *argv[2]) {
		printf("Usar semaforos: %s\n", argv[2]);
	}
	
	uint64_t result = test_sync(argc, argv);
	
	if (result == 0) {
		printf("[test_sync] Test completado exitosamente.\n");
	} else {
		printf("[test_sync] Fallo con codigo: %d\n", (int) result);
	}
	
	sys_exit();
	return 0;
}