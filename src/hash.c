/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "hash.h"

static long
cap_hashlw(const char* src, long nhash) {
	if (!src || nhash <= 0) {
		return -1;
	}

	long n = 0;
	long weight = 0;

	for (const char* p = src; *p; ++p, ++weight) {
		if (weight > 7) {
			weight = 0;
		}
		n += *p << (4 * weight);
	}

	return n % nhash;
}

long
cap_hashl(const char* src) {
	return cap_hashlw(src, HASH_NMOD);
}

