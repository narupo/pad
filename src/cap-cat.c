#include "cap-cat.h"

int
main(int argc, char *argv[]) {
	struct cap_config *conf = cap_confnewload();
	char *cd = cap_confgetcp(conf, "cd");
	cap_confdel(conf);

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

	free(cd);
	return 0;
}
