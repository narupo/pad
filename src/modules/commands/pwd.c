/**
 * Cap
 *
 * License: MIT
 *  Author: Aizawa Yuta
 *   Since: 2016, 2018
 */
#include "modules/commands/pwd.h"

struct opts {
    bool ishelp;
    bool isnorm;
};

static bool
optsparse(struct opts *self, int argc, char *argv[]) {
    // Parse options
    struct option longopts[] = {
        {"help", no_argument, 0, 'h'},
        {"normalize", no_argument, 0, 'n'},
        {},
    };
    const char *shortopts = "hn";

    extern int opterr;
    extern int optind;
    opterr = 0; // ignore error messages
    optind = 0; // init index of parse

    for (;;) {
        int optsindex;
        int cur = getopt_long(argc, argv, shortopts, longopts, &optsindex);
        if (cur == -1) {
            break;
        }

        switch (cur) {
        case 0: /* Long option only */ break;
        case 'h': self->ishelp = true; break;
        case 'n': self->isnorm = true; break;
        case '?':
        default: perror("Unknown option"); break;
        }
    }

    if (argc < optind) {
        perror("Failed to parse option");
        return false;
    }

    return true;
}

struct pwdcmd {
	config_t *config;
	cmdargs_t *cmdargs;
};

void
pwdcmd_del(pwdcmd_t *self) {
	if (self) {
		free(self);
	}
}

pwdcmd_t *
pwdcmd_new(config_t *config, cmdargs_t *cmdargs) {
	pwdcmd_t *self = mem_ecalloc(1, sizeof(*self));
	self->config = config;
	self->cmdargs = cmdargs;
	return self;
}

int
pwdcmd_run(pwdcmd_t *self) {
	int argc = cmdargs_get_argc(self->cmdargs);
	char **argv = cmdargs_get_argv(self->cmdargs);

    struct opts opts = {0};
    if (!optsparse(&opts, argc, argv)) {
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

    if (opts.isnorm) {
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
