#ifndef UTIL_H
#define UTIL_H

#include "program.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <errno.h>

void _Noreturn
die(char const* fmt, ...);

void _Noreturn
usage(void);

#endif

