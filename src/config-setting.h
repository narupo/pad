#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include "types.h"
#include "util.h"
#include "file.h"
#include "buffer.h"

/*****************
* Delete and New *
*****************/

/**
 * Destruct config_setting.
 *
 * @param[in: self
 */
void
config_setting_delete(ConfigSetting* self);

/**
 * Construct config_setting from file.
 *
 * @param[in] fname Load file name
 * @return Pointer to allocate memory of config_setting
 */
ConfigSetting*
config_setting_new_from_file(char const* fname);

/*********
* Getter *
*********/

char const*
config_setting_path(ConfigSetting const* self, char const* key);

/*********
* Setter *
*********/

bool
config_setting_save_to_file(ConfigSetting* self, char const* fname);

bool
config_setting_set_path(ConfigSetting* self, char const* key, char const* val);

#endif

