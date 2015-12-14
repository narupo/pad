#ifndef RUN_H
#define RUN_H

#include "util.h"
#include "term.h"
#include "config.h"
#include "cap-file.h"
#include "atcap.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>

void _Noreturn
run_usage(void);

int
run_make(Config const* config, CapFile* dstfile, int argc, char* argv[]);

int
run_main(int argc, char* argv[]);

#endif
