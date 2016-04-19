#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <crypt.h>
#include <pthread.h>
#include <string.h>

enum {
	HASH_NHASH = 701,
};

/**
 *
 *
 * @param[in]  src
 *
 * @return
 */
int
hash_int(const char* src);

/**
 *
 *
 * @param[in]  src
 *
 * @return
 */
long
hash_long(const char* src);

/**
 *
 *
 * @param[in]  path
 *
 * @return
 */
int
hash_int_from_path(const char* path);

/**
 *
 *
 * @param[in]  dst
 * @param[in]  dstsz
 * @param[in]  src
 *
 * @return
 */
char*
hash_sha2(char* dst, size_t dstsz, const char* src);

#endif
