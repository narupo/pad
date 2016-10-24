/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "cap.h"

static const char CAP_VERSION[] = "cap version 0.17";

/******
* cap *
******/

/**
 * Program option values for command.
 *
 */
struct opts {
	bool ishelp;
	bool isversion;
};

struct cap {
	int argc;
	char **argv;
	int cmdargc;
	char **cmdargv;
	char *alcmdln;
	struct opts opts;
};

static void
capdel(struct cap *cap) {
	if (cap) {
		freeargv(cap->argc, cap->argv);
		freeargv(cap->cmdargc, cap->cmdargv);
		free(cap->alcmdln);
		free(cap);
	}
}

static bool
optsparse(struct opts *opts, int argc, char *argv[]) {
	static struct option longopts[] = {
		{"help", no_argument, 0, 'h'},
		{"version", no_argument, 0, 'V'},
		{},
	};

	*opts = (struct opts){};
	optind = 0;
	
	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "hV", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': opts->ishelp = true; break;
		case 'V': opts->isversion = true; break;
		case '?':
		default: return false; break;
		}
	}

	if (argc < optind) {
		return false;
	}

	return true;
}

static struct cap *
capfixcmdargs(struct cap *cap) {
	static const char *capcmds[] = {
		"home",
		"cat",
		"cd",
		"pwd",
		"ls",
		"run",
		"alias",
		NULL,
	};
	
	const char *cmdname = cap->cmdargv[0];
	if (!cmdname) {
		return cap;
	}

	for (const char **p = capcmds; *p; ++p) {
		if (!strcmp(*p, cmdname)) {
			return cap; // Exists command
		}
	}

	// Not found command. Re-build command for cap-alias
	char bindir[FILE_NPATH];
	if (!cap_envget(bindir, sizeof bindir, "CAP_BINDIR")) {
		cap_log("error", "need bin directory path on environ");
		return NULL;
	}

	struct cap_string *cmdln = cap_strnew();
	if (!cmdln) {
		return NULL;
	}

	cap_strapp(cmdln, bindir);
	cap_strapp(cmdln, "/cap-alias --run '");

	for (int i = 0; i < cap->cmdargc-1; ++i) {
		cap_strapp(cmdln, cap->cmdargv[i]);
		cap_strapp(cmdln, " ");
	}
	if (cap->cmdargc > 0) {
		cap_strapp(cmdln, cap->cmdargv[cap->cmdargc-1]);
	}
	cap_strapp(cmdln, "'");

	cap->alcmdln = cap_strescdel(cmdln);

	return cap;
}

static struct cap *
capsolveopts(struct cap *cap, int ac, char *av[]) {
	// Parse options
	struct cap_array *args = cap_arrnew();
	struct cap_array *cmdargs = cap_arrnew();
	if (!args || !cmdargs) {
		cap_arrdel(args);
		cap_arrdel(cmdargs);
		return NULL;
	}

	struct cap_array *curarr = args; // Current array for parse

	cap_arrpush(curarr, av[0]);
	for (int i = 1; i < ac; ++i) {
		const char *ag = av[i];
		if (curarr == args && ag[0] != '-') {
			cap_arrmove(curarr, NULL); // For the final null in argv
			curarr = cmdargs;
		}
		cap_arrpush(curarr, ag);
	}
	cap_arrmove(curarr, NULL); // For the final null in argv

	// Parse options
	cap->argc = cap_arrlen(args)-1; // -1 for final null
	cap->argv = cap_arrescdel(args);
	cap->cmdargc = cap_arrlen(cmdargs)-1; // -1 for final null
	cap->cmdargv = cap_arrescdel(cmdargs);

	// Fix command arguments
	capfixcmdargs(cap);

	return cap;
}

static bool
capinitenv(const struct cap *cap) {
	char caproot[FILE_NPATH];
	char vardir[FILE_NPATH];
	char homedir[FILE_NPATH];
	const char bindir[] = "/usr/local/bin/";

	cap_fsolve(caproot, sizeof caproot, "~/.cap");
	if (!cap_fexists(caproot)) {
		cap_fmkdirq(caproot);
	}

	snprintf(homedir, sizeof homedir, "%s/home", caproot);
	if (!cap_fexists(homedir)) {
		cap_fmkdirq(homedir);
	}

	snprintf(vardir, sizeof vardir, "%s/var", caproot);
	if (!cap_fexists(vardir)) {
		cap_fmkdirq(vardir);
	}

	cap_envsetf("CAP_BINDIR", bindir);
	cap_envsetf("CAP_HOMEDIR", homedir);
	cap_envsetf("CAP_VARDIR", vardir);
	
	return cap_varinit(vardir);
}

static struct cap *
capnew(int argc, char *argv[]) {
	struct cap *cap = calloc(1, sizeof(*cap));
	if (!cap) {
		return NULL;
	}

	if (!capinitenv(cap)) {
		capdel(cap);
		return NULL;
	}

	if (!capsolveopts(cap, argc, argv)) {
		capdel(cap);
		return NULL;
	}

	if (!optsparse(&cap->opts, cap->argc, cap->argv)) {
		capdel(cap);
		return NULL;
	}
	
	return cap;
}

static void
capusage(struct cap *cap) {
	static const char usage[] =
		"Usage: cap [options] [command] [arguments]\n"
		"\n"
		"    Cap is simple snippet manager.\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help       show usage.\n"
		"    -V, --version    show version.\n"
		"\n"
		"The commands are:\n"
		"\n"
		"    home     change home directory, or show.\n"
		"    cd       change current directory by relative of home.\n"
		"    pwd      show current directory.\n"
		"    ls       show list in current directory.\n"
		"    cat      catenate files and show.\n"
		"    run      run script.\n"
		"    alias    run alias command.\n"
	;
	static const char *examples[] = {
		"    $ cap home\n"
		"    $ cap pwd\n"
		"    $ cap ls\n"
		,
		"    $ cap alias al alias\n"
		"    $ cap al\n"
		,
		"    $ cap cat path/to/code/file.c\n"
		"    $ cap ls path/to/code\n"
		,
	};
	const int exmlen = sizeof(examples)/sizeof(*examples);
	const char *example = NULL;

	srand(time(NULL));
	example = examples[randrange(0, exmlen-1)];

	fprintf(stderr,
		"%s\n"
		"Examples:\n\n"
		"%s\n"
	, usage, example);
	
	capdel(cap);
	exit(0);
}

static void
capversion(struct cap *cap) {
	fflush(stdout);
	fflush(stderr);

	printf("%s\n", CAP_VERSION);
	fflush(stdout);

	capdel(cap);
	exit(0);
}

static void
capfork(struct cap *cap) {
	char bindir[FILE_NPATH];
	if (!cap_envget(bindir, sizeof bindir, "CAP_BINDIR")) {
		capdel(cap);
		cap_die("need bin directory on environ");
	}

	char ppath[FILE_NPATH];
	cap_fsolvefmt(ppath, sizeof ppath, "%s/cap-%s", bindir, cap->cmdargv[0]);

	switch (fork()) {
	case -1:
		cap_die("failed to fork");
	break;
	case 0: // Child
		if (execv(ppath, cap->cmdargv) < 0) {
			cap_log("error", "failed to execute of %s", ppath);
			_exit(1);
		}
	break;
	default:// Parent
		wait(NULL);
		capdel(cap);
		exit(0);
	break;
	}
}

static void
caprun(struct cap *cap) {
	if (cap->opts.ishelp) {
		capusage(cap);
	}

	if (cap->opts.isversion) {
		capversion(cap);
	}

	if (cap->cmdargc < 1) {
		capusage(cap);
	}
	
	if (cap->alcmdln) {
		safesystem(cap->alcmdln);
	} else {
		capfork(cap);
	}
}

int
main(int argc, char *argv[]) {
	struct cap *cap = capnew(argc, argv);
	if (!cap) {
		cap_die("failed to create cap");
	}

	caprun(cap);
	capdel(cap);
	return 0;
}
