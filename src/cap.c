#include "cap.h"

struct var {
	const char *envkey;
	const char *fname;
	char defval[100];
};

static bool
putconfigto(const char *cnfpath) {
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
writeto(const char *path, const char *line) {
	if (cap_fexists(path)) {
		return false;
	}
	FILE *fout = fopen(path, "w");
	if (!fout) {
		return false;
	}
	fprintf(fout, "%s\n", line);
	fflush(fout);
	return fclose(fout) == 0;
}

static bool
combwriteto(const char *vardir, const char *fname, const char *line) {
	char path[100];
	char slvline[100];

	cap_fsolve(slvline, sizeof slvline, line);
	snprintf(path, sizeof path, "%s/%s", vardir, fname);

	return writeto(path, slvline);
}

static bool
putvarsin(const struct var *vars, const char *vardir) {
	for (const struct var *p = vars; p->envkey; ++p) {
		combwriteto(vardir, p->fname, p->defval);
	}
	return true;
}

static bool
readenvfrom(const struct var *vars, const char *vardir) {
	char path[100];

	for (const struct var *p = vars; p->envkey; ++p) {
		snprintf(path, sizeof path, "%s/%s", vardir, p->fname);
	
		FILE *fin = fopen(path, "rb");
		if (!fin) {
			cap_log("error", "fopen %s", path);
			continue;
		}
	
		char val[100];
		fgets(val, sizeof val, fin);

		size_t vallen = strlen(val);
		if (val[vallen-1] == '\n') {
			val[--vallen] = '\0';
		}
	
		setenv(p->envkey, val, 1);
		fclose(fin);
	}
	
	return true;
}

static bool
setup(int argc, char *const argv[]) {
	char caproot[100];
	char cnfpath[100];
	char vardir[100];
	char homedir[100];
	
	const struct var vars[] = {
		{"CAP_VARHOME", "home", "/tmp"},
		{"CAP_VARCD", "cd", "/tmp"},
		{"CAP_VAREDITOR", "editor", "/usr/bin/vi"},
		{},
	};

	cap_fsolve(caproot, sizeof caproot, "~/.cap2");
	if (!cap_fexists(caproot)) {
		cap_fmkdirq(caproot);
	}

	snprintf(cnfpath, sizeof cnfpath, "%s/config", caproot);
	if (!cap_fexists(cnfpath)) {
		putconfigto(cnfpath);
	}

	snprintf(homedir, sizeof homedir, "%s/home", caproot);
	if (!cap_fexists(homedir)) {
		cap_fmkdirq(homedir);
	}

	snprintf(vardir, sizeof vardir, "%s/var", caproot);
	if (!cap_fexists(vardir)) {
		cap_fmkdirq(vardir);
	}
	putvarsin(vars, vardir);

	setenv("CAP_CONFPATH", cnfpath, 1);
	setenv("CAP_HOMEDIR", homedir, 1);
	setenv("CAP_VARDIR", vardir, 1);

	if (!readenvfrom(vars, vardir)) {
		cap_log("error", "readenvfrom");
		return false;
	}

	return true;
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
		cap_die("fork");
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

int
main(int argc, char *argv[]) {
	if (!setup(argc, argv)) {
		cap_die("Failed to setup");
	}

	run(argc, argv);

	return 0;
}

