#include "include/shellFunctions.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/tests.h"
#include <stdint.h>

typedef uint64_t (*fn)(uint64_t argc, char **argv);

static uint64_t echo(int argc, char **argv);
static uint64_t clear(int argc, char **argv);
static uint64_t ps(int argc, char **argv);
static uint64_t loop(int argc, char **argv);
/* cat, wc, filter, mvar  no estan implementadas*/
static uint64_t run_test_mm(int argc, char **argv);
static uint64_t run_test_processes(int argc, char **argv);
static uint64_t test_priority(int argc, char **argv);

/* ------------------------ Funciones built-in de la shell ------------------------ */

void bi_help(int argc, char **argv) {
	if (argc != 0) {
		printf("El comando help no requiere argumentos.\n");
		return;
	}

	const char *manual =
		"-------------COMENTARIOS-------------\n"
		"Para ejecutar un proceso en segundo plano, escriba '&' al final del comando.\n"
		"Para conectar dos procesos mediante un pipe, utilice el símbolo '|'.\n\n"

		"-------------BUILT-INS-------------\n"
		"HELP                       Muestra el listado de comandos disponibles.\n"
		"FONT-SIZE                  Cambia el tamaño de la fuente. Uso: FONT-SIZE <número>.\n"
		"CLEAR                      Limpia completamente la pantalla.\n"
		"EXIT                       Cierra la shell actual.\n\n"
		"BLOCK                      Bloquea un proceso dado su ID.\n"
		"UNBLOCK                    Desbloquea un proceso dado su ID.\n"
		"KILL                       Mata un proceso dado su ID.\n"
		"NICE                       Cambia la prioridad de un proceso dado su ID y la nueva prioridad.\n"
		"PS                         Lista todos los procesos en ejecución con sus propiedades.\n"
		"MEM                        Muestra el estado de la memoria: total, ocupada y libre.\n"

		"-------------APLICACIONES DE USUARIO-------------\n"
		"ECHO                       Imprime los argumentos pasados por parámetro.\n"
		"LOOP                       Imprime su ID con un saludo cada cierta cantidad de segundos.\n"
		"CAT                        Imprime el contenido recibido por la entrada estándar (stdin).\n"
		"WC                         Cuenta la cantidad de líneas recibidas por la entrada estándar.\n"
		"FILTER                     Filtra las vocales del texto recibido por la entrada estándar.\n"
		"MVAR                       Simula múltiples lectores y escritores sobre una variable compartida, garantizando "
		"acceso exclusivo y sincronización.\n\n"

		"-------------TESTS DEL SISTEMA-------------\n"
		"TEST_MM                    Prueba el administrador de memoria física.\n"
		"TEST_PROCESSES             Crea, bloquea, desbloquea y mata procesos dummy aleatoriamente.\n"
		"TEST_PRIORITY              Crea 3 procesos que incrementan una variable desde 0 hasta un valor dado, "
		"mostrando las diferencias según su prioridad.\n"
		"TEST_SYNCHRO               Prueba la sincronización usando semáforos. Uso: TEST_SYNCHRO <iteraciones> "
		"<usar_sem>.\n"
		"TEST_NOSYNCHRO             Prueba la ausencia de sincronización. Uso: TEST_NOSYNCHRO <iteraciones>.\n"
		"MVAR                       Prueba las variables compartidas entre procesos.\n";

	printf("%s", manual);
}

void bi_mem(int argc, char **argv) {
	(void) argv;
	if (argc != 0) {
		printf("Uso: mem\n");
		return;
	}
	mem_t info;
	sys_mm_info(&info);
	printf("Memoria total: %u bytes\n", info.size);
	printf("Usada: %u bytes\n", info.used);
	printf("Libre: %u bytes\n", info.free);
}

void bi_fontSize(int argc, char **argv) {
	if (argc != 1 || argv[0] == NULL) {
		printf("Uso: font-size <n>\n");
		return;
	}
	int v = strtoi((char *) argv[0], NULL);
	if (v < MIN_FONT_SIZE || v > MAX_FONT_SIZE) {
		printf("Tamaño inválido. Rango: %d..%d\n", MIN_FONT_SIZE, MAX_FONT_SIZE);
		return;
	}
	sys_setFontSize((uint8_t) v);
}

void bi_kill(int argc, char **argv) {
	if (argc < 1 || argv[0] == NULL) {
		printf("Uso: kill <pid>\n");
		return;
	}
	pid_t pid = (pid_t) atoi(argv[0]);
	sys_killProcess(pid);
}

void bi_block(int argc, char **argv) {
	if (argc < 1 || argv[0] == NULL) {
		printf("Uso: block <pid>\n");
		return;
	}
	pid_t pid = (pid_t) atoi(argv[0]);
	sys_blockProcess(pid);
}

void bi_unblock(int argc, char **argv) {
	if (argc < 1 || argv[0] == NULL) {
		printf("Uso: unblock <pid>\n");
		return;
	}
	pid_t pid = (pid_t) atoi(argv[0]);
	sys_setReadyProcess(pid);
}

void bi_nice(int argc, char **argv) {
	if (argc < 2 || argv[0] == NULL || argv[1] == NULL) {
		printf("Uso: nice <pid> <priority>\n");
		return;
	}
	pid_t pid = (pid_t) atoi(argv[0]);
	int priority = (int) strtoi(argv[1], NULL);
	sys_changePriority(pid, priority);
}

void bi_exit(int argc, char **argv) {
	(void) argc;
	(void) argv;
	uint64_t mypid = sys_getPid();
	sys_killProcess((pid_t) mypid);
}

/* ------------------------ Funciones de aplicaciones de User Space ------------------------ */

/* ------------------------ CLEAR ------------------------ */
pid_t handle_clear(char *arg, int stdin, int stdout) {
	(void) arg;
	char *argv[] = {NULL};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; /* foreground */
	return (pid_t) sys_createProcess((uint64_t) clear, argv, 0, priority, ground, fds);
}

static uint64_t clear(int argc, char **argv) {
	(void) argc;
	(void) argv;
	sys_clear();
	return 0;
}
/* ------------------------ ECHO ------------------------ */

pid_t handle_echo(char *arg, int stdin, int stdout) {
	char *argv[] = {arg};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1;

	return (pid_t) sys_createProcess((uint64_t) echo, argv, 1, priority, ground, fds);
}

uint64_t echo(int argc, char **argv) {
	if (argc == 0) {
		return 1;
	}
	printf("%s\n", *argv);
	return 0;
}

/* ------------------------ PS ------------------------ */

pid_t handle_ps(char *arg, int stdin, int stdout) {
	(void) arg;
	char *argv[] = {NULL};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; // foreground
	return (pid_t) sys_createProcess((uint64_t) ps, argv, 0, priority, ground, fds);
}

static uint64_t ps(int argc, char **argv) {
	(void) argc;
	(void) argv;

	uint16_t qty = 0;
	ProcessInfo *list = sys_processInfo(&qty);
	for (uint16_t i = 0; i < qty; i++) {
		printf("PID:%d  NAME:%s  STATUS:%d  PRIO:%d\n", (int) list[i].pid, list[i].name, (int) list[i].status,
			   (int) list[i].priority);
	}
	return 0;
}

/* ------------------------ LOOP ------------------------ */

static void sleep_seconds(uint32_t s) {
	if (s == 0)
		return;
	uint32_t start = sys_getSeconds();
	while ((uint32_t) ((sys_getSeconds() + 86400 - start) % 86400) < s) {
		sys_yield();
	}
}

pid_t handle_loop(char *arg, int stdin, int stdout) {
	char *argv[] = {arg}; /* puede venir NULL si no pasan segundos */
	int argc = (arg && *arg) ? 1 : 0;

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; /* foreground */

	return (pid_t) sys_createProcess((uint64_t) loop, argv, argc, priority, ground, fds);
}

static uint64_t loop(int argc, char **argv) {
	uint32_t interval = 1;
	if (argc >= 1 && argv[0] && *argv[0]) {
		int v = strtoi(argv[0], NULL);
		if (v > 0)
			interval = (uint32_t) v;
	}

	uint64_t mypid = sys_getPid();
	while (1) {
		printf("[loop] Hola! soy PID=%d\n", (int) mypid);
		sleep_seconds(interval);
	}
	return 0;
}

/* ------------------------ FALTAN: implementar cuando hagamos lo de sincronizacion  ------------------------ */

pid_t handle_cat(char *arg, int stdin, int stdout) {
	return -1;
} // CAT
pid_t handle_wc(char *arg, int stdin, int stdout) {
	return -1;
} // WC
pid_t handle_filter(char *arg, int stdin, int stdout) {
	return -1;
} // FILTER
pid_t handle_mvar(char *arg, int stdin, int stdout) {
	return -1;
} // MVAR

/* ------------------------ TEST_MM ------------------------ */

pid_t handle_test_mm(char *arg, int stdin, int stdout) {
	char *argv[] = {arg};
	int argc = (arg && *arg) ? 1 : 0;
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; // foreground

	return (pid_t) sys_createProcess((uint64_t) run_test_mm, argv, argc, priority, ground, fds);
}

static uint64_t run_test_mm(int argc, char **argv) {
	printf("[test_mm] Iniciando test del administrador de memoria...\n");

	if (argc > 0 && argv[0] && *argv[0])
		printf("Tamaño máximo de prueba: %s bytes\n", argv[0]);

	uint64_t result = test_mm(argc, argv);

	if (result == 0)
		printf("[test_mm] Finalizado exitosamente.\n");
	else
		printf("[test_mm] Falló con código: %d\n", (int) result);

	return 0;
}

/* ------------------------ TEST_PROCESSES ------------------------ */

pid_t handle_test_processes(char *arg, int stdin, int stdout) {
	(void) arg;
	char *argv[] = {NULL};
	int argc = 0;
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; // foreground

	return (pid_t) sys_createProcess((uint64_t) run_test_processes, argv, argc, priority, ground, fds);
}

static uint64_t run_test_processes(int argc, char **argv) {
	(void) argc;
	(void) argv;
	printf("[test_processes] Creando, bloqueando y matando procesos dummy...\n");
	uint64_t result = test_processes(argc, argv);

	if (result == 0)
		printf("[test_processes] Completado exitosamente.\n");
	else
		printf("[test_processes] Falló con código: %d\n", (int) result);

	return 0;
}

/* ------------------------ TEST_PRIORITY ------------------------ */

pid_t handle_test_priority(char *arg, int stdin, int stdout) {
	(void) arg;
	char *argv[] = {NULL};
	int argc = 0;
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 1; // foreground

	return (pid_t) sys_createProcess((uint64_t) test_priority, argv, argc, priority, ground, fds);
}

static uint64_t test_priority(int argc, char **argv) {
	(void) argc;
	(void) argv;
	printf("[test_priority] Iniciando prueba de scheduling por prioridad...\n");
	uint64_t result = test_prio(argc, argv);

	if (result == 0)
		printf("[test_priority] Test completado exitosamente.\n");
	else
		printf("[test_priority] Falló con código: %d\n", (int) result);

	return 0;
}

/* ------------------------ FALTAN: implementar cuando hagamos lo de sincronizacion  ------------------------ */

pid_t handle_test_sync(char *arg, int stdin, int stdout) {
	return -1;
}
pid_t handle_test_no_sync(char *arg, int stdin, int stdout) {
	return -1;
}
