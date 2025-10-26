#include <stdint.h>
#include "include/tests.h"
#include "include/test_util.h"
#include "include/libasm.h"
#include "include/man.h"
#include "include/shell.h"
#include "include/stdio.h"
#include "include/stdlib.h"
#include "include/string.h"
#include "include/syscalls.h"

#define MAX_CHARS 256
#define BUFFER 1000
#define IS_BUILT_IN(i) ((i) < EXIT && (i) >= HELP)
#define CANT_INSTRUCTIONS 26


typedef struct command{
    int instruction;
    char * arguments;
    char foreground; // 1 si es foreground, 0 si es background
} command;

typedef struct pipecmd{
    command cmd1;
    command cmd2;
} pipeCmd;

typedef enum {
    HELP = 0,
    MAN,
    MEM,
    TIME,
    DIV,
    KABOOM,
    FONT_SIZE,
    PRINTMEM,
    CLEAR,
    ECHO, 
    KILL,
    BLOCK,
    UNBLOCK,
    EXIT,
    TEST_MM,
    TEST_PROCESSES,
    TEST_PRIORITY,
    TEST_SYNC,
    TEST_NO_SYNC,
    PS,
    LOOP,
    NICE,
    CAT,
    WC,
    FILTER,
    MVAR,
} instructions;

pid_t (*instruction_handlers[CANT_INSTRUCTIONS-4])(char *, int, int) = {
    handle_echo,
    handle_loop,
    handle_cat,
    handle_wc,
    handle_filter,
    handle_mvar,
    handle_testmm,
    handle_testprocesses,
    handle_testpriority,
    handle_testsync,
    handle_testnosync,
};

void (*built_in_handlers[(EXIT-HELP)])(char *) = {
    help,
    man,
    fontSize,
    clear,
    exit,
    block,
    unblock,
    kill,
    nice,
    ps,
    mem,
};

static char * instruction_list[] = {
    "help",
    "echo",
    "clear",
    "testmem",
    "testproc",
    "testprio",
    "testsync",
    "testnosync",
    "mem",
    "ps",
    "loop",
    "nice",
    "cat",
    "wc",
    "filter",
    "mvar",
    "kill",
    "block",
    "unblock",
    "exit"
};

static void help(int argc, char *argv[]) {
	if (argc != 0) {
		printf("Help doesn't need parameters\n");
		return;
	}

	const char *manual =
		"-------------COMENTARIOS-------------\n"
        "Para ejecutar un proceso en segundo plano, escriba '&' al final del comando.\n"
        "Para conectar dos procesos mediante un pipe, utilice el símbolo '|'.\n\n"

        "-------------BUILT-INS-------------\n"
        "HELP                       Muestra el listado de comandos disponibles.\n"
        "MAN                        Muestra el manual de uso de los comandos.\n"
        "FONT-SIZE                  Cambia el tamaño de la fuente. Uso: FONT-SIZE <número>.\n"
        "CLEAR                      Limpia completamente la pantalla.\n"
        "EXIT                       Cierra la shell actual.\n\n"
        "BLOCK                      Bloquea un proceso dado su ID.\n"
        "UNBLOCK                    Desbloquea un proceso dado su ID.\n"
        "KILL                       Mata un proceso dado su ID.\n"
        "NICE                       Cambia la prioridad de un proceso dado su ID y la nueva prioridad.\n"
        "PS                         Lista todos los procesos en ejecución con sus propiedades.\n"
        "MEM                        Muestra el estado de la memoria: total, ocupada y libre.\n"

        "-------------APLICACIONES DE USUARIO-------------\n"
        "ECHO                       Imprime los argumentos pasados por parámetro.\n"   
        "LOOP                       Imprime su ID con un saludo cada cierta cantidad de segundos.\n"
        "CAT                        Imprime el contenido recibido por la entrada estándar (stdin).\n"
        "WC                         Cuenta la cantidad de líneas recibidas por la entrada estándar.\n"
        "FILTER                     Filtra las vocales del texto recibido por la entrada estándar.\n"
        "MVAR                       Simula múltiples lectores y escritores sobre una variable compartida, garantizando acceso exclusivo y sincronización.\n\n"

        "-------------TESTS DEL SISTEMA-------------\n"
        "TEST_MM                    Prueba el administrador de memoria física.\n"
        "TEST_PROCESSES             Crea, bloquea, desbloquea y mata procesos dummy aleatoriamente.\n"
        "TEST_PRIORITY              Crea 3 procesos que incrementan una variable desde 0 hasta un valor dado, mostrando las diferencias según su prioridad.\n"
        "TEST_SYNCHRO               Prueba la sincronización usando semáforos. Uso: TEST_SYNCHRO <iteraciones> <usar_sem>.\n"
        "TEST_NOSYNCHRO             Prueba la ausencia de sincronización. Uso: TEST_NOSYNCHRO <iteraciones>.\n"
        "MVAR                       Prueba las variables compartidas entre procesos.\n"     ;  
        
    printf("%s", manual);
}

static void man(int argc, char *argv[]) {
    
}

int get_instruction_num(char * instruction){
    for(int i = 0; i < CANT_INSTRUCTIONS; i++){
        if(strcmp(instruction, instruction_list[i]) == 0){
            return i;
        }
    }
    return -1;
}

int instruction_parser(char * buffer, char * arguments){
    char * instruction = malloc(BUFFER * sizeof(char));
    if(instruction == NULL) {
        printErr("Error al asignar memoria para la instruccion.\n");
        return -1;
    }
    
    int i, j = 0;
    for(i = 0; i < BUFFER; i++){
        if(buffer[i] == ' ' || buffer[i] == '\0' || buffer[i] == '\n'){
            instruction[j] = '\0';
            i++;
            break;
        }else{
            instruction[j] = buffer[i];
            j++;
        }
    }


    int k = 0;
    while (buffer[i] != '\0' && buffer[i] != '\n') {
        arguments[k++] = buffer[i++];
    }
    arguments[k] = '\0';
    free(buffer);

    int instruction_num = 0;
    if((instruction_num = get_instruction_num(instruction)) == -1 && instruction[0] != 0){
        printferror("Comando no reconocido: %s\n", instruction);
    }
    free(instruction);
    return instruction_num;
}

int bufferCountInstructions(pipeCmd * pipe_cmd, char * line){
    int instructions = 0;
    char * pipe_pos = strstr(line, "|");
    if(pipe_pos != NULL){ // devuelve null si no hay pipe
        *(pipe_pos-1) = 0;
        *pipe_pos = 0;
        *(pipe_pos+1) = 0;
        char * arg2 = malloc(BUFFER * sizeof(char));
        pipe_cmd->cmd2.instruction = instruction_parser(pipe_pos+2, arg2);
        pipe_cmd->cmd2.arguments = arg2;
        if(pipe_cmd->cmd2.instruction >= 0) instructions++;
    }

    char * arg1 = malloc(BUFFER * sizeof(char));
    pipe_cmd->cmd1.instruction = instruction_parser(line, arg1);
    pipe_cmd->cmd1.arguments = arg1;
    if(pipe_cmd->cmd1.instruction >= 0) instructions++;

    if(IS_BUILT_IN(pipe_cmd->cmd1.instruction) && IS_BUILT_IN(pipe_cmd->cmd2.instruction)){
        printferror("No se pueden usar comandos built-in con pipes.\n");
        free(pipe_cmd->cmd1.arguments);
        free(pipe_cmd->cmd2.arguments);
        free(pipe_cmd);
        return -1;
    }
    if(IS_BUILT_IN(pipe_cmd->cmd1.instruction)){
        return 0;
    }

    return instructions;
}

void run_shell() {
    syscall_clearScreen();
    puts(WELCOME);

    char *line = sys_mm_alloc(MAX_CHARS * sizeof(char *));
    if(line == NULL){
        printErr("Error al asignar memoria para la linea de comandos\n");
        return 0;
    }

    pipeCmd * pipe_cmd;
    int instructions;

    while(1){
        
        putchar('>');
        scanf("%l", line);
        
        pipe_cmd = (pipeCmd *)sys_mm_alloc(sizeof(pipeCmd));
        if(pipe_cmd == NULL){
            printErr("Error al asignar memoria para los argumentos.\n");
            return 0;
        }

        instructions = bufferCountInstructions(pipe_cmd, line);
        switch(instructions){
            case 0:
                if (pipe_cmd->cmd1.instruction == -1) {
                    printErr("Comando invalido.\n");
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                }else if(IS_BUILT_IN(pipe_cmd->cmd1.instruction)){
                    built_in_handlers[pipe_cmd->cmd1.instruction - KILL](pipe_cmd->cmd1.arguments);
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                }
                break;
            case 1:
                if (pipe_cmd->cmd1.instruction == EXIT){
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                    return 1;
                }else{
                    // handler devuelve neg si hay error, 0 si es background y pid si es foreground
                    pid_t pid = instruction_handlers[pipe_cmd->cmd1.instruction](pipe_cmd->cmd1.arguments, 0, 1);
                    if(pid < 0){
                        printErr("Error al ejecutar el comando.\n");
                    }else if (pid == 0){
                        printf("Proceso %s ejecutado en background.\n", instruction_list[pipe_cmd->cmd1.instruction]);
                    }else{
                        sys_waitProcess(pid); // tendriamos que enviar un status
                        printf("Proceso %d terminado.\n", pid);
                    }
                    free(pipe_cmd->cmd1.arguments);
                    free(pipe_cmd);
                }
                break;
            case 2:
                handle_piped_commands(pipe_cmd);
                break;
            default:
                break;
        }
    }

    printf("Saliendo de la terminal...\n");
    // algun sys call sleep (2 segundos)
    sys_clear();
}