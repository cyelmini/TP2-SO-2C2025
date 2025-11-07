#include "include/shellfunctions.h"
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
static uint64_t mvar_writer(int argc, char **argv);
static uint64_t mvar_reader(int argc, char **argv);
static int my_isdigit(char c);
static uint32_t str_to_uint32(char *str) ;
static int checkparamsloop(char *arg);
static int checkparams(char *arg);
static int checkparamstest(char *arg);
static char findground(char *arg);


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
	char ground = 0;
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;

	if(arg != NULL && *arg != '\0'){
		ground = findground(arg);
		if(checkparams(arg) == -1){
			printf("Uso: clear [&]\n");
			return -1;
		}
	}

	char *argv[] = {"clear"};

	pid_t pid = sys_createProcess((uint64_t) clear, argv, 1, priority, ground, fds);
	return !ground ? pid : 0 ;
}

static uint64_t clear() {
	sys_clear();
	sys_exit();
	return 0;
}

/* ------------------------ PS ------------------------ */

pid_t handle_ps(char *arg, int stdin, int stdout) {
	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0; 

	if(arg != NULL && *arg != '\0'){
		ground = findground(arg);
		if(checkparams(arg) == -1){
			printf("Uso: ps [&]\n");
			return -1;
		}
	}

	char *argv[] = {"ps"};

	pid_t pid = sys_createProcess((uint64_t) ps, argv, 1, priority, ground, fds);
	return !ground ? pid : 0;
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

pid_t handle_loop(char *arg, int stdin, int stdout) {
	int argc;
	char *argv[3];
	argv[0]="loop";

	if (arg != NULL && *arg != '\0') {
		argc = 2;
		argv[1] = arg;
	} else {
		argc = 1;
	}

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 1;
	char ground = 0; 

	if(arg != NULL && *arg != '\0'){
		ground = findground(arg);
		if(checkparamsloop(arg) == -1){
			printf("Uso: loop [intervalo] [&]\n");
			return -1;
		}
	}


	pid_t pid = sys_createProcess((uint64_t) loop, argv, argc, priority, ground, fds);
	return !ground ? pid : 0;
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

static uint64_t is_vowel(char c) {
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
	if (!arg || !(*arg)) {
		printf("Uso: mvar <writers> <readers>\n");
		return -1;
	}

	// parse args: two integers
	char *entrada = arg;
	char *separador = NULL;
	for (char *p = arg; *p; p++) {
		if (*p == ' ') {
			separador = p;
			*p = '\0';
			break;
		}
	}
	if (!separador) {
		printf("Uso: mvar <writers> <readers>\n");
		return -1;
	}
	char *escritores_str = entrada;
	char *lectores_str = separador + 1;

	int cantidad_escritores = (int) str_to_uint32(escritores_str);
	int cantidad_lectores = (int) str_to_uint32(lectores_str);
	if (cantidad_escritores <= 0 || cantidad_lectores <= 0) {
		printf("Uso: mvar <writers> <readers> (valores > 0)\n");
		return -1;
	}

	int pipe_datos = sys_pipe_create();
	if (pipe_datos < 0) {
		printf("Error creando pipe_datos\n");
		return -1;
	}

	for (int idx_escritor = 0; idx_escritor < cantidad_escritores; idx_escritor++) {
		char *arg_letra = (char *) sys_mm_alloc(2);
		if (!arg_letra) continue;
		arg_letra[0] = 'A' + (idx_escritor % 26);
		arg_letra[1] = '\0';
		char *wargv[] = { arg_letra };
		int16_t fds_escritor[] = { STDIN, (int16_t) pipe_datos, STDERR };
		sys_createProcess((uint64_t) mvar_writer, wargv, 1, 1, 1, fds_escritor);
		sys_mm_free(arg_letra);
	}

	for (int idx_lector = 0; idx_lector < cantidad_lectores; idx_lector++) {
		char *arg_id = (char *) sys_mm_alloc(4);
		if (!arg_id) continue;
		// pass an id as string
		arg_id[0] = '0' + (idx_lector % 10);
		arg_id[1] = '\0';
		char *rargv[] = { arg_id };
		int16_t fds_lector[] = { (int16_t) pipe_datos, STDOUT, STDERR };
		sys_createProcess((uint64_t) mvar_reader, rargv, 1, 1, 1, fds_lector);
		sys_mm_free(arg_id);
	}

	return 0;
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
    int argc;

    int16_t fds[] = { stdin, stdout, STDERR };
    uint8_t priority = 1;
    char ground = 0; 

	if(arg != NULL && *arg != '\0'){
		ground = findground(arg);
		if(checkparamstest(arg) == -1){
			printf("Uso: testproc <max_processes> [&]\n");
			return -1;
		}
	}

	argc = 1;

    pid_t pid = sys_createProcess((uint64_t) run_test_processes, argv, argc, priority, ground, fds);
	return !ground ? pid : 0;
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
	
	char *token = arg;
	char *space = NULL;
	
	for (char *p = arg; *p; p++) {
		if (*p == ' ') {
			space = p;
			*p = '\0'; 
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

/* ------------------------ Funciones auxiliares ------------------------ */

static int checkparamsloop(char *arg) {
    int i = 0;
    int len = strlen(arg);
    int foundAmp = 0;

    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;

    //si no hay nada -> válido
    if (i >= len)
        return 0;

    int start = i;
    while (i < len && arg[i] != ' ' && arg[i] != '\t')
        i++;
    int end = i;
    int firstLen = end - start;

    //primer argumento
    if (firstLen == 1 && arg[start] == '&') {
        foundAmp = 1;
    } else {
        for (int j = start; j < end; j++) {
            if (!my_isdigit(arg[j]))
                return -1; 
        }
    }
    
    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;
    if (i >= len)
        return 0;
    if (foundAmp)
        return -1;

    //segundo argumento
    start = i;
    while (i < len && arg[i] != ' ' && arg[i] != '\t')
        i++;
    end = i;
    int secondLen = end - start;

    //el segundo argumento debe ser exactamente "&"
    if (secondLen == 1 && arg[start] == '&') {
        foundAmp = 1;
    } else {
        return -1; // cualquier otra cosa no válida
    }

    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;

    //si hay algo más después -> inválido
    if (i < len)
        return -1;

    return 0;
}

static int checkparams(char *arg){
	int i = 0;
    int len = strlen(arg);
    while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
        i++;

    if (i >= len)
        return 0; 

    
    if (arg[i] == '&') {
        i++;
        while (i < len && (arg[i] == ' ' || arg[i] == '\t'))
            i++;
        if (i >= len)
            return 0;
        else
            return -1; // había más cosas después de &
    }

    return -1;
}

static char findground(char *arg){
	if (arg == NULL || *arg == '\0') {
		return 0;
	}

	int len = strlen(arg);
	if(arg[len-1] == '&'){
		arg[len-1] = '\0';
		arg[len-2] = '\0';
		return 1;
	}

	return 0;
}

static int my_isdigit(char c) {
    return (c >= '0' && c <= '9');
}

static uint32_t str_to_uint32(char *str) {
    uint32_t result = 0;
    int i = 0;
    int hasDigit = 0;

    while (str[i] == ' ' || str[i] == '\t')
        i++;

    while (my_isdigit(str[i])) {
        hasDigit = 1;
        result = result * 10 + (str[i] - '0');
        i++;
    }

    while (str[i] == ' ' || str[i] == '\t')
        i++;

    if (!hasDigit || str[i] != '\0')
        return -1;

    return result;
}

static int checkparamstest(char *arg) {
    int i = 0;
    int hasDigit = 0;
    int hasAmp = 0;

    while (arg[i] == ' ' || arg[i] == '\t')
        i++;

    while (my_isdigit(arg[i])) {
        hasDigit = 1;
        i++;
    }

    if (!hasDigit)
        return -1;

    while (arg[i] == ' ' || arg[i] == '\t')
        i++;

    if (arg[i] == '&') {
        hasAmp = 1;
        i++;
    }

    while (arg[i] == ' ' || arg[i] == '\t')
        i++;

    if (arg[i] != '\0')
        return -1; 

    return 0;
}

/* ------------------------ MVAR helpers (writers/readers) ------------------------ */

static uint64_t mvar_writer(int argc, char **argv) {
	char letra = 'A';
	if (argc > 0 && argv[0] && argv[0][0])
		letra = argv[0][0];
	while (1) {
		putchar((char) letra);
		sys_sleep(1);
	}
	return 0;
}

static uint64_t mvar_reader(int argc, char **argv) {
	int id_lector = 0;
	if (argc > 0 && argv[0] && argv[0][0])
		id_lector = argv[0][0] - '0';

	uint8_t colores_rgb[][3] = {
		{100, 255, 100}, // verde
		{184, 153, 255}, // rosa
		{184,   0, 255}, // magenta
		{170, 169, 173}, // plata
		{255,   0,   0}  // rojo
	};
	int colorIdx = id_lector % (sizeof(colores_rgb) / sizeof(colores_rgb[0]));

	while (1) {
		int caracter = getchar();
		if (caracter == EOF) {
			sys_exit();
			return 0;
		}

		sys_setFontColor(colores_rgb[colorIdx][0], colores_rgb[colorIdx][1], colores_rgb[colorIdx][2]);
		putchar((char) caracter);
		sys_setFontColor(255, 255, 255);
	}
	return 0;
}
