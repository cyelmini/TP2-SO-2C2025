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
	uint8_t *bitmap;
	void *realMemStart;
	uint64_t blockCount;
	uint64_t usedBlocksCount;
} MemoryManagerCDT;

typedef struct MemoryManagerCDT *MemoryManagerADT;

static void *memoryBaseAddress;

MemoryManagerADT mm_create(void *const restrict startAddress, uint64_t totalSize) {
	memoryBaseAddress = startAddress;
	MemoryManagerADT manager = (MemoryManagerADT) memoryBaseAddress;

	uint64_t structSize = sizeof(MemoryManagerCDT);
	if (totalSize <= structSize) {
		return NULL;
	}

	manager->blockCount = (totalSize - structSize) / (BLOCK_SIZE + 1);
	if (manager->blockCount == 0) {
		return NULL;
	}
	manager->usedBlocksCount = 0;
	manager->bitmap =
		(uint8_t *) memoryBaseAddress + structSize; // El bitmap arranca después del espacio asignado al struct
	manager->realMemStart =
		(void *) ((uint8_t *) manager->bitmap +
				  manager->blockCount); // El espacio de memoria "usable" arranca después del espacio asignado al bitmap

	for (uint64_t i = 0; i < manager->blockCount; i++) {
		manager->bitmap[i] = FREE;
	}

	return manager;
}

static MemoryManagerADT getMemoryManager(void) {
	return (MemoryManagerADT) memoryBaseAddress;
}

void *mm_alloc(const size_t bytes) {
	MemoryManagerADT manager = getMemoryManager();
	if (manager == NULL || bytes == 0 || bytes > POW2(17)) {
		return NULL;
	}

	uint64_t blocksNeeded = (bytes + (BLOCK_SIZE - 1)) / BLOCK_SIZE;
	if (blocksNeeded > (manager->blockCount - manager->usedBlocksCount)) {
		return NULL;
	}

	uint64_t freeBlocks = 0;
	for (uint64_t i = 0; i < manager->blockCount; i++) {
		if (manager->bitmap[i] == FREE) {
			freeBlocks++;
			if (freeBlocks == blocksNeeded) {
				uint64_t start = i - freeBlocks + 1;

				manager->bitmap[start] = BORDER;
				for (uint64_t j = start + 1; j <= i; j++) {
					manager->bitmap[j] = USED;
				}

				manager->usedBlocksCount += blocksNeeded;

				return (uint8_t *) manager->realMemStart + start * BLOCK_SIZE;
			}
		}
		else {
			freeBlocks = 0;
		}
	}
	return NULL;
}

void mm_free(void *const restrict ptr) {
	MemoryManagerADT manager = getMemoryManager();
	if (manager == NULL || ptr == NULL) {
		return;
	}

	if ((uint8_t *) ptr < (uint8_t *) manager->realMemStart) {
		return;
	}

	uint64_t index = ((uint8_t *) ptr - (uint8_t *) manager->realMemStart) / BLOCK_SIZE;
	if (index >= manager->blockCount) {
		return;
	}
	if (manager->bitmap[index] != BORDER) {
		return;
	}

	manager->bitmap[index] = FREE;
	manager->usedBlocksCount--;

	for (uint64_t i = index + 1; i < manager->blockCount && manager->bitmap[i] == USED; i++) {
		manager->bitmap[i] = FREE;
		manager->usedBlocksCount--;
	}
}

mem_t mm_info(void) {
	MemoryManagerADT manager = getMemoryManager();
	mem_t info = {0, 0, 0};
	if (manager == NULL) {
		return info;
	}
	info.size = manager->blockCount;
	info.used = manager->usedBlocksCount;
	info.free = info.size - info.used;
	return info;
}
