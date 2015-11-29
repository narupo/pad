#ifndef EDITOR_H
#define EDITOR_H

#include "util.h"
#include "term.h"
#include "config.h"

void _Noreturn
editor_usage(void);

int
editor_main(int argc, char* argv[]);

#endif

