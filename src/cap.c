#include "cap.h"

/**
 * Structure for directory of ~/.cap/var/  
 *
 */
struct var {
	const char *envkey;
	const char *fname;
	char defval[100];
};

/**
 * Program option values for command.
 *
 */
struct opts {
	bool ishelp;
};

static void
freeargv(int argc, char *argv[]) {
	if (argv) {
		for (int i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
	}
}

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
	char path[100];
	char sval[100]; // Solve value

	for (const struct var *p = vars; p->envkey; ++p) {
		snprintf(path, sizeof path, "%s/%s", vardir, p->fname);
		if (cap_fexists(path)) {
			continue;
		}

		cap_fsolve(sval, sizeof sval, p->defval);
		
		if (!cap_fwriteline(sval, path)) {
			cap_log("error", "failed to write line to %s", path);
			continue;
		}
	}

	return true;
}

static bool
varsread(const struct var *vars, const char *vardir) {
	char path[100];
	char val[100];

	for (const struct var *p = vars; p->envkey; ++p) {
		snprintf(path, sizeof path, "%s/%s", vardir, p->fname);
		
		if (!cap_freadline(val, sizeof val, path)) {
			cap_log("error", "failed to read line from %s", path);
			continue;
		}

		setenv(p->envkey, val, 1);
	}
	
	return true;
}

static bool
varsrun(const char *vardir) {
	const struct var vars[] = {
		{"CAP_VARHOME", "home", "/tmp"},
		{"CAP_VARCD", "cd", "/tmp"},
		{"CAP_VAREDITOR", "editor", "/usr/bin/vi"},
		{},
	};

	varswrite(vars, vardir);
	return varsread(vars, vardir);
}

struct cap {
	int argc;
	char **argv;
	int cmdargc;
	char **cmdargv;
	struct opts opts;
};

static void
capdel(struct cap *cap) {
	if (cap) {
		freeargv(cap->argc, cap->argv);
		freeargv(cap->cmdargc, cap->cmdargv);
		free(cap);
	}
}

static bool
optsparse(struct opts *opts, int argc, char *argv[]) {
	*opts = (struct opts){};
	optind = 0;
	
	for (;;) {
		static struct option longopts[] = {
			{"help", no_argument, 0, 'h'},
			{},
		};
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

static struct cap *
capfixcmdargs(struct cap *cap) {
	static const char *capcmds[] = {
		"home",
		"cat",
		"make",
		"cd",
		"pwd",
		"ls",
		"editor",
		"edit",
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

	// Not found command. Find to alias
	printf("Not found command of '%s'\n", cmdname);
	return NULL;
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

static struct cap *
capnew(int ac, char *av[]) {
	struct cap *cap = calloc(1, sizeof(*cap));
	if (!cap) {
		return NULL;
	}

	if (!capsolveopts(cap, ac, av)) {
		capdel(cap);
		return NULL;
	}

	if (!optsparse(&cap->opts, cap->argc, cap->argv)) {
		capdel(cap);
		return NULL;
	}
	
	return cap;
}

static bool
capsetup(const struct cap *cap) {
	char caproot[100];
	char cnfpath[100];
	char vardir[100];
	char homedir[100];

	cap_fsolve(caproot, sizeof caproot, "~/.cap2");
	if (!cap_fexists(caproot)) {
		cap_fmkdirq(caproot);
	}

	snprintf(cnfpath, sizeof cnfpath, "%s/config", caproot);
	if (!cap_fexists(cnfpath)) {
		writeconfig(cnfpath);
	}
	setenv("CAP_CONFPATH", cnfpath, 1);

	snprintf(homedir, sizeof homedir, "%s/home", caproot);
	if (!cap_fexists(homedir)) {
		cap_fmkdirq(homedir);
	}
	setenv("CAP_HOMEDIR", homedir, 1);

	snprintf(vardir, sizeof vardir, "%s/var", caproot);
	if (!cap_fexists(vardir)) {
		cap_fmkdirq(vardir);
	}
	setenv("CAP_VARDIR", vardir, 1);
	
	return varsrun(vardir);
}

static void
capusage(struct cap *cap) {
	fprintf(stderr, "Usage: cap [options] [command] [arguments]\n"
		"\n"
		"The options are:\n"
		"\n"
		"    -h, --help    show usage.\n"
		"\n"
		"The commands are:\n"
		"\n"
		"    home    change home directory, or show.\n"
		"    cd      change current directory by relative of home.\n"
		"    pwd     show current directory.\n"
		"    ls      show list in current directory.\n"
		"    cat     catenate files and show.\n"
		"    make    make buffer from cap files.\n"
		"    editor  set editor or show it.\n"
		"    run     run script.\n"
		"    alias   run alias command.\n"
		"\n"
	);
	capdel(cap);
	exit(0);
}

static void
caprun(struct cap *cap) {
	const struct opts *opts = &cap->opts;

	if (cap->cmdargc < 1 || opts->ishelp) {
		capusage(cap);
	}

	char ppath[100];
	snprintf(ppath, sizeof ppath, "../bin/cap-%s", cap->cmdargv[0]); // TODO

	pid_t pid = fork();
	switch (pid) {
	case -1:
		cap_die("failed to fork");
		break;
	case 0: // Child
		if (execv(ppath, cap->cmdargv) < 0) {
			capdel(cap);
			cap_die("failed to execute of %s", ppath);
		}
		break;
	default:// Parent
		wait(NULL);
		capdel(cap);
		exit(0);
		break;
	}
}

int
main(int argc, char *argv[]) {
	struct cap *cap = capnew(argc, argv);
	if (!cap) {
		cap_die("failed to create cap");
	}

	if (!capsetup(cap)) {
		capdel(cap);
		cap_die("failed to setup");
	}

	caprun(cap);

	return 0;
}

