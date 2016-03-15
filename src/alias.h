#ifndef ALIAS_H
#define ALIAS_H

#include "define.h"
#include "util.h"
#include "term.h"
#include "config.h"
#include "csvline.h"
#include "strarray.h"
#include "caperr.h"
#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

/**
 * Convert alias record to CsvLine 
 * 
 * @param[in] name pointer to memory of C string for alias name
 * 
 * @return success to pointer to dynamic allocate memory of CsvLine
 * @return failed to NULL
 */
CsvLine*
alias_to_csvline(char const* name);

void
alias_usage(void);

int
alias_main(int argc, char* argv[]);

#endif

