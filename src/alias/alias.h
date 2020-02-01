#pragma once

#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/cstring_array.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/alias_manager.h"
#include "core/alias_info.h"

struct alcmd;
typedef struct alcmd alcmd_t;

void
alcmd_del(alcmd_t *self);

alcmd_t *
alcmd_new(const config_t *config, int argc, char **argv);

int
alcmd_run(alcmd_t *self);
