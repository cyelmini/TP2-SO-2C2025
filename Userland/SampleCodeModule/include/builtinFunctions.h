#ifndef BUILTINFUNCTIONS_H
#define BUILTINFUNCTIONS_H

#include "shared.h"
#include <stddef.h>
#include <stdint.h>


/* -------------------------------------------------------------------
   BUILT-IN COMMANDS
   ------------------------------------------------------------------- */

/*
 * Todos los built-ins comparten esta firma para poder despacharse
 * desde el mismo arreglo de punteros a función.
 */

void bi_help(int argc, char **argv);
void bi_mem(int argc);
void bi_kill(int argc, char **argv);
void bi_block(int argc, char **argv);
void bi_unblock(int argc, char **argv);
void bi_nice(int argc, char **argv);
void bi_fontSize(int argc, char **argv);

#endif // BUILTINFUNCTIONS_H