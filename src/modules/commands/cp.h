#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"

#include "modules/constant.h"
#include "modules/util.h"
#include "modules/config.h"
#include "modules/symlink.h"

/**
 * Structure and type of command
 */
struct cpcmd;
typedef struct cpcmd cpcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to cpcmd_t
 */
void
cpcmd_del(cpcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to config_t with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to cpcmd_t
 * @return failed to NULL
 */
cpcmd_t *
cpcmd_new(config_t *move_config, int argc, char **move_argv);

/**
 * Run command
 *
 * @param[in] self pointer to cpcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
cpcmd_run(cpcmd_t *self);
