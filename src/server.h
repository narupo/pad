#ifndef SERVER_H
#define SERVER_H

#include "util.h"
#include "term.h"
#include "config.h"
#include "caperr.h"
#include "memory.h"
#include "string.h"

void
server_usage(void);

int
server_main(int argc, char* argv[]);

#endif
