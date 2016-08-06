#include "util.h"

void
freeargv(int argc, char *argv[]) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

bool
capisoutofhome(const char *pth) {
	const char *hm = getenv("CAP_HOME");
	if (!hm) {
		cap_log("error", "invalid environment variable of 'CAP_HOME'");
		return true;
	}

	char home[FILE_NPATH];
	char path[FILE_NPATH];

	if (!cap_fsolve(home, sizeof home, hm) ||
		!cap_fsolve(path, sizeof path, pth)) {
		cap_log("error", "failed to solve path");
		return true;
	}

	if (!cap_fexists(path)) {
		return true;
	}

	size_t homelen = strlen(home);
	if (strncmp(home, path, homelen)) {
		return true;
	}

	return false;
}

#if defined(_TEST_UTIL)
int
main(int argc, char* argv[]) {
	if (isoutofhome(argv[1])) {
		puts("is out of home");
	} else {
		puts("not out of home");
	}
	return 0;
}
#endif