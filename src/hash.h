#ifndef CAP_HASH_H
#define CAP_HASH_H

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

