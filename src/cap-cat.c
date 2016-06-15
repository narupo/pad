#include "cap-cat.h"

int
main(int argc, char *argv[]) {
	const char *cd = getenv("CAP_CD");

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
		if (!cap_fcopy(stderr, fin)) {
			cap_log("error", "fcopy");
		}
		fclose(fin);
	}

	return 0;
}
