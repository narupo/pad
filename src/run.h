// @cap brief Source and header of Cap's run pattern.
#ifndef RUN_H
#define RUN_H

#include "util.h"
#include "term.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/wait.h>

void _Noreturn
run_usage(void);

int
run_main(int argc, char* argv[]);

#endif

