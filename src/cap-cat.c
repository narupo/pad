#include "cap-cat.h"

static bool
cat(FILE *fout, FILE *fin) {
	if (!cap_fcopy(fout, fin)) {
		return false;
	}
	return true;
}

int
main(int argc, char *argv[]) {
	setenv("CAP_PROCNAME", "cap cat", 1);

	if (argc < 2) {
		cat(stdout, stdin);
		return 0;
	}

	const char *cd = getenv("CAP_VARCD");
	if (!cd) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		
		// Make path
		char path[100];
		cap_fsolvefmt(path, sizeof path, "%s/%s", cd, name);

		// Copy stream
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			cap_error("fopen %s", path);
			continue;
		}

		if (!cat(stdout, fin)) {
			cap_error("failed to catenate");
		}
		
		if (fclose(fin) < 0) {
			cap_error("failed to close file");
		}
	}

	return 0;
}
