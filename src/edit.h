#ifndef EDIT_H
#define EDIT_H

#include "util.h"
#include "term.h"
#include "config.h"
#include <unistd.h>
#include <sys/wait.h>

void
edit_usage(void);

int
edit_main(int argc, char* argv[]);

#endif

