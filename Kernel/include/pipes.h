#ifndef PIPES_H
#define PIPES_H

#include "semaphore.h"

#define MAX_PIPES 16           
#define PIPE_BUFFER_SIZE 512    

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];  
    int readIdx;                    
    int writeIdx;                  
    int count;                       
    int semReaders;                  
    int semWriters;                  
    int mutex;         
    int readers;             
    int writers;             
    int isOpen;            
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES]; // vector de pipes
    int next_pipe_id;
} pipeManager;

/**
 * @brief Inicializa el gestor de pipes del sistema
 */
void initializePipeManager();

/**
 * @brief Crea un nuevo pipe
 * @return ID del pipe creado o -1 en caso de error
 */
int createPipe();

/**
 * @brief Lee datos de un pipe
 * @param pipe_id ID del pipe del cual leer
 * @param buffer Buffer donde se almacenarán los datos leídos
 * @param size Cantidad de bytes a leer
 * @return Cantidad de bytes leídos o -1 en caso de error
 */
int readPipe(int pipe_id, char *buffer, int size);

/**
 * @brief Escribe datos en un pipe
 * @param pipe_id ID del pipe en el cual escribir
 * @param buffer Buffer con los datos a escribir
 * @param size Cantidad de bytes a escribir
 * @return Cantidad de bytes escritos o -1 en caso de error
 */
int writePipe(int pipe_id, const char *buffer, int size);

/**
 * @brief Cierra un pipe
 * @param pipe_id ID del pipe a cerrar
 * @return 0 en caso de éxito, -1 en caso de error
 */
int closePipe(int pipe_id);

/**
 * @brief Limpia el contenido de un pipe
 * @param pipe_id ID del pipe a limpiar
 * @return 0 en caso de éxito, -1 en caso de error
 */
int clearPipe(int pipe_id);

#endif
