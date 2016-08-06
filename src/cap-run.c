#include "cap-run.h"

enum {
	NSCRIPTNAME = 100,
	NCMDLINE = 256
};

static char *
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
	cap_envsetf("CAP_PROCNAME", "cap run");

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

	char spath[FILE_NPATH]; // Script path
	cap_fsolvefmt(spath, sizeof spath, "%s/%s", varcd, argv[1]);
	if (capisoutofhome(spath)) {
		freeargv(argc, argv);
		cap_die("invalid script '%s'", spath);
	}

	char exesname[NSCRIPTNAME]; // Execute script name in file
	readscriptline(exesname, sizeof exesname, spath);
	// cap_log("debug", "exesname[%s]\n", exesname);

	struct cap_string *cmdline = cap_strnew();
	cap_strapp(cmdline, exesname);
	cap_strapp(cmdline, " ");
	cap_strapp(cmdline, spath);
	cap_strapp(cmdline, " ");
	for (int i = 2; i < argc; ++i) {
		cap_strapp(cmdline, argv[i]);
		cap_strapp(cmdline, " ");
	}
	// cap_log("debug", "exesname[%s] spath[%s] cmdline[%s]\n", exesname, spath, cap_strgetc(cmdline));

	// Start process communication
	FILE* pin = popen(cap_strgetc(cmdline), "r");
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

	// Done
	cap_strdel(cmdline);
	freeargv(argc, argv);
	return 0;
}
