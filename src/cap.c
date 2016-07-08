#include "cap.h"

struct var {
	const char *envkey;
	const char *fname;
	char defval[100];
};

const char *cmdnames[] = {
	"ls",
	"cat",
	"make",
	"cd",
	"home",
	"pwd",
	NULL,
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

static char *
readline(char *dst, size_t dstsz, const char *path) {
	FILE *fin = fopen(path, "rb");
	if (!fin) {
		return NULL;
	}

	if (cap_fgetline(dst, dstsz, fin) == EOF) {
		fclose(fin);
		return NULL;
	}

	if (fclose(fin) < 0) {
		return NULL;
	}

	return dst;
}

static const char *
writeline(const char *line, const char *path) {
	FILE *fout = fopen(path, "w");
	if (!fout) {
		return NULL;
	}

	fprintf(fout, "%s\n", line);
	fflush(fout);

	if (fclose(fout) < 0) {
		return NULL;
	}

	return line;
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
		
		if (!writeline(sval, path)) {
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
		
		if (!readline(val, sizeof val, path)) {
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

static bool
setup(int argc, char *const argv[]) {
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

/**
 * TODO
 */
static void
run(int argc, char *ap[]) {
	const char *pname = ap[1];
	char *const *argv = (char *const *)ap+1;

	char ppath[100];
	snprintf(ppath, sizeof ppath, "../bin/cap-%s", pname); // TODO

	pid_t pid = fork();
	if (pid == -1) {
		cap_die("failed to fork");
	}

	if (pid == 0) {
		// Child
		if (execv(ppath, argv) == -1) {
			cap_die("execv");
		}
	} else {
		// Parent
		wait(NULL);
		exit(0);
	}
}

struct args {
	struct cap_array *cap;
	struct cap_array *cmd;
};

static void
argsdel(struct args *args) {
	if (args) {
		cap_arrdel(args->cap);
		cap_arrdel(args->cmd);
		free(args);
	}
}

static struct args *
argsnew(int argc, char *argv[]) {
	struct args *args = calloc(1, sizeof(*args));
	if (!args) {
		return NULL;
	}

	args->cap = cap_arrnew();
	args->cmd = cap_arrnew();
	if (!args->cap || !args->cap) {
		argsdel(args);
		return NULL;
	}

	struct cap_array *arr = args->cap; // Current array for parse
	cap_arrpush(arr, argv[0]);

	for (int i = 1; i < argc; ++i) {
		const char *ag = argv[i];
		
		if (ag[0] != '-') {
			arr = args->cmd;
		}

		cap_arrpush(arr, ag);
	}

	arrdump(args->cap, stdout);
	printf("<\n");
	arrdump(args->cmd, stdout);
	
	return args;
}

static void
arrdump(const struct cap_array *arr, FILE *fout) {
	for (size_t i = 0; i < cap_arrlen(arr); ++i) {
		fprintf(fout, "%s\n", cap_arrgetc(arr, i));
	}
	fflush(fout);
}

int
main(int argc, char *argv[]) {
	struct args *args = argsnew(argc, argv);
	if (!args) {
		cap_die("failed to parse args");
	}
	argsdel(args);
	return 0;

	if (!setup(argc, argv)) {
		cap_die("failed to setup");
	}

	run(argc, argv);

	return 0;
}

