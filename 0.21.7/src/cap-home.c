/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-home.h"

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		char home[FILE_NPATH];
		if (!cap_envget(home, sizeof home, "CAP_VARHOME")) {
			cap_die("need environment variable of home");
		}
		printf("%s\n", home);
		return 0;
	}

	char vardir[FILE_NPATH];
	if (!cap_envget(vardir, sizeof vardir, "CAP_VARDIR") ||
		strlen(vardir) == 0) {
		cap_die("need environment variable of vardir");
	}

	char hmpath[FILE_NPATH];
	snprintf(hmpath, sizeof hmpath, "%s/home", vardir);

	char newhome[FILE_NPATH];
	cap_fsolve(newhome, sizeof newhome, argv[1]);
	if (!cap_fisdir(newhome)) {
		cap_die("%s is not a directory", newhome);
	}

	// Update var/home
	cap_fwriteline(newhome, hmpath);

	// Update var/cd
	char cdpath[FILE_NPATH];
	snprintf(cdpath, sizeof cdpath, "%s/cd", vardir);
	cap_fwriteline(newhome, cdpath);

	return 0;
}
