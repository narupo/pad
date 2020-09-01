#include <getopt.h>
#include <string.h>
#include <lib/memory.h>
#include <lib/file.h>
#include <lib/string.h>
#include <core/constant.h>
#include <core/util.h>
#include <core/config.h>
#include <core/error_stack.h>
#include <make/make.h>

/**
 * structure and type of command
 */
struct cookcmd;
typedef struct cookcmd cookcmd_t;

/**
 * destruct command
 *
 * @param[in] self pointer to cookcmd_t
 */
void
cookcmd_del(cookcmd_t *self);

/**
 * construct command
 *
 * @param[in] config      pointer to config_t (read-only)
 * @param[in] argc        number of arguments
 * @param[in] move_argv   pointer to array of arguments 
 *
 * @return success to pointer to cookcmd_t
 * @return failed to NULL
 */
cookcmd_t *
cookcmd_new(const config_t *config, int argc, char **argv);

/**
 * run command
 *
 * @param[in] self pointer to cookcmd_t
 *
 * @return success to number of 0
 * @return failed to number of not 0
 */
int
cookcmd_run(cookcmd_t *self);
