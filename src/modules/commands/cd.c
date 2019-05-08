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

	if (is_out_of_home(self->config->home_path, newcd)) {
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
	if (self->argc < 2) {
		if (!cdcmd_cd(self, self->config->home_path)) {
			err_die("failed to cd");
		}
		return 0;
	}

	const char *argpath = self->argv[1];
	const char *org;
	char path[FILE_NPATH];

	if (argpath[0] == '/' || argpath[0] == '\\') {
		// Absolute of home
		org = self->config->home_path;
	} else {
		// Relative of cd
		org = self->config->cd_path;
	}

	if (!strcmp(argpath, "/")) {
		if (!file_solvefmt(path, sizeof path, "%s", org)) {
			err_die("failed to solve path");
		}
	} else {
		if (!file_solvefmt(path, sizeof path, "%s/%s", org, argpath)) {
			err_die("failed to solve path (2)");
		}		
	}

	if (!cdcmd_cd(self, path)) {
		err_die("failed to cd");
	}

	return 0;
}
