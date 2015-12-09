#ifndef MAKE_H
#define MAKE_H

#include "util.h"
#include "term.h"
#include "config.h"
#include "file.h"
#include "csvline.h"
#include "atcap.h"
#include "cap-file.h"

#include "cat.h"

void _Noreturn
make_usage(void);

int
make_main(int argc, char* argv[]);

#endif

