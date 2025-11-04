#include "include/shellfunctions.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/tests.h"
#include "include/shared.h"
#include <stdint.h>

typedef uint64_t (*fn)(uint64_t argc, char **argv);

static uint64_t clear();
static uint64_t ps();
static uint64_t loop(int argc, char **argv);
static uint64_t cat();
static uint64_t wc();
static int is_vowel(char c);
static uint64_t filter();
static uint64_t run_test_mm(int argc, char **argv);
static uint64_t run_test_processes(int argc, char **argv);
static uint64_t run_test_priority(int argc, char **argv);
static uint64_t run_test_sync(int argc, char **argv);

/* ------------------------ Funciones built-in de la shell ------------------------ */

void bi_help(int argc, char **argv) {
	if (argc != 0) {
		printf("El comando help no requiere argumentos.\n");
		return;
	}

	const char *manual =
		"-------------COMENTARIOS-------------\n"
		"Para ejecutar un proceso en segundo plano, escriba '&' al final del comando.\n"
		"Para conectar dos procesos mediante un pipe, utilice el simbolo '|'.\n\n"

		"-------------BUILT-INS-------------\n"
		"help               Muestra el listado de comandos disponibles.\n"
		"font-size          Cambia el tamaño de la fuente. Uso: font-size <número>.\n"
		"block              Bloquea un proceso dado su ID.\n"
		"unblock            Desbloquea un proceso dado su ID.\n"
		"kill               Mata un proceso dado su ID.\n"
		"nice               Cambia la prioridad de un proceso dado su ID y la nueva prioridad.\n"
		"mem                Muestra el estado de la memoria: total, ocupada y libre.\n\n"

		"-------------APLICACIONES DE USUARIO-------------\n"
		"clear              Limpia completamente la pantalla.\n"
		"ps                 Lista todos los procesos en ejecucion con sus propiedades.\n"
		"loop               Imprime su ID con un saludo cada cierta cantidad de segundos.\n"
		"cat                Imprime el contenido recibido por la entrada estandar (stdin).\n"
		"wc                 Cuenta la cantidad de lineas recibidas por la entrada estandar.\n"
		"filter             Filtra las vocales del texto recibido por la entrada estandar.\n"
		"mvar               Simula multiples lectores y escritores sobre una variable compartida.\n\n"

		"-------------TESTS DEL SISTEMA-------------\n"
		"testmem            Prueba el administrador de memoria fisica.\n"
		"testproc           Crea, bloquea, desbloquea y mata procesos dummy aleatoriamente.\n"
		"testprio           Crea 3 procesos que incrementan una variable desde 0 hasta un valor dado.\n"
		"testsync           Prueba la sincronizacion usando semaforos. Uso: testsync <iteraciones> "
		"<usar_sem>.\n"
		"                   - <iteraciones>: numero de incrementos/decrementos por proceso\n"
		"                   - <usar_sem>: 1 = con semaforos (resultado estable), 0 = sin semaforos (race condition).\n";

	printf("%s", manual);
}

void bi_fontSize(int argc, char **argv) {
	if (argc != 1 || argv[0] == NULL) {
		printf("Uso: font-size <n>\n");
		return;
	}
	int v = strtoi((char *) argv[0], NULL);
	if (v < MIN_FONT_SIZE || v > MAX_FONT_SIZE) {
		printf("Tamaño invalido. Rango: %d..%d\n", MIN_FONT_SIZE, MAX_FONT_SIZE);
		return;
	}
	sys_setFontSize((uint8_t) v);
}

void bi_block(int argc, char **argv) {
    if (argc < 1 || argv[0] == NULL) {
        printf("Uso: block <pid>\n");
        return;
    }

    pid_t pid = (pid_t) atoi(argv[0]);
    pid_t shellPid = (pid_t) sys_getPid();   

    // proteger idle/shell
    if (pid == 0 || pid == 1 || pid == shellPid) {
        printf("Error: no se puede bloquear la shell ni el proceso idle.\n");
        return;
    }

    int rc = sys_blockProcess(pid);
    if (rc == -1) {
        printf("Error al bloquear el proceso %d\n", (int)pid);
    } else {
        printf("Proceso %d bloqueado correctamente\n", (int)pid);
    }
}

void bi_unblock(int argc, char **argv) {
    if (argc < 1 || argv[0] == NULL) {
        printf("Uso: unblock <pid>\n");
        return;
    }
    pid_t pid = (pid_t) atoi(argv[0]);
    sys_setReadyProcess(pid);
}

void bi_kill(int argc, char **argv) {
	if (argc != 1 || argv[0] == NULL) {
		printf("Uso: kill <pid>\n");
		return;
	}
	pid_t pid = (pid_t) atoi(argv[0]);
	if(pid <= 1){
		printf("PID debe ser mayor que 1\n");
		return;
	}
	sys_killProcess(pid);
}

void bi_nice(int argc, char **argv) {
    if (argc < 2 || argv[0] == NULL || argv[1] == NULL) {
        printf("Uso: nice <pid> <priority>\n");
        return;
    }

    pid_t pid = (pid_t) atoi(argv[0]);
    int priority = (int) strtoi(argv[1], NULL);

    if (pid == 0 || pid == 1) {
        printf("No se pueden cambiar las prioridades de idle o de la shell.\n");
        return;
    }
    if (priority < MIN_PRIORITY || priority > MAX_PRIORITY) {
        printf("Prioridad invalida (rango %d..%d)\n", MIN_PRIORITY, MAX_PRIORITY);
        return;
    }

    int out = sys_changePriority(pid, priority);
    if (out == -1) {
        printf("Proceso %d no encontrado\n", (int)pid);
    }
}

void bi_mem(int argc) {
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

/* ------------------------ Funciones de aplicaciones de User Space ------------------------ */

/* ------------------------ CLEAR ------------------------ */

pid_t handle_clear(char *arg, int stdin, int stdout) {
	char *argv[] = {"clear"};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;
	return (pid_t) sys_createProcess((uint64_t) clear, argv, 1, priority, ground, fds);
}

static uint64_t clear() {
	sys_clear();
	sys_exit();
	return 0;
}

/* ------------------------ PS ------------------------ */

pid_t handle_ps(char *arg, int stdin, int stdout) {
	char *argv[] = {"ps"};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0; 
	return (pid_t) sys_createProcess((uint64_t) ps, argv, 1, priority, ground, fds);
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

typedef uint64_t (*fnptr)(uint64_t argc, char **argv);


/* ------------------------ LOOP ------------------------ */

pid_t handle_loop(char *arg, int stdin, int stdout) {
	int argc;
	char *argv[2];

	if (arg != NULL) {
        argv [0]= "loop";
		argv[1] = arg;
        argc = 2;
    } else {
        argv [0]= "loop";
		argv[1] = NULL;
        argc = 1;
    }

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0; 

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
		sys_sleep(interval);
	}
	return 0;
}

/* ------------------------ CAT ------------------------ */

pid_t handle_cat(char *arg, int stdin, int stdout) {
	char *argv[] = {"cat"};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;
	return (pid_t) sys_createProcess((uint64_t) cat, argv, 1, priority, ground, fds);
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

pid_t handle_wc(char *arg, int stdin, int stdout) {
	char *argv[] = {"wc"};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;
	return (pid_t) sys_createProcess((uint64_t) wc, argv, 1, priority, ground, fds);
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

static int is_vowel(char c) {
	return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
	        c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

pid_t handle_filter(char *arg, int stdin, int stdout) {
	char *argv[] = {"filter"};
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;
	return (pid_t) sys_createProcess((uint64_t) filter, argv, 1, priority, ground, fds);
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


/* ------------------------ MVAR ------------------------ */

pid_t handle_mvar(char *arg, int stdin, int stdout) {
	// TODO: Implementar más adelante
	return -1;
}

/* ------------------------ TEST_MM ------------------------ */

pid_t handle_test_mm(char *arg, int stdin, int stdout) {
	if (!arg || !(*arg)) {
		printf("Uso: testmem <max_memory>\n");
		return -1;
	}

	char *argv[] = {arg};
	int argc = 1;
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;

	return (pid_t) sys_createProcess((uint64_t) run_test_mm, argv, argc, priority, ground, fds);
}

static uint64_t run_test_mm(int argc, char **argv) {
	printf("[test_mm] Iniciando test del administrador de memoria...\n");

	if (argc > 0 && argv[0] && *argv[0])
		printf("Tamanio maximo de prueba: %s bytes\n", argv[0]);

	uint64_t result = test_mm(argc, argv);

	if (result == 0)
		printf("[test_mm] Finalizado exitosamente.\n");
	else
		printf("[test_mm] Fallo con codigo: %d\n", (int) result);

	sys_exit();
	return 0;
}

/* ------------------------ TEST_PROCESSES ------------------------ */

pid_t handle_test_processes(char *arg, int stdin, int stdout) {
    char *argv[] = { arg };
    int argc = (arg && *arg) ? 1 : 0;

    if (argc != 1) {
        printf("Uso: testproc <max_processes>\n");
        return -1;
    }
    int16_t fds[] = { stdin, stdout, STDERR };
    uint8_t priority = 1;
    char ground = 0; 

    return (pid_t) sys_createProcess((uint64_t) run_test_processes, argv, argc, priority, ground, fds);
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

pid_t handle_test_priority(char *arg, int stdin, int stdout) {
    char *argv[] = { arg };
    int argc = (arg && *arg) ? 1 : 0;

    if (argc != 1) {
        printf("Uso: testprio <max_value>\n");
        return -1;
    }

    int16_t fds[] = { stdin, stdout, STDERR };
    uint8_t priority = 1;
    char ground = 0; 

    return (pid_t) sys_createProcess((uint64_t) run_test_priority, argv, argc, priority, ground, fds);
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

pid_t handle_test_sync(char *arg, int stdin, int stdout) {
	char *argv[3];
	int argc = 0;
	
	if (!arg || !(*arg)) {
		printf("Uso: testsync <iteraciones> <usar_sem>\n");
		printf("  <iteraciones>: numero de incrementos/decrementos por proceso\n");
		printf("  <usar_sem>: 1 = con semaforos, 0 = sin semaforos\n");
		return -1;
	}
	
	// Separar los dos argumentos
	char *token = arg;
	char *space = NULL;
	
	// Buscar el primer espacio
	for (char *p = arg; *p; p++) {
		if (*p == ' ') {
			space = p;
			*p = '\0'; // Terminar el primer token
			break;
		}
	}
	
	if (!space) {
		printf("Error: se requieren dos argumentos\n");
		printf("Uso: testsync <iteraciones> <usar_sem>\n");
		return -1;
	}
	
	argv[0] = token;      // iteraciones
	argv[1] = space + 1;  // usar_sem
	argv[2] = NULL;
	argc = 2;
	
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0;
	
	return (pid_t) sys_createProcess((uint64_t) run_test_sync, argv, argc, priority, ground, fds);
}

static uint64_t run_test_sync(int argc, char **argv) {
	printf("[test_sync] Iniciando prueba de sincronizacion con semaforos...\n");
	
	if (argc > 0 && argv[0] && *argv[0]) {
		printf("Iteraciones: %s\n", argv[0]);
	}
	if (argc > 1 && argv[1] && *argv[1]) {
		printf("Usar semaforos: %s\n", argv[1]);
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
