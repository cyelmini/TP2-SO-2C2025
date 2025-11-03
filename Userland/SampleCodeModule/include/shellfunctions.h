#ifndef SHELLFUNCTIONS_H
#define SHELLFUNCTIONS_H

#include "shared.h"
#include <stddef.h>
#include <stdint.h>

typedef struct command {
	int instruction;
	char *arguments;
	char foreground; // 1 si es foreground, 0 si es background
} command;

typedef struct pipecmd {
	command cmd1;
	command cmd2;
} pipeCmd;

/* -------------------------------------------------------------------
   BUILT-IN COMMANDS
   ------------------------------------------------------------------- */

/*
 * Todos los built-ins comparten esta firma para poder despacharse
 * desde el mismo arreglo de punteros a función.
 */

void bi_help(int argc, char **argv);
void bi_mem(int argc);
void bi_kill(int argc, char **argv);
void bi_block(int argc, char **argv);
void bi_unblock(int argc, char **argv);
void bi_nice(int argc, char **argv);
void bi_fontSize(int argc, char **argv);

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
pid_t handle_clear(int stdin, int stdout);
pid_t handle_ps(char *arg, int stdin, int stdout);
pid_t handle_loop(char *arg, int stdin, int stdout);
pid_t handle_cat(int stdin, int stdout);
pid_t handle_wc(int stdin, int stdout);
pid_t handle_filter(int stdin, int stdout);
pid_t handle_mvar(char *arg, int stdin, int stdout);
pid_t handle_test_mm(char *arg, int stdin, int stdout);
pid_t handle_test_processes(char *arg, int stdin, int stdout);
pid_t handle_test_priority(char *arg, int stdin, int stdout);
pid_t handle_test_sync(char *arg, int stdin, int stdout);
pid_t handle_test_no_sync(char *arg, int stdin, int stdout);

#endif // SHELLFUNCTIONS_H