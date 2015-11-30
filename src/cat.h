#ifndef CAT_H
#define CAT_H

#include "util.h"
#include "file.h"
#include "config.h"
#include "term.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void _Noreturn
cat_usage(void);

int
cat_main(int argc, char* argv[]);

#endif

