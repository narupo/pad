/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-cat.h"

static int
catstream(FILE *fout, FILE *fin) {
	if (!cap_fcopy(fout, fin)) {
		return 1;
	}
	return 0;
}

static char *
makepath(char *dst, size_t dstsz, const char *cdpath, const char *name) {
	if (!cap_fsolvefmt(dst, dstsz, "%s/%s", cdpath, name)) {
		return NULL;
	}

	if (isoutofhome(dst)) {
		return NULL;
	}

	if (cap_fisdir(dst)) {
		return NULL;
	}

	return dst;
}

static bool
catfile(const char *path, FILE *fout) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return false;
	}

	if (catstream(fout, fin) != 0) {
		fclose(fin);
		return false;
	}
	
	if (fclose(fin) < 0) {
		return false;
	}

	return true;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap cat");

	if (argc < 2) {
		return catstream(stdout, stdin);
	}

	char cdpath[FILE_NPATH];
	if (!cap_envget(cdpath, sizeof cdpath, "CAP_VARCD")) {
		cap_error("need environment variable of cd");
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		
		char path[FILE_NPATH];
		if (!makepath(path, sizeof path, cdpath, name)) {
			cap_error("failed to make path by '%s'", name);
			continue;
		}

		if (!catfile(path, stdout)) {
			cap_error("failed to catenate of '%s'", path);
			continue;
		}
	}

	return 0;
}
