#include "cap-cd.h"

static bool
docd(const char *newcd) {
	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir) {
		cap_die("need environment variable of vardir");
	}

	if (!cap_fisdir(newcd)) {
		return false;
	}

	char cdpath[100];
	snprintf(cdpath, sizeof cdpath, "%s/cd", vardir);
	// printf("vardir[%s]\n", cdpath);

	FILE *fout = fopen(cdpath, "w");
	if (!fout) {
		return false;
	}

	fprintf(fout, "%s\n", newcd);

	return fclose(fout) == 0;
}

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		const char *home = getenv("CAP_VARHOME");
		if (!home) {
			cap_die("need environment variable of home");
		}
		if (!docd(home)) {
			cap_die("failed to cd");
		}
		return 0;
	}
	
	char newcd[100];
	cap_fsolve(newcd, sizeof newcd, argv[1]);
	
	if (!docd(newcd)) {
		cap_die("failed to cd");
	}

	return 0;
}
