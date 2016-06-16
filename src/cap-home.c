#include "cap-home.h"

static void
setline(const char *path, const char *line) {
	FILE *fout = fopen(path, "w");
	if (!fout) {
		cap_die("fopen");
	}

	fprintf(fout, "%s\n", line);
	fflush(fout);

	fclose(fout);
}

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		const char *home = getenv("CAP_HOME");
		home = (home ? home : "/tmp");
		printf("%s\n", home);
		return 0;
	}

	char newhome[100];
	cap_fsolve(newhome, sizeof newhome, argv[1]);

	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir) {
		cap_die("getenv");
	}

	char fpath[100];

	snprintf(fpath, sizeof fpath, "%s/home", vardir);
	setline(fpath, newhome);

	snprintf(fpath, sizeof fpath, "%s/cd", vardir);
	setline(fpath, newhome);

	return 0;
}
