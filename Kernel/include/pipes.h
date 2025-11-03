#ifndef PIPES_H
#define PIPES_H

#include "semaphore.h"

#define MAX_PIPES 16           
#define PIPE_BUFFER_SIZE 512    

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];  // buffer circular
    int readIdx;                    
    int writeIdx;                  
    int count;                       // cantidad de datos en el buffer
    int semReaders;                  // ID del semáforo para lectores (señala datos disponibles)
    int semWriters;                  // ID del semáforo para escritores (señala espacio disponible)
    int mutex;                       // ID del mutex para acceso exclusivo al buffer
    int readers;                     // contador de lectores
    int writers;                     // contador de escritores
    int isOpen;                      // indica si el pipe está abierto
} pipe_t;

typedef struct {
    pipe_t pipes[MAX_PIPES]; // vector de pipes
    int next_pipe_id;
} pipeManager;

void initializePipeManager();

int createPipe();

int readPipe(int pipe_id, char *buffer, int size);

int writePipe(int pipe_id, const char *buffer, int size);

int closePipe(int pipe_id);

int clearPipe(int pipe_id);

#endif
