#ifndef CAT_H
#define CAT_H

#include "util.h"
#include "file.h"
#include "config.h"
#include "term.h"
#include "buffer.h"
#include "atcap.h"
#include "strarray.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

void _Noreturn
cat_usage(void);

int
cat_main(int argc, char* argv[]);

/**
 * This Interface for the cap make command
 * Do not clear and delete CapFile because append row to it for make
 */
int
cat_make(Config const* config, CapFile* dst, int argc, char* argv[]);

#endif

