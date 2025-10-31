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
	TESTSYNC,
	TESTNOSYNC
} instructions;

pid_t (*instruction_handlers[CANT_INSTRUCTIONS - 8])(char *, int, int) = {
	handle_clear,
	handle_ps,
	handle_loop,
	handle_cat,	   // todavia no
	handle_wc,	   // todavia no
	handle_filter, // todavia no
	handle_mvar,   // todavia no
	handle_test_mm,		handle_test_processes, handle_test_priority,
	handle_test_sync,	// todavia no
	handle_test_no_sync // todavia no
};

void (*built_in_handlers[])(int, char **) = {bi_help,	 bi_mem,  bi_kill,	   bi_block,
											 bi_unblock, bi_nice, bi_fontSize};

static char *instruction_list[] = {"help",	"mem",	   "kill",	   "block",	   "unblock",  "nice",		"font-size",

								   "clear",    "ps",	   "loop",	   "cat",	   "wc",		"filter",
								   "mvar",	"testmem", "testproc", "testprio", "testsync", "testnosync"};

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
			i++;
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
		printErr("Comando no reconocido\n");
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
	printf("Todavia no hay pipes. Aguante boca\n");
	return;
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
					sys_setReadyProcess(pid);
                    sys_waitProcess(pid);
                    printf("Proceso %d terminado.\n", pid);
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