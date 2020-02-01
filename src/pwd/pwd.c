/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016
 */
#include "pwd/pwd.h"

struct opts {
    bool ishelp;
    bool isnorm;
};

struct pwdcmd {
    const config_t *config;
    int argc;
    char **argv;
    struct opts opts;
};

static bool
pwdcmd_parse_opts(pwdcmd_t *self) {
    // Parse options
    struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {0},
    };
    const char *shortopts = "hn";

    self->opts = (struct opts){0};
    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(self->argc, self->argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->opts.ishelp = true; break;
        case 'n': self->opts.isnorm = true; break;
        case '?':
        default: perror("Unknown option"); break;
        }
    }

    if (self->argc < optind) {
        perror("Failed to parse option");
        return false;
    }

    return true;
}

void
pwdcmd_del(pwdcmd_t *self) {
	if (!self) {
        return;
    }

    free(self);
}

pwdcmd_t *
pwdcmd_new(const config_t *config, int argc, char **argv) {
	pwdcmd_t *self = mem_ecalloc(1, sizeof(*self));
	self->config = config;
    self->argc = argc;
    self->argv = argv;
	return self;
}

int
pwdcmd_run(pwdcmd_t *self) {
    if (!pwdcmd_parse_opts(self)) {
        err_error("failed to parse option");
        return 1;
    }

	char cd[FILE_NPATH];
	if (!file_readline(cd, sizeof cd, self->config->var_cd_path)) {
		err_error("need environment variable of cd");
		return 2;
	}

    char home[FILE_NPATH];
    if (!file_readline(home, sizeof home, self->config->var_home_path)) {
        err_error("need environment variable of home");
        return 3;
    }

    if (self->opts.isnorm) {
    	printf("%s\n", cd);
    } else {
        int32_t homelen = strlen(home);
        int32_t cdlen = strlen(cd);
        if (cdlen-homelen < 0) {
            err_error("invalid cd \"%s\" or home \"%s\"", cd, home);
            return 4;
        }
        if (cdlen-homelen == 0) {
            printf("/\n");
        } else {
            const char *p = cd + homelen;
            printf("%s\n", p);
        }
    }

	return 0;
}
