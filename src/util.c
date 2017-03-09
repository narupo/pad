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

#if defined(_TEST_UTIL)
static int
test_randrange(int argc, char *argv[]) {
	if (argc < 3) {
		cap_die("need value of min, max");
	}
	srand(time(NULL));
	int min = atoi(argv[1]);
	int max = atoi(argv[2]);
	printf("%d\n", randrange(min, max));
	return 0;
}

static int
test_isoutofhome(int argc, char *argv[]) {
	if (isoutofhome(argv[1])) {
		puts("is out of home");
	} else {
		puts("not out of home");
	}
	return 0;
}

static int
test_safesystem(int argc, char *argv[]) {
	if (argc < 2) {
		perror("Need command line string.");
		return 1;
	}

	showargv(argc, argv);
	puts("do safesystem");
	return safesystem(argv[1]);
}

int
main(int argc, char* argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"randrange", test_randrange},
		{"isoutofhome", test_isoutofhome},
		{"safesystem", test_safesystem},
		{},
	};

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [command]\n\n", argv[0]);
		fprintf(stderr, "The commands are:\n\n");
		for (const struct cmd *p = cmds; p->name; ++p) {
			fprintf(stderr, "    %s\n", p->name);
		}
		fprintf(stderr, "\n");
		return 1;
	}

	for (const struct cmd *p = cmds; p->name; ++p) {
		if (!strcmp(p->name, argv[1])) {
			return p->func(--argc, ++argv);
		}
	}

	fprintf(stderr, "Not found command of '%s'\n", argv[1]);
	return 1;
}
#endif
