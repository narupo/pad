#ifndef MAKE_H
#define MAKE_H

#include "define.h"
#include "caperr.h"
#include "util.h"
#include "term.h"
#include "config.h"
#include "file.h"
#include "csvline.h"
#include "atcap.h"
#include "cap-file.h"
#include "string.h"

#include "cat.h"
#include "run.h"

void
make_usage(void);

int
make_main(int argc, char* argv[]);

#endif

