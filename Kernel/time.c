// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "include/time.h"
#include <stdint.h>

static uint64_t ticks = 0;

void timerHandler() {
	ticks++;
}

uint64_t ticksElapsed() {
	return ticks;
}

int secondsElapsed() {
	return ticks / 18;
}