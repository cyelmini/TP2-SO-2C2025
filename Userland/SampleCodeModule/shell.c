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

static void handle_piped_commands(pipeCmd *pipe_cmd);

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

typedef pid_t (*process_cmd)(char *, int, int);

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

int get_instruction_num(char *instruction) {
	for (int i = 0; i < CANT_INSTRUCTIONS; i++) {
		if (strcmp(instruction, instruction_list[i]) == 0) {
			return i;
		}
	}
	return -1;
}

int instruction_parser(char *buffer, char *arguments) {
	char *instruction = sys_mm_alloc(BUFFER * sizeof(char));
	if (instruction == NULL) {
		printErr("Error al asignar memoria para la instruccion.\n");
		return -1;
	}

	int i, j = 0;
	for (i = 0; i < BUFFER; i++) {
		if (buffer[i] == ' ' || buffer[i] == '\0' || buffer[i] == '\n') {
			instruction[j] = '\0';
			if (buffer[i] == ' ') {
				i++;
			}
			break;
		}
		else {
			instruction[j] = buffer[i];
			j++;
		}
	}

	int k = 0;
	while (buffer[i] != '\0' && buffer[i] != '\n') {
		arguments[k++] = buffer[i++];
	}
	arguments[k] = '\0';

	int instruction_num = 0;
	if ((instruction_num = get_instruction_num(instruction)) == -1 && instruction[0] != 0) {
		printErr("Error\n");
	}
	sys_mm_free(instruction);
	return instruction_num;
}

int bufferCountInstructions(pipeCmd *pipe_cmd, char *line) {
	int instructions = 0;
	char *pipe_pos = find_char(line, '|');
	if (pipe_pos != NULL) { // devuelve null si no hay pipe
		*(pipe_pos - 1) = 0;
		*pipe_pos = 0;
		*(pipe_pos + 1) = 0;
		char *arg2 = sys_mm_alloc(BUFFER * sizeof(char));
		pipe_cmd->cmd2.instruction = instruction_parser(pipe_pos + 2, arg2);
		pipe_cmd->cmd2.arguments = arg2;
		if (pipe_cmd->cmd2.instruction >= 0)
			instructions++;
	}

	char *arg1 = sys_mm_alloc(BUFFER * sizeof(char));
	pipe_cmd->cmd1.instruction = instruction_parser(line, arg1);
	pipe_cmd->cmd1.arguments = arg1;
	if (pipe_cmd->cmd1.instruction >= 0)
		instructions++;

	if (instructions>1 && IS_BUILT_IN(pipe_cmd->cmd1.instruction) && IS_BUILT_IN(pipe_cmd->cmd2.instruction)) {
		printErr("No se pueden usar comandos built-in con pipes.\n");
		sys_mm_free(pipe_cmd->cmd1.arguments);
		sys_mm_free(pipe_cmd->cmd2.arguments);
		sys_mm_free(pipe_cmd);
		return -1;
	}
	if (IS_BUILT_IN(pipe_cmd->cmd1.instruction)) {
		return 0;
	}

	return instructions;
}

static int split_args(char *args, char ***out_argv) {
	int argc = 0;
	char **argv = (char **) sys_mm_alloc(sizeof(char *) * MAX_ARGS);
	if (!argv)
		return -1;

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
	*out_argv = argv;
	return argc;
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
	pids[0] = instruction_handlers[pipe_cmd->cmd1.instruction - FONT_SIZE - 1](pipe_cmd->cmd1.arguments, STDIN, pipe_fd);
	pids[1] = instruction_handlers[pipe_cmd->cmd2.instruction - FONT_SIZE - 1](pipe_cmd->cmd2.arguments, pipe_fd, STDOUT);
	
	// Esperar a que terminen ambos procesos
	sys_waitProcess(pids[0]);
	sys_mm_free(pipe_cmd->cmd1.arguments);
	
	sys_waitProcess(pids[1]);
	sys_mm_free(pipe_cmd->cmd2.arguments);
	
	// Cerrar el pipe
	sys_pipe_close(pipe_fd);
	sys_mm_free(pipe_cmd);
}

void run_shell() {
    sys_clear();
    puts(WELCOME);

    char *line = sys_mm_alloc(MAX_CHARS * sizeof(char));
    if (line == NULL) {
        printErr("Error al asignar memoria para la linea de comandos\n");
        return;
    }

    pipeCmd *pipe_cmd;
    int instructions;

    while (1) {
        putchar('>');
        int n = read_line(line, MAX_CHARS);
        if (n <= 0) {
            continue; 
        }

        pipe_cmd = (pipeCmd *) sys_mm_alloc(sizeof(pipeCmd));
        if (pipe_cmd == NULL) {
            printErr("Error al asignar memoria para los argumentos.\n");
            return;
        }

        instructions = bufferCountInstructions(pipe_cmd, line);

        switch (instructions) {
            case 0: {
                if (pipe_cmd->cmd1.instruction == -1) {
                    printErr("Comando invalido.\n");
                } else if (IS_BUILT_IN(pipe_cmd->cmd1.instruction)) {
                    char **argv = NULL;
                    int argc = split_args(pipe_cmd->cmd1.arguments, &argv);
                    if (argc >= 0) {
                        built_in_handlers[pipe_cmd->cmd1.instruction - HELP](argc, argv);
                        sys_mm_free(argv);
                    }
                }
                sys_mm_free(pipe_cmd->cmd1.arguments);
                sys_mm_free(pipe_cmd);
            } break;

            case 1: {
                pid_t pid = instruction_handlers[pipe_cmd->cmd1.instruction - CLEAR](
                                pipe_cmd->cmd1.arguments, STDIN, STDOUT);

                if (pid < 0) {
                    printErr("Error al ejecutar el comando.\n");
                } else if (pid == 0) {
                    printf("Proceso %s ejecutado en background.\n",
                           instruction_list[pipe_cmd->cmd1.instruction]);
                } else {
                    sys_waitProcess(pid);
                }
                sys_mm_free(pipe_cmd->cmd1.arguments);
                sys_mm_free(pipe_cmd);
            } break;
            case 2:
                handle_piped_commands(pipe_cmd);
                break;

            default:
                break;
        }
    }
}