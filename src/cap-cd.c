#include "cap-cd.h"

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		printf("%s\n", getenv("CAP_CD"));
		return 0;
	}
	
	const char *newcd = argv[1];
	const char *confpath = getenv("CAP_CONFPATH");

	if (!cap_fisdir(newcd)) {
		cap_log("error", "can't move to %s", newcd);
		return 1;
	}
	
	FILE *fin = fopen(confpath, "rb");
	if (!fin) {
		cap_log("error", "fopen %s", confpath);
		return 1;
	}

	const char *tmppath = "/tmp/cap.config";
	FILE *fout = fopen(tmppath, "w");
	if (!fout) {
		cap_log("error", "fopen %s", tmppath);
		return 1;
	}

	char line[1024];
	for (; fgets(line, sizeof line, fin); ) {
		size_t len = strlen(line);
		if (line[len-1] == '\n') {
			line[--len] = '\0';
		}
		if (strncmp(line, "cd", 2) == 0) {
			fprintf(fout, "cd = \"%s\"\n", newcd);
		} else {
			fprintf(fout, "%s\n", line);
		}
	}

	fclose(fin);
	fclose(fout);

	if (cap_frename(tmppath, confpath) != 0) {
		cap_log("error", "rename %s -> %s", tmppath, confpath);
		return 1;
	}
	
	return 0;
}
