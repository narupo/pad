#include "cap-home.h"

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		const char *home = getenv("CAP_VARHOME");
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
	if (!cap_fisdir(newhome)) {
		cap_die("%s is not a directory", newhome);
	}

	// Update var/home
	cap_fwriteline(newhome, hmpath);

	// Update var/cd
	char cdpath[100];
	snprintf(cdpath, sizeof cdpath, "%s/cd", vardir);
	cap_fwriteline(newhome, cdpath);

	return 0;
}
