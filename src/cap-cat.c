/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include "cap-cat.h"

static bool
capcat(FILE *fout, FILE *fin) {
	if (!cap_fcopy(fout, fin)) {
		return false;
	}
	return true;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap cat");

	if (argc < 2) {
		capcat(stdout, stdin);
		return 0;
	}

	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_error("need environment variable of cd");
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		
		// Make path
		char path[FILE_NPATH];
		cap_fsolvefmt(path, sizeof path, "%s/%s", cd, name);
		if (isoutofhome(path)) {
			cap_error("invalid path '%s'", path);
			continue;
		}

		if (cap_fisdir(path)) {
			cap_error("'%s' is directory", path);
			continue;
		}

		// Copy stream
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			cap_error("fopen %s", path);
			continue;
		}

		if (!capcat(stdout, fin)) {
			cap_error("failed to catenate");
		}
		
		if (fclose(fin) < 0) {
			cap_error("failed to close file");
		}
	}

	return 0;
}
