#include "../../include/pipes.h"
#include "../../include/semaphore.h"
#include <stddef.h>

static pipeManager pipes;
static int next_sem_id = 0;

static int validatePipeId(int *pipeId);

void initializePipeManager() {
    pipes.next_pipe_id = 0;
    for(int i = 0; i < MAX_PIPES; i++) {
        pipes.pipes[i].isOpen = 0;
        pipes.pipes[i].readIdx = 0;
        pipes.pipes[i].writeIdx = 0;
        pipes.pipes[i].count = 0;
        pipes.pipes[i].readers = 0;
        pipes.pipes[i].writers = 0;
        pipes.pipes[i].semReaders = -1;
        pipes.pipes[i].semWriters = -1;
        pipes.pipes[i].mutex = -1; 
    }
}

int createPipe() {
    for(int i = 0; i < MAX_PIPES; i++) {
        if(!pipes.pipes[i].isOpen) {
            pipe_t *pipe = &pipes.pipes[i];
            
            pipe->readIdx = 0;
            pipe->writeIdx = 0;
            pipe->count = 0;
            pipe->readers = 0;
            pipe->writers = 0;
            pipe->isOpen = 1;
            
            pipe->semReaders = next_sem_id++;
            pipe->semWriters = next_sem_id++;
            pipe->mutex = next_sem_id++;
            
            if(sem_create(pipe->semReaders, 0) < 0 ||           
               sem_create(pipe->semWriters, PIPE_BUFFER_SIZE) < 0 || 
               sem_create(pipe->mutex, 1) < 0) {                    
                return -1;
            }
            
            if(i == 0) {
                return 0;
            } else {
                return i + 2;
            }
        }
    }
    return -1;
}

int readPipe(int pipe_id, char *buffer, int size) {
    if(validatePipeId(&pipe_id) == -1)
        return -1;
    
    if(buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_read = 0;

    for(int i = 0; i < size; i++) {
        
        sem_wait(pipe->semReaders); 
        
        sem_wait(pipe->mutex);          
        
        if(pipe->count > 0) {
            buffer[i] = pipe->buffer[pipe->readIdx];
            pipe->readIdx = (pipe->readIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count--;
            bytes_read++;
        }
        
        sem_post(pipe->mutex);
        
        sem_post(pipe->semWriters); 
    }
    
    return bytes_read;
}

int writePipe(int pipe_id, const char *buffer, int size) {
    if(validatePipeId(&pipe_id) == -1)
        return -1;
    
    if(buffer == NULL || size <= 0)
        return -1;
        
    pipe_t *pipe = &pipes.pipes[pipe_id];
    int bytes_written = 0;
    
    for(int i = 0; i < size; i++) {
        sem_wait(pipe->semWriters); 
        
        sem_wait(pipe->mutex);           
        
        if(pipe->count < PIPE_BUFFER_SIZE) {
            pipe->buffer[pipe->writeIdx] = buffer[i];
            pipe->writeIdx = (pipe->writeIdx + 1) % PIPE_BUFFER_SIZE;
            pipe->count++;
            bytes_written++;
        }
        
        sem_post(pipe->mutex);
        
        sem_post(pipe->semReaders); 
    }
    
    return bytes_written;
}

int closePipe(int pipe_id) {
    if(pipe_id == 1 || pipe_id == 2)
        return -1;

    if(validatePipeId(&pipe_id) == -1)
        return -1;

    pipe_t *pipe = &pipes.pipes[pipe_id];
    
    sem_destroy(pipe->semReaders);
    sem_destroy(pipe->semWriters);
    sem_destroy(pipe->mutex);
    
    pipe->isOpen = 0;
    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;
    pipe->readers = 0;
    pipe->writers = 0;
    pipe->semReaders = -1;
    pipe->semWriters = -1;
    pipe->mutex = -1;
    
    return 0;
}

int clearPipe(int pipe_id){
    if(validatePipeId(&pipe_id) == -1)
        return -1;
    
    pipe_t *pipe = &pipes.pipes[pipe_id];
    
    sem_wait(pipe->mutex);
    
    pipe->readIdx = 0;
    pipe->writeIdx = 0;
    pipe->count = 0;
    
    sem_post(pipe->mutex);
    
    return 0;
}

static int validatePipeId(int *pipeId) {
    int id = *pipeId;

    if (id < 0) {
        return -1;
    }

    if (id == 0) {
        if (!pipes.pipes[0].isOpen) return -1;
        *pipeId = 0;
        return 0;
    }

    if (id == 1 || id == 2) {
        return -1;
    }

    if (id > (MAX_PIPES - 1) + 2) {
        return -1;
    }

    id -= 2;
    if (!pipes.pipes[id].isOpen) {
        return -1;
    }

    *pipeId = id;
    return 0;
}