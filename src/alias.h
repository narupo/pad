#ifndef ALIAS_H
#define ALIAS_H

#include "util.h"
#include "term.h"
#include "config.h"
#include "csvline.h"
#include "strarray.h"
#include "caperr.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

CsvLine*
alias_to_csvline(char const* name);

void
alias_usage(void);

int
alias_main(int argc, char* argv[]);

#endif

