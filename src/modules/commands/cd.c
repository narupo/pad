/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "modules/commands/cd.h"

struct cdcmd {
    config_t *config;
    int argc;
    char **argv;
};

void
cdcmd_del(cdcmd_t *self) {
	if (self) {
		config_del(self->config);
		freeargv(self->argc, self->argv);
		free(self);
	}
}

cdcmd_t *
cdcmd_new(config_t *config, int argc, char *argv[]) {
	cdcmd_t *self = mem_ecalloc(1, sizeof(*self));
	self->config = config;
	self->argc = argc;
	self->argv = argv;
	return self;
}

bool
cdcmd_cd(cdcmd_t *self, const char *drtpath) {
	char newcd[FILE_NPATH];
	file_solve(newcd, sizeof newcd, drtpath);

	if (isoutofhome(self->config->var_home_path, newcd)) {
		err_die("'%s' is out of home", newcd);
	}

	if (!file_isdir(newcd)) {
		err_die("'%s' is not a directory", newcd);
	}

	if (!file_writeline(newcd, self->config->var_cd_path)) {
		err_die("invalid var cd path");
	}

	return true;
}

int
cdcmd_run(cdcmd_t *self) {
	int argc = self->argc;
	char **argv = self->argv;

	const char *varcdpath = self->config->var_cd_path;
	const char *varhomepath = self->config->var_home_path;
	char varcdval[FILE_NPATH];
	char varhomeval[FILE_NPATH];
	if (!file_readline(varcdval, sizeof varcdval, varcdpath)) {
		err_die("invalid var cd path");
	}
	if (!file_readline(varhomeval, sizeof varhomeval, varhomepath)) {
		err_die("invalid var home path");
	}

	if (argc < 2) {
		if (!cdcmd_cd(self, varhomeval)) {
			err_die("failed to cd");
		}
		return 0;
	}

	char path[FILE_NPATH];

	if (argv[1][0] == '/' || argv[1][0] == '\\') {
		// Absolute of home
		file_solvefmt(path, sizeof path, "%s/%s", varhomeval, argv[1]);
	} else {
		// Relative of cd
		file_solvefmt(path, sizeof path, "%s/%s", varcdval, argv[1]);
	}

	if (!cdcmd_cd(self, path)) {
		err_die("failed to cd");
	}

	return 0;
}
