#ifndef HELP_H
#define HELP_H

#include "util.h"
#include "term.h"
#include <stdbool.h>
#include <string.h>

#include "cat.h"
#include "ls.h"
#include "cd.h"
#include "edit.h"
#include "editor.h"
#include "deploy.h"
#include "make.h"
#include "path.h"
#include "run.h"

int
help_main(int argc, char* argv[]);

void
help_usage(void);

#endif

