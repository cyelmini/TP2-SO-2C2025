#include <stdint.h>
#include "include/interrupts.h"
#include "include/lib.h"
#include "include/moduleLoader.h"
#include "include/video.h"
#include "include/memoryManagement.h"
#include "include/scheduler.h"


extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static void *const sampleCodeModuleAddress = (void *) 0x400000;
static void *const sampleDataModuleAddress = (void *) 0x500000;
static void * const memoryManagerModuleAddress = (void*)0x600000;

typedef int (*EntryPoint)();
MemoryManagerADT memoryManager;

void clearBSS(void *bssAddress, uint64_t bssSize) {
	memset(bssAddress, 0, bssSize);
}

void *getStackBase() {
	return (void *) ((uint64_t) &endOfKernel + PageSize * 8 // The size of the stack itself, 32KiB
					 - sizeof(uint64_t)						// Begin at the top of the stack
	);
}

void initializeKernelBinary() {
	void *moduleAddresses[] = {sampleCodeModuleAddress, sampleDataModuleAddress};
	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	memoryManager = mm_create(memoryManagerModuleAddress, HEAP_SIZE);
}

int main() {
	load_idt(); 

	_cli(); 
	//Memory manager y scheduler
	createScheduler();
	//Proceso shell
	char *argsShell[1] = {"shell"};
	int16_t fileDescriptors[] = {STDIN, STDOUT, STDERR}; 
	createProcess((uint64_t) sampleCodeModuleAddress, argsShell, 1, MAX_PRIORITY, fileDescriptors, 0);
	_sti();
	
	((EntryPoint) sampleCodeModuleAddress)();
	while (1)
		_hlt();
	return 0;
}
