#include "cap-editor.h"

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
main(int argc, char *argv[]) {
	if (argc < 2) {
		const char *edtr = getenv("CAP_EDITOR");
		if (!edtr) {
			cap_die("need environment variable of editor");
		}
		printf("%s\n", edtr);
		return 0;
	}

	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir || strlen(vardir) == 0) {
		cap_die("need environment variable of vardir");
	}

	char edtrpath[100];
	snprintf(edtrpath, sizeof edtrpath, "%s/editor", vardir);

	char newedtr[100];
	cap_fsolve(newedtr, sizeof newedtr, argv[1]);
	if (!cap_fexists(newedtr)) {
		cap_die("%s is not exists", newedtr);
	}

	setline(edtrpath, newedtr);

	return 0;
}
