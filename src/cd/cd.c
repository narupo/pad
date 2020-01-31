/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "cd/cd.h"

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
	char normpath[FILE_NPATH];
	if (!symlink_norm_path(self->config, normpath, sizeof normpath, drtpath)) {
		err_error("failed to normalize path");
		return false;
	}

	char realpath[FILE_NPATH];
	if (!symlink_follow_path(self->config, realpath, sizeof realpath, normpath)) {
		err_error("failed to follow path");
		return false;
	}

	if (is_out_of_home(self->config->home_path, realpath)) {
		err_error("\"%s\" is out of home", normpath);
		return false;
	}

	if (!file_isdir(realpath)) {
		err_error("\"%s\" is not a directory", normpath);
		return false;
	}

	if (!file_writeline(normpath, self->config->var_cd_path)) {
		err_error("invalid var cd path");
		return false;
	}

	return true;
}

int
cdcmd_run(cdcmd_t *self) {
	if (self->argc < 2) {
		if (!cdcmd_cd(self, self->config->home_path)) {
			return 1;
		}
		return 0;
	}

	const char *argpath = self->argv[1];
	const char *org;
	char drtpath[FILE_NPATH*2];

	if (argpath[0] == '/' || argpath[0] == '\\') {
		// Absolute of home
		org = self->config->home_path;
	} else {
		// Relative of cd
		org = self->config->cd_path;
	}

	if (!strcmp(argpath, "/")) {
		snprintf(drtpath, sizeof drtpath, "%s", org);
	} else {
		snprintf(drtpath, sizeof drtpath, "%s/%s", org, argpath);
	}

	if (!cdcmd_cd(self, drtpath)) {
		return 1;
	}

	return 0;
}
