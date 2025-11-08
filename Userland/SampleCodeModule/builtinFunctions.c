#include "include/builtinFunctions.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/tests.h"
#include "include/shared.h"
#include <stdint.h>


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
