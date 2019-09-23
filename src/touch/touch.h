#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/symlink.h"

/**
 * Structure and type of command
 */
struct touchcmd;
typedef struct touchcmd touchcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to touchcmd_t
 */
void
touchcmd_del(touchcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to config_t with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to touchcmd_t
 * @return failed to NULL
 */
touchcmd_t *
touchcmd_new(config_t *move_config, int argc, char **move_argv);

/**
 * Run command
 *
 * @param[in] self pointer to touchcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
touchcmd_run(touchcmd_t *self);
