/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "util.h"

void
freeargv(int argc, char *argv[]) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

void
showargv(int argc, char *argv[]) {
	for (int i = 0; i < argc; ++i) {
		printf("%s\n", argv[i]);
	}
}

bool
isoutofhome(const char *pth) {
	char hm[FILE_NPATH];
	if (!cap_envget(hm, sizeof hm, "CAP_VARHOME")) {
		cap_error("invalid environment variable of 'CAP_VARHOME'");
		return true;
	}

	char home[FILE_NPATH];
	char path[FILE_NPATH];

	if (!cap_fsolve(home, sizeof home, hm) ||
		!cap_fsolve(path, sizeof path, pth)) {
		cap_error("failed to solve path");
		return true;
	}

	if (!cap_fexists(path)) {
		return true;
	}

	size_t homelen = strlen(home);
	if (strncmp(home, path, homelen)) {
		return true;
	}

	return false;
}

int
randrange(int min, int max) {
	return min + (int)(rand() * (max - min + 1.0) / (1.0 + RAND_MAX));
}

int
safesystem(const char *cmdline) {
	struct cap_cl *cl = cap_clnew();
	if (!cap_clparsestropts(cl, cmdline, 0)) {
		cap_cldel(cl);
		return -1;
	}

	int argc = cap_cllen(cl);
	char **argv = cap_clescdel(cl);
	if (!argv) {
		return -1;
	}
	
	switch (fork()) {
	case -1:
		return -1;
	break;
	case 0:
		if (execv(argv[0], argv) == -1) {
			freeargv(argc, argv);
			_exit(1);
		}
	break;
	default:
		freeargv(argc, argv);
		wait(NULL);
	break;
	}

	return 0;
}

struct cap_array *
argsbyoptind(int argc, char *argv[], int optind) {
	struct cap_array *args = cap_arrnew();
	
	/* DO NOT DELETE FOR DEBUG.

	printf("argc[%d] optind[%d]\n", argc, optind);
	for (int i = 0; i < argc; ++i) {
		printf("%d %s\n", i, argv[i]);
	}
	*/

	cap_arrpush(args, argv[0]);
	for (int i = optind; i < argc; ++i) {
		cap_arrpush(args, argv[i]);
	}

	return args;
}
