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
	setenv("CAP_PROCNAME", "cap cat", 1);

	if (argc < 2) {
		capcat(stdout, stdin);
		return 0;
	}

	const char *cd = getenv("CAP_VARCD");
	if (!cd) {
		cap_error("need environment variable of cd");
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		
		// Make path
		char path[FILE_NPATH];
		cap_fsolvefmt(path, sizeof path, "%s/%s", cd, name);
		if (!cap_fexists(path)) {
			cap_error("invalid path of '%s'", path);
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
