/**
 * Cap
 *
 * License: MIT
 *  Author: narupo
 *   Since: 2016
 */
#include "cap.h"

/**
 * Structure for directory of ~/.cap/var/  
 *
 */
struct var {
	const char *envkey;
	const char *fname;
	char defval[FILE_NPATH];
};

/**
 * Program option values for command.
 *
 */
struct opts {
	bool ishelp;
};

static bool
writeconfig(const char *cnfpath) {
	FILE *fout = fopen(cnfpath, "w");
	if (!fout) {
		return false;
	}

	char tmp[100];

	fprintf(fout, "# Cap's constant config\n");
	fprintf(fout, "ignore = \"%s\"\n", cap_fsolve(tmp, sizeof tmp, "/tmp"));

	if (fclose(fout) != 0) {
		return false;
	}

	return true;
}

static bool
varswrite(const struct var *vars, const char *vardir) {
	char wrpath[FILE_NPATH]; // Write path
	char sval[100]; // Solve value

	for (const struct var *p = vars; p->envkey; ++p) {
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
varsread(const struct var *vars, const char *vardir) {
	char rdpath[FILE_NPATH]; // Read path
	char val[100];

	for (const struct var *p = vars; p->envkey; ++p) {
		cap_fsolvefmt(rdpath, sizeof rdpath, "%s/%s", vardir, p->fname);
		
		if (!cap_freadline(val, sizeof val, rdpath)) {
			cap_log("error", "failed to read line from %s", rdpath);
			continue;
		}

		cap_envsetf(p->envkey, val);
	}
	
	return true;
}

static bool
varsinit(const char *vardir) {
	const struct var vars[] = {
		{"CAP_VARHOME", "home", "/tmp"},
		{"CAP_VARCD", "cd", "/tmp"},
		{"CAP_VAREDITOR", "editor", "/usr/bin/vi"},
		{},
	};

	varswrite(vars, vardir);
	return varsread(vars, vardir);
}

/******
* cap *
******/

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
		{},
	};

	*opts = (struct opts){};
	optind = 0;
	
	for (;;) {
		int optsindex;
		int cur = getopt_long(argc, argv, "h", longopts, &optsindex);
		if (cur == -1) {
			break;
		}

		switch (cur) {
		case 'h': opts->ishelp = true; break;
		case '?':
		default: cap_log("error", "unknown option"); break;
		}
	}

	if (argc < optind) {
		return false;
	}

	return true;
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

	char **newargv = wrapargvvals(cap->cmdargc, cap->cmdargv);
	if (!newargv) {
		cap_strdel(cmdln);
		return NULL;
	}

	cap_strapp(cmdln, bindir);
	cap_strapp(cmdln, "/cap-alias --run '");

	for (int i = 0; i < cap->cmdargc; ++i) {
		cap_strapp(cmdln, newargv[i]);
		cap_strapp(cmdln, " ");
	}
	cap_strapp(cmdln, "'");

	freeargv(cap->cmdargc, newargv);
	cap->alcmdln = cap_strescdel(cmdln);
	// cap_log("debug", "cap->alcmdln[%s]\n", cap->alcmdln);
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
	char cnfpath[FILE_NPATH];
	char vardir[FILE_NPATH];
	char homedir[FILE_NPATH];

	cap_fsolve(caproot, sizeof caproot, "~/.cap2"); // TODO
	if (!cap_fexists(caproot)) {
		cap_fmkdirq(caproot);
	}

	snprintf(cnfpath, sizeof cnfpath, "%s/config", caproot);
	if (!cap_fexists(cnfpath)) {
		writeconfig(cnfpath);
	}

	snprintf(homedir, sizeof homedir, "%s/home", caproot);
	if (!cap_fexists(homedir)) {
		cap_fmkdirq(homedir);
	}

	snprintf(vardir, sizeof vardir, "%s/var", caproot);
	if (!cap_fexists(vardir)) {
		cap_fmkdirq(vardir);
	}

	cap_envsetf("CAP_BINDIR", "/home/narupo/src/cap/bin"); // TODO
	cap_envsetf("CAP_CONFPATH", cnfpath);
	cap_envsetf("CAP_HOMEDIR", homedir);
	cap_envsetf("CAP_VARDIR", vardir);
	
	return varsinit(vardir);
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
		freeargv(argc, argv);
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
	fprintf(stderr,
		"Usage: cap [options] [command] [arguments]\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help    show usage.\n"
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
		"\n"
		"Examples:\n"
		"\n"
		"    $ cap home\n"
		"    $ cap pwd\n"
		"    $ cap ls ./\n"
		"\n"
	);
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
	snprintf(ppath, sizeof ppath, "%s/cap-%s", bindir, cap->cmdargv[0]);

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
	if (cap->cmdargc < 1 || cap->opts.ishelp) {
		capusage(cap);

	} else if (cap->alcmdln) {
		// cap_log("debug", "cap->alcmdln[%s]", cap->alcmdln);
		system(cap->alcmdln);

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
