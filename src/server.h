#ifndef SERVER_H
#define SERVER_H

#include "util.h"
#include "term.h"
#include "file.h"
#include "config.h"
#include "caperr.h"
#include "string.h"
#include "buffer.h"
#include "strarray.h"
#include "socket.h"
#include "http-header.h"
#include "signal.h"

#include <pthread.h>
#include <time.h>

void
server_usage(void);

int
server_main(int argc, char* argv[]);

#endif