#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"

/**
 * Structure and type of command
 */
struct editorcmd;
typedef struct editorcmd editorcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to editorcmd_t
 */
void
editorcmd_del(editorcmd_t *self);

/**
 * Construct command
 *
 * @param[in] config reference to config_t 
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments 
 *
 * @return success to pointer to editorcmd_t
 * @return failed to NULL
 */
editorcmd_t *
editorcmd_new(const config_t *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to editorcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
editorcmd_run(editorcmd_t *self);
