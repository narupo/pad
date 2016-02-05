#ifndef CONFIGSERVER_H
#define CONFIGSERVER_H

#include "util.h"
#include "file.h"
#include "string.h"
#include "csvline.h"
#include "strmap.h"

typedef struct ConfigServer ConfigServer;

/*****************
* Delete and New *
*****************/

/**
 * Destruct configserver
 *
 * @param[in] self
 */
void
configserver_delete(ConfigServer* self);

/**
 * Construct configserver from file.
 *
 * @param[in] fname Load file name
 *
 * @return pointer to allocate memory of configserver
 */
ConfigServer*
configserver_new_from_file(char const* fname);

/*********
* Getter *
*********/

/**
 * Get string of path by key
 *
 * @param[in] self
 * @param[in] key string of key
 *
 * @return success to pointer to string of path
 * @return failed to pointer to dummy string
 */
char const*
configserver_path(ConfigServer const* self, char const* key);

/*********
* Setter *
*********/

/**
 * Save config servers to file by name
 *
 * @param[in] self
 * @param[in] fname file name
 *
 * @return success to true
 * @return failed to false
 */
bool
configserver_save_to_file(ConfigServer* self, char const* fname);

/**
 * Set value of string to object by key
 *
 * @param[in] self
 * @param[in] key key of string
 * @param[in] val set value of string
 *
 * @return success to true
 * @return failed to true
 */
bool
configserver_set_path(ConfigServer* self, char const* key, char const* val);

#endif

