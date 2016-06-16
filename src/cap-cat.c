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
	if (argc < 2) {
		cat(stdout, stdin);
		return 0;
	}

	const char *cd = getenv("CAP_CD");
	if (!cd) {
		cap_log("error", "need environ variable of cd");
		return 1;
	}

	for (int i = 1; i < argc; ++i) {
		const char *name = argv[i];
		
		// Make path
		char path[100];
		snprintf(path, sizeof path, "%s/%s", cd, name);
		//printf("path[%s]\n", path);

		// Copy stream
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			cap_log("error", "fopen %s", path);
			continue;
		}
		if (!cat(stdout, fin)) {
			cap_log("error", "cat");
		}
		fclose(fin);
	}

	return 0;
}
