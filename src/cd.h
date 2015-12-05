#ifndef CD_H
#define CD_H

#include "term.h"
#include "file.h"
#include "config.h"

void _Noreturn
cd_usage(void);

int
cd_main(int argc, char* argv[]);

#endif

