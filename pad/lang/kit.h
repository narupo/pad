#pragma once

#include <pad/lib/memory.h>
#include <pad/core/config.h>
#include <pad/core/error_stack.h>
#include <pad/lang/context.h>
#include <pad/lang/ast.h>
#include <pad/lang/compiler.h>
#include <pad/lang/tokenizer.h>
#include <pad/lang/traverser.h>
#include <pad/lang/gc.h>
#include <pad/lang/opts.h>
#include <pad/lang/types.h>

void
kit_del(kit_t *self);

kit_t *
kit_new(const PadConfig *config);

kit_t *
kit_new_ref_gc(const PadConfig *config, gc_t *ref_gc);

kit_t *
kit_compile_from_path(kit_t *self, const char *path);

kit_t *
kit_compile_from_path_args(kit_t *self, const char *path, int argc, char *argv[]);

kit_t *
kit_compile_from_string(kit_t *self, const char *str);

kit_t *
kit_compile_from_string_args(kit_t *self, const char *path, const char *str, int argc, char *argv[]);

const char *
kit_getc_stdout_buf(const kit_t *self);

const char *
kit_getc_stderr_buf(const kit_t *self);

void
kit_clear_context(kit_t *self);

PadCtx *
kit_get_context(kit_t *self);

bool
kit_has_error_stack(const kit_t *self);

const PadErrStack *
kit_getc_error_stack(const kit_t *self);

void
kit_clear_context_buffer(kit_t *self);

void
kit_trace_error(const kit_t *self, FILE *fout);

void
kit_trace_error_debug(const kit_t *self, FILE *fout);
