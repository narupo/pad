#ifndef HASH_H
#define HASH_H

enum {
	HASH_NHASH = 701,
};

/**
 * Create integer hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
static inline int
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
static inline long
hash_long(const char* src) {
	long n = 0;

	for (const char* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

static inline int
hash_int_from_path(const char* path) {
	int n = 0;

	for (const char* p = path; *p; ++p) {
		if (strchr("/\\:", *p)) {
			continue; // ignore this character
		}
		n += *p;
	}

	return n;
}

#endif
