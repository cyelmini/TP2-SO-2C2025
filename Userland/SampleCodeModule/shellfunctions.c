#include <shellfunctions.h>
#include <syscalls.h>
#include <stdlib.h>
#include <shell.h>


pid_t handle_help(char * arg, int stdin, int stdout){
    return syscall_create_process("help", (fnptr)doHelp, NULL, 1, 1, stdin, stdout);
}

handle_echo

handle_clear 

handle_test_mm

handle_test_processes

handle_test_priority

handle_test_sync

handle_test_no_sync

handle_mem

handle_ps

handle_loop

handle_nice

handle_cat

handle_wc

handle_filter

handle_mvar