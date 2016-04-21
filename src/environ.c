#include "environ.h"

static const char PROGNAME[] = "cap environ";

const char*
environ_get(const char* key) {
	if (strcmp(key, "HOME") == 0) {
#if defined(_CAP_WINDOWS)
		return getenv("USERPROFILE");
#else
		return getenv("HOME");
#endif
	}

	// Not found key
	caperr(PROGNAME, CAPERR_NOTFOUND, "environ's key \"%s\"", key);
	return NULL;
}
