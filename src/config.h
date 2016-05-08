#ifndef CONFIG_H
#define CONFIG_H

#include "define.h"
#include "caperr.h"
#include "util.h"
#include "buffer.h"
#include "file.h"
#include "json.h"
#include "hash.h"

typedef struct Config Config;

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
 * Get config's directory path
 *
 * @param[in] self
 * @param[in] key key of path (examples "root", "trash")
 *
 * @return success to pointer to string of path
 * @return failed to pointer to NULL
 */
const char*
config_dirpath(const Config* self, const char* key);

char*
config_make_dirpath(const Config* self, const char* key);

/**
 * Get config's directory path with name
 *
 * @param[in] self
 * @param[out] dst destination buffer for path
 * @param[in] dstsz size of destination buffer
 * @param[in] with key of path (examples "root", "trash")
 * @param[in] name composition name by a with
 *
 * @return success to pointer to destination buffer (dst)
 * @return failed to pointer to NULL
 */
char*
config_dirpath_with(const Config* self, char* dst, size_t dstsz, const char* with, const char* name);

/**
 * Get confing's file path
 *
 * @param[in] self
 * @param[in] key key of path (examples "config")
 *
 * @return success to pointer to string of path
 * @return failed to pointer to NULL
 */
const char*
config_filepath(const Config* self, const char* key);

/**
 * Make normalized cap's path from basename with keyword
 *
 * @param[in] self
 * @param[out] dst destination of normalized path
 * @param[in] dstsize size of destination
 * @param[in] with keyword (examples "cd", "home")
 * @param[in] base basename
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
config_path_with(const Config* self, char* dst, size_t dstsize, const char* with, const char* base);

/**
 * Make normalized cap's path from basename with home directory
 *
 * @param[in] self
 * @param[out] dst destination of normalized path
 * @param[in] dstsize size of destination
 * @param[in] base basename
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
config_path_with_home(const Config* self, char* dst, size_t dstsize, const char* base);

/**
 * Make normalized cap's path from basename with current directory
 *
 * @param[in] self
 * @param[out] dst destination of normalized path
 * @param[in] dstsize size of destination
 * @param[in] base basename
 *
 * @return success to pointer to dst
 * @return failed to pointer to NULL
 */
char*
config_path_with_cd(const Config* self, char* dst, size_t dstsize, const char* basename);

/**
 * Get path by key
 *
 * @param[in] self
 * @param[in] key key (example "cd" or "editor" etc...)
 *
 * @return success to pointer to path
 * @return failed to NULL
 */
const char*
config_path(const Config* self, const char* key);

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
config_set_path(Config* self, const char* key, const char* path);

/**
 * Save config to file system
 *
 * @param self
 *
 * @return success to true
 * @return failed to false
 */
bool
config_save(const Config* self);

/**
 * Check path is out of home
 *
 * @param      self
 * @param      path  check path
 *
 * @return     is out of home to true
 * @return     is not out of home to false
 */
bool
config_is_out_of_home(const Config* self, const char* path);

#endif

