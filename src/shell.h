#ifndef SHELL_H
#define SHELL_H 1

#include "util.h"
#include "io.h"

char*
shell_read_script_line(char* dst, size_t dstsize, FILE* stream);

#endif
