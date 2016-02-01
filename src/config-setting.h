#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include "types.h"
#include "util.h"
#include "file.h"
#include "string.h"
#include "csvline.h"

/*****************
* Delete and New *
*****************/

/**
 * Destruct configsetting.
 *
 * @param[in] self
 */
void
configsetting_delete(ConfigSetting* self);

/**
 * Construct configsetting from file.
 *
 * @param[in] fname Load file name
 *
 * @return pointer to allocate memory of configsetting
 */
ConfigSetting*
configsetting_new_from_file(char const* fname);

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
configsetting_path(ConfigSetting const* self, char const* key);

/*********
* Setter *
*********/

/**
 * Save config settings to file by name
 *
 * @param[in] self
 * @param[in] fname file name
 *
 * @return success to true
 * @return failed to false
 */
bool
configsetting_save_to_file(ConfigSetting* self, char const* fname);

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
configsetting_set_path(ConfigSetting* self, char const* key, char const* val);

#endif

