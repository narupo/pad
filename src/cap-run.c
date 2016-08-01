#include "cap-run.h"

enum {
	NSCRIPTNAME = 100,
	NCMDLINE = 256
};

char *
readscriptline(char *dst, size_t dstsz, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return NULL;
	}

	char tmp[dstsz];
	cap_fgetline(tmp, sizeof tmp, fin);

	const char *needle = "#!";
	char *at = strstr(tmp, "#!");
	if (!at) {
		fclose(fin);
		return NULL;
	}

	snprintf(dst, dstsz, "%s", at + strlen(needle));

	if (fclose(fin) < 0) {
		return NULL;
	}

	return dst;
}

static char **
wrapargvvals(int argc, char *argv[]) {
	struct cap_args *args = cap_argsnew();
	cap_argsparse(args, argc, argv);
	for (int i = 2; i < cap_argslen(args); ++i) {
		cap_argwrapvalue(cap_argsget(args, i), '"');
	}
	return cap_argsescdel(args);
}

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		cap_error("need script name");
		return 1;
	}

	char varcd[FILE_NPATH];
	if (!cap_envget(varcd, sizeof varcd, "CAP_VARCD")) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	argv = wrapargvvals(argc, argv);
	char path[FILE_NPATH];
	char sname[NSCRIPTNAME]; // Script name
	char cmdline[NCMDLINE] = {};

	cap_fsolvefmt(path, sizeof path, "%s/%s", varcd, argv[1]);
	readscriptline(sname, sizeof sname, path);
	// cap_log("debug", "sname[%s]\n", sname);

	capstrncat(cmdline, sizeof cmdline, sname);
	capstrncat(cmdline, sizeof cmdline, " ");
	capstrncat(cmdline, sizeof cmdline, path);
	capstrncat(cmdline, sizeof cmdline, " ");
	for (int i = 2; i < argc; ++i) {
		capstrncat(cmdline, sizeof cmdline, argv[i]);
		capstrncat(cmdline, sizeof cmdline, " ");
	}
	// cap_log("debug", "cap-run: cmdline[%s]\n", cmdline);

	FILE* pin = popen(cmdline, "r");
	if (!pin) {
		cap_die("failed open process '%s'", cmdline);
	}

	for (int ch; (ch = fgetc(pin)) != EOF; ) {
		putchar(ch);
	}
	fflush(stdout);

	if (pclose(pin) < 0) {
		freeargv(argc, argv);
		cap_die("failed to close process");
	}

	freeargv(argc, argv);
	return 0;
}
