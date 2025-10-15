#ifndef _SHELL_H

#define _SHELL_H
#define QTY_COMMANDS 10

/* Enum para la cantidad de argumentos recibidos */
typedef enum { NO_PARAMS = 0, SINGLE_PARAM, DUAL_PARAM } functionType;
#define QTY_BYTES 32 /* Cantidad de bytes de respuesta del printmem */
#define DEFAULT_FONT_SIZE 1
#define MIN_FONT_SIZE 1
#define MAX_FONT_SIZE 3

#define WELCOME "Bienvenido! Ingrese 'help' para ver los comandos disponibles\n"
#define INVALID_COMMAND "Comando invalido!\n"
#define WRONG_PARAMS "La cantidad de parametros ingresada es invalida\n"
#define INVALID_FONT_SIZE "Dimension invalida de fuente\n"
#define CHECK_MAN "Escriba \"man %s\" para ver como funciona el comando\n"
#define CHECK_MAN_FONT "Escriba \"man font-size\" para ver las dimensiones validas\n"

typedef struct {
	char *name;		   // Nombre del comando
	char *description; // Descripcion del comando (para help)
	union {			   // Puntero a la funcion
		int (*f)(void);
		int (*g)(char *);
		int (*h)(char *, char *);
	};
	functionType ftype; // Cantidad de argumentos del comando
} Command;

/**
 * @brief Corre la terminal
 *
 */
void run_shell();

#endif