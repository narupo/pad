#include "cap-cd.h"

static char *
makevarcdpath(char *dst, size_t dstsz) {
	const char *vardir = getenv("CAP_VARDIR");
	if (!vardir) {
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
docd(const char *drtpath) {
	char newcd[FILE_NPATH];
	cap_fsolve(newcd, sizeof newcd, drtpath);
	// printf("newcd[%s]\n", newcd);

	if (!writevarcd(newcd)) {
		cap_die("failed to write var");
	}	
	
	return true;
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
	
	char varcd[FILE_NPATH];
	if (!readvarcd(varcd, sizeof varcd)) {
		cap_die("failed to read var");
	}
	// printf("varcd[%s]\n", varcd);

	char path[FILE_NPATH];
	cap_fsolvefmt(path, sizeof path, "%s/%s", varcd, argv[1]);
	if (!docd(path)) {
		cap_die("failed to cd");
	}

	return 0;
}
