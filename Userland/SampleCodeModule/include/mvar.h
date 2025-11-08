#ifndef MVAR_H
#define MVAR_H

#include "shared.h"
#include <stddef.h>
#include <stdint.h>

pid_t handle_mvar(char **argv, int argc, int ground, int stdin, int stdout);


#endif // MVAR