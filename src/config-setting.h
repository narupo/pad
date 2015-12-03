#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include "types.h"
#include "util.h"
#include "file.h"
#include "buffer.h"
#include "csvline.h"

/*****************
* Delete and New *
*****************/

/**
 * Destruct configsetting.
 *
 * @param[in: self
 */
void
configsetting_delete(ConfigSetting* self);

/**
 * Construct configsetting from file.
 *
 * @param[in] fname Load file name
 * @return Pointer to allocate memory of configsetting
 */
ConfigSetting*
configsetting_new_from_file(char const* fname);

/*********
* Getter *
*********/

char const*
configsetting_path(ConfigSetting const* self, char const* key);

/*********
* Setter *
*********/

bool
configsetting_save_to_file(ConfigSetting* self, char const* fname);

bool
configsetting_set_path(ConfigSetting* self, char const* key, char const* val);

#endif

