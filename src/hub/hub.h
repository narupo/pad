#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/cstring.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/args.h"
#include "hub/commands/runserver.h"

/**
 * Structure and type of command
 */
struct hubcmd;
typedef struct hubcmd hubcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to hubcmd_t
 */
void
hubcmd_del(hubcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to config_t with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to hubcmd_t
 * @return failed to NULL
 */
hubcmd_t *
hubcmd_new(config_t *move_config, int argc, char **move_argv);

/**
 * Run command
 *
 * @param[in] self pointer to hubcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
hubcmd_run(hubcmd_t *self);
