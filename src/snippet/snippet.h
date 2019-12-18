#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/cstring.h"
#include "lib/string.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "lang/tokenizer.h"
#include "lang/ast.h"
#include "lang/compiler.h"
#include "lang/traverser.h"
#include "lang/context.h"
#include "lang/opts.h"

/**
 * Structure and type of command
 */
struct snptcmd;
typedef struct snptcmd snptcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to snptcmd_t
 */
void
snptcmd_del(snptcmd_t *self);

/**
 * Construct command
 *
 * @param[in] move_config pointer to config_t with move semantics
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments with move semantics
 *
 * @return success to pointer to snptcmd_t
 * @return failed to NULL
 */
snptcmd_t *
snptcmd_new(config_t *move_config, int argc, char **move_argv);

/**
 * Run command
 *
 * @param[in] self pointer to snptcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
snptcmd_run(snptcmd_t *self);
