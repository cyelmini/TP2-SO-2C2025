#include "include/color.h"
#include "include/keyboard.h"
#include "include/lib.h"
#include "include/memory.h"
#include "include/memoryManagement.h"
#include "include/process.h"
#include "include/scheduler.h"
#include "include/time.h"
#include "include/video.h"
#include <stdint.h>

#define SYSCALL_COUNT 26

// File Descriptors
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define KBDIN 3

// IDs de syscalls
#define READ 0
#define WRITE 1
#define CLEAR 2
#define SECONDS 3
#define MINUTES 4
#define HOURS 5
#define GET_REGISTER_ARRAY 6
#define SET_FONT_SIZE 7
#define GET_RESOLUTION 8
#define GET_TICKS 9
#define GET_MEMORY 10
#define SET_FONT_COLOR 11
#define GET_FONT_COLOR 12
#define MM_ALLOC 13
#define MM_FREE 14
#define MM_INFO 15
#define CREATE_PS 16
#define GET_PID 17
#define PS_INFO 18
#define KILL_PROCESS 19
#define CHANGE_PRIORITY 20
#define BLOCK_PS 21
#define READY_PS 22
#define YIELD 23
#define WAIT_PS 24
#define EXIT 25

static uint8_t syscall_read(uint32_t fd);

static void syscall_write(uint32_t fd, char c);

static void syscall_clear();

static uint32_t syscall_seconds();

static uint32_t syscall_minutes();

static uint32_t syscall_hours();

static uint64_t *syscall_registerArray(uint64_t *regarr);

static void syscall_fontSize(uint8_t size);

static uint32_t syscall_resolution();

static uint64_t syscall_getTicks();

static void syscall_getMemory(uint64_t pos, uint8_t *vec);

static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b);

static uint32_t syscall_getFontColor();

static void *syscall_mm_alloc(size_t size);

static void syscall_mm_free(void *const restrict ptr);

static void syscall_mm_info(mem_t *info);

static uint64_t syscall_create_process(uint64_t rip, char **args, int argc, uint8_t priority, char ground,
									   int16_t fileDescriptors[]);

static uint64_t syscall_getPid();

static ProcessInfo *syscall_process_info(uint16_t *processQty);

static void syscall_exit();

static void syscall_sleep(uint32_t s);

typedef uint64_t (*syscall)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

static const syscall syscalls[] = {
	(syscall) syscall_read,
	(syscall) syscall_write,
	(syscall) syscall_clear,
	(syscall) syscall_seconds,
	(syscall) syscall_minutes,
	(syscall) syscall_hours,
	(syscall) syscall_registerArray,
	(syscall) syscall_fontSize,
	(syscall) syscall_resolution,
	(syscall) syscall_getTicks,
	(syscall) syscall_getMemory,
	(syscall) syscall_setFontColor,
	(syscall) syscall_getFontColor,
	(syscall) syscall_mm_alloc,
	(syscall) syscall_mm_free,
	(syscall) syscall_mm_info,
	(syscall) syscall_create_process,
	(syscall) syscall_getPid,
	(syscall) syscall_process_info,
	(syscall) killProcess,
	(syscall) changePriority,
	(syscall) blockProcess,
	(syscall) setReadyProcess,
	(syscall) yield,
	(syscall) waitProcess,
	(syscall) syscall_exit,
	(syscall) syscall_sleep,
};

uint64_t syscallDispatcher(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4,
						   uint64_t arg5) {
	if (nr >= SYSCALL_COUNT)
		return -1;
	return syscalls[nr](arg0, arg1, arg2, arg3, arg4, arg5);
}

static uint8_t syscall_read(uint32_t fd) {
	switch (fd) {
		case STDIN:
			return getAscii();
		case KBDIN:
			return getScancode();
	}
	return 0;
}

static void syscall_write(uint32_t fd, char c) {
	Color prevColor = getFontColor();
	if (fd == STDERR)
		setFontColor(ERROR_COLOR);
	else if (fd != STDOUT)
		return;
	printChar(c);
	setFontColor(prevColor);
}

static void syscall_clear() {
	videoClear();
}

static uint32_t syscall_seconds() {
	uint8_t h, m, s;
	getTime(&h, &m, &s);
	return s + m * 60 + ((h + 24 - 3) % 24) * 3600;
}

static uint32_t syscall_minutes() {
	uint8_t h, m, s;
	getTime(&h, &m, &s);
	return m + ((h + 24 - 3) % 24) * 60;
}

static uint32_t syscall_hours() {
	uint8_t h, m, s;
	getTime(&h, &m, &s);
	return ((h + 24 - 3) % 24);
}

static uint64_t *syscall_registerArray(uint64_t *regarr) {
	uint64_t *snapshot = getLastRegSnapshot();
	for (int i = 0; i < QTY_REGS; i++)
		regarr[i] = snapshot[i];
	return regarr;
}

static void syscall_fontSize(uint8_t size) {
	setFontSize(size - 1);
}

static uint32_t syscall_resolution() {
	return getScreenResolution();
}

static uint64_t syscall_getTicks() {
	return ticksElapsed();
}

static void syscall_getMemory(uint64_t pos, uint8_t *vec) {
	memcpy(vec, (uint8_t *) pos, 32);
}

static void syscall_setFontColor(uint8_t r, uint8_t g, uint8_t b) {
	setFontColor((Color) {b, g, r});
}

static uint32_t syscall_getFontColor() {
	ColorInt c = {.color = getFontColor()};
	return c.bits;
}

static void *syscall_mm_alloc(size_t size) {
	return mm_alloc(size);
}

static void syscall_mm_free(void *const restrict ptr) {
	mm_free(ptr);
}

static void syscall_mm_info(mem_t *info) {
	*info = mm_info();
}

static uint64_t syscall_create_process(uint64_t rip, char **args, int argc, uint8_t priority, char ground,
									   int16_t fileDescriptors[]) {
	return createProcess(rip, args, argc, priority, fileDescriptors, ground);
}

static uint64_t syscall_getPid() {
	return getPid();
}

static ProcessInfo *syscall_process_info(uint16_t *processQty) {
	return ps(processQty);
}

static void syscall_exit() {
	killCurrentProcess();
	yield();
}

static void syscall_sleep(uint32_t s){
	if (s == 0)
		return;
	uint32_t start = syscall_seconds();
	while ((uint32_t) ((syscall_seconds() + 86400 - start) % 86400) < s) {
		yield();
	}
}