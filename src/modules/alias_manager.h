#pragma once

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib/error.h"
#include "lib/memory.h"

#include "modules/config.h"

struct alias_manager;
typedef struct alias_manager almgr_t;

void
almgr_del(almgr_t *self);

almgr_t *
almgr_new(const config_t *config);

almgr_t *
almgr_find_alias_value(almgr_t *self, char *dst, uint32_t dstsz, const char *key, int scope);
