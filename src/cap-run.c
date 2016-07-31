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

int
main(int argc, char *argv[]) {
	if (argc < 2) {
		return 1;
	}

	char cd[FILE_NPATH];
	if (!cap_envget(cd, sizeof cd, "CAP_VARCD")) {
		cap_log("error", "need environment variable of cd");
		return 1;
	}

	char path[FILE_NPATH];
	char sname[NSCRIPTNAME]; // Script name
	char cmdline[NCMDLINE];

	snprintf(path, sizeof path, "%s/%s", cd, argv[1]);
	readscriptline(sname, sizeof sname, path);

	strcat(cmdline, sname);
	strcat(cmdline, " ");
	strcat(cmdline, path);
	strcat(cmdline, " ");
	for (int i = 2; i < argc; ++i) {
		strcat(cmdline, argv[i]);
		strcat(cmdline, " ");
	}

	FILE* pin = popen(cmdline, "r");
	if (!pin) {
		cap_die("failed open process '%s'", cmdline);
	}

	for (int ch; (ch = fgetc(pin)) != EOF; ) {
		putchar(ch);
	}
	fflush(stdout);

	if (pclose(pin) < 0) {
		cap_die("failed to close process");
	}

	return 0;
}

