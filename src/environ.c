#include "environ.h"

static char const PROGNAME[] = "cap environ";

char const*
environ_get(char const* key) {
	if (strcmp(key, "HOME") == 0) {
#if defined(_WIN32) || defined(_WIN64)
		return getenv("USERPROFILE");
#else
		return getenv("HOME");
#endif
	}

	// Not found key
	caperr(PROGNAME, CAPERR_NOTFOUND, "environ's key \"%s\"", key);
	return NULL;
}
