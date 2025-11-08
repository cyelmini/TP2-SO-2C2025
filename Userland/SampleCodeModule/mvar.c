#include "include/shell.h"
#include "include/processFunctions.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/shared.h"
#include <stdint.h>

/* Identificadores reservados para los semaforos utilizados por la MVar */
#define MVAR_SEM_EMPTY 90
#define MVAR_SEM_FULL 91
#define MVAR_SEM_MUTEX 92
#define MVAR_SEM_PRINT 93

/* Estado compartido de la celda MVar */
static volatile char mvar_cell = 0;
static volatile uint32_t mvar_total_writers = 0;
static volatile uint32_t mvar_total_readers = 0;

/* Paleta de colores para distinguir lectores en la salida */
static const Color reader_palette[] = {
	{0, 0, 255},      /* Red */
	{0, 255, 0},      /* Green */
	{255, 0, 0},      /* Blue */
	{0, 255, 255},    /* Yellow */
	{255, 0, 255},    /* Magenta */
	{255, 255, 255}   /* White */
};

static const char *const reader_palette_names[] = {
	"red",
	"green",
	"blue",
	"yellow",
	"magenta",
	"white"
};

#define READER_PALETTE_SIZE (sizeof(reader_palette) / sizeof(reader_palette[0]))

static uint64_t mvar_manager(int argc, char **argv);
static uint64_t mvar_writer(int argc, char **argv);
static uint64_t mvar_reader(int argc, char **argv);
static void random_busy_wait(uint32_t salt);
static char writer_value(uint32_t writer_id);
static Color reader_color(uint32_t reader_id);
static int init_semaphores(void);
static void destroy_semaphores(void);

static void random_busy_wait(uint32_t salt) {
	uint64_t ticks = sys_getTicks();
	uint64_t mix = ((ticks << 3) ^ (salt * 1103515245u + 12345u));
	uint64_t loops = (mix % 40000u) + 20000u;
	volatile uint64_t sink = 0;
	for (uint64_t i = 0; i < loops; i++) {
		sink += i;
	}
}

static char writer_value(uint32_t writer_id) {
	if (writer_id < 26) {
		return (char) ('A' + writer_id);
	}
	writer_id -= 26;
	if (writer_id < 26) {
		return (char) ('a' + writer_id);
	}
	writer_id -= 26;
	return (char) ('0' + (writer_id % 10));
}

static Color reader_color(uint32_t reader_id) {
	if (READER_PALETTE_SIZE == 0) {
		Color fallback = {255, 255, 255};
		return fallback;
	}
	return reader_palette[reader_id % READER_PALETTE_SIZE];
}

static void destroy_semaphores(void) {
	sys_sem_destroy(MVAR_SEM_EMPTY);
	sys_sem_destroy(MVAR_SEM_FULL);
	sys_sem_destroy(MVAR_SEM_MUTEX);
	sys_sem_destroy(MVAR_SEM_PRINT);
}

static int init_semaphores(void) {
	destroy_semaphores();
	if (sys_sem_create(MVAR_SEM_EMPTY, 1) != 0) {
		return -1;
	}
	if (sys_sem_create(MVAR_SEM_FULL, 0) != 0) {
		destroy_semaphores();
		return -1;
	}
	if (sys_sem_create(MVAR_SEM_MUTEX, 1) != 0) {
		destroy_semaphores();
		return -1;
	}
	if (sys_sem_create(MVAR_SEM_PRINT, 1) != 0) {
		destroy_semaphores();
		return -1;
	}
	return 0;
}

static uint64_t mvar_manager(int argc, char **argv) {
	if (argc < 3 || argv == NULL || argv[1] == NULL || argv[2] == NULL) {
		printf("Uso: mvar <writers> <readers>\n");
		sys_exit();
		return -1;
	}

	uint32_t writers = str_to_uint32(argv[1]);
	uint32_t readers = str_to_uint32(argv[2]);
	if (writers == 0 || readers == 0) {
		printf("Uso: mvar <writers> <readers> (valores > 0)\n");
		sys_exit();
		return -1;
	}

	mvar_total_writers = writers;
	mvar_total_readers = readers;
	mvar_cell = 0;

	if (init_semaphores() != 0) {
		printf("[mvar] Error inicializando semaforos.\n");
		sys_exit();
		return -1;
	}

	int16_t fds[] = {STDIN, STDOUT, STDERR};
	uint8_t proc_priority = 3;
	char background = 1;

	for (uint32_t i = 0; i < writers; i++) {
		char id_buf[12];
		itoa((uint64_t) i, id_buf, 10);
		char name_buf[16];
		strcpy(name_buf, "writer-");
		name_buf[7] = writer_value(i);
		name_buf[8] = '\0';
		char *writer_args[] = {name_buf, id_buf, NULL};

		pid_t pid = (pid_t) sys_createProcess((uint64_t) mvar_writer, writer_args, 2,
			proc_priority, background, fds);
		if (pid < 0) {
			if (sys_sem_wait(MVAR_SEM_PRINT) == 0) {
				printf("[mvar] Error al crear escritor %u.\n", (uint64_t) i);
				sys_sem_post(MVAR_SEM_PRINT);
			} else {
				printf("[mvar] Error al crear escritor %u.\n", (uint64_t) i);
			}
		}
	}

	for (uint32_t i = 0; i < readers; i++) {
		char id_buf[12];
		itoa((uint64_t) i, id_buf, 10);
		const char *label = reader_palette_names[i % READER_PALETTE_SIZE];
		char name_buf[24];
		strcpy(name_buf, "reader-");
		strcpy(name_buf + 7, label);
		char *reader_args[] = {name_buf, id_buf, NULL};

		pid_t pid = (pid_t) sys_createProcess((uint64_t) mvar_reader, reader_args, 2,
			proc_priority, background, fds);
		if (pid < 0) {
			if (sys_sem_wait(MVAR_SEM_PRINT) == 0) {
				printf("[mvar] Error al crear lector %u.\n", (uint64_t) i);
				sys_sem_post(MVAR_SEM_PRINT);
			} else {
				printf("[mvar] Error al crear lector %u.\n", (uint64_t) i);
			}
		}
	}

	sys_exit();
	return 0;
}

static uint64_t mvar_writer(int argc, char **argv) {
	if (argc < 2 || argv == NULL || argv[1] == NULL) {
		sys_exit();
		return -1;
	}

	uint32_t writer_id = str_to_uint32(argv[1]);

	while (1) {
		random_busy_wait(writer_id + 1);

		if (sys_sem_wait(MVAR_SEM_EMPTY) != 0) {
			break;
		}
		if (sys_sem_wait(MVAR_SEM_MUTEX) != 0) {
			sys_sem_post(MVAR_SEM_EMPTY);
			break;
		}

		mvar_cell = writer_value(writer_id);

		sys_sem_post(MVAR_SEM_MUTEX);
		sys_sem_post(MVAR_SEM_FULL);
	}

	sys_exit();
	return 0;
}

static uint64_t mvar_reader(int argc, char **argv) {
	if (argc < 2 || argv == NULL || argv[1] == NULL) {
		sys_exit();
		return -1;
	}

	uint32_t reader_id = str_to_uint32(argv[1]);
	Color color = reader_color(reader_id);

	while (1) {
		random_busy_wait(reader_id + mvar_total_writers + 7);

		if (sys_sem_wait(MVAR_SEM_FULL) != 0) {
			break;
		}
		if (sys_sem_wait(MVAR_SEM_MUTEX) != 0) {
			sys_sem_post(MVAR_SEM_FULL);
			break;
		}

		char value = mvar_cell;
		mvar_cell = 0;

		sys_sem_post(MVAR_SEM_MUTEX);
		sys_sem_post(MVAR_SEM_EMPTY);

		if (sys_sem_wait(MVAR_SEM_PRINT) != 0) {
			break;
		}

		Color previous = sys_getFontColor();
		sys_setFontColor(color.r, color.g, color.b);
		putchar(value);
		sys_setFontColor(previous.r, previous.g, previous.b);

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

	uint32_t writers = str_to_uint32(argv[1]);
	uint32_t readers = str_to_uint32(argv[2]);
	if (writers == 0 || readers == 0) {
		printf("Uso: mvar <writers> <readers> (valores > 0)\n");
		return -1;
	}

	int16_t fds[] = {stdin, stdout, STDERR};
	uint8_t priority = 2;
	pid_t pid = (pid_t) sys_createProcess((uint64_t) mvar_manager, argv, argc,
			priority, (char) (!ground), fds);
	return ground ? pid : 0;
}