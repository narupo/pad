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
hash_int(char const* src) {
	int n = 0;

	for (char const* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

/**
 * Create integer hash value from string and mod number
 *
 * @param[in] src source string
 * @param[in] nhash max hash size
 *
 * @return number of hash value
 */
static inline int
hash_int_from(char const* src, int nhash) {
	int n = 0;

	for (char const* p = src; *p; ++p) {
		n += *p;
	}

	return n % nhash;
}

/**
 * Create long hash value from string
 *
 * @param[in] src source string
 *
 * @return number of hash value
 */
static inline long
hash_long(char const* src) {
	long n = 0;

	for (char const* p = src; *p; ++p) {
		n += *p;
	}

	return n % HASH_NHASH;
}

#endif
