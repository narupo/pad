#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/error.h"
#include "lib/cmdline.h"

#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"

/**
 * Structure and type of command
 */
struct exec;
typedef struct exec execcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to execcmd_t
 */
void
execcmd_del(execcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to config_t with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to execcmd_t
 * @return failed to NULL
 */
execcmd_t *
execcmd_new(config_t *move_config, int argc, char **move_argv);

/**
 * Run command
 *
 * @param[in] self pointer to execcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
execcmd_run(execcmd_t *self);
