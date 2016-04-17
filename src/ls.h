#ifndef LS_H
#define LS_H

#include "define.h"
#include "term.h"
#include "io.h"
#include "file.h"
#include "config.h"
#include "strarray.h"
#include "atcap.h"
#include "string.h"

#include <strings.h>

void
ls_usage(void);

int
ls_main(int argc, char* argv[]);

#endif

