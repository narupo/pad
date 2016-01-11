#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
#include "util.h"
#include "buffer.h"
#include "file.h"
#include "config-setting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

/*****************
* Delete and New *
*****************/

/**
 * Get pointer to instance of Config
 * Using singleton-pattern
 * Multi thread safe
 * 
 * @return pointer to Config object
 */
Config*
config_instance(void);

/*********
* Getter *
*********/

/**
 * Get root directory path of config
 * This path is directory of all config's file
 * 
 * @param self
 * 
 * @return success to pointer to string of path
 * @return failed to pointer to NULL
 */
char const*
config_root(Config const* self);

/**
 * Make normalized cap's path from basename
 *
 * @param[in] self 
 * @param[out] dst destination of normalized path
 * @param[in] dstsize size of destination
 * @param[in] basename basename
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
config_path_from_base(Config const* self, char* dst, size_t dstsize, char const* basename);

/**
 * Make normalized cap's path from basename
 * 
 * @param[in] self
 * @param[in] basename basename like a file name of cap's management files
 *
 * @return success to pointer to normalized path of dynamic allocate memory
 * @return failed to pointer to NULL
 */
char*
config_make_path_from_base(Config const* self, char const* basename);

/**
 * Get path by key
 *
 * @param[in] self
 * @param[in] key key (example "cd" or "editor" etc...)
 *
 * @return success to pointer to path
 * @return failed to NULL
 */
char const*
config_path(Config const* self, char const* key);

/*********
* Setter *
*********/

/**
 * Set path by key
 *
 * @param[in] self
 * @param[in] key key
 * @param[in] path set path
 *
 * @return success to true
 * @return failed to false
 */
bool
config_set_path(Config* self, char const* key, char const* path);

/**
 * Save config to file system
 *
 * @param self
 *
 * @return success to true
 * @return failed to false
 */
bool
config_save(Config const* self);

#endif

