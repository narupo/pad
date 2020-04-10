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
struct find;
typedef struct find findcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to findcmd_t
 */
void
findcmd_del(findcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config reference to config_t 
 * @param[in] argc        number of arguments
 * @param[in] move_argv   reference to array of arguments 
 *
 * @return success to pointer to findcmd_t
 * @return failed to NULL
 */
findcmd_t *
findcmd_new(const config_t *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to findcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
findcmd_run(findcmd_t *self);
