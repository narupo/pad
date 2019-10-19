#pragma once

#include <assert.h>

#include "lib/memory.h"
#include "lib/string.h"
#include "lib/dict.h"

struct opts;
typedef struct opts opts_t;

void
opts_del(opts_t *self);

opts_t *
opts_new(void);

opts_t *
opts_parse(opts_t *self, int argc, char *argv[]);

const char *
opts_getc(const opts_t *self, const char *optname);