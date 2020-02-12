#include <getopt.h>
#include <string.h>

#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/symlink.h>

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
 * @param[in] config reference to config_t 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to touchcmd_t
 * @return failed to NULL
 */
touchcmd_t *
touchcmd_new(const config_t *config, int argc, char **argv);

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
