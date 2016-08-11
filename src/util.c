/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
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

bool
isoutofhome(const char *pth) {
	char hm[FILE_NPATH];
	if (!cap_envget(hm, sizeof hm, "CAP_VARHOME")) {
		cap_log("error", "invalid environment variable of 'CAP_VARHOME'");
		return true;
	}

	char home[FILE_NPATH];
	char path[FILE_NPATH];

	if (!cap_fsolve(home, sizeof home, hm) ||
		!cap_fsolve(path, sizeof path, pth)) {
		cap_log("error", "failed to solve path");
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

static bool
isnormch(int c) {
	return isalpha(c) || isdigit(c) || c == '-' || c == '_';
}

/*
    if (execve("/usr/bin/any_cmd", args, env) == -1) {
      _Exit(127);
    } 
*/
int
safesystem(const char *cmdline) {
	return system(cmdline);
}

#if defined(_TEST_UTIL)
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

	return safesystem(argv[1]);
}

int
main(int argc, char* argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"isoutofhome", test_isoutofhome},
		{"safesystem", test_safesystem},
		{},
	};

	if (argc < 2) {
		fprintf(stderr, "Commands:\n");
		for (const struct cmd *p = cmds; p->name; ++p) {
			fprintf(stderr, "    %s\n", p->name);
		}
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
