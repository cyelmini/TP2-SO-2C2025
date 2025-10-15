include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard *.c) $(wildcard utils/*.c) $(wildcard utils/drivers/*.c) $(wildcard utils/processes/*.c)

MM_TYPE ?= bitmap
MM_DIR = utils/memory
MM_SOURCE = $(MM_DIR)/$(MM_TYPE).c
MM_OBJECT = $(MM_TYPE).o

SOURCES_ASM=$(wildcard asm/*.asm)
OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(MM_OBJECT)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(MM_OBJECT) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld --oformat=elf64-x86-64 -o kernel.elf $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(MM_OBJECT) $(STATICLIBS)

$(MM_OBJECT): $(MM_SOURCE)
	$(GCC) $(GCCFLAGS) -I./include -c $(MM_SOURCE) -o $(MM_OBJECT)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf asm/*.o utils/*.o utils/memory/*.o utils/drivers/*.o utils/processes/*.o *.o *.bin $(MM_OBJECT)

.PHONY: all clean
