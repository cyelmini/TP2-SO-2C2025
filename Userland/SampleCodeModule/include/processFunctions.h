#ifndef PROCESSFUNCTIONS_H
#define PROCESSFUNCTIONS_H

#include "shared.h"
#include <stddef.h>
#include <stdint.h>

#define EOF (-1)

typedef struct command {
	int instruction;
	char ** arguments;
	int argc;
	char ground; // 1 si es foreground, 0 si es background
} command;

typedef struct pipecmd {
	command cmd1;
	command cmd2;
} pipeCmd;

/* -------------------------------------------------------------------
   USER-SPACE APPLICATION HANDLERS
   ------------------------------------------------------------------- */

/*
 * Cada handler lanza un nuevo proceso de usuario.
 * Reciben:
 *   - arg: cadena con los argumentos (sin dividir)
 *   - stdin/stdout: file descriptors para redirección o pipe
 * Devuelven:
 *   - pid > 0  → proceso en foreground
 *   - pid = 0  → proceso en background
 *   - pid < 0  → error
 */
pid_t handle_clear(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_ps(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_loop(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_cat(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_wc(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_filter(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_test_mm(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_test_processes(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_test_priority(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_test_sync(char **argv, int argc, int ground, int stdin, int stdout);
pid_t handle_test_no_sync(char **argv, int argc, int ground, int stdin, int stdout);

#endif // PROCESSFUNCTIONS_H