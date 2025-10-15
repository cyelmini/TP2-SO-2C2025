#include <stdint.h>
#include "include/color.h"
#include "include/keyboard.h"
#include "include/lib.h"
#include "include/memory.h"
#include "include/memoryManagement.h"
#include "include/process.h"
#include "include/time.h"
#include "include/video.h"
#include "include/scheduler.h"

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
#define GET_REGISTER_ARRAY 4
#define SET_FONT_SIZE 5
#define GET_RESOLUTION 6
#define GET_TICKS 7
#define GET_MEMORY 8
#define SET_FONT_COLOR 9
#define GET_FONT_COLOR 10
#define MM_ALLOC 11
#define MM_FREE 12
#define MM_INFO 13
#define CREATE_PS 14
#define GET_PID 15
#define PS_INFO 16

static uint8_t syscall_read(uint32_t fd);

static void syscall_write(uint32_t fd, char c);

static void syscall_clear();

static uint32_t syscall_seconds();

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

static uint64_t syscall_create_process(uint64_t rip, char **args, int argc, uint8_t priority, char ground, int16_t fileDescriptors[]);

static uint64_t syscall_getPid();

static ProcessInfo *syscall_process_info(uint16_t *processQty);

//typedef uint64_t (*syscall)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

//static syscall syscalls[] = {(syscall) syscall_read};


uint64_t syscallDispatcher(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4,
						   uint64_t arg5) {
	switch (nr) {
		case READ:
			return syscall_read((uint32_t) arg0);

		case WRITE:
			syscall_write((uint32_t) arg0, (char) arg1);
			break;

		case CLEAR:
			syscall_clear();
			break;

		case SECONDS:
			return syscall_seconds();

		case GET_REGISTER_ARRAY:
			return (uint64_t) syscall_registerArray((uint64_t *) arg0);

		case SET_FONT_SIZE:
			syscall_fontSize((uint8_t) arg0);
			break;

		case GET_RESOLUTION:
			return syscall_resolution();

		case GET_TICKS:
			return syscall_getTicks();

		case GET_MEMORY:
			syscall_getMemory((uint64_t) arg0, (uint8_t *) arg1);
			break;

		case SET_FONT_COLOR:
			syscall_setFontColor((uint8_t) arg0, (uint8_t) arg1, (uint8_t) arg2);
			break;

		case GET_FONT_COLOR:
			return syscall_getFontColor();

		case MM_ALLOC:
			return (uint64_t) syscall_mm_alloc((size_t) arg0);

		case MM_FREE:
			syscall_mm_free((void *) arg0);
			break;

		case MM_INFO:
			syscall_mm_info((mem_t *) arg0);
			break;
		
		case CREATE_PS:
			syscall_create_process((uint64_t)arg0, (char **) arg1, (int)arg2, (uint8_t)arg3, (char)arg4, (int16_t *)arg5);
			
		case GET_PID:
   			 return syscall_getPid();
		
		case PS_INFO: 
		    syscall_process_info((uint16_t *)arg0);
			break;
	}
	return 0;
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
	ColorInt c = {color: getFontColor()};
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

static uint64_t syscall_create_process(uint64_t rip, char **args, int argc, uint8_t priority, char ground, int16_t fileDescriptors[]){
	return createProcess(rip, args, argc, priority, fileDescriptors, ground);
}

static uint64_t syscall_getPid() {
	return getPid();
}

static ProcessInfo *syscall_process_info(uint16_t *processQty) {
	return ps(processQty);
}