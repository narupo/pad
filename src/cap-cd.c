/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap-cd.h"

static char *
makevarcdpath(char *dst, size_t dstsz) {
	char vardir[FILE_NPATH];
	if (!cap_envget(vardir, sizeof vardir, "CAP_VARDIR")) {
		cap_die("need environment variable of vardir");
	}

	cap_fsolvefmt(dst, dstsz, "%s/cd", vardir);
	return dst;
}

static char *
readvarcd(char *dst, size_t dstsz) {
	char varcdpath[FILE_NPATH];
	makevarcdpath(varcdpath, sizeof varcdpath);
	return cap_freadline(dst, dstsz, varcdpath);
}

static const char *
writevarcd(const char *line) {
	char varcdpath[FILE_NPATH];
	makevarcdpath(varcdpath, sizeof varcdpath);
	return cap_fwriteline(line, varcdpath);
}

static bool
cd(const char *drtpath) {
	char newcd[FILE_NPATH];
	cap_fsolve(newcd, sizeof newcd, drtpath);

	if (isoutofhome(newcd)) {
		cap_die("'%s' is out of home", newcd);
	}

	if (!cap_fisdir(newcd)) {
		cap_die("'%s' is not a directory", newcd);
	}

	if (!writevarcd(newcd)) {
		cap_die("failed to write var");
	}	

	return true;
}

int
main(int argc, char *argv[]) {
	cap_envsetf("CAP_PROCNAME", "cap cd");

	if (argc < 2) {
		char home[FILE_NPATH];
		if (!cap_envget(home, sizeof home, "CAP_VARHOME")) {
			cap_die("need environment variable of home");
		}

		if (!cd(home)) {
			cap_die("failed to cd");
		}
		
		return 0;
	}
	
	char varcd[FILE_NPATH];
	if (!readvarcd(varcd, sizeof varcd)) {
		cap_die("failed to read var");
	}

	char tmp[FILE_NPATH];
	char path[FILE_NPATH];

	if (argv[1][0] == '/' || argv[1][0] == '\\') {
		// Absolute of home
		if (!cap_envget(tmp, sizeof tmp, "CAP_VARHOME")) {
			cap_die("failed to get environment variable of home");
		}
		cap_fsolvefmt(path, sizeof path, "%s/%s", tmp, argv[1]);
	} else {
		// Relative of cd
		cap_fsolvefmt(path, sizeof path, "%s/%s", varcd, argv[1]);
	}

	if (!cd(path)) {
		cap_die("failed to cd");
	}

	return 0;
}
