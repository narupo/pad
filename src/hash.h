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
	CAP_HASH_NMOD = 701,
};

/**
 * Get hash value by string of source.
 *
 * @param[in]  src string of source
 *
 * @return success to number of hash value
 * @return failed to under of zero
 */
long
cap_hashl(const char *src);

#endif

