#ifndef DEPLOY_H
#define DEPLOY_H

#include "util.h"
#include "term.h"
#include "config.h"
#include "file.h"

void _Noreturn
deploy_usage(void);

int
deploy_main(int argc, char* argv[]);

#endif
