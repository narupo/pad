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
 * @param[in] config reference to config_t 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to cpcmd_t
 * @return failed to NULL
 */
cpcmd_t *
cpcmd_new(const config_t *config, int argc, char **argv);

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
