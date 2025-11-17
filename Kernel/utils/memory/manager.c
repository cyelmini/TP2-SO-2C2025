// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "../../include/memoryManagement.h"
#include <stdint.h>
#include <string.h>

#define FREE 0
#define USED 1
#define BORDER 2
#define BLOCK_SIZE 64
#define BYTE_SIZE 8

typedef struct MemoryManagerCDT {
    uint8_t *vec;
	void *realMemStart;
	uint64_t blockCount;
	uint64_t usedBlocksCount;
} MemoryManagerCDT;

typedef struct MemoryManagerCDT *MemoryManagerADT;
static void *memoryBaseAddress;


static MemoryManagerADT getMemoryManager(void){
    return (MemoryManagerADT)memoryBaseAddress;  
}

MemoryManagerADT mm_create(void *const restrict startAddress, uint64_t totalSize){
    memoryBaseAddress = startAddress;
    MemoryManagerADT manager = (MemoryManagerADT) memoryBaseAddress;
    
    uint64_t structsize = sizeof(MemoryManagerCDT);
    if(totalSize <= structsize){
        return NULL;
    }

    manager->blockCount = (totalSize - structsize) / (BLOCK_SIZE + 1);
    if(manager->blockCount == 0){
        return NULL;
    }

    manager->usedBlocksCount = 0;
    manager->vec = (uint8_t *) memoryBaseAddress + structsize;
    manager->realMemStart = (void *) ((uint8_t *) manager->vec + manager->blockCount);
    for(uint64_t i = 0 ; i < manager->blockCount ; i++){
        manager->vec[i] = FREE;
    }

    return manager;
}

void *mm_alloc(size_t size){
    if (size == 0)return NULL;
    
    MemoryManagerADT manager = (MemoryManagerADT) memoryBaseAddress;
    if(manager->blockCount == 0 || manager->vec == NULL|| manager->realMemStart == NULL) return NULL;
    
    uint64_t total_needed = (uint64_t) size;
    uint64_t needed_blocks = (total_needed + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if(needed_blocks == 0 || needed_blocks > manager->blockCount) return NULL;
    
    uint64_t freeBlocks = 0;
	for (uint64_t i = 0; i < manager->blockCount; i++) {
		if (manager->vec[i] == FREE) {
			freeBlocks++;
			if (freeBlocks == needed_blocks) {
				uint64_t start = i - freeBlocks + 1;

				manager->vec[start] = BORDER;
				for (uint64_t j = start + 1; j <= i; j++) {
					manager->vec[j] = USED;
				}

				manager->usedBlocksCount += needed_blocks;

				return (uint8_t *) manager->realMemStart + start * BLOCK_SIZE;
			}
		}
		else {
			freeBlocks = 0;
		}
	}
	return NULL; 

}

void mm_free(void *const restrict ptr){
    if(ptr == NULL){
        return;
    }

    MemoryManagerADT manager = getMemoryManager();
    if(manager == NULL){
        return;
    }
    
    uint8_t *start = (uint8_t *) manager->realMemStart;
    
    if((uint8_t *)ptr < start){
        return;
    }

    uint64_t i = ((uint8_t *) ptr - start) / BLOCK_SIZE;

    if(i >= manager->blockCount || manager->vec[i] != BORDER){
        return;
    }

    manager->vec[i] = FREE;
    manager->usedBlocksCount--;

    for(uint64_t j = i + 1; j < manager->blockCount && manager->vec[j] == USED; j++){
        manager->vec[j] = FREE;
        manager->usedBlocksCount--;

    }
}

