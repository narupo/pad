#include "cap.h"

/*
	@cap {{ cat file.c }}
*/

static bool
putconfig(const char *path) {
	FILE *fout = fopen(path, "w");
	if (!fout) {
		return false;
	}

	char tmp[100];

	fprintf(fout, "# Cap's config\n");
	fprintf(fout, "home = \"%s\"\n", cap_fsolve(tmp, sizeof tmp, "/tmp"));
	fprintf(fout, "cd = \"%s\"\n", cap_fsolve(tmp, sizeof tmp, "/tmp"));
	fprintf(fout, "editor = \"%s\"\n", cap_fsolve(tmp, sizeof tmp, "/usr/bin/vi"));

	if (fclose(fout) != 0) {
		return false;
	}

	return true;
}

static bool
deployenv(const char *confpath) {
	struct cap_config *conf = cap_confnewfile(confpath);
	if (!conf) {
		return false;
	}

	char tmp[100];
	
	setenv("CAP_CONFPATH", confpath, 1);
	setenv("CAP_HOME", cap_fsolve(tmp, sizeof tmp, cap_confgetc(conf, "home")), 1);
	setenv("CAP_CD", cap_fsolve(tmp, sizeof tmp, cap_confgetc(conf, "cd")), 1);
	setenv("CAP_EDITOR", cap_fsolve(tmp, sizeof tmp, cap_confgetc(conf, "editor")), 1);

	cap_confdel(conf);
	return true;
}

static bool
setup(int argc, char *const argv[]) {
	char path[100];
	char tmp[100];
	cap_fsolve(path, sizeof path, "~");

	snprintf(tmp, sizeof tmp, "%s/.cap2", path);
	if (!cap_fexists(tmp)) {
		cap_fmkdirq(tmp);
	}

	snprintf(tmp, sizeof tmp, "%s/.cap2/config", path);
	if (!cap_fexists(tmp)) {
		putconfig(tmp);
	}

	if (!deployenv(tmp)) {
		cap_log("error", "deployenv");
		return false;
	}

	return true;
}

int
main(int argc, char *ap[]) {
	if (!setup(argc, ap)) {
		cap_log("error", "setup");
		return 1;
	}

	const char *pname = ap[1];
	char *const *argv = (char *const *)ap+1;

	char ppath[100];
	snprintf(ppath, sizeof ppath, "../bin/cap-%s", pname); // TODO

	// Fork
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

	return 0;
}
