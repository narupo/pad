#include "cap-home.h"

static void
setline(const char *path, const char *line) {
	FILE *fout = fopen(path, "w");
	if (!fout) {
		cap_die("fopen %s", path);
	}

	fprintf(fout, "%s\n", line);
	fflush(fout);

	fclose(fout);
}

int
main(int argc, char* argv[]) {
	if (argc < 2) {
		const char *home = getenv("CAP_HOME");
		if (!home) {
			cap_die("need environment variable of home");
		}
		printf("%s\n", home);
		return 0;
	}

	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir || strlen(vardir) == 0) {
		cap_die("need environment variable of vardir");
	}

	char hmpath[100];
	snprintf(hmpath, sizeof hmpath, "%s/home", vardir);

	char newhome[100];
	cap_fsolve(newhome, sizeof newhome, argv[1]);
	if (!cap_fexists(newhome)) {
		cap_die("%s is not a directory", newhome);
	}

	setline(hmpath, newhome);

	char cdpath[100];
	snprintf(cdpath, sizeof cdpath, "%s/cd", vardir);
	setline(cdpath, newhome);

	return 0;
}
