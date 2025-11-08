#include "include/shell.h"
#include "include/processFunctions.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/shared.h"
#include <stdint.h>

static uint64_t mvar_writer(int argc, char **argv);
static uint64_t mvar_reader(int argc, char **argv);


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


pid_t handle_mvar(char **argv, int argc, int ground, int stdin, int stdout) {
	/* Expect argv: ["mvar", writers, readers]
	   Validate minimal args and forward to a spawned process as before. */
	if (argc < 3 || !argv[1] || !argv[2]) {
		printf("Uso: mvar <writers> <readers>\n");
		return -1;
	}

	int cantidad_escritores = (int) str_to_uint32(argv[1]);
	int cantidad_lectores = (int) str_to_uint32(argv[2]);
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
		arg_id[0] = '0' + (idx_lector % 10);
		arg_id[1] = '\0';
		char *rargv[] = { arg_id };
		int16_t fds_lector[] = { (int16_t) pipe_datos, STDOUT, STDERR };
		sys_createProcess((uint64_t) mvar_reader, rargv, 1, 1, 1, fds_lector);
		sys_mm_free(arg_id);
	}

	return 0;
}