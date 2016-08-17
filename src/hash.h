/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <crypt.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

enum {
	CAP_NHASH = 701,
};

/**
 *
 *
 * @param[in]  src
 *
 * @return
 */
long
cap_hashl(const char *src);

#endif

