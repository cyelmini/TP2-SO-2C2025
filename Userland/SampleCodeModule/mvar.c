// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "include/color.h"
#include "include/processFunctions.h"
#include "include/shared.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/test_util.h"
#include <stdbool.h>
#include <stdint.h>

#define MVAR_SEM_PRINT 93
#define MIN_PAUSE_STEPS 1
#define PAUSE_SPREAD_STEPS 4
#define MVAR_VALUE_BYTES 1
#define READER_PALETTE_SIZE 6

static const struct {
	Color color;
	const char *name;
}

reader_palette[READER_PALETTE_SIZE] = {{RED, "red"},   {LIGHT_GREEN, "light-green"}, {DARK_GREEN, "dark-green"},
									   {PINK, "pink"}, {MAGENTA, "magenta"},		 {SILVER, "silver"}};

static uint64_t mvar_manager(int argc, char **argv);
static uint64_t mvar_writer(int argc, char **argv);
static uint64_t mvar_reader(int argc, char **argv);

static void random_pause(void);
static char writer_value(int writer_id);
static Color reader_color(int reader_id);
static bool pipe_write_all(int pipe_id, const char *buffer, int byte_count);
static bool pipe_read_all(int pipe_id, char *buffer, int byte_count);
static int ensure_print_semaphore(void);
static void log_spawn_error(const char *role, int id);
static void spawn_writer(int id, int value_pipe_id, int16_t descriptors[], uint8_t priority, char background);
static void spawn_reader(int id, int value_pipe_id, int16_t descriptors[], uint8_t priority, char background);
static void compose_writer_args(int id, int value_pipe_id, char *name_buf, char *writer_id_buf, char *value_pipe_buf,
								char *argv_out[]);
static void compose_reader_args(int id, int value_pipe_id, char *name_buf, char *reader_id_buf, char *value_pipe_buf,
								char *argv_out[]);

static void random_pause(void) {
	int spins = MIN_PAUSE_STEPS + (int) getUniform(PAUSE_SPREAD_STEPS);
	while (spins-- > 0) {
		sys_yield();
	}
}

static char writer_value(int writer_id) {
	return (char) ('A' + (writer_id % 26));
}

static Color reader_color(int reader_id) {
	if (READER_PALETTE_SIZE == 0) {
		return BLACK;
	}
	return reader_palette[reader_id % READER_PALETTE_SIZE].color;
}

static bool pipe_write_all(int pipe_id, const char *buffer, int byte_count) {
	if (pipe_id < 0 || buffer == NULL || byte_count <= 0) {
		return false;
	}
	return sys_pipe_write(pipe_id, buffer, byte_count) == byte_count;
}

static bool pipe_read_all(int pipe_id, char *buffer, int byte_count) {
	if (pipe_id < 0 || buffer == NULL || byte_count <= 0) {
		return false;
	}
	return sys_pipe_read(pipe_id, buffer, byte_count) == byte_count;
}

static int ensure_print_semaphore(void) {
	sys_sem_destroy(MVAR_SEM_PRINT);
	return sys_sem_create(MVAR_SEM_PRINT, 1);
}

static void log_spawn_error(const char *role, int id) {
	if (sys_sem_wait(MVAR_SEM_PRINT) == 0) {
		printf("[mvar] Error al crear %s %u.\n", (char *) role, (uint64_t) id);
		sys_sem_post(MVAR_SEM_PRINT);
	}
	else {
		printf("[mvar] Error al crear %s %u.\n", (char *) role, (uint64_t) id);
	}
}

static void spawn_writer(int id, int value_pipe_id, int16_t descriptors[], uint8_t priority, char background) {
	char writer_id_buf[12];
	char value_pipe_buf[12];
	char name_buf[16];
	char *writer_argv[4];

	compose_writer_args(id, value_pipe_id, name_buf, writer_id_buf, value_pipe_buf, writer_argv);

	pid_t pid = (pid_t) sys_createProcess((uint64_t) mvar_writer, writer_argv, 3, priority, background, descriptors);
	if (pid < 0) {
		log_spawn_error("escritor", id);
	}
}

static void spawn_reader(int id, int value_pipe_id, int16_t descriptors[], uint8_t priority, char background) {
	char reader_id_buf[12];
	char value_pipe_buf[12];
	char name_buf[24];
	char *reader_argv[4];
	compose_reader_args(id, value_pipe_id, name_buf, reader_id_buf, value_pipe_buf, reader_argv);

	pid_t pid = (pid_t) sys_createProcess((uint64_t) mvar_reader, reader_argv, 3, priority, background, descriptors);
	if (pid < 0) {
		log_spawn_error("lector", id);
	}
}

static void compose_writer_args(int id, int value_pipe_id, char *name_buf, char *writer_id_buf, char *value_pipe_buf,
								char *argv_out[]) {
	itoa((uint64_t) id, writer_id_buf, 10);
	itoa((uint64_t) value_pipe_id, value_pipe_buf, 10);
	strcpy(name_buf, "writer-");
	name_buf[7] = writer_value(id);
	name_buf[8] = '\0';

	argv_out[0] = name_buf;
	argv_out[1] = writer_id_buf;
	argv_out[2] = value_pipe_buf;
	argv_out[3] = NULL;
}

static void compose_reader_args(int id, int value_pipe_id, char *name_buf, char *reader_id_buf, char *value_pipe_buf,
								char *argv_out[]) {
	const char *reader_label = reader_palette[id % READER_PALETTE_SIZE].name;
	itoa((uint64_t) id, reader_id_buf, 10);
	itoa((uint64_t) value_pipe_id, value_pipe_buf, 10);
	strcpy(name_buf, "reader-");
	strcpy(name_buf + 7, reader_label);

	argv_out[0] = name_buf;
	argv_out[1] = reader_id_buf;
	argv_out[2] = value_pipe_buf;
	argv_out[3] = NULL;
}

static uint64_t mvar_manager(int argc, char **argv) {
	if (argc < 3 || argv == NULL || argv[1] == NULL || argv[2] == NULL) {
		printf("Uso: mvar <writers> <readers>\n");
		sys_exit();
		return -1;
	}

	int writer_count = (int) str_to_uint32(argv[1]);
	int reader_count = (int) str_to_uint32(argv[2]);
	if (writer_count == 0 || reader_count == 0) {
		printf("Uso: mvar <writers> <readers> (valores > 0)\n");
		sys_exit();
		return -1;
	}

	if (ensure_print_semaphore() != 0) {
		printf("[mvar] Error creando semaforo de impresion.\n");
		sys_exit();
		return -1;
	}

	int value_pipe_id = sys_pipe_create();
	if (value_pipe_id < 0) {
		printf("[mvar] Error creando pipe principal.\n");
		sys_exit();
		return -1;
	}

	int16_t io_descriptors[] = {STDIN, STDOUT, STDERR};
	uint8_t process_priority = 3;
	char background = 1;

	for (int i = 0; i < writer_count; i++) {
		spawn_writer(i, value_pipe_id, io_descriptors, process_priority, background);
	}

	for (int i = 0; i < reader_count; i++) {
		spawn_reader(i, value_pipe_id, io_descriptors, process_priority, background);
	}

	sys_exit();
	return 0;
}

static uint64_t mvar_writer(int argc, char **argv) {
	if (argc < 3 || argv == NULL || argv[1] == NULL || argv[2] == NULL) {
		sys_exit();
		return -1;
	}

	int writer_id = (int) str_to_uint32(argv[1]);
	int value_pipe = atoi(argv[2]);
	char produced_value;

	while (1) {
		random_pause();

		produced_value = writer_value(writer_id);
		if (!pipe_write_all(value_pipe, &produced_value, MVAR_VALUE_BYTES)) {
			break;
		}
	}

	sys_exit();
	return 0;
}

static uint64_t mvar_reader(int argc, char **argv) {
	if (argc < 3 || argv == NULL || argv[1] == NULL || argv[2] == NULL) {
		sys_exit();
		return -1;
	}

	int reader_id = (int) str_to_uint32(argv[1]);
	int value_pipe = atoi(argv[2]);
	Color reader_font = reader_color(reader_id);
	char consumed_value;

	while (1) {
		random_pause();

		if (!pipe_read_all(value_pipe, &consumed_value, MVAR_VALUE_BYTES)) {
			break;
		}

		if (sys_sem_wait(MVAR_SEM_PRINT) != 0) {
			break;
		}

		Color previous_font = sys_getFontColor();
		sys_setFontColor(reader_font.r, reader_font.g, reader_font.b);
		putchar(consumed_value);
		sys_setFontColor(previous_font.r, previous_font.g, previous_font.b);

		sys_sem_post(MVAR_SEM_PRINT);
	}

	sys_exit();
	return 0;
}

pid_t handle_mvar(char **argv, int argc, int ground, int stdin, int stdout) {
	if (argc < 3 || argv[1] == NULL || argv[2] == NULL) {
		printf("Uso: mvar <writers> <readers>\n");
		return -1;
	}

	int writer_count = (int) str_to_uint32(argv[1]);
	int reader_count = (int) str_to_uint32(argv[2]);
	if (writer_count == 0 || reader_count == 0) {
		printf("Uso: mvar <writers> <readers> (valores > 0)\n");
		return -1;
	}

	if (writer_count > 26) {
		printf("Uso: mvar <writers> <readers> (max writers = 26)\n");
		return -1;
	}

	int16_t descriptors[] = {stdin, stdout, STDERR};
	uint8_t priority = 2;
	pid_t manager_pid =
		(pid_t) sys_createProcess((uint64_t) mvar_manager, argv, argc, priority, (char) (!ground), descriptors);
	return ground ? manager_pid : 0;
}