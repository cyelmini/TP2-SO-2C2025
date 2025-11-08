#include "include/shell.h"
#include "include/libasm.h"
#include "include/man.h"
#include "include/shellfunctions.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"
#include "include/test_util.h"
#include "include/tests.h"
#include <stdint.h>

#define MAX_CHARS 256
#define BUFFER 1000
#define IS_BUILT_IN(i) ((i) >= HELP && (i) <= FONT_SIZE)
#define CANT_INSTRUCTIONS 21
#define CANT_BUILTIN 7
#define CANT_PROCESS (CANT_INSTRUCTIONS - CANT_BUILTIN -1)
#define MAX_ARGS 16

static int split_args(char *args, char **out_argv);
static void separate_cmds(char ** argv, char ** cmd1, char ** cmd2);
static int check_fore(char ** argv, int * argc);
static void remove_name(char **argv, int *argc);
static void handle_piped_commands(pipeCmd *pipe_cmd);
static void handle_process_command(char ** argv, int argc, int inst_n);

typedef enum {
	// built-in
	HELP = 0,
	MEM,
	KILL,
	BLOCK,
	UNBLOCK,
	NICE,
	FONT_SIZE,

	// user apps
	CLEAR,
	PS,
	LOOP,
	CAT,
	WC,
	FILTER,
	MVAR,
	TESTMEM,
	TESTPROC,
	TESTPRIO,
	TESTSYNC
} instructions;

typedef pid_t (*process_cmd)(char **, int, int, int, int);

static const process_cmd instruction_handlers[CANT_PROCESS] = {
	(process_cmd) handle_clear,
	(process_cmd) handle_ps,
	(process_cmd) handle_loop,
	(process_cmd) handle_cat,
	(process_cmd) handle_wc,
	(process_cmd) handle_filter,
	(process_cmd) handle_mvar,
	(process_cmd) handle_test_mm,
	(process_cmd) handle_test_processes,
	(process_cmd) handle_test_priority,
	(process_cmd) handle_test_sync,
};

typedef void (*built_in_cmd)(int, char **);

static const built_in_cmd built_in_handlers[CANT_BUILTIN] = {
	(built_in_cmd) bi_help,
	(built_in_cmd) bi_mem,
	(built_in_cmd) bi_kill,
	(built_in_cmd) bi_block,
	(built_in_cmd) bi_unblock,
	(built_in_cmd) bi_nice,
	(built_in_cmd) bi_fontSize,
};

static char *instruction_list[] = {"help",	"mem",	   "kill",	   "block",	   "unblock",  "nice",		"font-size",
								   "clear",    "ps",	   "loop",	   "cat",	   "wc",		"filter",
								   "mvar",	"testmem", "testproc", "testprio", "testsync"};

static int split_args(char *args, char **out_argv) {
	int argc = 0;
	char **argv = out_argv;

	if (!argv || !args) return -1;

	while (*args == ' ')
		args++;

	while (*args && argc < MAX_ARGS) {
		argv[argc++] = args;
		while (*args && *args != ' ')
			args++;
		if (*args == ' ') {
			*args = '\0';
			args++;
			while (*args == ' ')
				args++;
		}
	}
	/* Null-terminate remaining entries if any */
	for (int i = argc; i < MAX_ARGS; i++)
		argv[i] = NULL;
	return argc;
}

static void separate_cmds(char ** argv, char ** cmd1, char ** cmd2){
	if (!argv || !cmd1 || !cmd2) return;

	int sep = str_in_list("|", argv, MAX_ARGS);

	/* copy before separator into cmd1 */
	int c = 0;
	for (int j = 0; j < sep && j < MAX_ARGS; j++) {
		cmd1[c++] = argv[j];
	}
	cmd1[c] = NULL;

	/* copy after separator into cmd2 */
	c = 0;
	for (int j = sep + 1; j < MAX_ARGS && argv[j]; j++) {
		cmd2[c++] = argv[j];
	}
	cmd2[c] = NULL;
}

static int check_fore(char ** argv, int * argc){
	if (!argv || !argc || *argc <= 0) return 1; // default foreground
	int res = argv[*argc - 1] && argv[*argc - 1][0] == '&';
	if(res){
		argv[*argc - 1] = NULL;
		*argc -= 1;
	}
	return !res; // return 1 for foreground, 0 for background
}

static void remove_name(char **argv, int *argc) {
	if (!argv || !argc) return;
	if (*argc <= 0) return;

	/* shift left */
	for (int i = 0; i < (*argc) - 1 && i < MAX_ARGS - 1; i++) {
		argv[i] = argv[i+1];
	}

	/* last slot becomes NULL */
	int last = (*argc) - 1;
	if (last >= 0 && last < MAX_ARGS) argv[last] = NULL;

	/* decrement argc */
	(*argc)--;
	if (*argc < 0) *argc = 0;
}

static void handle_piped_commands(pipeCmd *pipe_cmd) {

	if (pipe_cmd->cmd1.instruction == -1 || pipe_cmd->cmd2.instruction == -1) {
		printErr("Comando invalido.\n");
		sys_mm_free(pipe_cmd->cmd1.arguments);
		sys_mm_free(pipe_cmd->cmd2.arguments);
		sys_mm_free(pipe_cmd);
		return;
	}
	
	if (IS_BUILT_IN(pipe_cmd->cmd1.instruction) || IS_BUILT_IN(pipe_cmd->cmd2.instruction)) {
		printErr("No se pueden usar comandos built-in con pipes.\n");
		sys_mm_free(pipe_cmd->cmd1.arguments);
		sys_mm_free(pipe_cmd->cmd2.arguments);
		sys_mm_free(pipe_cmd);
		return;
	}
	
	int pipe_fd = sys_pipe_create();
	if (pipe_fd < 0) {
		printErr("Error al crear el pipe\n");
		sys_mm_free(pipe_cmd->cmd1.arguments);
		sys_mm_free(pipe_cmd->cmd2.arguments);
		sys_mm_free(pipe_cmd);
		return;
	}
	
	// Crear ambos procesos
	// cmd1 lee de stdin (0) y escribe al pipe
	// cmd2 lee del pipe y escribe a stdout (1)
	pid_t pids[2];
	pids[0] = instruction_handlers[pipe_cmd->cmd1.instruction - FONT_SIZE - 1](pipe_cmd->cmd1.arguments, pipe_cmd->cmd1.argc, pipe_cmd->cmd1.ground, STDIN, pipe_fd);
	pids[1] = instruction_handlers[pipe_cmd->cmd2.instruction - FONT_SIZE - 1](pipe_cmd->cmd2.arguments, pipe_cmd->cmd2.argc, pipe_cmd->cmd2.ground, pipe_fd, STDOUT);
	
	// Esperar a que terminen ambos procesos
	sys_waitProcess(pids[0]);
	sys_mm_free(pipe_cmd->cmd1.arguments);
	
	sys_waitProcess(pids[1]);
	sys_mm_free(pipe_cmd->cmd2.arguments);
	
	// Cerrar el pipe
	sys_pipe_close(pipe_fd);
	sys_mm_free(pipe_cmd);
}

static void handle_process_command(char ** argv, int argc, int inst_n){
	int ground = check_fore(argv, &argc);

	pid_t pid = instruction_handlers[inst_n - CLEAR](argv, argc, ground, STDIN, STDOUT);

	if (pid < 0) {
		printErr("Error al ejecutar el comando.\n");
	} else if (pid == 0) {
		printf("Proceso %s ejecutado en background.\n", instruction_list[inst_n]);
	} else {
		sys_waitProcess(pid);
	}
}

static command set_cmd(char ** argv){
	command cmd;
	int c = 0;
	while (argv && argv[c] != NULL && c < MAX_ARGS) c++;
	int argc = c;
	int ground = check_fore(argv, &argc);
	cmd.argc = argc;
	cmd.ground = ground;
	cmd.arguments = argv;
	cmd.instruction = str_in_list(argv[0], instruction_list, CANT_INSTRUCTIONS);
	return cmd;
}

void run_shell() {
    sys_clear();
    puts(WELCOME);

    char *line = sys_mm_alloc(MAX_CHARS * sizeof(char));
    if (line == NULL) {
        printErr("Error al asignar memoria para la linea de comandos\n");
        return;
    }

    while (1) {
        putchar('>');
        int n = read_line(line, MAX_CHARS);
        if (n <= 0) {
            continue; 
        }

		char *argv[MAX_ARGS];
		int argc = split_args(line, argv);
		if (argc == 0) { continue; }

		/* comando con pipes */
		if (str_in_list("|", argv, MAX_ARGS) != -1) {
			pipeCmd *pipecmds = (pipeCmd *) sys_mm_alloc(sizeof(pipeCmd));
			if (!pipecmds) {
				printErr("Error al asignar memoria para pipeCmd\n");
				continue;
			}

			char *cmd1[MAX_ARGS];
			char *cmd2[MAX_ARGS];
			separate_cmds(argv, cmd1, cmd2);

			pipecmds->cmd1 = set_cmd(cmd1);
			pipecmds->cmd2 = set_cmd(cmd2);

			handle_piped_commands(pipecmds);
			continue;
		}

		/* comando sin pipes */
		int instruction_n = str_in_list(argv[0], instruction_list, CANT_INSTRUCTIONS);
		if (instruction_n == -1) {
			printErr("Comando invalido, ejecuta 'help' para conocer los comandos\n");
			continue;
		}

		if (IS_BUILT_IN(instruction_n)) {
			remove_name(argv, &argc);
			built_in_handlers[instruction_n - HELP](argc, argv);
			continue;
		}

		handle_process_command(argv, argc, instruction_n);
	}
}