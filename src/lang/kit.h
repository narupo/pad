#pragma once

#include <lib/memory.h>
#include <core/config.h>
#include <lang/context.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/tokenizer.h>
#include <lang/traverser.h>
#include <lang/gc.h>
#include <lang/opts.h>

struct kit;
typedef struct kit kit_t;

void
kit_del(kit_t *self);

kit_t *
kit_new(const config_t *config);

bool
kit_compile_from_path(kit_t *self, const char *path);

bool
kit_compile_from_string(kit_t *self, const char *str);

const char *
kit_getc_compiled(const kit_t *self);

void
kit_clear_context(kit_t *self);

bool
kit_has_error(const kit_t *self);

void
kit_clear_context_buffer(kit_t *self);