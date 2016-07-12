#include "cap-editor.h"

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		const char *edtr = getenv("CAP_VAREDITOR");
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

	cap_fwriteline(newedtr, edtrpath);

	return 0;
}
