#ifndef RUN_H
#define RUN_H

#include "define.h"
#include "util.h"
#include "io.h"
#include "term.h"
#include "config.h"
#include "cap-file.h"
#include "atcap.h"
#include "string.h"
#include "file.h"
#include "shell.h"

void
run_usage(void);

int
run_make(const Config* config, CapFile* dstfile, int argc, char* argv[]);

int
run_main(int argc, char* argv[]);

#endif
