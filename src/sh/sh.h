#include <stdio.h>
#include <getopt.h>
#include <string.h>

#include "lib/memory.h"
#include "lib/file.h"
#include "lib/string.h"
#include "lib/cstring.h"
#include "lib/cmdline.h"
#include "lib/cl.h"
#include "lib/term.h"
#include "core/constant.h"
#include "core/util.h"
#include "core/config.h"
#include "core/alias_manager.h"
#include "home/home.h"
#include "cd/cd.h"
#include "pwd/pwd.h"
#include "ls/ls.h"
#include "cat/cat.h"
#include "run/run.h"
#include "exec/exec.h"
#include "alias/alias.h"
#include "edit/edit.h"
#include "editor/editor.h"
#include "mkdir/mkdir.h"
#include "rm/rm.h"
#include "mv/mv.h"
#include "cp/cp.h"
#include "touch/touch.h"
#include "snippet/snippet.h"
#include "link/link.h"
#include "make/make.h"

/**
 * Structure and type of command
 */
struct sh;
typedef struct sh shcmd_t;

/**
 * Destruct command
 *
 * @param[in] self pointer to shcmd_t
 */
void
shcmd_del(shcmd_t *self);

/**
 * Construct command
 *
 * @param[in] config pointer to config_t (writable)
 * @param[in] argc   number of arguments
 * @param[in] argv   reference to array of arguments
 *
 * @return success to pointer to shcmd_t
 * @return failed to NULL
 */
shcmd_t *
shcmd_new(config_t *config, int argc, char **argv);

/**
 * Run command
 *
 * @param[in] self pointer to shcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
shcmd_run(shcmd_t *self);
