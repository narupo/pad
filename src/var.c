#include "var.h"

/**
 * Structure for directory of ~/.cap/var/  
 *
 */
struct cap_var {
	const char *envkey;
	const char *fname;
	char defval[FILE_NPATH];
};

enum {
	VAR_NVALUE = 256,
};

static bool
varwrite(const struct cap_var *vars, const char *vardir) {
	char wrpath[FILE_NPATH]; // Write path
	char sval[VAR_NVALUE]; // Solve value

	for (const struct cap_var *p = vars; p->envkey; ++p) {
		cap_fsolvefmt(wrpath, sizeof wrpath, "%s/%s", vardir, p->fname);
		if (cap_fexists(wrpath)) {
			continue; // Don't over write
		}

		cap_fsolve(sval, sizeof sval, p->defval);
		
		if (!cap_fwriteline(sval, wrpath)) {
			cap_log("error", "failed to write line to %s", wrpath);
			continue;
		}
	}

	return true;
}

static bool
varread(const struct cap_var *vars, const char *vardir) {
	char rdpath[FILE_NPATH]; // Read path
	char val[VAR_NVALUE];

	for (const struct cap_var *p = vars; p->envkey; ++p) {
		cap_fsolvefmt(rdpath, sizeof rdpath, "%s/%s", vardir, p->fname);
		
		if (!cap_freadline(val, sizeof val, rdpath)) {
			cap_log("error", "failed to read line from %s", rdpath);
			continue;
		}

		cap_envsetf(p->envkey, val);
	}
	
	return true;
}

bool
cap_varinit(const char *vardir) {
	static const struct cap_var vars[] = {
		{"CAP_VARHOME", "home", "/tmp"},
		{"CAP_VARCD", "cd", "/tmp"},
		{},
	};

	varwrite(vars, vardir);
	return varread(vars, vardir);
}

#if defined(_TEST_VAR)
#include <stdio.h>

static int
test_varinit(int argc, char *argv[]) {
	if (argc < 2) {
		cap_die("need path of initialize directory by cap_varinit");
	}

	int ret = 0;
	const char *dirpath = argv[1];

	fprintf(stderr, "initialize '%s' directory ... ", dirpath);

	if (!cap_varinit(dirpath)) {
		fprintf(stderr, "failed");
		ret = 1;
	} else {
		fprintf(stderr, "success");
		ret = 0;
	}

	fprintf(stderr, "\n");

	return ret;
}

int
main(int argc, char *argv[]) {
	static const struct cmd {
		const char *name;
		int (*func)(int, char**);
	} cmds[] = {
		{"varinit", test_varinit},
		{},
	};

	if (argc < 2) {
		fprintf(stderr,
			"Usage: %s [command]\n"
			"\n"
			"The commands are:\n\n"
		, argv[0]);
		for (const struct cmd *p = cmds; p->name; ++p) {
			fprintf(stderr, "    %s\n", p->name);
		}
		fprintf(stderr, "\n");
		return 1;
	}

	for (const struct cmd *p = cmds; p->name; ++p) {
		if (!strcmp(p->name, argv[1])) {
			return p->func(argc-1, argv+1);
		}
	}

	fprintf(stderr, "Not found command of '%s'\n", argv[1]);
	return 1;
}
#endif
