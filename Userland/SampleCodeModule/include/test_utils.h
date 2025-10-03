#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdlib.h>
#include <syscalls.h>

uint32_t getUint();

uint32_t getUniform(uint32_t max);

uint8_t memcheck(void *start, uint8_t value, uint32_t size);

int64_t satoi(char *str);

#endif
