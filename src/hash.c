#include "hash.h"

static pthread_mutex_t hash_crypt_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Create integer hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
int
hash_int(const char* src) {
	int n = 0;

	for (const char* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

/**
 * Create long hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
long
hash_long(const char* src) {
	long n = 0;

	for (const char* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

int
hash_int_from_path(const char* path) {
	int n = 0;

	for (const char* p = path; *p; ++p) {
		if (strchr("/\\:", *p)) {
			continue; // ignore this character
		}
		n += *p;
	}

	return n % HASH_NHASH;
}

char*
hash_sha2(char* dst, size_t dstsz, const char* src) {
	if (pthread_mutex_lock(&hash_crypt_mutex) != 0) {
		return NULL;
	}

	const char* p = crypt(src, "$5$");

	for (int found = 0; *p; ++p) {
		if (*p == '$') {
			++found;
		}
		if (found >= 3) {
			++p;
			break;
		}
	}

	if (!*p) {
		return NULL;
	}

	snprintf(dst, dstsz, "%s", p);

	if (pthread_mutex_unlock(&hash_crypt_mutex) != 0) {
		return NULL;
	}

	return dst;
}
