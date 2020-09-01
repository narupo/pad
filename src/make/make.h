#pragma once

#include <lib/error.h>
#include <lib/string.h>
#include <lib/file.h>
#include <core/config.h>
#include <core/util.h>
#include <core/symlink.h>
#include <core/error_stack.h>
#include <lang/tokenizer.h>
#include <lang/ast.h>
#include <lang/compiler.h>
#include <lang/traverser.h>
#include <lang/context.h>
#include <lang/opts.h>

struct makecmd;
typedef struct makecmd makecmd_t;

/**
 * destruct command
 * 
 * @param[in] *self pointer to makecmd_t (dynamic allocate memory) 
 */
void
makecmd_del(makecmd_t *self);

/**
 * construct command
 * 
 * @param[in] *config pointer to config
 * @param[in] argc    number of arguments
 * @param[in] **argv  arguments
 * 
 * @return success to pointer to makecmd_t (dynamic allocate memory)
 * @return failed to NULL
 */
makecmd_t *
makecmd_new(const config_t *config, int argc, char **argv);

/**
 * run command
 * 
 * @param[in] *self 
 * 
 * @return success to 0, else other
 */
int
makecmd_run(makecmd_t *self);

/**
 * make script or stdin from program arguments
 * 
 * @param[in] *config    pointer to config_t (read-only)
 * @param[out] *errstack pointer to errstack_t (writeable)
 * @param[in] argc       number of arguments
 * @param[in] *argv[]    arguments
 * @param[in] solve_path if want to solve path then true else false
 * 
 * @return success to 0, else other
 */
int
make_from_args(
    const config_t *config,
    errstack_t *errstack,
    int argc,
    char *argv[],
    bool solve_path
);
