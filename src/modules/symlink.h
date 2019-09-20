/**
 * symlinkの仕様
 *
 * Capのシンボリックリンクはファイルで表現される
 * ファイル内にはヘッダーとパスが書かれている
 * たとえば↓のようにである
 *
 *      cap symlink: /path/to/file
 *
 * 'cap symlink:`はヘッダーである
 * Capはファイルにこれが記述されている場合、そのファイルをシンボリックリンクとして判断する
 *
 * ヘッダーに続く文字列はパスである
 * このパスはCapの環境下の*絶対パス*である（ファイルシステム上のパスではない）
 * Capの環境下のパスにすることでCapの移植性と可用性を上げている
 */
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "lib/file.h"
#include "lib/cstring_array.h"
#include "lib/string.h"
#include "modules/constant.h"
#include "modules/config.h"

#define SYMLINK_HEADER "cap symlink:"

/**
 * Follow path for symbolic links and save real path at destination
 *
 * @param[in] *dst pointer to destination
 * @param[in] dstsz number of size of destination
 * @param[in] *drtpath string of dirty path
 *
 * @return success to pointer to path, failed to NULL
 */
char *
symlink_follow_path(config_t *config, char *dst, uint32_t dstsz, const char *drtpath);

/**
 * Check file is Cap's symbolic link
 *
 * @param[in] *path pointer to path
 *
 * @return file is link to true, else false
 */
bool
symlink_is_link_file(const char *path);

